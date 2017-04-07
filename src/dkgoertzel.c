//http://www.embedded.com/design/configurable-systems/4024443/The-Goertzel-Algorithm
// for coeffs
// using standard goertzel iir/fir filter representation otherwise

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "m_pd.h"
#include "dkbuf.h"
#include "dkmath.h"

#define DKGOERTZ_NMIN 2
#define DKGOERTZ_STACK 1024
#define DKGOERTZ_HOP 2

static t_class *dkgoertzel_tilde_class;

typedef struct _dkgoertzel_tilde
{
  t_object  x_obj;
  t_outlet  *x_outlet;
  int     x_sr;
  int    x_n;
  int    x_k;
  double x_w;
  double x_cos;
  double x_sin;
  double x_coeff;
  t_float x_freq;
  //pointers to the delay bufs
  double  * x_xbuf; 
  double x_ffstack[DKGOERTZ_STACK];
  int     x_alloc; //if we are using allocated bufs
  unsigned int     x_sz; //actual allocated size of each delay buffer
  unsigned int x_bufsz; //actual size of buf we're using
  unsigned int       x_wh;     //writehead
} t_dkgoertzel_tilde;


static void dkgoertzel_coeffs(t_dkgoertzel_tilde *x)
{
  int w, k = (int)(0.5 + ((double)x->x_n * x->x_freq)/(double)x->x_sr);
  x->x_k = k;
  w = (TPI/(double)x->x_n)*(double)k;
  x->x_w = w;
  x->x_cos = cos(w);
  x->x_sin = sin(w);
  coeff = 2*x->x_cos;
  
}

double dkgoertzel_power(t_dkgoertzel_tilde *x, int startpos)
{
  int n = x->x_n; //frame size
  int i;
  double y0 = 0, y1 = 0, y2 = 0, x0 = 0, power = 0, coeff = x->_coeff;
  //iir calcuation
  for(i=0; i<n; i++)
    {
      int realidx = (startpos+i)%x->x_bufsz; //realidx takes into account wrapping and startpos
      x0 = x->x_xbuf[realidx];
      y0 = x0 + coeff * y1 - y2;
      y2 = y1;
      y1 = y0;
    };

  //final mag squared calculation
  power = y1*y1 + y2 * y2 - (coeff * y1 * y2);
  return power;
  
}

static void dkgoertzel_clear(t_dkgoertzel_tilde *x){
    unsigned int i;
    for(i=0; i<x->x_sz; i++){
        x->x_xbuf[i] = 0.;
        x->x_ybuf[i] = 0.;
    };
    x->x_wh = 0;
}

