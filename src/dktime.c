//copied from pure data's x_time.c

#include "dktime.h"

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


