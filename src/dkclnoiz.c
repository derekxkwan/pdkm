/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include <stdlib.h>
#include "m_pd.h"
#include "dkrnd.h"


static t_class *dkclnoiz_tilde_class;

typedef struct _dkclnoiz_tilde {
	t_object x_obj;
	t_float x_in; //dummy
        t_dkrnd * x_rnd;
	t_outlet *x_outlet;
} t_dkclnoiz_tilde;


static void *dkclnoiz_tilde_new(void){

	t_dkclnoiz_tilde *x = (t_dkclnoiz_tilde *)pd_new(dkclnoiz_tilde_class);
        x->x_rnd = dkrnd_new(0);


	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkclnoiz_tilde_perform(t_int *w){
	t_dkclnoiz_tilde *x = (t_dkclnoiz_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
	int i;
        for(i=0;i<n;i++){
            double output;
            double r = dkrnd_next(x->x_rnd);
            if(r < 0.5){
                output = -1.;
            }
            else{
                output = 1.;
            };
            out[i] = output;
        };
	return(w+4);
}

static void dkclnoiz_tilde_dsp(t_dkclnoiz_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	dsp_add(dkclnoiz_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);

}


static void dkclnoiz_tilde_seed(t_dkclnoiz_tilde *x, t_float _seed){
    int seed = _seed;
    if(seed < 0){
        seed = 0;
    };
        dkrnd_seed(x->x_rnd, (unsigned int)seed);
}



static void *dkclnoiz_tilde_free(t_dkclnoiz_tilde *x){
	outlet_free(x->x_outlet);
        dkrnd_free(x->x_rnd);
	
	return (void *)x;
}




void dkclnoiz_tilde_setup(void){
	dkclnoiz_tilde_class = class_new(gensym("dkclnoiz~"), (t_newmethod)dkclnoiz_tilde_new,
                (t_method)dkclnoiz_tilde_free,
			sizeof(t_dkclnoiz_tilde), CLASS_DEFAULT, 0);
	class_addmethod(dkclnoiz_tilde_class, (t_method)dkclnoiz_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkclnoiz_tilde_class, t_dkclnoiz_tilde, x_in);
   class_addmethod(dkclnoiz_tilde_class, (t_method)dkclnoiz_tilde_seed, gensym("seed"), A_FLOAT, 0);
}
