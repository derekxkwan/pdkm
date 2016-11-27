/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <string.h>
#include "m_pd.h"
#include <stdint.h>
#include "dkrnd.h"


#define MKV2DIM 12 //dimension of second-markov table
#define MKV2MAX 128 //max dim

static t_class *dkmkv_class;

typedef struct _dkmkv {
	t_object x_obj;
        dkrnd *x_rand; //pointer to my dkrnd class
        int x_offset; //offset in input
        int x_num; //number of notes input so far, won't go past two.
        t_float x_mode; //0 = play mode, 1 = rec mode
        int x_dim; //dimensions of the table
        t_float x_prev; //previous note
        uint32_t ** x_table; //first  = first note, second = second note, third = quanities of note
         //note: leave an extra entry in third for total notes

} t_dkmkv;

static void dkmkv_dump(t_dkmkv  *x){

    int i,j;
    char retstr[x->x_dim * 100];
    int offset = x->x_offset;
    size_t curlen = 0; //current length of string
    int strtopost = 0; //flag if there's a string to post
    for(i=0; i<x->x_dim; i++){
        //looping over first note
        int firstnote = offset + i;
        if(x->x_table[i][x->x_dim] > 0){
            //if there are actually notes to report
            strtopost = 1;
            sprintf(retstr + curlen, "[%d] \0", firstnote);
            curlen = strlen(retstr);
            for(j=0;j<x->x_dim;j++){
                int val = x->x_table[i][j];
                if(val > 0){
                    int note = offset+j;
                    sprintf(retstr + curlen, "%d: %d, \0", note, val);
                    curlen = strlen(retstr);

                };
            };
        };
    };
    if(strtopost){
             post(retstr);
        };

}


static void dkmkv_prev(t_dkmkv  *x, t_float f){
    x->x_prev = f;
}

static void dkmkv_unalloc(t_dkmkv *x){
    int i;
    int dim = x->x_dim;
    x->x_num = 0;
    for(i=0;i<dim; i++){
        free(x->x_table[i]);
    };

}

static void dkmkv_clear(t_dkmkv *x){
    int i,j,k;
    int dim = x->x_dim;
    //0 out array
    for(i=0;i<dim; i++){
        for(j=0;j<dim; j++){
            x->x_table[i][j] = 0;
      };
    };

    //reset prevs and nums
    x->x_prev = x->x_offset;
    x->x_num = 0;

}

static void dkmkv_dim(t_dkmkv *x, t_float _dim){
   //clears old cotents of x_table and reallocs if needed 
    int i,j;
    int dimold = x->x_dim; //old dim
    int dim = (int)_dim;
    if(dim != dimold){
        dkmkv_unalloc(x);
        uint32_t ** arr = (uint32_t **)calloc(dim,sizeof(uint32_t *));
        for(i=0; i<dim; i++){
           arr[i] = (uint32_t *)calloc(dim,sizeof(uint32_t)); 
        };
        if(arr){
            x->x_table = arr;
            x->x_dim = dim;
            x->x_prev = x->x_offset;
        }
        else{
            pd_error(x, "dkmkv: realloc failed!");
        };
    
    }
    else{
            dkmkv_clear(x);
    };
}

static void dkmkv_seed(t_dkmkv *x, t_float f){
	dkrnd_seed(x->x_rand,(unsigned int)f);
}


static void dkmkv_offset(t_dkmkv * x, t_float f){
    int oldoff = x->x_offset; //old offset
    int offset = (int)f;
    if(offset < 0){
        offset = 0;
    };
    x->x_offset = offset;
    //if there's a difference between old offset and new offset, change prevs
    int diff = (offset-oldoff);
    if(diff){
        x->x_prev = x->x_prev + diff;
    };
}

static int bmap(t_dkmkv *x, t_float ipt){
    //checking bounds of input and mapping
    int offset = (int)x->x_offset;
    int dim = x->x_dim;
    int ret = (int)ipt; //return value
    if(ret < offset){
        ret = offset;
    }
    else if(ret >= (offset+dim)){
        //values co from offset to offset+ dim -1 inclusive since 0-index
        ret = offset + dim - 1;
    };
    //now to bring it to x_table dimensions

    ret -= offset;

    return ret;
}

static void dkmkv_setnext(t_dkmkv *x,t_float ipt){
    int input = bmap(x,ipt);
    if(x->x_num >= 1){
        int prev = bmap(x,(t_float)x->x_prev);
        //if total is less than max, go ahead and add
        unsigned int total = x->x_table[prev][x->x_dim];
        if(total < UINT32_MAX){
            x->x_table[prev][x->x_dim]=++total;
            unsigned int curval = x->x_table[prev][input];
            x->x_table[prev][input] = ++curval;
        };
    };
    //increment
    x->x_prev = input + x->x_offset;
    if(x->x_num < 1){
        x->x_num = x->x_num + 1;
    };
}

