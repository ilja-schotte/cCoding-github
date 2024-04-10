#include <stdio.h>
#include <stdlib.h>



struct dataset{

    double **data;

};

int allocate_memory(double ***matrix, int rows, int cols);

int main(){

    double **a;
    
    struct dataset dd;
   
    printf("%p\n",dd.data);
    allocate_memory(&(dd.data), 5, 3);
    printf("%p\n",dd.data);
    
    return 0;

}





int allocate_memory(double ***matrix, int rows, int cols){

    int idx, jdx;
    double **ptr;

    ptr = (double**) calloc(rows, sizeof(double*));
    if (ptr == NULL){
        printf("Speicherfehler\n");
    }
    else{
        for (idx=0; idx<rows; idx++){
        
            ptr[idx] = (double*) calloc(cols, sizeof(double));
            if (ptr[idx] == NULL){
                printf("Speicherfeler\n");
            }
        }
    }
    *matrix = ptr;
    printf("%p\n", *matrix);
}
