#include "grainyrd~.h"

//adding on to grainyrd's code from pure data's src/d_array.c
static t_class *grainyrd_tilde_class;

typedef struct _grainyrd_tilde
{
    t_object x_obj;
	//tabread stuff
    int x_npoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
	//attributes of grains 0-numgrains
	int x_grainsize[DXKGNYNUMGR]; //grainsize in samps
	int x_grainstart[DXKGNYNUMGR]; //start position of each grain
	double x_grainstep[DXKGNYNUMGR]; //relative step size per input step (for transposition,1 in: 2 out = octave higher)
	int x_grainrate; //how many samples before the start of each new grain
	double x_grainamp[DXKGNYNUMGR]; //amp of each grain
	int x_overlap; //how many grains overlap
	int x_actgn[DXKGNYNUMGR]; //active grain indices, should be calloced to size x_overlap * sizeof(int)
	double x_grainpos[DXKGNYNUMGR]; //position within the grain
	int x_freegr[DXKGNYNUMGR]; //indexes of free grains

	float x_win[DXKGNYM]; //window  (shape of each grain)
	double x_winmap[DXKGNYNUMGR]; //stepsize in window, winsize/grainsize
	double x_winpos[DXKGNYNUMGR]; //position within each window
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
} t_grainyrd_tilde;

static double exprseed(void){//range 0 to 1
	double retval;
	static unsigned long int exprandseed = 1997333137;
	exprandseed = exprandseed * 2891336453 + 1500450271;
	exprandseed = exprandseed % 4294967296;
	exprandseed &= 0x7fffffff;
	retval = exprandseed * (1.f/2147483647.f); //biggest num in 32-bit div 2
	return retval;
}


static double getreadscale(t_grainyrd_tilde *x, float transpose){
	double dbltp = (double) transpose;
	if(x->x_randtp){
		dbltp += ((double)x->x_rtpamt * ((exprseed()*2.f)-1.f));
	};
	return pow(2.f, dbltp/12.f);

}

static void freegr(t_grainyrd_tilde *x, int actidx){
	//free specified index in x_actgn
	int i;
	int freeidx = x->x_actgn[actidx]; //actual grain index

	for(i=actidx; i<DXKGNYNUMGR-1; i++){//overwrite grain by scooting everything down an idx
			x->x_actgn[i] = x->x_actgn[i+1];
	};
	x->x_actgn[DXKGNYNUMGR-1] = DXKGNYNUMGR; //set last active grain to inactive 
	for(i=DXKGNYNUMGR-1; i > 0; i--){
		x->x_freegr[i] = x->x_freegr[i-1]; //make room for new freegr by moving everything 1 index up
	};

	x->x_freegr[0] = freeidx;
}

static int getnextgr(t_grainyrd_tilde *x){
	int i;
	int nextgridx = x->x_freegr[0]; //take first free grain
	for(i=0;i< DXKGNYNUMGR-1; i++){
		x->x_freegr[i] = x->x_freegr[i+1]; //shift free grains down one idx
	};
	x->x_freegr[DXKGNYNUMGR-1] = DXKGNYNUMGR; // add unfree grain marker to end of free grains
	for(i=0;i<DXKGNYNUMGR; i++){
		if(x->x_actgn[i] == DXKGNYNUMGR){//check if flagged freed
			x->x_actgn[i] = nextgridx; //add new grain idx
			break;
		};
	};
	return nextgridx;
}

static void grainyrd_tilde_grainrate(t_grainyrd_tilde *x, t_float grms){
	if(grms < 3){
		grms = 3;
		};
	x->x_grainrate = (int)(x->m_sr * grms);
	x->x_nextgr = 0;
}

