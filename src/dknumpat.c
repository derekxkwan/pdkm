/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */

#include "m_pd.h"
#include <stdlib.h>

//taken from puredata's src/x_list.c

#ifndef HAVE_ALLOCA     /* can work without alloca() but we never need it */
#define HAVE_ALLOCA 1
#endif
#define TEXT_NGETBYTE 100 /* bigger that this we use alloc, not alloca */
#if HAVE_ALLOCA
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)((n) < TEXT_NGETBYTE ?  \
        alloca((n) * sizeof(t_atom)) : getbytes((n) * sizeof(t_atom))))
#define ATOMS_FREEA(x, n) ( \
    ((n) < TEXT_NGETBYTE || (freebytes((x), (n) * sizeof(t_atom)), 0)))
#else
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)getbytes((n) * sizeof(t_atom)))
#define ATOMS_FREEA(x, n) (freebytes((x), (n) * sizeof(t_atom)))
#endif

static t_class *dknumpat_class;

typedef struct _dknumpat {
	t_object x_obj;
} t_dknumpat;

static void *dknumpat_new(void){
	t_dknumpat *x = (t_dknumpat *)pd_new(dknumpat_class);
	outlet_new(&x->x_obj, &s_list);
	return (x);
}

static void dknumpat_list(t_dknumpat *x, t_symbol *s, int argc, t_atom * argv){
   int total = 0;
   int cursize = argc;
   int curidx = 0;
   int i;
   t_atom * outlist = (t_atom *)getbytes(cursize * sizeof(t_atom));
    while(argc){
        if(argv -> a_type == A_FLOAT){
            int curint = (int)atom_getfloatarg(0, argc, argv);
            if(curint > 0){
                total += curint;
                if(total > cursize){
                    //if needed, realloc
                    outlist = (t_atom *)resizebytes(outlist, cursize*sizeof(t_atom), total*sizeof(t_atom));
                    cursize = total;
                };
                for(i=0;i<curint; i++){
                    if(i==0){
                        SETFLOAT(&outlist[curidx], (t_float)1.);
                    }
                    else{
                        SETFLOAT(&outlist[curidx], (t_float)0.);
                    };
                    curidx++;
                    
                };
            };
        };
        argc--;
        argv++;
    };
    outlet_list(x->x_obj.ob_outlet, &s_list, curidx, outlist);
    freebytes(outlist, cursize * sizeof(t_atom));
}

void dknumpat_setup(void){
	dknumpat_class = class_new(gensym("dknumpat"), (t_newmethod)dknumpat_new, 0,
			sizeof(t_dknumpat), 0, 0);
    class_addlist(dknumpat_class, dknumpat_list);
}
