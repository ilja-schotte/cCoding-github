
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <setjmp.h>
    #include <errno.h>
    #include <stdint.h>
    #include <string.h>
#endif



// ######################################################################### struct declaration ##################################################################################

struct dataset{

    unsigned int order;
    unsigned int input_data_length;
    double **input_data;
    double **input_data_powers;			// powers of the input values
    double **avg_sd_input_data_powers;		// average and standard deviation of the powers of the input values
    double **cov_data_powers;			// covariance matrix of the powers of the input values
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
};






// ######################################################################## function declaration #################################################################################
// int *polynomial_regression(double **input_data, const int length, const int n);

int read_input_data(double **input_data, const int length, const int order, struct dataset *data);	// reads the input data / performs some checks / imports to internal dataset.
int allocate_fmatrix(double ***matrix, unsigned int rows, unsigned int columns);			// allocates memory for a rows x colmuns matrix of type double.
int allocate_fvector(double **vector, unsigned int length);						// allocates memory for a vector of length "length" and of type double.

void free_fmatrix_memory(double **matrix, unsigned int rows, unsigned int columns);			// frees the allocated memory of a matrix of datatype double.
void free_fvector_memory(double *vector, unsigned int length);						// frees the allocated memory of a vector of datatype double.

double *polynomial_regression(double **input_data, const int length, const int n);			// function to perform a polynimocal regression.


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ######################################################################## function definition ##################################################################################


