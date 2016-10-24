/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "dkwtab.h"
#include <stdlib.h>
#include <math.h>
#include "m_pd.h"
#include "string.h"


#define LOG10 2.302585092994

//defaults
#define COMPTHRESH 93.9794 //0.5 rms
#define COMPSBELOW 1. //slope agove
#define COMPSABOVE 1. //slope below
#define COMPCLMS 5. //clamp time in ms
#define COMPRELMS 5. //relax time in ms

static t_class *dkcomp_tilde_class;

//dur = ms, sz = smp, sr/1000 = smp/ms
//x = sr/1000 * dur = smp traversed in dur ms
// sz/x = traverse these many samples in env to last dur ms
typedef struct _dkcomp_tilde {
	t_object x_obj;
        double x_ksr; //sr in ms
        t_float x_clms; //clamp time in ms
        t_float x_relms; //relax time in ms

        int x_rampct; //ramp counter
        int x_goalct; //count we want to get to

        t_float x_thresh; //threshold level in db
        t_float x_slopebelow; //slope below thresh
        t_float x_slopeabove; //slope above thresh
        double x_curgain; //current gain in rms 
        double x_prevdb; //db measurement from prev block

        t_float x_in; //input to be modified
        t_float x_ctrl; //countrol input
        t_inlet * x_ctrlet;
        t_inlet * x_envlet;
        t_inlet * x_lvllet;
        t_inlet * x_durlet;
} t_dkcomp_tilde;


//powtodb and dbtorms from pure data's x_acoustics.c
double powtodb(double f)
{
    double val = 0.;
    if(f <= 0){
        return 0;
    }
    else
    {
        val = 100 + (10.*log(f))/LOG10;
        if(val < 0){
            val = 0.;
        };
    };
    return val;
}

double dbtorms(double f)
{
    double val = 0.;
    if (f <= 0){
        val = 0.;
    }
    else{
        if (f > 485){
            f = 485;
        };

        val = exp((LOG10*0.05)*(double)(f-100.));
    };
    return val;

}



void *dkcomp_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_dkcomp_tilde *x = (t_dkcomp_tilde *)pd_new(dkcomp_tilde_class);
	t_atom params[10];
	x->x_sr = sys_getsr();
        x->x_sz = ENVSZ;
        x->x_type = PERC;
        x->x_redur = ENVRT;        	
        if(argc >= 1){
            if(argv->a_type == A_SYMBOL){
                t_symbol * arg1 = atom_getsymbolarg(0, argc, argv);
                if(strcmp(arg1->s_name, "perc")==0){
                    x->x_type = PERC;
                    }
                else if(strcmp(arg1->s_name, "linen")==0){
                    x->x_type = LINEN;
                }
                else if(strcmp(arg1->s_name, "sine")==0){
                    x->x_type = HANN;
                }
                else if (strcmp(arg1->s_name, "tri")==0){
                    x->x_type = TRI;

                
                }
                else{
                    goto errstate;
                };
                argc--;
                argv++;
            }
            else{
                goto errstate;
            };
        };


        dkcomp_param(x, 0, argc, argv);

	x->x_ctrlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	x->x_paramlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("param"));
	x->x_lvllet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("level"));
	x->x_durlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("dur"));
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
    errstate:
        post("dkcomp~: improper args");
        return NULL;
}


