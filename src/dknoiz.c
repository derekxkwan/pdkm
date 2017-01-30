/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "m_pd.h"
#include "dkrnd.h"

static t_class *dknoiz_tilde_class;

typedef struct _dknoiz_tilde {
	t_object x_obj;
	t_float x_freq; //freq
        int x_sr; //sampling rate
        int x_ramp; //if we are ramping
        double x_last; //last random number
        double x_cur; //current random number
        double x_lo; //lo part of range
        double x_hi; //hi part of range 
        double x_range; //range
        double x_phase; 
        int x_newrand; //if we want a new rand
        t_dkrnd * x_rnd; //rand obj
        t_inlet *x_phaselet; 
	t_outlet *x_outlet;
} t_dknoiz_tilde;

static void *dknoiz_tilde_new(t_symbol *s, int argc, t_atom * argv){
	t_dknoiz_tilde *x = (t_dknoiz_tilde *)pd_new(dknoiz_tilde_class);
        t_float lo = -1., hi = 1., freq = 0., ramp = 1;
        int argnum = 0; 
        //arg 1 = freq,
        //if argc 2, freq, hi
        //if argc 3, freq, lo hi
        //if argc 4, freq, lo hi ramp
        switch(argc){
            case 0: break;
            case 1:
                freq = atom_getfloatarg(0, argc, argv);
                break;
            case 2:
                freq = atom_getfloatarg(0, argc, argv);
                hi = atom_getfloatarg(1, argc, argv);
                break;
            default:
            case 4:
                ramp = atom_getfloatarg(3, argc, argv);
            case 3:
                freq = atom_getfloatarg(0, argc, argv);
                lo = atom_getfloatarg(1, argc, argv);
                hi = atom_getfloatarg(2, argc, argv);
                break;
        };
        x->x_ramp = ramp <= 0 ? 0 : 1;
        x->x_sr = sys_getsr();
        x->x_freq = freq;
        x->x_lo = (double)lo;
        x->x_hi = (double)hi;
        x->x_range = x->x_hi - x->x_lo;
        x->x_rnd = dkrnd_new(1, x->x_range);
        x->x_cur = dkrnd_next(x->x_rnd) + x->x_lo;
        x->x_newrand = 1;
        x->x_phase = 0.;
	x->x_phaselet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static t_int *dknoiz_tilde_perform(t_int *w){
	 t_dknoiz_tilde *x = (t_dknoiz_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int block = (int)(w[4]);
	int i=0;
        int sr = x->x_sr;
        int ramp = x->x_ramp;
        int newrand = x->x_newrand; //if we want a new rand
        double phase = x->x_phase; 
        double output;
        for(i=0;i<block;i++){
            double freq = (double)in[i];
            freq = freq < 0 ? 0. : freq;

            if(newrand){
                newrand =0;
                x->x_last = x->x_cur;
                x->x_cur = dkrnd_next(x->x_rnd) + x->x_lo;
            };

            output = x->x_last;
            if(ramp){
                output += ((x->x_cur - x->x_last) * phase);
            };

            out[i] = output;
            phase += freq/(double)sr;
            while(phase >= 1){
                phase -= 1.;
                newrand = 1;
            };
            while(phase < 0){
                phase += 1.;
                newrand = 1;
            };
        };
        x->x_phase = phase;
        x->x_newrand = newrand;
        return(w+5);
}

static void dknoiz_tilde_dsp(t_dknoiz_tilde *x, t_signal **sp){

	//(dens*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..dens/samplerate
    x->x_sr = sp[0]->s_sr; //amount to change phase for dens 1
    x->x_newrand = 1;
	    dsp_add(dknoiz_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void dknoiz_tilde_range(t_dknoiz_tilde *x, t_float lo, t_float hi){
        x->x_lo = (double)lo;
        x->x_hi = (double)hi;
        x->x_range = x->x_hi - x->x_lo;
        dkrnd_range(x->x_rnd, x->x_range);
}

static void dknoiz_tilde_reset(t_dknoiz_tilde *x){
    x->x_phase = 0;
    x->x_newrand = 1;
}


static void dknoiz_tilde_phase(t_dknoiz_tilde *x, t_float phase){
            int newrand = 0;
            while(phase >= 1){
                phase -= 1.;
                newrand = 1;
            };
            while(phase < 0){
                phase += 1.;
                newrand = 1;
            };
            x->x_phase = phase;
            x->x_newrand = newrand;
}

static void dknoiz_tilde_ramp(t_dknoiz_tilde *x, t_float ramp){
    x->x_ramp = ramp <= 0 ? 0 : 1;
}    

static void *dknoiz_tilde_free(t_dknoiz_tilde *x){
	outlet_free(x->x_outlet);
	inlet_free(x->x_phaselet);
	dkrnd_free(x->x_rnd);
	return (void *)x;
}

void dknoiz_tilde_setup(void){
	dknoiz_tilde_class = class_new(gensym("dknoiz~"), (t_newmethod)dknoiz_tilde_new, 0,
			sizeof(t_dknoiz_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(dknoiz_tilde_class, (t_method)dknoiz_tilde_dsp, gensym("dsp"), A_CANT, 0);
	class_addmethod(dknoiz_tilde_class, (t_method)dknoiz_tilde_range, gensym("range"), A_FLOAT, A_FLOAT, 0);
	class_addmethod(dknoiz_tilde_class, (t_method)dknoiz_tilde_phase, gensym("phase"), A_FLOAT, 0);
	class_addmethod(dknoiz_tilde_class, (t_method)dknoiz_tilde_ramp, gensym("ramp"), A_FLOAT, 0);
	class_addmethod(dknoiz_tilde_class, (t_method)dknoiz_tilde_reset, gensym("reset"), 0);
   CLASS_MAINSIGNALIN(dknoiz_tilde_class, t_dknoiz_tilde, x_freq);
}
