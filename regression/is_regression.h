
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <setjmp.h>
    #include <errno.h>
    #include <stdint.h>
#endif



// ######################################################################### struct declaration ##################################################################################

struct dataset{

    uint8_t order;
    int input_data_length;
    double **input_data;
    
};






// ######################################################################## function declaration #################################################################################
// int *polynomial_regression(double **input_data, const int length, const int n);

int read_input_data(double **input_data, const int length, const int order, struct dataset *data);





double *polynomial_regression(double **input_data, const int length, const int n);





// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ######################################################################## function definition ##################################################################################


double *polynomial_regression(double **input_data, const int length, const int order){


    int err = EXIT_FAILURE;
    
    // declare internal data:
    struct dataset data = {
        .order = 0,
        .input_data_length = 0,
    };
    
    
    // read the input_dataset, perform some checks and add the arguments to the internal dataset:
    err = read_input_data(input_data, length, order, &data);
    if (err == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }
    
    
    
}


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################


int read_input_data(double **input_data, const int length, const int order, struct dataset *data){

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
    
    int idx, jdx;
    jmp_buf env;
    
    // ########################################### functions ###########################################  
    void read_data(double **input_data, const int length, const int order, struct dataset *data){
    
        // check for length and order of input_dataset
        if (order > length){
            longjmp(env, 1);
        }
        
        // check for nan - values:
        for (idx=0; idx<length; idx++){
            for (jdx=0; jdx<2; jdx++){
                if (isnan(input_data[idx][jdx])){
                    longjmp(env, 2);
                }
            }      
        }
        
        // set order of polynomial function:
        data->order = order;
        
        // import input_dataset to the internal data:
        data->input_data;
        
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: read_data(input_data, length, order, data); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: The order of the polynomial function cannot be higher then the number of datapoints.\n"); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR: The dataset consists nan - values.\n"); return EXIT_FAILURE;              
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }    
    
    
    
}
























