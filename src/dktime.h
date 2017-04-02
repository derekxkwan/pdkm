//copied from pure data's x_time

#include "m_pd.h"
#include <string.h>

void parsetimeunits(void *x, t_float amount, t_symbol *unitname,
    t_float *unit, int *samps);
