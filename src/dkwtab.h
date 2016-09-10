/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include "m_pd.h"
#include <math.h>

#ifndef TABLEN
#define TABLEN 4096//size of saw wavetable
#endif

#ifndef TPI
#define TPI 2.f*acos(-1.f)
#endif

#ifndef TABDEP
#define TABDEP 500 //number of steps to compute saw
#endif

#ifndef OPI
#define OPI 3.1415926535897932384626433832795028841971693993751
#endif

#ifndef WINLEN
#define WINLEN 1024 //length of window
#endif

#ifndef FFTOV
#define FFTOV 4 //fft overlap
#endif

double dkgetlin(double tab[], int sz, double idx); //linear interpolator