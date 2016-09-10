
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
