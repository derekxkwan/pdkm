/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include "m_pd.h"
#include <stdlib.h>

//taken from puredata's src/x_list.c

#ifndef HAVE_ALLOCA     /* can work without alloca() but we never need it */
#define HAVE_ALLOCA 1
#endif
#define TEXT_NGETBYTE 100 /* bigger that this we use alloc, not alloca */
#if HAVE_ALLOCA
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)((n) < TEXT_NGETBYTE ?  \
        alloca((n) * sizeof(t_atom)) : getbytes((n) * sizeof(t_atom))))
#define ATOMS_FREEA(x, n) ( \
    ((n) < TEXT_NGETBYTE || (freebytes((x), (n) * sizeof(t_atom)), 0)))
#else
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)getbytes((n) * sizeof(t_atom)))
#define ATOMS_FREEA(x, n) (freebytes((x), (n) * sizeof(t_atom)))
#endif

#define RANDMAX(a,b) \
	({__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b;})
#define RANDMIN(a,b) \
	({__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	 _a < _b ? _a : _b;})

#ifndef RNDLMIN
#define RNDLMIN 0.f
#endif

#ifndef RNDLMAX
#define RNDLMAX 1.f
#endif

#ifndef RNDLNUM
#define RNDLNUM 1
#endif

static t_class *dkrndl_class;

typedef struct _dkrndl {
	t_object x_obj;
	t_float minval;
	t_float maxval;
	t_float num;
        int     x_toint; //if we will round to int
	unsigned int x_state;
} t_dkrndl;

static unsigned int exprseed(void){
	static unsigned long int exprandseed = 1997333137;
	exprandseed = exprandseed * 2891336453 + 1500450271;
	exprandseed = exprandseed % 4294967296;
	exprandseed &= 0x7fffffff;
	return exprandseed;
	//retval = exprandseed * (1.f/2147483647.f); //biggest num in 32-bit div 2
	//return retval;
}

static void dkrndl_toint(t_dkrndl *x, t_float ti){
	int toint = (int)ti;
        if(toint <= 0){
            toint = 0;
        }
        else{
            toint = 1;
        };
        x->x_toint = toint;

}



static void *dkrndl_new(t_symbol *s, int argc, t_atom *argv){
	t_dkrndl *x = (t_dkrndl *)pd_new(dkrndl_class);
	
	x->minval = RNDLMIN;
	x->maxval = RNDLMAX;
	x->num = RNDLNUM;
        t_float toint = 0;
	x->x_state = exprseed();

	//argc = 1, set max, 2 = set max and num, 3 = set min, max, and num
	
	switch(argc){
		case 0: 
			break;
		case 1:
			x->maxval = atom_getfloatarg(0, argc, argv);
			break;
		case 2:
			x->maxval = atom_getfloatarg(0, argc, argv);
			x->num = atom_getfloatarg(1, argc, argv);
			break;
		case 3:
			x->minval = atom_getfloatarg(0, argc, argv);	
			x->maxval = atom_getfloatarg(1, argc, argv);
			x->num = atom_getfloatarg(2, argc, argv);
			break;
                default:
		case 4:
			x->minval = atom_getfloatarg(0, argc, argv);	
			x->maxval = atom_getfloatarg(1, argc, argv);
			x->num = atom_getfloatarg(2, argc, argv);
                        toint = atom_getfloatarg(3, argc, argv);
			break;

	};
        dkrndl_toint(x, toint);
	outlet_new(&x->x_obj, &s_list);
	floatinlet_new(&x->x_obj, &x->minval);
	floatinlet_new(&x->x_obj, &x->maxval);
	floatinlet_new(&x->x_obj, &x->num);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("toint"));
	return (x);
}


static void dkrndl_bang(t_dkrndl *x){

	float minv, maxv, rng;
	int num, i;
	t_atom *outatom;
	num = (int)x->num;
	minv = x->minval;
	maxv = x->maxval;
	
	if(num < 1){
		num = 1;
	};
		
	
	ATOMS_ALLOCA(outatom, num); //allocate memory for outatom 

	if(minv > maxv){//checking ranges
		float temp;
		temp = maxv;
		maxv = minv;
		minv = temp;
		};
	rng = maxv - minv;
	for(i=0; i<num; i++){//use helper function to set outatom one by one
		float currand;
		unsigned int randval = x->x_state;
		x->x_state = randval = ((randval * 2891336453 + 1500450271) % 4294967296) & 0x7fffffff;
		currand = randval * (1.f/2147483647.f); //biggest num in 32-bit div 2
		currand*= rng;
		currand+= minv;
                if(x->x_toint){
                    currand = (t_float)(int)currand;
                };

		SETFLOAT((outatom+i), (t_float)currand);
	};
	outlet_list(x->x_obj.ob_outlet, &s_list, num, outatom);
	ATOMS_FREEA(outatom, num); //free allocated memory for outatom
	
}




static void dkrndl_setrange(t_dkrndl *x, t_float lo, t_float hi){
	float minv, maxv;

	minv = lo;
	maxv = hi;
	if(minv > maxv){//checking ranges
		float temp;
		temp = maxv;
		maxv = minv;
		minv = temp;
		};

	x->minval = minv;
	x->maxval = maxv;

}

static void dkrndl_setnum(t_dkrndl *x, t_float argnum){
	int num;
	if(argnum < 1.f){
		num = 1;
	}
	else{
		num = (int)argnum;
	};
	x->num = num;
}

static void dkrndl_seed(t_dkrndl *x, t_float f, t_float glob){
	x->x_state = (int)f;
}

void dkrndl_setup(void){
	dkrndl_class = class_new(gensym("dkrndl"), (t_newmethod)dkrndl_new, 0,
			sizeof(t_dkrndl), 0, A_GIMME, 0);
	class_addbang(dkrndl_class, dkrndl_bang);
	class_addmethod(dkrndl_class, (t_method)dkrndl_setrange, gensym("range"), A_FLOAT, A_FLOAT, 0);
	class_addmethod(dkrndl_class, (t_method)dkrndl_setnum, gensym("num"), A_FLOAT, 0);
	class_addmethod(dkrndl_class, (t_method)dkrndl_seed, gensym("seed"), A_FLOAT, 0);
	class_addmethod(dkrndl_class, (t_method)dkrndl_seed, gensym("toint"), A_FLOAT, 0);
}
