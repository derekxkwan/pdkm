/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <math.h>
#include "m_pd.h"
#include <float.h>

static t_class *dkctr_class;

typedef struct _dkctr {
	t_object x_obj;
        double x_lo; //lo part of range
        double x_hi; //hi part of range
        int x_mode; //0 = normal direction, 1 = reversed, 2 = norm->rev, 3=rev->norm
        int x_stage; //0 = stage 1 of mode 2/3, 1 = stage 2 or mode 2/3
        int x_bangnext; //enables bang at start of every sequence
        t_float x_ssize; //stepsize
        double x_state; //current count
        t_inlet *x_countlet;
        t_inlet *x_lolet;
        t_inlet *x_hilet;
        t_inlet *x_steplet;
        t_inlet *x_modelet;
        t_inlet *x_resetlet;
        t_outlet *x_floatlet; //count outlet
        t_outlet *x_donelet; //bang when boundary hit
} t_dkctr;


static void dkctr_step(t_dkctr *x, t_float step){
    x->x_ssize = step;
}

static void dkctr_mode(t_dkctr *x, t_float m){
    int mode = (int)m;
    if(mode < 0){
        mode = 0;
    }
    else if(mode > 3){
        mode = 3;
    };
    x->x_mode = mode;
    x->x_stage = 0;
    x->x_bangnext = 1;
}

static void dkctr_reset(t_dkctr *x){
    //reset state according to mode
    int mode = x->x_mode;
    int neg = x->x_ssize < 0; //if step size is negative
    if((mode == 0 || mode == 2) && (!neg)){
        x->x_state = x->x_lo;
    }
    else{
        x->x_state = x->x_hi;
    };
    x->x_stage = 0;
    x->x_bangnext = 1;
}


static void dkctr_lo(t_dkctr *x, t_float lo){
    x->x_lo = lo;
}

static void dkctr_hi(t_dkctr *x, t_float hi){
    x->x_hi = hi;
}

static void dkctr_count(t_dkctr *x, t_float f){
    //set current count
    double count = (double)f;
    double lo = x->x_lo;
    double hi = x->x_hi;
    //bounds checking
    if(count < lo){
        count = lo;
    }
    else if(count > hi){
        count = hi;
    };
    x->x_state = count;
}

static void dkctr_bang(t_dkctr *x){
    double lo = x->x_lo;
    double hi = x->x_hi;
    double ssize = (double)x->x_ssize;
    double state = x->x_state;
    double nextstate;
    double dir = 1.;
    int mode = x->x_mode;
    int stage = x->x_stage;
    int neg = ssize < 0. ;//if step size is negative
    int dobang = 0; //if we are finished with the direction, do a bang
    
    //make sure lo is lower than high
    if(lo > hi){
        double temp = lo;
        lo = hi;
        hi = temp;
    };

    if(mode == 0 || mode == 1){
        x->x_stage = stage = 0;
        //make sure stage is 0 for 0 and 1 because there is no second stage
    };


    if(mode == 1 || mode == 3){
        dir = -1.;
        //go in reverse direction for modes 1 and 3
    };

    if(stage){
        dir *= -1.;

        //reverse direction we are going in if in the second stage of mode 2/3
        //the rev part of norm -> rev for mode 2, the norm part of rev -> norm for mode 3
    };

    //now do the incrementing for the next state
    nextstate = state + (ssize  * dir);

    //now check bounds for next state
    
    //if there is a bang left over from last time (modes 2/3)
    if(x->x_bangnext){
        x->x_bangnext = 0;
	outlet_bang(x->x_donelet);

    };

    switch(mode){
        case 0:
            //normal ascending
            if(!neg){
                if(nextstate >= hi){
                     nextstate = lo;
                    x->x_bangnext = 1;
                 };
            }
            else{
                if(nextstate <= lo){
                    nextstate = hi;
                    x->x_bangnext = 1;
                };
            };
            break;
        case 1:
            //descending (opposite of ascending)
            if(neg){
                if(nextstate >= hi){
                     nextstate = lo;
                    x->x_bangnext = 1;
                 };
            }
            else{
                if(nextstate <= lo){
                    nextstate = hi;
                    x->x_bangnext = 1;
                };
            };
            break;
        case 2:
            //ascending then descending
            if(nextstate >= hi){
                nextstate = hi;
                x->x_stage = 1 ^ neg;
                x->x_bangnext = 1;
            }
            else if(nextstate <= lo){
                nextstate = lo;
                x->x_stage = 0 ^ neg;
                x->x_bangnext = 1;
            };
            break;
        case 3:
            //descending then ascending
            if(nextstate >= hi){
                nextstate = hi;
                x->x_stage = 0 ^ neg;
                x->x_bangnext = 1;
            }
            else if(nextstate <= lo){
                nextstate = lo;
                x->x_stage = 1 ^ neg;
                x->x_bangnext = 1;
            };
            break;

        default:
            break;
    };
    x->x_state = nextstate;


    outlet_float(x->x_floatlet, (t_float)state);

}

