/* Copyright 2016 by Derek Kwan
 * Distributed under GPL v3 */


//parsetimeunits is copied from puredata's x_time.c

	/* parse a time unit such as "5 msec", "60 perminute", or "1 sample" to
    a form usable by clock_setunit)( and clock_gettimesincewithunits().
    This brute-force search through symbols really ought not to be done on
    the fly for incoming 'tempo' messages, hmm...  This isn't public because
    its interface migth want to change - but it's used in x_text.c as well
    as here. */

/*idea is to have a base unit for clock call back for have multiples of that unit for bangs.
 each multiple gets their own inlet and outlet, and thus need to copy selectelement code
 from select in x_connective.c

 inlet 0 = on/off, inlet 1-n = set multiple (int), inlet n+1=set base unit
 outlet 0-n-1 = outlet bangs
 */

//define defaults for ft1, tempo

#include "m_pd.h"
#include <string.h>
#define MTXFT 1 
#define MTXTPO 1

void parsetimeunits(void *x, t_float amount, t_symbol *unitname,
    t_float *unit, int *samps)
{
    char *s = unitname->s_name;
    if (amount <= 0)
        amount = 1;
    if (s[0] == 'p' && s[1] == 'e' && s[2] == 'r')  /* starts with 'per' */
    {
        char *s2 = s+3;
        if (!strcmp(s2, "millisecond") || !strcmp(s2, "msec"))  /* msec */
            *samps = 0, *unit = 1./amount;
        else if (!strncmp(s2, "sec", 3))        /* seconds */
            *samps = 0, *unit = 1000./amount;
        else if (!strncmp(s2, "min", 3))        /* minutes */
            *samps = 0, *unit = 60000./amount;
        else if (!strncmp(s2, "sam", 3))        /* samples */
            *samps = 1, *unit = 1./amount;
        else goto fail;
    }
    else
    {
            /* empty string defaults to msec */
        if (!strcmp(s, "millisecond") || !strcmp(s, "msec"))
            *samps = 0, *unit = amount;
        else if (!strncmp(s, "sec", 3))
            *samps = 0, *unit = 1000.*amount;
        else if (!strncmp(s, "min", 3))
            *samps = 0, *unit = 60000.*amount;
        else if (!strncmp(s, "sam", 3))
            *samps = 1, *unit = amount;
        else
        {
        fail:
                /* empty time unit specification defaults to 1 msec for
                back compatibility, since it's possible someone threw a
                float argument to timer which had previously been ignored. */
            if (*s)
                pd_error(x, "%s: unknown time unit", s);
            else pd_error(x,
                "tempo setting needs time unit ('sec', 'samp', 'permin', etc.");
            *unit = 1;
            *samps = 0;
        }
    }
}


/* -------------------------- metro ------------------------------ */
static t_class *dkmtx_class;

typedef struct _metroxelt
{
    //t_word e_w;
    t_outlet *e_outlet;
	t_inlet *e_inlet;
	t_float e_mult; //multiple of the timebase
	int e_count; //the current count
} t_dkmtxelt;


typedef struct _dkmtx
{
    t_object x_obj;
    t_clock *x_clock;
    double x_deltime;
    int x_hit;
	t_inlet * x_baselet; //inlet for callback time base

	t_int x_numelt; //number of metroxelts
	t_dkmtxelt *x_vec;
} t_dkmtx;

static void dkmtx_ft1(t_dkmtx *x, t_floatarg g)
{
    if (g <= 0) /* as of 0.45, we're willing to try any positive time value */
        g = 1;  /* but default to 1 (arbitrary and probably not so good) */
    x->x_deltime = g;
}

static void dkmtx_tick(t_dkmtx *x)
{
    t_dkmtxelt *e;
    int numelt = x->x_numelt;
    x->x_hit = 0;

	for (numelt, e = x->x_vec + (numelt - 1); numelt > 0; numelt--, e--){
		//iterating through x_dxkmetroelts in backwards dir
        //bounds checking low side for multiplier
		if(e->e_mult < 1){
			e->e_mult = 1;
		};

		if(e->e_count == 0){
			//if the count is 0, bang
			outlet_bang(e->e_outlet);
		};
		e->e_count = e->e_count + 1; //increment count

		if(e->e_count >= e->e_mult){
			//if bigger or equal than mult, set back to 0
			e->e_count = 0;
		};

	};

    if (!x->x_hit) clock_delay(x->x_clock, x->x_deltime);
}

static void dkmtx_float(t_dkmtx *x, t_float f)
{
    t_dkmtxelt *e;
    int numelt;
	for (numelt = 0, e = x->x_vec; numelt < x->x_numelt; numelt++, e++){
		//reset all counts to 0
		e->e_count = 0;
	};
    if (f != 0) dkmtx_tick(x);
    else clock_unset(x->x_clock);
    x->x_hit = 1;
}

static void dkmtx_bang(t_dkmtx *x)
{
    dkmtx_float(x, 1);
}

