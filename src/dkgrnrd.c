/* Copyright 2016 - Derek Kwan
 *  Distributed under GPL v3 */

#include "dkgrnrd.h"

//adding on to grainyrd's code from pure data's src/d_array.c
static t_class *dkgrnrd_tilde_class;

typedef struct _dkgrnrd_tilde
{
    t_object x_obj;
    t_dkbuf *x_dkbuf;//buffer helper struct
    t_dkrnd *x_dkrnd;//rand helper struct
    t_float x_f;
	//attributes of grains 0-numgrains
	int x_grainsize[DKGNYNUMGR]; //grainsize in samps
	int x_grainstart[DKGNYNUMGR]; //start position of each grain
	double x_grainstep[DKGNYNUMGR]; //relative step size per input step (for transposition,1 in: 2 out = octave higher)
	int x_grainrsmp; //how many samples before the start of each new grain
	int x_grainrms; //how many ms before the start of each new grain
	double x_grainamp[DKGNYNUMGR]; //amp of each grain
	int x_overlap; //how many grains overlap
	int x_actgn[DKGNYNUMGR]; //active grain indices, should be calloced to size x_overlap * sizeof(int)
	double x_grainpos[DKGNYNUMGR]; //position within the grain
	int x_freegr[DKGNYNUMGR]; //indexes of free grains

	float x_win[DKGNYM]; //window  (shape of each grain)
	double x_winmap[DKGNYNUMGR]; //stepsize in window, winsize/grainsize
	double x_winpos[DKGNYNUMGR]; //position within each window
	int x_env; //envelope type
	
	int x_nextgr; //samps until next grain
	t_float x_trans; //current transposition
	int x_randsz; //generate random sizes; 1 = yes
	float x_rszamt; //random size amount
	double m_sr; //sample rate in milliseconds
	float x_amp; //amplitude
	float x_rampamt; //random amp amount
	int x_randamp; //generate random amps; 1 = yes
	int x_randtp; //generate random transpositions, 1 = yes
	double x_rtpamt; //random transposition amount
	float x_posvar; //amount of position variance per grain in ms
	float x_hopvar; //amount of position variance per grain in ms

	t_float x_offset; //offset
	float x_init; //init step completed	
	int x_numact; //number of active grains

	t_outlet *x_outlet;
} t_dkgrnrd_tilde;


static double getreadscale(t_dkgrnrd_tilde *x, float transpose){
	double dbltp = (double) transpose;
	if(x->x_randtp){
		dbltp += ((double)x->x_rtpamt * ((dkrnd_next(x->x_dkrnd)*2.f)-1.f));
	};
	return pow(2.f, dbltp/12.f);

}

static void freegr(t_dkgrnrd_tilde *x, int actidx){
	//free specified index in x_actgn
	int i;
	int freeidx = x->x_actgn[actidx]; //actual grain index

	for(i=actidx; i<DKGNYNUMGR-1; i++){//overwrite grain by scooting everything down an idx
			x->x_actgn[i] = x->x_actgn[i+1];
	};
	x->x_actgn[DKGNYNUMGR-1] = DKGNYNUMGR; //set last active grain to inactive 
	for(i=DKGNYNUMGR-1; i > 0; i--){
		x->x_freegr[i] = x->x_freegr[i-1]; //make room for new freegr by moving everything 1 index up
	};

	x->x_freegr[0] = freeidx;
}

static int getnextgr(t_dkgrnrd_tilde *x){
	int i;
	int nextgridx = x->x_freegr[0]; //take first free grain
	for(i=0;i< DKGNYNUMGR-1; i++){
		x->x_freegr[i] = x->x_freegr[i+1]; //shift free grains down one idx
	};
	x->x_freegr[DKGNYNUMGR-1] = DKGNYNUMGR; // add unfree grain marker to end of free grains
	for(i=0;i<DKGNYNUMGR; i++){
		if(x->x_actgn[i] == DKGNYNUMGR){//check if flagged freed
			x->x_actgn[i] = nextgridx; //add new grain idx
			break;
		};
	};
	return nextgridx;
}

static void dkgrnrd_tilde_grainrsmp(t_dkgrnrd_tilde *x, t_float grms){
	if(grms != x->x_grainrms){
		x->x_grainrms = grms;
	};
	if(grms < 3){
		grms = 3;
		};
	x->x_grainrsmp = (int)(x->m_sr * grms);
	x->x_nextgr = 0;
}

