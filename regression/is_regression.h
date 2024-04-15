

/*###############################################################################################################################################################################
Author:		Schotte, Ilja
Latest Update:	14.04.2024
-------------------------------------------------------------------------------------
How to use:
-------------------------------------------------------------------------------------
int polynomial_regression(double **input_data,
                          unsigned int length, 
                          unsigned int order, 
                          double *result)
                          
Input:
-------------------------------------------------------------------------------------

double **input_data	...	pointer to an array of type double with your input datapoints in following format:
				|val_x0	|val_0y	|
				|val_x1	|val_1y	|
				|...	|...	|
				|val_xn	|val_yn	|
				
-------------------------------------------------------------------------------------
				
unsigned int length	...	number of datapoints in your input data array.

-------------------------------------------------------------------------------------

unsigned int order	...	order of the wanted polynomial function
			
-------------------------------------------------------------------------------------

double *result		...	result/output of of this function is a pointer of type double with a length "order + 2".
				- In the first position (result[0]) you will get the coefficient of determination.
				- on the following positions you will get the weights (b) for x starting with b(0) to b(order)
				
				Example:
				You are looking for the weights of a function with order by 3: y = b3*x^3 + b2*x^2 + b1*x^1 + b0
				
				result:
				result[0]	...	coefficient of determination
				result[1]	...	weight b0
				result[2]	...	weight b1
				result[3]	...	weight b2
				result[4]	...	weight b3
				
				!!! Make sure to give this function a pointer of type double !!! 
				
-------------------------------------------------------------------------------------				
				
				
				
-------------------------------------------------------------------------------------
###############################################################################################################################################################################*/

#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <setjmp.h>
    #include <errno.h>
    #include <stdint.h>
    #include <string.h>
#endif

#define EPS 1E-14


// ######################################################################### struct declaration ##################################################################################

struct dataset{

    unsigned int order;
    unsigned int input_data_length;
    double **input_data;
    double **input_data_powers;			// powers of the input values
    double **avg_sd_input_data_powers;		// average and standard deviation of the powers of the input values
    double **cov_data_powers;		// covariance matrix of the powers of the input values
    double **cor_data_powers;			// intra-correlation matrix of predictors
    double **cor_data_powers_temp;		// temorary used intra-correlation matrix of predictors
    double **cor_data_powers_inverted;		// inverted intra-correlation matrix.
    double *cor_data_powers_criteria;		// vector of intra-criteria-correlation
    double *cov_data_powers_criteria;		// vector of intra-criteria-covariances
    double *beta_weights;			// vector of beta-weight
    double *b_weights;				// vector of b-weights and coefficient of determination
    double *predicted_values;			// predicted values of the coefficients
    double **lower_matrix;			// lower matrix for the calculation of the determinat
    double **upper_matrix;			// upper matrix for the calculation of the determinat
    double det_cor;				// determinant of intra-correlation matrix  
};






// ######################################################################## function declaration #################################################################################
// int polynomial_regression(double **input_data, const int length, const int n);

int read_input_data(double **input_data, const int length, const int order, struct dataset *data);	// reads the input data / performs some checks / imports to internal dataset.
int allocate_fmatrix(double ***matrix, unsigned int rows, unsigned int columns);			// allocates memory for a rows x colmuns matrix of type double.
int allocate_fvector(double **vector, unsigned int length);						// allocates memory for a vector of length "length" and of type double.
int calc_input_data_powers(struct dataset *internal_data);						// calculate the powers of the input values according to the selected order of regression.
int calc_avg_sd_input_data_powers(struct dataset *internal_data);					// calculate the averages and standard deviation of the powers of the input values. 
int calc_cov_cor_input_data_powers(struct dataset *internal_data);					// calculates the covariance matrix of input data and intra-correlation matrix of predictors.
int calc_determinat(struct dataset *internal_data);							// calculates the determinant of a square matrix
int calc_inverse_matrix(struct dataset *internal_data);							// determines the inverse of a square matrix
int calc_cor_cov_crieria(struct dataset *internal_data);						// calculates the vectors of intra-criteria-correlation and covariances
int calc_beta_weights(struct dataset *internal_data);							// calculates the vector of beta weights.
int calc_b_weights(struct dataset *internal_data);							// calculates the vector of b weights.
int get_coefficient_of_determination(struct dataset *internal_data);					// determines the coefficient of determination.

void free_fmatrix_memory(double **matrix, unsigned int rows, unsigned int columns);			// frees the allocated memory of a matrix of datatype double.
void free_fvector_memory(double *vector, unsigned int length);						// frees the allocated memory of a vector of datatype double.

int polynomial_regression(double **input_data, const unsigned int length, const unsigned int order, double *result);	// function to perform a polynimocal regression.


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ######################################################################## function definition ##################################################################################


