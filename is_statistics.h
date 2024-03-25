
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <math.h>
    #include <string.h>
    #include <setjmp.h>
#endif



// double - functions
double is_farr_sum(double *array, int length);			// sum of all values within the array
double is_farr_maximum(double *array, int length);		// maximum value of a double array
double is_farr_minimum(double *array, int length);		// minimum value of a double array
double is_farr_median(double *array, int length);		// median of a double array
double is_farr_average(double *array, int length);		// average of a double array
double is_farr_variance(double *array, int length);		// variance of a double array
double is_farr_stddev(double *array, int length);		// standarddeviation of a double array
double *is_farr_sort(double *array, int length, char *dir);	// sorts an double array ascending or descending
double *is_farr_reverse(double *array, int length);		// will reverse an array of doubles


// int functions
int is_darr_sum(int *array, int length, int *result);			// sum of all values within the array
int is_darr_maximum(int *array, int length, int *result);		// maximum value of a double array
int is_darr_minimum(int *array, int length, int *result);		// minimum value of a double array
int is_darr_median(int *array, int length, int *result);		// median of a double array
int is_darr_average(int *array, int length, int *result);		// average of a double array
int is_darr_variance(int *array, int length, int *result);		// variance of a double array
int is_darr_stddev(int *array, int length, int *result);		// standarddeviation of a double array
int *is_darr_sort(int *array, int length, char *dir, int *result);	// sorts an double array ascending or descending
int *is_darr_reverse(int *array, int length, int *result);		// will reverse an array if integers

// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ########################################################################## double - functions #################################################################################

