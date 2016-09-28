/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "m_pd.h"

static t_class *dkclik_tilde_class;

typedef struct _dkclik_tilde {
	t_object x_obj;
        int x_newi; //if we are due a new impulse
	t_outlet *x_outlet;
        t_float x_in; //dummy
} t_dkclik_tilde;

static void *dkclik_tilde_new(t_floatarg freq, t_floatarg rand){
	t_dkclik_tilde *x = (t_dkclik_tilde *)pd_new(dkclik_tilde_class);
        x->x_newi = 0;
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkclik_tilde_perform(t_int *w){
	
	t_dkclik_tilde *x = (t_dkclik_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
        int newi = x->x_newi;
	while(n--){
            t_float output;
            if(newi){
                output = 1;
                x->x_newi = 0;
            }
            else{
                output = 0;
            };
                *out++ = output;
	};
	return(w+4);
}

static void dkclik_tilde_dsp(t_dkclik_tilde *x, t_signal **sp){

	dsp_add(dkclik_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);

}

static void dkclik_tilde_bang(t_dkclik_tilde *x){
    x->x_newi = 1;
}

static void *dkclik_tilde_free(t_dkclik_tilde *x){
	outlet_free(x->x_outlet);
	
	return (void *)x;
}

void dkclik_tilde_setup(void){
	dkclik_tilde_class = class_new(gensym("dkclik~"), (t_newmethod)dkclik_tilde_new,
                (t_method)dkclik_tilde_free,
			sizeof(t_dkclik_tilde), CLASS_DEFAULT, 0);
	class_addmethod(dkclik_tilde_class, (t_method)dkclik_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkclik_tilde_class, t_dkclik_tilde, x_in);
    class_addbang(dkclik_tilde_class, dkclik_tilde_bang);
}
