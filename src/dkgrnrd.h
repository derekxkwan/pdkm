#include "m_pd.h"
#include <math.h>
#include <string.h>
#include "dkbuf.h"
#include "dkrnd.h"

#ifndef DKGNYNUMGR //number of grains
#define DKGNYNUMGR 300
#endif

#ifndef DKGNYM //window size
#define DKGNYM 8192
#endif

#ifndef DKGNYOV //overlap
#define DKGNYOV 64
#endif

#ifndef DKGNYTP //transposition
#define DKGNYTP 0.f
#endif

#ifndef DKGNYAMP //amp
#define DKGNYAMP 1.f
#endif

#ifndef DKGNY2PI // 2 pi
#define DKGNY2PI 6.283185307179586f
#endif 

#ifndef DKGNYR //r val for tukey window
#define DKGNYR 0.25f
#endif 

#ifndef DKGNYRNDSZ //default of random sizes, yes
#define DKGNYRNDSZ 1
#endif

#ifndef DKGNYRNDAMP //default of random amps, yes
#define DKGNYRNDAMP 1
#endif

#ifndef DKGNYRNDTP //default of random transpositions, no
#define DKGNYRNDTP 0
#endif

#ifndef DKGNYGRATEMS //default grain rate in ms
#define DKGNYGRATEMS 5.f
#endif


#ifndef DXXKGNYRSZAMT //default random size amount in ms
#define DKGNYRSZAMT 10.0f
#endif

#ifndef DKGNYRAMPAMT //default random amp amount
#define DKGNYRAMPAMT 0.1f
#endif

#ifndef DKGNYRTPAMT //default random transposition amount
#define DKGNYRTPAMT 0.01f
#endif

#ifndef DKGNYPOSVAR //position in sample variance in ms.
#define DKGNYPOSVAR 1.0f
#endif

#ifndef DKGNYHOPVAR //hop variance as a proportion of grainrate
#define DKGNYHOPVAR 0.5f
#endif

#ifndef DKGNYENV //env type
#define DKGNYENV 0 //0 = hanning, 1 = tukey
#endif
