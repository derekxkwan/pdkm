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

#ifndef DXKPI
#define DXKPI 3.1415926535897932384626433832795028841971693993751
#endif

#ifndef DXKWINLEN
#define DXKWINLEN 1024 //length of window
#endif
