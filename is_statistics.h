
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <math.h>
    #include <string.h>
    #include <setjmp.h>
    #include <errno.h>
#endif



// functions for arrays of doubles
int is_farr_sum(double *array, int length, double *result);	// sum of over all values of an array				(dtype: double)
int is_farr_maximum(double *array, int length, double *result);	// maximum value of an array 					(dtype: double)
int is_farr_minimum(double *array, int length, double *result);	// minimum value of an array 					(dtype: double)
int is_farr_median(double *array, int length, double *result);	// median of an array						(dtype: double)
int is_farr_average(double *array, int length, double *result);	// average of an array						(dtype: double)
int is_farr_variance(double *array, int length, double *result);// variance of an array						(dytpe: double)
int is_farr_stddev(double *array, int length, double *result);	// standard deviation of an array				(dtype: double)
int is_farr_sort(double *array, int length, char *dir);		// sorts an array in ascending or descending order		(dtype: double)
int is_farr_reverse(double *array, int length);			// changes the order of values in an array in reverse direction	(dtype: double)


// functions for arrays of integers
int is_darr_sum(int *array, int length, double *result);	// sum over all values of an array				(dtype: integer)
int is_darr_maximum(int *array, int length, double *result);	// maximum value of an array					(dtype: integer)
int is_darr_minimum(int *array, int length, double *result);	// minimum value of an array					(dytpe: integer)
int is_darr_median(int *array, int length, double *result);	// median of an array						(dtype: integer)
int is_darr_average(int *array, int length, double *result);	// average of an array						(dtype: integer)
int is_darr_variance(int *array, int length, double *result);	// variance of an array						(dtype: integer)
int is_darr_stddev(int *array, int length, double *result);	// standardd eviation of an array				(dtype: integer)
int is_darr_sort(int *array, int length, char *dir);		// sorts an array in ascending or descending order		(dtype: integer)
int is_darr_reverse(int *array, int length);			// changes the order of values in an array in reverse direction	(dtype: integer)

// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ########################################################################## double - functions #################################################################################