static void dkgrnrd_tilde_overlap(t_dkgrnrd_tilde *x, t_float overlap){
	int overlapint;
	if(overlap <= 1.){
		overlapint = 1;
		}
	else if(overlap >= 64.){
			overlapint = 64;
	}
	else{
		overlapint = (int)overlap;
	};
	x->x_overlap = overlapint;
}

static void dkgrnrd_tilde_randsize(t_dkgrnrd_tilde *x, t_float randsize){
	int randsizeint;
	if(randsize <= 0.){
		randsizeint = 0;
		}
	else{
		randsizeint= 1;
	};
	x->x_randsz = randsizeint;
}


static void dkgrnrd_tilde_randsizeamt(t_dkgrnrd_tilde *x, t_float randsizeamt){
	if(randsizeamt <= 0.){
		randsizeamt = 0.;
		};
	x->x_rszamt = randsizeamt;
}


static void dkgrnrd_tilde_randamp(t_dkgrnrd_tilde *x, t_float randamp){
	int randampint;
	if(randamp <= 0.){
		randampint = 0;
		}
	else{
		randampint= 1;
	};
	x->x_randamp = randampint;
}


static void dkgrnrd_tilde_randampamt(t_dkgrnrd_tilde *x, t_float randampamt){
	if(randampamt <= 0.){
		randampamt = 0.;
		};
	x->x_rampamt = randampamt;
}


static void dkgrnrd_tilde_randtp(t_dkgrnrd_tilde *x, t_float randtp){
	int randtpint;
	if(randtp <= 0.){
		randtpint = 0;
		}
	else{
		randtpint= 1;
	};
	x->x_randtp = randtpint;
}


static void dkgrnrd_tilde_randtpamt(t_dkgrnrd_tilde *x, t_float randtpamt){
	if(randtpamt <= 0.){
		randtpamt = 0.;
		};
	x->x_rtpamt = randtpamt;
}


static void dkgrnrd_tilde_posvar(t_dkgrnrd_tilde *x, t_float posvar){
	if(posvar <= 0.){
		posvar = 0.;
		};
	x->x_posvar = posvar;
}

static void dkgrnrd_tilde_hopvar(t_dkgrnrd_tilde *x, t_float hopvar){
	if(hopvar <= 0.){
		hopvar = 0.;
		};
	x->x_hopvar = hopvar;
}
static void dkgrnrd_tilde_makeenv(t_dkgrnrd_tilde *x, int envtype){
		int i;
		switch(envtype){//make envelopes
			case 0:
				for(i=0; i<DKGNYM; i++){//hanning
					double xval = (double)i/(double)DKGNYM;
					double winval;
					winval = 0.5*(1-cos(DKGNY2PI*xval));
					x->x_win[i] = winval;
				};
				break;
			case 1:
				for(i=0; i<DKGNYM; i++){//tukey
					double xval = (double)i/(double)DKGNYM;
					double winval;
					if(xval < (DKGNYR/2.f)){
						winval =0.5*(1+cos((DKGNY2PI/DKGNYR)*(xval-(DKGNYR/2.f))));	
					}
					else if(xval > (DKGNYR/2.f) && xval < (1-(DKGNYR/2.f))){
						winval = 1.;
					}
					else{
						winval =0.5*(1+cos((DKGNY2PI/DKGNYR)*(xval-1.+(DKGNYR/2.f))));	
					};
					x->x_win[i] = winval;

				};
				break;
			};



};

static void dkgrnrd_tilde_envtype(t_dkgrnrd_tilde *x, t_symbol *env){
	int envtype;
	if(strcmp(env->s_name, "hanning") == 0){
		envtype = 0;
	}
	else{
		envtype = 1; //tukey
	};

	if(envtype != x->x_env && x->x_init == 1){
		dkgrnrd_tilde_makeenv(x, envtype);
	};
	x->x_env = envtype; //store what type it is in struct
}


