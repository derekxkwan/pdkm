//copied from my work from other libraries... Derek Kwan GPL v.2

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "m_pd.h"

#define DKATMP_STACK 48000 //stack buf size, 1 sec at 48k for good measure
#define DKATMP_DELAY  48000.0 //maximum delay
#define DKATMP_MIND 1 //minumum delay 
#define DKATMP_MAXD 4294967294 //max delay = 2**32 - 2

#define DKATMP_MINMS 0. //min delay in ms

#define DKATMP_DEFGAIN 0.5 //default gain
#define DKATMP_DEFFF 0. //default ff gain
#define DKATMP_DEFFB 0. //default fb gain

static t_class *dkatmp_tilde_class;

typedef struct _dkatmp_tilde
{
    t_object  x_obj;
    t_inlet  *x_dellet;
    t_inlet  *x_gainlet;
    t_outlet  *x_outlet;
    int     x_sr;
    //pointers to the delay bufs
    double  * x_ybuf; 
    double x_ffstack[DKATMP_STACK];
    double * x_xbuf;
    double x_fbstack[DKATMP_STACK];
    int     x_alloc; //if we are using allocated bufs
    unsigned int     x_sz; //actual size of each delay buffer

    t_float     x_maxdel;  //maximum delay in ms
    unsigned int       x_wh;     //writehead
} t_dkatmp_tilde;

static void dkatmp_clear(t_dkatmp_tilde *x){
    unsigned int i;
    for(i=0; i<x->x_sz; i++){
        x->x_xbuf[i] = 0.;
        x->x_ybuf[i] = 0.;
    };
    x->x_wh = 0;
}

