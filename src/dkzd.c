/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include <stdlib.h>
#include "m_pd.h"


#define DKZDBUF 4096 //default bufsize
#define DKZDMAX 441000 //max buffer size

static t_class *dkzd_tilde_class;

typedef struct _dkzd_tilde {
	t_object x_obj;
	t_float x_in; 
	int x_z; //delay in samples
        double x_stb[DKZDBUF]; //stack buffer
        double * x_buf;//buffer pointer
        int x_alloc; //if allocated or not
        int x_sz; //buffer size
        int x_wh; //writehead for buffer
        t_inlet *x_dellet;
        t_outlet *x_outlet;
} t_dkzd_tilde;

static void dkzd_tilde_clear(t_dkzd_tilde *x){
    int i;
    for(i=0; i<x->x_sz; i++){
        x->x_buf[i] = 0.;
    };
    x->x_wh = 0;
}

static void dkzd_tilde_delay(t_dkzd_tilde *x, t_float f){
    int del = (int)f;
    if(del < 0){
        del = 0;
    }
    else if(del >= x->x_sz){
        del = x->x_sz -1;
    };
    x->x_z = del;
    post("del: %d", x->x_z);

}

static void dkzd_tilde_sz(t_dkzd_tilde *x, t_float f){
    int newsz = (int)f; //desired size
    int alloc = x->x_alloc;
    int cursz = x->x_sz; //current size

    if(newsz < 0){
        newsz = 0;
    }
    else if(newsz > DKZDMAX){
        newsz = DKZDMAX;
    };
    if(!alloc && newsz > DKZDBUF){
        post("1");
        x->x_buf = (double *)malloc(sizeof(double)*newsz);
        x->x_alloc = 1;
        x->x_sz = newsz;
    }
    else if(alloc && newsz > cursz){
        post("2");
        x->x_buf = (double *)realloc(x->x_buf, sizeof(double)*newsz);
        x->x_sz = newsz;
    }
    else if(alloc && newsz < DKZDBUF){
        post("3");
        free(x->x_buf);
        x->x_buf = x->x_stb;
        x->x_alloc = 0;
    };
    dkzd_tilde_clear(x);
    post("bufsz: %d", x->x_sz);
}

static void *dkzd_tilde_new(t_symbol *s, int argc, t_atom * argv){

        //could use t_floatarg f, but this is an easy way to check of an arg is passed or not
	t_dkzd_tilde *x = (t_dkzd_tilde *)pd_new(dkzd_tilde_class);

        t_float del = 0; //default delay
        t_float bufsz = DKZDBUF; //default buf size
        int argnum = 0; //current argument
       
        //setting up x_buf to point to x_stb to start out
        int i;
        for(i=0; i<DKZDBUF; i++){
            x->x_stb[i] = 0.;
        };
        x->x_alloc = 0;
        x->x_sz = DKZDBUF;
        x->x_buf = x->x_stb;
       
        //parsing args
        while(argc){
            if(argv->a_type == A_FLOAT){
                t_float curf = (int)atom_getfloatarg(0,argc,argv);
                switch(argnum){
                    case 0: 
                        del = curf;
                        break;
                    case 1:
                        bufsz = curf;
                        break;
                    default:
                        break;
                };
                argnum++;
            };
            argc--;
            argv++;
        };
	
        dkzd_tilde_sz(x, bufsz);
        dkzd_tilde_delay(x, del);
        dkzd_tilde_clear(x);
        
	x->x_dellet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("delay"));
        x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkzd_tilde_perform(t_int *w){
	 t_dkzd_tilde *x = (t_dkzd_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	int i;
        int sz = x->x_sz; //size of buffer
        int z = x->x_z; //delay in samples
        for(i=0;i<n;i++){
            double output;
            double input = (double)in[i];
            int wh = x->x_wh; //writehead

            //write current input to buffer
            x->x_buf[wh] = input;

            //get delayed input from buffer
            int rh = (wh +  z) % sz;
            out[i] = x->x_buf[rh];
            //increment
            x->x_wh = (wh + 1) % sz;
            
            
        };
	return(w+5);
}

static void dkzd_tilde_dsp(t_dkzd_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(dkzd_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}




static void *dkzd_tilde_free(t_dkzd_tilde *x){
	inlet_free(x->x_dellet);
        if(x->x_alloc){
            free(x->x_buf);
        };
        outlet_free(x->x_outlet);
	
	return (void *)x;
}




void dkzd_tilde_setup(void){
	dkzd_tilde_class = class_new(gensym("dkzd~"), (t_newmethod)dkzd_tilde_new,
                (t_method)dkzd_tilde_free,
			sizeof(t_dkzd_tilde), 0, A_GIMME, 0);
	class_addmethod(dkzd_tilde_class, (t_method)dkzd_tilde_dsp, gensym("dsp"), A_CANT, 0);
	class_addmethod(dkzd_tilde_class, (t_method)dkzd_tilde_delay, gensym("delay"), A_FLOAT, 0);
	class_addmethod(dkzd_tilde_class, (t_method)dkzd_tilde_sz, gensym("bufsize"), A_FLOAT, 0);
	class_addmethod(dkzd_tilde_class, (t_method)dkzd_tilde_clear, gensym("clear"), 0);
   CLASS_MAINSIGNALIN(dkzd_tilde_class, t_dkzd_tilde, x_in);
}