int polynomial_regression(double **input_data, const unsigned int length, const unsigned int order, double *result){


    int err = EXIT_FAILURE;
    jmp_buf env;
    
    
    // declare internal data:
    struct dataset internal_data = {
        .order = 0,
        .input_data_length = 0,
        .det_cor = 0,
    };
    
    
    // ########################################################################################
    // ##################### LOAD AND ADD INPUT DATA TO INTERN DATASET ########################
    void read_data(double **input_data, const unsigned int length, const unsigned int order){
    
        /*
           read the input_dataset, perform some checks and add the arguments to the internal dataset:
        */
        
        err = read_input_data(input_data, 
                              length, order, 
                              &internal_data);
        if (err == EXIT_FAILURE){
            longjmp(env, 1);
        }
    }
    
    
    // ########################################################################################
    // ################################ MEMORY ALLOCATION #####################################       
    void allocate_memory(struct dataset *internal_data){
    
        /*
           allocates all the necessary memory.
        */   
          
        // Allocate a matrix to calculate the powers of the input data.
        err = allocate_fmatrix(&(internal_data->input_data_powers), 
                                 internal_data->input_data_length, 
                                 (internal_data->order+1));
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
    
        // Allocate a matrix to calculate the average and standard deviation of the powers of the input data.    
        err = allocate_fmatrix(&(internal_data->avg_sd_input_data_powers), 
                                 2, 
                                (internal_data->order+2));
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }    

        // Allocate a matrix to calculate the covariances of the powers of the input data.    
        err = allocate_fmatrix(&(internal_data->cov_data_powers), 
                                 internal_data->order, 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
        // Allocate a matrix for the correlation of predictors    
        err = allocate_fmatrix(&(internal_data->cor_data_powers), 
                                 internal_data->order, 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
    
        // Allocate a matrix for temporary calculations of the correlation of predictors    
        err = allocate_fmatrix(&(internal_data->cor_data_powers_temp), 
                                 internal_data->order, 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
    
        // Allocate a matrix for the inverted correlation matrix of the predictors    
        err = allocate_fmatrix(&(internal_data->cor_data_powers_inverted), 
                                 internal_data->order, 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
    
        // Allocate memory for the lower matrix to calculate the determinat
        err = allocate_fmatrix(&(internal_data->lower_matrix), 
                                 internal_data->order, 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }    
    
        // Allocate memory for the lower matrix to calculate the determinat
        err = allocate_fmatrix(&(internal_data->upper_matrix), 
                                 internal_data->order, 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
          
        // Allocate a vector for the intra-criteria-correlation
        err = allocate_fvector(&(internal_data->cor_data_powers_criteria), 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }  
    
        // Allocate a vector for the intra-criteria-covariances
        err = allocate_fvector(&(internal_data->cov_data_powers_criteria), 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
    
        // Allocate a vector for the beta-weights
        err = allocate_fvector(&(internal_data->beta_weights), 
                                 internal_data->order);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }    
    
        // Allocate a vector for the b-weights and coefficient of determination
        err = allocate_fvector(&(internal_data->b_weights), 
                                 internal_data->order+2);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
    
        // Allocate a vector for the predicted values of the coefficients
        err = allocate_fvector(&(internal_data->predicted_values), 
                                 internal_data->input_data_length);
        if (err == EXIT_FAILURE){
            longjmp(env, 2);
        }
    } 
    
       
    // ########################################################################################
    // ################################## CALCULATIONS ########################################    
    void perform_calculations(struct dataset *internal_data){
    
        /*
            calculates the weights (b) of the wanted polynomic function.
        */
    
        // calculate the powers of the input data according to the selected order of polynomic regression
        err = calc_input_data_powers(internal_data);
        if (err == EXIT_FAILURE){
            longjmp(env, 3);
        }
    
        /// calculate the averages and standard deviation of the powers of the input values.
        err = calc_avg_sd_input_data_powers(internal_data);
        if (err == EXIT_FAILURE){
            longjmp(env, 3);
        } 
       
        // calculates the covariance matrix of input data and intra-correlation matrix of predictors.
        err = calc_cov_cor_input_data_powers(internal_data);
        if (err == EXIT_FAILURE){
            longjmp(env, 3);
        }
    
        // calculate the determinant of the intra-correlation matrix of predictors to show if an inverse matrix exists.
        err = calc_determinat(internal_data);
        if (err == EXIT_FAILURE){
            longjmp(env, 3);
        }     
        else{
            // check for existence of inverse matrix 
            if (EPS-internal_data->det_cor > 0){
                fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The determinant of intra-correlation matrix is equal to 0.\n>>> There is no inverse matrix!\n", __FILE__, __func__);
                longjmp(env, 3);
            }
            else{
                // determine the inversed matrix of the intra-correlation-matrix of predictors
                err = calc_inverse_matrix(internal_data); 
                if (err == EXIT_FAILURE){
                    longjmp(env, 3);
                } 
            }
        
            // Calculate the vectors of kovariance and correlation crieria
            err = calc_cor_cov_crieria(internal_data);
            if (err == EXIT_FAILURE){
                longjmp(env, 3);
            }        
        
            // calculate the beta weights
            err = calc_beta_weights(internal_data);
            if (err == EXIT_FAILURE){
                longjmp(env, 3);
            }
        
            // calculate the b weights
            err = calc_b_weights(internal_data);
            if (err == EXIT_FAILURE){
                longjmp(env, 3);
            } 
          
            // calculate the coefficient of determination
            err = get_coefficient_of_determination(internal_data);
            if (err == EXIT_FAILURE){
                longjmp(env, 3);
            }            
        }
    }


    // ########################################################################################
    // #################################### CLEAN UP ##########################################    
    void free_allocated_memory(struct dataset *internal_data){
    
        // free the internal dataset of the input data:
        free_fmatrix_memory(internal_data->input_data, 
                            internal_data->input_data_length, 
                            2);
        // free the internal matrix of power values:
        free_fmatrix_memory(internal_data->input_data_powers,
                            internal_data->input_data_length, 
                            (internal_data->order+1));
        // free the internal matrix of the averages and standard deviations of the powers of input data:
        free_fmatrix_memory(internal_data->avg_sd_input_data_powers,
                            2, 
                            (internal_data->order+2));
        // free the internal matrix of covariances of the powers of input data:
        free_fmatrix_memory(internal_data->cov_data_powers,
                            internal_data->order, 
                            internal_data->order);
        // free the internal matrix of the correlation of predictors:
        free_fmatrix_memory(internal_data->cor_data_powers,
                            internal_data->order, 
                            internal_data->order);
        // free the internal matrix for temporary calculations on the matrix of intra-correlations of predictors:
        free_fmatrix_memory(internal_data->cor_data_powers_temp,
                            internal_data->order, 
                            internal_data->order);
        // free the internal inverted matrix of the correlation of predictors:
        free_fmatrix_memory(internal_data->cor_data_powers_inverted,
                            internal_data->order, 
                            internal_data->order); 
        // free the internal lower matrix for calculation of determinant:
        free_fmatrix_memory(internal_data->lower_matrix,
                            internal_data->order, 
                            internal_data->order);                        
        // free the internal upper matrix for calculation of determinant:
        free_fmatrix_memory(internal_data->upper_matrix,
                            internal_data->order, 
                            internal_data->order);
        // free the internal vector of the intra-criteria-correlation:
        free_fvector_memory(internal_data->cor_data_powers_criteria,
                            internal_data->order); 
        // free the internal vector of the intra-criteria-covariances:
        free_fvector_memory(internal_data->cov_data_powers_criteria,
                            internal_data->order);                        
        // free the internal vector of the beta-weights:
        free_fvector_memory(internal_data->beta_weights,
                            internal_data->order);
        // free the internal vector of the b-weights and coefficient of determination:
        free_fvector_memory(internal_data->b_weights,
                            internal_data->order+2);
        // free the internal vector of predicted values of the coefficients:
        free_fvector_memory(internal_data->predicted_values,
                            internal_data->input_data_length);
    }
    
    
    switch(setjmp(env)){
        case 0: read_data(input_data, length, order);
                allocate_memory(&internal_data);
                perform_calculations(&internal_data); 
                free_allocated_memory(&internal_data);
                return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> During reading the input data!\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: (%s -> %s)\n>>> During memory allocation!\n", __FILE__, __func__); return EXIT_FAILURE;
        case 3: fprintf(stderr,"ERROR: (%s -> %s)\n>>> During performing calculations!\n", __FILE__, __func__); return EXIT_FAILURE;
        case 4: fprintf(stderr,"ERROR: (%s -> %s)\n>>> During memory deallocation!\n", __FILE__, __func__); return EXIT_FAILURE;        
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n", __FILE__, __func__); return EXIT_FAILURE;
    }
                              
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int read_input_data(double **input_data, const int length, const int order, struct dataset *internal_data){

    /*
        DESCRIPTION:
        Reads the input dataset and performs several checks to it.
    
        INPUT:
        double **input_data	...	Pointer of type double to the input dataset in shape of:
        				x-values: input_data[0 bis n][0]
        				y-values: input_data[0 bis n][1]
                                		
        const int length        ...	Number of points.
        const int order		...	order of regression.
        struct dataset *internal_data	internal dataset to calculate with.
             
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - the order of the polynomial function must be lower or equal to the number of input points. 
        - 
    */
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ###########################################  
    void read_data(double **input_data, int length, int order, struct dataset *internal_data){
    
        // check for length and order of input_dataset
        if (order <= 0){
            longjmp(env, 4);
        }
        else{
            if (order > length){
                longjmp(env, 1);
            }
            else{
                // set order of polynomial function:
                internal_data->order = order;            
            }
        }
        
        if (length <= 0){
            longjmp(env, 2);
        }
        else{
            // set order of polynomial function:
            internal_data->input_data_length = length;            
        }
        
        // check for nan - values:
        for (idx=0; idx<length; idx++){
            for (jdx=0; jdx<2; jdx++){
                if (isnan(input_data[idx][jdx])){
                    longjmp(env, 3);
                }
            }      
        }
        // import input_dataset to the internal data:
        internal_data->input_data = input_data;
        
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: read_data(input_data, length, order, internal_data); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The order of the polynomic function cannot be higher than the number of datapoints.\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The length of the input dataset has to be higher than 0.\n", __FILE__, __func__); return EXIT_FAILURE;        
        case 3: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The dataset consists nan - values.\n", __FILE__, __func__); return EXIT_FAILURE;
        case 4: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The order of the regression function must be greater then 0.\n", __FILE__, __func__); return EXIT_FAILURE;             
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }    
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_input_data_powers(struct dataset *internal_data){

    /*
        DESCRIPTION:
        calculates the powers of the input datapoints regarding to the selected order 
        of the polynomic regression.
    
        INPUT:
        struct dataset *internal_data	...	pointer to the internal dataset 
             
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - order of polynomic regression must be smaller/equal to the number of input values.
        - number of input value smust be greater than 0.
    */
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ###########################################  
    void calc_powers(struct dataset *internal_data){    
    
        // check if the order of the polynomic regression is smaller/equal to the number of datapoints
        if (internal_data->order > internal_data->input_data_length){
            longjmp(env,1);
        }
        
        // check for length of dataset
        if (internal_data->input_data_length <= 0){
            longjmp(env,2);
        }
        
        // perform calculations to get the powers of the datapoints regarding to the selected order of polynomic regression.
        // x1 | x1² | x1³ | y1
        // x2 | x2² | x2³ | y2

        for (idx=0; idx<internal_data->input_data_length; idx++){
            for (jdx=0; jdx<(internal_data->order)+1; jdx++){
            
                if (jdx < internal_data->order){
                    // calculate powers:
                    internal_data->input_data_powers[idx][jdx] = pow(internal_data->input_data[idx][0], jdx+1);
                }
                else{
                    // adopt y-values to the last column:
                    internal_data->input_data_powers[idx][jdx] = internal_data->input_data[idx][1];
                }
            }        
        } 
    }
    // #################################################################################################
       
    switch(setjmp(env)){
        case 0: calc_powers(internal_data); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: ( %s -> %s)\n>>> The order of the polynomic function cannot be higher than the number of datapoints.\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The length of the input dataset has to be higher than 0.\n", __FILE__, __func__); return EXIT_FAILURE;                         
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_avg_sd_input_data_powers(struct dataset *internal_data){

    /*
        DESCRIPTION:
        calculates the averages and standard deviations of the powers of the input values.
    
        INPUT:
        struct dataset *internal_data	...	pointer to the internal dataset 
             
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        --
    */

    int idx, jdx, kdx;
    jmp_buf env;
    
    // ########################################### functions ###########################################  
    void calc_avg_sd(struct dataset *internal_data){
    
        // calculate the averages of all power vectors.
        for (idx=0; idx<1; idx++){
        
            for (jdx=0; jdx<(internal_data->order)+1; jdx++){
            
                for (kdx=0; kdx<internal_data->input_data_length; kdx++){
                
                    internal_data->avg_sd_input_data_powers[idx][jdx] += internal_data->input_data_powers[kdx][jdx];
                }
                internal_data->avg_sd_input_data_powers[idx][jdx] /= (double)internal_data->input_data_length;
            }
        }
        
        // calculate the standard deviations of all power vectors.
        for (idx=1; idx<2; idx++){
        
            for (jdx=0; jdx<(internal_data->order)+1; jdx++){
                
                for (kdx=0; kdx<internal_data->input_data_length; kdx++){
                
                    internal_data->avg_sd_input_data_powers[idx][jdx] += pow((internal_data->input_data_powers[kdx][jdx] - internal_data->avg_sd_input_data_powers[0][jdx]),2);
                }
                internal_data->avg_sd_input_data_powers[idx][jdx] = sqrt(internal_data->avg_sd_input_data_powers[idx][jdx] /= (double)internal_data->input_data_length);
            }   
        }  
    }
    // #################################################################################################
       
    switch(setjmp(env)){
        case 0: calc_avg_sd(internal_data); return EXIT_SUCCESS;                         
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }    
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_cov_cor_input_data_powers(struct dataset *internal_data){

    /*
        DESCRIPTION:
        calculates the covariance matrix of input points and intra-correlation matrix of predictors.
    
        INPUT:
        struct dataset *internal_data	...	pointer to the internal dataset 
             
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        --
    */
    
    int idx, jdx, kdx;
    jmp_buf env;    
    
    // ########################################### functions ###########################################  
    void calc_cov_cor(struct dataset *internal_data){
    
        // calculate the covariance matrix:
        for (idx=0; idx<internal_data->order; idx++){
    
            for (jdx=0; jdx<internal_data->order; jdx++){
        
                for (kdx=0; kdx<internal_data->input_data_length; kdx++){
                
                    internal_data->cov_data_powers[idx][jdx] += ((internal_data->input_data_powers[kdx][idx] - internal_data->avg_sd_input_data_powers[0][idx]) * 
                                                                 (internal_data->input_data_powers[kdx][jdx] - internal_data->avg_sd_input_data_powers[0][jdx]));
                }
                internal_data->cov_data_powers[idx][jdx] /= (double)internal_data->input_data_length;
            }
        }
        
        // calculate the intra-correlation matrix:
        for (idx=0; idx<internal_data->order; idx++){
    
            for (jdx=0; jdx<internal_data->order; jdx++){
        
                internal_data->cor_data_powers[idx][jdx] = internal_data->cov_data_powers[idx][jdx] / (internal_data->avg_sd_input_data_powers[1][idx] * internal_data->avg_sd_input_data_powers[1][jdx]);
            }
        }                             
    }
    // #################################################################################################
       
    switch(setjmp(env)){
        case 0: calc_cov_cor(internal_data); return EXIT_SUCCESS;                         
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n>>>%s\n\n", __FILE__, __func__, strerror(errno)); return EXIT_FAILURE;
    } 

}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_determinat(struct dataset *internal_data){

    /*
        DESCRIPTION:
        calculates the determinant of a square matrix
    
        INPUT:
        struct dataset *internal_data	...	pointer to the internal dataset 
             
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        --
    */
    
    int idx, jdx, kdx;
    double sum=0;
    double DetL = 1.0;				// Determinant of lower-diagonal-matrix
    double DetU = 1.0;				// Determinant of upper-diagonal-matrix
    double Det;					// Determinant
    
    jmp_buf env;    
    
    // ########################################### functions ###########################################  
    void calc_det(struct dataset *internal_data){
    
        for (idx=0; idx<internal_data->order; idx++){
    
            internal_data->upper_matrix[idx][idx] = 1;
        }

        for (jdx=0; jdx<internal_data->order; jdx++){
	    for (idx=jdx; idx<internal_data->order; idx++){
	
	        sum=0;
	    
	        for (kdx=0; kdx<jdx; kdx++){
	    
		    sum = sum + internal_data->lower_matrix[idx][kdx] * internal_data->upper_matrix[kdx][jdx];
	        }
	    
	        internal_data->lower_matrix[idx][jdx] = internal_data->cor_data_powers[idx][jdx] - sum;
	    }

	    for (idx=jdx; idx<internal_data->order; idx++){
	
	        sum = 0;
	    
	        for(kdx=0; kdx<jdx; kdx++){
	    
	            sum = sum + internal_data->lower_matrix[jdx][kdx] * internal_data->upper_matrix[kdx][idx];
	        }
	        
	        if (internal_data->lower_matrix[jdx][jdx] == 0){
	    
	            longjmp(env, 1);
	        }
	        internal_data->upper_matrix[jdx][idx] = (internal_data->cor_data_powers[jdx][idx] - sum) / internal_data->lower_matrix[jdx][jdx];
	    }
        }
    
        // multiply the diagonal of the columns
        for (idx=0; idx<internal_data->order; idx++){
    
            DetL = DetL * internal_data->lower_matrix[idx][idx];
            DetU = DetU * internal_data->upper_matrix[idx][idx];

        }    

        // calculate the determinant by Det(L) * Det(U)
        Det = DetL * DetU;

        // check for nan:
        if (isnan(Det)){
            longjmp(env, 2);
        }
        
        // check for determinant equal to 0:
        if ((EPS-Det) > 0){
            internal_data->det_cor = 0.0;            
        }
        else{
            internal_data->det_cor = Det;
        }
    }
    // #################################################################################################
       
    switch(setjmp(env)){
        case 0: calc_det(internal_data); return EXIT_SUCCESS; 
        case 1: fprintf(stderr, "ERROR: ( %s -> %s)\n>>> Determinant of lower Matrix close to 0. It's not possible to divide by 0!\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: ( %s -> %s)\n>>> An Error occured! The determinant is nan!\n", __FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n>>>%s\n\n", __FILE__, __func__, strerror(errno)); return EXIT_FAILURE;
    }
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_inverse_matrix(struct dataset *internal_data){

    /*
        DESCRIPTION:
        Determines the inverse of a square matrix of type double.
            
        INPUT:
        struct dataset *internal_data	...	pointer to the internal dataset
             
        OUTPUT:
        Outputs an error code:
        success:		...	1 (EXIT_SUCCESS)
        failure:		...	0 (EXIT_FAILURE)       
    */

    int idx, jdx, kdx;
    jmp_buf env;

    // ########################################### functions ########################################### 
    void calc_inverse(struct dataset *internal_data){

        double temp;

        // copy the content of the intra-correlation matrix to a temporary matrix
        for(idx=0; idx<internal_data->order; idx++){
            for(jdx=0; jdx<internal_data->order; jdx++){
            					
	        internal_data->cor_data_powers_temp[idx][jdx] = internal_data->cor_data_powers[idx][jdx];
	    }
        }
        
        // Set the diagonal values of the inverse matrix to 1 else 0
        for(idx=0; idx<internal_data->order; idx++){
        
            internal_data->cor_data_powers_inverted[idx][idx] = 1;
	}

    
        // determine the inversed intra-correlation-matrix:
        for(idx=0 ;idx<internal_data->order; idx++){
    														
	    temp = internal_data->cor_data_powers_temp[idx][idx];
											
            for(jdx=0; jdx<internal_data->order; jdx++){
        
	        internal_data->cor_data_powers_temp[idx][jdx] /= temp;
	        internal_data->cor_data_powers_inverted[idx][jdx] /= temp;
	    }
														
	    for(kdx=0; kdx<internal_data->order; kdx++){
	
	        temp = internal_data->cor_data_powers_temp[kdx][idx];
	    									
	        for(jdx=0; jdx<internal_data->order; jdx++){
	    												
		    if(kdx == idx){
		        break;
		    }
											
		    internal_data->cor_data_powers_temp[kdx][jdx] -= internal_data->cor_data_powers_temp[idx][jdx] * temp;
								
		    internal_data->cor_data_powers_inverted[kdx][jdx] -= internal_data->cor_data_powers_inverted[idx][jdx] * temp;
	        }
	    }
        }
        
        // check for nan- and inf-values within the inversed matrix
        for (idx=0; idx<internal_data->order; idx++){
            for (jdx=0; jdx<internal_data->order; jdx++){
                if (isnan(internal_data->cor_data_powers_inverted[idx][jdx]) || 
                    isinf(internal_data->cor_data_powers_inverted[idx][jdx])){
                    
                    longjmp(env, 1);    
                }
            }
        }
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: calc_inverse(internal_data); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The inversed matrix contains \"nan\" or \"inf\" values\n", __FILE__, __func__); return EXIT_FAILURE;        
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }   
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_cor_cov_crieria(struct dataset *internal_data){

    /*
        DESCRIPTION:
        Calculates the vectors of intra-criteria-correlations and covariances
            
        INPUT:
        struct dataset *internal_data	...	pointer to the internal dataset
             
        OUTPUT:
        Outputs an error code:
        success:		...	1 (EXIT_SUCCESS)
        failure:		...	0 (EXIT_FAILURE)       
    */

    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ########################################### 
    void calc_criteria(struct dataset *internal_data){

        // Calculate the vector of intra-criteria-covariance:
        for (idx=0; idx<internal_data->order; idx++){
            for (jdx=0; jdx<internal_data->input_data_length; jdx++){
            
                internal_data->cov_data_powers_criteria[idx] += ((internal_data->input_data_powers[jdx][idx] - 
                                                                  internal_data->avg_sd_input_data_powers[0][idx]) * (internal_data->input_data_powers[jdx][internal_data->order] - 
                                                                                                                      internal_data->avg_sd_input_data_powers[0][internal_data->order]));
            }
            internal_data->cov_data_powers_criteria[idx] /= (double)internal_data->input_data_length;
        }
        
        // check for "nan"- and "inf"-values:
        for (idx=0; idx<internal_data->order; idx++){
            if (isnan(internal_data->cov_data_powers_criteria[idx]) || 
                isinf(internal_data->cov_data_powers_criteria[idx])){
                longjmp(env, 1);
            }
        }
    
        // Calculate the vector of intra-criteria-correlation:   
        for (idx=0; idx<internal_data->order; idx++){
        
            internal_data->cor_data_powers_criteria[idx] = internal_data->cov_data_powers_criteria[idx] / ((internal_data->avg_sd_input_data_powers[1][idx]) * 
                                                                                                           (internal_data->avg_sd_input_data_powers[1][internal_data->order]));
        }
        
        // check for "nan"- and "inf"-values:
        for (idx=0; idx<internal_data->order; idx++){
            if (isnan(internal_data->cor_data_powers_criteria[idx]) || 
                isinf(internal_data->cor_data_powers_criteria[idx])){
                longjmp(env, 2);
            }
        }          
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: calc_criteria(internal_data); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The vector of intra-criteria-covariance contains \"nan\" or \"inf\" values\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The vector of intra-criteria-correlation contains \"nan\" or \"inf\" values\n", __FILE__, __func__); return EXIT_FAILURE;          
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }

}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_beta_weights(struct dataset *internal_data){

    /*
        DESCRIPTION:
        Calculates the vector of beta weights by multiplication of the inverse matrix of intra-correlation 
        and the vector of criteria correlation..
    
        INPUT:
        struct dataset *internal_data	...	pointer to the internal data.
             
        OUTPUT:
        Outputs an error code:
        success:		...	1 (EXIT_SUCCESS)
        failure:		...	0 (EXIT_FAILURE)       
    */
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ########################################### 
    void calc_weights(struct dataset *internal_data){
    
        // Determination of beta-weights.
        for (idx=0; idx<internal_data->order; idx++){
    
            for (jdx=0; jdx<internal_data->order; jdx++){
            
                internal_data->beta_weights[jdx] += (internal_data->cor_data_powers_inverted[jdx][idx] * 
                                                     internal_data->cor_data_powers_criteria[idx]);
            }
        }
    
        
        // check for "nan"- and "inf"-values:
        for (idx=0; idx<internal_data->order; idx++){
            if (isnan(internal_data->beta_weights[idx]) || 
                isinf(internal_data->beta_weights[idx])){
                longjmp(env, 1);
            }
        }         
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: calc_weights(internal_data); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The vector of beta-weights contains \"nan\" or \"inf\" values\n", __FILE__, __func__); return EXIT_FAILURE;       
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); EXIT_FAILURE;
    }
    
    
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calc_b_weights(struct dataset *internal_data){

    /*
        DESCRIPTION:
        Calculates the vector of b weights by multiplication of the inverse matrix of intra-correlation 
        and the vector of criteria correlation..
    
        INPUT:
        struct dataset *internal_data	...	pointer to the internal data.
             
        OUTPUT:
        Outputs an error code:
        success:		...	1 (EXIT_SUCCESS)
        failure:		...	0 (EXIT_FAILURE)       
    */
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ########################################### 
    void calc_weights(struct dataset *internal_data){
    
        // Calculates the vector of b weights
        for (idx=0; idx<internal_data->order+1; idx++){
        
            if (idx == 0){
                internal_data->b_weights[idx] = internal_data->avg_sd_input_data_powers[0][internal_data->order];
            
                continue;
            }
            else{
                internal_data->b_weights[idx] = (internal_data->avg_sd_input_data_powers[1][internal_data->order] / 
                                                 internal_data->avg_sd_input_data_powers[1][idx-1]) * internal_data->beta_weights[idx-1];
            }
    
        }
        for (idx=1; idx<internal_data->order+1; idx++){
            internal_data->b_weights[0] = internal_data->b_weights[0] - (internal_data->avg_sd_input_data_powers[0][idx-1] * internal_data->b_weights[idx]);
        }
        
        // check for "nan"- and "inf"-values:
        for (idx=0; idx<internal_data->order+1; idx++){
            if (isnan(internal_data->b_weights[idx]) || 
                isinf(internal_data->b_weights[idx])){
                longjmp(env, 1);
            }
        }
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: calc_weights(internal_data); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The vector of b-weights contains \"nan\" or \"inf\" values\n", __FILE__, __func__); return EXIT_FAILURE;       
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); EXIT_FAILURE;
    }
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int get_coefficient_of_determination(struct dataset *internal_data){

    /*
        DESCRIPTION:
        Calculates the coefficient of determination out of the predicted datapoints.
    
        INPUT:
        struct dataset *internal_data	...	pointer to the internal data.
             
        OUTPUT:
        Outputs an error code:
        success:		...	1 (EXIT_SUCCESS)
        failure:		...	0 (EXIT_FAILURE)       
    */
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ########################################### 
    void calc_coeff(struct dataset *internal_data){
    
        double sumDiff_y0=0;	// sum of the squared differences between observed datapoints and their average.
        double sumDiff_y=0;	// sum of the squared differences between predicted datapoints and their average.
        double temp;		// just a temporary value for checking purposes
    
        // Calculation of the predicted values for the given points and their average:
        // =============================================================================================
        for (idx=0; idx<internal_data->input_data_length; idx++){
    
            internal_data->predicted_values[idx] = internal_data->b_weights[0];

            for (jdx=0; jdx<internal_data->order; jdx++){
            
                temp = internal_data->b_weights[jdx+1] * internal_data->input_data_powers[idx][jdx];
                
                // check for nan- or inf-values
                if (isnan(temp) || isinf(temp)){
                    longjmp(env, 1);
                }
                else{
                    internal_data->predicted_values[idx] += temp;
                } 
            }
            // sum up these values to calculate the average later on
            internal_data->avg_sd_input_data_powers[0][(internal_data->order)+1] += internal_data->predicted_values[idx];
        }
        
        // Calculation of the average of the predicted values:
        internal_data->avg_sd_input_data_powers[0][(internal_data->order)+1] /= internal_data->input_data_length;

        
        
        // Calculation of the coefficient of determination:
        // =============================================================================================
        for (idx=0; idx<internal_data->input_data_length; idx++){
    
            // sum of the squared differences between observed datapoints and their average.
            temp = pow((internal_data->input_data[idx][1] - internal_data->avg_sd_input_data_powers[0][internal_data->order]),2);
            if (isnan(temp) || isinf(temp)){
                longjmp(env, 2);
            }
            else{
                sumDiff_y0 += temp;
            }
            
            // sum of the squared differences between predicted datapoints and their average.
            temp = pow((internal_data->predicted_values[idx] - internal_data->avg_sd_input_data_powers[0][(internal_data->order)+1]),2);
            if (isnan(temp) || isinf(temp)){
                longjmp(env, 2);
            }
            else{
                sumDiff_y += temp;
            }   
        }
    
        // Calculation of the coefficient of determination by the division of the sum of the squared differences
        internal_data->b_weights[internal_data->order+1] = ((double)sumDiff_y / (double)sumDiff_y0);
        if (isnan(internal_data->b_weights[0]) || isinf(internal_data->b_weights[0])){
            longjmp(env, 3);
        }
        
        for (idx=0; idx<internal_data->order+2; idx++){
            printf("%6.3f\n",internal_data->b_weights[idx]);
        }
       
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: calc_coeff(internal_data); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> One of the predicted values is \"nan\" or \"inf\"!\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: (%s -> %s)\n>>> One of the squared differences are \"nan\" or \"inf\"!\n", __FILE__, __func__); return EXIT_FAILURE;
        case 3: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The coefficient of determination is \"nan\" or \"inf\"\n!", __FILE__, __func__); return EXIT_FAILURE;       
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); EXIT_FAILURE;
    }
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int allocate_fmatrix(double ***matrix, unsigned int rows, unsigned int columns){

    /*
        DESCRIPTION:
        Allocates memory for a rows x columns matrix of datatype double.
    
        INPUT:
        double ***matrix	...	address of the pointer of the matrix
        unsigned int rows	...	number of rows
        unsigned int cols	...	number of columns
             
        OUTPUT:
        Outputs an error code:
        success:		...	1 (EXIT_SUCCESS)
        failure:		...	0 (EXIT_FAILURE)       
    */
    
    int idx, jdx;
    jmp_buf env;

    // ########################################### functions ########################################### 
    void allocate_matrix(double ***matrix, unsigned int rows, unsigned int columns){
    
        double **ptr;
    
        // check if rows and columns are greater than 0
        if ((rows <= 0) || (columns <= 0)){
            longjmp(env, 1);
        }
    
        ptr = (double**) calloc(rows, sizeof(double*));
        if (ptr == NULL){
            longjmp(env, 2);
        }
        else{
            for (idx=0; idx<rows; idx++){
                ptr[idx] = (double*) calloc(columns, sizeof(double));
                if (ptr[idx] == NULL){
                
                    // clean memory until failure happend:
                    for (jdx=0; jdx<idx; jdx++){
                        free(ptr[jdx]);
                    }
                    free(ptr);
                    
                    longjmp(env, 2);                    
                }
            }
            // assign the address of ptr to the address of the matrix
            *matrix = ptr;
        }
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: allocate_matrix(matrix, rows, columns); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The number of rows and columns has to be greater than 0.\n", __FILE__, __func__); EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: (%s -> %s)\n>>> %s.\n", __FILE__, __func__, strerror(errno)); EXIT_FAILURE;        
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); EXIT_FAILURE;
    }
          
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int allocate_fvector(double **vector, unsigned int length){

    /*
        DESCRIPTION:
        Allocates memory for a vector with the lenght of "length" and of datatype double.
    
        INPUT:
        double **vector		...	address of the pointer of the vector
        unsigned int length	...	length of the vector
             
        OUTPUT:
        Outputs an error code:
        success:		...	1 (EXIT_SUCCESS)
        failure:		...	0 (EXIT_FAILURE)       
    */
    
    int idx, jdx;
    jmp_buf env;

    // ########################################### functions ########################################### 
    void allocate_vector(double **vector, unsigned int length){
    
        double *ptr;
    
        // check if length is greater than 0
        if (length <= 0){
            longjmp(env, 1);
        }
        else{
        
            ptr = (double*) calloc(length, sizeof(double));
            if (ptr == NULL){
                longjmp(env, 2);
            }
            else{
                // assign the address of ptr to the address of the vector
                *vector = ptr;
            }
        }
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: allocate_vector(vector, length); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The length of the vector has to be greater than 0.\n", __FILE__, __func__); EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: (%s -> %s)\n>>> %s.\n", __FILE__, __func__, strerror(errno)); EXIT_FAILURE;        
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); EXIT_FAILURE;
    }
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


void free_fmatrix_memory(double **matrix, unsigned int rows, unsigned int columns){

    /*
        DESCRIPTION:
        frees the allocated memory of a rows x cols matrix of datatype double.
    
        INPUT:
        double **matrix		...	Pointer to the allocated memory to free
        unsigned int rows	...	number of rows
        unsigned int cols	...	number of columns
             
        OUTPUT:
        --
    */    
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ###########################################     
    void free_memory(double **matrix, unsigned int rows, unsigned int columns){
    
        //printf("rows: %d, cols: %d\n", rows, columns);
    
        // check if rows and columns are greater than 0
        if ((rows <= 0) || (columns <= 0)){
            longjmp(env, 1);
        }
    
        // free the memory:
        for (idx=0; idx<rows; idx++){
            free(matrix[idx]);
        }
        free(matrix);
        
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: free_memory(matrix, rows, columns); break;
        case 1: fprintf(stderr,"ERROR:( %s -> %s)\n>>> The number of rows and columns has to be greater than 0.\n", __FILE__, __func__); break;         
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); break;
    }     
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


void free_fvector_memory(double *vector, unsigned int length){

    /*
        DESCRIPTION:
        frees the allocated memory of a vector with length "length" and of datatype double.
    
        INPUT:
        double *vector		...	Pointer to the allocated memory to free
        unsigned int length	...	length of the vector
             
        OUTPUT:
        --
    */
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ###########################################     
    void free_memory(double *vector, unsigned int length){
    
        //printf("length: %d\n", length);
    
        // check if rows and columns are greater than 0
        if (length <= 0){
            longjmp(env, 1);
        }
        else{
            // free the memory:        
            free(vector);
        }
        
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: free_memory(vector, length); break;
        case 1: fprintf(stderr,"ERROR:( %s -> %s)\n>>> The length of the vector has to be greater than 0.\n", __FILE__, __func__); break;         
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); break;
    } 

}
















