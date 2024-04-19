
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <stdbool.h>
    #include <time.h>
#endif





double determinantOfMatrix(double **matrix, int rang, int k);
double getCofactor(double **matrix, int rang, int f);
double transpose(double **matrix, double **fac, int rang, int r);

int main(){


    double **matrix;
    double ratio;
    long double det = 1;
    int rows, cols;
    int idx, jdx, kdx;

    
    rows = 20;
    cols = 20;
    
    srand((unsigned)time(NULL));
    
    matrix = (double **) calloc(rows, sizeof(double*));
    if (matrix == NULL){
    
        printf("Speicherfehler: rows\n");
        return 1;
    }
    else{
    
        for (idx=0; idx<cols; idx++){
        
            matrix[idx] = (double*) calloc(cols, sizeof(double));
        
            if (matrix[idx] == NULL){
        
                printf("Speicherfehler: cols\n");
                return 1;
        
            }
        }
    }
    
    
    
    // Matrix füllen:
    for (idx=0; idx<rows; idx++){
    
        for (kdx=0; kdx<cols; kdx++){
        
            matrix[idx][kdx] = (rand()%100+1)/10.0;
        
        }
    }
    
    
    
    // Matrix zeigen:
    for (idx=0; idx<rows; idx++){
    
        for (kdx=0; kdx<cols; kdx++){
        
            printf("%4.1f ",matrix[idx][kdx]);
        
        }
        printf("\n");
    }
    printf("\n\n");
    
    
    //####################################################################
    //####################################################################    
    
    
    // Berechne die Determinante:
    det = determinantOfMatrix(matrix, rows, cols);
    
    if (det != 0){
    
        printf("Determinante: %.3Lf\n", det);        
        //getCofactor(matrix, 3, 3); 
    
    }
    else{
    
        printf("Inverse nicht möglich! Determinate gleich 0!\n");
    
    }
    

    


    return 0;

}



double determinantOfMatrix(double **matrix, int rang, int n){

    // matrix	...	Matrix, von der die Determinante bestimmt werden soll.
    // rang	...	Rang der Matrix. Wird benötigt, um ein Feld für die Kofaktoren zu initialisieren.
    // n	...	Anzahl der Elemente in Spalte und Zeile (sollte mei quadratischen Matrizen gleich sein)
  

    int idx, jdx, kdx, ldx;
    int m, k;

    // Initialisiere das Ergebnis:
    double Det = 1;
    
    // Um die Kofaktoren zu speichern
    double **temp;
  
  
    // Erstelle eine Kopie der matrix:
    temp = (double**) malloc(rang * sizeof(double*));
    if (temp == NULL){
    
        printf("Speicherfehler: Determinante (Zeilen).\n");
        return false;
    }
    else{
    
        // Erstelle die Spaltenvektoren:
        for (idx=0; idx<rang; idx++){
        
            temp[idx] = (double*) malloc(rang * sizeof(double));
            if (temp[idx] == NULL){
            
                printf("Speicherfehler: Determinante (Spalten).\n");
                return false;
            }
        }
    }

    
    // Um das Vorzeichen des Multiplikators zu speichern
    int sign = 1; 
    
    //  Beinhaltet die Matrix nur ein Element,
    //  ... dann gib das Element zurück.
    
    if (n == 1){
    
        return (matrix[0][0]);
     
    }
    else{
    
        Det = 0;
        
        for (idx=0; idx<n; idx++){
        
            m = 0;
            k = 0;
            
            for (jdx=0; jdx<n; jdx++){
            
                for (kdx=0; kdx<n; kdx++){
                
                    temp[jdx][kdx] = 0;
                    
                    if (jdx != 0 && kdx != idx){
                    
                        temp[m][k] = matrix[jdx][kdx];
                        
                        if (k < (n - 2)){
                    
                            k++;
                        }
                        else{
                        
                            k = 0;
                            m++;
                        }
                    }
                }
            }
            
            Det = Det + sign * (matrix[0][idx] * determinantOfMatrix(temp, rang, n - 1));
            sign = -1 * sign;
        }
    }
 
    return Det;
    
    // Speicher wieder freigeben:
    for (idx=0; idx<rang; idx++){
    
        free(temp[idx]);
    
    }
    free(temp);
    //printf("Speicher freigegeben (Determinante).\n");
 
    return Det;
}


