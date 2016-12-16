//essentially taken from my work of abstracting out the buffer bits of cyclone's arsic
//(credit to the original authors Czaja/Kraan/Steiner) but making it single array
//and cleaning up code and renaming things to make things clearer as to their function

//GPL v3

#include "dkbuf.h"

//basically triggers a graphical redraw if it can find the proper array
void dkbuf_redraw(t_dkbuf *b)
{
    if ( b->b_bufname != &s_)
    {
	//tries to find array by looking up its symbol
        t_garray *ap = (t_garray *)pd_findbyclass(b->b_bufname, garray_class);
	if(ap)
        {
            garray_redraw(ap);
        }
	else if (b->b_vec){
            //there's something there and we can't redraw it?! uh oh
            pd_error(b->b_owner, "dkbuf: cannot redraw!");
        };

    };
}



t_word *dkbuf_loadbuf(t_dkbuf *b, t_symbol * name, int *bufsize, int indsp, int complain){
//in dsp = used in dsp, 
  
    if (name && name != &s_){
        //basically trying to find an array by looking up its symbol
	t_garray *arraypointer = (t_garray *)pd_findbyclass(name, garray_class);
	if (arraypointer){
	    int bufsz;
	    t_word *vec;

            //checks if the array is floats, calls garray_npoints to set bufsz, garray_vec to get contents
	    if (garray_getfloatwords(arraypointer, &bufsz, &vec)){
   	        //b->b_len = garray_npoints(ap);

                //used to set flag of garray struct saying we're gonna use this in dsp
		if (indsp){
                    garray_usedindsp(arraypointer);
                };
		if (bufsize){
                    *bufsize = bufsz;
                };
		return (vec);
	    }
	     else pd_error(b->b_owner,  /* always complain */
			"bad template of array '%s'", name->s_name);
        }
	else{
            if(complain){
	        pd_error(b->b_owner, "no such array '%s'", name->s_name);
            };
	};
    }
    return (0);
}

//essentially zeros everything
void dkbuf_clear(t_dkbuf *b)
{
    b->b_npts = 0;
}

//cheecks if we can play the darn thing (not disabled, larger than minsize
void dkbuf_playcheck(t_dkbuf *b){
    b->b_playable = (!b->b_disabled && b->b_npts >= b->b_minsize);
}

//enable playback
void dkbuf_enable(t_dkbuf *b, t_floatarg f)
{
    b->b_disabled = (f == 0);
    dkbuf_playcheck(b);
}


void dkbuf_setminsize(t_dkbuf *b, int i)
{
    b->b_minsize = i;
}


//essentially clears local vetor and bufsize and tries to load buffer
void dkbuf_loader(t_dkbuf *b, int complain)
{
    dkbuf_clear(b);
    //b->b_npts = SHARED_INT_MAX;
    if (b->b_bufname != &s_)
    {
	b->b_vec= dkbuf_loadbuf(b, b->b_bufname, &b->b_npts, 1, complain);
    };
    //if (b->b_npts == SHARED_INT_MAX){
    //    b->b_npts = 0;
    //};
}

void dkbuf_initarray(t_dkbuf *b, t_symbol *name, int complain)
{   
    //setting array names	
    if (name){
	b->b_bufname = name;
	dkbuf_loader(b, complain);
    };
    dkbuf_playcheck(b);
}

//run in the parent's dsp method, basically if we haven't loaded the array yet, load it
void dkbuf_checkdsp(t_dkbuf *b)
{
    dkbuf_loader(b, 1);
    dkbuf_playcheck(b);

}


//wrapper around dkbuf_initarray so you don't have to pass the complain flag each time
void dkbuf_setarray(t_dkbuf *b, t_symbol *name)
{
   dkbuf_initarray(b, name, 1); 
}

void *dkbuf_init(t_class *owner, t_symbol *bufname)
{
    t_dkbuf *b = (t_dkbuf *)getbytes(sizeof(t_dkbuf));
    t_word *vec;
    if (!bufname){
        bufname = &s_;
    };
    b->b_bufname = bufname;
    vec = (t_word *)getbytes(sizeof(b->b_vec));
    if(!vec){
        return 0;
    };

    b->b_owner = owner;
    b->b_npts = 0;
    b->b_disabled = 0;
    b->b_playable = 0;
    b->b_minsize = 1;
    if(bufname != &s_){
        dkbuf_initarray(b, bufname, 0);
    };
    return (b);
}


void dkbuf_free(t_dkbuf *b)
{
    freebytes(b, sizeof(t_dkbuf));
}

