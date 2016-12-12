
//essentially taken from my work of abstracting out the buffer bits of cyclone's arsic
//(credit to the original authors Czaja/Kraan/Steiner) but making it single array
//and cleaning up code and renaming things to make things clearer as to their function

//GPL v3


#ifndef __dkbuf_H__
#define __dkbuf_H__

#ifdef INT_MAX
#define SHARED_INT_MAX  INT_MAX
#else
#define SHARED_INT_MAX  0x7FFFFFFF
#endif

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

typedef struct _dkbuf {
    void        * b_owner; //parent object
    int         b_npts; //size of buf
    t_symbol    * b_bufname; //name of bound buffer
    t_word      * b_vec;
    int         b_playable; //if array is playable under proper conditions
    int         b_disabled; //flag to disable for various reasons
    int         b_minsize; //minimum size to be playable, depends on interp but usu 1

} t_dkbuf;


void dkbuf_redraw(t_dkbuf *b);
t_word *dkbuf_loadbuf(t_dkbuf *b, t_symbol * name, int *bufsize, int indsp, int complain);
void dkbuf_clear(t_dkbuf *b);
void dkbuf_playcheck(t_dkbuf *b);
void dkbuf_enable(t_dkbuf *b, t_floatarg f);
void dkbuf_setminsize(t_dkbuf *b, int i);
void dkbuf_loader(t_dkbuf *b, int complain);
void dkbuf_initarray(t_dkbuf *b, t_symbol *name, int complain); 
void dkbuf_checkdsp(t_dkbuf *b);
void dkbuf_setarray(t_dkbuf *b, t_symbol *name);
void *dkbuf_init(t_class *owner, t_symbol *bufname);
void dkbuf_free(t_dkbuf *b);

#endif
