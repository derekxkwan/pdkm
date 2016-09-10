/* Copyright 2016 - Derek Kwan
 * Distributed under GPL v3 */

#include "dkrnd.h"

//0x7ffffff = 2^31

double dkrnd_next(dkrnd * x){
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

void dkrnd_free(dkrnd * x){
    free(x);
};
void dkrnd_range(dkrnd * x, double _range){
    double range = _range;
    if(range < 0){
        range = 0;
    };
    x->d_range = range;
}

void dkrnd_seed(dkrnd * x, unsigned int _seed){
    x->d_seed = _seed;
}

dkrnd * dkrnd_new(int num, ... ){
    //num of args passed, 1 = range, 2 = seed
    dkrnd * x;
    x = (dkrnd *)malloc(sizeof(dkrnd));
    if(x){
        double range = 1.;
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
        dkrnd_range(x, range);
        dkrnd_seed(x, seed);
        return (x);
        }
    else{
        return NULL;
        };
    }
