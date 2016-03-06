#include "dxkwavtab.h"
#include <string.h>

#ifndef DXKRBRAMPMS
#define DXKRBRAMPMS 1 //ramping time in ms
#endif

#ifndef DXKRBMODE
#define DXKRBMODE 0 //default mode
#endif

#ifndef DXKRBMAX
#define DXKRBMAX 30 //max number of sines
#endif

#ifndef DXKRBAMP
#define DXKRBAMP 1.f //amp default
#endif

#ifndef DXKRBRING
#define DXKRBRING 1000.f //ring time in ms
#endif

#ifndef DXKRBRDN
#define DXKRBRDN 1.f //ramping down time, not user settable
#endif


static t_class *resobank_tilde_class;

typedef struct _resobank_tilde {
	t_object x_obj;
	int x_numsin; //number of sines
	double relfreq[DXKRBMAX]; //array of relative frequencies	
	double stamps[DXKRBMAX]; //array of starting amplitudes
	double phases[DXKRBMAX]; //array of current phases
	double curamps[DXKRBMAX]; //array of current amplitudes
	double t60mult[DXKRBMAX]; // array of amp multipliers accordings to T60
	double x_conv; //1/samprate, duration of one sample in seconds
	double x_sin[DXKTABLEN]; //table for holding sin values
	int x_mode; //mode, 0 = relative freqs, 1 = absolute freqs
	double x_sr; //sample rate
	int x_rampsamp; //ramping time for attack
	double x_curfreq; //current specified freq
	double x_pendfreq; //pending freqency
	int x_newatt; //new attack
	int x_rampdown; //if currently ramping down
	int x_rampup; //if current ramping up
	double x_curramp; //current ramping amt
	double x_rampdt; //ranmping down time, not user settable
} t_resobank_tilde;

static void resobank_abs_to_rel(t_resobank_tilde *x){
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

static double get_t60_mult(t_resobank_tilde *x, double ringms){
	//ringms = ring time in ms, 0.001 = goal multiplication dest
	//log(x, 0.001) = times to mult = (samplerate/1000)*(ringms)
	//x = exp(log(0.001)/((samplerate/1000)*dur)
	double sr = x->x_sr;
	double times = (sr/1000.f)*ringms;
	double retval = exp(log(0.001)/times);
	return retval;
};

static void resobank_tilde_setmode(t_resobank_tilde*x, t_symbol *s){
	if(strcmp(s->s_name, "abs") == 0){//absolute freqs;
		x->x_mode = 1;
	}
	else{//relative freqs
		x->x_mode = 0;
	}


}

static void resobank_tilde_init(t_resobank_tilde *x){
	int i;
	for(i=0; i<DXKRBMAX; i++){
		x->relfreq[i] = 0.f;
		x->stamps[i] = DXKRBAMP;
		x->phases[i] = 0.f;
		x->curamps[i] = 0.f;
		x->t60mult[i] = get_t60_mult(x, DXKRBRING);
	};
}

static void resobank_tilde_setrampsamp(t_resobank_tilde *x, t_float rampms){
	double sr = x->x_sr;
	int rampsamp = (int)((double)rampms * sr * 0.001);
	x->x_rampsamp = rampsamp;


}

static void resobank_tilde_freqs(t_resobank_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<DXKRBMAX){
			t_float curfreq = atom_getfloatarg(i, argc, argv);
			x->relfreq[i] = (double)curfreq;
		};
	};
	x->x_numsin = argc;
	resobank_abs_to_rel(x);
}

static void resobank_tilde_amps(t_resobank_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<DXKRBMAX){
			t_float curamp = atom_getfloatarg(i, argc, argv);
			x->stamps[i] = (double)curamp;
		};
	};
}


static void resobank_tilde_rings(t_resobank_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<DXKRBMAX){
			t_float curring = atom_getfloatarg(i, argc, argv);
			x->t60mult[i] = get_t60_mult(x, (double)curring);
		};
	};
}


