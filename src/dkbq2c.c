/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <math.h>
#include "m_pd.h"

#define LN2D2 0.34657359027997264 //log(2)/2
static t_class *dkbq2c_class;

typedef enum _filttype{
	LPF;
	HPF;
	BPF1; //const skirt gain, peak gain = Q
	BPF2; //const 0 db peak gain
	NOTCH;
	APF;
	PEAK;
	LSHELF;
	HSHELF;
} filttype;

typedef enum _alphatype{
	QTYPE;
	BWTYPE;
	STYPE;
} alphatype;

typedef struct _dkbq2c {
	t_object x_obj;
	t_atom coeff[5]; //fb1 fb2 ff1 ff2 ff3
	double x_omega; //2*pi*f0/fs
	double x_A; //10^(dB_gain/40)
	double x_alpha;
	filttype x_ftype; 
	alphatype x_atype;
	int x_sr;
	unsigned int x_state;
	t_float x_minmax; //0 = tend towards 0, nonzero = tend towards high
} t_dkbq2c;


//pd's biquad~ is direct form 2 with ff2 and ff3 pos instead of neg
//y(n) = ff1 * w(n) + ff2 * w(n-1) + ff3 * w(n-2)
//w(n) = x(n) + fb1 * w(n-1) + fb2 * w(n-2)
//which means: y(n) = ff1*( x(n) + fb1 * w(n-1) + fb2 * w(n-2)) + ff2 *w(n-1) + ff3 * w(n-3)
// y(n) = ff1 * x(n) + (ff1*fb1+fb2)*w(n-1) + (ff1*fb2 + ff3) * w(n-3)

//taken from pd's biquad~.c

static void dkbq2c_alpha(t_dkbq2c * x, t_float _qbs){
	//qbs = q or bw or s depending on filter type
	double qbs = (double)_qbs;
	alphatype atype = x->x_atype;
	double omega = x->x_omega;
	double A = x->x_A;
	double sinw = sin(omega);
	
	switch(atype){
		case QTYPE:
			x->x_alpha=sinw/(2.*qbs);
			break;
		case BWTYPE:
			x->x_alpha = sinw*sinh(LN2D2 * qbs * (omega/sinw));
			break;
		case STYPE:
			x->x_alpha = (sinw/2.0) * sqrt( (A+(1./A)) * ((1./qbs) -1.) + 2.);
			break;
		default:
			break;
	
	};

};


static void dkbq2c_A(t_dkbq2c * x, t_float gain){
	x->x_A = pow(10., (double)gain/40.);
}

static void dkbq2c_omega(t_dkbq2c * x, t_float _freq){
	double freq = _freq < 0 ? 0 : (double)_freq;
	double sr = (double)sys_getsr();
	x->x_omega = (6.28318530718 * freq)/sr;
	
}

static void sigbiquad_list(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv)
{
    float fb1 = atom_getfloatarg(0, argc, argv);
    float fb2 = atom_getfloatarg(1, argc, argv);
    float ff1 = atom_getfloatarg(2, argc, argv);
    float ff2 = atom_getfloatarg(3, argc, argv);
    float ff3 = atom_getfloatarg(4, argc, argv);
    float discriminant = fb1 * fb1 + 4 * fb2;
    t_biquadctl *c = x->x_ctl;
    if (discriminant < 0) /* imaginary roots -- resonant filter */
    {
    	    /* they're conjugates so we just check that the product
    	    is less than one */
    	if (fb2 >= -1.0f) goto stable;
    }
    else    /* real roots */
    {
    	    /* check that the parabola 1 - fb1 x - fb2 x^2 has a
    	    	vertex between -1 and 1, and that it's nonnegative
    	    	at both ends, which implies both roots are in [1-,1]. */
    	if (fb1 <= 2.0f && fb1 >= -2.0f &&
    	    1.0f - fb1 -fb2 >= 0 && 1.0f + fb1 - fb2 >= 0)
    	    	goto stable;
    }
    	/* if unstable, just bash to zero */
   return 0;
stable:
	return 1;
}



static int linseed(void){
	static unsigned int linrandseed = 1997333137;
	linrandseed = linrandseed * 2891336453 + 1500450271;
	linrandseed = linrandseed % 4294967296;
	return (linrandseed & 0x7fffffff);
}

static void *dkbq2c_new(t_floatarg f, t_floatarg minmax){
	t_dkbq2c *x = (t_dkbq2c *)pd_new(dkbq2c_class);
	x->x_f = f;
	x->x_minmax = minmax;
	x->x_state = linseed();
	floatinlet_new(&x->x_obj, &x->x_f);
	floatinlet_new(&x->x_obj, &x->x_minmax);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

static void dkbq2c_bang(t_dkbq2c *x){
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

static void dkbq2c_seed(t_dkbq2c *x, t_float f, t_float glob){
	x->x_state = (int)f;
}

void dkbq2c_setup(void){
	dkbq2c_class = class_new(gensym("dkbq2c"), (t_newmethod)dkbq2c_new, 0,
			sizeof(t_dkbq2c), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addbang(dkbq2c_class, dkbq2c_bang);
	class_addmethod(dkbq2c_class, (t_method)dkbq2c_seed, gensym("seed"), A_FLOAT, 0);
}
