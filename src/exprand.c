#include <math.h>
#include "m_pd.h"

static t_class *exprand_class;

typedef struct _exprand {
	t_object x_obj;
	t_float x_f;
	unsigned int x_state;
} t_exprand;

static int exprseed(void){
	static unsigned int exprandseed = 1997333137;
	exprandseed = exprandseed * 2891336453 + 1500450271;
	exprandseed = exprandseed % 4294967296;
	return (exprandseed & 0x7fffffff);
}

static void *exprand_new(t_floatarg f){
	t_exprand *x = (t_exprand *)pd_new(exprand_class);
	x->x_f = f;
	x->x_state = exprseed();
	floatinlet_new(&x->x_obj, &x->x_f);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

static void exprand_bang(t_exprand *x){
	int n = x->x_f;
	float nval;
	int range = (n < 1 ? 1 : n);
	unsigned int randval = x->x_state;
	x->x_state = randval = ((randval * 2891336453 + 1500450271) % 4294967296) & 0x7fffffff;
	nval = (float)range*exp(-1.*range*randval * (1./2147483647.)); //divide by 2^31
	if(nval >= range){
		nval = range - 1.;
	};
	outlet_float(x->x_obj.ob_outlet, nval);
}

static void exprand_seed(t_exprand *x, t_float f, t_float glob){
	x->x_state = (int)f;
}

void exprand_setup(void){
	exprand_class = class_new(gensym("exprand"), (t_newmethod)exprand_new, 0,
			sizeof(t_exprand), 0, A_DEFFLOAT, 0);
	class_addbang(exprand_class, exprand_bang);
	class_addmethod(exprand_class, (t_method)exprand_seed, gensym("seed"), A_FLOAT, 0);
}