double is_farr_sum(double *array, int length){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: The sum over all values of the array.
        Failure: NAN.
        
        CHECKS:
        - checks if the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    double calc_sum(double *array, int length){
    
    
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
        // ###########################################
        // ###########################################
        // return the only one value if length is 1.
        if (length == 1){
            return array[0];
        }
        else{     
        
            double sum = 0;
    
            for (idx=0; idx<length; idx++){
    
                sum += array[idx];
            }
            return sum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: return calc_sum(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return NAN;
        case 2: printf("ERROR! ( %s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return NAN;
        default: printf("Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return NAN;
    } 
    
}


// ########################################################################################
// ########################################################################################


double is_farr_maximum(double *array, int length){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: The maximum value of a array of length "length".
        Failure: NAN.
        
        CHECKS:
        - checks if the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    double calc_maximum(double *array, int length){
    
    
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
        // ###########################################
        // ###########################################
        // return the only one value if length is 1.
        if (length == 1){
            return array[0];
        }
        else{     
        
            double maximum = array[0];
    
            for (idx=0; idx<length; idx++){
    
                if (array[idx] > maximum){
        
                    maximum = array[idx];
                }
            }
            return maximum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: return calc_maximum(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return NAN;
        case 2: printf("ERROR! ( %s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return NAN;
        default: printf("Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return NAN;
    } 
    
}

// ########################################################################################
// ########################################################################################


double is_farr_minimum(double *array, int length){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: The minimum value of a array of length "length".
        Failure: NAN.
        
        CHECKS:
        - checks if the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    double calc_minimum(double *array, int length){
    
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
        // ###########################################
        // ###########################################
        
        // return the only one value if length is 1.
        if (length == 1){
            return array[0];
        }
        else{
            double minimum = array[0];
    
            for (idx=0; idx<length; idx++){
    
                if (array[idx] < minimum){
        
                    minimum = array[idx];
                }
            }
            return minimum;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: return calc_minimum(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return NAN;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return NAN;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return NAN;
    }
}


// ########################################################################################
// ########################################################################################


double is_farr_average(double *array, int length){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: The average value of a array of length "length".
        Failure: NAN
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTION ############################
    double calc_average(double *array, int length){
    
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
        // ###########################################
        // ###########################################
        
        // return the only one value if length is 1:
        if (length == 1){
            return array[0];
        }
        else{        
            double average = 0;
    
            average = is_farr_sum(array, length);
            
            return average /= (double)length;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: return calc_average(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return NAN;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return NAN;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return NAN;
    }
}


// ########################################################################################
// ########################################################################################


double is_farr_median(double *array, int length){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        The median value of a array of length "length".
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    double calc_median(double *array, int length){
    
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
        // ###########################################
        // ###########################################
        double median;
        
        // median is equal to the only one value if length is 1:
        if (length == 1){
            return array[0];
        }
        else{
        
            double *array_tmp = is_farr_sort(array, length, "asc");
            if (array_tmp == NULL){
                longjmp(env, 3);
            }
         
            median = (length % 2) ? array_tmp[(int)(length/2)] : (array_tmp[((int)(length/2)-1)] + array_tmp[(int)(length/2)]) / 2;
        
            free(array_tmp);
        
            return median;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: return calc_median(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return NAN;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return NAN;
        case 3: printf("ERROR! (%s -> %s)\n>>> Memory fault!\nPlease check function 'is_farr_sort'\n\n", __FILE__, __func__); return NAN;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return NAN;
    }
}


// ########################################################################################
// ########################################################################################


double is_farr_variance(double *array, int length){


    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: The variance of a array of length "length".
        Failure: NAN
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    double calc_variance(double *array, int length){
    
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
        // ###########################################
        // ###########################################
        double average = is_farr_average(array, length);
        double sum = 0;
        
        // check for NAN
        if (isnan(average)){
            longjmp(env, 3);
        }
        
        // calculate variance        
        for (idx=0; idx<length; idx++){
        
            sum += pow((array[idx] - average),2);
        
        }
        return sum/(double)length;
        
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: return calc_variance(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return NAN;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n", __FILE__, __func__); return NAN;
        case 3: printf("ERROR! (%s -> %s)\n>>> The previous function returned NAN.\n\n", __FILE__, __func__); return NAN;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return NAN;
    }
}


// ########################################################################################
// ########################################################################################


double is_farr_stddev(double *array, int length){


    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: The standard deviation of a array of length "length".
        Failure: NAN
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    double calc_standard_deviation(double *array, int length){
    
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
        // ###########################################
        // ###########################################   
        double variance = is_farr_variance(array, length);
        
        // Check for NAN in previous function:
        if (isnan(variance)){
            longjmp(env, 3);
        }
        
        // Check for variance smaller then 0:
        if (variance < 0){
            longjmp(env, 4);
        }
        else{
            return sqrt(variance);
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: return calc_standard_deviation(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return NAN;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return NAN;
        case 3: printf("ERROR! (%s -> %s)\n>>> The previous function returned NAN.\n\n",__FILE__, __func__); return NAN; 
        case 4: printf("ERROR! (%s -> %s)\n>>> The previous function returned a variance smaller then 0.\n\n",__FILE__, __func__); return NAN;        
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return NAN;
    }
}


// ########################################################################################
// ########################################################################################


double *is_farr_sort(double *array, int length, char *dir){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: pointer of ascending or descending sorted array of length "length".
        Failure: NULL-pointer
        
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
    
    double *sort_array(double *array, int length, char *dir){
    
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
        // ###########################################
        // ###########################################       
        double *sorted_array;
    
    	sorted_array = (double*) malloc(length * sizeof(double));
    	if (sorted_array == NULL){
    	    longjmp(env, 3);
    	}
        
        // copy values:
        for (idx=0; idx<length; idx++){
            sorted_array[idx] = array[idx];
        }
        // return the one and only value if length equal to 1.
        if (length == 1){
            return sorted_array;
        }
        else if (strcmp(dir, "asc")){
            qsort(sorted_array, length, sizeof(double), asc_compare);
        }
        else if (strcmp(dir, "desc")){
            qsort(sorted_array, length, sizeof(double), desc_compare);        
        }
        else{
            longjmp(env, 3);
        }
        return sorted_array;
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: return sort_array(array, length, dir);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return NULL;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return NULL;
        case 3: printf("ERROR! (%s -> %s)\n>>> Memory fault!\n\n",__FILE__, __func__); return NULL;
        case 4: printf("ERROR! (%s -> %s)\n>>> It is only allowed to sort an array in:\nascending (asc) or descending (desc) direction.\n\n",__FILE__, __func__); return NULL;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return NULL;
    }

}


// ########################################################################################
// ########################################################################################


double *is_farr_reverse(double *array, int length){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        Success: pointer of the reversed input array.
        Failure: NULL-pointer
        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx, kdx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    double *reverse_array(double *array, int length){
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
        // ###########################################
        // ###########################################       
        double *reversed_array;
    
    	reversed_array = (double*) malloc(length * sizeof(double));
    	if (reversed_array == NULL){
    	    longjmp(env, 2);
    	}
        
        // reverse values:
        for (idx=0, kdx=length-1; idx<length; idx++, kdx--){
        
            reversed_array[idx] = array[kdx];
        }
        
        return reversed_array;
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: return reverse_array(array, length);
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return NULL;
        case 2: printf("ERROR! (%s -> %s)\n>>> Memory fault!\n\n",__FILE__, __func__); return NULL;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return NULL;
    }

}

// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ########################################################################## int - functions ####################################################################################

int is_darr_sum(int *array, int length, int *result){

    /*
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array.
        int *result	...	result value of this function.
    
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
    int calc_sum(int *array, int length){
    
    
        // ################# CHECKS ##################    
        // check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // ###########################################
        // ###########################################
        // return the only one value if length is 1.
        if (length == 1){
            
            return array[0];
        }
        else{     
        
            int sum = 0;
    
            for (idx=0; idx<length; idx++){
    
                sum += array[idx];
            }
            
            return sum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: *result = calc_sum(array, length); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: printf("Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    } 
    
}


// ########################################################################################
// ########################################################################################


int is_darr_maximum(int *array, int length, int *result){

    /*
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
        int *result	...	result value of this function
    
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
    int calc_maximum(int *array, int length){
    
    
        // ################# CHECKS ##################    
        // check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // ###########################################
        // ###########################################
        // return the only one value if length is 1.
        if (length == 1){
            return array[0];
        }
        else{     
        
            int maximum = array[0];
    
            for (idx=0; idx<length; idx++){
    
                if (array[idx] > maximum){
        
                    maximum = array[idx];
                }
            }
            return maximum;
        }
    }
    // ################################################################
 
    switch(setjmp(env)){
        case 0: *result = calc_maximum(array, length); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: printf("Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    } 
    
}

// ########################################################################################
// ########################################################################################


int is_darr_minimum(int *array, int length, int *result){

    /*
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array.
        int *result	...	result value of this function.
    
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
    int calc_minimum(int *array, int length){
    
        // Check for length
        if (length <= 0){
            longjmp(env, 1);
        }
        
        // ###########################################
        // ###########################################
        
        // return the only one value if length is 1.
        if (length == 1){
            return array[0];
        }
        else{
            int minimum = array[0];
    
            for (idx=0; idx<length; idx++){
    
                if (array[idx] < minimum){
        
                    minimum = array[idx];
                }
            }
            return minimum;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: *result = calc_minimum(array, length); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_average(int *array, int length, int *result){

    /*
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
    int calc_average(int *array, int length){
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
          
        // ###########################################
        // ###########################################
        
        // return the only one value if length is 1:
        if (length == 1){
            return array[0];
        }
        else{        
            int average;
    
            if (!(is_darr_sum(array, length, &average))){
            
                return average /= (int)length;
            }
            else{
                longjmp(env, 2);
            }
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: *result = calc_average(array, length); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: printf("ERROR! (%s -> %s)\n>>> The previous function \"is_darr_sum()\" returned an error.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_median(int *array, int length, int *result){

    /*
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
        int *result	...	result value of this function.
    
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
    int calc_median(int *array, int length){
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // ###########################################
        // ###########################################
        int median;
        
        // median is equal to the only one value if length is 1:
        if (length == 1){
            return array[0];
        }
        else{
        
            int *array_tmp;
            
            if (!(is_darr_sort(array, length, "asc", array_tmp))){
                longjmp(env, 3);
            }
         
            median = (length % 2) ? array_tmp[(int)(length/2)] : (array_tmp[((int)(length/2)-1)] + array_tmp[(int)(length/2)]) / 2;
        
            free(array_tmp);
        
            return median;
        }
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: *result = calc_median(array, length); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 3: printf("ERROR! (%s -> %s)\n>>> Memory fault!\nPlease check previous function 'is_farr_sort'\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_variance(int *array, int length, int *result){


    /*
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
        int *result	...	result value of this function.
    
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
    int calc_variance(int *array, int length){
    
        // check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }

        // ###########################################
        // ###########################################
        int average = 0;
        int sum = 0;
 
        
        if (!(is_darr_average(array, length, &average))){
            
            // calculate variance        
            for (idx=0; idx<length; idx++){
        
                sum += pow((array[idx] - average),2);
        
            }     
            return sum/(int)length;       
        }
        else{
            longjmp(env, 2);
        }
        
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: *result = calc_variance(array, length); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: printf("ERROR! (%s -> %s)\n>>> The previous function \"is_darr_average()\" returned an error.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 3: printf("ERROR! (%s -> %s)\n>>> The previous function returned NAN.\n\n", __FILE__, __func__); return EXIT_FAILURE;        
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int is_darr_stddev(int *array, int length, int *result){


    /*
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array.
        int *result	...	result value of this function.
    
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
    int calc_standard_deviation(int *array, int length){
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // ###########################################
        // ###########################################   
        int variance;
        
        if (!(is_darr_variance(array, length, &variance))){
        
            // Check for variance smaller then 0:
            if (variance < 0){
                longjmp(env, 4);
            }
            else{
                return sqrt(variance);
            }
        }
        else{
            longjmp(env, 5);
        }
        

    }
    // ################################################################

    switch(setjmp(env)){
        case 0: *result = calc_standard_deviation(array, length); return EXIT_SUCCESS;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 2: printf("ERROR! (%s -> %s)\n>>> The array contains 'NaN' values.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 3: printf("ERROR! (%s -> %s)\n>>> The previous function returned NAN.\n\n",__FILE__, __func__); return EXIT_FAILURE; 
        case 4: printf("ERROR! (%s -> %s)\n>>> The previous function \"is_darr_variance()\" returned a variance value smaller then 0.\n\n",__FILE__, __func__); return EXIT_FAILURE;
        case 5: printf("ERROR! (%s -> %s)\n>>> The previous function \"is_darr_variance()\" returned an error.\n\n",__FILE__, __func__); return EXIT_FAILURE;        
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return EXIT_FAILURE;
    }
}


// ########################################################################################
// ########################################################################################


int *is_darr_sort(int *array, int length, char *dir, int *result){

    /*
        INPUT:
        int *array	...	pointer to an array of int values.
        int length	...	length of that array
    	int *result	...	result array of this function.
    	
        OUTPUT:
        Outputs an error code:
        success:	...	pointer of result array.
        failure:	...	NULL 

        
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
    
    int *sort_array(int *array, int length, char *dir){
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
    
        // ###########################################
        // ###########################################       
        int *sorted_array;
    
    	sorted_array = (int*) malloc(length * sizeof(int));
    	if (sorted_array == NULL){
    	    longjmp(env, 2);
    	}
        
        // copy values:
        for (idx=0; idx<length; idx++){
            sorted_array[idx] = array[idx];
        }
        // return the one and only value if length is equal to 1.
        if (length == 1){
            return sorted_array;
        }
        else if (strcmp(dir, "asc")){
            qsort(sorted_array, length, sizeof(int), asc_compare);
        }
        else if (strcmp(dir, "desc")){
            qsort(sorted_array, length, sizeof(int), desc_compare);        
        }
        else{
            longjmp(env, 3);
        }
        return sorted_array;
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: result = sort_array(array, length, dir); return result;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return NULL;
        case 2: printf("ERROR! (%s -> %s)\n>>> Memory fault!\n\n",__FILE__, __func__); return NULL;
        case 3: printf("ERROR! (%s -> %s)\n>>> It is only allowed to sort an array in:\nascending (asc) or descending (desc) direction.\n\n",__FILE__, __func__); return NULL;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return NULL;
    }

}


// ########################################################################################
// ########################################################################################


int *is_darr_reverse(int *array, int length, int *result){

    /*
        INPUT:
        double *array	...	pointer to an array of integer values.
        int length	...	length of that array
        int *result	...	result array of this function
    
        OUTPUT:
        Outputs an error code:
        success:	...	pointer of result array.
        failure:	...	NULL        
        
        CHECKS:
        - checks the length of array is greater then 0.
    */
    
    int idx, kdx;
    jmp_buf env;
    
    // ######################### FUNCTIONS ###########################
    int *reverse_array(int *array, int length){
    
        // Check for length greater then 0:
        if (length <= 0){
            longjmp(env, 1);
        }
        // ###########################################
        // ###########################################       
        int *reversed_array;
    
    	reversed_array = (int*) malloc(length * sizeof(int));
    	if (reversed_array == NULL){
    	    longjmp(env, 2);
    	}
        
        // reverse values:
        for (idx=0, kdx=length-1; idx<length; idx++, kdx--){
        
            reversed_array[idx] = array[kdx];
        }
        
        return reversed_array;
    }
    // ################################################################

    switch(setjmp(env)){
        case 0: result = reverse_array(array, length); return result;
        case 1: printf("ERROR! (%s -> %s)\n>>> The length of an array cannot be smaller/equal to zero.\n\n",__FILE__, __func__); return NULL;
        case 2: printf("ERROR! (%s -> %s)\n>>> Memory fault!\n\n",__FILE__, __func__); return NULL;
        default: printf("Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n",__FILE__, __func__); return NULL;
    }

}

