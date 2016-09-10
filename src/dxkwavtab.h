/* Copyright 2016 - Derek Kwan
 *  * Distributed under GPL v3 */


#include "m_pd.h"
#include <math.h>

#ifndef DXKTABLEN
#define DXKTABLEN 4096//size of saw wavetable
#endif

#ifndef DXK2PI
#define DXK2PI 2.f*acos(-1.f)
#endif

#ifndef DXKTABDEP
#define DXKTABDEP 500 //number of steps to compute saw
#endif

#ifndef DXKPI
#define DXKPI 3.1415926535897932384626433832795028841971693993751
#endif

#ifndef DXKWINLEN
#define DXKWINLEN 1024 //length of window
#endif

#ifndef DXKFFTOV
#define DXKFFTOV 4 //fft overlap
#endif

double dxkgetlin(double tab[], int sz, double idx); //linear interpolator
