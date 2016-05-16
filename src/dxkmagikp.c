/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include "m_pd.h"
#include <math.h>

#ifndef DXKARPMAX
#define DXKARPMAX 9601
#endif

#ifndef DXKARPCOEF
#define DXKARPCOEF 0.995f
#endif

static t_class *dxkmagikp_tilde_class;

typedef struct _dxkmagikp_tilde {
	t_object x_obj;
	t_float x_freq; //scalar freq
	int x_sr; //samplerate
	double noisebuf[DXKARPMAX];
	int x_buflen;
	float x_coef;
	int x_newatt;
	int x_phase; 
	t_inlet *x_freqlet;
	t_inlet *x_coeflet;
	t_outlet *x_outlet;
} t_dxkmagikp_tilde;

static double exprseed(void){//range -1 to 1
	double retval;
	static unsigned long int exprandseed = 1997333137;
	exprandseed = exprandseed * 2891336453 + 1500450271;
	exprandseed = exprandseed % 4294967296;
	exprandseed &= 0x7fffffff;
	retval = exprandseed * (2.f/2147483647.f)-1.f; //biggest num in 32-bit div 2
	return retval;
}
static void dxkmagikp_tilde_setfreq(t_dxkmagikp_tilde *x, t_float freq){
	if(freq < 20.f){
		freq = 20.f;
	};
	x->x_freq = freq;
	int buflen = (int)roundf((float)x->x_sr/(float)freq)+1;
	if(buflen > DXKARPMAX){
		buflen = DXKARPMAX;
	};
	x->x_buflen = buflen;

}

static void dxkmagikp_tilde_setcoef(t_dxkmagikp_tilde *x, t_float coef){
	if(coef < 0.f){
		coef = 0.f;
	}
	else if(coef >= 1.f){
		coef = 0.9999999999999999999999999999999999f;
	};
	x->x_coef = coef;
}

static void *dxkmagikp_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_dxkmagikp_tilde *x = (t_dxkmagikp_tilde *)pd_new(dxkmagikp_tilde_class);
	for(i=0;i<DXKARPMAX; i++){
		x->noisebuf[i] = 0.f;
	};
	x->x_sr = sys_getsr();
	x->x_coef = DXKARPCOEF;
	x->x_newatt = 0;
	x->x_phase = 0;
	switch(argc){
		float freq,coef;
		case 2:
			coef = atom_getfloatarg(1, argc,argv);
			dxkmagikp_tilde_setcoef(x, coef);
		case 1:
			freq = atom_getfloatarg(0, argc,argv);
			dxkmagikp_tilde_setfreq(x, freq);
			break;
		default:
			goto errstate;
	};
	x->x_freqlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("freq"));
	x->x_coeflet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("coef"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
	errstate:
		pd_error(x, "magikp~: improper args");
		return NULL;
}

static void dxkmagikp_tilde_float(t_dxkmagikp_tilde *x, t_float freq){
	int prevfreq = x->x_freq;
	if(freq != prevfreq){
		dxkmagikp_tilde_setfreq(x,freq);
	};
	x->x_newatt = 1;
}

static void dxkmagikp_tilde_bang(t_dxkmagikp_tilde *x){
	x->x_newatt = 1;
}


static t_int *dxkmagikp_tilde_perform(t_int *w){
		 t_dxkmagikp_tilde *x = (t_dxkmagikp_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
	int newatt = x->x_newatt;
	int buflen = x->x_buflen;
	float coef = x->x_coef;
	int i;
	while(n--){
		if(newatt){
			x->x_newatt = 0;
			for(i=0;i<=buflen;i++){
				x->noisebuf[i] = exprseed();
			};
			x->x_phase = 0;
		};
		int phs = x->x_phase;
		int nextphs = (phs + 1) % (buflen);
		double avg = (x->noisebuf[phs] + x->noisebuf[nextphs]*coef)*0.5;
		*out++ = avg;
		x->noisebuf[phs] = avg;
		x->x_phase += 1;
		x->x_phase = x->x_phase % buflen;
	};
	return(w+4);
}

static void dxkmagikp_tilde_dsp(t_dxkmagikp_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
    int sr = sp[0]->s_sr; //amount to change phase for freq 1
	if(sr != x->x_sr){
		x->x_sr = sr;
		float freq = x->x_freq;
		dxkmagikp_tilde_setfreq(x,freq);
	};
	    dsp_add(dxkmagikp_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);

}

static void *dxkmagikp_tilde_free(t_dxkmagikp_tilde *x){
	inlet_free(x->x_freqlet);
	inlet_free(x->x_coeflet);
	outlet_free(x->x_outlet);
	
	return (void *)x;
}

void dxkmagikp_tilde_setup(void){
	dxkmagikp_tilde_class = class_new(gensym("dxkmagikp~"), (t_newmethod)dxkmagikp_tilde_new, 0,
			sizeof(t_dxkmagikp_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addfloat(dxkmagikp_tilde_class, (t_method)dxkmagikp_tilde_float);
	class_addbang(dxkmagikp_tilde_class, (t_method)dxkmagikp_tilde_bang);
	class_addmethod(dxkmagikp_tilde_class, (t_method)dxkmagikp_tilde_dsp, gensym("dsp"), A_CANT, 0);
   class_addmethod(dxkmagikp_tilde_class, (t_method)dxkmagikp_tilde_setfreq, gensym("freq"), A_FLOAT, 0);
   class_addmethod(dxkmagikp_tilde_class, (t_method)dxkmagikp_tilde_setcoef, gensym("coef"), A_FLOAT, 0);
}
