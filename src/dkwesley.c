/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include <math.h>
#include "m_pd.h"


#ifndef WESBIT
#define WESBIT 32 //bits of precision
#endif

#ifndef WESSH
#define WESSH 1 //sample to hold for
#endif

static t_class *dkwesley_tilde_class;

typedef struct _dkwesley_tilde {
	t_object x_obj;
	int x_phase;
	double prev; //holding a value as in samphold~
	int twopow[WESBIT]; //powers of 2 lookup table
	t_float downsamp; //downsampling multiplier
	t_float bitres;//bitresolution
	t_float x_in; //dummy
} t_dkwesley_tilde;

void *dkwesley_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_dkwesley_tilde *x = (t_dkwesley_tilde *)pd_new(dkwesley_tilde_class);
	x->x_phase = 0;
	x->prev = 0.f;
	x->downsamp = WESSH;
	x->bitres = WESBIT;

	switch(argc){
		case 0: break;
		case 1: x->bitres = atom_getfloatarg(0, argc, argv);
				break;
		default:
		case 2: x->bitres = atom_getfloatarg(0, argc, argv);
				x->downsamp = atom_getfloatarg(1, argc,argv);
				break;

	};


	int tpval = 1;
	for(i=0; i<WESBIT; i++){
		x->twopow[i] = tpval;
		tpval *= 2;
	};

	floatinlet_new(&x->x_obj, &x->bitres);
	floatinlet_new(&x->x_obj, &x->downsamp);
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

t_int *dkwesley_tilde_perform(t_int *w){
	 t_dkwesley_tilde *x = (t_dkwesley_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	int downsamp = (int)x->downsamp;
	int bitres = (int)x->bitres-1;
	if(bitres< 0){
		bitres = 0;
	}
	else if(bitres > WESBIT - 1){
		bitres = WESBIT - 1;
	};
	while(n--){
		double input = *in++;
		double output;
		x->x_phase++;
		if(x->x_phase >= downsamp){
			x->x_phase = 0;
			output = input;
			x->prev = input; 
			}
		else{
			output = x->prev;
		};
		if(bitres < WESBIT){
			double bitamt = (double)x->twopow[bitres];
			output =  round(output*bitamt)/bitamt;
		};
		*out++ = output;

		};
	return(w+5);
}

void dkwesley_tilde_dsp(t_dkwesley_tilde *x, t_signal **sp){

	    dsp_add(dkwesley_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}



void dkwesley_tilde_setup(void){
	dkwesley_tilde_class = class_new(gensym("dkwesley~"), (t_newmethod)dkwesley_tilde_new, 0,
			sizeof(t_dkwesley_tilde), 0, A_GIMME, 0);
	class_addmethod(dkwesley_tilde_class, (t_method)dkwesley_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkwesley_tilde_class, t_dkwesley_tilde, x_in);
}
