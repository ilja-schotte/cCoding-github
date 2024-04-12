
#ifdef __unix__
    #include <stdio.h>
    #include "is_regression.h"
    #include <time.h>
#endif





void ausgabe(double **input_data, int length);



int main(){

    int idx, jdx;

    int err = 0;
    int length = 10;
    int order = 4;
    double **input_data;
    double result;
    srand((unsigned)time(NULL));
    
    
    input_data = (double**) calloc(length, sizeof(double*));
    if (input_data == NULL){
        printf("Speicherfehler.\n");
    }
    else{
        for (idx=0; idx<length; idx++){
            
            input_data[idx] = (double *) calloc(2, sizeof(double));
            if (input_data[idx] == NULL){
                printf("Speicherfehler.\n");
            }
        }
    }
    
    input_data[0][0] = 1;
    input_data[1][0] = 3; 
    input_data[2][0] = 4; 
    input_data[3][0] = 8; 
    input_data[4][0] = 11;
    input_data[5][0] = 12;
    input_data[6][0] = 15;
    input_data[7][0] = 20;
    input_data[8][0] = 21;
    input_data[9][0] = 24;    
    
    
    input_data[0][1] = 2;
    input_data[1][1] = 5; 
    input_data[2][1] = 7; 
    input_data[3][1] = 8; 
    input_data[4][1] = 9;
    input_data[5][1] = 7;
    input_data[6][1] = 5;
    input_data[7][1] = 2;
    input_data[8][1] = 3;
    input_data[9][1] = 5;   
    
        
    ausgabe(input_data, length);
    
    polynomial_regression(input_data, length, order);


    

    return 0;
}





void ausgabe(double **input_data, int length){

    int idx, jdx;
    printf("Input data:\n");
    for (idx=0; idx<length; idx++){
    
        printf("%d:%7.3f %7.3f\n", idx, input_data[idx][0], input_data[idx][1]);

    }
    printf("=================\n");
}






















