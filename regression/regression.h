
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
#endif



// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ######################################################################## function declaration #################################################################################
double *polynomial_regression(double **input_data, const int length, const int n);




// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ######################################################################## function definition ##################################################################################




double *polynomial_regression(double **input_data, const int length, const int n){

    
    /*
        DESCRIPTION:
        Performs polynomial regression of order n to a given dataset.
    
        INPUT:
        input_data	...	Pointer of type double to the input dataset in shape of:
                                x-values: input_data[0 bis n][0]
                                y-values: input_data[0 bis n][1]
                                		
        length		...	Number of points.
        n		...	order of regression.
             
        OUTPUT:
        
        
    */
    
    
    int idx, jdx, kdx;				// Indexvariablen
    const double eps = 1.0e-14;			// Minimum
    double temp;				// allge. temporärer Zwischenwert
    double sum = 0;				// allge. temporäre Zwischensumme  
    
    double DetL = 1.0;				// Determinante der Lower-Diagonalmatrize
    double DetU = 1.0;				// Determinante der Upper-Diagonalmatrize
    double Det;					// Determinante
    
    double sumDiff_y0=0;			// Summe der quadrierten Differenzen von gemessenem Wert und Mittelwert der Messwerte
    double sumDiff_y=0;				// Summe der quadrierten Differenzen von vorhergesagtem Wert und Mittelwert der Vorhersagewerte
    double *data_powers_COVCriteria;		// Vektor mit den Intrakriteriumskovarianzen (COV(x,y), COV(x^2,y), ... ,COV(x^n, y))
    double *data_powers_CORCriteria;		// Vektor mit den Intrakriteriumskorrelationen (COR(x,y), COR(x^2,y), ... ,COR(x^n, y))
    double *beta_weights;			// Vektor mit den Beta-Gewichtungen zur Berechnung der b-Gewichte
    double *b_weights;				// Vektor mit den b-Gewichten und dem Bestimmtheitsmaß (Lösung der polynomialen Regression).
    double *values_pred;			// Vektor der laut Modell vorhergesagten Werte.
    double **data_powers;			// Matrix mit den Pädiktoren von x^1 ... x^n (x-Werte) & y-Werten
    double **data_powers_MW_SD;			// Matrix mit den Mittelwerten und Standardabweichungen der Eingabedaten (x & y-Werte)
    double **data_powers_COVMatrix;		// Intrakovarianzmatrix der Prädiktoren von x^1 ... x^n (x-Werte)
    double **data_powers_CORMatrix;		// Intrakorrelationsmatrix der Prädiktoren von x^1 ... x^n (x-Werte)
    double **data_powers_CORMatrix_inverse;	// inverse Intrakorrelationsmatrix der Prädiktoren von x^1 ... x^n (x-Werte)
    double **temp_data_powers_CORMatrix;	// temporäre Intrakorrelationsmatrix der Prädiktoren von x^1 ... x^n (x-Werte) zur Berechnung der Inversen
    double **LowMatrix;				// Lower-Diagonalmatrize
    double **UppMatrix;				// Upper-Diagonalmatrize    
    
    
    
    
    // Der Grad der Regression darf nicht größer als die Anzahl der Eingabewerte (length) sein:
    if (n > length){
    
        printf("Der Grad der polynomialen Regressionsfunktion darf nicht größer als die Anzahl der Eingabewerte sein.\n");
        printf("Vorgang beendet.\n");
        return NULL;
    
    }
    // Prüfe die Eingabedaten auf unzulässige Werte:
    for (idx=0; idx<length; idx++){
    
        if (isnan(input_data[idx][0]) || isnan(input_data[idx][1])){
            printf("Die Eingabedaten enthalten in Zeile %d NaN Werte.\n", idx);
            printf("Vorgang wird beendet.\n");
            return NULL;
        } 
    }
    
    

    // Reservieren des Speichers:
    data_powers = (double**) calloc(length , sizeof(double*));			// Matrix für die Potenzen der Eingabedaten (x-Werte) & y-Werten
    data_powers_MW_SD = (double**) calloc(2, sizeof(double*));			// Matrix mit den Mittelwerten und Standardabweichungen der Eingabedaten (x & y-Werte)
    data_powers_COVMatrix = (double**) calloc(n, sizeof(double*));		// Matrix mit den Intrakovarianzen der Prädiktoren COV(x,x), COV(x,x^2), ... , COV(x^n, x^n)
    data_powers_CORMatrix = (double**) calloc(n, sizeof(double*));		// Matrix mit den Intrakorrelationen der Prädiktoren COR(x,x), COR(x,x^2), ... , COR(x^n, x^n)
    temp_data_powers_CORMatrix = (double**) calloc(n, sizeof(double*));		// temporäre Matrix mit den Intrakorrelationen der Prädiktoren COR(x,x), COR(x,x^2), ... , COR(x^n, x^n)
    data_powers_CORMatrix_inverse = (double**) calloc(n, sizeof(double*));	// inverse Intrakorrelationsmatrix der Prädiktoren COR(x,x), COR(x,x^2), ... , COR(x^n, x^n)
    data_powers_CORCriteria = (double*) calloc(n, sizeof(double*));		// Vektor mit den Intrakriteriumskorrelationen (COR(x,y), COR(x^2,y), ... ,COR(x^n, y))
    data_powers_COVCriteria = (double*) calloc(n, sizeof(double*));		// Vektor mit den Intrakriteriumskovarianzen (COV(x,y), COV(x^2,y), ... ,COV(x^n, y))
    beta_weights = (double*) calloc(n, sizeof(double*));			// Vektor mit den beta-Gewichten zur Berechnung des b-Gewichte
    b_weights = (double*) calloc(n+2, sizeof(double*));				// Vektor mit den b-Gewichten (b0, b1, ..., bn) und dem Bestimmtheitsmaß.
    values_pred = (double*) calloc(length, sizeof(double*));			// Vektor mit den laut Modell berechneten bzw. vorhergesagten Werten.
    LowMatrix = (double**) calloc(n, sizeof(double*));				// Lower-Diagonalmatrize
    UppMatrix = (double**) calloc(n, sizeof(double*));				// Upper-Diagonalmatrize
    
    if ((data_powers == NULL) || 
        (data_powers_MW_SD == NULL) || 
        (data_powers_COVMatrix == NULL) || 
        (data_powers_CORMatrix == NULL) ||
        (temp_data_powers_CORMatrix == NULL) ||
        (data_powers_CORMatrix_inverse == NULL) ||
        (data_powers_CORCriteria == NULL) ||
        (data_powers_COVCriteria == NULL) ||
        (beta_weights == NULL) ||
        (b_weights == NULL) ||
        (values_pred == NULL)){
        
        printf("Speicherfehler!");
        return NULL;
    }
    else{
    
        // Spalten der Matrix für die Potenzen der Eingabedaten
        for(idx=0; idx<length; idx++){
        
           data_powers[idx] = (double*) calloc((n+1) , sizeof(double));
           if (data_powers[idx] == NULL){
               printf("Speicherfehler!\n");
               return NULL;
           }
        }
        // Spalten der Matrix mit den Mittelwerten und Standardabweichungen der Eingabedaten
        for(idx=0; idx<2; idx++){
        
           data_powers_MW_SD[idx] = (double*) calloc((n+2) , sizeof(double));
           if (data_powers_MW_SD[idx] == NULL){
               printf("Speicherfehler!\n");
               return NULL;
           }
        }  
        //  Spalten der Matrix mit den Intrakovarianzen der Prädiktoren:
        for(idx=0; idx<n; idx++){
        
            data_powers_COVMatrix[idx] = (double*) calloc(n , sizeof(double));
            if (data_powers_COVMatrix[idx] == NULL){
                printf("Speicherfehler!\n");
                return NULL;
            }
        } 
        //  Spalten der Matrix mit den Intrakorrelationen der Prädiktoren:       
        for(idx=0; idx<n; idx++){
        
            data_powers_CORMatrix[idx] = (double*) calloc(n , sizeof(double));
            if (data_powers_CORMatrix[idx] == NULL){
                printf("Speicherfehler!\n");
                return NULL;
            }
        }
        //  Spalten der temporären Matrix mit den Intrakorrelationen der Prädiktoren:       
        for(idx=0; idx<n; idx++){
        
            temp_data_powers_CORMatrix[idx] = (double*) calloc(n , sizeof(double));
            if (temp_data_powers_CORMatrix[idx] == NULL){
                printf("Speicherfehler!\n");
                return NULL;
            }
        } 
        //  Spalten der temporären Matrix mit den Intrakorrelationen der Prädiktoren:       
        for(idx=0; idx<n; idx++){
        
            data_powers_CORMatrix_inverse[idx] = (double*) calloc(n , sizeof(double));
            if (data_powers_CORMatrix_inverse[idx] == NULL){
                printf("Speicherfehler!\n");
                return NULL;
            }
        }
        // Spalten der Lower-Diagonalmatrize zum Berechnen der Determinanten
        for(idx=0; idx<n; idx++){
        
            LowMatrix[idx] = (double*) calloc(n , sizeof(double));
            if (LowMatrix[idx] == NULL){
                printf("Speicherfehler!\n");
                return NULL;
            }
        }
        // Spalten der Upper-Diagonalmatrize zum Berechnen der Determinanten
        for(idx=0; idx<n; idx++){
        
            UppMatrix[idx] = (double*) calloc(n , sizeof(double));
            if (UppMatrix[idx] == NULL){
                printf("Speicherfehler!\n");
                return NULL;
            }
        }                              
    }
    
    /*#########################################################################################################
    #                                                                                                         #
    #                                               Functions                                                 #
    #                                                                                                         #
    #########################################################################################################*/    
    
    void free_memory(){
    
    
        for (idx=0; idx<length; idx++){
            free(data_powers[idx]);
        }
    
        for (idx=0; idx<2; idx++){
            free(data_powers_MW_SD[idx]);
        }
        
        for (idx=0; idx<n; idx++){
            free(data_powers_COVMatrix[idx]);
            free(data_powers_CORMatrix[idx]);
            free(temp_data_powers_CORMatrix[idx]);
            free(data_powers_CORMatrix_inverse[idx]);
            free(LowMatrix[idx]);
            free(UppMatrix[idx]);
        }
    
        free(data_powers);
        free(data_powers_MW_SD);
        free(data_powers_COVMatrix);
        free(data_powers_CORMatrix); 
        free(temp_data_powers_CORMatrix);
        free(data_powers_CORMatrix_inverse);
        free(data_powers_COVCriteria);
        free(data_powers_CORCriteria);
        free(beta_weights);
        free(values_pred);
        free(LowMatrix);
        free(UppMatrix);        
    }
    
    
    /*#########################################################################################################
    #                                                                                                         #
    #                                             Calculations                                                #
    #                                                                                                         #
    #########################################################################################################*/

    // ========================================================================================================
    // 1. Berechnen der Potenzen der Eingabedaten (x-Werte) und übernahme der y-Werte:
    for (idx=0; idx<length; idx++){
    
        for (jdx=0; jdx<n+1; jdx++){
        
            if (jdx < n){
                
                data_powers[idx][jdx] = pow(input_data[idx][0],jdx+1);
            }
            else{
                data_powers[idx][jdx] = input_data[idx][1];
            }
        }
    }
    
    // ========================================================================================================   
    // 2. Berechnen der Mittelwerte und der Standardabweichungen der Spalten aus der Matrix "Potenzen der Eingabedaten":
    // 2.1 Berechnen des Mittelwerts für jede Spalte in data_powers_MW_SD[0][0 ... n+1]:
    for (idx=0; idx<1; idx++){
        for (jdx=0; jdx<n+1; jdx++){
            for (kdx=0; kdx<length; kdx++){
                data_powers_MW_SD[idx][jdx] += data_powers[kdx][jdx];
            }
            data_powers_MW_SD[idx][jdx] /= (double)length;
        }
    }
    // 2.2 Berechnen des Mittelwerts für jede Spalte in data_powers_MW_SD[1][0 ... n+1]:  
    for (idx=1; idx<2; idx++){
        for (jdx=0; jdx<n+1; jdx++){
            for (kdx=0; kdx<length; kdx++){
                data_powers_MW_SD[idx][jdx] += pow((data_powers[kdx][jdx] - data_powers_MW_SD[0][jdx]),2);
            }
            data_powers_MW_SD[idx][jdx] = sqrt(data_powers_MW_SD[idx][jdx] /= length);
        }
    }    
    
    // ========================================================================================================     
    // 3. Berechnen der Korrelationsmatrix der Prädiktoren:
    // 3.1 Berechnen der Kovarianzmatrix der Prädiktoren:
    for (idx=0; idx<n; idx++){
    
        for (jdx=0; jdx<n; jdx++){
        
            for (kdx=0; kdx<length; kdx++){
                data_powers_COVMatrix[idx][jdx] += ((data_powers[kdx][idx] - data_powers_MW_SD[0][idx]) * 
                                                    (data_powers[kdx][jdx] - data_powers_MW_SD[0][jdx]));
            }
            data_powers_COVMatrix[idx][jdx] /= (double)length;
        }
    }
    // 3.2 Berechnen der Intrakorrelationsmatrix der Prädiktoren:
    for (idx=0; idx<n; idx++){
    
        for (jdx=0; jdx<n; jdx++){
        
            data_powers_CORMatrix[idx][jdx] = data_powers_COVMatrix[idx][jdx] / (data_powers_MW_SD[1][idx] * data_powers_MW_SD[1][jdx]);

        }
    }
    
    // ========================================================================================================    
    // 4. Berechnen der Determinanten der Korrelationsmatrix der Prädiktoren um zu prüfen, ob es eine inverse
    //    Matrix gibt.
    
        for (idx=0; idx<n; idx++){
    
            UppMatrix[idx][idx] = 1;
        }

        for (jdx=0; jdx<n; jdx++){
	    for (idx=jdx; idx<n; idx++){
	
	        sum=0;
	    
	        for (kdx=0; kdx<jdx; kdx++){
	    
		    sum = sum + LowMatrix[idx][kdx] * UppMatrix[kdx][jdx];
	        }
	    
	        LowMatrix[idx][jdx] = data_powers_CORMatrix[idx][jdx] - sum;
	    }

	    for (idx=jdx; idx<n; idx++){
	
	        sum = 0;
	    
	        for(kdx=0; kdx<jdx; kdx++){
	    
	            sum = sum + LowMatrix[jdx][kdx] * UppMatrix[kdx][idx];
	        }
	        
	        if (LowMatrix[jdx][jdx] == 0){
	    
	            printf("Determinante der Lower-Matrix nahe 0!\n");
	            printf("Es kann nicht durch 0 dividiert werden.\n");
	            printf("Vorgang beendet.\n");
	            free_memory();
	            return NULL;
	        }
	        UppMatrix[jdx][idx] = (data_powers_CORMatrix[jdx][idx] - sum) / LowMatrix[jdx][jdx];
	    }
        }
    
        // Multiplizieren der Spaltendiagonalen:
        for (idx=0; idx<n; idx++){
    
            DetL = DetL * LowMatrix[idx][idx];
            DetU = DetU * UppMatrix[idx][idx];

        }    

        // Berechnen der Determinanten aus Det(L) * Det(U)
        Det = DetL * DetU;
        
        if (isnan(Det)){
            printf("Fehler! Determinante ist nan.");
	    printf("Vorgang beendet.\n"); 
	    free_memory();         
            return NULL;
        }
        if ((eps-Det) > 0){
            printf("Determinante ist nahe/gleich 0! (Wert: %.16f)\n", Det);
            printf("Es existiert keine inverse Matrix.\n");
	    printf("Vorgang beendet.\n");
	    free_memory(); 
	    return NULL;
        }
    
    
    // ========================================================================================================     
    // 5. Berechnen der inversen Korrelationsmatrix der Prädiktoren:
    // 5.1 Kopieren des Inhalts der Korrelationsmatrix in die temporäre Korrelationsmatrix:
    for(idx=0; idx<n; idx++){
    				
        for(jdx=0; jdx<n; jdx++){
            					
	    temp_data_powers_CORMatrix[idx][jdx] = data_powers_CORMatrix[idx][jdx];
	}
    }
    
    // 5.2 Setze die Diagonale der Inversen auf 1, sonst 0:
    for(idx=0; idx<n; idx++){
    				
        for(jdx=0; jdx<n; jdx++){								
	
	    if(idx == jdx){										
	        data_powers_CORMatrix_inverse[idx][jdx] = 1;
	    }									
	    else{
	        data_powers_CORMatrix_inverse[idx][jdx] = 0;
	    }
	}
    }
    
    // 5.3 Berechnung der inversen Korrelationsmatrix:
    for(kdx=0 ;kdx<n; kdx++){
    														
	temp = temp_data_powers_CORMatrix[kdx][kdx];
											
        for(jdx=0; jdx<n; jdx++){
        
	    temp_data_powers_CORMatrix[kdx][jdx] /= temp;
	    data_powers_CORMatrix_inverse[kdx][jdx] /= temp;
	}
														
	for(idx=0; idx<n; idx++){
	
	    temp = temp_data_powers_CORMatrix[idx][kdx];
	    									
	    for(jdx=0; jdx<n; jdx++){
	    												
		if(idx == kdx){
		    break;
		}
											
		temp_data_powers_CORMatrix[idx][jdx] -= temp_data_powers_CORMatrix[kdx][jdx] * temp;
								
		data_powers_CORMatrix_inverse[idx][jdx] -= data_powers_CORMatrix_inverse[kdx][jdx] * temp;
	    }
	}
    }
    
    // ========================================================================================================        
    // 6. Berechnung des Vektors mit den Kriteriumskorrelationen:
    // 6.1 Berechnung des Vektors mit den Kriteriumskovarianzen:
    for (idx=0; idx<n; idx++){
        for (jdx=0; jdx<length; jdx++){
            
            data_powers_COVCriteria[idx] += ((data_powers[jdx][idx] - data_powers_MW_SD[0][idx]) * (data_powers[jdx][n] - data_powers_MW_SD[0][n]));
        }
        data_powers_COVCriteria[idx] /= (double)length;
    }
    
    // 6.2 Berechnung des Vektors mit den Kriteriumskorrelationen:    
    for (idx=0; idx<n; idx++){
        data_powers_CORCriteria[idx] = data_powers_COVCriteria[idx] / ((data_powers_MW_SD[1][idx]) * (data_powers_MW_SD[1][n]));
    }
    
    // ========================================================================================================        
    // 7. Berechnung der Beta-Gewichte durch Multiplikation der inversen Intrakorrelationsmatrix mit dem Vektor der Kriteriumskorrelation:    
    for (idx=0; idx<n; idx++){
    
        for (jdx=0; jdx<n; jdx++){
            
            beta_weights[jdx] += (data_powers_CORMatrix_inverse[jdx][idx] * data_powers_CORCriteria[idx]);
        }
    }
    
    
    // ======================================================================================================== 
    // 8. Berechung der b-Gewichte aus den Beta-Gewichten:
    for (idx=0; idx<n+1; idx++){
    
        if (idx == 0){
            b_weights[idx] = data_powers_MW_SD[0][n];
            
            continue;
        }
        else{
            b_weights[idx] = (data_powers_MW_SD[1][n] / data_powers_MW_SD[1][idx-1]) * beta_weights[idx-1];
        }
    
    }
    for (idx=1; idx<n+1; idx++){
        b_weights[0] = b_weights[0] - (data_powers_MW_SD[0][idx-1] * b_weights[idx]);
    }
    
    // ========================================================================================================
    // 9. Berechnung des Bestimmtheitsmaßes:
    // 9.1 Berechnung der Vorhersagewerte:
    for (idx=0; idx<length; idx++){
    
        values_pred[idx] = b_weights[0];
        
        for (jdx=0; jdx<n; jdx++){
            
            values_pred[idx] += (b_weights[jdx+1] * data_powers[idx][jdx]);
        }
        
        data_powers_MW_SD[0][n+1] += values_pred[idx];
        
    }
    // Berechnung des Mittelwertes der Vorhersagedaten:
    data_powers_MW_SD[0][n+1] /= length;

    // ========================================================================================================    
    // 10.2 Berechnung des Bestimmtheitsmaßes:    
    for (idx=0; idx<length; idx++){
    
        // Summe der quadrierten Differenzen der Messwerte vom Mittelwert der Messwerte.
        sumDiff_y0 += pow((input_data[idx][1] - data_powers_MW_SD[0][n]),2);
        
        // Summe der quadrierten Differenzen der Vorhersagewerte vom Mittelwert der Vorhersagewerte .      
        sumDiff_y += pow((values_pred[idx] - data_powers_MW_SD[0][n+1]),2);        
    }
    
    // Berechnung des Bestimmtheitsmaßes R² als Division der Summe der quadratischen Differenzen:
    b_weights[n+1] = ((double)sumDiff_y / (double)sumDiff_y0);
    

    // Speicher freigeben:
    free_memory();
    
    // Ausgabe der b-Gewichte:   
    return b_weights;

}