double *polynomial_regression(double **input_data, const int length, const int order){


    int err = EXIT_FAILURE;
    
    // declare internal data:
    struct dataset internal_data = {
        .order = 0,
        .input_data_length = 0,
    };
    
    
    // read the input_dataset, perform some checks and add the arguments to the internal dataset:
    err = read_input_data(input_data, 
                          length, order, 
                          &internal_data);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    
    
    // ########################################################################################
    // ################################ MEMORY ALLOCATION #####################################
       
    // Allocate a matrix to calculate the powers of the input data.
    err = allocate_fmatrix(&(internal_data.input_data_powers), 
                           internal_data.input_data_length, 
                           (internal_data.order+1));
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    // Allocate a matrix to calculate the average and standard deviation of the powers of the input data.    
    err = allocate_fmatrix(&(internal_data.avg_sd_input_data_powers), 
                           2, 
                           (internal_data.order+2));
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }    

     // Allocate a matrix to calculate the covariances of the powers of the input data.    
    err = allocate_fmatrix(&(internal_data.cov_data_powers), 
                           internal_data.order, 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
     // Allocate a matrix for the correlation of predictors    
    err = allocate_fmatrix(&(internal_data.cor_data_powers), 
                           internal_data.order, 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    // Allocate a matrix for temporary calculations of the correlation of predictors    
    err = allocate_fmatrix(&(internal_data.cor_data_powers_temp), 
                           internal_data.order, 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    // Allocate a matrix for the inverted correlation matrix of the predictors    
    err = allocate_fmatrix(&(internal_data.cor_data_powers_inverted), 
                           internal_data.order, 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    // Allocate memory for the lower matrix to calculate the determinat
    err = allocate_fmatrix(&(internal_data.lower_matrix), 
                           internal_data.order, 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }    
    
    // Allocate memory for the lower matrix to calculate the determinat
    err = allocate_fmatrix(&(internal_data.upper_matrix), 
                           internal_data.order, 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
          
    // Allocate a vector for the intra-criteria-correlation
    err = allocate_fvector(&(internal_data.cor_data_powers_criteria), 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }  
    
    // Allocate a vector for the intra-criteria-covariances
    err = allocate_fvector(&(internal_data.cov_data_powers_criteria), 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    // Allocate a vector for the beta-weights
    err = allocate_fvector(&(internal_data.beta_weights), 
                           internal_data.order);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }    
    
    // Allocate a vector for the b-weights and coefficient of determination
    err = allocate_fvector(&(internal_data.b_weights), 
                           internal_data.order+2);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    // Allocate a vector for the predicted values of the coefficients
    err = allocate_fvector(&(internal_data.predicted_values), 
                           internal_data.input_data_length);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    // ########################################################################################
    // #################################### CLEAN UP ##########################################
    
    // free the internal dataset of the input data:
    free_fmatrix_memory(internal_data.input_data, 
                        internal_data.input_data_length, 
                        2);
    // free the internal matrix of power values:
    free_fmatrix_memory(internal_data.input_data_powers,
                        internal_data.input_data_length, 
                        (internal_data.order+1));
    // free the internal matrix of the averages and standard deviations of the powers of input data:
    free_fmatrix_memory(internal_data.avg_sd_input_data_powers,
                        2, 
                        (internal_data.order+2));
    // free the internal matrix of covariances of the powers of input data:
    free_fmatrix_memory(internal_data.cov_data_powers,
                        internal_data.order, 
                        internal_data.order);
    // free the internal matrix of the correlation of predictors:
    free_fmatrix_memory(internal_data.cor_data_powers,
                        internal_data.order, 
                        internal_data.order);
    // free the internal matrix for temporary calculations on the matrix of intra-correlations of predictors:
    free_fmatrix_memory(internal_data.cor_data_powers_temp,
                        internal_data.order, 
                        internal_data.order);
    // free the internal inverted matrix of the correlation of predictors:
    free_fmatrix_memory(internal_data.cor_data_powers_inverted,
                        internal_data.order, 
                        internal_data.order); 
    // free the internal lower matrix for calculation of determinant:
    free_fmatrix_memory(internal_data.lower_matrix,
                        internal_data.order, 
                        internal_data.order);                        
    // free the internal upper matrix for calculation of determinant:
    free_fmatrix_memory(internal_data.upper_matrix,
                        internal_data.order, 
                        internal_data.order);
     // free the internal vector of the intra-criteria-correlation:
    free_fvector_memory(internal_data.cor_data_powers_criteria,
                        internal_data.order); 
     // free the internal vector of the intra-criteria-covariances:
    free_fvector_memory(internal_data.cov_data_powers_criteria,
                        internal_data.order);                        
    // free the internal vector of the beta-weights:
    free_fvector_memory(internal_data.beta_weights,
                        internal_data.order);
    // free the internal vector of the b-weights and coefficient of determination:
    free_fvector_memory(internal_data.b_weights,
                        internal_data.order+2);
    // free the internal vector of predicted values of the coefficients:
    free_fvector_memory(internal_data.predicted_values,
                        internal_data.input_data_length);                                              
                                          
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
        if (order > length){
            longjmp(env, 1);
        }
        else{
            // set order of polynomial function:
            internal_data->order = order;
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
        case 1: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The order of the polynomial function cannot be higher then the number of datapoints.\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The length of the input dataset has to be higher then 0.\n", __FILE__, __func__); return EXIT_FAILURE;        
        case 3: fprintf(stderr,"ERROR: ( %s -> %s)\n>>> The dataset consists nan - values.\n", __FILE__, __func__); return EXIT_FAILURE;              
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }    
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int calculate_input_data_powers(struct dataset *internal_data){

    /*
        DESCRIPTION:
        Reads the input dataset and performs several checks to it.
    
        INPUT:
        double **input_data	...	Pointer of type double to the input dataset in shape of:
        				x-values: input_data[0 bis n][0]
        				y-values: input_data[0 bis n][1]
                                		
        const int length        ...	Number of points.
        const int n		...	order of regression.
             
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - the order of the polynomial function must be lower or equal to the number of input points. 
        - 
    */

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
    
        // check if rows and columns are greater then 0
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
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The number of rows and columns has to be greater then 0.\n", __FILE__, __func__); EXIT_FAILURE;
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
    
        // check if length is greater then 0
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
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The length of the vector has to be greater then 0.\n", __FILE__, __func__); EXIT_FAILURE;
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
    
        printf("rows: %d, cols: %d\n", rows, columns);
    
        // check if rows and columns are greater then 0
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
        case 1: fprintf(stderr,"ERROR:( %s -> %s)\n>>> The number of rows and columns has to be greater then 0.\n", __FILE__, __func__); break;         
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
    
        printf("length: %d\n", length);
    
        // check if rows and columns are greater then 0
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
        case 1: fprintf(stderr,"ERROR:( %s -> %s)\n>>> The length of the vector has to be greater then 0.\n", __FILE__, __func__); break;         
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); break;
    } 

}
