static void grainyrd_tilde_overlap(t_grainyrd_tilde *x, t_float overlap){
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

static void grainyrd_tilde_randsize(t_grainyrd_tilde *x, t_float randsize){
	int randsizeint;
	if(randsize <= 0.){
		randsizeint = 0;
		}
	else{
		randsizeint= 1;
	};
	x->x_randsz = randsizeint;
}


static void grainyrd_tilde_randsizeamt(t_grainyrd_tilde *x, t_float randsizeamt){
	if(randsizeamt <= 0.){
		randsizeamt = 0.;
		};
	x->x_rszamt = randsizeamt;
}


static void grainyrd_tilde_randamp(t_grainyrd_tilde *x, t_float randamp){
	int randampint;
	if(randamp <= 0.){
		randampint = 0;
		}
	else{
		randampint= 1;
	};
	x->x_randamp = randampint;
}


static void grainyrd_tilde_randampamt(t_grainyrd_tilde *x, t_float randampamt){
	if(randampamt <= 0.){
		randampamt = 0.;
		};
	x->x_rampamt = randampamt;
}


static void grainyrd_tilde_randtp(t_grainyrd_tilde *x, t_float randtp){
	int randtpint;
	if(randtp <= 0.){
		randtpint = 0;
		}
	else{
		randtpint= 1;
	};
	x->x_randtp = randtpint;
}


static void grainyrd_tilde_randtpamt(t_grainyrd_tilde *x, t_float randtpamt){
	if(randtpamt <= 0.){
		randtpamt = 0.;
		};
	x->x_rtpamt = randtpamt;
}


static void grainyrd_tilde_posvar(t_grainyrd_tilde *x, t_float posvar){
	if(posvar <= 0.){
		posvar = 0.;
		};
	x->x_posvar = posvar;
}

static void grainyrd_tilde_hopvar(t_grainyrd_tilde *x, t_float hopvar){
	if(hopvar <= 0.){
		hopvar = 0.;
		};
	x->x_hopvar = hopvar;
}
static void grainyrd_tilde_makeenv(t_grainyrd_tilde *x, int envtype){
		int i;
		switch(envtype){//make envelopes
			case 0:
				for(i=0; i<DXKGNYM; i++){//hanning
					double xval = (double)i/(double)DXKGNYM;
					double winval;
					winval = 0.5*(1-cos(DXKGNY2PI*xval));
					x->x_win[i] = winval;
				};
				break;
			case 1:
				for(i=0; i<DXKGNYM; i++){//tukey
					double xval = (double)i/(double)DXKGNYM;
					double winval;
					if(xval < (DXKGNYR/2.f)){
						winval =0.5*(1+cos((DXKGNY2PI/DXKGNYR)*(xval-(DXKGNYR/2.f))));	
					}
					else if(xval > (DXKGNYR/2.f) && xval < (1-(DXKGNYR/2.f))){
						winval = 1.;
					}
					else{
						winval =0.5*(1+cos((DXKGNY2PI/DXKGNYR)*(xval-1.+(DXKGNYR/2.f))));	
					};
					x->x_win[i] = winval;

				};
				break;
			};



};

static void grainyrd_tilde_envtype(t_grainyrd_tilde *x, t_symbol *env){
	int envtype;
	if(strcmp(env->s_name, "hanning") == 0){
		envtype = 0;
	}
	else{
		envtype = 1; //tukey
	};

	if(envtype != x->x_env && x->x_init == 1){
		grainyrd_tilde_makeenv(x, envtype);
	};
	x->x_env = envtype; //store what type it is in struct
}


static void *grainyrd_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;
    t_grainyrd_tilde *x = (t_grainyrd_tilde *)pd_new(grainyrd_tilde_class);
	x->x_init=0;
	if(argc == 0){
		goto errstate;
	}
	else{
		float sr = sys_getsr(); //sample rate in seconds
		sr *= .001f; //sample rate in ms
		x->x_vec = 0;
		floatinlet_new(&x->x_obj, &x->x_trans);
		floatinlet_new(&x->x_obj, &x->x_offset);
		outlet_new(&x->x_obj, gensym("signal"));
		x->x_f = 0;
		x->m_sr = (double) sr; //samples per millisecond
		t_symbol *name = atom_getsymbolarg(0, argc, argv);
		if(name != &s_){
			x->x_arrayname = name;
			argc--;
			argv++;
			x->x_overlap = DXKGNYOV;
			x->x_randsz = DXKGNYRNDSZ;
			x->x_rszamt = DXKGNYRSZAMT;
			x->x_randamp = DXKGNYRNDAMP;
			x->x_rampamt = DXKGNYRAMPAMT;
			x->x_randtp = DXKGNYRNDTP;
			x->x_rtpamt = DXKGNYRTPAMT;
			x->x_grainrate = (int)(DXKGNYGRATEMS * x->m_sr);
			x->x_amp = DXKGNYAMP;
			x->x_trans = DXKGNYTP;
			x->x_posvar = DXKGNYPOSVAR;
			x->x_hopvar = DXKGNYHOPVAR;
			x->x_offset = 0;
			x->x_env = DXKGNYENV;
			x->x_numact = 0;
		}
		else{
			goto errstate;
		};
		while(argc > 0){
			t_symbol *arg0 = atom_getsymbolarg(0, argc, argv);
			t_symbol *arg1 = atom_getsymbolarg(1, argc, argv);
			if(strcmp(arg0->s_name, "-grainrate") == 0){
				if(arg1 == &s_){
					float grms = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_grainrate(x, grms);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-overlap") == 0){
				if(arg1 == &s_){
					float overlap = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_overlap(x, overlap);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randsize") == 0){
				if(arg1 == &s_){
					float randsize = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_randsize(x, randsize);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randsizeamt") == 0){
				if(arg1 == &s_){
					float randsizeamt = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_randsizeamt(x, randsizeamt);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randamp") == 0){
				if(arg1 == &s_){
					float randamp = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_randamp(x, randamp);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randampamt") == 0){
				if(arg1 == &s_){
					float randampamt = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_randampamt(x, randampamt);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randtp") == 0){
				if(arg1 == &s_){
					float randtp = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_randtp(x, randtp);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-randtpamt") == 0){
				if(arg1 == &s_){
					float randtpamt = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_randtpamt(x, randtpamt);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-posvar") == 0){
				if(arg1 == &s_){
					float posvar = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_posvar(x, posvar);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-hopvar") == 0){
				if(arg1 == &s_){
					float hopvar = atom_getfloatarg(1, argc, argv);
					grainyrd_tilde_posvar(x, hopvar);
					argc-= 2;
					argv+= 2;
				}
				else{
					goto errstate;
				};
			}
			else if(strcmp(arg0->s_name, "-env") == 0){
				if(arg1 != &s_){
					grainyrd_tilde_envtype(x, arg1);
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

		for(i=0; i<DXKGNYNUMGR; i++){//init all matrices
			x->x_grainsize[i] = 0; 
			x->x_grainstart[i] = 0;
			x->x_grainstep[i] = 0.f;
			x->x_grainamp[i] = 0.; 
			x->x_grainpos[i] = 0.f;
			x->x_freegr[i] = i; //indexes of free grains
			x->x_actgn[i] = DXKGNYNUMGR;

			x->x_winmap[i] = 0.f;
			x->x_winpos[i] = 0.f; 
		};
		grainyrd_tilde_makeenv(x, x->x_env);
		x->x_init = 1;
    return (x);
	};
	errstate:
		pd_error(x, "grainyrd~: improper args");
		return NULL;
}

static t_int *grainyrd_tilde_perform(t_int *w)
{
    t_grainyrd_tilde *x = (t_grainyrd_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    int maxindex;
    t_word *buf = x->x_vec;
    int i, j;
	int grainrate = x->x_grainrate;
	int overlap = x->x_overlap;
	int grainsz = grainrate*overlap; //grainsize determined by grainrate * overlap
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
    
	maxindex = x->x_npoints - 1;
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
				cursz += (int)(exprseed()*((double)m_sr*rszamt));
			};
			int curidx = getnextgr(x); //getting next grain idx
			x->x_grainsize[curidx] = cursz;
			x->x_grainpos[curidx] = 0;
			int curpos = index  + (int)((2.f*exprseed()-1.f)*(m_sr*posvar));
			if(curpos < 0){
				curpos = 0;
			}
			else if(curpos > maxindex){
				curpos = maxindex;
			};
			x->x_grainstart[curidx] = curpos;
			x->x_grainstep[curidx] = getreadscale(x, curtp);
			x->x_winmap[curidx] = (double)DXKGNYM/(double)cursz;
			x->x_winpos[curidx] = 0.f;
			x->x_nextgr = grainrate + (int)(exprseed()*((double)grainrate*hopvar));
			//post("%d", x->x_nextgr);
			x->x_grainamp[curidx] = amp;
			if(randamp){
				x->x_grainamp[curidx] += (rampamt * (2.f*exprseed()-1.f));
			};
			x->x_numact++;

		};
		int graincount = 0;
		for(j=0; j<DXKGNYNUMGR; j++){
			//add active grain check
				int grnidx = x->x_actgn[j];
				if(grnidx < DXKGNYNUMGR){
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
					if(winidx> DXKGNYM - 1){
						winidx = DXKGNYM -1;
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

static void grainyrd_tilde_set(t_grainyrd_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            pd_error(x, "grainyrd~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatwords(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for grainyrd~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void grainyrd_tilde_dsp(t_grainyrd_tilde *x, t_signal **sp)
{
    grainyrd_tilde_set(x, x->x_arrayname);
	x->x_nextgr = 0;

    dsp_add(grainyrd_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void grainyrd_tilde_free(t_grainyrd_tilde *x)
{
}

void grainyrd_tilde_setup(void)
{
    grainyrd_tilde_class = class_new(gensym("grainyrd~"),
        (t_newmethod)grainyrd_tilde_new, (t_method)grainyrd_tilde_free,
        sizeof(t_grainyrd_tilde), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(grainyrd_tilde_class, t_grainyrd_tilde, x_f);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_dsp,
        gensym("dsp"), A_CANT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_set,
        gensym("set"), A_SYMBOL, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_overlap,
        gensym("overlap"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_randsize,
        gensym("randsize"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_randsizeamt,
        gensym("randsizeamt"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_randtp,
        gensym("randtp"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_randtpamt,
        gensym("randtpamt"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_randamp,
        gensym("randamp"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_randampamt,
        gensym("randampamt"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_grainrate,
        gensym("grainrate"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_posvar,
        gensym("posvar"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_hopvar,
        gensym("hopvar"), A_FLOAT, 0);
    class_addmethod(grainyrd_tilde_class, (t_method)grainyrd_tilde_envtype,
        gensym("env"), A_SYMBOL, 0);
}
