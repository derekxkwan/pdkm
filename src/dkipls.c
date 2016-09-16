/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include <stdlib.h>
#include "m_pd.h"
#include "dkrnd.h"

static t_class *dkipls_tilde_class;

typedef struct _dkipls_tilde {
	t_object x_obj;
	t_float x_freq; //scalar freq
	double x_phase;
	double x_conv; //1/samprate, duration of one sample in seconds
        int x_newi; //if we are due a new impulse
        int x_rndamp; //if we want random amplitudes
        dkrnd * x_rnd;
	t_inlet *x_phaselet;
        t_inlet *x_randlet;
	t_outlet *x_outlet;
} t_dkipls_tilde;

static void *dkipls_tilde_new(t_floatarg freq, t_floatarg rand){
	int i,j, rndamp;
	t_dkipls_tilde *x = (t_dkipls_tilde *)pd_new(dkipls_tilde_class);
	x->x_freq = freq;
        if(rand <= 0){
            rndamp = 0;
        }
        else{
            rndamp = 1;    
        };
        x->x_rndamp = rndamp;
	x->x_phase = 0.f;
        x->x_newi = 1;
        x->x_rnd = dkrnd_new(0);
	x->x_randlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("rand"));
	x->x_phaselet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkipls_tilde_perform(t_int *w){
	 t_dkipls_tilde *x = (t_dkipls_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	
	while(n--){
		double freq = (double) abs(*in++);
		double output;
                double phs = x->x_phase;
                int newi = x->x_newi;
                if(newi){
                    if(x->x_rndamp){
                        output = dkrnd_next(x->x_rnd);
                    }
                    else{
                        output = 1.;
                    };
                    x->x_newi = 0;
                }
                else{
                    output = 0.;
                };
                phs += freq*x->x_conv;
                if(phs > 1.){
                    phs = 0.;
                    x->x_newi = 1;
                };

                x->x_phase = phs;
                *out++ = output;
	};
	return(w+5);
}

static void dkipls_tilde_dsp(t_dkipls_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
    x->x_conv = 1./(double)sp[0]->s_sr; //amount to change phase for freq 1
	    dsp_add(dkipls_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void dkipls_tilde_ft1(t_dkipls_tilde *x, t_float phase){
        if(phase < 0){
            phase = 0;
        };
	x->x_phase = (double)phase;

}

static void dkipls_tilde_seed(t_dkipls_tilde *x, t_float _seed){
    int seed = _seed;
    if(seed < 0){
        seed = 0;
    };
        dkrnd_seed(x->x_rnd, (unsigned int)seed);
}

    static void dkipls_tilde_rand(t_dkipls_tilde *x, t_float rand){
    int rndamp;
    if(rand <= 0){
            rndamp = 0;
        }
        else{
            rndamp = 1;    
        };
        x->x_rndamp = rndamp;

}

static void *dkipls_tilde_free(t_dkipls_tilde *x){
	inlet_free(x->x_phaselet);
	inlet_free(x->x_randlet);
	outlet_free(x->x_outlet);
        dkrnd_free(x->x_rnd);
	
	return (void *)x;
}

void dkipls_tilde_setup(void){
	dkipls_tilde_class = class_new(gensym("dkipls~"), (t_newmethod)dkipls_tilde_new,
                (t_method)dkipls_tilde_free,
			sizeof(t_dkipls_tilde), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(dkipls_tilde_class, (t_method)dkipls_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkipls_tilde_class, t_dkipls_tilde, x_freq);
   class_addmethod(dkipls_tilde_class, (t_method)dkipls_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
   class_addmethod(dkipls_tilde_class, (t_method)dkipls_tilde_rand, gensym("rand"), A_FLOAT, 0);
   class_addmethod(dkipls_tilde_class, (t_method)dkipls_tilde_seed, gensym("seed"), A_FLOAT, 0);
}
