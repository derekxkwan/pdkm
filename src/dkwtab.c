#include "dkwtab.h"

//build cosine window wavetable
void dkmakecoswin(){
    int i;
    static int made = 0; //if sin tabe was made already
    if(!made){
        for(i=0;i<TABLEN;i++){
             double idx = (OPI*(double)i)/(double)TABLEN;
            dkcoswin[i] = sin(idx);

        };
        made = 1;
    };
}


//build sine wavetable
void dkmakesintab(){
    int i;
    static int made = 0; //if sin tabe was made already
    if(!made){
        for(i=0;i<TABLEN;i++){
             double idx = (TPI*(double)i)/(double)TABLEN;
            dksintab[i] = sin(idx);

        };
        made = 1;
    };
}

void dkmaketukey(){
    int i;
    static int made = 0; //if table was made already
    if(!made){
	for(i=0; i<TABLEN; i++){//tukey
	    double xval = (double)i/(double)TABLEN;
	    double winval;
	    if(xval < (DKTYR/2.f)){
	        winval =0.5*(1+cos((TPI/DKTYR)*(xval-(DKTYR/2.f))));	
	        }
	    else if(xval > (DKTYR/2.f) && xval < (1-(DKTYR/2.f))){
		winval = 1.;
		}
	    else{
		winval =0.5*(1+cos((TPI/DKTYR)*(xval-1.+(DKTYR/2.f))));	
		};
	    dktytab[i] = winval;

	};
        made = 1;
    };

}


double dkgetlin(double tab[], int sz, double idx){
        double output;
        int tabphase1 = (int)idx;
        int tabphase2 = tabphase1 + 1;
        double frac = idx - (double)tabphase1;
        if(tabphase1 >= (sz - 1)){
                tabphase1 = sz - 1; //checking to see if index falls within bounds
                output = tab[tabphase1];
        }
        else if(tabphase1 < 0){
                tabphase1 = 0;
                output = tab[tabphase1];
            }
        else{
            double yb = tab[tabphase2]; //linear interp
            double ya = tab[tabphase1];
            output = ya+((yb-ya)*frac);
        
        };
        return output;

}
