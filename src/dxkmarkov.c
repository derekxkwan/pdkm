/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <math.h>
#include "m_pd.h"
#include <stdint.h>
#include "dxkrand.h"


#define DXKMRK2DIM 12 //dimension of second-markov table
static t_class *dxkmrk2_class;

typedef struct _dxkmrk2 {
	t_object x_obj;
        int x_prev; //previous note
        int x_prev2; //previous note to the previous note
        uint64_t x_table[DXKMRK2DIM][DXKMRK2DIM][DXKMRK2DIM];
} t_dxkmrk2;


static int midiCheck(int midinote){
    //bounds checking for midinotes
    if(midinote < 0){
        return 0;
    }
    else if(midinote > 127){
        return 127;
    }
    else{
        return midinote;
    };
}

static int getNextNote(t_dxkmrk2 *x){
        int prev = x->x_prev % DXKMRK2DIM;
        int prev2 = x->x_prev2 % DXKMRK2DIM;
        double curnotes[DXKMRK2DIM];
        int i=0;
        uint64_t total = 0;
        for(i=0; i<DXKMRK2DIM; i++){
            //calculate total events
            total += x->x_table[prev2][prev][i];
        };
        double currand = dxkrand(); //current random number

        double runsum = x->x_table[prev2][prev][0]*(1.0/total); //running sum, should add up to 1.0 at end
        //idea is running sum should eventually equal 1.0
        //scan through the notes adding their proportions to runsum, if runsum > currand, we found our note
        int note = 0; //note to return
        while(note<DXKMRK2DIM && runsum < currand){
            note++;
            runsum += x->x_table[prev2][prev][note]*(1.0/total);
        };
        return note;
}

static int exprseed(void){
	static unsigned int dxkmrk2seed = 1997333137;
	dxkmrk2seed = exprandseed * 2891336453 + 1500450271;
	dxkmrk2seed = exprandseed % 4294967296;
	return (dxkmrk2seed & 0x7fffffff);
}

static void *dxkmrk2_new(t_floatarg f){
	t_dxkmrk2 *x = (t_dxkmrk2 *)pd_new(exprand_class);
	x->x_f = f;
	x->x_state = exprseed();
	floatinlet_new(&x->x_obj, &x->x_f);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

static void dxkmrk2_bang(t_dxkmrk2 *x){
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

static void dxkmrk2_seed(t_dxkmrk2 *x, t_float f, t_float glob){
	x->x_state = (int)f;
}

void dxkmrk2_setup(void){
	dxkmrk2_class = class_new(gensym("exprand"), (t_newmethod)exprand_new, 0,
			sizeof(t_dxkmrk2), 0, A_DEFFLOAT, 0);
	class_addbang(dxkmrk2_class, exprand_bang);
	class_addmethod(dxkmrk2_class, (t_method)exprand_seed, gensym("seed"), A_FLOAT, 0);
}
