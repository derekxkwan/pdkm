#include <math.h>
#include "m_pd.h"


#ifndef DXKWESBIT
#define DXKWESBIT 32 //bits of precision
#endif

#ifndef DXKWESSH
#define DXKWESSH 1 //sample to hold for
#endif

static t_class *wesley_tilde_class;

typedef struct _wesley_tilde {
	t_object x_obj;
	int x_phase;
	double prev; //holding a value as in samphold~
	int twopow[DXKWESBIT]; //powers of 2 lookup table
	t_float downsamp; //downsampling multiplier
	t_float bitres;//bitresolution
	t_float x_in; //dummy
} t_wesley_tilde;

void *wesley_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_wesley_tilde *x = (t_wesley_tilde *)pd_new(wesley_tilde_class);
	x->x_phase = 0;
	x->prev = 0.f;
	x->downsamp = DXKWESSH;
	x->bitres = DXKWESBIT;

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
	for(i=0; i<DXKWESBIT; i++){
		x->twopow[i] = tpval;
		tpval *= 2;
	};

	floatinlet_new(&x->x_obj, &x->bitres);
	floatinlet_new(&x->x_obj, &x->downsamp);
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

t_int *wesley_tilde_perform(t_int *w){
	 t_wesley_tilde *x = (t_wesley_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	int downsamp = (int)x->downsamp;
	int bitres = (int)x->bitres-1;
	if(bitres< 0){
		bitres = 0;
	}
	else if(bitres > DXKWESBIT - 1){
		bitres = DXKWESBIT - 1;
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
		if(bitres < DXKWESBIT){
			double bitamt = (double)x->twopow[bitres];
			output =  round(output*bitamt)/bitamt;
		};
		*out++ = output;

		};
	return(w+5);
}

void wesley_tilde_dsp(t_wesley_tilde *x, t_signal **sp){

	    dsp_add(wesley_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}



void wesley_tilde_setup(void){
	wesley_tilde_class = class_new(gensym("wesley~"), (t_newmethod)wesley_tilde_new, 0,
			sizeof(t_wesley_tilde), 0, A_GIMME, 0);
	class_addmethod(wesley_tilde_class, (t_method)wesley_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(wesley_tilde_class, t_wesley_tilde, x_in);
}
