#include "m_pd.h"
#include <math.h>
#include <string.h>

#ifndef DXKGNYNUMGR //number of grains
#define DXKGNYNUMGR 300
#endif

#ifndef DXKGNYM //window size
#define DXKGNYM 8192
#endif

#ifndef DXKGNYOV //overlap
#define DXKGNYOV 64
#endif

#ifndef DXKGNYTP //transposition
#define DXKGNYTP 0.f
#endif

#ifndef DXKGNYAMP //amp
#define DXKGNYAMP 1.f
#endif

#ifndef DXKGNY2PI // 2 pi
#define DXKGNY2PI 6.283185307179586f
#endif 

#ifndef DXKGNYR //r val for tukey window
#define DXKGNYR 0.25f
#endif 

#ifndef DXKGNYRNDSZ //default of random sizes, yes
#define DXKGNYRNDSZ 1
#endif

#ifndef DXKGNYRNDAMP //default of random amps, yes
#define DXKGNYRNDAMP 1
#endif

#ifndef DXKGNYRNDTP //default of random transpositions, no
#define DXKGNYRNDTP 0
#endif

#ifndef DXKGNYGRATEMS //default grain rate in ms
#define DXKGNYGRATEMS 5.f
#endif


#ifndef DXXKGNYRSZAMT //default random size amount in ms
#define DXKGNYRSZAMT 10.0f
#endif

#ifndef DXKGNYRAMPAMT //default random amp amount
#define DXKGNYRAMPAMT 0.1f
#endif

#ifndef DXKGNYRTPAMT //default random transposition amount
#define DXKGNYRTPAMT 0.01f
#endif

#ifndef DXKGNYPOSVAR //position in sample variance in ms.
#define DXKGNYPOSVAR 1.0f
#endif

#ifndef DXKGNYHOPVAR //hop variance as a proportion of grainrate
#define DXKGNYHOPVAR 0.5f
#endif

#ifndef DXKGNYENV //env type
#define DXKGNYENV 0 //0 = hanning, 1 = tukey
#endif
