
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <stdbool.h>
    #include <time.h>
#endif


void showMatrix(double **matrix, int order);
void InverseOfMatrix(double **matrix, double **matrixInv, int order);

int main(){


    double **matrix, **matrixInv;

    int order = 3;
    
    int idx, jdx, kdx;

    

    
    srand((unsigned)time(NULL));
    
    matrix = (double **) calloc(order, sizeof(double*));
    matrixInv = (double **) calloc(order, sizeof(double*));

    
    
    if (matrix == NULL){
    
        printf("Speicherfehler\n");
        return 1;
    }
    else{
    
        for (idx=0; idx<order; idx++){
        
            matrix[idx] = (double*) calloc(order, sizeof(double));
            matrixInv[idx] = (double*) calloc(order, sizeof(double));
                   
            if (matrix[idx] == NULL){
        
                printf("Speicherfehler\n");
                return 1;
        
            }
        }
    }
    
    
    
    // Matrix füllen:
    for (idx=0; idx<order; idx++){
    
        for (kdx=0; kdx<order; kdx++){
        
            matrix[idx][kdx] = (rand()%10)/10.0;
        
        }
    }
    

    
    //####################################################################
    //####################################################################    
    
    // Zeige:
    showMatrix(matrix, order);   
    
    // Berechne Inverse:
    InverseOfMatrix(matrix, matrixInv, order);
    
    // Zeige:    
    showMatrix(matrixInv, order);
    
    printf("Fertig.\n");
    
    for (idx=0; idx<order; idx++){
        
        free(matrix[idx]);
        free(matrixInv[idx]);        
           
    }
    
    free(matrix);
    printf("Matrix freigegeben\n");
    
    free(matrixInv);
    printf("Inverse freigegeben\n");
      
    
    return 0;

}








void showMatrix(double **matrix, int order){

    int idx, kdx;

    // Matrix zeigen:
    for (idx=0; idx<order; idx++){
    
        for (kdx=0; kdx<order; kdx++){
        
            printf("%6.3f ",matrix[idx][kdx]);
        
        }
        printf("\n");
    }
    printf("\n\n");
}




void InverseOfMatrix(double **matrix, double **matrixInv, int order){


    int idx, jdx, kdx;
    double temp;

    for(idx=0; idx<order; idx++){
    				
        for(jdx=0; jdx<order; jdx++){								
	
	    if(idx == jdx){										
	        matrixInv[idx][jdx] = 1;
	    }									
	    else{
	        matrixInv[idx][jdx] = 0;
	    }
	}
    }
    

    for(kdx=0 ;kdx<order; kdx++){
    														
	temp=matrix[kdx][kdx];
											
        for(jdx=0; jdx<order; jdx++){
        
	    matrix[kdx][jdx] /= temp;
	    matrixInv[kdx][jdx] /= temp;
	}
														
	for(idx=0; idx<order; idx++){
	
	    temp=matrix[idx][kdx];
	    									
	    for(jdx=0; jdx<order; jdx++){
	    												
		if(idx == kdx){
		    break;
		}									
		matrix[idx][jdx] -= matrix[kdx][jdx] * temp;
								
		matrixInv[idx][jdx] -= matrixInv[kdx][jdx] * temp;
	    }
	}
    }
}















