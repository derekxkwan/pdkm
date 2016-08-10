/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "dxkwavtab.h"

#define DXKFMMAXMOD 20 //max number of modulators
static t_class *dxkfm_tilde_class;

typedef struct _dxkfm_tilde {
	t_object x_obj;
	t_float x_freq; //scalar freq
	double x_conv; //DXKTABLEN/samprate, duration of one sample in seconds
	double x_sin[DXKTABLEN]; //table for sine wave
        float x_carharm; //carrier harmonic rel to fund
        double x_carphs; //carrier phase
        int x_nummod; //number of modulators
        float x_modharm[DXKFMMAXMOD]; //modulator harmonics rel to fund
        double x_modidx[DXKFMMAXMOD]; //modulator indices (percentages)
        double x_modphs[DXKFMMAXMOD]; //modulator phases
        t_inlet *x_carhlet; //float inlet for carrier harmonic
        t_inlet *x_modhlet; //list inlet for modulator harmonics
        t_inlet *x_modilet; //list inlet for modulator indices
	t_inlet *x_phaselet;
	t_outlet *x_outlet;
} t_dxkfm_tilde;


static double phsbounds(double phs){
    //bounds checking for phase
    double retphs = phs;
    while(retphs < 0){
        retphs += DXKTABLEN;
    };
    while(retphs >= DXKTABLEN){
        retphs -= DXKTABLEN;
    };
    return retphs;
};

static float harmbounds(t_float f){
    //bounds checking for harmonics
    if(f < 0){
        return 0;
    }
    else{
        return (float) f;
    };
}


static void dxkfm_tilde_ft1(t_dxkfm_tilde *x, t_float f){
    double phs = fmod(f, DXKTABLEN);
    x->x_carphs = phs*DXKTABLEN;
}

static void dxkfm_tilde_carharm(t_dxkfm_tilde *x, t_float f){
    x->x_carharm = harmbounds(f);
}

static void dxkfm_tilde_modharm(t_dxkfm_tilde *x, t_symbol *s, int argc, t_atom * argv){
    int nummod =0; //number of modulators processed
    while(nummod < argc && nummod < DXKFMMAXMOD){
        t_float f = atom_getfloatarg(nummod, argc, argv);
        x->x_modharm[nummod] = harmbounds(f);
        //reset phases
        x->x_modphs[nummod] = 0.;
        nummod++;
    };
    x->x_carphs = 0.;
    x->x_nummod = nummod;
}

static void dxkfm_tilde_modidx(t_dxkfm_tilde *x, t_symbol *s, int argc, t_atom * argv){
    int nummod=0; //number of modulators processed
    while(nummod < argc && DXKFMMAXMOD){
        t_float f = atom_getfloatarg(nummod, argc, argv);
        x->x_modidx[nummod] = f;
        nummod++;
    };
}

static double dxkfm_getsin(t_dxkfm_tilde *x, double phs){
    //get sin value for current phase using linear interpolation
    int xa= (int)phs; //int part of phs
    int xb = (xa + 1) % DXKTABLEN; //next index
    double frac = phs - (double)xa;
    double yb = x->x_sin[xb]; //linear interp
    double ya = x->x_sin[xa];
    double output = ya+((yb-ya)*frac);
    return output;
}