static void dkatmp_sz(t_dkatmp_tilde *x){
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
    else if(newsz > DKATMP_MAXD){
        newsz = DKATMP_MAXD;
    };
    if(!alloc && newsz > DKATMP_STACK){
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
    else if(alloc && newsz < DKATMP_STACK){
        free(x->x_xbuf);
        free(x->x_ybuf);
        x->x_sz = DKATMP_STACK;
        x->x_xbuf = x->x_ffstack;
        x->x_ybuf = x->x_fbstack;
        x->x_alloc = 0;
    };
    dkatmp_clear(x);
}




static double dkatmp_getlin(double tab[], unsigned int sz, double idx){
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

static double dkatmp_readmsdelay(t_dkatmp_tilde *x, double arr[], t_float ms){
    //helper func, basically take desired ms delay, convert to samp, read from arr[]

    //eventual reading head
    double rh = (double)ms*((double)x->x_sr*0.001); //converting ms to samples
    //bounds checking for minimum delay in samples
        if(rh < DKATMP_MIND){
            rh = DKATMP_MIND;
        };
        rh = (double)x->x_wh+((double)x->x_sz-rh); //essentially subracting from writehead to find proper position in buffer
        //wrapping into length of delay buffer
        while(rh >= x->x_sz){
            rh -= (double)x->x_sz;
        };
        //now to read from the buffer!
        double output = dkatmp_getlin(arr, x->x_sz, rh);
        return output;
        
}



static t_int *dkatmp_perform(t_int *w)
{
    t_dkatmp_tilde *x = (t_dkatmp_tilde *)(w[1]);
    int n = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float *din = (t_float *)(w[4]);
    t_float *gin = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    
    int i;
    for(i=0; i<n;i++){
        int wh = x->x_wh;
        double input = (double)xin[i];
        //first off, write input to delay buf
        x->x_xbuf[wh] = input;
        //get delayed values of x and y
        t_float delms = din[i];
        //first bounds checking
        if(delms < 0){
            delms = 0;
        }
        else if(delms > x->x_maxdel){
            delms = x->x_maxdel;
        };
        //now get those delayed vals
        double delx = dkatmp_readmsdelay(x, x->x_xbuf, delms);
        double dely = dkatmp_readmsdelay(x, x->x_ybuf, delms);
        //figure out your current y term: y[n] = -g*x[n] + x[n-d] + g*y[n-d]
        double output = (double)gin[i]*-1.*input + delx + (double)gin[i]*dely;
        //stick this guy in the ybuffer and output
        x->x_ybuf[wh] = output;
        out[i] = output;

        //increment writehead
        x->x_wh = (wh + 1) % x->x_sz;
    };
    
    return (w + 7);
}

static void dkatmp_dsp(t_dkatmp_tilde *x, t_signal **sp)
{
    int sr = sp[0]->s_sr;
    if(sr != x->x_sr){
        //if new sample rate isn't old sample rate, need to realloc
        x->x_sr = sr;
        dkatmp_sz(x);
    };
    dsp_add(dkatmp_perform, 6, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
	    sp[3]->s_vec);
}

static void dkatmp_list(t_dkatmp_tilde *x, t_symbol *s, int argc, t_atom * argv){
  
   
    int argnum = 0; //current argument
    while(argc){
        if(argv -> a_type == A_FLOAT){
            t_float curf = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    //initdel
                        if(curf < DKATMP_MINMS){
                            curf = DKATMP_MINMS;
                          }
                        else if(curf > x->x_maxdel){
                             curf = x->x_maxdel;
                         };
                    pd_float((t_pd *)x->x_dellet, curf);
                    break;
                case 1:
                    //gain
                    pd_float((t_pd *)x->x_gainlet, curf);
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

static void dkatmp_maxdel(t_dkatmp_tilde *x, t_float maxdel){
    
    x->x_maxdel = maxdel > 0 ? maxdel : DKATMP_DELAY; 
    dkatmp_sz(x);

}

static void *dkatmp_new(t_symbol *s, int argc, t_atom * argv){
    t_dkatmp_tilde *x = (t_dkatmp_tilde *)pd_new(dkatmp_tilde_class);
   
    //defaults
    t_float maxdel = DKATMP_DELAY;
    t_float initdel = DKATMP_MINMS;
    t_float gain = DKATMP_DEFGAIN;
    x->x_sr = sys_getsr();
    
    x->x_alloc = 0;
    x->x_sz = DKATMP_STACK;
    //clear out stack bufs, set pointer to stack
    x->x_ybuf = x->x_fbstack;
    x->x_xbuf = x->x_ffstack;
    dkatmp_clear(x);

    int argnum = 0; //current argument
    while(argc){
        if(argv -> a_type == A_FLOAT){
            t_float curf = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    initdel = curf;
                    break;
                case 1:
                    gain = curf;
                    break;
                default:
                    break;
            };
            argnum++;
            argc--;
            argv++;
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
    };
    

    x->x_maxdel = maxdel > 0 ? maxdel : DKATMP_DELAY; 
      //ship off to the helper method to deal with allocation if necessary
    dkatmp_sz(x);
    //boundschecking
    //this is 1/44.1 (1/(sr*0.001) rounded up, good enough?

    if(initdel < DKATMP_MINMS){
        initdel = DKATMP_MINMS;
    }
    else if(initdel > x->x_maxdel){
        initdel = x->x_maxdel;
    };

    //inlets outlets
    x->x_dellet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_dellet, initdel);
    x->x_gainlet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_gainlet, gain);
    x->x_outlet = outlet_new((t_object *)x, &s_signal);
    return (x);
    errstate:
    pd_error(x, "dkatmp~: improper args!");
    return NULL;
}





static void * dkatmp_free(t_dkatmp_tilde *x){
    if(x->x_alloc){
        free(x->x_xbuf);
        free(x->x_ybuf);
    };
    inlet_free(x->x_dellet);
    inlet_free(x->x_gainlet);
    outlet_free(x->x_outlet);
    return (void *)x;
}

void dkatmp_tilde_setup(void)
{
    dkatmp_tilde_class = class_new(gensym("dkatmp~"),
			   (t_newmethod)dkatmp_new,
			   (t_method)dkatmp_free,
			   sizeof(t_dkatmp_tilde), 0,
			   A_GIMME, 0);
    class_addmethod(dkatmp_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(dkatmp_tilde_class, (t_method)dkatmp_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(dkatmp_tilde_class, (t_method)dkatmp_clear, gensym("clear"), 0);
    class_addmethod(dkatmp_tilde_class, (t_method)dkatmp_maxdel, gensym("maxdelay"), A_FLOAT, 0);
    class_addlist(dkatmp_tilde_class, (t_method)dkatmp_list);
}
