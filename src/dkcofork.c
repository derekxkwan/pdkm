/* Copyright 2017 by Derek Kwan
 * Distributed under GPL v3 */


#include "m_pd.h"

static t_class *dkcofork_class;

typedef struct _dkcofork
{
    t_object x_obj;
    t_inlet * x_chooselet;
    t_outlet ** x_outlets; //our outputs
    t_float x_choice; //outlet to spit stuff out of (idx = 0)
    int x_numouts; //number of outets
} t_dkcofork;


static void dkcofork_free(t_dkcofork *x)
{
    freebytes(x->x_outlets, x->x_numouts * sizeof(t_outlet *));
    inlet_free(x->x_chooselet); 
}

static void dkcofork_anything(t_dkcofork *x, t_symbol *s, int argc, t_atom * argv)
{

    int numouts = x->x_numouts;
    int choice = x->x_choice < 0 ? 0 : (x->x_choice >= numouts ? numouts - 1 : (int)x->x_choice);
    if(argc == 1)
    {
        if(argv -> a_type == A_FLOAT)
        {
            outlet_float(x->x_outlets[choice], argv[0].a_w.w_float);
        }
        else if(argv-> a_type == A_SYMBOL)
        {
            outlet_symbol(x->x_outlets[choice], argv[0].a_w.w_symbol);
        }
        else if(argv-> a_type == A_POINTER)
        {
            outlet_pointer(x->x_outlets[choice], argv[0].a_w.w_gpointer);
        };
    }
    else
    {
        outlet_anything(x->x_outlets[choice], s, argc, argv);
    };

}


static void dkcofork_pointer(t_dkcofork *x, t_gpointer *gp)
{

    t_atom at[0];
    SETPOINTER(&at[0], gp);
    dkcofork_anything(x, 0, 1, at);
}
static void dkcofork_symbol(t_dkcofork *x, t_symbol *s)
{
    t_atom at[0];
    SETSYMBOL(&at[0], s);
    dkcofork_anything(x, 0, 1, at);
}

static void dkcofork_list(t_dkcofork *x, t_symbol *s, int argc, t_atom * argv)
{
    dkcofork_anything(x, s, argc, argv);
}

static void dkcofork_float(t_dkcofork *x, t_float f)
{
    t_atom at[0];
    SETFLOAT(&at[0], f);
    dkcofork_anything(x, 0, 1, at);

}

static void *dkcofork_new(t_floatarg f1, t_floatarg f2)
{
    t_dkcofork *x = (t_dkcofork *)pd_new(dkcofork_class);
    int numouts = f1 < 1 ? 1 :  (int) f1;
    int i;
    x->x_numouts = numouts;
    x->x_choice = f2;
    x->x_outlets = (t_outlet **)getbytes(numouts * sizeof(t_outlet *));
    
    x->x_chooselet = floatinlet_new(&x->x_obj, &x->x_choice);
    for(i=0; i< numouts; i++)
    {
    x->x_outlets[i] = outlet_new(&x->x_obj, &s_anything);

    };
    return (x);
}

void dkcofork_setup(void)
{
    dkcofork_class = class_new(gensym("dkcofork"), (t_newmethod)dkcofork_new,
        (t_method)dkcofork_free, sizeof(t_dkcofork), 0,
            A_DEFFLOAT,A_DEFFLOAT, 0);
    class_addfloat(dkcofork_class, dkcofork_float);
    class_addsymbol(dkcofork_class, dkcofork_symbol);
    class_addlist(dkcofork_class, dkcofork_list);
    class_addpointer(dkcofork_class, dkcofork_pointer);
    class_addanything(dkcofork_class, dkcofork_anything);
}

