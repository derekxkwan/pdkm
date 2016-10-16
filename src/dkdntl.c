//copied from my work from other libraries... Derek Kwan GPL v.2

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "m_pd.h"

#define DKDNTL_STACK 48000 //stack buf size, 1 sec at 48k for good measure
#define DKDNTL_DELAY  48000.0 //maximum delay
#define DKDNTL_MIND 1 //minumum delay 
#define DKDNTL_MAXD 4294967294 //max delay = 2**32 - 2

#define DKDNTL_MINMS 0. //min delay in ms

#define DKDNTL_DEFGAIN 1. //default gain
#define DKDNTL_DEFFF 0. //default ff gain
#define DKDNTL_DEFFB 0. //default fb gain

static t_class *dkdntl_tilde_class;

typedef struct _dkdntl_tilde
{
    t_object  x_obj;
    t_inlet  *x_ffdellet;
    t_inlet  *x_fbdellet;
    t_inlet  *x_alet;
    t_inlet  *x_blet;
    t_inlet  *x_clet;
    t_outlet  *x_outlet;
    int     x_sr;
    //pointers to the delay bufs
    double  * x_ybuf; 
    double x_ffstack[DKDNTL_STACK];
    double * x_xbuf;
    double x_fbstack[DKDNTL_STACK];
    int     x_alloc; //if we are using allocated bufs
    unsigned int     x_sz; //actual size of each delay buffer

    t_float     x_maxdel;  //maximum delay in ms
    unsigned int       x_wh;     //writehead
} t_dkdntl_tilde;

static void dkdntl_clear(t_dkdntl_tilde *x){
    unsigned int i;
    for(i=0; i<x->x_sz; i++){
        x->x_xbuf[i] = 0.;
        x->x_ybuf[i] = 0.;
    };
    x->x_wh = 0;
}

static void dkdntl_sz(t_dkdntl_tilde *x){
    //helper function to deal with allocation issues if needed
    //ie if wanted size x->x_maxdel is bigger than stack, allocate
    
    //convert ms to samps
    unsigned int newsz = (unsigned int)ceil((double)x->x_maxdel*0.001*(double)x->x_sr);
    newsz++; //add a sample for good measure since say bufsize is 2048 and 
    //you want a delay of 2048 samples,.. problem!

    int alloc = x->x_alloc;
    unsigned int cursz = x->x_sz; //current size

    if(newsz < 0){
        newsz = 0;
    }
    else if(newsz > DKDNTL_MAXD){
        newsz = DKDNTL_MAXD;
    };
    if(!alloc && newsz > DKDNTL_STACK){
        x->x_xbuf = (double *)malloc(sizeof(double)*newsz);
        x->x_ybuf = (double *)malloc(sizeof(double)*newsz);
        x->x_alloc = 1;
        x->x_sz = newsz;
    }
    else if(alloc && newsz > cursz){
        x->x_xbuf = (double *)realloc(x->x_xbuf, sizeof(double)*newsz);
        x->x_ybuf = (double *)realloc(x->x_ybuf, sizeof(double)*newsz);
        x->x_sz = newsz;
    }
    else if(alloc && newsz < DKDNTL_STACK){
        free(x->x_xbuf);
        free(x->x_ybuf);
        x->x_sz = DKDNTL_STACK;
        x->x_xbuf = x->x_ffstack;
        x->x_ybuf = x->x_fbstack;
        x->x_alloc = 0;
    };
    dkdntl_clear(x);
}




static double dkdntl_getlin(double tab[], unsigned int sz, double idx){
    //copying from my own library, linear interpolated reader - DK
        double output;
        unsigned int tabphase1 = (unsigned int)idx;
        unsigned int tabphase2 = tabphase1 + 1;
        double frac = idx - (double)tabphase1;
        if(tabphase1 >= sz - 1){
                tabphase1 = sz - 1; //checking to see if index falls within bounds
                output = tab[tabphase1];
        }
        else if(tabphase1 < 0){
                tabphase1 = 0;
                output = tab[tabphase1];
            }
        else{
            double yb = tab[tabphase2]; //linear interp
            double ya = tab[tabphase1];
            output = ya+((yb-ya)*frac);
        
        };
        return output;
}

