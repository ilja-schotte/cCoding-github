
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <stdbool.h>
    #include <time.h>
#endif





void crout(double **matrix, double **L, double **U, int n);

int main(){


    double **matrix, **L, **U;
    long double det = 1;
    int rang;
    int idx, jdx, kdx;

    
    rang = 178;
    
    srand((unsigned)time(NULL));
    
    matrix = (double **) calloc(rang, sizeof(double*));
    L = (double **) malloc(rang * sizeof(double*));
    U = (double **) malloc(rang * sizeof(double*));
      
    if (matrix == NULL){
    
        printf("Speicherfehler: rows\n");
        return 1;
    }
    else{
    
        for (idx=0; idx<rang; idx++){
        
            matrix[idx] = (double*) calloc(rang, sizeof(double));
            L[idx] = (double*) malloc(rang * sizeof(double));
            U[idx] = (double*) malloc(rang * sizeof(double)); 
                   
            if (matrix[idx] == NULL){
        
                printf("Speicherfehler: cols\n");
                return 1;
        
            }
        }
    }
    
    
    
    // Matrix füllen:
    for (idx=0; idx<rang; idx++){
    
        for (kdx=0; kdx<rang; kdx++){
        
            matrix[idx][kdx] = (rand()%100+1)/100.0;
        
        }
    }
    
    
    
    // Matrix zeigen:
    /*for (idx=0; idx<rang; idx++){
    
        for (kdx=0; kdx<rang; kdx++){
        
            printf("%4.1f ",matrix[idx][kdx]);
        
        }
        printf("\n");
    }
    printf("\n\n");*/
    
    
    //####################################################################
    //####################################################################    
    
    
    // Berechne die Determinante:
    crout(matrix, L, U, rang);
    
    
    

    
    for (idx=0; idx<rang; idx++){
    
        free(matrix[idx]);
        free(L[idx]);        
        free(U[idx]);
           
    }
    free(matrix);
    free(U);
    free(L);
    
    return 0;

}


void crout(double **matrix, double **L, double **U, int n){

    int idx, jdx, kdx;
    
    double sum = 0;
    
    double DetL = 1;
    double DetU = 1;

    for (idx=0; idx<n; idx++){
    
        U[idx][idx] = 1;
    }

    for (jdx=0; jdx<n; jdx++){
    
	for (idx=jdx; idx<n; idx++){
	
	    sum=0;
	    
	    for (kdx=0; kdx<jdx; kdx++){
	    
		sum = sum + L[idx][kdx] * U[kdx][jdx];
			
	    }
	    
	    L[idx][jdx] = matrix[idx][jdx] - sum;
	}

	for (idx=jdx; idx<n; idx++){
	
	    sum = 0;
	    
	    for(kdx=0; kdx<jdx; kdx++){
	    
	        sum = sum + L[jdx][kdx] * U[kdx][idx];
	    }
	    if (L[jdx][jdx] == 0){
	    
	        printf("det(L) close to 0!\n Can't divide by 0...\n");
	        exit(EXIT_FAILURE);
	    }
	    U[jdx][idx] = (matrix[jdx][idx] - sum) / L[jdx][jdx];
	}
    }
    
    /*for (idx=0; idx<n; idx++){
    
        for (jdx=0; jdx<n; jdx++){
        
            printf("%6.3f ", U[idx][jdx]);
        
        }
        printf("\n");
    
    }
    
    printf("\n");
    
    for (idx=0; idx<n; idx++){
    
        for (jdx=0; jdx<n; jdx++){
        
            printf("%6.3f ", L[idx][jdx]);
        
        }
        printf("\n");
    
    } */   
    
    
    for (idx=0; idx<n; idx++){
    
        for (jdx=0; jdx<n; jdx++){
        
            if (idx == jdx){
            
                DetL = DetL * L[idx][jdx];
                DetU = DetU * U[idx][jdx];
            }
        }
    
    }
    
    printf("%.3f\n", DetL*DetU);
    
}




