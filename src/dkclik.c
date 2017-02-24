/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "m_pd.h"

#define DKCLIKLEN 1 //length of impulse

static t_class *dkclik_tilde_class;

typedef struct _dkclik_tilde {
	t_object x_obj;
	t_inlet *x_lenlet;
        t_outlet *x_outlet;
        int x_len; //length of impulse
        int x_cur; //current length of impulse
        t_float x_in; //dummy
} t_dkclik_tilde;

static void dkclik_tilde_len(t_dkclik_tilde *x, t_float f){

    int len = (int)f;
    if(f < 1){
        f = 1;
    };

    x->x_len = len;
}

static void dkclik_tilde_clear(t_dkclik_tilde *x){
    x->x_cur = 0;
}

static void *dkclik_tilde_new(t_symbol *s, int argc, t_atom * argv){
	t_dkclik_tilde *x = (t_dkclik_tilde *)pd_new(dkclik_tilde_class);
        t_float len = DKCLIKLEN; //default len
        while(argc){
            if(argv->a_type == A_FLOAT){
                len = atom_getfloatarg(0, argc, argv);
            };
            argc--;
            argv++;
        };
        dkclik_tilde_len(x, len);
	dkclik_tilde_clear(x);

        x->x_lenlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("len"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkclik_tilde_perform(t_int *w){
	
	t_dkclik_tilde *x = (t_dkclik_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
        int cur = x->x_cur;
	while(n--){
            t_float output;
            if(cur){
                output = 1;
                cur--;
            }
            else{
                output = 0;
            };
                *out++ = output;
	};
        x->x_cur = cur;
	return(w+4);
}

static void dkclik_tilde_dsp(t_dkclik_tilde *x, t_signal **sp){

	dsp_add(dkclik_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);

}

static void dkclik_tilde_bang(t_dkclik_tilde *x){
    x->x_cur = x->x_len;
}

static void *dkclik_tilde_free(t_dkclik_tilde *x){
	outlet_free(x->x_outlet);
	inlet_free(x->x_lenlet);
	
	return (void *)x;
}

void dkclik_tilde_setup(void){
	dkclik_tilde_class = class_new(gensym("dkclik~"), (t_newmethod)dkclik_tilde_new,
                (t_method)dkclik_tilde_free,
			sizeof(t_dkclik_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(dkclik_tilde_class, (t_method)dkclik_tilde_dsp, gensym("dsp"), A_CANT, 0);
	class_addmethod(dkclik_tilde_class, (t_method)dkclik_tilde_len, gensym("len"), A_FLOAT, 0);
	class_addmethod(dkclik_tilde_class, (t_method)dkclik_tilde_clear, gensym("clear"), 0);
   CLASS_MAINSIGNALIN(dkclik_tilde_class, t_dkclik_tilde, x_in);
    class_addbang(dkclik_tilde_class, dkclik_tilde_bang);
}