static void *resobank_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_resobank_tilde *x = (t_resobank_tilde *)pd_new(resobank_tilde_class);
	x->x_mode = DXKRBMODE;
	int sincount = 0; //count actual number of sines
	x->x_sr = sys_getsr(); //get sampling rate
	resobank_tilde_setrampsamp(x, DXKRBRAMPMS);
	x->x_curfreq = 0.f;
	x->x_pendfreq = 0.f;
    x->x_conv = (double)DXKTABLEN/(double)x->x_sr; //amount to change phase for freq 1
	x->x_newatt = 0; 
	x->x_rampdown = 0;
	x->x_rampup = 0;
	x->x_curramp = 1.f;
	x->x_rampdt = (int)(DXKRBRDN * x->x_sr * 0.001);

	resobank_tilde_init(x);
	while(argc> 0){
		t_symbol *curarg = atom_getsymbolarg(0, argc, argv);
		if(curarg == &s_){//if it is a number
			if(sincount <DXKRBMAX){
				t_float freq = atom_getfloatarg(0, argc, argv);
				x->relfreq[sincount] = (double)freq;
				sincount++;
			};
			argc--;
			argv++;
		}
		else{//symbol 
			if(argc >= 2){//should be at leas two args
				t_symbol * arg2 = atom_getsymbolarg(1, argc, argv);
				if(strcmp(curarg->s_name, "-mode") == 0){
					if(arg2 != &s_){//if not num, else just ignore arg
						resobank_tilde_setmode(x, arg2);
					}; 
					argc-=2;
					argv+=2;
				}
				else if(strcmp(curarg->s_name, "-ramp") == 0){
					if(arg2 == &s_){
						t_float rampms = atom_getfloatarg(1, argc, argv);
						resobank_tilde_setrampsamp(x, rampms);
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
		};
	};

	if(sincount == 0){//if no actual sines recorded
		goto errstate;
	};
	x->x_numsin = sincount; //set number of sines
	resobank_abs_to_rel(x);
	for(i=0; i<DXKTABLEN; i++){//make sin table
		double xval = (double)i/(double)DXKTABLEN;
			double varterm = sin(xval*DXK2PI);
		x->x_sin[i] = varterm;
	};

	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("freqs"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("amps"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("rings"));
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);

errstate:
		pd_error(x, "resobank~: improper args");
		return NULL;

}

static void resobank_arrs_reset(t_resobank_tilde *x){
	int i, numsin = x->x_numsin;
	for(i=0; i<numsin; i++){
		x->curamps[i] = x->stamps[i];
		x->phases[i] = 0.f;

	};

}

static void resobank_rampfunc(t_resobank_tilde *x){
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
			resobank_arrs_reset(x);
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

static double resobank_tilde_getsin(t_resobank_tilde *x, double phs){
	//getting the sin value for the given phase
		int tabphase1 = (int)phs;
		int tabphase2 = tabphase1 + 1;
		double frac = (double)phs - tabphase1;
		if(tabphase1 >= (DXKTABLEN - 1)){
			tabphase1 = DXKTABLEN - 1; //checking to see if index falls within bounds
			tabphase2 = 0;
		}
		else if(tabphase1 < 0){
			tabphase1 = 0;
			tabphase2 = 1;
		};
		double yb = x->x_sin[tabphase2]; //linear interp
		double ya = x->x_sin[tabphase1];
		double output = ya+((yb-ya)*frac);
	return output;

};

static t_int *resobank_tilde_perform(t_int *w){
	 t_resobank_tilde *x = (t_resobank_tilde *)(w[1]);
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
			resobank_rampfunc(x);
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
				phs = fmod(phs, DXKTABLEN);//phases need to be wrapped  just in case
				while(phs < 0){
					phs += DXKTABLEN;
				};
				double curout = resobank_tilde_getsin(x, phs);
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

static void resobank_tilde_dsp(t_resobank_tilde *x, t_signal **sp){
	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(resobank_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
    x->x_conv = (double)DXKTABLEN/(double)sp[0]->s_sr; //amount to change phase for freq 1

}

static void resobank_tilde_float(t_resobank_tilde *x, t_float freq){
	x->x_pendfreq = freq;
	x->x_newatt = 1;
}

static void resobank_tilde_bang(t_resobank_tilde *x){
	x->x_pendfreq = x->x_curfreq;
	x->x_newatt = 1;

}

void resobank_tilde_setup(void){
	resobank_tilde_class = class_new(gensym("resobank~"), (t_newmethod)resobank_tilde_new, 0,
			sizeof(t_resobank_tilde), 0, A_GIMME, 0);
	class_addfloat(resobank_tilde_class, (t_method)resobank_tilde_float);
	class_addbang(resobank_tilde_class, (t_method)resobank_tilde_bang);
	class_addmethod(resobank_tilde_class, (t_method)resobank_tilde_dsp, gensym("dsp"), A_CANT, 0);
   class_addmethod(resobank_tilde_class, (t_method)resobank_tilde_freqs, gensym("freqs"), A_GIMME, 0);
   class_addmethod(resobank_tilde_class, (t_method)resobank_tilde_amps, gensym("amps"), A_GIMME, 0);
   class_addmethod(resobank_tilde_class, (t_method)resobank_tilde_rings, gensym("rings"), A_GIMME, 0);
   class_addmethod(resobank_tilde_class, (t_method)resobank_tilde_setmode, gensym("mode"), A_SYMBOL, 0);
   class_addmethod(resobank_tilde_class, (t_method)resobank_tilde_setrampsamp, gensym("ramp"), A_FLOAT, 0);

}
