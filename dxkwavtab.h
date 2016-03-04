#include "m_pd.h"
#include <math.h>

#ifndef DXKTABLEN
#define DXKTABLEN 512//size of saw wavetable
#endif

#ifndef DXK2PI
#define DXK2PI 2.f*acos(-1.f)
#endif

#ifndef DXKTABDEP
#define DXKTABDEP 500 //number of steps to compute saw
#endif


