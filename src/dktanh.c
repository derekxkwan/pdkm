/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

//from user cschueler at tanh under the post "rational tanh approximation"

#include <stdlib.h>
#include "m_pd.h"


#define DKTANHPG 1. //dfault pregain

static t_class *dktanh_tilde_class;

typedef struct _dktanh_tilde {
	t_object x_obj;
	t_float x_in; 
        t_float x_gain;
	t_outlet *x_outlet;
} t_dktanh_tilde;

static double dktanh_approx(double f){

    double ret;
    if(f < -3){
        ret = -1.;
    }
    else if (f > 3){
        ret = 1.;
    }
    else{
        ret = f * (27. + (f*f))/(27. + (9. * f * f)); 
    };
    return ret;

}

static void *dktanh_tilde_new(t_symbol *s, int argc, t_atom * argv){

        //could use t_floatarg f, but this is an easy way to check of an arg is passed or not
	t_dktanh_tilde *x = (t_dktanh_tilde *)pd_new(dktanh_tilde_class);
        t_float gain = DKTANHPG;
        if(argc){
            if(argv->a_type == A_FLOAT){
                gain = atom_getfloatarg(0,argc,argv);
            };
            argc--;
            argv++;
        };
        x->x_gain = gain;
	floatinlet_new(&x->x_obj, &x->x_gain);
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dktanh_tilde_perform(t_int *w){
	 t_dktanh_tilde *x = (t_dktanh_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	int i;
        for(i=0;i<n;i++){
            double output = (double)in[i]*(double)x->x_gain;
            out[i] = dktanh_approx(output);
        };
	return(w+5);
}

static void dktanh_tilde_dsp(t_dktanh_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(dktanh_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}




static void *dktanh_tilde_free(t_dktanh_tilde *x){
	outlet_free(x->x_outlet);
	
	return (void *)x;
}




void dktanh_tilde_setup(void){
	dktanh_tilde_class = class_new(gensym("dktanh~"), (t_newmethod)dktanh_tilde_new,
                (t_method)dktanh_tilde_free,
			sizeof(t_dktanh_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(dktanh_tilde_class, (t_method)dktanh_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dktanh_tilde_class, t_dktanh_tilde, x_in);
}
