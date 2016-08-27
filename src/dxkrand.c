/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include "dxkrand.h"

//0x7ffffff = 2^31

double dxkrand_next(dxkrand * x){
	double nval;
	double range = x->d_range;
        unsigned int seed = x->d_seed;
	seed = ((seed * 2891336453 + 1500450271) % 4294967296) & 0x7fffffff;
        x->d_seed = seed;
	nval = range*(double)seed*(2.0/4294967296); //divide by 2^31
	if(nval >= range){
		nval = range;
	};
        return nval;
}

void dxkrand_free(dxkrand * x){
    free(x);
};
void dxkrand_range(dxkrand * x, double _range){
    double range = range;
    if(range < 0){
        range = 0;
    };
    x->d_range = range;
}

void dxkrand_seed(dxkrand * x, unsigned int _seed){
    x->d_seed = _seed;
}

dxkrand * dxkrand_new(int num, ... ){
    //num of args passed, 1 = range, 2 = seed
    dxkrand * x;
    x = (dxkrand *)malloc(sizeof(dxkrand));
    if(x){
        double range = 1.f;
        unsigned int seed = 1997333137;
        if(num){
            int i;
            va_list args;
            va_start(args,num);
            for(i=0; x<num; i++){
                switch(i){
                    case 0: 
                        range = va_arg(args,double);
                        break;
                    case 1:
                        seed = va_arg(args,unsigned int);
                        break;
                    default:
                        break;
                    };
                };
            va_end(args);
            };
        dxkrand_range(x, range);
        dxkrand_seed(x, seed);
        return (x);
        }
    else{
        return NULL;
        };
    }
