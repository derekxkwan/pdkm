#include <math.h>
#include "m_pd.h"

static t_class *red_tilde_class;

typedef struct _red_tilde {
	t_object x_obj;
	int x_state;
	t_float x_red;
} t_red_tilde;

static void *red_tilde_new(t_floatarg red){
	t_red_tilde *x = (t_red_tilde *)pd_new(red_tilde_class);
	static int red_init = 1997333137;
	x->x_state = red_init;
	x->x_red = red;
	floatinlet_new(&x->x_obj, &x->x_red);
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *red_tilde_perform(t_int *w){
	t_red_tilde *x = (t_red_tilde *)(w[2]);
	int nblock = (int)(w[3]);
	t_sample *out = (t_sample*)(w[1]);
	int state = x->x_state;
	float red;
	if(x->x_red < 0.){
		red = 0.;
	}
	else if( x->x_red > 1.){
		red = 1.;
	}
	else{
		red = x->x_red;
	}
	float prev = 0.;
	while(nblock--){
		float white, curr;
		state = state * 2891336453 + 1500450271;
		state = state % 4294967296;
		state = state & 0x7fffffff;
		white = (float)state* (2.5/4294967296.);
		white = white - 0.625;
		curr = red*prev + pow(1-pow(red, 2.), 0.5)*white;
		prev = curr;
		*out++ = curr;
	};
	x->x_state = state;
	return(w+4);
}

static void red_tilde_dsp(t_red_tilde *x, t_signal **sp){
	dsp_add(red_tilde_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

void red_tilde_setup(void){
	red_tilde_class = class_new(gensym("red~"), (t_newmethod)red_tilde_new, 0,
			sizeof(t_red_tilde), 0, A_DEFFLOAT, 0);
	class_addmethod(red_tilde_class, (t_method)red_tilde_dsp, gensym("dsp"), A_CANT, 0);
}
