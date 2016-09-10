/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "m_pd.h"
#include "dxkrand.h"

static t_class *dkpop_tilde_class;

typedef struct _dkpop_tilde {
	t_object x_obj;
	t_float x_dens; // number of impulses per second
        int x_sr; //sampling rate
	t_outlet *x_outlet;
} t_dkpop_tilde;

static void *dkpop_tilde_new(t_floatarg dens){
	t_dkpop_tilde *x = (t_dkpop_tilde *)pd_new(dkpop_tilde_class);
	x->x_dens = dens;
        x->x_sr = sys_getsr();
        int i=0;
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkpop_tilde_perform(t_int *w){
	 t_dkpop_tilde *x = (t_dkpop_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int block = (int)(w[4]);
	int i=0;
        int sr = x->x_sr;
        for(i=0;i<block;i++){
            double dens = in[i];
            double thresh = (double)sr - dens;
            double onoff = dxkrand(); //if there will be an impulse this sample
            onoff *= (double)sr; 
            if(onoff >= thresh){
                //then there will be an impulse
                double amp = dxkrand(); //find the amplitude
                out[i] = amp; 
            }
            else{
                //else nothing
                out[i] = 0.;
            };
        };
        return(w+5);
}

static void dkpop_tilde_dsp(t_dkpop_tilde *x, t_signal **sp){

	//(dens*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..dens/samplerate
    x->x_sr = sp[0]->s_sr; //amount to change phase for dens 1
	    dsp_add(dkpop_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}


static void *dkpop_tilde_free(t_dkpop_tilde *x){
	outlet_free(x->x_outlet);
	
	return (void *)x;
}

void dkpop_tilde_setup(void){
	dkpop_tilde_class = class_new(gensym("dkpop~"), (t_newmethod)dkpop_tilde_new, 0,
			sizeof(t_dkpop_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
	class_addmethod(dkpop_tilde_class, (t_method)dkpop_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkpop_tilde_class, t_dkpop_tilde, x_dens);
}
