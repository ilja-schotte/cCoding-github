
#include <stdio.h>
#include <stdlib.h>






int main( int argc, char *argv[]){


    int idx;
    int zeilen = 10;
    int spalten = 2;
    double **zgr;
    

    zgr = (double**) malloc(zeilen * sizeof(double*));
    for (idx=0; idx<zeilen; idx++){
    
        zgr[idx] = (double*) malloc(spalten * sizeof(double));
    
    }
    
    for (idx=0; idx<zeilen; idx++){
    
        
        zgr[idx][0] = 0;
        zgr[idx][1] = 1;
        
    }
    
    
    
    
    for (idx=0; idx<zeilen; idx++){
        free(zgr[idx]);
    }
    free(zgr);




    return 0;

}