static void dkgoertzel_sz(t_dkgoertzel_tilde *x){
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
    else if(newsz > DKGOERTZEL_MAXD){
        newsz = DKGOERTZEL_MAXD;
    };
    if(!alloc && newsz > DKGOERTZ_STACK){
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
    else if(alloc && newsz < DKGOERTZ_STACK){
        free(x->x_xbuf);
        free(x->x_ybuf);
        x->x_sz = DKGOERTZ_STACK;
        x->x_xbuf = x->x_ffstack;
        x->x_ybuf = x->x_fbstack;
        x->x_alloc = 0;
    };
    dkgoertzel_clear(x);
}




static double dkgoertzel_getlin(double tab[], unsigned int sz, double idx){
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

static double dkgoertzel_readmsdelay(t_dkgoertzel_tilde *x, double arr[], t_float ms){
    //helper func, basically take desired ms delay, convert to samp, read from arr[]

    //eventual reading head
    double rh = (double)ms*((double)x->x_sr*0.001); //converting ms to samples
    //bounds checking for minimum delay in samples
        if(rh < DKGOERTZEL_MIND){
            rh = DKGOERTZEL_MIND;
        };
        rh = (double)x->x_wh+((double)x->x_sz-rh); //essentially subracting from writehead to find proper position in buffer
        //wrapping into length of delay buffer
        while(rh >= x->x_sz){
            rh -= (double)x->x_sz;
        };
        //now to read from the buffer!
        double output = dkgoertzel_getlin(arr, x->x_sz, rh);
        return output;
        
}



static t_int *dkgoertzel_perform(t_int *w)
{
    t_dkgoertzel_tilde *x = (t_dkgoertzel_tilde *)(w[1]);
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
        double delx = dkgoertzel_readmsdelay(x, x->x_xbuf, delms);
        double dely = dkgoertzel_readmsdelay(x, x->x_ybuf, delms);
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

static void dkgoertzel_dsp(t_dkgoertzel_tilde *x, t_signal **sp)
{
    int sr = sp[0]->s_sr;
    if(sr != x->x_sr){
        //if new sample rate isn't old sample rate, need to realloc
        x->x_sr = sr;
        dkgoertzel_sz(x);
    };
    dsp_add(dkgoertzel_perform, 6, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
	    sp[3]->s_vec);
}

static void dkgoertzel_list(t_dkgoertzel_tilde *x, t_symbol *s, int argc, t_atom * argv){
  
   
    int argnum = 0; //current argument
    while(argc){
        if(argv -> a_type == A_FLOAT){
            t_float curf = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    //initdel
                        if(curf < DKGOERTZEL_MINMS){
                            curf = DKGOERTZEL_MINMS;
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

static void dkgoertzel_maxdel(t_dkgoertzel_tilde *x, t_float maxdel){
    
    x->x_maxdel = maxdel > 0 ? maxdel : DKGOERTZEL_DELAY; 
    dkgoertzel_sz(x);

}

static void *dkgoertzel_new(t_symbol *s, int argc, t_atom * argv){
    t_dkgoertzel_tilde *x = (t_dkgoertzel_tilde *)pd_new(dkgoertzel_tilde_class);
   
    //defaults
    t_float maxdel = DKGOERTZEL_DELAY;
    t_float initdel = DKGOERTZEL_MINMS;
    t_float gain = DKGOERTZEL_DEFGAIN;
    x->x_sr = sys_getsr();
    
    x->x_alloc = 0;
    x->x_sz = DKGOERTZ_STACK;
    //clear out stack bufs, set pointer to stack
    x->x_ybuf = x->x_fbstack;
    x->x_xbuf = x->x_ffstack;
    dkgoertzel_clear(x);

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
    

    x->x_maxdel = maxdel > 0 ? maxdel : DKGOERTZEL_DELAY; 
      //ship off to the helper method to deal with allocation if necessary
    dkgoertzel_sz(x);
    //boundschecking
    //this is 1/44.1 (1/(sr*0.001) rounded up, good enough?

    if(initdel < DKGOERTZEL_MINMS){
        initdel = DKGOERTZEL_MINMS;
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
    pd_error(x, "dkgoertzel~: improper args!");
    return NULL;
}





static void * dkgoertzel_free(t_dkgoertzel_tilde *x){
    if(x->x_alloc){
        free(x->x_xbuf);
        free(x->x_ybuf);
    };
    inlet_free(x->x_dellet);
    inlet_free(x->x_gainlet);
    outlet_free(x->x_outlet);
    return (void *)x;
}

void dkgoertzel_tilde_setup(void)
{
    dkgoertzel_tilde_class = class_new(gensym("dkgoertzel~"),
			   (t_newmethod)dkgoertzel_new,
			   (t_method)dkgoertzel_free,
			   sizeof(t_dkgoertzel_tilde), 0,
			   A_GIMME, 0);
    class_addmethod(dkgoertzel_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(dkgoertzel_tilde_class, (t_method)dkgoertzel_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(dkgoertzel_tilde_class, (t_method)dkgoertzel_clear, gensym("clear"), 0);
    class_addmethod(dkgoertzel_tilde_class, (t_method)dkgoertzel_maxdel, gensym("maxdelay"), A_FLOAT, 0);
    class_addlist(dkgoertzel_tilde_class, (t_method)dkgoertzel_list);
}
