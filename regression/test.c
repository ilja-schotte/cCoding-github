
#ifdef __unix__
    #include <stdio.h>
    #include "is_regression.h"
#endif





void ausgabe(double **input_data, int length);



int main(void){

    int idx, jdx;

    int err = 0;
    int length = 13;
    int order = 3;
    double **input_data;
    double result;
    
    
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
    input_data[1][0] = 0; 
    input_data[2][0] = 3; 
    input_data[3][0] = 3; 
    input_data[4][0] = 4;
    input_data[5][0] = 2;
    input_data[6][0] = 5;
    input_data[7][0] = 10;
    input_data[8][0] = 9;
    input_data[9][0] = 6; 
    input_data[10][0] = 1;
    input_data[11][0] = 10;       
    input_data[12][0] = 8;    
    
    input_data[0][1] = 14;
    input_data[1][1] = 8; 
    input_data[2][1] = 30; 
    input_data[3][1] = 48; 
    input_data[4][1] = 49;
    input_data[5][1] = 24;
    input_data[6][1] = 81;
    input_data[7][1] = 45;
    input_data[8][1] = 83;
    input_data[9][1] = 99;   
    input_data[10][1] = 8;
    input_data[11][1] = 35;
    input_data[12][1] = 100;    
        
    ausgabe(input_data, length);
    
    err = polynomial_regression(input_data, length, order, &result);


    

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






















