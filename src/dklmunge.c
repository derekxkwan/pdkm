/* Copyright 2017 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "m_pd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "dkmem.h"
#include "dkrnd.h"

static t_class *dklmunge_class;

typedef enum{
    NONE,
    REV,
    ROT,
    RPT1,
    EVERYN,
    REVROT,
    DRIP,
    STREAM,
    INTERL,
    DEINTER,
    GROUP,
    SWAP,
    SUBST,
    RANDELT,
    FARO,
    FYSHUF,
    RANDF,
    RANDI,
    COUNT,
    ADD,
    SUB,
    MULT,
    DIV,
    POW,
    LOG,
    MIN,
    MAX,
    AVG,
    SUM,
    PROD,
    MTOF


} munger;


typedef struct _dklmunge {
	t_object x_obj;
        munger x_munger;
        t_dkmem * x_dkmungee;
        t_dkrnd * x_dkrnd;
        int       x_mungeelen; 
        t_dkmem * x_dkparam;
        int       x_paramlen;
        int       x_munge; //if we are going to munge or not
        t_inlet * x_mungeelet;
        t_inlet * x_mungerlet;
        t_inlet * x_mungelet;
        t_outlet * x_munged; 
} t_dklmunge;

static void dklmunge_setmunger(t_dklmunge *x, t_symbol * s)
{
    char * cs = s->s_name;
    if(strcmp(cs, "add") == 0)
    {
        x->x_munger = ADD;
    }
    else if(strcmp(cs, "rev") == 0)
    {
        x->x_munger = REV;
    }
    else if(strcmp(cs, "rot") == 0)
    {
        x->x_munger = ROT;
    }
    else if(strcmp(cs, "revrot") == 0)
    {
        x->x_munger = REVROT;
    }
    else if (strcmp(cs, "drip") == 0)
    {
        x->x_munger = DRIP;
    }
    else if (strcmp(cs, "stream") == 0)
    {
        x->x_munger = STREAM;
    }
    else if(strcmp(cs, "deinter") == 0)
    {
        x->x_munger = DEINTER;
    }
    else if(strcmp(cs, "group") == 0)
    {
        x->x_munger = GROUP;
    }
    else if(strcmp(cs, "swap") == 0)
    {
        x->x_munger = SWAP;
    }
    else if(strcmp(cs, "subst") == 0)
    {
        x->x_munger = SUBST;
    }
    else if( strcmp(cs, "randelt") == 0)
    {
        x->x_munger = RANDELT;
    }
    else if(strcmp(cs, "faro") == 0)
    {
        x->x_munger = FARO;
    }
    else if(strcmp(cs, "fyshuf") == 0)
    {
        x->x_munger = FYSHUF;
    }
    else if(strcmp(cs, "randf") == 0)
    {
        x->x_munger = RANDF;
    }
    else if(strcmp(cs, "randi") == 0)
    {
        x->x_munger = RANDI;
    }
    else if(strcmp(cs, "count") == 0)
    {
        x->x_munger = COUNT;
    }
    else if(strcmp(cs, "sub") == 0)
    {
        x->x_munger = SUB;
    }
    else if(strcmp(cs, "mult") == 0)
    {
        x->x_munger = MULT;
    }
    else if(strcmp(cs, "div") == 0)
    {
        x->x_munger = DIV;
    }
    else if(strcmp(cs, "max") == 0)
    {
        x->x_munger = MAX;
    }
    else if(strcmp(cs, "min") == 0)
    {
        x->x_munger = MIN;
    }
    else if(strcmp(cs, "avg") == 0)
    {
        x->x_munger = AVG;
    }
    else if(strcmp(cs, "mtof") == 0)
    {
        x->x_munger = MTOF;
    }
    else if(strcmp(cs, "sum") == 0)
    {
        x->x_munger = SUM;
    }
    else if(strcmp(cs, "prod") == 0)
    {
        x->x_munger = PROD;
    }
    else if(strcmp(cs, "interl") == 0)
    {
        x->x_munger = INTERL;
    }
    else if(strcmp(cs, "rpt1") == 0)
    {
        x->x_munger = RPT1;
    }
    else if(strcmp(cs, "everyn") == 0)
    {
        x->x_munger = EVERYN;
    };
}

static void dklmunge_eltset(t_atom * src, t_atom * dest)
{
    if(src->a_type == A_FLOAT)
    {
        SETFLOAT(dest, src->a_w.w_float);
    }
    else if(src->a_type == A_SYMBOL)
    {
        SETSYMBOL(dest, src->a_w.w_symbol);
    };
}

static void dklmunge_eltswap(t_atom * src, t_atom * dest)
{
    t_atom temp;
    dklmunge_eltset(dest, &temp);
    dklmunge_eltset(src, dest);
    dklmunge_eltset(&temp, src);
}

static void dklmunge_listcopy(t_atom * src, int srcsz, t_atom * dest, int destsz)
{
    int loopsz = srcsz < destsz ? srcsz : destsz;
    int i=0;
    for(i=0;i<loopsz; i++)
    {
        dklmunge_eltset(&src[i],&dest[i]);
    };

}


static void dklmunge_params(t_dklmunge *x, t_symbol *s, int argc, t_atom * argv)
{
    
    dkmem_alloc(x->x_dkparam, argc);
    dklmunge_listcopy(argv, argc, x->x_dkparam->m_data, x->x_dkparam->m_allocsz);
    x->x_paramlen = argc;
}

static void dklmunge_mungee(t_dklmunge *x, t_symbol*s, int argc, t_atom * argv)
{
    dkmem_alloc(x->x_dkmungee, argc);
    dklmunge_listcopy(argv, argc, x->x_dkmungee->m_data, x->x_dkmungee->m_allocsz);
    x->x_mungeelen = argc;
}

static void dklmunge_munger(t_dklmunge *x, t_symbol *s, int argc, t_atom * argv)
{
    if(argc)
    {
        if(argv->a_type == A_SYMBOL)
        {
            dklmunge_setmunger(x, argv->a_w.w_symbol);
            argc--;
            argv++;
            if(argc) dklmunge_params(x, &s_, argc, argv);
        };
    };
}
static void dklmunge_output(t_dklmunge * x, t_symbol * s, int argc, t_atom * argv)
{
    if(s) outlet_list(x->x_munged, s, argc, argv);
    else outlet_list(x->x_munged, 0, argc, argv);
}

static void dklmunge_rev(t_dklmunge *x, int argc, t_atom * argv)
{
    t_atom ret[argc];
    int i;
    for(i=0;i<argc;i++)
    {
        dklmunge_eltset(&argv[argc-i-1],&ret[i]);
    };
    dklmunge_output(x, 0, argc, ret);

}

static void dklmunge_revrot(t_dklmunge *x, int argc, t_atom * argv, t_float f)
{
    int rot = (int)f % argc;
    t_atom temp[argc];
    t_atom ret[argc];
    int leftover = argc-rot;
    int i;
    for(i=0;i<argc;i++)
    {
        dklmunge_eltset(&argv[argc-i-1],&temp[i]);
    };

    dklmunge_listcopy(&temp[rot], argc-rot, &ret[0], argc-rot);
   dklmunge_listcopy(&temp[0], rot, &ret[argc-rot], rot);
   dklmunge_output(x, 0, argc, ret);
}

static void dklmunge_fyshuf(t_dklmunge *x, int argc, t_atom * argv)
{
    t_atom ret[argc];
    int i, j;
    dkrnd_range(x->x_dkrnd, argc-0.5);
    dklmunge_listcopy(argv, argc, ret, argc);
    for(i=argc-1;i>= 1;i--)
    {
        j = (int)dkrnd_next(x->x_dkrnd);
        dklmunge_eltswap(&ret[i],&ret[j]); 
    };
   dklmunge_output(x, 0,  argc, ret);
}

static void dklmunge_rot(t_dklmunge *x, int argc, t_atom * argv, t_float f)
{
    int rot = (int)f % argc;
    t_atom ret[argc];
    int leftover = argc-rot;
    //example: argc = 7, rot = 5, put stuff from argv[5] into 0 (5,6) (argc-rot) = 2
    //then argv[0] to argv[4] (5 elts) into dest[2]
   dklmunge_listcopy(&argv[rot], argc-rot, &ret[0], argc-rot);
   dklmunge_listcopy(&argv[0], rot, &ret[argc-rot], rot);
   dklmunge_output(x, 0, argc, ret);
}

static void dklmunge_faro(t_dklmunge *x, int argc, t_atom * argv)
{
    t_atom ret[argc];
    int i=0, j, k=0, mid = (int)argc/2;
    if(argc % 2 == 0) j = mid;
    else j = mid + 1;
    //example argc=7, mid = 3;
    //0->0, 4->1, 1->2, 5->3, 2->4, 6->5, 3->6
    //argc=6, mid = 3
    //0->0, 3->1, 1->2, 4->3,2->4,5->5 
    while(i <= mid)
    {
        dklmunge_eltset(&argv[i], &ret[k]);
        i++; k++;
        if(j < argc)
        {
            dklmunge_eltset(&argv[j], &ret[k]);
            j++;k++;
        };
    };

   dklmunge_output(x, 0, argc, ret);
}
static int dklmunge_numdigits(int num)
{
    int count = 0;
    while(num)
    {
        num /= 10;
        ++count;
    };
    return count;
}

static void dklmunge_help_everyn(int argc, t_atom *argv, \
        t_atom * ret, int _n, int _offset)
{
    int n = _n < 1 ? 1 : (_n > argc ? argc : _n);
    int offset = _offset % n;
    int sz = (int)(argc/n);
    int i, idx;
    for(i=0;i<sz; i++)
    {   idx = (i * n) + offset;
        idx = idx > argc ? argc : idx;
        dklmunge_eltset(&argv[idx], &ret[i]);
    };


}

static double dklmunge_help_sum(int argc, t_atom * argv)
{
    int i;
    double sum = 0;
    for(i=0;i<argc; i++)
    {
        if(argv[i].a_type == A_FLOAT)
        {
            sum += argv[i].a_w.w_float;
        };
    };
    return sum;
}

static double dklmunge_help_prod(int argc, t_atom * argv)
{
    int i;
    double prod = 1;
    for(i=0;i<argc; i++)
    {
        if(argv[i].a_type == A_FLOAT)
        {
            prod *= argv[i].a_w.w_float;
        };
    };
    return prod;
}

static void dklmunge_group(t_dklmunge *x, int argc, t_atom * argv, t_float f)
{
    int groupnum, outputnum,  lastgroup =1;
    int argclen = dklmunge_numdigits(argc);
    int group = f <= 0 ? 1 : (f >= argc ? argc : (int) f);
    t_atom ret[group + 1];
    char groupname[argclen+1];
    int idx = (int)ceil(argc/(double)group)-1;
    idx *= group;

    //doesn't work if group = 1
    //example: group =3, argc = 8, we want: 0 1 2; 3 4 5; 6 7; ceil(8/3) = 3-1 = 2;
    
    if(group > 1)
    {
        while(idx >= 0)
        {
            //have to worry about last group
            if(lastgroup)
            {
                outputnum = (argc - idx) + 1;
                lastgroup = 0;
            }
            else outputnum = group + 1;
            groupnum = (int)idx/group;
            sprintf(groupname, "l%d%c", groupnum,0);

            SETSYMBOL(&ret[0], gensym((char *)groupname));
            dklmunge_listcopy(&argv[idx], outputnum -1, &ret[1], outputnum-1);
            dklmunge_output(x, 0, outputnum, ret);
            idx -= group;
        };
    }
    else
    {
        for(idx=argc-1;idx>= 0; idx--)
        {
            sprintf(groupname, "l%d%c", idx,0); 
            SETSYMBOL(&ret[0], gensym((char *)groupname));
            dklmunge_listcopy(&argv[idx], 1, &ret[1], 1);
            dklmunge_output(x, 0, 2, ret);
        };


    };
}

static void dklmunge_everyn(t_dklmunge * x, int argc, t_atom * argv, int paramlen, t_atom * params)
{
    int i;
     int offset = 0;
    int n = 1;
    for(i=0;i<2; i++)
    {
        if(params[i].a_type == A_FLOAT)
        {
            switch(i)
            {
                case 0: n = (int)params[i].a_w.w_float;
                        break;
                case 1: offset = (int)params[i].a_w.w_float;
            };
        };
    };
    int sz = (int)argc/n;
    t_atom ret[sz];
    dklmunge_help_everyn(argc, argv, ret, n, offset);
    dklmunge_output(x, 0, sz,ret);

}

static void dklmunge_sum(t_dklmunge *x, int argc, t_atom * argv)
{
    double sum = dklmunge_help_sum(argc, argv);
    outlet_float(x->x_munged, (t_float) sum);
}


static void dklmunge_prod(t_dklmunge *x, int argc, t_atom * argv)
{
    double prod = dklmunge_help_prod(argc, argv);
    outlet_float(x->x_munged, (t_float) prod);
}

static void dklmunge_avg(t_dklmunge *x, int argc, t_atom * argv)
{

    double avg = dklmunge_help_sum(argc, argv);
    avg /= (double)argc;
    outlet_float(x->x_munged, (t_float) avg);
}

static void dklmunge_rpt1(t_dklmunge * x, int argc, t_atom * argv, int paramlen, t_atom * params)
{
    int argsz = (int)paramlen/2;
    int i,j, idx, rpt, actidx =0;

    //actidx = indexer into return array
    //format: idx, rpt, idx, rpt, ....
    if(paramlen)
    {
        if(paramlen >= 2){
            t_atom rpts[argsz];
            //get every other elt starting with 2nd elt, these are the rpts
            dklmunge_help_everyn(paramlen, params, rpts, 2, 1);
            int sz = (int)dklmunge_help_sum(argsz,rpts); //number of elts we need
            t_atom ret[sz];
            for(i=0; i< argsz; i++)
            {
                //use if only both floats, idx is valid, and rpt > 0
                if(params[i*2].a_type == A_FLOAT && params[(i*2)+1].a_type == A_FLOAT)
                {
                    idx = (int)params[(i*2)].a_w.w_float;
                    rpt = (int)params[(i*2)+1].a_w.w_float;
                    if ((idx >=0) && (idx < argc) && (rpt > 0))
                    {
                        for(j=0;j<rpt;j++)
                        {
                            dklmunge_eltset(&argv[idx], &ret[actidx]);
                            actidx++;
                        };
                    };
                };
            };
            if(actidx) dklmunge_output(x, 0, actidx, ret);
        };
            

    
    };
}

static void dklmunge_router(t_dklmunge * x)
{
    t_atom * mungee = x->x_dkmungee->m_data;
    int mlen = x->x_mungeelen;
    t_atom * params = x->x_dkparam->m_data;
    int paramlen = x->x_paramlen;
    if(x->x_munge == 0)
    {

        dklmunge_output(x, 0, mlen, mungee);
        return;
    }
    else if(mlen)
    {
        switch(x->x_munger)
        {
            case NONE:dklmunge_output(x, 0, mlen, mungee);
                      break;
            case REV: dklmunge_rev(x, mlen, mungee);
                      break;
            case ROT: 
                      if(paramlen)
                      {
                        if(params->a_type == A_FLOAT) dklmunge_rot(x, mlen, mungee, params->a_w.w_float);
                      };
                      break;
            case REVROT:
                     if(paramlen)
                      {
                        if(params->a_type == A_FLOAT) dklmunge_revrot(x, mlen, mungee, params->a_w.w_float);
                      };
                      break;
            case FARO: dklmunge_faro(x, mlen, mungee);
                       break;
            case FYSHUF: dklmunge_fyshuf(x, mlen, mungee);
                         break;
            case GROUP:
                         if(paramlen)
                         {
                            if(params->a_type == A_FLOAT) dklmunge_group(x, mlen, mungee, params->a_w.w_float);
                         };
            case EVERYN:
                         if(paramlen)
                         {
                            if(params->a_type == A_FLOAT) dklmunge_everyn(x, mlen, mungee, paramlen, params);
                         };
                         break;
            case RPT1:
                         if(paramlen >= 2)
                         {
                            if(params[0].a_type == A_FLOAT && params[1].a_type == A_FLOAT)
                                dklmunge_rpt1(x, mlen, mungee, paramlen, params);
                         };
                         break;
            case SUM:
                         dklmunge_sum(x, mlen, mungee);
                         break;
            case PROD:
                         dklmunge_prod(x, mlen, mungee);
                         break;
            case AVG:
                         dklmunge_avg(x, mlen, mungee);
                         break;

            default:
                      break;

        };
    };
}

static void dklmunge_munge(t_dklmunge *x, t_float f)
{
    x->x_munge = f <= 0 ? 0 : 1; 
}

static void *dklmunge_new(t_symbol *s, int argc, t_atom * argv){
	t_dklmunge *x = (t_dklmunge *)pd_new(dklmunge_class);
	x->x_dkmungee = dkmem_new();
	x->x_dkparam = dkmem_new();
        x->x_munger = NONE;
        x->x_dkrnd = dkrnd_new(0);
        x->x_munge = 1;
        x->x_mungeelen = 0;
        x->x_paramlen = 0;
	x->x_mungeelet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("mungee"));
	x->x_mungerlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("munger"));
	x->x_mungelet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("munge"));
	x->x_munged = outlet_new(&x->x_obj, &s_anything);
	return (x);
}

static void *dklmunge_free(t_dklmunge *x){
        dkmem_free(x->x_dkmungee);
        dkmem_free(x->x_dkparam);
        dkrnd_free(x->x_dkrnd);
        inlet_free(x->x_mungeelet);
        inlet_free(x->x_mungerlet);
        inlet_free(x->x_mungelet);
	outlet_free(x->x_munged);
	
	return (void *)x;
}


void dklmunge_setup(void){
	dklmunge_class = class_new(gensym("dklmunge"), (t_newmethod)dklmunge_new, (t_method)dklmunge_free,
			sizeof(t_dklmunge), 0, A_GIMME, 0);
	class_addbang(dklmunge_class, dklmunge_router);
    class_addmethod(dklmunge_class, (t_method)dklmunge_munger, gensym("munger"), A_GIMME, 0);
    class_addmethod(dklmunge_class, (t_method)dklmunge_mungee, gensym("mungee"), A_GIMME, 0);
    class_addmethod(dklmunge_class, (t_method)dklmunge_munge, gensym("munge"), A_FLOAT, 0);
}
