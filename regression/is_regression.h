
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
    double **powers_of_input_data;
    
};






// ######################################################################## function declaration #################################################################################
// int *polynomial_regression(double **input_data, const int length, const int n);

int read_input_data(double **input_data, const int length, const int order, struct dataset *data);	// reads the input data / performs some checks / imports to internal dataset.
int allocate_fmatrix(double **matrix, unsigned int rows, unsigned int columns);					// allocates memory for a rows x colmuns matrix of type double.

void free_fmatrix_memory(double **matrix, unsigned int rows, unsigned int columns);			// frees the allocated memory of a matrix of datatype double.

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
    
    
    // Allocate a matrix to calculate the powers of the input data.
    err = allocate_fmatrix(internal_data.powers_of_input_data, 
                           internal_data.input_data_length, 
                           (internal_data.order+1));
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    printf("%p\n", internal_data.powers_of_input_data);
    
    
    
    // #################################### CLEAN UP ##########################################
    // free the internal dataset of the input data:
    free_fmatrix_memory(internal_data.input_data, 
                        internal_data.input_data_length, 
                        2);
    // free the internal matrix of power values:
    //free_fmatrix_memory(internal_data.powers_of_input_data,
    //                    internal_data.input_data_length, 
    //                    (internal_data.order+1));
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


int calculate_powers_of_input_data(struct dataset *internal_data){

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


int allocate_fmatrix(double **matrix, unsigned int rows, unsigned int columns){

    /*
        DESCRIPTION:
        Allocates memory for a rows x columns matrix of datatype double.
    
        INPUT:
        double **matrix		...	pointer to the allocated memory of type double.
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
    void allocate_matrix(double **matrix, unsigned int rows, unsigned int columns){
    
        printf("rows: %d, cols: %d\n",rows, columns);
    
        // check if rows and columns are greater then 0
        if ((rows <= 0) || (columns <= 0)){
            longjmp(env, 1);
        }
    
        matrix = (double**) calloc(rows, sizeof(double*));
        if (matrix == NULL){
            longjmp(env, 2);
        }
        else{
            for (idx=0; idx<rows; idx++){
                matrix[idx] = (double*) calloc(columns, sizeof(double));
                if (matrix[idx] == NULL){
                    longjmp(env, 2);                    
                }
            }
        }
        printf("allocated: %p\n", matrix);
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: allocate_matrix(matrix, rows, columns); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR:( %s -> %s)\n>>> The number of rows and columns has to be greater then 0.\n", __FILE__, __func__); EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR:( %s -> %s)\n>>> %s.\n", __FILE__, __func__, strerror(errno)); EXIT_FAILURE;        
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); EXIT_FAILURE;
    }
          
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


void free_fmatrix_memory(double **matrix, unsigned int rows, unsigned int columns){

    /*
        DESCRIPTION:
        frees the allocated memory of a rows x cols matrix of datatype double.
    
        INPUT:
        double matrix		...	allocated memory to free
        unsigned int rows	...	number of rows
        unsigned int cols	...	number of columns
             
        OUTPUT:
        --
    */    
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ###########################################     
    void free_memory(double **matrix, unsigned int rows, unsigned int columns){
    
        printf("rows: %d, cols: %d\n",rows, columns);
    
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




















