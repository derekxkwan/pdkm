/* Copyright 2017 - Derek Kwan
 *  * Distributed under GPL v3 */

//old naive additive version
#include "m_pd.h"

static t_class *dkdutysaw_tilde_class;

typedef struct _dkdutysaw_tilde {
	t_object x_obj;
	t_float x_freq; //scalar freq
        t_float x_duty;
	double x_phase; //0-1
        double x_conv; //1/samprate, duration of one sample in seconds
        t_inlet *x_dutylet;
	t_inlet *x_phaselet;
	t_outlet *x_outlet;
} t_dkdutysaw_tilde;

static void *dkdutysaw_tilde_new(t_floatarg freq, t_floatarg duty){
	int i,j;
	t_dkdutysaw_tilde *x = (t_dkdutysaw_tilde *)pd_new(dkdutysaw_tilde_class);
	x->x_freq = freq;
	x->x_phase = 0.;


	x->x_dutylet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        pd_float((t_pd *)x->x_dutylet, duty);

	x->x_phaselet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dkdutysaw_tilde_perform(t_int *w){
	 t_dkdutysaw_tilde *x = (t_dkdutysaw_tilde *)(w[1]);
	 t_float *in1 = (t_float *)(w[2]);
         t_float *in2 = (t_float *)(w[3]);
	t_float *out = (t_float *)(w[4]);
	int i, n = (int)(w[5]);
        double output;
        double phase = x->x_phase;
        double conv = x->x_conv;
	for(i=0; i< n; i++)
        {
            t_float freq = in1[i];
            t_float duty = in2[i];

            if(duty <= 0.)
            {
                output = phase*(-2.)+1.;
            }
            else if (duty >= 1.)
            {
                output = phase*2.-1.;
            }
            else
            {
                if(phase < duty)
                {
                    output = phase/(double)duty;
                    output = output * 2. -1;
                }
                else
                {
                    output = (1.-phase)/(double)duty;
                    output = output * 2. - 1.;
                };
            };

            out[i] = output;
            phase += (conv*freq);
            phase = phase < 0 ? 1 : (phase > 1 ? 0 : phase);


        };
	
        x->x_phase = phase;

        return(w+6);
}

static void dkdutysaw_tilde_dsp(t_dkdutysaw_tilde *x, t_signal **sp){

    x->x_conv = 1./(double)sp[0]->s_sr; //amount to change phase for freq 1
	    dsp_add(dkdutysaw_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);

}

static void dkdutysaw_tilde_ft1(t_dkdutysaw_tilde *x, t_float phase){
	x->x_phase = phase < 0 ? 0 : (phase > 1 ? 1 : phase);

}


static void *dkdutysaw_tilde_free(t_dkdutysaw_tilde *x){
	inlet_free(x->x_dutylet);
	inlet_free(x->x_phaselet);
	outlet_free(x->x_outlet);
	
	return (void *)x;
}


void dkdutysaw_tilde_setup(void){
	dkdutysaw_tilde_class = class_new(gensym("dkdutysaw~"), (t_newmethod)dkdutysaw_tilde_new, 0,
			sizeof(t_dkdutysaw_tilde), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(dkdutysaw_tilde_class, (t_method)dkdutysaw_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkdutysaw_tilde_class, t_dkdutysaw_tilde, x_freq);
   class_addmethod(dkdutysaw_tilde_class, (t_method)dkdutysaw_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
}
