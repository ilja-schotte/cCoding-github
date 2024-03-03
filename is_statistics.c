
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <stdbool.h>
    #include <math.h>
    #include <string.h>
#endif



double is_farr_maximum(double *array, int length);
double is_farr_minimum(double *array, int length);
double is_farr_median(double *array, int length);
double is_farr_average(double *array, int length);
double is_farr_variance(double *array, int length);
double is_farr_stddev(double *array, int length);
double *is_farr_sort(double *array, int length, char *dir);




int main(){

    double *arr;
    double maximum;
    double minimum;
    double average;
    double median;
    double variance;
    double stddev;
    double *sorted_array;
    int length = 10;
    int idx;
    
    
    
    srand((unsigned)time(NULL));
    
    
    arr = (double*) malloc(length * sizeof(double));
    if (arr == NULL){
        printf("Speicherfehler!\n");
    }
    
    for (idx=0; idx<length; idx++){
    
        arr[idx] = ((rand()%100)+1)/10.0;
        printf("%6.1f", arr[idx]);
    }
    printf("\n");
    maximum = is_farr_maximum(arr, length);
    minimum = is_farr_minimum(arr, length);
    average = is_farr_average(arr, length);
    median = is_farr_median(arr, length);
    variance = is_farr_variance(arr, length);
    stddev = is_farr_stddev(arr, length);
    sorted_array = is_farr_sort(arr, length, "asc");
    
    printf("Maximum: %.3f\n", maximum);
    printf("Minimum: %.3f\n", minimum);
    printf("Average: %.3f\n", average);
    printf("Median: %.3f\n", median); 
    printf("Variance: %.3f\n", variance);   
    printf("Stddev: %.3f\n", stddev);
    
    for (idx=0; idx<length; idx++){
        printf("%6.3f",sorted_array[idx]);
    } 
    printf("\n");
    
    
    free(sorted_array);
    free(arr);

    return 0; 
}


// ########################################################################################
// ########################################################################################


