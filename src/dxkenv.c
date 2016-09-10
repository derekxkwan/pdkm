/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "dxkwavtab.h"
#include <stdlib.h>
#include <math.h>
#include "m_pd.h"
#include "string.h"

#define ENVTYPE PERC //default envelope
#define ENVSZ 8192 //default env size

typedef enum{
    PERC,
    LINEN,
    TRI,
    HANN
} envtype;

typedef enum{
SIN,
LIN,
WELCH,
EXP
} curvetype;

static t_class *dxkenv_tilde_class;

//dur = ms, sz = smp, sr/1000 = smp/ms
//x = sr/1000 * dur = smp traversed in dur ms
// sz/x = traverse these many samples in env to last dur ms
typedef struct _dxkenv_tilde {
	t_object x_obj;
	double x_phase;
        double x_st; //current state of env output
        double x_conv; // sz/(sr/1000 * dur)
        int x_sr;
        int x_sz; //envelope size in samps
        t_float x_lvl; //attack level
        int x_dur; //duration in ms
        double x_env[ENVSZ]; //envelope
        envtype x_type; //envelope type
        int x_act; //if envelope is in progress
        int x_redur; //retriggering release dur
        int x_irt; //interrupting current attack

        t_float x_in; //dummy
        t_inlet * x_paramlet;
        t_inlet * x_envlet;
} t_dxkenv_tilde;





static void buildperc(t_dxkenv_tilde * x, t_float attms, t_float relms, t_float curve){
    //IDEA: att and rel times are in ms, add together to get total time
    //find proportions of each rel to total, convert to samps
    int sz = x->x_sz;
    //bounds checking
    if(attms < 0){
        attms = 0;
    };
    if(relms < 0){
        relms = 0;
    };
    if(curve == 0){
        curve = -1;
    };
    t_float total = attms + relms;
    if(total){
        int attsamp = 0;
        int relsamp = 0;
        int cursamp = 0;
        if(attms){
            double attprop = (double)(attms/total);
            attsamp = (int)(attprop*(double)sz);
            relsamp = sz - attsamp;
        }
        else if(relms){
            relsamp = sz - attsamp;
        };
        if(curve <= -1){
            //normal percussion curve
            while(cursamp < attsamp){
                //amt goes from 1 to 0
                double amt = (double)(attsamp-cursamp)/(double)attsamp;
                //apply the curve
                if(curve <= -2){
                    amt = powf(amt, abs(curve));
                };
                //subtract from 1 to get the proper convex attack curve
                x->x_env[cursamp] = 1. - amt;
                cursamp++;

            };
            while(cursamp < sz){
                //release the rest of the way
                double amt = (double)(sz-cursamp-1)/(double)(sz-cursamp);
                if(curve <= -2){
                    amt = powf(amt, abs(curve));
                };
                x->x_env[cursamp] = amt;
                cursamp++;
            };
            
        }
        else{
            //reversed curve
            while(cursamp < relsamp){
                double amt = (double)cursamp/(double)(relsamp);
                if(curve >= 2){
                    amt = powf(amt, curve);
                };
                x->x_env[cursamp] = amt;
                cursamp++;
            };
            while(cursamp < sz){
                //attack the rest of the way
                double amt = (double)(cursamp-relsamp)/(double)(sz-relsamp-1);
                if(curve >= 2){
                    amt = powf(amt, curve);
                };
                x->x_env[cursamp] = 1. - amt;
                cursamp++;
            };
        };

    };

}

static void buildtri(t_dxkenv_tilde * x){
    int sz = x->x_sz;
    int mid = (int)(sz/2);
    int i;
    for(i=0; i< sz; i++){
        if(i<mid){
            x->x_env[i] = (double)(mid-i)/(double)mid;
        }
        else{
            x->x_env[i] = 1.-(double)(i-mid)/(double)(sz-mid);
        };
    };

}

static void buildsin(t_dxkenv_tilde *x){
    int sz = x->x_sz;
    double pi = 3.141592653589793238462643;
    int i;
    for(i=0;i<sz; i++){
        x->x_env[i] = 0.5*(1.-cos((2.*pi*i)/(sz-1)));
    };
}