static int dkmkv_getnext(t_dkmkv *x){
        int prev;
        int dim = x->x_dim;
        int offset = x->x_offset;
        switch(x->x_num){
            case 0:
                prev = (int)(offset + (int)(dkrnd_next(x->x_rand)*dim));
                break;
            default:
                prev = bmap(x,(t_float)x->x_prev);
                break;
        };
            double curnotes[dim];
            int i=0;
            uint32_t total = x->x_table[prev][dim];
            double currand = dkrnd_next(x->x_rand); //current random number

            double runsum = (double)x->x_table[prev][0]*(1.0/total); //running sum, should add up to 1.0 at end
            //idea is running sum should eventually equal 1.0
            //scan through the notes adding their proportions to runsum, if runsum > currand, we found our note
            int note = 0; //note to return
            //post("currand:%f, runsum: %f",currand, runsum);
            while(note< dim && runsum < currand){
                note++;
                runsum += (double)x->x_table[prev][note]*(1.0/total);
            };
            note += offset; //apply offset
            //set prevs
            x->x_prev = note;
        //set number of notes seen/generated if < 2 to 2 (basically use to generate rands for prevs
        if(x->x_num < 1){    
            x->x_num =1;
        };
        return note;
}

static void *dkmkv_free(t_dkmkv *x){
    dkmkv_unalloc(x);
    dkrnd_free(x->x_rand);
    return (void *)x;
}

static void *dkmkv_new(t_symbol * s, int argc, t_atom * argv){
        //arg order: dim, offset
	t_dkmkv *x = (t_dkmkv *)pd_new(dkmkv_class);
        int argnum = 0;
        int setseed = 0; //if seed is set
        //defaults
        int dim = MKV2DIM;
        int offset = 0;
        t_float seed = 0;
        while(argc){
            if(argv->a_type == A_FLOAT){
                t_float curf = atom_getfloatarg(0, argc, argv);
                switch(argnum){
                    case 0:
                        dim = (int)curf;
                        break;
                    case 1:
                        offset = (int)offset;
                        break;
                    default:
                        break;
                };
                argnum++;
                argc--;
                argv++;
                }
            else{
                if(argc >= 2){
                    t_symbol * curs = atom_getsymbolarg(0, argc, argv);
                    t_float curf = atom_getfloatarg(1, argc, argv);
                    if(strcmp(curs->s_name, "-dim") == 0){
                        dim = (int)curf;
                        }
                    else if(strcmp(curs->s_name, "-offset") == 0){
                        offset = (int)offset;
                        }
                    else if(strcmp(curs->s_name, "-seed") == 0){
                        setseed = 1;
                        seed = curf;
                        };
                    argc-=2;
                    argv+=2;
                };

            };
        };

        //boundschecking
        if(dim < 0){
            dim = 0;
        }
        else if(dim >= MKV2MAX){
            dim = MKV2MAX;
        };

        x->x_dim = dim;
        x->x_mode = 0;
        x->x_num = 0;
        x->x_offset = offset;
        x->x_prev = offset;
        x->x_rand = dkrnd_new(0);
        if(setseed){
            dkmkv_seed(x,seed);
        };

        //allocate x_table
        int i,j;
        uint32_t ** arr = (uint32_t **)calloc(dim,sizeof(uint32_t *));
        for(i=0; i<dim; i++){
           arr[i] = (uint32_t *)calloc(dim,sizeof(uint32_t)); 
        };
        if(arr){
            x->x_table = arr;
        }
        else{
            goto errstate;
        };
	outlet_new(&x->x_obj, &s_float);
	return (x);
            errstate:
                post("dkmkv: unable to create");
                return NULL;
}

static void dkmkv_mode(t_dkmkv * x, t_float f){
    if(f<=0){
        x->x_mode = 0; //play mode
    }
    else{
        x->x_mode = 1; //rec mode
    }
}

static void dkmkv_rec(t_dkmkv * x){
    x->x_mode = 1;
}

static void dkmkv_play(t_dkmkv * x){
    x->x_mode = 0;
}

static void dkmkv_bang(t_dkmkv *x){
    if(x->x_mode == 0){ 
        //if in play mode, get next note
        int nval = dkmkv_getnext(x); 
	outlet_float(x->x_obj.ob_outlet, (t_float)nval);
    };
}

static void dkmkv_float(t_dkmkv *x, t_float f){
    if(x->x_mode == 1){
        //if in record mode, record it
        dkmkv_setnext(x, f);
    }
    else{
        //else in play mode, count it as a bang and make it prev
        x->x_prev = bmap(x,f);
        int nval = dkmkv_getnext(x);
	outlet_float(x->x_obj.ob_outlet, (t_float)nval);
    };
}

void dkmkv_setup(void){
	dkmkv_class = class_new(gensym("dkmkv"), (t_newmethod)dkmkv_new,
                (t_method)dkmkv_free, sizeof(t_dkmkv), 0, A_GIMME, 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_seed, gensym("seed"), A_FLOAT, 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_prev, gensym("prev"), A_FLOAT, 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_dim, gensym("dim"), A_FLOAT, 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_offset, gensym("offset"), A_FLOAT, 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_mode, gensym("mode"), A_FLOAT, 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_rec, gensym("rec"), 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_play, gensym("play"), 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_clear, gensym("clear"), 0);
	class_addmethod(dkmkv_class, (t_method)dkmkv_dump, gensym("dump"), 0);
	class_addbang(dkmkv_class, dkmkv_bang);
	class_addfloat(dkmkv_class, dkmkv_float);
}
