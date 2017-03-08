#include "dkmem.h"

//value allocation helper function
void dkmem_alloc(t_dkmem * m, int wantsz){
   int cursz = m->m_allocsz;
    int heaped = m->m_heaped;


    if(heaped && wantsz <= DKMEM_STACK){
        //if x_value is pointing to a heap and incoming list can fit into DKMEM_STACK
        //deallocate x_value and point it to stack, update status
        freebytes((m->m_data), (cursz) * sizeof(t_atom));
        m->m_data = m->m_stack;
        m->m_heaped = 0;
        m->m_allocsz = DKMEM_STACK;
        }
    else if(heaped && wantsz > DKMEM_STACK && wantsz > cursz){
        //if already heaped, incoming list can't fit into DKMEM_STACK and can't fit into allocated t_atom
        //reallocate to accomodate larger list, update status
        
        int toalloc = wantsz; //size to allocate
        //bounds checking for maxsize
        if(toalloc > DKMEM_MAX){
            toalloc = DKMEM_MAX;
        };
        m->m_data = (t_atom *)resizebytes(m->m_data, cursz * sizeof(t_atom), toalloc * sizeof(t_atom));
        m->m_allocsz = toalloc;
    }
    else if(!heaped && wantsz > DKMEM_STACK){
        //if not heaped and incoming list can't fit into DKMEM_STACK
        //allocate and update status

        int toalloc = wantsz; //size to allocate
        //bounds checking for maxsize
        if(toalloc > DKMEM_MAX){
            toalloc = DKMEM_MAX;
        };
        m->m_data = (t_atom *)getbytes(toalloc * sizeof(t_atom));
        m->m_heaped = 1;
        m->m_allocsz = toalloc;
    };

}

void dkmem_free(t_dkmem * m){
    if(m->m_heaped) free(m->m_data);
    free(m);
}

t_dkmem * dkmem_new(void)
{

    t_dkmem * m;
    m = (t_dkmem *)malloc(sizeof(t_dkmem));
    m->m_data = m->m_stack;
    m->m_heaped = 0;
    m->m_storesz = 0;
    m->m_allocsz = DKMEM_STACK;
    return m;
}
