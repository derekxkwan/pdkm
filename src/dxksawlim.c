/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "dxkwavtab.h"

static t_class *dxksawlim_tilde_class;

typedef struct _dxksawlim_tilde {
	t_object x_obj;
	t_float x_freq; //scalar freq
	double x_phase;
	double x_conv; //1/samprate, duration of one sample in seconds
	double x_saw[DXKTABLEN]; //table for holding saw values

	t_inlet *x_phaselet;
	t_outlet *x_outlet;
} t_dxksawlim_tilde;

static void *dxksawlim_tilde_new(t_floatarg freq){
	int i,j;
	t_dxksawlim_tilde *x = (t_dxksawlim_tilde *)pd_new(dxksawlim_tilde_class);
	x->x_freq = freq;
	x->x_phase = 0.f;

	for(i=0; i<DXKTABLEN; i++){
		double xval = (double)i/(double)DXKTABLEN;
		double yval = 0.f;
		for(j=1; j<=DXKTABDEP; j++){
			double cterm = 1.f/(double)j;
			double varterm = sin(xval * (double)j*DXK2PI);
			double valterm = cterm*varterm;
			yval += valterm;
		};
		x->x_saw[i] = yval;
	};

	x->x_phaselet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dxksawlim_tilde_perform(t_int *w){
	 t_dxksawlim_tilde *x = (t_dxksawlim_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	
	while(n--){
		double freq = (double) *in++;
		x->x_phase = fmod(x->x_phase, (double)(DXKTABLEN));//phases need to be wrapped  just in case
		while(x->x_phase < 0){
			x->x_phase += DXKTABLEN;
		};
		double dtabphase = x->x_phase;//find corresponding index for table 
		int tabphase1 = (int)dtabphase;
		int tabphase2 = tabphase1 + 1;
		double frac = (double)dtabphase - tabphase1;
		if(tabphase1 >= (DXKTABLEN - 1)){
			tabphase1 = DXKTABLEN - 1; //checking to see if index falls within bounds
			tabphase2 = 0;
		}
		else if(tabphase1 < 0){
			tabphase1 = 0;
			tabphase2 = 1;
		};
		double yb = x->x_saw[tabphase2]; //linear interp
		double ya = x->x_saw[tabphase1];
		double output = ya+((yb-ya)*frac);
		*out++ = output;
		x->x_phase += freq* x->x_conv;
	};
	return(w+5);
}

static void dxksawlim_tilde_dsp(t_dxksawlim_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
    x->x_conv = (double)DXKTABLEN/(double)sp[0]->s_sr; //amount to change phase for freq 1
	    dsp_add(dxksawlim_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void dxksawlim_tilde_ft1(t_dxksawlim_tilde *x, t_float phase){
	x->x_phase = (double)DXKTABLEN*phase;

}


static void *dxksawlim_tilde_free(t_dxksawlim_tilde *x){
	inlet_free(x->x_phaselet);
	outlet_free(x->x_outlet);
	
	return (void *)x;
}


void dxksawlim_tilde_setup(void){
	dxksawlim_tilde_class = class_new(gensym("dxksawlim~"), (t_newmethod)dxksawlim_tilde_new, 0,
			sizeof(t_dxksawlim_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
	class_addmethod(dxksawlim_tilde_class, (t_method)dxksawlim_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dxksawlim_tilde_class, t_dxksawlim_tilde, x_freq);
   class_addmethod(dxksawlim_tilde_class, (t_method)dxksawlim_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
}