static void dkmtx_stop(t_dkmtx *x)
{
    dkmtx_float(x, 0);
}

static void dkmtx_tempo(t_dkmtx *x, t_symbol *unitname, t_floatarg tempo)
{
    t_float unit;
    int samps;
    parsetimeunits(x, tempo, unitname, &unit, &samps);
    clock_setunit(x->x_clock, unit, samps);
}

static void dkmtx_free(t_dkmtx *x)
{
    t_dkmtxelt *e;
    int numelt;

    clock_free(x->x_clock);
	for (numelt = 0, e = x->x_vec; numelt < x->x_numelt; numelt++, e++){
		//freeing individual inlets and outlets
		outlet_free(e->e_outlet);
		inlet_free(e->e_inlet);
	};
    freebytes(x->x_vec, x->x_numelt * sizeof(*x->x_vec));
	inlet_free(x->x_baselet); //freeing the last inlet
}

static void *dkmtx_new(t_symbol *s, int argc, t_atom * argv)
{

    t_dkmtx *x = (t_dkmtx *)pd_new(dkmtx_class);
    
	//PARSING TEMPO
	
	/*two options for parsing tempo:
	 * one where one arg is given for tempo (arg for ft1), which is then ms
	 * two is where 3 args are given:
	 * 1. mult = arg for ft1, 2. tempo, 3. timeunit
	 */

	//setting defaults
	t_float ft1arg = MTXFT;
	t_float tempo = MTXTPO;
	t_symbol * unitname;
	switch(argc < 3){
		case 0: // bigger than 3 args, can check type of third arg
			if((argv+2)->a_type == A_SYMBOL){
				//this is the second case: mult, tempo, timeunit
				ft1arg = atom_getfloatarg(0, argc, argv);
				tempo = atom_getfloatarg(1, argc, argv);
				unitname = atom_getsymbolarg(2, argc, argv);
				argc-=3;
				argv+=3;
				break;
			};
			//otherwise, we're in the first case
		case 1: //smaller than three args
			//also ends up here if case 0 doesn't hit if and break
			if(argc >= 1){
				//if there's actually args
				//and first is float,parse it as ft1
				if(argv->a_type == A_FLOAT){
					ft1arg = atom_getfloatarg(0, argc, argv);
				};
				//increment/dec in either case
					argc--;
					argv++;
			};
			//else we're sticking with the defaults
			//and setting unit name as msec
				unitname = gensym("msec");
				break;
		default:
				break;
	};

	//NOW FOR PARSING THE REST
    
	t_dkmtxelt *e;
	if(argc < 1){
		//if there's nothing left to parse
		//just default to normal-ish behavior and make one metroxelt
		x->x_numelt = 1;
		e = x->x_vec = (t_dkmtxelt *)getbytes(sizeof(*x->x_vec));
		e->e_mult = 1;
		e->e_outlet = outlet_new(&x->x_obj, &s_bang);
		e->e_inlet = floatinlet_new(&x->x_obj, &e->e_mult);
		e->e_count = 0;


	}
	else{
		//parse the rest of the args as metroxelts
		int n;
		x->x_numelt = argc;
		x->x_vec = (t_dkmtxelt *)getbytes(argc * sizeof(*x->x_vec));

        for (n = 0, e = x->x_vec; n < argc; n++, e++)
        {
			e->e_mult = 1; //default in case arg is not float
			if((argv+n)->a_type == A_FLOAT){
				e->e_mult = atom_getfloatarg(n, argc, argv);
			};
			//bounds checking
			if(e->e_mult< 1){
				e->e_mult = 1;
			};
			e->e_inlet = floatinlet_new(&x->x_obj, &e->e_mult);
            e->e_outlet = outlet_new(&x->x_obj, &s_bang);
			e->e_count = 0;
        };
	};
    
	dkmtx_ft1(x, ft1arg);
    x->x_hit = 0;
    x->x_clock = clock_new(x, (t_method)dkmtx_tick);
    x->x_baselet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));

	if (tempo != 0)
        dkmtx_tempo(x, unitname, tempo);
    return (x);
}

void dkmtx_setup(void)
{
    dkmtx_class = class_new(gensym("dkmtx"), (t_newmethod)dkmtx_new,
        (t_method)dkmtx_free, sizeof(t_dkmtx), 0,
            A_GIMME, 0);
    class_addbang(dkmtx_class, dkmtx_bang);
    class_addmethod(dkmtx_class, (t_method)dkmtx_stop, gensym("stop"), 0);
    class_addmethod(dkmtx_class, (t_method)dkmtx_ft1, gensym("ft1"),
        A_FLOAT, 0);
    class_addmethod(dkmtx_class, (t_method)dkmtx_tempo,
        gensym("tempo"), A_FLOAT, A_SYMBOL, 0);
    class_addfloat(dkmtx_class, (t_method)dkmtx_float);
}