double getCofactor(double **matrix, int rang, int f){
                 
    double **temp, **fac;    
    
    int pdx, qdx, m, n, idx, jdx;
    
    
    
    // Erstelle die Matrizen "temp" und "fac"
    // Erstelle eine Kopie der matrix:
    temp = (double**) malloc(rang * sizeof(double*));
    if (temp == NULL){
    
        printf("Speicherfehler: Kofaktorenmatrix (Zeilen/temp).\n");
        return false;
    }
    
    fac = (double**) malloc(rang * sizeof(double*));
    if (temp == NULL){
    
        printf("Speicherfehler: Kofaktorenmatrix (Zeilen/fac).\n");
        return false;
    }    
    else{
    
        // Erstelle die Spaltenvektoren:
        for (idx=0; idx<rang; idx++){
        
            temp[idx] = (double*) malloc(rang * sizeof(double));
            if (temp[idx] == NULL){
            
                printf("Speicherfehler: Kovarianzmatrix (Spalten/temp).\n");
                return false;
            }
            fac[idx] = (double*) malloc(rang * sizeof(double));
            if (fac[idx] == NULL){
            
                printf("Speicherfehler: Kovarianzmatrix (Spalten/fac).\n");
                return false;
            }            
        }
    }
    
    // Berechnung:
    for (qdx=0; qdx<f; qdx++){
    
        for (pdx=0; pdx<f; pdx++){
        
            m = 0;
            n = 0;
            
            for (idx=0; idx<f; idx++){
            
                for (jdx=0 ;jdx<f; jdx++){
                
                    if (idx != qdx && jdx != pdx){
                    
                        temp[m][n] = matrix[idx][jdx];
                        
                        if (n < (f - 2)){
                        
                            n++;
                        }
                        else{
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            fac[qdx][pdx] = pow(-1, qdx + pdx) * determinantOfMatrix(temp, rang, f - 1);
        }
    }
    
    
    transpose(matrix, fac, rang, f);    
       
    
    // Speicher wieder freigeben:
    for (idx=0; idx<rang; idx++){
    
        free(temp[idx]);
        free(fac[idx]);
    }
    free(temp);
    free(fac);
    printf("Speicher freigegeben (Kofaktoren).\n");                 
    
}


double transpose(double **matrix, double **fac, int rang, int r){


    int idx, jdx;
    
    double **temp, **inverse;
    double Det;
 
 
    temp = (double**) malloc(rang * sizeof(double*));
    if (temp == NULL){
    
        printf("Speicherfehler: Transponierte (Zeilen/temp).\n");
        return false;
    }
    
    inverse = (double**) malloc(rang * sizeof(double*));
    if (inverse == NULL){
    
        printf("Speicherfehler: Transponierte (Zeilen/inverse).\n");
        return false;
    }
    else{
    
        for (idx=0; idx<rang; idx++){
        
            temp[idx] = (double *) malloc(rang * sizeof(double));
            if (temp[idx] == NULL){
            
                printf("Speicherfehler: Transponente (Spalten/temp).\n");
                return false;                
            
            }
        
            inverse[idx] = (double *) malloc(rang * sizeof(double));
            if (inverse[idx] == NULL){
            
                printf("Speicherfehler: Transponente (Spalten/inverse).\n");
                return false;                
            
            }
        }
        
        for (idx=0; idx<r; idx++){
     
            for (jdx=0; jdx<r; jdx++){
            
                temp[idx][jdx] = fac[jdx][idx];
            }
        }
        
        Det = determinantOfMatrix(matrix, rang, r);

        for (idx=0; idx<r; idx++){
       
            for (jdx=0; jdx<r; jdx++){
           
                inverse[idx][jdx] = temp[idx][jdx] / Det;
            }
        }
     
        printf("\nThe inverse of matrix: \n");
       
        for (idx=0; idx<r; idx++){
       
            for (jdx=0; jdx<r; jdx++){
           
                printf("%25.22f", inverse[idx][jdx]);
            }
            printf("\n");
        }
        
        
        // Speicher freigeben:
        for (idx=0; idx<rang; idx++){
        
            free(temp[idx]);
            free(inverse[idx]);
        
        }
        free(temp);
        printf("Speicher freigegeben (Inverse/temp)\n");
        
        free(inverse);
        printf("Speicher freigegeben (Inverse/inverse)\n");        
    }
   
}




