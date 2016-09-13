/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include "dkwtab.h"
#include "dksnbnk.h"

static t_class *dksnbnkd_tilde_class;

typedef struct _dksnbnkd_tilde_tilde {
	t_object x_obj;
	int x_numsin; //number of sines
	double relfreq[SBMAX]; //array of relative frequencies	
	double stamps[SBMAX]; //array of starting amplitudes
	double phases[SBMAX]; //array of current phases
	double curamps[SBMAX]; //array of current amplitudes
	double t60mult[SBMAX]; // array of amp multipliers accordings to T60
	float  ringms[SBMAX]; //ring times in ms
	double x_conv; //1/samprate, duration of one sample in seconds
	double * x_sin; //table for holding sin values
	int x_mode; //mode, 0 = relative freqs, 1 = absolute freqs
	int x_sr; //sample rate
	int x_rampsamp; //ramping time for attack
	double x_curfreq; //current specified freq
	double x_pendfreq; //pending freqency
	int x_newatt; //new attack
	int x_rampdown; //if currently ramping down
	int x_rampup; //if current ramping up
	double x_curramp; //current ramping amt
	double x_rampdt; //ranmping down time, not user settable


	t_inlet *x_flistlet;
	t_inlet *x_alistlet;
	t_inlet *x_rlistlet;
	t_outlet *x_outlet;
} t_dksnbnkd_tilde_tilde;

static void dksnbnkd_tilde_abs_to_rel(t_dksnbnkd_tilde_tilde *x){
	int i, numsin = x->x_numsin, mode = x->x_mode;
	if(mode){
		double reffreq = x->relfreq[0];
		for(i=0; i<numsin; i++){
			x->relfreq[i] *= 1.f/reffreq;
		};
		x->x_pendfreq = reffreq;
		x->x_curfreq = reffreq;
	};

}

static double get_t60mult(t_dksnbnkd_tilde_tilde *x, float ringms){
	//ringms = ring time in ms, 0.001 = goal multiplication dest
	//log(x, 0.001) = times to mult = (samplerate/1000)*(ringms)
	//x = exp(log(0.001)/((samplerate/1000)*dur)
	int sr = x->x_sr;
	double times = ((double)sr/1000.f)*ringms;
	double retval = exp(log(0.001)/times);
	return retval;
};

static void dksnbnkd_tilde_setmode(t_dksnbnkd_tilde_tilde*x, t_symbol *s){
	if(strcmp(s->s_name, "abs") == 0){//absolute freqs;
		x->x_mode = 1;
	}
	else{//relative freqs
		x->x_mode = 0;
	}


}

static void dksnbnkd_tilde_init(t_dksnbnkd_tilde_tilde *x){
	int i;
	for(i=0; i<SBMAX; i++){
		x->relfreq[i] = 0.f;
		x->stamps[i] = SBAMP;
		x->phases[i] = 0.f;
		x->curamps[i] = 0.f;
		x->ringms[i] = SBRING;
		x->t60mult[i] = get_t60mult(x, SBRING);
	};
}

static void dksnbnkd_tilde_setrampsamp(t_dksnbnkd_tilde_tilde *x, t_float rampms){
	int sr = x->x_sr;
	int rampsamp = (int)((double)rampms * (double)sr * 0.001);
	x->x_rampsamp = rampsamp;


}

static void dksnbnkd_tilde_freqs(t_dksnbnkd_tilde_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<SBMAX){
			t_float curfreq = atom_getfloatarg(i, argc, argv);
			x->relfreq[i] = (double)curfreq;
		};
	};
	x->x_numsin = argc;
	dksnbnkd_tilde_abs_to_rel(x);
}

static void dksnbnkd_tilde_amps(t_dksnbnkd_tilde_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<SBMAX){
			t_float curamp = atom_getfloatarg(i, argc, argv);
			x->stamps[i] = (double)curamp;
		};
	};
}


static void dksnbnkd_tilde_rings(t_dksnbnkd_tilde_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<SBMAX){
			t_float curring = atom_getfloatarg(i, argc, argv);
			x->ringms[i] = curring;
			x->t60mult[i] = get_t60mult(x, curring);
		};
	};
}


static void *dksnbnkd_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_dksnbnkd_tilde_tilde *x = (t_dksnbnkd_tilde_tilde *)pd_new(dksnbnkd_tilde_class);
	x->x_mode = SBMODE;
	int sincount = 0; //count actual number of sines
	x->x_sr = sys_getsr(); //get sampling rate
	dksnbnkd_tilde_setrampsamp(x, SBRAMPMS);
	x->x_curfreq = 0.f;
	x->x_pendfreq = 0.f;
    x->x_conv = (double)TABLEN/(double)x->x_sr; //amount to change phase for freq 1
	x->x_newatt = 0; 
	x->x_rampdown = 0;
	x->x_rampup = 0;
	x->x_curramp = 1.f;
	x->x_rampdt = (int)(SBRDN * x->x_sr * 0.001);

	dksnbnkd_tilde_init(x);
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
						dksnbnkd_tilde_setmode(x, arg2);
					}; 
					argc-=2;
					argv+=2;
				}
				else if(strcmp(curarg->s_name, "-ramp") == 0){
					if(arg2 == &s_){
						t_float rampms = atom_getfloatarg(1, argc, argv);
						dksnbnkd_tilde_setrampsamp(x, rampms);
						argc -=2;
						argv += 2;
					}
					else{//not a number
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
		}
		else{
			goto errstate;
		};
	};

	if(sincount == 0){//if no actual sines recorded
		goto errstate;
	};
	x->x_numsin = sincount; //set number of sines
	dksnbnkd_tilde_abs_to_rel(x);

        dkmakesintab();
        x->x_sin = dksintab;	
	
        x->x_flistlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("freqs"));
	x->x_alistlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("amps"));
	x->x_rlistlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("rings"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);

