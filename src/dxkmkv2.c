/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <string.h>
#include "m_pd.h"
#include <stdint.h>
#include "dxkrand.h"


#define dxkmkv2DIM 12 //dimension of second-markov table
#define dxkmkv2MAX 128 //max dim

static t_class *dxkmkv2_class;

typedef struct _dxkmkv2 {
	t_object x_obj;
        dxkrand *x_rand; //pointer to my dxkrand class
        t_float x_offset; //offset in input
        int x_num; //number of notes input so far, won't go past two.
        t_float x_mode; //0 = play mode, 1 = rec mode
        int x_dim; //dimensions of the table
        t_float x_prev; //previous note
        t_float x_prev2; //previous note to the previous note
        uint32_t *** x_table; //first  = first note, second = second note, third = quanities of note
         //note: leave an extra entry in third for total notes

} t_dxkmkv2;

static void dxkmkv2_prev2(t_dxkmkv2  *x, t_float f){
    x->x_prev2 = f;
}

static void dxkmkv2_prev(t_dxkmkv2  *x, t_float f){
    x->x_prev = f;
}

static void dxkmkv2_unalloc(t_dxkmkv2 *x){
    int i,j;
    int dim = x->x_dim;
    for(i=0;i<dim; i++){
        for(j=0;j<dim; j++){
            free(x->x_table[i][j]);
        };
        free(x->x_table[i]);
    };

}

static void dxkmkv2_clear(t_dxkmkv2 *x){
    int i,j,k;
    int dim = x->x_dim;
    //0 out array
    for(i=0;i<dim; i++){
        for(j=0;j<dim; j++){
            for(k=0; k<=dim; k++){
                x->x_table[i][j][k] = 0;
            };
      };
    };

    //reset prevs and nums
    x->x_prev2 = x->x_offset;
    x->x_prev = x->x_offset;
    x->x_num = 0;

}

static void dxkmkv2_dim(t_dxkmkv2 *x, t_float _dim){
   //clears old cotents of x_table and reallocs if needed 
    int i,j;
    int dimold = x->x_dim; //old dim
    int dim = (int)_dim;
    if(dim != dimold){
        dxkmkv2_unalloc(x);
        uint32_t *** arr = (uint32_t ***)calloc(dim,sizeof(uint32_t **));
        for(i=0; i<dim; i++){
           arr[i] = (uint32_t **)calloc(dim,sizeof(uint32_t *)); 
            for(j=0; j<dim; j++){
            arr[i][j] = (uint32_t *)calloc((dim+1),sizeof(uint32_t));
            };
        };
        if(arr){
            x->x_table = arr;
            x->x_dim = dim;
            x->x_prev = x->x_offset;
            x->x_prev2 = x->x_offset;
        }
        else{
            pd_error(x, "dxkmkv2: realloc failed!");
        };
    
    }
    else{
            dxkmkv2_clear(x);
    };
}

static void dxkmkv2_seed(t_dxkmkv2 *x, t_float f){
	dxkrand_seed(x->x_rand,(unsigned int)f);
}


static void dxkmkv2_offset(t_dxkmkv2 * x, t_float f){
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
        x->x_prev2 = x->x_prev2 + diff;
    };
}