static void buildlinen(t_dxkenv_tilde *x, t_float attms, t_float susms, t_float relms, curvetype curve){
    //IDEA: att,rel,sus times are in ms, add together to get total time
    //find proportions of each rel to total, convert to samps
    
    int sz = x->x_sz;
    double pi = 3.141592653589793238462643;
    if(attms < 0){
        attms = 0;
    };
    if(relms < 0){
        relms = 0;
    };
    if(susms < 0){
        relms = 0;
    };
    t_float total = attms + susms + relms;
    if(total){
        int attsamp=0; double as1 = 0;
        int sussamp = 0;
        int relsamp = 0;
        int cursamp = 0;
        if(attms){
            double attprop = (double)(attms/total);
            attsamp = (int)(attprop * sz);
            as1 = (double)attsamp-1.;
            if(as1 == 0){
                as1 = 1.;
            };
        };
        if(susms){
            double susprop = (double)(susms/total);
            sussamp = (int)(susprop * sz);
            relsamp = sz - (attsamp + sussamp);
            if(relsamp < 0){
                relsamp = 0;
            };
        }
        else if(relms){
            double relprop = (double)(relms/total);
            relsamp = (int)(relprop*sz);
        };
        while(cursamp < attsamp){
            double output;
            double idx = (double)(attsamp-cursamp)/(double)attsamp;
            switch(curve){
                case SIN:
                    output = sin(idx*0.25*pi);
                    break;
                case LIN:
                    output = idx;
                    break;
                case WELCH:
                    output = 1. - powf((idx*0.5-(as1/2.))/(as1/2.), 2.);
                    break;
                case EXP:
                    output = powf(idx, 2.718);
                default:
                    break;
            };
            x->x_env[cursamp] = output;
            cursamp++;

        };
        while(cursamp < (sz - relsamp)){

            x->x_env[cursamp] = 1.;
            cursamp++;
        };
        while(cursamp < sz){
            double output;
            double idx = (double)((sz-1)-cursamp)/(double)(sz - relsamp);
             switch(curve){
                case SIN:
                    output = sin(idx*0.25*pi);
                    break;
                case LIN:
                    output = idx;
                    break;
                case WELCH:
                    output = 1. - powf((idx*0.5-(as1/2.))/(as1/2.), 2.);
                    break;
                case EXP:
                    output = powf(idx, 2.718);
                default:
                    break;
            }; 
            x->x_env[cursamp] = output;
            cursamp++;
        };
    };

}

static void dxkenv_list(t_dxkenv_tilde *x, int argc, t_atom * argv){

        if(x->x_type == PERC){
            t_float att = 10;
            t_float rel = 1000;
            t_float curve = -4;
            t_float level = 1.;
            int curarg = 0;
            while(curarg < argc && curarg < 4){
                t_float curf = atom_getfloatarg(curarg, argc, argv);
                switch(curarg){
                    case 0: 
                        att = curf;
                        break;
                    case 1:
                        rel = curf;
                        break;
                    case 2:
                        level = curf;
                        break;
                    case 3:
                        curve = curf;
                        break;
                    default:
                        break;
                };
                curarg++;
            };

                x->x_lvl = level;
                x->x_dur = att+rel;
                buildperc(x,att,rel,curve);
        }
        else if(x->x_type == LINEN){
            t_float att = 10;
            t_float sus = 1000;
            t_float rel = 1000;
            t_float level = 1.;
            curvetype curve = LIN;

            int curarg = 0;
            while(curarg < 4 && curarg < argc){
                t_float curf = atom_getfloatarg(curarg, argc, argv);
                switch(curarg){
                    case 0: 
                        att = curf;
                        break;
                    case 1:
                        sus = curf;
                        break;
                    case 2:
                        rel = curf;
                        break;
                    case 3: 
                        level = curf;
                        break;
                    default:
                        break;
                };
                curarg++;
            };
            if(curarg == 4 && argc >= 5){
                if((argv+curarg)->a_type == A_SYMBOL){
                    t_symbol * curs = atom_getsymbolarg(curarg, argc, argv);
                    if(strcmp(curs->s_name, "sine")==0){
                        curve = SIN;
                    }
                    else if(strcmp(curs->s_name, "lin") == 0){
                        curve = LIN;
                    }
                    else if(strcmp(curs->s_name, "welch") == 0){
                        curve = WELCH;
                    }
                    else if(strcmp(curs->s_name, "exp") == 0){
                        curve = EXP;
                    };
                };
            };

            x->x_lvl = level;
            x->x_dur = att+sus+rel;
            buildlinen(x,att,sus,rel,curve);

        }
        else if (x->x_type == HANN || x->x_type == TRI){
            t_float dur = 1000;
            t_float level = 1.;

            int curarg = 0;
            while(curarg < argc && curarg < 2){
                t_float curf = atom_getfloatarg(curarg, argc, argv);
                switch(curarg){
                    case 0:
                        dur = curf;
                        break;
                    case 1:
                        level = curf;
                        break;
                    default:
                        break;
                };
                
                curarg++;    
            };
            x->x_dur = dur;
            x->x_lvl = level;
            if(x->x_type == HANN){
                buildsin(x);
            }
            else{
                buildtri(x);
            };
        };
        if(x->x_act){
            x->x_irt = 1;
        };
        x->x_act = 0;
        x->x_phase = 0;
}