static void *dkgrnrd_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;
    t_dkgrnrd_tilde *x = (t_dkgrnrd_tilde *)pd_new(dkgrnrd_tilde_class);
    t_symbol * bufname = NULL;

    //some init/default stuff
	x->x_init=0;//we haven't init yet
        float sr = sys_getsr(); //sample rate in seconds
        sr *= .001f; //sample rate in ms
        floatinlet_new(&x->x_obj, &x->x_trans);
        floatinlet_new(&x->x_obj, &x->x_offset);
        x->x_outlet = outlet_new(&x->x_obj, gensym("signal"));
        x->x_f = 0;
        x->m_sr = (double) sr; //samples per millisecond
        x->x_overlap = DKGNYOV;
        x->x_randsz = DKGNYRNDSZ;
        x->x_rszamt = DKGNYRSZAMT;
        x->x_randamp = DKGNYRNDAMP;
        x->x_rampamt = DKGNYRAMPAMT;
        x->x_randtp = DKGNYRNDTP;
        x->x_rtpamt = DKGNYRTPAMT;
        x->x_grainrsmp = (int)(DKGNYGRATEMS * x->m_sr);
        x->x_amp = DKGNYAMP;
        x->x_trans = DKGNYTP;
        x->x_posvar = DKGNYPOSVAR;
        x->x_hopvar = DKGNYHOPVAR;
        x->x_offset = 0;
        x->x_env = DKGNYENV;
        x->x_numact = 0;

        //parse bufname first if exists
        if(argc){
		if(argv -> a_type == A_SYMBOL){
		    bufname = atom_getsymbolarg(0, argc, argv);
		    argc--;
		    argv++;
                };
        };

		while(argc > 0){
			t_symbol *arg0 = atom_getsymbolarg(0, argc, argv);
			t_symbol *arg1 = atom_getsymbolarg(1, argc, argv);
			if(strcmp(arg0->s_name, "-grainrate") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float grms = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_grainrsmp(x, grms);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-overlap") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float overlap = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_overlap(x, overlap);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randsize") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float randsize = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_randsize(x, randsize);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randsizeamt") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float randsizeamt = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_randsizeamt(x, randsizeamt);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randamp") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float randamp = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_randamp(x, randamp);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randampamt") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float randampamt = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_randampamt(x, randampamt);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randtp") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float randtp = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_randtp(x, randtp);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randtpamt") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float randtpamt = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_randtpamt(x, randtpamt);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-posvar") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float posvar = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_posvar(x, posvar);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-hopvar") == 0){
				if((argv+1) -> a_type == A_FLOAT){
					float hopvar = atom_getfloatarg(1, argc, argv);
					dkgrnrd_tilde_hopvar(x, hopvar);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-env") == 0){
				if((argv+1) -> a_type == A_SYMBOL){
					dkgrnrd_tilde_envtype(x, arg1);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else{
				goto errstate;
			};
		};

		for(i=0; i<DKGNYNUMGR; i++){//init all matrices
			x->x_grainsize[i] = 0; 
			x->x_grainstart[i] = 0;
			x->x_grainstep[i] = 0.f;
			x->x_grainamp[i] = 0.; 
			x->x_grainpos[i] = 0.f;
			x->x_freegr[i] = i; //indexes of free grains
			x->x_actgn[i] = DKGNYNUMGR;

			x->x_winmap[i] = 0.f;
			x->x_winpos[i] = 0.f; 
		};
		dkgrnrd_tilde_makeenv(x, x->x_env);

                x->x_dkbuf = dkbuf_init((t_class *)x,bufname);
                x->x_dkrnd = dkrnd_new(0);
		x->x_init = 1;
    return (x);
	errstate:
		pd_error(x, "dkgrnrd~: improper args");
		return NULL;
}