static t_int *dkcomp_tilde_perform(t_int *w){
	 t_dkcomp_tilde *x = (t_dkcomp_tilde *)(w[1]);
	 t_float *in = (t_float *)(w[2]);
	 t_float *ctrl = (t_float *)(w[3]);
	t_float *out = (t_float *)(w[4]);
	int n = (int)(w[3]);
        double ksr = x->x_ksr;
        double thresh = (double)x->x_thresh;
        double factor;
        double conv; //relax or clamp
        int i;
        double power = 0.;

        double prevdb = x->x_prevdb;

        //boundschecking
        if(thresh < 0.){
            thresh = 0.;
        };

        //first calculate power of ctrl
	for(i=0; i<n; i++){
            power += (ctrl[i]*ctrl[i]);
	};
        power /= (double)n;

        //convert to db
        
        double db = powtodb(power);
      
        if(prevdb != db){
            //if not equal, need to recalculate gainconv
                 
            //calculate adjustment amount
            //dist from thresh, scale (div) by factor, amt to increase/dec (dist of scaled from reference), (scaled diff- unscaled diff) = (diff)(scale - 1)
            if(db >= thresh){
                factor = 1./(double)slopeabove;
            }
            else{
                factor = 1./(double)slopebelow;
            };
            double gain = (db-thresh)*(factor-1.) + 100.; //add to full scale db
            
            //change back to rms
            gain = dbtorms(gain);
         
           if(((prevdb > db) && (prevdb >= thresh) && (db >= thresh)) ||
                (prevdb < db) && (prevdb <= thresh) && (db <= thresh)){
            //if db is closer to thresh than prevdb and both bigger (or less than) thresh
            // use relax, or else use attack

            //relax time in samp = relms * ksr, lower bound 1 ms
            double relsamp x->x_relms < 1. ? ksr : (x->x_relms * ksr);

            x->x_goalct = (int)relsamp; //amount of samples ramp will take

            //want to make up the diff between old gain and goal gain over specificed ms
            x->x_gainconv = (gain - x->x_curgain)/relsamp; //clamp amt per samp = 1/relax time in samples
            }
            else{

                double clsamp = x->x_clms < 1. ? ksr : (x->x_clms*ksr);

                x->x_goalct = (int)clsamp;

                x->x_gainconv = (gain - x->x_curgain)/((double)x->x_clms*ksr); //clamp amt per samp = 1/clamp time in samples
            };
            x->x_rampct = 0; //reset ramp counter

        };

        int goalct = x->x_goalct ; //count in samples we want to get to in ramp
        int rampct = x->x_rampct ; //current ramp counter
        double curgain = x->x_curgain; //current gain
        double gainconv = x->x_gainconv; //amt to change gain per sample
        
        for(i=0; i< n; i++){
            //increment gain
            if(rampct < goalct){
                curgain += gainconv;
                rampct++;
            };
            if(in[i] == 0.){
                out[i] = 0.;
            }
            else{
                out[i] = in[i] * curgain;
            };
      

        };

        //save variables for next go-around

        x->x_rampct = rampct;
        x->x_curgain = curgain;

	return(w+6);
}

static void dkcomp_tilde_dsp(t_dkcomp_tilde *x, t_signal **sp){
	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	        double ksr = sp[0]->s_sr*0.001;
                if(ksr != x->x_ksr){
                    //if sample rate changed, force gainconv recalc
                    x->x_prevdb = -2.;
                    x->x_ksr = ksr;
                };
	    dsp_add(dkcomp_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);

}

static void *dkcomp_tilde_free(t_dkcomp_tilde *x)
{
	inlet_free(x->x_paramlet);
	inlet_free(x->x_envlet);
	inlet_free(x->x_lvllet);
	inlet_free(x->x_durlet);
	return (void *)x;
}


void dkcomp_tilde_setup(void){
	dkcomp_tilde_class = class_new(gensym("dkcomp~"), (t_newmethod)dkcomp_tilde_new, (t_method)dkcomp_tilde_free,
			sizeof(t_dkcomp_tilde), 0, A_GIMME, 0);
	class_addmethod(dkcomp_tilde_class, (t_method)dkcomp_tilde_dsp, gensym("dsp"), A_CANT, 0);
   class_addmethod(dkcomp_tilde_class, (t_method)dkcomp_param, gensym("param"), A_GIMME, 0);
   class_addmethod(dkcomp_tilde_class, (t_method)dkcomp_env, gensym("env"), A_SYMBOL, 0);
   class_addmethod(dkcomp_tilde_class, (t_method)dkcomp_lvl, gensym("level"), A_FLOAT, 0);
   class_addmethod(dkcomp_tilde_class, (t_method)dkcomp_dur, gensym("dur"), A_FLOAT, 0);
    class_addbang(dkcomp_tilde_class, dkcomp_tilde_bang);
   CLASS_MAINSIGNALIN(dkcomp_tilde_class, t_dkcomp_tilde, x_in);
}
