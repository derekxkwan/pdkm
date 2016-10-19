/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include "m_pd.h"
#include <stdlib.h>

static t_class *dkrpt_class;

typedef struct _dkrpt {
	t_object x_obj;
	t_float x_rpt; //number of repeats
        t_float x_cnt; //if sending out count
        t_outlet *x_anylet; //main output
        t_outlet *x_cntlet; //outlet for count
} t_dkrpt;




static void *dkrpt_new(t_floatarg rpt, t_floatarg cnt){
	t_dkrpt *x = (t_dkrpt *)pd_new(dkrpt_class);
	x->x_rpt = rpt >= 0 ? rpt: 0.;
        x->x_cnt = cnt > 0 ? 1 : 0;
	
        x->x_anylet = outlet_new(&x->x_obj, 0);
        x->x_cntlet = outlet_new(&x->x_obj, &s_float);
	floatinlet_new(&x->x_obj, &x->x_rpt);
	floatinlet_new(&x->x_obj, &x->x_cnt);
	return (x);
}


static void dkrpt_anything(t_dkrpt *x, t_symbol *s, int argc, t_atom * argv){
    int rpt = x->x_rpt >= 0 ? (int)x->x_rpt: 0; 
    int cnt = x->x_cnt > 0 ? 1 : 0;

    int cur = 0; //number of times repeated (so goes up to rpt - 1)
    while(rpt){
	// while rpt > 0
        if(cnt){
            outlet_float(x->x_cntlet, (t_float)cur);
            cur++;
        };
        outlet_anything(x->x_anylet, s, argc, argv);
        rpt--; 
    };
}

static void *dkrpt_free(t_dkrpt *x){
    outlet_free(x->x_anylet);
    outlet_free(x->x_cntlet);
    return (void *)x;
}



void dkrpt_setup(void){
	dkrpt_class = class_new(gensym("dkrpt"), (t_newmethod)dkrpt_new, (t_method)dkrpt_free,
			sizeof(t_dkrpt), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addanything(dkrpt_class, dkrpt_anything);
}