errstate:
		pd_error(x, "sinbankd~: improper args");
		return NULL;

}

static void dksnbnkd_tilde_arrs_reset(t_dksnbnkd_tilde_tilde *x){
	int i, numsin = x->x_numsin;
	for(i=0; i<numsin; i++){
		x->curamps[i] = x->stamps[i];
		x->phases[i] = 0.f;

	};

}

static void dksnbnkd_tilde_rampfunc(t_dksnbnkd_tilde_tilde *x){
	int down = x->x_rampdown;
	int up = x->x_rampup;
	int rs = x->x_rampsamp;
	double curramp = x->x_curramp;
	if(down){
		int rampdt = x->x_rampdt; //ramping down time in samps
		if(curramp > 0.f){
			curramp -= 1.f/(double)rampdt;
			if(curramp < 0.f){
				curramp = 0.f;
			};
			x->x_curramp = curramp;

		}
		else{
			x->x_rampdown = 0;
			x->x_rampup = 1;
			x->x_curfreq = x->x_pendfreq;
			//now must reset curamps and phases arrs
			dksnbnkd_tilde_arrs_reset(x);
		};
	}
	else if(up){
		if(curramp < 1.f){
			curramp += 1.f/(double)rs;
			if(curramp >= 1.f){
				curramp = 1.f;
			};
			x->x_curramp = curramp;
		}
		else{
			x->x_rampup = 0;

		};

	};

}

static t_int *dksnbnkd_tilde_perform(t_int *w){
	 t_dksnbnkd_tilde_tilde *x = (t_dksnbnkd_tilde_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
	float conv = x->x_conv;	
	int numsin = x->x_numsin;
	while(n--){
		if(x->x_newatt){
			x->x_newatt = 0;
			x->x_rampdown = 1;//start ramping down
			x->x_rampup = 0; // cancel potential ramping up
		};
		if(x->x_rampdown || x->x_rampup){
			dksnbnkd_tilde_rampfunc(x);
		};
		double rampamp = x->x_curramp;
		double freq = x->x_curfreq;

		int i;
		double output = 0.f;
		if(freq > 0.f && numsin > 0){
			for(i=0; i<numsin; i++){
				double phs = x->phases[i];
				double relfreq = x->relfreq[i];
				double curamp = x->curamps[i];
				double ampmul = x->t60mult[i];
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
				curamp *= ampmul;

				//now set everything
				
				x->curamps[i] = curamp;
				x->phases[i] = phs;
			};

			//normalize and ramp total output;
			output *= 1.f/(double)numsin;
			output *= rampamp;
		};

		//output it!
		*out++ = output;
	};
	return(w+4);
}

static void dksnbnkd_tilde_dsp(t_dksnbnkd_tilde_tilde *x, t_signal **sp){
	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(dksnbnkd_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
		int sr = sp[0]->s_sr;
		if(x->x_sr != sr){
			int i, numsin = x->x_numsin;
			x->x_sr = sr;
    		x->x_conv = (double)TABLEN/(double)sr; //amount to change phase for freq 1
			for(i=0; i<numsin; i++){
				float curring = x->ringms[i];
				x->t60mult[i] = get_t60mult(x, curring);
			};
		};

}

static void dksnbnkd_tilde_float(t_dksnbnkd_tilde_tilde *x, t_float freq){
	x->x_pendfreq = freq;
	x->x_newatt = 1;
}

static void dksnbnkd_tilde_bang(t_dksnbnkd_tilde_tilde *x){
	x->x_pendfreq = x->x_curfreq;
	x->x_newatt = 1;

}

static void *dksnbnkd_tilde_free(t_dksnbnkd_tilde_tilde *x){
	inlet_free(x->x_flistlet);
	inlet_free(x->x_alistlet);
	inlet_free(x->x_rlistlet);
	outlet_free(x->x_outlet);
	return (void *)x;

}


void dksnbnkd_tilde_setup(void){
	dksnbnkd_tilde_class = class_new(gensym("dksnbnkd~"), (t_newmethod)dksnbnkd_tilde_new, 0,
			sizeof(t_dksnbnkd_tilde_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addfloat(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_float);
	class_addbang(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_bang);
	class_addmethod(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_dsp, gensym("dsp"), A_CANT, 0);
   class_addmethod(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_freqs, gensym("freqs"), A_GIMME, 0);
   class_addmethod(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_amps, gensym("amps"), A_GIMME, 0);
   class_addmethod(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_rings, gensym("rings"), A_GIMME, 0);
   class_addmethod(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_setmode, gensym("mode"), A_SYMBOL, 0);
   class_addmethod(dksnbnkd_tilde_class, (t_method)dksnbnkd_tilde_setrampsamp, gensym("ramp"), A_FLOAT, 0);

}
