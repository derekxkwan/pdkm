/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


/*based on the work of N. Jeremy Kasdin from IEEE, vol 83, No. 5, May 1995, p. 822
entitled Discrete Simulation of Colored Noise and Stochastic Processes 
and 1/f^alpha Power Law Noise Generation */

#include <stdlib.h>
#include "m_pd.h"
#include "dkrnd.h"


#define DKPINKCOEFF 10 //number of coefficients (counting a0)
#define DKPINKALPHA 1 //default for alpha  where alpha is between 0 and 2
static t_class *dkpink_tilde_class;

typedef struct _dkpink_tilde {
	t_object x_obj;
	t_float x_in; //dummy
        t_dkrnd * x_rnd;
        t_float x_alpha;
	t_inlet *x_alphalet;
	t_outlet *x_outlet;
        double a[DKPINKCOEFF];//coefficients
        double y[DKPINKCOEFF]; //prev outputs, for sanity's sake have y[i] = y[n-i]
                            //and we never use y[0] for that reason
} t_dkpink_tilde;

static void dkpink_tilde_alpha(t_dkpink_tilde *x, t_float alpha){
    /*alpha should be between 0 and 2, check then calc coeffs
    for y[n] = x[n] - a1*y[n-1]-a2*y[n-2]-a3*y[n-3]-...
    a0=1, ak = (k-1-(alpha/2))*(a[k-1]/k)
    */
    if(alpha < 0){
        alpha = 0.;
    }
    else if(alpha > 2){
        alpha = 2.;
    };
    x->x_alpha = alpha;
    int i;
    for(i=0;i<DKPINKCOEFF;i++){
        if(!i){
            x->a[i] = 1.;
        }
        else{
            x->a[i] = ((double)i-1.-((double)alpha/2.))*(x->a[i-1]/(double)i);
        };
    };

}



static void *dkpink_tilde_new(t_symbol *s, int argc, t_atom * argv){

        //could use t_floatarg f, but this is an easy way to check of an arg is passed or not
	t_dkpink_tilde *x = (t_dkpink_tilde *)pd_new(dkpink_tilde_class);
        t_float alpha = DKPINKALPHA;
        if(argc){
            if(argv->a_type == A_FLOAT){
                alpha = atom_getfloatarg(0,argc,argv);
            };
            argc--;
            argv++;
        };
        dkpink_tilde_alpha(x, alpha);
        //set range of dkrnd to be 0-2
        x->x_rnd = dkrnd_new(1,2.);

        int i; //init y array
        for(i=0;i<DKPINKCOEFF;i++){
            x->y[i] = 0.;
        };

	x->x_alphalet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("alpha"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkpink_tilde_perform(t_int *w){
	t_dkpink_tilde *x = (t_dkpink_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
	int i,j;
        for(i=0;i<n;i++){
            //dkrnd spits out 0-2 vals, need -1 to 1
            double wh = dkrnd_next(x->x_rnd);
            wh = wh-1.;
            //do the difference equation for the iir
            for(j=1;j<DKPINKCOEFF;j++){
                wh -= (x->a[j]*x->y[j]); 
            };

            out[i] = wh/(double)DKPINKCOEFF; //normalization
            //now increment y vals
            x->y[1] = wh;
            for(j=1;j<(DKPINKCOEFF-1);j++){
                x->y[j+1] = x->y[j];
            };
        };
	return(w+4);
}

static void dkpink_tilde_dsp(t_dkpink_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	dsp_add(dkpink_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);

}


static void dkpink_tilde_seed(t_dkpink_tilde *x, t_float _seed){
    int seed = _seed;
    if(seed < 0){
        seed = 0;
    };
        dkrnd_seed(x->x_rnd, (unsigned int)seed);
}



static void *dkpink_tilde_free(t_dkpink_tilde *x){
	inlet_free(x->x_alphalet);
	outlet_free(x->x_outlet);
        dkrnd_free(x->x_rnd);
	
	return (void *)x;
}




void dkpink_tilde_setup(void){
	dkpink_tilde_class = class_new(gensym("dkpink~"), (t_newmethod)dkpink_tilde_new,
                (t_method)dkpink_tilde_free,
			sizeof(t_dkpink_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(dkpink_tilde_class, (t_method)dkpink_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkpink_tilde_class, t_dkpink_tilde, x_in);
   class_addmethod(dkpink_tilde_class, (t_method)dkpink_tilde_seed, gensym("seed"), A_FLOAT, 0);
   class_addmethod(dkpink_tilde_class, (t_method)dkpink_tilde_alpha, gensym("alpha"), A_FLOAT, 0);
}
