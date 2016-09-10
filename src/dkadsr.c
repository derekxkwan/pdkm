/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include "m_pd.h"
#include <string.h>

#ifndef ADSRL
#define ADSRL 1.f //default level
#endif

//have to consider if release comes if in middle of one of the stages, pick shortest of decay and release and then do it to 0
//have to consider if new attack, happens before release, just reattack from current level

static t_class *dkadsr_tilde_class;

typedef struct _dkadsr_tilde {
	t_object x_obj;
	int x_sr; // sample rate

	float x_attms; //attack in ms
	double x_attinc; //attack incrementor per samp normalized to 1
	float x_decms; //decay in samples
	double x_decinc; //decay incrementor per samp normalized to 1 
	double x_sus; //sustain percentage
	float x_relms; //release in samples
	double x_relinc; //release incrementor normalized to 1
	double x_lvl; //level
	int x_stage; //current stage, 0=none, 1 = attack, 2=decay, 3=sustain, 4 = release, 5=interrupt (release comes in middle of one of the stages not release
	int x_newatt; //new attack
	int x_newrel; //new release
	double x_curlvl; //current level
	t_inlet * x_alet;
	t_inlet * x_dlet;
	t_inlet * x_slet;
	t_inlet * x_rlet;
	t_inlet * x_llet;
	t_outlet * x_outlet;
} t_dkadsr_tilde;

static double ms_to_inc(t_dkadsr_tilde *x, float mstime){
	double srms = x->x_sr * 0.001f;
	double inc = 1.f/((double)mstime*(double)srms);
	return inc;
}

static void dkadsr_tilde_adsrer(t_dkadsr_tilde *x){
	int stage = x->x_stage;
	double curlvl = x->x_curlvl;
	double lvl = x->x_lvl;

	switch(stage){
		double attinc, decinc, relinc, sus;
		case 1:
			attinc = x->x_attinc * lvl;
			curlvl += attinc;
			if(curlvl > lvl){
				curlvl = lvl;
				x->x_stage = 2;
			};
			break;
		case 2:
			sus = x->x_sus;
			sus *= lvl;
			decinc = x->x_decinc*lvl;
			curlvl -= decinc;
			if(curlvl < sus){
				curlvl = sus;
				x->x_stage = 3;
			};
			break;
		case 3:
			break;
		case 4:
			relinc = x->x_relinc*lvl;
			curlvl -= relinc;
			if(curlvl < 0.f){
				curlvl = 0.f;
				x->x_stage = 0;
			};
			break;
		case 5:
			if(x->x_decms < x->x_relms){
				relinc = x->x_decinc*lvl;
			}
			else{
				relinc = x->x_relinc*lvl;
			};
			curlvl -= relinc;
			if(curlvl < 0){
				curlvl = 0.f;
				x->x_stage =0;
			};

	};
	x->x_curlvl = curlvl;

}

