/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <stdarg.h>
#include <stdlib.h>

typedef struct _dkrnd{
    double d_range;
    unsigned int d_seed;
} dkrnd;

double dkrnd_next(dkrnd * x);
dkrnd * dkrnd_new(int args, ...);
void dkrnd_free(dkrnd * x);
void dkrnd_range(dkrnd * x, double _range);
void dkrnd_seed(dkrnd * x, unsigned int _seed);
