
#ifdef __unix__
    #include <stdio.h>
    #include <math.h>
#endif
#define EPS 1.0E-6



void calc_cov_values(double *values, double *calc_values, double nugget, double sill, double *distances, double range);
double calc_RSME(double *calc_values, double *values);
int change_sill(double *values, double *calc_values, double *distances, double old_RSME, double *sill, double *nugget, double *range, int *changes);
int change_range(double *values, double *calc_values, double *distances, double old_RSME, double *sill, double *nugget, double *range, int *changes);

int main(){


    int idx, jdx, kdx;

    double nugget = 0.001;
    double sill = 0;
    double sill_wanted;
    
    double range = 0.1;
    double range_wanted;
    
    double RSME;
    double RSME_min;
    int changes = 0;


    double distances[18] = {36.474, 76.380, 126.574, 175.934, 225.506, 275.207, 325.014, 374.265, 
                            423.981, 473.134, 522.761, 573.541, 623.242, 672.347, 719.971, 769.814, 815.043, 857.858};
                            
                            
    double values[18] = {1.722, 5.050, 9.239, 11.588, 12.797, 14.248, 15.752, 16.098, 17.603, 
                         19.127, 22.336, 26.651, 34.659, 42.630, 57.519, 77.137, 87.498, 121.458};


    double calc_values[18];
    
    
    for (idx=1; idx<100; idx++){
    
        for (jdx=1; jdx<100; jdx++){
        
            for (kdx=1; kdx<100; kdx++){
 
            
            
            
            } 
        }
    }
    
    
    
    
    
    
    
    calc_cov_values(values, calc_values, nugget, 0, distances, 1);
    RSME_min = calc_RSME(calc_values, values);

    //sill
    for (sill=0; sill<100; sill+=0.1){
    
        // range
        for(range=1; range<374.3; range+=0.1){
        
            calc_cov_values(values, calc_values, nugget, sill, distances, range);
            RSME = calc_RSME(calc_values, values);
            
            if (RSME < RSME_min){
            
                RSME_min = RSME;
                sill_wanted = sill;
                range_wanted = range;
                continue;
            }
        }
    }
    printf("sill: %.3f\n", sill_wanted);
    printf("range: %.3f\n", range_wanted);
    printf("RSME_min: %.3f", RSME_min);
    
    
    


    
    //change_sill(values, calc_values, distances, RSME, &sill, &nugget, &range, &changes);
    
    

    return 0;
  
}



int change_sill(double *values, 
                   double *calc_values, 
                   double *distances,
                   double old_RSME, 
                   double *sill,
                   double *nugget,
                   double *range,
                   int *changes){

    double new_RSME;
    // Erhöhe den sill um 1:
    *sill += 1;
    
    // Berechne die neuen Werte der Kovarianz:
    calc_cov_values(values, calc_values, *nugget, *sill, distances, *range);
    
    // Berechne den RSME:
    new_RSME = calc_RSME(calc_values, values);
    
    // Ist der neue RSME kleiner als der alte Wert:
    if (new_RSME <= old_RSME){
        
        printf("change_sill: old_RSME: %.3f new_RSME: %.3f range: %.3f sill: %.3f\n", old_RSME, new_RSME, *range, *sill);
        // So starte die Funktion erneut
        change_sill(values, calc_values, distances, new_RSME, sill, nugget, range, changes);
    }
    else{
        *sill-=1;
        *changes += 1;
        if (*changes > 100){
            return 1;
        }
        else{
            printf("changes: %d\n", *changes);
            printf("change_sill: old_RSME: %.3f new_RSME: %.3f range: %.3f sill: %.3f\n", old_RSME, new_RSME, *range, *sill);                    
            change_range(values, calc_values, distances, old_RSME, sill, nugget, range, changes);
        }
    }
}


int change_range(double *values, 
                    double *calc_values, 
                    double *distances,
                    double old_RSME, 
                    double *sill,
                    double *nugget,
                    double *range,
                    int *changes){

    double new_RSME;
    
    // Erhöhe den sill um 1:
    *range += 10;
    
    // Berechne die neuen Werte der Kovarianz:
    calc_cov_values(values, calc_values, *nugget, *sill, distances, *range);
    
    // Berechne den RSME:
    new_RSME = calc_RSME(calc_values, values);
    
    // Ist der neue RSME kleiner als der alte Wert:
    if (new_RSME <= old_RSME){
        printf("change_range: old_RSME: %.3f new_RSME: %.3f range: %.3f sill: %.3f\n", old_RSME, new_RSME, *range, *sill);
        // So starte die Funktion erneut
        change_range(values, calc_values, distances, new_RSME, sill, nugget, range, changes);
    }
    else{
        *range-=10;
        *changes += 1;
        if (*changes > 100){
            return 1;
        }
        else{
            printf("changes: %d\n", *changes);
            printf("change_range: old_RSME: %.3f new_RSME: %.3f range: %.3f sill: %.3f\n", old_RSME, new_RSME, *range, *sill);                   
            change_sill(values, calc_values, distances, old_RSME, sill, nugget, range, changes);
        }
    }
}














void calc_cov_values(double *values, double *calc_values, double nugget, double sill, double *distances, double range){

    int idx;

    for (idx=0; idx<9; idx++){
    
        // Exponential
        //calc_values[idx] = nugget + sill * (1-exp((-1.0*fabs(distances[idx]))/(range/3.0)));
    
        // Gauss:
        calc_values[idx] = nugget + sill * (1-exp((-1.0*fabs(pow(distances[idx],2)))/pow((range/3.0),2)));
    }

}


double calc_RSME(double *calc_values, double *values){


    int idx;
    double qsum=0;
    
    for (idx=0; idx<7; idx++){
    
        qsum += pow((calc_values[idx] - values[idx]),2);
    
    }
    
    return sqrt(qsum/9);

}























 

