/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */
#include "dkrnd.h"
#include "dkwtab.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PSGLEN 240000 //max length of delaybuf
#define PSGWLEN 200 //length of window in ms
#define PSGMIND 1.5 //min delay in ms for each grain
#define PSGN 4 //number of grains
#define PSGTDISP 50 //time dispersion in ms, can't be more than window length
static t_class *dkgpitch_tilde_class;

typedef struct _dkgpitch_tilde {
	t_object x_obj;
	double x_dbuf[PSGLEN]; //delay table
        double x_phs; //phasor for the rotating tape head, between 0 and 1
        dkrnd * x_rnd; //random number generator
	int x_wh; //write head for stutter buf
	//readhead will be writehead + sr*0.001*msdelay for each of the 4 grains
	double * x_win; //grain window pointer
        //readhead sim to readhead for main delay buf
        double x_gdprop[PSGN]; //grain delays in proportion of phase
        int x_sr; //sample rate
        double x_tdisp; //time dispersion in ms, can't be more than window length
        double x_wms; //window length in ms
        t_float x_tp; //transposition in half steps
        double x_freq; //frequency of phasor
	t_float x_input; //dummy
	t_inlet *x_tlet; //transposition inlet
	t_outlet *x_outlet;
} t_dkgpitch_tilde;

static void dkhs2f(t_dkgpitch_tilde *x){
    //transposition in halfsteps to frequency
    //speed change is 2^(tp/12) = exp((log2/12)*tp) ~= exp(0.05776*tp)
    double tp = (double)x->x_tp;
    t_float wms = x->x_wms;
    double change = exp(0.05776*tp);
    double freq = -1.*(change-1.)/(wms*0.001);
    x->x_freq = freq;
}

static void dkgpitch_tp(t_dkgpitch_tilde * x, t_float tp){
    if(tp < -48.){
        tp = -48.;
    }
    else if(tp > 48.){
        tp = 48.;
    };
    x->x_tp = tp;
    dkhs2f(x);
}

static void dkgpitch_tdisp(t_dkgpitch_tilde *x, t_float _tdisp){
    double tdisp = (double)_tdisp;
    if(tdisp < 0){
        tdisp = 0.;
    }
    else if(tdisp > x->x_wms){
        tdisp = x->x_wms;
    };
    x->x_tdisp = tdisp;
}

static void dkgpitch_wms(t_dkgpitch_tilde *x, t_float wms){
    if(wms < 1){
        wms = 1;
    };
    x->x_wms = wms;
}

static double dkgpitch_grnrd(t_dkgpitch_tilde *x, int idx, double phs){
    //take in main phase, calculate grain's phase, recalc grain delay if needed
    //read appropriate output from grain delay buf and return
    double offset = 0.25 * (double)idx;
    double curphs = phs - offset;
    int newdel = 0; //if we need a new delay
    if(curphs >= 1.){
        curphs -=1.;
        newdel = 1;
    }
    else if(curphs < 0.){
        curphs += 1.;
        newdel = 1;
    };
    //recalculate grain delay if necessary
    if(newdel){
        x->x_gdprop[idx] = dkrnd_next(x->x_rnd) * (x->x_tdisp/x->x_wms);
    };
    double output = 0.; 
    //idea: each grain can start from the grain's phase, but delayed by delay proportion of phase
    //scale resulting grain by wms*(1-gdprop) but read from delaybuf normally
    if(curphs >= x->x_gdprop[idx]){
        //getting proper place to read from main delay buf
        double bufphs = (curphs*x->x_wms)+(double)PSGMIND; //getting proper ms of delay
        bufphs *= ((double)x->x_sr*0.001); //converting to samples
        bufphs = (double)x->x_wh+((double)PSGLEN-bufphs); //essentially subracting from writehead to find proper position in buffer
        //wrapping into lenght of delay buffer
        while(bufphs >= PSGLEN){
            bufphs -= (double)PSGLEN;
        };
        //reading from main delaybuf
        output  = dkgetlin(x->x_dbuf, PSGLEN, bufphs);

        //get scaled window phase 
        double winphs = curphs - x->x_gdprop[idx];
        winphs *= (double)TABLEN;
        double winval = dkgetlin(x->x_win, TABLEN, winphs);
        //window output
        output *= winval;
    };

    return output;


}

