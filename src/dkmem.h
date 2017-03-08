/* Copyright 2017 - Derek Kwan
 * Distributed under GPL v3 */

#include <stdarg.h>
#include <stdlib.h>
#include "m_pd.h"

#define DKMEM_STACK 128 //stack size
#define DKMEM_MAX 4096 //maximum allocated size
typedef struct _dkmem{
    t_atom * m_data;
    t_atom   m_stack[DKMEM_STACK];
    int      m_heaped; //if allocated
    int      m_allocsz; //allocated size
    int      m_storesz; //stored size
} t_dkmem;

t_dkmem * dkmem_new(void);
void dkmem_free(t_dkmem * m);
void dkmem_alloc(t_dkmem * m, int wantsz);