static double dkdntl_readmsdelay(t_dkdntl_tilde *x, double arr[], t_float ms){
    //helper func, basically take desired ms delay, convert to samp, read from arr[]

    //eventual reading head
    double rh = (double)ms*((double)x->x_sr*0.001); //converting ms to samples
    //bounds checking for minimum delay in samples
        if(rh < DKDNTL_MIND){
            rh = DKDNTL_MIND;
        };
        rh = (double)x->x_wh+((double)x->x_sz-rh); //essentially subracting from writehead to find proper position in buffer
        //wrapping into length of delay buffer
        while(rh >= x->x_sz){
            rh -= (double)x->x_sz;
        };
        //now to read from the buffer!
        double output = dkdntl_getlin(arr, x->x_sz, rh);
        return output;
        
}



static t_int *dkdntl_perform(t_int *w)
{
    t_dkdntl_tilde *x = (t_dkdntl_tilde *)(w[1]);
    int n = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float *ffdin = (t_float *)(w[4]);
    t_float *fbdin = (t_float *)(w[5]);
    t_float *ain = (t_float *)(w[6]);
    t_float *bin = (t_float *)(w[7]);
    t_float *cin = (t_float *)(w[8]);
    t_float *out = (t_float *)(w[9]);
    
    int i;
    for(i=0; i<n;i++){
        int wh = x->x_wh;
        double input = (double)xin[i];
        //first off, write input to delay buf
        x->x_xbuf[wh] = input;
        //get delayed values of x and y
        t_float ffdelms = ffdin[i];
        t_float fbdelms = fbdin[i];
        //first bounds checking
        if(ffdelms < 0){
            ffdelms = 0;
        }
        else if(ffdelms > x->x_maxdel){
            ffdelms = x->x_maxdel;
        };
         if(fbdelms < 0){
            fbdelms = 0;
        }
        else if(fbdelms > x->x_maxdel){
            fbdelms = x->x_maxdel;
        };

        //now get those delayed vals
        double delx = dkdntl_readmsdelay(x, x->x_xbuf, ffdelms);
        double dely = dkdntl_readmsdelay(x, x->x_ybuf, fbdelms);
        //figure out your current y term: y[n] = a*x[n] + b*x[n-ffdel] + c*y[n-fbdel]
        double output = (double)ain[i]*input + (double)bin[i]*delx + (double)cin[i]*dely;
        //stick this guy in the ybuffer and output
        x->x_ybuf[wh] = output;
        out[i] = output;

        //increment writehead
        x->x_wh = (wh + 1) % x->x_sz;
    };
    
    return (w + 10);
}

static void dkdntl_dsp(t_dkdntl_tilde *x, t_signal **sp)
{
    int sr = sp[0]->s_sr;
    if(sr != x->x_sr){
        //if new sample rate isn't old sample rate, need to realloc
        x->x_sr = sr;
        dkdntl_sz(x);
    };
    dsp_add(dkdntl_perform, 9, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
	    sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec);
}

static void dkdntl_list(t_dkdntl_tilde *x, t_symbol *s, int argc, t_atom * argv){
  
   
    int argnum = 0; //current argument
    while(argc){
        if(argv -> a_type == A_FLOAT){
            t_float curf = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    //ffdel
                        if(curf < DKDNTL_MINMS){
                            curf = DKDNTL_MINMS;
                          }
                        else if(curf > x->x_maxdel){
                             curf = x->x_maxdel;
                         };
                    pd_float((t_pd *)x->x_ffdellet, curf);
                    break;
                 case 1:
                    //fbdel
                        if(curf < DKDNTL_MINMS){
                            curf = DKDNTL_MINMS;
                          }
                        else if(curf > x->x_maxdel){
                             curf = x->x_maxdel;
                         };
                    pd_float((t_pd *)x->x_fbdellet, curf);
                    break;

                case 2:
                    //gain
                    pd_float((t_pd *)x->x_alet, curf);
                    break;
                case 3:
                    //ffcoeff
                    pd_float((t_pd *)x->x_blet, curf);
                    break;
                case 4:
                    //fbcoeff
                    pd_float((t_pd *)x->x_clet, curf);
                    break;
                default:
                    break;
            };
            argnum++;
        };
        argc--;
        argv++;
    };



}