static t_int *dkgrnrd_tilde_perform(t_int *w)
{
    t_dkgrnrd_tilde *x = (t_dkgrnrd_tilde *)(w[1]);
    t_dkbuf *b = x->x_dkbuf;
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    int maxindex;
    t_word *buf = b->b_vec;
    int i, j;
	int grainrsmp = x->x_grainrsmp;
	int overlap = x->x_overlap;
	int grainsz = grainrsmp*overlap; //grainsize determined by grainrsmp * overlap
	int randsz = x->x_randsz; //random sizes
	int randamp = x->x_randamp;
	double curtp = x->x_trans; //current transposition
	double m_sr = x->m_sr;
	double rszamt = x->x_rszamt;
	double rampamt = x->x_rampamt;
	double amp = x->x_amp;
	double posvar = x->x_posvar;
	double hopvar = x->x_hopvar; 
	int offset = x->x_offset;
    
	maxindex = b->b_npts - 1;
    if(maxindex<0) goto zero;
    if (!buf) goto zero;

    for (i = 0; i < n; i++)
    {
		//tabreading stuff
        int index = *in++;
		index += offset;
        if (index < 0)
            index = 0;
        else if (index > maxindex)
            index = maxindex;

		double output = 0.f;	
		if(x->x_nextgr == 0){
			//init new grain
			int cursz = grainsz;
			if(randsz){
				cursz += (int)(dkrnd_next(x->x_dkrnd)*((double)m_sr*rszamt));
			};
			int curidx = getnextgr(x); //getting next grain idx
			x->x_grainsize[curidx] = cursz;
			x->x_grainpos[curidx] = 0;
			int curpos = index  + (int)((2.f*dkrnd_next(x->x_dkrnd)-1.f)*(m_sr*posvar));
			if(curpos < 0){
				curpos = 0;
			}
			else if(curpos > maxindex){
				curpos = maxindex;
			};
			x->x_grainstart[curidx] = curpos;
			x->x_grainstep[curidx] = getreadscale(x, curtp);
			x->x_winmap[curidx] = (double)DKGNYM/(double)cursz;
			x->x_winpos[curidx] = 0.f;
			x->x_nextgr = grainrsmp + (int)(dkrnd_next(x->x_dkrnd)*((double)grainrsmp*hopvar));
			//post("%d", x->x_nextgr);
			x->x_grainamp[curidx] = amp;
			if(randamp){
				x->x_grainamp[curidx] += (rampamt * (2.f*dkrnd_next(x->x_dkrnd)-1.f));
			};
			x->x_numact++;

		};
		int graincount = 0;
		for(j=0; j<DKGNYNUMGR; j++){
			//add active grain check
				int grnidx = x->x_actgn[j];
				if(grnidx < DKGNYNUMGR){
					graincount++;
					double grnpos = x->x_grainpos[grnidx];
					int grnst = x->x_grainstart[grnidx];
					int curidx = (int)(grnst + grnpos);
					double grnamp = x->x_grainamp[grnidx];
					if(curidx > maxindex){
						curidx = maxindex;
					}
					else if(curidx < 0){
						curidx = 0;
					};
					double curout = buf[curidx].w_float*grnamp;//get current output for each grain times grain's amp;
					int winidx = (int)x->x_winpos[grnidx];
					if(winidx> DKGNYM - 1){
						winidx = DKGNYM -1;
					}
					else if(winidx < 0){
						winidx = 0;
					};
					curout *= x->x_win[winidx]; //window signal
					output += curout; //add grain output to total output

					//increment everything
					x->x_winpos[grnidx] += x->x_winmap[grnidx];
					x->x_grainpos[grnidx] += x->x_grainstep[grnidx];
					if(x->x_grainpos[grnidx] > x->x_grainsize[grnidx]){
						freegr(x, j);
						x->x_numact--;
					};
					if(graincount >= x->x_numact){
						break;
					};
				};
		};
	//	output *= (1.f/(double) graincount);
      	output *= (1.f/(double) overlap);
		*out++ = output;
		x->x_nextgr--;
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

static void dkgrnrd_tilde_set(t_dkgrnrd_tilde *x, t_symbol *s)
{
    dkbuf_setarray(x->x_dkbuf, s);
}

static void dkgrnrd_tilde_dsp(t_dkgrnrd_tilde *x, t_signal **sp)
{
    dkbuf_checkdsp(x->x_dkbuf);
	x->x_nextgr = 0;
	
    dsp_add(dkgrnrd_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

	float newsr = sp[0]->s_sr * .001f;
	if(x->m_sr != newsr){
		x->m_sr = newsr; //sample rate in ms
		dkgrnrd_tilde_grainrsmp(x, x->x_grainrms);
	};

}

static void *dkgrnrd_tilde_free(t_dkgrnrd_tilde *x)
{
        dkbuf_free(x->x_dkbuf);
        dkrnd_free(x->x_dkrnd);
	outlet_free(x->x_outlet);
	return (void *)x;
}

void dkgrnrd_tilde_setup(void)
{
    dkgrnrd_tilde_class = class_new(gensym("dkgrnrd~"),
        (t_newmethod)dkgrnrd_tilde_new, (t_method)dkgrnrd_tilde_free,
        sizeof(t_dkgrnrd_tilde), CLASS_DEFAULT, A_GIMME, 0);
    CLASS_MAINSIGNALIN(dkgrnrd_tilde_class, t_dkgrnrd_tilde, x_f);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_dsp,
        gensym("dsp"), A_CANT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_set,
        gensym("set"), A_SYMBOL, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_overlap,
        gensym("overlap"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_randsize,
        gensym("randsize"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_randsizeamt,
        gensym("randsizeamt"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_randtp,
        gensym("randtp"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_randtpamt,
        gensym("randtpamt"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_randamp,
        gensym("randamp"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_randampamt,
        gensym("randampamt"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_grainrsmp,
        gensym("grainrate"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_posvar,
        gensym("posvar"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_hopvar,
        gensym("hopvar"), A_FLOAT, 0);
    class_addmethod(dkgrnrd_tilde_class, (t_method)dkgrnrd_tilde_envtype,
        gensym("env"), A_SYMBOL, 0);
}