int is_farr_sum(double *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the sum over all values of an input array (double *array).
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks if the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    void calc_sum(double *array, int length, double *result){
    
    
        double sum = 0.0;
    
        // ###########################################
        // ###########################################   
    
        // ################# CHECKS ##################    
        // check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // check for NaN values
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }

        // return the only one value if length is 1.
        if (length == 1){
            *result = array[0];
        }
        else{     
        
            for (idx=0; idx<length; idx++){
    
                sum += array[idx];
            }
            *result = sum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: calc_sum(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr,"ERROR! ( %s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    } 
    
}


// ########################################################################################
// ########################################################################################


int is_farr_maximum(double *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the maximum value of an input array (double *array).
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks if the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    void calc_maximum(double *array, int length, double *result){
    
    
        double maximum;
        
        // ###########################################
        // ###########################################    
    
        // ################# CHECKS ##################    
        // check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // check for NaN values
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }
        
        // return the only one value if length is 1.
        if (length == 1){
            *result = array[0];
        }
        else{     
        
            maximum = array[0];
    
            for (idx=0; idx<length; idx++){
    
                if (array[idx] > maximum){
        
                    maximum = array[idx];
                }
            }
            *result = maximum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: calc_maximum(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! ( %s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    } 
    
}

// ########################################################################################
// ########################################################################################


int is_farr_minimum(double *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the minimum value of an input array (double *array).
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks if the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    void calc_minimum(double *array, int length, double *result){
    
    
        double minimum;
    
        // ###########################################
        // ###########################################    
    
        // Check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // Check for NaN-values
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }
        
        // return the only one value if length is 1.
        if (length == 1){
            *result = array[0];
        }
        else{
            minimum = array[0];
    
            for (idx=0; idx<length; idx++){
    
                if (array[idx] < minimum){
        
                    minimum = array[idx];
                }
            }
            *result = minimum;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_minimum(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_farr_average(double *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the average over all values of an input array (double *array).
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    void calc_average(double *array, int length, double *result){
    
    
        double average; 
            
        // ###########################################            
        // ###########################################   
                
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
        // Check for NaN values:
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }
        
        // return the only one value if length is 1:
        if (length == 1){
            *result = array[0];
        }
        else{
        
            if (!(is_farr_sum(array, length, &average))){
            
                *result = average /= length;
            }
            else{
                longjmp(env, 3);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_average(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_farr_sum\" returns an error.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_farr_median(double *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the median value over all values of an input array (double *array).
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
        double *result	...	result value of this function
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)        
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void calc_median(double *array, int length, double *result){
    
    
        double median;  
          
        // ###########################################
        // ###########################################  
          
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // Check for NaN values:
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }

        // median is equal to the only one value if length is 1:
        if (length == 1){
            *result = array[0];
        }
        else{
        
            if (!(is_farr_sort(array, length, "asc"))){
            
                median = (length % 2) ? array[(int)(length/2)] : (array[((int)(length/2)-1)] + array[(int)(length/2)]) / 2.0;
                
                *result = median;
            }
            else{
                longjmp(env, 3);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_median(array, length, result); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 3: printf("ERROR! (%s -> %s)\n>>> The previous function \"is_farr_sort\" returns an error.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_farr_variance(double *array, int length, double *result){


    /*
        DESCRIPTION:
        Returns the variance of an input array (double *array).
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE) 
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void calc_variance(double *array, int length, double *result){
    
        double sum = 0.0;    
        double average;
         
        // ###########################################
        // ###########################################
            
        // check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // check for NaN values:
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
                break;
            }
        }

        if (!(is_farr_average(array, length, &average))){
        
            // check for NAN
            if (isnan(average)){
                longjmp(env, 3);
            }
            else{
            
                // calculate variance        
                for (idx=0; idx<length; idx++){
        
                    sum += pow((array[idx] - average),2);
                }
                *result = sum/(double)length;
            }
        }
        else{
            longjmp(env, 4);
        }   
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_variance(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_farr_average\" returned NAN as average value.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 4: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_farr_average\" returned an error.\n\n", __FILE__, __func__); return EXIT_FAILURE;        
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_farr_stddev(double *array, int length, double *result){


    /*
        DESCRIPTION:
        Returns the standard deviation of an input array (double *array).
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
        double *result	...	result value of tis function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE) 
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void calc_standard_deviation(double *array, int length, double *result){
    
    
        double variance; 
    
        // ###########################################
        // ########################################### 
           
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // Check for NaN values:
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }
  
  
        if (!(is_farr_variance(array, length, &variance))){
        
            // Check for NAN in previous function:
            if (isnan(variance)){
                longjmp(env, 3);
            }
            // Check for variance smaller then 0:
            if (variance < 0){
                longjmp(env, 4);
            }
            else{
                *result = sqrt(variance);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_standard_deviation(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_farr_variance\" returned NAN as variance value.\n\n",__FILE__, __func__); return EXIT_FAILURE; 
        case 4: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_farr_variance\" returned a variance smaller then 0.\n\n",__FILE__, __func__); return EXIT_FAILURE;        
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_farr_sort(double *array, int length, char *dir){

    /*
        Sorts the values of an input array (double *array) inplace in ascending or descending direction.
        Plaese make sure to work with a copy of your original data if you dont want any changes at your
        original data. This function changes the order inplace.
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    int asc_compare(const void *val1, const void *val2){
        return *(double*)val1 < *(double*)val2;
    }
    
    int desc_compare(const void *val1, const void *val2){
        return *(double*)val1 > *(double*)val2;   
    }
    
    void sort_array(double *array, int length, char *dir){
    
        
        // ###########################################
        // ########################################### 
           
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // Check for NaN values:
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }
        
        // sort in ascending or descending order.
        if (length > 1){
            if (strcmp(dir, "asc")){
                qsort(array, length, sizeof(double), asc_compare);
            }
            else if (strcmp(dir, "desc")){
                qsort(array, length, sizeof(double), desc_compare);        
            }
            else{
                longjmp(env, 3);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: sort_array(array, length, dir); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR! (%s -> %s)\n>>> It is only allowed to sort an array in:\nascending (asc) or descending (desc) direction.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }

}


// ########################################################################################
// ########################################################################################


int is_farr_reverse(double *array, int length){

    /*
        DESCRIPTION
        Changes the order of the values of an input array (double *array) in reversed order.
        If you dont want any changes on your original data please make sure to work with a
        copy of your data. This function changes the order inplace.
    
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx, kdx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void reverse_array(double *array, int length){
    
        double tmp;
    
        // ###########################################
        // ###########################################
           
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
        
        for (idx=0; idx<length; idx++){
            if (isnan(array[idx])){
                longjmp(env, 2);
            }
        }
        
        // reverse values:
        for (idx=0, kdx=length-1; idx<kdx; idx++, kdx--){
            
            tmp = array[idx];
            array[idx] = array[kdx];
            array[kdx] = tmp;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: reverse_array(array, length); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }

}

// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ########################################################################## int - functions ####################################################################################

int is_darr_sum(int *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the sum over all values of an input array (int *array).
    
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array.
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks if the length of array is greater then 0.
    */
    
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    int calc_sum(int *array, int length, double *result){
    
        double sum = 0.0;    
    
        // ###########################################
        // ###########################################
           
        // ################# CHECKS ##################    
        // check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // return the only one value if length is 1.
        if (length == 1){
        
            *result = (double)array[0];
        }
        else{     
    
            for (idx=0; idx<length; idx++){
            
                sum += (double)array[idx];
            }
            
            *result = sum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: calc_sum(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    } 
    
}


// ########################################################################################
// ########################################################################################


int is_darr_maximum(int *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the maximum value of an input array (int *array).
    
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
        double *result	...	result value of this function
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks if the length of array is greater then 0.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    void calc_maximum(int *array, int length, double *result){
    
    
        int maximum;
    
        // ###########################################
        // ###########################################
          
        // ################# CHECKS ##################    
        // check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        maximum = array[0];

        // return the only one value if length is 1.
        if (length == 1){
            *result = array[0];
        }
        else{     
        
    
            for (idx=0; idx<length; idx++){
    
                if (array[idx] > maximum){
        
                    maximum = array[idx];
                }
            }
            *result = (double)maximum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: calc_maximum(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    } 
    
}

// ########################################################################################
// ########################################################################################


int is_darr_minimum(int *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the minimum value of an input array (int *array).

        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array.
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks if the length of array is greater then 0.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    void calc_minimum(int *array, int length, double *result){
    
    
        int minimum;
        
        // ###########################################
        // ###########################################    
    
        // Check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        minimum = array[0];        
        
        // return the only one value if length is 1.
        if (length == 1){
            *result = array[0];
        }
        else{
            
            for (idx=0; idx<length; idx++){
    
                if (array[idx] < minimum){
        
                    minimum = array[idx];
                }
            }
            *result = (double)minimum;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_minimum(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_average(int *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the average of an input array (int *array).
    
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    void calc_average(int *array, int length, double *result){
    
        double sum;
          
        // ###########################################
        // ###########################################
        
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // return the only one value if length is 1:
        if (length == 1){
            *result = (double)array[0];
        }
        else{
        
            if (!(is_darr_sum(array, length, &sum))){
            
                *result = sum/length;
            }
            else{
                longjmp(env, 2);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_average(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_darr_sum()\" returned an error.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_median(int *array, int length, double *result){

    /*
        DESCRIPTION:
        Returns the median of an input array (int *array).
    
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)        
        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void calc_median(int *array, int length, double *result){
    
        double median;    
 
        // ###########################################
        // ########################################### 
          
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
        // median is equal to the only value if length is 1:
        if (length == 1){
            *result = (double)array[0];
        }
        else{
        
            if (!(is_darr_sort(array, length, "asc"))){
            
                median = (length % 2) ? (double)array[(int)(length/2)] : (array[((int)(length/2)-1)] + array[(int)(length/2)]) / 2.0;            
                *result = median;
            }            
            else{
                longjmp(env, 2);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_median(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> Memory fault!\nPlease check previous function 'is_farr_sort'\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_variance(int *array, int length, double *result){


    /*
        DESCRIPTION:
        Returns the variance of an input array (int *array).
    
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)         
        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void calc_variance(int *array, int length, double *result){

        double average = 0.0;
        double sum = 0.0;
        
        // ###########################################
        // ###########################################
        // check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }        
        // check for successful average calculation:
        if (!(is_darr_average(array, length, &average))){
            
            // calculate variance        
            for (idx=0; idx<length; idx++){
        
                sum += pow((array[idx] - average),2);
        
            }     
            *result = sum/length;       
        }
        else{
            longjmp(env, 2);
        }
        
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_variance(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_darr_average()\" returned an error.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_stddev(int *array, int length, double *result){


    /*
        DESCRIPTION:
        Returns the standard deviation of an input array (int *array).
    
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array.
        double *result	...	result value of this function.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)         
        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void calc_standard_deviation(int *array, int length, double *result){
    
  
        double variance;
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // ###########################################
        // ########################################### 
        
        if (!(is_darr_variance(array, length, &variance))){
        
            // Check for variance smaller then 0:
            if (variance < 0){
                longjmp(env, 3);
            }
            else{
                *result = sqrt(variance);
            }
        }
        else{
            longjmp(env, 2);
        }
        

    }
    // ################################################################

    switch(setjmp(env)){
        case 0: calc_standard_deviation(array, length, result); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_darr_variance()\"returned an error.\n\n",__FILE__, __func__); return EXIT_FAILURE; 
        case 3: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The previous function \"is_darr_variance()\" returned a variance value smaller then 0.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_sort(int *array, int length, char *dir){

    /*
        DESCRIPTION:
        Sorts an input array "int *array" inplace in ascending or descending order.
        If there is a copy needed, please make sure you're working with a copy of your original data.
    
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
    	
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE) 

        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    int asc_compare(const void *val1, const void *val2){
        return *(int*)val1 < *(int*)val2;
    }
    
    int desc_compare(const void *val1, const void *val2){
        return *(int*)val1 > *(int*)val2;   
    }
    
    void sort_array(int *array, int length, char *dir){
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // ###########################################
        // ########################################### 

        // return the one and only value if length is equal to 1.
        if (length > 1){
            if (strcmp(dir, "asc")){
                qsort(array, length, sizeof(int), asc_compare);
            }
            else if (strcmp(dir, "desc")){
                qsort(array, length, sizeof(int), desc_compare);        
            }
            else{
                longjmp(env, 2);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: sort_array(array, length, dir); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> It is only allowed to sort an array in:\nascending (asc) or descending (desc) direction.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }

}


// ########################################################################################
// ########################################################################################


int is_darr_reverse(int *array, int length){

    /*
        DESCRIPTION:
        Changes the order of the values of the input array to reverse direction.
        All these changes happens "inplace". If you dont want any changes at the original array 
        please make sure to work with a copy of the original data.
    
        INPUT:
        int *array	...	pointer to an array of integer values.
        int length	...	length of that array
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)     
        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx, kdx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    void reverse_array(int *array, int length){
    
        int tmp;
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
        // If the length of the array is greater then 1 reverse the direction of the array.
        if (length > 1){
        
            // reverse values:
            for (idx=0, kdx=length-1; idx<kdx; idx++, kdx--){
            
                tmp = array[idx];
                array[idx] = array[kdx];
                array[kdx] = tmp;
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: reverse_array(array, length); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR! (%s -> %s)\n>>> Memory fault!\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr, "Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }

}