static void dkdntl_maxdel(t_dkdntl_tilde *x, t_float maxdel){
    
    x->x_maxdel = maxdel > 0 ? maxdel : DKDNTL_DELAY; 
    dkdntl_sz(x);

}

static void *dkdntl_new(t_symbol *s, int argc, t_atom * argv){
    t_dkdntl_tilde *x = (t_dkdntl_tilde *)pd_new(dkdntl_tilde_class);
   
    //defaults
    t_float maxdel = DKDNTL_DELAY;
    t_float ffdel = DKDNTL_MINMS;
    t_float fbdel = DKDNTL_MINMS;
    t_float gain = DKDNTL_DEFGAIN;
    t_float ffcoeff = DKDNTL_DEFFF;
    t_float fbcoeff = DKDNTL_DEFFB;
    x->x_sr = sys_getsr();
    
    x->x_alloc = 0;
    x->x_sz = DKDNTL_STACK;
    //clear out stack bufs, set pointer to stack
    x->x_ybuf = x->x_fbstack;
    x->x_xbuf = x->x_ffstack;
    dkdntl_clear(x);

    int argnum = 0; //current argument
    while(argc){
        if(argv -> a_type == A_FLOAT){
            t_float curf = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    ffdel = curf;
                    break;
                case 1:
                    fbdel = curf;
                    break;
                case 2:
                    gain = curf;
                    break;
                case 3:
                    ffcoeff = curf;
                    break;
                case 4:
                    fbcoeff = curf;
                    break;
                default:
                    break;
            };
            argnum++;
        }
        else if(argv -> a_type == A_SYMBOL){
            t_symbol * cursym = atom_getsymbolarg(0, argc, argv);
            if(argc>= 2){
                if(strcmp(cursym->s_name, "-maxdelay") == 0){
                    t_float curf = atom_getfloatarg(1, argc, argv);
                    maxdel = curf;
                }
                else{
                    goto errstate;
                };
                argc -=2;
                argv += 2;
            }
            else{
                goto errstate;
            };
        };
        argc--;
        argv++;
    };
    

    x->x_maxdel = maxdel > 0 ? maxdel : DKDNTL_DELAY; 
      //ship off to the helper method to deal with allocation if necessary
    dkdntl_sz(x);
    //boundschecking
    //this is 1/44.1 (1/(sr*0.001) rounded up, good enough?

    if(ffdel < DKDNTL_MINMS){
        ffdel = DKDNTL_MINMS;
    }
    else if(ffdel > x->x_maxdel){
        ffdel = x->x_maxdel;
    };

    if(fbdel < DKDNTL_MINMS){
        fbdel = DKDNTL_MINMS;
    }
    else if(fbdel > x->x_maxdel){
        fbdel = x->x_maxdel;
    };

    //inlets outlets
    x->x_ffdellet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_ffdellet, ffdel);
    x->x_fbdellet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_fbdellet, fbdel);
    x->x_alet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_alet, gain);
    x->x_blet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_blet, ffcoeff);
    x->x_clet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_clet, fbcoeff);
    x->x_outlet = outlet_new((t_object *)x, &s_signal);
    return (x);
    errstate:
    pd_error(x, "dkdntl~: improper args!");
    return NULL;
}





static void * dkdntl_free(t_dkdntl_tilde *x){
    if(x->x_alloc){
        free(x->x_xbuf);
        free(x->x_ybuf);
    };
    inlet_free(x->x_ffdellet);
    inlet_free(x->x_fbdellet);
    inlet_free(x->x_alet);
    inlet_free(x->x_blet);
    inlet_free(x->x_clet);
    outlet_free(x->x_outlet);
    return (void *)x;
}

void dkdntl_tilde_setup(void)
{
    dkdntl_tilde_class = class_new(gensym("dkdntl~"),
			   (t_newmethod)dkdntl_new,
			   (t_method)dkdntl_free,
			   sizeof(t_dkdntl_tilde), 0,
			   A_GIMME, 0);
    class_addmethod(dkdntl_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(dkdntl_tilde_class, (t_method)dkdntl_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(dkdntl_tilde_class, (t_method)dkdntl_clear, gensym("clear"), 0);
    class_addmethod(dkdntl_tilde_class, (t_method)dkdntl_maxdel, gensym("maxdelay"), A_FLOAT, 0);
    class_addlist(dkdntl_tilde_class, (t_method)dkdntl_list);
}