double is_farr_maximum(double *array, int length){

    /*
        INPUT:
        double *array	...	pointer to an array of double values.
        int length	...	length of that array
    
        OUTPUT:
        The maximum value of a array of length "length".
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    int err_code = 0;
    
    // ######################### FUNCTION ############################
    double calc_maximum(double *array, int length){
    
        double maximum = array[0];
    
        for (idx=0; idx<length; idx++){
    
            if (array[idx] > maximum){
        
                maximum = array[idx];
            }
        }
        return maximum;
    }
    // ################################################################

    // Check for length greater then 0:
    if (length <= 0){
        err_code = 1;
    }
    
    // Check for NaN values:
    for (idx=0; idx<length; idx++){
        if (isnan(array[idx])){
            err_code = 2;
            break;
        }
    }
    
    
    switch(err_code){
        case 0: return calc_maximum(array, length);
        case 1: printf("ERROR! The length of array cannot be smaller/equal then zero.\n"); return false;
        case 2: printf("ERROR! The array contains 'NaN' values.\n"); return false;
        default: printf("Something unexpected has happend.\nPlease check function 'is_farr_maximum'.\n"); return false;
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
        The minimum value of a array of length "length".
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    int err_code = 0;
    
    // ######################### FUNCTION ############################
    double calc_minimum(double *array, int length){
    
        double minimum = array[0];
    
        for (idx=0; idx<length; idx++){
    
            if (array[idx] < minimum){
        
                minimum = array[idx];
            }
        }
        return minimum;
    }
    // ################################################################

    // Check for length greater then 0:
    if (length <= 0){
        err_code = 1;
    }
    
    // Check for NaN values:
    for (idx=0; idx<length; idx++){
        if (isnan(array[idx])){
            err_code = 2;
            break;
        }
    }
    
    
    switch(err_code){
        case 0: return calc_minimum(array, length);
        case 1: printf("ERROR! The length of array cannot be smaller/equal then zero.\n"); return false;
        case 2: printf("ERROR! The array contains 'NaN' values.\n"); return false;
        default: printf("Something unexpected has happend.\nPlease check function 'is_farr_minimum'.\n"); return false;
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
        The average value of a array of length "length".
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    int err_code = 0;
    
    // ######################### FUNCTION ############################
    double calc_average(double *array, int length){
    
        double average = 0;
    
        for (idx=0; idx<length; idx++){
    
            average += array[idx];
        }
        return average /= (double)length;
    }
    // ################################################################

    // Check for length greater then 0:
    if (length <= 0){
        err_code = 1;
    }
    
    // Check for NaN values:
    for (idx=0; idx<length; idx++){
        if (isnan(array[idx])){
            err_code = 2;
            break;
        }
    }
    
    
    switch(err_code){
        case 0: return calc_average(array, length);
        case 1: printf("ERROR! The length of array cannot be smaller/equal then zero.\n"); return false;
        case 2: printf("ERROR! The array contains 'NaN' values.\n"); return false;
        default: printf("Something unexpected has happend.\nPlease check function 'is_farr_average'.\n"); return false;
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
    int err_code = 0;
    
    // ######################### FUNCTIONS ###########################
    double calc_median(double *array, int length){
    
        double median;
        double *array_tmp = is_farr_sort(array, length, "asc");
                
        median = (length % 2) ? array_tmp[(int)(length/2)] : (array_tmp[((int)(length/2)-1)] + array_tmp[(int)(length/2)]) / 2;
        
        free(array_tmp);
        
        return median;
    }

    
    // ################################################################

    // Check for length greater then 0:
    if (length <= 0){
        err_code = 1;
    }
    
    // Check for NaN values:
    for (idx=0; idx<length; idx++){
        if (isnan(array[idx])){
            err_code = 2;
            break;
        }
    }
    
    
    switch(err_code){
        case 0: return calc_median(array, length);
        case 1: printf("ERROR! The length of array cannot be smaller/equal then zero.\n"); return false;
        case 2: printf("ERROR! The array contains 'NaN' values.\n"); return false;
        default: printf("Something unexpected has happend.\nPlease check function 'is_farr_median'.\n"); return false;
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
        The variance of a array of length "length".
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    int err_code = 0;
    
    // ######################### FUNCTIONS ###########################
    double calc_variance(double *array, int length){
    
        double average = is_farr_average(array, length);
        double sum = 0;
        
        for (idx=0; idx<length; idx++){
        
            sum += pow((array[idx] - average),2);
        
        }
        return sum/(double)length;
        
    }

    
    // ################################################################

    // Check for length greater then 0:
    if (length <= 0){
        err_code = 1;
    }
    
    // Check for NaN values:
    for (idx=0; idx<length; idx++){
        if (isnan(array[idx])){
            err_code = 2;
            break;
        }
    }
    
    
    switch(err_code){
        case 0: return calc_variance(array, length);
        case 1: printf("ERROR! The length of array cannot be smaller/equal then zero.\n"); return false;
        case 2: printf("ERROR! The array contains 'NaN' values.\n"); return false;
        default: printf("Something unexpected has happend.\nPlease check function 'is_farr_variance'.\n"); return false;
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
        The standard deviation of a array of length "length".
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    int err_code = 0;
    
    // ######################### FUNCTIONS ###########################
    double calc_standard_deviation(double *array, int length){
    
        double variance = is_farr_variance(array, length);

        return sqrt(variance);
        
    }

    
    // ################################################################

    // Check for length greater then 0:
    if (length <= 0){
        err_code = 1;
    }
    
    // Check for NaN values:
    for (idx=0; idx<length; idx++){
        if (isnan(array[idx])){
            err_code = 2;
            break;
        }
    }
    
    
    switch(err_code){
        case 0: return calc_standard_deviation(array, length);
        case 1: printf("ERROR! The length of array cannot be smaller/equal then zero.\n"); return false;
        case 2: printf("ERROR! The array contains 'NaN' values.\n"); return false;
        default: printf("Something unexpected has happend.\nPlease check function 'is_farr_variance'.\n"); return false;
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
        pointer of ascending or descending sorted array of length "length".
        
        CHECKS:
        - checks the length of array is greater then 0.
        - checks for NaN values within the array.
    */
    
    int idx;
    int err_code = 0;
    
    // ######################### FUNCTIONS ###########################
    int asc_compare(const void *val1, const void *val2){
        return *(double*)val1 < *(double*)val2;
    }
    
    int desc_compare(const void *val1, const void *val2){
        return *(double*)val1 > *(double*)val2;   
    }
    
    double *sort_array(double *array, int length, char *dir){
    
        double *sorted_array;
    
    	sorted_array = (double*) malloc(length * sizeof(double));
    	if (sorted_array == NULL){
    	    printf("Speicherfehler!\n");
    	    return false;
    	}
        
        for (idx=0; idx<length; idx++){
            sorted_array[idx] = array[idx];
        }
        
        if (strcmp(dir, "asc")){
            qsort(sorted_array, length, sizeof(double), asc_compare);
        }
        else if (strcmp(dir, "desc")){
            qsort(sorted_array, length, sizeof(double), desc_compare);        
        }
        else{
            printf("ERROR! It is only allowed to sort an array in:\nascending (asc) or descending (desc) direction.\n");
            return false;
        }
        return sorted_array;
    }

    
    // ################################################################

    // Check for length greater then 0:
    if (length <= 0){
        err_code = 1;
    }
    
    // Check for NaN values:
    for (idx=0; idx<length; idx++){
        if (isnan(array[idx])){
            err_code = 2;
            break;
        }
    }
    
    
    switch(err_code){
        case 0: return sort_array(array, length, dir);
        case 1: printf("ERROR! The length of array cannot be smaller/equal then zero.\n"); return false;
        case 2: printf("ERROR! The array contains 'NaN' values.\n"); return false;
        default: printf("Something unexpected has happend.\nPlease check function 'is_farr_variance'.\n"); return false;
    }

}