static void dxkenv_env(t_dxkenv_tilde * x, t_symbol * s){
                if(strcmp(s->s_name, "linen")==0){
                    x->x_type = LINEN;
                }
                else if(strcmp(s->s_name, "sine")==0){
                    x->x_type = HANN;
                }
                else if (strcmp(s->s_name, "tri")==0){
                    x->x_type = TRI;

                
                }
                else{
                    x->x_type = PERC;
                };
        dxkenv_list(x, 0, 0);
}

void *dxkenv_tilde_new(t_symbol *s, int argc, t_atom *argv){
	int i;
	t_dxkenv_tilde *x = (t_dxkenv_tilde *)pd_new(dxkenv_tilde_class);
	t_atom params[10];
	x->x_sr = sys_getsr();
        x->x_sz = ENVSZ;
        x->x_type = PERC;
        x->x_redur = 1;        	
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


        dxkenv_list(x, argc, argv);

	x->x_envlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_symbol, gensym("env"));
	x->x_paramlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("param"));


	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
    errstate:
        post("dxkenv~: improper args");
        return NULL;
}



static t_int *dxkenv_tilde_perform(t_int *w){
	 t_dxkenv_tilde *x = (t_dxkenv_tilde *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);
        int i;
	for(i=0; i<n; i++){
            double output;
           if(x->x_act){
                double phs = x->x_phase;
                output = dxkgetlin(x->x_env, x->x_sz, phs);
                output *= x->x_lvl;
                x->x_phase = phs + x->x_conv;
                if(x->x_phase >= x->x_sz){
                    //if env done
                    x->x_phase = 0;
                    x->x_act = 0;
                };
           }
            else if(x->x_irt){
                output = x->x_st;
                int pos = output >= 0;
                if(pos){
                    output -= x->x_lvl/(double)(x->x_sr*0.001*x->x_redur);
                    if(output < 0){
                        //if ramp finished
                        output = 0;
                        x->x_irt = 0;
                        x->x_act = 1;
                        x->x_phase = 0;
                    };
                }
                else{
                    output += x->x_lvl/(double)(x->x_sr*0.001*x->x_redur);
                    if(output > 0){
                        //if ramp finished
                        output = 0;
                        x->x_irt = 0;
                        x->x_act = 1;
                        x->x_phase = 0;
                    };
                };
            }
            else{
                output = 0;
                };

                x->x_st = output;
                out[i] = output;
	};
	return(w+4);
}

static void dxkenv_tilde_dsp(t_dxkenv_tilde *x, t_signal **sp){
	//(freq*tablelen)/(samplerate) = array values per sample to advance
	// divide by tablelen to map to 0 to 1 range,..freq/samplerate
	    dsp_add(dxkenv_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
		int sr = sp[0]->s_sr;
                if(sr != x->x_sr){
                    x->x_conv = (double)(x->x_sz)/(double)((double)sr*0.001*x->x_dur);
                    x->x_sr = sr;
                };

}

static void *dxkenv_tilde_free(t_dxkenv_tilde *x)
{
	inlet_free(x->x_paramlet);
	inlet_free(x->x_envlet);
	return (void *)x;
}



static void dxkenv_tilde_bang(t_dxkenv_tilde *x){
    if(x->x_act){
        //if currently active, trigger release then new attack
        x->x_act = 0;
        x->x_irt = 1;
        x->x_phase = 0;
    }
    else if(x->x_act == 0 && x->x_irt == 0){
        //if not interrupting and not active ,new attack (if interrupting, do nothing)
        x->x_act = 1;
        x->x_phase = 0;
        x->x_conv = (double)(x->x_sz)/(double)((double)x->x_sr*0.001*x->x_dur);
    };
}

void dxkenv_tilde_setup(void){
	dxkenv_tilde_class = class_new(gensym("dxkenv~"), (t_newmethod)dxkenv_tilde_new, (t_method)dxkenv_tilde_free,
			sizeof(t_dxkenv_tilde), 0, A_GIMME, 0);
	class_addmethod(dxkenv_tilde_class, (t_method)dxkenv_tilde_dsp, gensym("dsp"), A_CANT, 0);
   class_addmethod(dxkenv_tilde_class, (t_method)dxkenv_list, gensym("params"), A_GIMME, 0);
   class_addmethod(dxkenv_tilde_class, (t_method)dxkenv_env, gensym("env"), A_SYMBOL, 0);
    class_addbang(dxkenv_tilde_class, dxkenv_tilde_bang);
   CLASS_MAINSIGNALIN(dxkenv_tilde_class, t_dxkenv_tilde, x_in);
}
