#include <math.h>
#include "m_pd.h"

static t_class *linrand_class;

typedef struct _linrand {
	t_object x_obj;
	t_float x_f;
	unsigned int x_state;
	t_float x_minmax; //0 = tend towards 0, nonzero = tend towards high
} t_linrand;

static int linseed(void){
	static unsigned int linrandseed = 1997333137;
	linrandseed = linrandseed * 2891336453 + 1500450271;
	linrandseed = linrandseed % 4294967296;
	return (linrandseed & 0x7fffffff);
}

static void *linrand_new(t_floatarg f, t_floatarg minmax){
	t_linrand *x = (t_linrand *)pd_new(linrand_class);
	x->x_f = f;
	x->x_minmax = minmax;
	x->x_state = linseed();
	floatinlet_new(&x->x_obj, &x->x_f);
	floatinlet_new(&x->x_obj, &x->x_minmax);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

static void linrand_bang(t_linrand *x){
	int n = x->x_f;
	float lrmm = x->x_minmax;
	float nval;
	int range = (n < 1 ? 1 : n);
	int lohi;
	if(lrmm <= 0.){
		lohi = 0;
		}
	else{
		lohi = 1; // > 0
	};
	unsigned int randval = x->x_state;
	x->x_state = randval = ((randval * 2891336453 + 1500450271) % 4294967296) & 0x7fffffff;
	if(lohi == 0){ //tend towards 0
		nval = (float)range*(1.-pow(1.-(randval * (1./2147483647.)),0.5)); //divide by 2^31
		}
	else{ //tend towards 1
		nval = (float)range*(pow(randval * (1./2147483647.), 0.5));
		};
	if(nval >= range){
		nval = range - 1.;
		};
	outlet_float(x->x_obj.ob_outlet, nval);
}

static void linrand_seed(t_linrand *x, t_float f, t_float glob){
	x->x_state = (int)f;
}

void linrand_setup(void){
	linrand_class = class_new(gensym("linrand"), (t_newmethod)linrand_new, 0,
			sizeof(t_linrand), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addbang(linrand_class, linrand_bang);
	class_addmethod(linrand_class, (t_method)linrand_seed, gensym("seed"), A_FLOAT, 0);
}
