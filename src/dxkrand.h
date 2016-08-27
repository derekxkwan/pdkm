/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include <stdarg.h>
#include <stdlib.h>

typedef struct _dxkrand{
    double d_range;
    unsigned int d_seed;
} dxkrand;

double dxkrand_next(dxkrand * x);
dxkrand * dxkrand_new(int args, ...);
void dxkrand_free(dxkrand * x);
void dxkrand_range(dxkrand * x, double _range);
void dxkrand_seed(dxkrand * x, unsigned int _seed);
