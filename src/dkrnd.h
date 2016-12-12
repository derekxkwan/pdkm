/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <stdarg.h>
#include <stdlib.h>

typedef struct _dkrnd{
    double d_range;
    unsigned int d_seed;
} t_dkrnd;

double dkrnd_next(t_dkrnd * x);
t_dkrnd * dkrnd_new(int args, ...);
void dkrnd_free(t_dkrnd * x);
void dkrnd_range(t_dkrnd * x, double _range);
void dkrnd_seed(t_dkrnd * x, unsigned int _seed);