static int bmap(t_dxkmkv2 *x, t_float ipt){
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

static void dxkmkv2_setnext(t_dxkmkv2 *x,t_float ipt){
    int input = bmap(x,ipt);
    if(x->x_num >= 2){
        int prev = bmap(x,(t_float)x->x_prev);
        int prev2 = bmap(x,(t_float)x->x_prev2);
        //if total is less than max, go ahead and add
        int total = x->x_table[prev2][prev][x->x_dim];
        if((unsigned int)total < UINT32_MAX){
            total++;
            x->x_table[prev2][prev][x->x_dim]=total;
            int curval = x->x_table[prev2][prev][input];
            curval++;
            x->x_table[prev2][prev][input] = curval;
        };
    };
    //increment
    x->x_prev2 = x->x_prev;
    x->x_prev = input + x->x_offset;
    if(x->x_num < 2){
        x->x_num = x->x_num + 1;
    };
}

static int dxkmkv2_getnext(t_dxkmkv2 *x){
        int prev, prev2;
        int dim = x->x_dim;
        int offset = x->x_offset;
        switch(x->x_num){
            case 0:
                prev2 = (int)(offset + (int)(dxkrand_next(x->x_rand)*dim));
            case 1:
                prev = (int)(offset + (int)(dxkrand_next(x->x_rand)*dim));
                break;
            default:
                prev = bmap(x,(t_float)x->x_prev);
                prev2 = bmap(x,(t_float)x->x_prev2);
                break;
        };
        double curnotes[dim];
        int i=0;
        uint32_t total = x->x_table[prev2][prev][dim];
        double currand = dxkrand_next(x->x_rand); //current random number

        double runsum = x->x_table[prev2][prev][0]*(1.0/total); //running sum, should add up to 1.0 at end
        //idea is running sum should eventually equal 1.0
        //scan through the notes adding their proportions to runsum, if runsum > currand, we found our note
        int note = 0; //note to return
        while(note< dim && runsum < currand){
            note++;
            runsum += x->x_table[prev2][prev][note]*(1.0/total);
        };
        note += offset; //apply offset
        //set prevs
        x->x_prev2 = x->x_prev;
        x->x_prev = note;
        //increment number of notes seen/generated if < 2
        if(x->x_num < 2){
            x->x_num = x->x_num + 1;
        };
        return note;
}

static void *dxkmkv2_free(t_dxkmkv2 *x){
    dxkmkv2_unalloc(x);
    dxkrand_free(x->x_rand);
    return (void *)x;
}

static void *dxkmkv2_new(t_symbol * s, int argc, t_atom * argv){
        //arg order: dim, offset
	t_dxkmkv2 *x = (t_dxkmkv2 *)pd_new(dxkmkv2_class);
        int argnum = 0;
        int setseed = 0; //if seed is set
        //defaults
        int dim = dxkmkv2DIM;
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
        else if(dim >= dxkmkv2MAX){
            dim = dxkmkv2MAX;
        };

        x->x_dim = dim;
        x->x_mode = 0;
        x->x_offset = offset;
        x->x_prev = offset;
        x->x_prev2 = offset;
        x->x_rand = dxkrand_new(0);
        if(setseed){
            dxkmkv2_seed(x,seed);
        };

        //allocate x_table
        int i,j;
        uint32_t *** arr = (uint32_t ***)calloc(dim,sizeof(uint32_t **));
        for(i=0; i<dim; i++){
           arr[i] = (uint32_t **)calloc(dim,sizeof(uint32_t *)); 
            for(j=0; j<dim; j++){
            arr[i][j] = (uint32_t *)calloc((dim+1),sizeof(uint32_t));
            };
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
                post("dxkmkv2: unable to create");
                return NULL;
}

static void dxkmkv2_mode(t_dxkmkv2 * x, t_float f){
    if(f<=0){
        x->x_mode = 0; //play mode
    }
    else{
        x->x_mode = 1; //rec mode
    }
}

static void dxkmkv2_rec(t_dxkmkv2 * x){
    x->x_mode = 1;
}

static void dxkmkv2_play(t_dxkmkv2 * x){
    x->x_mode = 0;
}

static void dxkmkv2_bang(t_dxkmkv2 *x){
    if(x->x_mode == 0){ 
        //if in play mode, get next note
        int nval = dxkmkv2_getnext(x); 
	outlet_float(x->x_obj.ob_outlet, (t_float)nval);
    };
}

static void dxkmkv2_float(t_dxkmkv2 *x, t_float f){
    if(x->x_mode == 1){
        //if in record mode, record it
        dxkmkv2_setnext(x, f);
    }
    else{
        //else in play mode, count it as a bang and make it prev, scooting everything over
        x->x_prev2 = x->x_prev;
        x->x_prev = bmap(x,f);
        int nval = dxkmkv2_getnext(x);
	outlet_float(x->x_obj.ob_outlet, (t_float)nval);
    };
}

void dxkmkv2_setup(void){
	dxkmkv2_class = class_new(gensym("dxkmkv2"), (t_newmethod)dxkmkv2_new,
                (t_method)dxkmkv2_free, sizeof(t_dxkmkv2), 0, A_GIMME, 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_seed, gensym("seed"), A_FLOAT, 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_prev, gensym("prev"), A_FLOAT, 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_prev2, gensym("prev2"), A_FLOAT, 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_dim, gensym("dim"), A_FLOAT, 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_offset, gensym("offset"), A_FLOAT, 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_mode, gensym("mode"), A_FLOAT, 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_rec, gensym("rec"), 0);
	class_addmethod(dxkmkv2_class, (t_method)dxkmkv2_play, gensym("play"), 0);
	class_addbang(dxkmkv2_class, dxkmkv2_bang);
	class_addfloat(dxkmkv2_class, dxkmkv2_float);
}