static void *dkctr_new(t_symbol *s, int argc, t_atom * argv){

        //0 arg = all defaults
        //1 arg = stepsize
        //2 arg = hi, stepsize
        //3 arg = lo, hi, stepsize
        //4 arg = lo, hi,stepsize, mode
        
	t_dkctr *x = (t_dkctr *)pd_new(dkctr_class);
	
        //defaults
        double lo = 0.;
        double hi = DBL_MAX;
        t_float ssize = 1.;
        t_float mode = 0;
        
        if(argc){
            switch(argc){
             case 1:
                    hi = atom_getfloatarg(0, argc, argv);
                    break;
            case 2:
                    lo = (double)atom_getfloatarg(0, argc, argv);
                    hi = atom_getfloatarg(1, argc, argv);
                    break;
            case 3:
                    lo = (double)atom_getfloatarg(0, argc, argv);
                    hi = (double)atom_getfloatarg(1, argc, argv);
                    ssize = atom_getfloatarg(2, argc, argv);
                    break;
            default:
            case 4:
                    lo = (double)atom_getfloatarg(0, argc, argv);
                    hi = (double)atom_getfloatarg(1, argc, argv);
                    ssize = atom_getfloatarg(2, argc, argv);
                    mode = atom_getfloatarg(3, argc, argv);
                    break;
            };
        };

        x->x_lo = lo;
        x->x_hi = hi;
        dkctr_step(x,ssize);
        dkctr_mode(x, mode);

        dkctr_reset(x);
	x->x_countlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("count"));
	x->x_lolet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("lo"));
	x->x_hilet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("hi"));
	x->x_steplet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("step"));
	x->x_modelet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("mode"));
	x->x_resetlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_bang, gensym("reset"));
	x->x_floatlet = outlet_new(&x->x_obj, &s_float);
        x->x_donelet = outlet_new(&x->x_obj, &s_bang);
	return (x);
}

static void *dkctr_free(t_dkctr *x){
    inlet_free(x->x_countlet);
    inlet_free(x->x_lolet);
    inlet_free(x->x_hilet);
    inlet_free(x->x_steplet);
    inlet_free(x->x_modelet);
    inlet_free(x->x_resetlet);
    outlet_free(x->x_floatlet);
    outlet_free(x->x_donelet);
    return (void *)x;
}

void dkctr_setup(void){
	dkctr_class = class_new(gensym("dkctr"), (t_newmethod)dkctr_new, (t_method)dkctr_free,
			sizeof(t_dkctr), 0, A_GIMME, 0);
	class_addbang(dkctr_class, dkctr_bang);
	class_addmethod(dkctr_class, (t_method)dkctr_count, gensym("count"), A_FLOAT, 0);
	class_addmethod(dkctr_class, (t_method)dkctr_lo, gensym("lo"), A_FLOAT, 0);
	class_addmethod(dkctr_class, (t_method)dkctr_hi, gensym("hi"), A_FLOAT, 0);
	class_addmethod(dkctr_class, (t_method)dkctr_step, gensym("step"), A_FLOAT, 0);
	class_addmethod(dkctr_class, (t_method)dkctr_mode, gensym("mode"), A_FLOAT, 0);
	class_addmethod(dkctr_class, (t_method)dkctr_reset, gensym("reset"), 0);
}
