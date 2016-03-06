#include "dxkwavtab.h"
#include "dxksinbank.h"


static t_class *sinbank_tilde_class;

typedef struct _sinbank_tilde {
	t_object x_obj;
	int x_numsin; //number of sines
	double relfreq[DXKSBMAX]; //array of relative frequencies	
	double stamps[DXKSBMAX]; //array of starting amplitudes
	double phases[DXKSBMAX]; //array of current phases
	double x_conv; //1/samprate, duration of one sample in seconds
	double x_sin[DXKTABLEN]; //table for holding sin values
	int x_mode; //mode, 0 = relative freqs, 1 = absolute freqs
	double x_sr; //sample rate
	t_float x_freq; //current specified freq
} t_sinbank_tilde;

static void sinbank_abs_to_rel(t_sinbank_tilde *x){
	int i, numsin = x->x_numsin, mode = x->x_mode;
	if(mode){
		double reffreq = x->relfreq[0];
		for(i=0; i<numsin; i++){
			x->relfreq[i] *= 1.f/reffreq;
		};
		x->x_freq = reffreq;
	};

}

static void sinbank_tilde_setmode(t_sinbank_tilde*x, t_symbol *s){
	if(strcmp(s->s_name, "abs") == 0){//absolute freqs;
		x->x_mode = 1;
	}
	else{//relative freqs
		x->x_mode = 0;
	}


}

static void sinbank_tilde_init(t_sinbank_tilde *x){
	int i;
	for(i=0; i<DXKSBMAX; i++){
		x->relfreq[i] = 0.f;
		x->stamps[i] = DXKSBAMP;
		x->phases[i] = 0.f;
	};
}

static void sinbank_tilde_freqs(t_sinbank_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<DXKSBMAX){
			t_float curfreq = atom_getfloatarg(i, argc, argv);
			x->relfreq[i] = (double)curfreq;
		};
	};
	x->x_numsin = argc;
	sinbank_abs_to_rel(x);
}

static void sinbank_tilde_amps(t_sinbank_tilde *x, t_symbol *s, int argc, t_atom *argv){
	int i;
	for(i=0; i<argc; i++){
		if(i<DXKSBMAX){
			t_float curamp = atom_getfloatarg(i, argc, argv);
			x->stamps[i] = (double)curamp;
		};
	};
}


static void *sinbank_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_sinbank_tilde *x = (t_sinbank_tilde *)pd_new(sinbank_tilde_class);
	x->x_mode = DXKSBMODE;
	int sincount = 0; //count actual number of sines
	x->x_sr = sys_getsr(); //get sampling rate
	x->x_freq = 0.f;
    x->x_conv = (double)DXKTABLEN/(double)x->x_sr; //amount to change phase for freq 1

	sinbank_tilde_init(x);
	while(argc> 0){
		t_symbol *curarg = atom_getsymbolarg(0, argc, argv);
		if(curarg == &s_){//if it is a number
			if(sincount <DXKSBMAX){
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
						sinbank_tilde_setmode(x, arg2);
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
		};
	};

	if(sincount == 0){//if no actual sines recorded
		goto errstate;
	};
	x->x_numsin = sincount; //set number of sines
	sinbank_abs_to_rel(x);
	for(i=0; i<DXKTABLEN; i++){//make sin table
		double xval = (double)i/(double)DXKTABLEN;
			double varterm = sin(xval*DXK2PI);
		x->x_sin[i] = varterm;
	};

	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("freqs"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("amps"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("ft1"));
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);

errstate:
		pd_error(x, "sinbank~: improper args");
		return NULL;

}



static double sinbank_tilde_getsin(t_sinbank_tilde *x, double phs){
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

static t_int *sinbank_tilde_perform(t_int *w){
	
	t_sinbank_tilde *x = (t_sinbank_tilde *)(w[1]);
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
				phs = fmod(phs, DXKTABLEN);//phases need to be wrapped  just in case
				while(phs < 0){
					phs += DXKTABLEN;
				};
				double curout = sinbank_tilde_getsin(x, phs);
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

static void sinbank_tilde_dsp(t_sinbank_tilde *x, t_signal **sp){
	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(sinbank_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
    x->x_conv = (double)DXKTABLEN/(double)sp[0]->s_sr; //amount to change phase for freq 1

}


static void sinbank_tilde_ft1(t_sinbank_tilde *x, t_symbol * s,int argc, t_atom *argv){//0 to 1
	int i;

	for(i=0; i<argc; i++){
		if(i < DXKSBMAX){
			t_float phase = atom_getfloatarg(i, argc, argv);
			phase *= DXKTABLEN;
				phase = fmod(phase, DXKTABLEN);//phases need to be wrapped  just in case
			while(phase < 0.f){
				phase += DXKTABLEN;
			};

				x->phases[i] = phase;

		};
	};
}


void sinbank_tilde_setup(void){
	sinbank_tilde_class = class_new(gensym("sinbank~"), (t_newmethod)sinbank_tilde_new, 0,
			sizeof(t_sinbank_tilde), 0, A_GIMME, 0);
   CLASS_MAINSIGNALIN(sinbank_tilde_class, t_sinbank_tilde, x_freq);
	class_addmethod(sinbank_tilde_class, (t_method)sinbank_tilde_dsp, gensym("dsp"), A_CANT, 0);
   class_addmethod(sinbank_tilde_class, (t_method)sinbank_tilde_freqs, gensym("freqs"), A_GIMME, 0);
   class_addmethod(sinbank_tilde_class, (t_method)sinbank_tilde_amps, gensym("amps"), A_GIMME, 0);
   class_addmethod(sinbank_tilde_class, (t_method)sinbank_tilde_setmode, gensym("mode"), A_SYMBOL, 0);
	class_addmethod(sinbank_tilde_class, (t_method)sinbank_tilde_ft1, gensym("ft1"), A_GIMME, 0);
}
