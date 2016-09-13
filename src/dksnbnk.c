/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include "dkwtab.h"
#include "dksnbnk.h"


static t_class *dksnbnk_tilde_class;

typedef struct _dksnbnk_tilde {
	t_object x_obj;
	int x_numsin; //number of sines
	double relfreq[SBMAX]; //array of relative frequencies	
	double stamps[SBMAX]; //array of starting amplitudes
	double phases[SBMAX]; //array of current phases
	double x_conv; //1/samprate, duration of one sample in seconds
	double * x_sin; //table for holding sin values
	int x_mode; //mode, 0 = relative freqs, 1 = absolute freqs
	int x_sr; //sample rate
	t_float x_freq; //current specified freq


	t_inlet *x_flistlet;
	t_inlet *x_alistlet;
	t_inlet *x_plistlet;
	t_outlet *x_outlet;

} t_dksnbnk_tilde;

static void dksnbnk_tilde_abs_to_rel(t_dksnbnk_tilde *x){
	int i, numsin = x->x_numsin, mode = x->x_mode;
	if(mode){
		double reffreq = x->relfreq[0];
		for(i=0; i<numsin; i++){
			x->relfreq[i] *= 1.f/reffreq;
		};
		x->x_freq = reffreq;
	};

}

static void dksnbnk_tilde_setmode(t_dksnbnk_tilde*x, t_symbol *s){
	if(strcmp(s->s_name, "abs") == 0){//absolute freqs;
		x->x_mode = 1;
	}
	else{//relative freqs
		x->x_mode = 0;
	}


}

static void dksnbnk_tilde_init(t_dksnbnk_tilde *x){
	int i;
	for(i=0; i<SBMAX; i++){
		x->relfreq[i] = 0.f;
		x->stamps[i] = SBAMP;
		x->phases[i] = 0.f;
	};
}

static void dksnbnk_tilde_freqs(t_dksnbnk_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<SBMAX){
			t_float curfreq = atom_getfloatarg(i, argc, argv);
			x->relfreq[i] = (double)curfreq;
		};
	};
	x->x_numsin = argc;
	dksnbnk_tilde_abs_to_rel(x);
}

static void dksnbnk_tilde_amps(t_dksnbnk_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<SBMAX){
			t_float curamp = atom_getfloatarg(i, argc, argv);
			x->stamps[i] = (double)curamp;
		};
	};
}


static void *dksnbnk_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_dksnbnk_tilde *x = (t_dksnbnk_tilde *)pd_new(dksnbnk_tilde_class);
	x->x_mode = SBMODE;
	int sincount = 0; //count actual number of sines
	x->x_sr = sys_getsr(); //get sampling rate
	x->x_freq = 0.f;
    x->x_conv = (double)TABLEN/(double)x->x_sr; //amount to change phase for freq 1

	dksnbnk_tilde_init(x);
	while(argc> 0){
		if(argv -> a_type == A_FLOAT){//if it is a number
			if(sincount <SBMAX){
				t_float freq = atom_getfloatarg(0, argc, argv);
				x->relfreq[sincount] = (double)freq;
				sincount++;
			};
			argc--;
			argv++;
		}
		else if (argv ->a_type == A_SYMBOL){//symbol 
			t_symbol *curarg = atom_getsymbolarg(0, argc, argv);
			if(argc >= 2){//should be at leas two args
				t_symbol * arg2 = atom_getsymbolarg(1, argc, argv);
				if(strcmp(curarg->s_name, "-mode") == 0){
					if(arg2 != &s_){//if not num, else just ignore arg
						dksnbnk_tilde_setmode(x, arg2);
					}; 
					argc-=2;
					argv+=2;
				}
				else{
					goto errstate;
				};
			}
			else{
				goto errstate;
			};
		}
		else{
			goto errstate;
		};
	};

	if(sincount == 0){//if no actual sines recorded
		goto errstate;
	};
	x->x_numsin = sincount; //set number of sines
	dksnbnk_tilde_abs_to_rel(x);

        dkmakesintab();
        x->x_sin = dksintab;	

        x->x_flistlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("freqs"));
	x->x_alistlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("amps"));
	x->x_plistlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("ft1"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);

errstate:
		pd_error(x, "dksnbnk~: improper args");
		return NULL;

}

static t_int *dksnbnk_tilde_perform(t_int *w){
	
	t_dksnbnk_tilde *x = (t_dksnbnk_tilde *)(w[1]);
	t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	
	float conv = x->x_conv;	
	int numsin = x->x_numsin;
	while(n--){
		double freq = *in++;
		int i;
		double output = 0.f;
		if(freq > 0.f && numsin > 0){
			for(i=0; i<numsin; i++){
				double phs = x->phases[i];
				double relfreq = x->relfreq[i];
				double curamp = x->stamps[i];
				phs = fmod(phs, TABLEN);//phases need to be wrapped  just in case
				while(phs < 0){
					phs += TABLEN;
				};
				double curout = dkgetlin(x->x_sin, TABLEN, phs);
				curout *= (curamp);

				//add contribution to total output

				output += curout;
				//increment everything

				phs += (relfreq*freq) * conv;

				//now set everything
				
				x->phases[i] = phs;
			};

			//normalize and ramp total output;
			output *= 1.f/(double)numsin;
		};

		//output it!
		*out++ = output;
	};
	return(w+5);
}

static void dksnbnk_tilde_dsp(t_dksnbnk_tilde *x, t_signal **sp){
	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(dksnbnk_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
    x->x_conv = (double)TABLEN/(double)sp[0]->s_sr; //amount to change phase for freq 1

}


static void dksnbnk_tilde_ft1(t_dksnbnk_tilde *x, t_symbol * s,int argc, t_atom *argv){//0 to 1
	int i;

	for(i=0; i<argc; i++){
		if(i < SBMAX){
			t_float phase = atom_getfloatarg(i, argc, argv);
			phase *= TABLEN;
				phase = fmod(phase, TABLEN);//phases need to be wrapped  just in case
			while(phase < 0.f){
				phase += TABLEN;
			};

				x->phases[i] = phase;

		};
	};
}


static void *dksnbnk_tilde_free(t_dksnbnk_tilde *x){
	inlet_free(x->x_flistlet);
	inlet_free(x->x_alistlet);
	inlet_free(x->x_plistlet);
	outlet_free(x->x_outlet);
	return (void *)x;

}


void dksnbnk_tilde_setup(void){
	dksnbnk_tilde_class = class_new(gensym("dksnbnk~"), (t_newmethod)dksnbnk_tilde_new, 0,
			sizeof(t_dksnbnk_tilde), CLASS_DEFAULT, A_GIMME, 0);
   CLASS_MAINSIGNALIN(dksnbnk_tilde_class, t_dksnbnk_tilde, x_freq);
	class_addmethod(dksnbnk_tilde_class, (t_method)dksnbnk_tilde_dsp, gensym("dsp"), A_CANT, 0);
   class_addmethod(dksnbnk_tilde_class, (t_method)dksnbnk_tilde_freqs, gensym("freqs"), A_GIMME, 0);
   class_addmethod(dksnbnk_tilde_class, (t_method)dksnbnk_tilde_amps, gensym("amps"), A_GIMME, 0);
   class_addmethod(dksnbnk_tilde_class, (t_method)dksnbnk_tilde_setmode, gensym("mode"), A_SYMBOL, 0);
	class_addmethod(dksnbnk_tilde_class, (t_method)dksnbnk_tilde_ft1, gensym("ft1"), A_GIMME, 0);
}