static t_int *dkadsr_tilde_perform(t_int *w){
	 t_dkadsr_tilde *x = (t_dkadsr_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
	int stage = x->x_stage;
	while(n--){
		if(x->x_newatt){
			//set in attack stage
			x->x_newatt = 0;
			x->x_stage = 1;
		}
		else if(x->x_newrel){
			x->x_newrel = 0;
			if(stage == 3 || stage == 4){
				//if in sustain or release stage
				x->x_stage = 4;
			}
			else if(stage == 0){
				//in passive mode, do nothing
			}
			else{
				//in attack or decay stage, go to interrupt stage
				x->x_stage = 5;
			};

		};
		dkadsr_tilde_adsrer(x);
		double output = x->x_curlvl;
		//output it!
		*out++ = output;
	};
	return(w+4);
}

static void dkadsr_tilde_float(t_dkadsr_tilde *x, t_float input){
	if(input <= 0.f){
		x->x_newrel = 1;
	}
	else{
	//input > 0
	x->x_lvl = input;//set lvl to input float
	x->x_newatt = 1; //set a new attack 
	}
}

static void dkadsr_tilde_attack(t_dkadsr_tilde *x, t_float attms){
	//given attack time in ms
	if(attms != x->x_attms){
		x->x_attms = attms;
	};
	x->x_attinc = ms_to_inc(x, attms);
}


static void dkadsr_tilde_decay(t_dkadsr_tilde *x, t_float decms){
	//given decay time in ms
	if(decms != x->x_decms){
		x->x_decms = decms;
	};
	double decinc = ms_to_inc(x, decms);
	double dist = 1 - x->x_sus;
	decinc *= dist;
	x->x_decinc = decinc;
}

static void dkadsr_tilde_release(t_dkadsr_tilde *x, t_float relms){
	//given release time in ms
	if(relms != x->x_relms){
		x->x_relms = relms;
	};
	double relinc = ms_to_inc(x, relms);
	relinc *= x->x_sus;
	x->x_relinc = relinc;
}

static void dkadsr_tilde_sustain(t_dkadsr_tilde *x, t_float sustain){
	float decms = x->x_decms;
	float relms = x->x_relms;
	float sus = x->x_sus;
	if(sustain != sus){

		if(sustain < 0.f){
			sustain = 0.f;
		}
		else if(sustain > 1.f){
			sustain = 1.f;
		}
		x->x_sus = sustain;
		dkadsr_tilde_release(x, relms);
		dkadsr_tilde_decay(x, decms);
	};
}

void dkadsr_tilde_level(t_dkadsr_tilde *x, t_float level){
	if(level < 0.f){
		level= 0.f;
	};
	x->x_lvl = level;
};

void *dkadsr_tilde_free(t_dkadsr_tilde *x){
	inlet_free(x->x_alet);
	inlet_free(x->x_dlet);
	inlet_free(x->x_slet);
	inlet_free(x->x_rlet);
	inlet_free(x->x_llet);
	outlet_free(x->x_outlet);
	return (void *)x;
}



static void *dkadsr_tilde_new(t_symbol *s, int argc, t_atom *argv){
	
	t_dkadsr_tilde *x = (t_dkadsr_tilde *)pd_new(dkadsr_tilde_class);

	x->x_lvl = ADSRL;
	x->x_newatt = 0;
	x->x_curlvl = 0.f;
	x->x_stage = 0;
	x->x_newrel = 0;
	switch(argc){
		float att, dec, sus, rel, lvl;
		case 4:
			att = atom_getfloatarg(0, argc, argv);
			dec = atom_getfloatarg(1, argc, argv);
			sus = atom_getfloatarg(2, argc, argv);
			rel = atom_getfloatarg(3, argc, argv);
			goto setting;
		case 5:
			lvl = atom_getfloatarg(0, argc, argv);
			if(lvl < 0.f){
				lvl =0.f;
			};
			x->x_lvl = lvl;
			att = atom_getfloatarg(1, argc, argv);
			dec = atom_getfloatarg(2, argc, argv);
			sus = atom_getfloatarg(3, argc, argv);
			rel = atom_getfloatarg(4, argc, argv);
			goto setting;
		default:
			goto errstate;
		setting:
			if(sus < 0.f){
				sus = 0.f;
			}
			else if(sus > 1.f){
				sus = 1.f;
			};
			x->x_sus = sus;
			dkadsr_tilde_attack(x,att);
			dkadsr_tilde_decay(x,dec);
			dkadsr_tilde_release(x,rel);
	};
	x->x_llet=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("level"));
	x->x_alet=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("attack"));
	x->x_dlet=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("decay"));
	x->x_slet=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("sustain"));
	x->x_rlet=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("release"));
	x->x_outlet=outlet_new(&x->x_obj, gensym("signal"));
	return (x);

errstate:
		pd_error(x, "dkadsr~: improper args");
		return NULL;
}


static void dkadsr_tilde_dsp(t_dkadsr_tilde *x, t_signal **sp){
	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(dkadsr_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
		int sr = sp[0]->s_sr;
		if(sr != x->x_sr){
			float attms = x->x_attms;
			float sus = x->x_sus;
			double decms = x->x_decms;
			double relms = x->x_relms;
			x->x_sr = sr;
			dkadsr_tilde_attack(x,attms);
			dkadsr_tilde_release(x,relms);
			dkadsr_tilde_decay(x,decms);
			dkadsr_tilde_sustain(x,sus);
		};

}


void dkadsr_tilde_setup(void){
	dkadsr_tilde_class = class_new(gensym("dkadsr~"), (t_newmethod)dkadsr_tilde_new, 0,
			sizeof(t_dkadsr_tilde), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(dkadsr_tilde_class, (t_method)dkadsr_tilde_dsp, gensym("dsp"), A_CANT, 0);
	class_addmethod(dkadsr_tilde_class, (t_method)dkadsr_tilde_attack, gensym("attack"), A_FLOAT, 0);
	class_addmethod(dkadsr_tilde_class, (t_method)dkadsr_tilde_decay, gensym("decay"), A_FLOAT, 0);
	class_addmethod(dkadsr_tilde_class, (t_method)dkadsr_tilde_sustain, gensym("sustain"), A_FLOAT, 0);
	class_addmethod(dkadsr_tilde_class, (t_method)dkadsr_tilde_release, gensym("release"), A_FLOAT, 0);
	class_addmethod(dkadsr_tilde_class, (t_method)dkadsr_tilde_level, gensym("level"), A_FLOAT, 0);
	class_addfloat(dkadsr_tilde_class, dkadsr_tilde_float);

}