static void *dkgpitch_tilde_new(t_symbol * s, int argc, t_atom * argv){
	//float arg is transposition
        int i, j;
	t_dkgpitch_tilde *x = (t_dkgpitch_tilde *)pd_new(dkgpitch_tilde_class);
	x->x_sr = sys_getsr();
        x->x_rnd = dkrnd_new(0);
        //defaults
        t_float wms = PSGWLEN;
        t_float tdisp = PSGTDISP;
        t_float tp = 0;

        while(argc){
            if(argv->a_type == A_FLOAT){
               tp = atom_getfloatarg(0, argc, argv);
                argc--;
                argv++;
            }
            else if(argv->a_type == A_SYMBOL){
               if(argc >= 2){
                    t_symbol * curs = atom_getsymbolarg(0, argc, argv);
                    t_float curf = atom_getfloatarg(1, argc, argv);
                    if(strcmp(curs->s_name, "-tdisp")==0){
                        //time dispersion
                        tdisp = curf;
                    }
                    else if(strcmp(curs->s_name, "-wms")==0){
                        //window length in ms
                        wms =curf;
                    }
                    else{
                        goto errstate;
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
        dkgpitch_tdisp(x, tdisp);
        dkgpitch_wms(x, wms);
        //make window
	dkmakecoswin();
        x->x_win = dkcoswin; 
        
        //init out stuff
	x->x_wh = 0.;
        for(i=0; i<PSGN; i++){
            //x->x_gdprop[i] = dkrnd_next(x->x_rnd) * (x->x_tdisp/x->x_wms);
            x->x_gdprop[i] = 0.;
        };
        for(i=0;i<PSGLEN; i++){
            x->x_dbuf[i] = 0.;
        };

        dkgpitch_tp(x, tp);

	x->x_tlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("tp"));
	x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
	return (x);

	errstate:
		pd_error(x, "dkgpitch~: improper args");
		return NULL;
}



static t_int *dkgpitch_tilde_perform(t_int *w){
	 t_dkgpitch_tilde *x = (t_dkgpitch_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
        int i,j;
	double freq = x->x_freq;
        //amt to change x->x_phs
        double conv = 1./(double)x->x_sr * freq;
	for(i=0; i<n;i++){
           double phs = x->x_phs;
           double output = 0.;
           double input = in[i];
           //write input into delay
           x->x_dbuf[x->x_wh] = input;
           
           //iterate through each grain
            for(j=0;j<PSGN; j++){
                double grnout = dkgpitch_grnrd(x, j, phs);
                output += grnout;
            };
            //scale output to account for 4x overlap and set into out
            output *= 0.25;
            out[i] = output;

           //incrementing 
           phs = phs + conv;
            if(phs >= 1.){
                phs -= 1.;
            }
            else if(phs < 0){
                phs += 1.; 
            };
            x->x_phs = phs;
            x->x_wh = (x->x_wh + 1) % PSGLEN;

        };
        return(w+5);
}

static void dkgpitch_tilde_dsp(t_dkgpitch_tilde *x, t_signal **sp){

    int sr = sp[0]->s_sr; 
    dsp_add(dkgpitch_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}


static void *dkgpitch_tilde_free(t_dkgpitch_tilde *x){
        dkrnd_free(x->x_rnd);
	inlet_free(x->x_tlet);
	outlet_free(x->x_outlet);
	
	return (void *)x;
}

void dkgpitch_tilde_setup(void){
	dkgpitch_tilde_class = class_new(gensym("dkgpitch~"), (t_newmethod)dkgpitch_tilde_new,
                (t_method)dkgpitch_tilde_free, sizeof(t_dkgpitch_tilde),
                CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(dkgpitch_tilde_class, (t_method)dkgpitch_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(dkgpitch_tilde_class, t_dkgpitch_tilde, x_input);
	class_addmethod(dkgpitch_tilde_class, (t_method)dkgpitch_wms, gensym("wms"), A_FLOAT, 0);
	class_addmethod(dkgpitch_tilde_class, (t_method)dkgpitch_tdisp, gensym("tdisp"), A_FLOAT, 0);
	class_addmethod(dkgpitch_tilde_class, (t_method)dkgpitch_tp, gensym("tp"), A_FLOAT, 0);
}
