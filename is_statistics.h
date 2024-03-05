
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <math.h>
    #include <string.h>
    #include <setjmp.h>
#endif


double is_farr_sum(double *array, int length);			// sum of all values within the array
double is_farr_maximum(double *array, int length);		// maximum value of a double array
double is_farr_minimum(double *array, int length);		// minimum value of a double array
double is_farr_median(double *array, int length);		// median of a double array
double is_farr_average(double *array, int length);		// average of a double array
double is_farr_variance(double *array, int length);		// variance of a double array
double is_farr_stddev(double *array, int length);		// standarddeviation of a double array
double *is_farr_sort(double *array, int length, char *dir);	// sorts an double array ascending or descending


// ########################################################################################
// ########################################################################################


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