static void *dxkfm_tilde_new(t_symbol *s, int argc, t_atom * argv){
	int i,j;
	t_dxkfm_tilde *x = (t_dxkfm_tilde *)pd_new(dxkfm_tilde_class);
        //setting defaults
        t_float freq = 0.;
        t_float carharm = 1;

	x->x_carphs = 0.f;

        //init mod arrays
        for(i=0; i<DXKFMMAXMOD; i++){
            x->x_modharm[i] = 0.f;
            x->x_modidx[i] = 0.;
            x->x_modphs[i] = 0.;
        };

        //now parse args
        int curarg = 0; //current arg number
        int nummod = 0; //number of modulators
        while(argc){
            if(argv->a_type == A_FLOAT){
                t_float curfloat = atom_getfloatarg(0, argc, argv);
                if(curarg == 0){
                    //first arg is freq
                    freq = curfloat;
                }
                else if(curarg == 1){
                    carharm=harmbounds(curfloat);
                }
                else{
                    int idx = (int)(curarg/2) - 1; //index for arrays
                    if(curarg % 2 == 0){
                        //even args = harmonics
                        x->x_modharm[idx] = harmbounds(curfloat);
                        nummod++;
                    }
                    else{
                        //odd indices = indices
                        x->x_modidx[idx] = (double)curfloat;
                    };
                };
            }
            else{
                goto errstate;
            };
            //increment/decrement
            curarg++;
            argc--;
            argv++;
        };

	//write sine wavetable
        for(i=0; i<DXKTABLEN; i++){
	    double xval = (double)i/(double)(DXKTABLEN);
	    double yval = sin(xval * DXK2PI);
	    x->x_sin[i] = yval;

	};

        x->x_nummod = nummod;
        x->x_freq = freq;
        x->x_carharm = carharm;
        x->x_conv = ((double)DXKTABLEN)/(double)sys_getsr();
	x->x_carhlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("carharm"));
	x->x_modhlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("modharm"));
	x->x_modilet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("modidx"));
	x->x_phaselet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);
	errstate:
		pd_error(x, "dxkfm~: improper args");
		return NULL;
}

static t_int *dxkfm_tilde_perform(t_int *w){
	 t_dxkfm_tilde *x = (t_dxkfm_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int block =  (int)(w[4]);
	int i = 0;
	for(i=0; i < block; i++){
            int j = 0;
           double output = 0.0;
           double freq = (double)in[i]; 
           double carphs = x->x_carphs;
           double curphs = carphs; //since we are modulating carphs, want to copy it to curphs which we can modulate
            //update carrier phase
            double newcarphs = carphs + (x->x_conv * freq * (double)x->x_carharm);
            x->x_carphs = phsbounds(newcarphs);
            for(j=0; j< x->x_nummod; j++){
                double modphs = x->x_modphs[j];
                double curout = dxkfm_getsin(x,modphs);
                //add to curphs (mult by DXKTABLEN since curout goes from -1 to 1
                //since idx is in percent, mult by 0.01
                curphs += (curout*(double)DXKTABLEN*(x->x_modidx[j]*0.01)); 
                //update phase of current mod
                double newmodphs = modphs + (x->x_conv * freq * (double)x->x_modharm[j]);
                x->x_modphs[j] = phsbounds(newmodphs);
            };
            //make sure modulated phase falls within range
            curphs = phsbounds(curphs);
            output = dxkfm_getsin(x,curphs);
            out[i] = output;
	};
	return(w+5);
}

static void dxkfm_tilde_dsp(t_dxkfm_tilde *x, t_signal **sp){

	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
    x->x_conv = (double)DXKTABLEN/(double)sp[0]->s_sr; //amount to change phase for freq 1
	    dsp_add(dxkfm_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}


static void *dxkfm_tilde_free(t_dxkfm_tilde *x){
	inlet_free(x->x_phaselet);
        inlet_free(x->x_carhlet);
        inlet_free(x->x_modhlet);
        inlet_free(x->x_modilet);
	outlet_free(x->x_outlet);
	
	return (void *)x;
}

void dxkfm_tilde_setup(void){
	dxkfm_tilde_class = class_new(gensym("dxkfm~"), (t_newmethod)dxkfm_tilde_new, 0,
			sizeof(t_dxkfm_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(dxkfm_tilde_class, (t_method)dxkfm_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dxkfm_tilde_class, t_dxkfm_tilde, x_freq);
   class_addmethod(dxkfm_tilde_class, (t_method)dxkfm_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
   class_addmethod(dxkfm_tilde_class, (t_method)dxkfm_tilde_carharm, gensym("carharm"), A_FLOAT, 0);
   class_addmethod(dxkfm_tilde_class, (t_method)dxkfm_tilde_modharm, gensym("modharm"), A_GIMME, 0);
   class_addmethod(dxkfm_tilde_class, (t_method)dxkfm_tilde_modidx, gensym("modidx"), A_GIMME, 0);
}
