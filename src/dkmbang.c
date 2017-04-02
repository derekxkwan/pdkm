/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include "dktime.h"

#define DKMBNG_MAXRPT 1024 //max number of repeats 
#define DKMBNG_TEMPO 125 //default tempo in ms
#define DKMBNG_NDIV 1 //default subdivisions 

static t_class *dkmbang_class;

typedef struct _dkmbang {
	t_object    x_obj;
        t_clock     *x_clock;
        t_float     x_tempo; //main duration in ms
        t_float      x_ndiv; //number of subdivisions, can be fractional
        double      x_divdur; //duration in ms of subdivision
        int         x_count; //number of bangs sent through
        int         x_gate[DKMBNG_MAXRPT]; //whether to let bangs through or not
        int         x_mode; //0 = default, let all bangs through, 1 = defer to x_gate
        int         x_set; //if clock is set
        t_inlet     *x_ftlet;
        t_inlet     *x_ndivlet;
        t_inlet     *x_modelet;
        t_outlet    *x_banglet; //bang outlet
} t_dkmbang;

//clear current queued bangs
static void dkmbang_clear(t_dkmbang *x){
    if(x->x_set){
        clock_unset(x->x_clock);
        x->x_set = 0;
    };
    x->x_count = 0;

}

static void dkmbang_reset(t_dkmbang *x){
//clear, reset to mode 0
    x->x_mode = 0;
    dkmbang_clear(x);

}

static void dkmbang_mode(t_dkmbang *x, t_float f){
    int _mode = f > 0 ? 1 : 0;
    if(_mode != x->x_mode){
        //reset 
        x->x_mode = _mode;
        dkmbang_reset(x);
    };
}

static void dkmbang_list(t_dkmbang *x, t_symbol *s, int argc, t_atom * argv){
    t_float curfloat = 0;
    int i;
    int ndiv = x->x_ndiv;
    int curint = 0;
    for(i=0; i < ndiv; i++){
        if(i < argc){
            //if there are atoms left to parse
            curfloat = atom_getfloatarg(i, argc, argv);
            curint = curfloat > 0 ? 1 : 0;
        }
        else{
            //else just default to 0
            curint = 0;
        };
        x->x_gate[i] = curint;
    };
}

static void dkmbang_ft1(t_dkmbang *x, t_floatarg f){
    if(f < 1){
        f = 1;
    };
    x->x_tempo = f;
    double divdur = (double)x->x_tempo/(double)x->x_ndiv;
    x->x_divdur = divdur > 1 ? divdur : 1.;
    
    //clear bangs (but don't reset mode)
    dkmbang_clear(x);
}

static void dkmbang_tempo(t_dkmbang *x, t_symbol *unitname, t_floatarg tempo)
{
    if(tempo < 1){
        tempo = 1;
    };

    double divdur = (double)tempo/(double)x->x_ndiv;
    divdur = divdur > 1 ? divdur : 1.;
    x->x_divdur = divdur;

    t_float unit;
    int samps;
    parsetimeunits(x, divdur, unitname, &unit, &samps);
    clock_setunit(x->x_clock, unit, samps);
    
    //clear bangs (but don't reset mode)
    dkmbang_clear(x);
}

static void dkmbang_ndiv(t_dkmbang *x, t_float f){
    t_float ndiv = f > 1 ? f : 1;
    if(ndiv > DKMBNG_MAXRPT){
        ndiv = DKMBNG_MAXRPT;
    };

    double divdur = (double)x->x_tempo/(double)ndiv;
    x->x_divdur = divdur > 1 ? divdur : 1.;
    //clear bangs (but don't reset mode)
    dkmbang_clear(x);
}

static void dkmbang_tick(t_dkmbang *x){

    int count = x->x_count; //current count
    post("%d %f", count, x->x_ndiv);
    //if the current gate value is 1 or mode is 0, let through
    if(x->x_gate[count] || !(x->x_mode)){
        outlet_bang(x->x_banglet);
    };

    count++;

    //if there's a next subdivision, set callback, else clear
    if(count < x->x_ndiv){
        post("HI");
        x->x_count = count;
        x->x_set = 1;
        clock_delay(x->x_clock, x->x_divdur);
    }
    else{
        dkmbang_clear(x);
    };
}

static void dkmbang_bang(t_dkmbang *x){
    //reset count
    x->x_count = 0;
    //just pass off to the tick method
    dkmbang_tick(x);
}


static void *dkmbang_new(t_symbol *s, int argc, t_atom * argv){

        int i = 0;
	t_dkmbang *x = (t_dkmbang *)pd_new(dkmbang_class);
        //defaults
	t_symbol * unitname = gensym("msec");
        t_float _tempo = DKMBNG_TEMPO;
        t_float _ndiv = DKMBNG_NDIV;

        //parsing args
        int argnum = 0;
        while(argc){
            if(argv -> a_type == A_FLOAT){
                t_float curf = atom_getfloatarg(0, argc, argv);
                switch(argnum){
                    case 0:
                        _tempo = curf > 1 ? curf : 1;
                        break;
                    case 1:
                        _ndiv = curf > 1 ? curf : 1;
                        if(_ndiv > DKMBNG_MAXRPT){
                            _ndiv = DKMBNG_MAXRPT;
                        };
                        break;
                    default:
                        break;
                };
                argnum++;
            }else if(argv-> a_type == A_SYMBOL){
                unitname = atom_getsymbolarg(0, argc, argv);
            };

            argc--;
            argv++;
        };
        x->x_clock = clock_new(x, (t_method)dkmbang_tick);
        x->x_set = 0; //if a clock is set
        x->x_mode = 0; //default mode is 0

        x->x_tempo = _tempo;
        x->x_ndiv = _ndiv;

        x->x_divdur = divdur;

        //some resetting of members
        dkmbang_clear(x);
        for(i=0;i<DKMBNG_MAXRPT; i++){
            x->x_gate[i] = 0;
        };

        dkmbang_tempo(x, unitname, _tempo);
	
        x->x_ftlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	x->x_ndivlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ndiv"));
	x->x_modelet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("mode"));
        x->x_banglet = outlet_new(&x->x_obj, &s_bang);
	return (x);
}

static void *dkmbang_free(t_dkmbang *x){
    dkmbang_clear(x);
    inlet_free(x->x_ftlet);
    inlet_free(x->x_ndivlet);
    inlet_free(x->x_modelet);
    outlet_free(x->x_banglet);

    clock_free(x->x_clock);
    return (void *)x;
}

void dkmbang_setup(void){
	dkmbang_class = class_new(gensym("dkmbang"), (t_newmethod)dkmbang_new, (t_method)dkmbang_free,
			sizeof(t_dkmbang), 0, A_GIMME, 0);
	class_addbang(dkmbang_class, dkmbang_bang);
	class_addlist(dkmbang_class, (t_method)dkmbang_list);
	class_addmethod(dkmbang_class, (t_method)dkmbang_tempo, gensym("tempo"), A_FLOAT, A_SYMBOL, 0);
	class_addmethod(dkmbang_class, (t_method)dkmbang_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(dkmbang_class, (t_method)dkmbang_ndiv, gensym("ndiv"), A_FLOAT, 0);
	class_addmethod(dkmbang_class, (t_method)dkmbang_mode, gensym("mode"), A_FLOAT, 0);
	class_addmethod(dkmbang_class, (t_method)dkmbang_reset, gensym("reset"), 0);
	class_addmethod(dkmbang_class, (t_method)dkmbang_clear, gensym("clear"), 0);
}
