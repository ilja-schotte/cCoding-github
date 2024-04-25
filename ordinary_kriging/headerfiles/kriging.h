
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <math.h>
    #include <stdbool.h>
    #include "regression.h"
    #include <setjmp.h>
    #include <errno.h>
#endif

#define NO_VALUE -1.0
#define M_PI 3.14159265358979323846
#define EPS 1.0E-3
#define RADIUS_EARTH 6365.265


// Deklaration: Funktion
// ###########################################################################
// ###########################################################################

struct usr_data_point **create_2D_raster(int rows, int cols);

int set_config(struct usr_map *Map, int argc, char **argv);							// reads the arguments this function is called with 
														// ... and performs calculation regarding to the resolution


int fill_raster_with_default_data(struct usr_map *Map);
int input_csv_data(struct usr_map *Map, char *input_datafile);
int fill_raster_with_input_data(struct usr_map *Map);
int create_variogram(struct usr_map *Map);
int interpolate_raster(struct usr_map *Map);
int correct_negative_weights(double *weights_vector, double *cov_vector, int length);
int outputRasterCSV(struct usr_data_point **raster, char *output_dir, char *filename, int rows, int cols, bool show_output);
int get_output_information(struct usr_map *Map);
int get_variogram_model(struct usr_map *Map);
int *create_vector(int length);
int get_dvector_max(int *values, int length);
int get_dvector_min(int *values, int length);
int show_map_info(struct usr_map *Map);
int show_input_data(struct usr_map *Map);
int show_variogram_data(struct usr_map *Map);
int show_matrix(char *name, double **matrix, int rows, int cols, bool show_output);
int check_matrix(double **matrix, int rows, int cols, bool show_output);
int create_covariance_matrix(struct usr_map *Map);
int create_inverted_covariance_matrix(struct usr_map *Map);
int create_distance_matrix(struct usr_map *Map);
int multiplyMatrixVector(double **matrix, double *vector_in, double *weights_vector, int rows, int cols);
int find_model_adjust_index(struct usr_map *Map, double *variogram_variances, int length);
      
double calc_distance(double latA, double lonA, double latB, double lonB);
double calc_covariance(double distance, double sill, double nugget, double range);
double calc_RSME(double *values1, double *values2, int length);
double get_fvector_max(double *values, int length);
double get_fvector_min(double *values, int length);
double **create_fmatrix(int rows, int cols);
double *create_fvector(int length);

void free_raster(struct usr_map *Map);
void free_vector(struct usr_map *Map);


// ##################################################################################################
// ##################################### Definition: Funktionen #####################################

int create_maps_raster(struct usr_data_point ***raster, int rows, int cols){

    /*
    
        DESCRIPTION:
        Initialize an raster of dimensions "Map->rows" x "Map->cols" of type struct usr_data_point.
        
        INPUT:
        struct usr_data_point ***rasters	...	pointer to the maps 2d raster
        int rows				...	number of rows
        int cols				...	number of cols
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE       
     
    */
    
    int idx, jdx;
    int excno;
    jmp_buf env;    
    
    if ((excno = setjmp(env)) == 0){

        struct usr_data_point **zgr;


        zgr = (struct usr_data_point **) malloc(rows * sizeof(struct usr_data_point *));
        if (zgr == NULL){
            longjmp(env, 1);
        }
        else{
            for (idx=0; idx<rows; idx++){
        
                zgr[idx] = (struct usr_data_point *) malloc(cols * sizeof(struct usr_data_point));        
                if (zgr[idx] == NULL){
                    
                    // clean the already allocated memory
                    for (jdx=0; jdx<idx; jdx++){
                        free(zgr[jdx]);
                    }
                    free(zgr);
                    longjmp(env, 1);
                }
            }
            // assign the address of the matrix to the raster address.
            *raster = zgr;
        }
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n Failure during the attempt to allocate memory\n--> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
        }
    } 
}

// ##################################################################################################
// ##################################################################################################


int set_config(struct usr_map *Map, int argc, char **argv){

    /*
        DESCRIPTION:
        Reads the given arguments and performs some calculations to complete the intern configuration.
    
        INPUT:
        struct usr_map *zgr	...	pointer to the "map" object of datatype "struct usr_map".
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
       
    */

    int idx;
    int excno;
    jmp_buf env;
    double cols;
    
    
    
    if ((excno = setjmp(env)) == 0){

        // read the arguments:
        for (idx=0; idx<argc; idx++){
        
            // correct negative weights?
            if (!strcmp(argv[idx],"-c")){
                Map->weights_correction = true;
            }
            
            // show output during calculations?
            if (!strcmp(argv[idx],"-o")){
                Map->show_output = true;
            }
            
            if ((!strcmp(argv[idx],"-co")) || (!strcmp(argv[idx],"-oc"))){
                Map->show_output = true;
                Map->weights_correction = true;
            }                     
        }
    
        // check rows to be greater then 0;
        if (Map->rows <= 0){
            longjmp(env, 1);
        }
        // check maxLon & minLon to be greater then 0;        
        if ((Map->maxLon <= 0) || (Map->minLon <= 0)){
            longjmp(env, 2);
        }
        // check maxLat & minLat to be greater then 0;        
        if ((Map->maxLat <= 0) || (Map->minLat <= 0)){
            longjmp(env, 3);
        }        
        
        // calculate the number of columns:
        cols = ceil(Map->rows * (double)(Map->maxLon - Map->minLon) / (double)(Map->maxLat - Map->minLat));
        
        if (!(isnan(cols)) || (!(isinf(cols)))){
            Map->cols = (int)cols;
        }
        else{
            longjmp(env, 4);
        }
        
        // Calculate the resolution of the matrix in decimal degree:
        Map->lonRes = (Map->maxLon - Map->minLon) / (Map->cols-1);
        Map->latRes= (Map->maxLat - Map->minLat) / (Map->rows-1);
    
        // Calculate the resolution of the matrix in kilometer:
        Map->latMetRes = calc_distance(0.0, 0.0, Map->latRes, 0);		// Distance to the origin of the coordinates (y)
        Map->lonMetRes = calc_distance(0.0, 0.0, 0.0, Map->lonRes);		// Distance to the origin of the coordinates (x)
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n The number of rows must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n The number of maxLon and minLon must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 3: fprintf(stderr, "ERROR: %s --> %d:\n The number of maxLat and minLat must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 4: fprintf(stderr, "ERROR: %s --> %d:\n The calculated number of columns of the output raster is \"NAN\" or \"INF\"!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;        
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int fill_raster_with_default_data(struct usr_map *Map){

    /*
        DESCRIPTION:
        Fills any point of the raster with default data.
    
        INPUT:
        struct usr_map *Map	...	pointer to map object
        int rows		...	number of rows
        int cols		...	number of cols
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */


    int idx, jdx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){

        // check for rows greater then 0:
        if (Map->rows <= 0){
            longjmp(env, 1);
        }
        
        // check for columns greater then 0:
        if (Map->cols <= 0){
            longjmp(env, 2);
        }
        
        // check for lonRes greater then 0: 
        if (Map->lonRes <= 0){
            longjmp(env, 3);
        }
        
        // check for latRes greater then 0: 
        if (Map->latRes <= 0){
            longjmp(env, 4);
        }        
        
        // fill any point of the raster with default data.
        for (idx=0; idx<Map->rows; idx++){
    
            for (jdx=0; jdx<Map->cols; jdx++){
        
                // set the indices:
                Map->raster[idx][jdx].row_idx = idx;
                Map->raster[idx][jdx].col_idx = jdx;
            
                // set the coordinates:
                Map->raster[idx][jdx].lon = (Map->minLon) + jdx*(Map->lonRes);
                Map->raster[idx][jdx].lat = (Map->maxLat) - idx*(Map->latRes);            
            
                // set the default values:
                Map->raster[idx][jdx].value = NO_VALUE;
            }
        }
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n The number of rows must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n The number of columns must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 3: fprintf(stderr, "ERROR: %s --> %d:\n The resolution of longitude must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 4: fprintf(stderr, "ERROR: %s --> %d:\n The resolution of latitude must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;                            
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
        } 
    }   
}


// ##################################################################################################
// ##################################################################################################


int input_csv_data(struct usr_map *Map, char *input_datafile){

    /*
        DESCRIPTION:
        Reads the input dataset provided by the csv-file in the input directory.
    
        INPUT:
        struct usr_map *Map	...	pointer to map object
        char *input_datafile	...	filename of the input datafile in the input directory
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
      
    jmp_buf env;

    FILE *fp;
    int rows = 0;
    int idx, jdx;
    int excno;
    
    double sum = 0; 
    double *values_array;
    
    char inputRow[255];
    char inputDecimal[20];
    char path[100];
    
    
    
    if ((excno = setjmp(env)) == 0){
        
        // open input file:
        fp = fopen(strcat(strcpy(path,Map->config.input_dir), input_datafile), "r");
        if (fp == NULL){
            longjmp(env, 1);
        }
        else{
            // determine the length of the dataset (number of values)
            while (fgets(inputRow, 255, fp) != NULL){
                rows++;
            }
            
            // check for rows ist greater then 0:
            if (rows <= 0){
                longjmp(env, 2);
            }
            else{
                // Länge des Input_Datensatzes übergeben:
                Map->input_data.length = rows-1;
            }
        
            /* 
            allocate memory of type usr_data_point.
            contains:
               - stationname
               - geogr. latitude
               - geogr. longitude
               - value
            */
            Map->input_data.data = (struct usr_data_point*) malloc(rows * sizeof(struct usr_data_point));
            if (Map->input_data.data == NULL){
                longjmp(env, 3);
            }
            
            idx = 0;
            // set the pointer to the start of the file:
            fseek(fp, 0, SEEK_SET);
            
            // read the data row by row:
            while (fgets(inputRow, 255, fp) != NULL){
            
                if (idx > 0){
                    // Einlesen des Stationsnamens:
                    strcpy(Map->input_data.data[idx-1].name, strtok(inputRow, ";"));
            
                    // Einlesen der geogr. Breite:
                    strcpy(inputDecimal, strtok(NULL, ";"));
                    for (jdx=0; jdx<(int)strlen(inputDecimal); jdx++){
            
                        if (inputDecimal[jdx] == ','){
                
                            inputDecimal[jdx] = '.';
                            break;
                        }
                    }
                    Map->input_data.data[idx-1].lat = atof(inputDecimal);
            
                    // Einlesen der geogr. Länge:            
                    strcpy(inputDecimal, strtok(NULL, ";"));
                    for (jdx=0; jdx<(int)strlen(inputDecimal); jdx++){
            
                        if (inputDecimal[jdx] == ','){
                
                            inputDecimal[jdx] = '.';
                            break;
                        }
                    }
                    Map->input_data.data[idx-1].lon = atof(inputDecimal);  
            
                    // Einlesen des Messwerts:         
                    strcpy(inputDecimal, strtok(NULL, ";"));
                    for (jdx=0; jdx<(int)strlen(inputDecimal); jdx++){
            
                        if (inputDecimal[jdx] == ','){
                
                            inputDecimal[jdx] = '.';
                            break;
                        }
                    }
                    Map->input_data.data[idx-1].value = atof(inputDecimal);             
                    idx++;
                }
                else{
                    idx++;
                    continue;
                }
            }
        
        
            // determine statistical metadata over the input datasets.
            Map->input_data.minimum = Map->input_data.data[0].value;
            Map->input_data.maximum = Map->input_data.data[0].value;
            Map->input_data.average = Map->input_data.data[0].value;
                
            for (idx=0; idx<Map->input_data.length; idx++){
        
                sum += Map->input_data.data[idx].value;
        
                if (Map->input_data.data[idx].value < Map->input_data.minimum){
            
                    Map->input_data.minimum = Map->input_data.data[idx].value;
                }
            
                if (Map->input_data.data[idx].value > Map->input_data.maximum){
            
                    Map->input_data.maximum = Map->input_data.data[idx].value;
                }
            }
        
            Map->input_data.average = (double)(sum / Map->input_data.length);
        
            fclose(fp);
            return EXIT_SUCCESS;
        }
    }
    // ##############################################################################################
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n Failure when opening the csv-file:\n>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n The counted number of datapoints is 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 3: fprintf(stderr, "ERROR: %s --> %d:\n Failure when allocating the memory for the input dataset:\n>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }    
}


// ##################################################################################################
// ##################################################################################################




double **create_fmatrix(int rows, int cols){

    /*
        DESCRIPTION:
        Allocates memory for a matrix of type double with the dimensions of rows x cols.
        
        INPUT:
        int rows	...	number of rows.
        int cols	...	number of columns.
        
        OUTPUT:
        on success	...	pointer of type double to this matrix.
        on failure	...	NULL       
    */

    int idx, jdx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){   
    
    
        double **zgr;
    
        zgr = (double **) calloc(rows, sizeof(double*));
        if (zgr == NULL){
            longjmp(env, 1);
        }  
        for (idx=0; idx<rows; idx++){
        
            zgr[idx] = (double *) calloc(cols, sizeof(double));
            if (zgr[idx] == NULL){
            
                // clean up
                for (jdx=0; jdx<idx; jdx++){
                    free(zgr[jdx]);
                }
                free(zgr);
                
                longjmp(env, 1);
            }
        }
        return zgr;
    }
    // ##############################################################################################

    switch(excno){
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return NULL;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NULL;
    }     
    
}


// ##################################################################################################
// ##################################################################################################


double *create_fvector(int length){

    /*
        DESCRIPTION:
        Allocates memory for a vector of datatype double and length. 
        
        INPUT:
        int length	...	length of the vector you want to allocate.
        
        OUTPUT:
        on success	...	pointer to this vector
        on failure	...	NULL
              
    */

    jmp_buf env;
    int idx;
    int excno;

    
    if ((excno = setjmp(env)) == 0){
    
        double *zgr;
        
        zgr = (double *) calloc(length, sizeof(double));
        if (zgr == NULL){
            longjmp(env, 1);
        }
        else{
            return zgr;
        }   
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n>>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return NULL;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NULL;
        }
    }
}




// ##################################################################################################
// ##################################################################################################


int *create_vector(int length){

    /*
        DESCRIPTION:
        Allocates memory for a vector of datatype int and length. 
        
        INPUT:
        int length	...	length of the vector you want to allocate.
        
        OUTPUT:
        on success	...	pointer to this vector
        on failure	...	NULL
              
    */

    jmp_buf env;
    int idx;
    int excno;

    
    if ((excno = setjmp(env)) == 0){
    
        int *zgr;
        
        zgr = (int *) calloc(length, sizeof(int));
        if (zgr == NULL){
            longjmp(env, 1);
        }
        else{
            return zgr;
        }
        
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n>>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return NULL;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NULL;
        }   
    }
}


// ##################################################################################################
// ##################################################################################################



int fill_raster_with_input_data(struct usr_map *Map){

    /*
        DESCRIPTION:
        writes the input dataset into the raster points of the map raster.
    
        INPUT:
        struct usr_map *Map	...	pointer to map object
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */ 

    
    int idx, jdx, kdx;
    int excno;
    jmp_buf env;
    
    
    if ((excno = setjmp(env)) == 0){ 
    
        int min_row_idx, min_col_idx;
        double mlat, mlon, distance;
        double rlat, rlat_rad, rlon, rlon_rad;
        double min_distance, min_mlat, min_mlon, min_rlat, min_rlon;
        
    
        // output?
        if (Map->show_output){
            printf("Assign points to raster ... ");
            fflush(stdout);
        }
        
        
    
        if (Map->input_data.length <= 0){
            longjmp(env, 1);
        }
    
        for (idx=0; idx<Map->input_data.length; idx++){
    
            // the default distance is the highest distance between 2 points on earth 
            min_distance = 44000.00;
        
            // latitude and longitude of the station:
            mlat = Map->input_data.data[idx].lat;
            mlon = Map->input_data.data[idx].lon;
        
            // find the shortest distance between the station and a raster point
            for (jdx=0; jdx<Map->rows; jdx++){
                for (kdx=0; kdx<Map->cols; kdx++){
            
                    // latitude and longitude of the raster point:
                    rlat = Map->raster[jdx][kdx].lat;
                    rlon = Map->raster[jdx][kdx].lon;
                            
                    // Distance on a sphere with radius at 51°N.
                    distance = calc_distance(mlat, mlon, rlat, rlon);
                
                    if (distance < min_distance){                  
                        min_distance = distance;
                        min_mlat = mlat;
                        min_mlon = mlon;
                        min_rlat = rlat;
                        min_rlon = rlon;
                        min_row_idx = Map->raster[jdx][kdx].row_idx;
                        min_col_idx = Map->raster[jdx][kdx].col_idx;
                    }
                }
            }
        
            // give the raster point the name of the station
            strcpy(Map->raster[min_row_idx][min_col_idx].name, Map->input_data.data[idx].name);
            Map->raster[min_row_idx][min_col_idx].value = Map->input_data.data[idx].value;
        
            // give the input data the corresponding index values of the raster point
            Map->input_data.data[idx].row_idx = min_row_idx;
            Map->input_data.data[idx].col_idx = min_col_idx;
        }
        
        // output:
        if (Map->show_output){
            printf(" ok.\n");
        }
        
        return EXIT_SUCCESS;
    }  
    else{  
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n The length of the input dataset is 0\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }    

}


// ##################################################################################################
// ##################################################################################################


int create_distance_matrix(struct usr_map *Map){

    /*
        DESCRIPTION:
        Determines the distance of all points to each other.
        It creates a distance matrix of dimensions "length of input dataset" x "length of input dataset".
    
        INPUT:
        struct usr_map *Map	...	pointer to map object
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
    
    jmp_buf env;
    int excno;
    int idx, jdx;
    
    
    
    if ((excno = setjmp(env)) == 0){    
    
        // output ?
        if (Map->show_output){
            printf("Calculating distance matrix ... ");
            fflush(stdout);
        }
    
        // Allocate memory for a square distance matrix of "Map.input_data.length" x "Map.input_data.length"
        Map->distance_matrix = create_fmatrix(Map->input_data.length, Map->input_data.length);
        if (Map->distance_matrix == NULL){
            longjmp(env, 1);    
        }
        else{
    
            for (idx=0; idx<Map->input_data.length; idx++){
                for (jdx=0; jdx<Map->input_data.length; jdx++){
            
                    // Is the difference between the longitude and latitude value smaler then EPS, the distance must be 0:
                    if ((fabs(Map->input_data.data[idx].lat - Map->input_data.data[jdx].lat) < EPS) && 
                        (fabs(Map->input_data.data[idx].lon - Map->input_data.data[jdx].lon) < EPS)){
                
                        Map->distance_matrix[idx][jdx] = 0.0;
                    }
                    else{
                        // calculate the distance between these points:                
                        Map->distance_matrix[idx][jdx] = calc_distance(Map->input_data.data[idx].lat, 
                                                                       Map->input_data.data[idx].lon, 
                                                                       Map->input_data.data[jdx].lat, 
                                                                       Map->input_data.data[jdx].lon);
                    }
                }
            }
            
            // output ?
            if (Map->show_output){
                printf("ok!\n");
            }       
        }
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
}

// ##################################################################################################



int create_variogram(struct usr_map *Map){

    /*
        DESCRIPTION:
        1. Defines classes of distance
        -> maximum distance between two points in germany is 900 km.
        -> categorization in distance-classes with a width of 50 km. -> 18 classes .
        
        2. Calculates the average distance of all point of a specific distance interval (class).
        3. Calculates the average variance of these distance classes.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object.
        
        OUTPUT: (errorcode)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
    
    jmp_buf env;
    int idx, jdx, kdx;
    int excno;
    
    if ((excno = setjmp(env)) == 0){
    
        int numValue;
        double semiVarianz;
        double avgDistance;
        double sumPowValue;		// Summe der quadratischen abweichungen pro Abstandsklasse.
        double distance;
        double sumDistance;
        double sumClassVarianz;
        double sumClassDistance;
        double **input_data_poly_reg;
        double *solution;    
        
        
        if (Map->show_output){
            printf("Create variogram ... ");
            fflush(stdout);
        }
    
        // Calculate the number of distance classes with given max. distance and the wanted interval width:
        Map->variogram.numClasses = (int)(Map->variogram.maxDistance / Map->variogram.distInterval);
        if (Map->variogram.numClasses <= 0){
            longjmp(env, 1);
        }

        // Allocate a matrix of type "usr_vario_class" to store all the relevant information to the variogram.
        Map->variogram.classes = (struct usr_vario_class *) malloc(Map->variogram.numClasses * sizeof(struct usr_vario_class));
        if (Map->variogram.classes == NULL){
            longjmp(env, 2);
        }
        
        // Create the attributs of the variogram-classes (lowerLimit / upperLimit):
        for (idx=1; idx<=(Map->variogram.numClasses); idx++){
        
            Map->variogram.classes[idx-1].lowerLimit = (int)((idx-1)*Map->variogram.distInterval);	// lower Limit of the distance class in km.
            Map->variogram.classes[idx-1].upperLimit = (int)((idx)*Map->variogram.distInterval);	// upper limit of the distance class in km.
            Map->variogram.classes[idx-1].num_variance_values = 0;					// set the start value to the number of variance value of a class 
            Map->variogram.classes[idx-1].num_distance_values = 0;					// set the start value to the number of distance values of a class
            Map->variogram.classes[idx-1].variance_avg = 0;						// set 0 as default value for average variance
            Map->variogram.classes[idx-1].distance_avg = 0;						// set 0 as default value for average distance
        
        }
        
        // Calculate now the average semivariance for any distance class over all datapoints:
        // Take a station out of the input dataset.
        for (idx=0; idx<Map->input_data.length; idx++){
                        
            
            // check over the distance classes to what distance classes this station belongs to:
            // Take a distance class:
            for (jdx=0; jdx<Map->variogram.numClasses; jdx++){
            
                // Set the start values to 0:
                sumPowValue = 0;	// sum of the powered differences
                numValue = 0;		// number of the powered differences
                semiVarianz = 0;	// Semivariance for this station of a specific distance class
                sumDistance = 0;	// sum of all distances for this distance class
            
                // Now compare the value of the station with the other ones
                // Take a station out of the input dataset to compare its value with the current one:
                for (kdx=0; kdx<Map->input_data.length; kdx++){
                
                    // Dont compare a station with itself:
                    if (idx != kdx){
                
                        // Calculate the distance between this station with a station we want to compare with
                        distance = calc_distance(Map->input_data.data[idx].lat, 
                                                 Map->input_data.data[idx].lon, 
                                                 Map->input_data.data[kdx].lat,
                                                 Map->input_data.data[kdx].lon);
                        
                        // Is the distance located in the current selected distance class;
                        // ... add this station to the current distance and variance value of the distance class.
                        if ((Map->variogram.classes[jdx].lowerLimit < distance) && (distance <= Map->variogram.classes[jdx].upperLimit)){
                        
                            sumPowValue += pow((Map->input_data.data[idx].value - Map->input_data.data[kdx].value),2);
                            
                            sumDistance += distance;
                            
                            numValue ++;
                            continue;
                            
                        }
                        else{
                            // Is the distance not located in the curretn selected class than take another one.
                            continue;
                        }
                
                    }
                    else{
                        // take another station if it is the same station:
                        continue;
                    }
                }
               
                // If all stations were compared with each other, calculate the semivariance of each distance class and add it to the distance class if it is greater then 0:
                // Check if the sum is greater then 0 to prevent division by 0:
                if (sumPowValue > 0){
                
                    // 2. Calculate the semivariance
                    semiVarianz = (double)(sumPowValue / (2*numValue));
                    
                    // 3. calculate the average distance for this distance class for this station:
                    avgDistance = (double)sumDistance / numValue;
                    
                    if ((semiVarianz > 0) && (avgDistance > 0)){
                
                        // Add the value of semivariance to the coresponding distance class:
                        Map->variogram.classes[jdx].variance_values[Map->variogram.classes[jdx].num_variance_values++] = semiVarianz;
                        
                        // Füge die durchschnittl. Distanz dieser Klasse für diese Station hinzu: 
                        Map->variogram.classes[jdx].distance_values[Map->variogram.classes[jdx].num_distance_values++] = avgDistance;                      
                    }
                    else{
                        continue;
                    }
                }
                // continue with the next class
                continue; 
            }
        }
                    
        // Now calculate the average of the distance and semivariance of each distance class:
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
            sumClassVarianz = 0.0;
            sumClassDistance = 0.0;
        
            // There must be at least one semivariance value
            if ((Map->variogram.classes[idx].num_variance_values > 0) && (Map->variogram.classes[idx].num_variance_values > 0)){
        
                for (jdx=0; jdx<Map->variogram.classes[idx].num_variance_values; jdx++){
            
                    sumClassVarianz += Map->variogram.classes[idx].variance_values[jdx];
                    sumClassDistance += Map->variogram.classes[idx].distance_values[jdx];
                }
                
                Map->variogram.classes[idx].variance_avg = sumClassVarianz / (double)(Map->variogram.classes[idx].num_variance_values);
                Map->variogram.classes[idx].distance_avg = sumClassDistance / (double)(Map->variogram.classes[idx].num_distance_values);
            }
            // else set it to 0  
            else{
            
                Map->variogram.classes[jdx].variance_avg = sumClassVarianz;
                Map->variogram.classes[jdx].distance_avg = sumClassDistance;                
                continue;
            }
        }
        
        if (Map->show_output){
            printf("ok.\n");
        }
        
        return EXIT_SUCCESS;
        
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> There must be at least one variogram class.\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;        
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
}


// ##################################################################################################
// ##################################################################################################



int get_variogram_model(struct usr_map *Map){

    /*
    
        DESCRIPTION:
        Finds on the basis of the semivarince values of the variogram the appropriate model.
        
        1: Find a polynomic function of order 4 that fits best to the semivariance values of the variogram
        2: Find with the help of this function the following values for a possible exponential model:
           - sill
           - range
       
        prepare the following values
           - distance_avg (x-Werte)
           - variance_avg (y-Werte)
        to perform a polynomic regression.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object
        
        OUTPUT: (error code)
        on success:		...	EXIT_SUCCESS
        on failure:		...	EXIT_FAILURE        
        
    */
    
    int idx, jdx;
    int excno;
    jmp_buf env;
    
    
    
    if ((excno = setjmp(env)) == 0){
    
        int cnt_neg_slope;
        int cnt_raising_slope;
        int itemp;
        int *density_variances;  
      
        double sill, range, RSME, interval;	//
        double RSME_min=-1;			// Minimumwert des RSME
        double y_max;				// Maximumwert der Vorhersagewerte der Semivarianzen
        double **input_data_poly_reg;		// Doppelzeiger auf Matrix mit Eingabedaten.
        double *diff_variance_avg_reg;		// abgeleitete Werte der Vorhersagewerte der Semivarianz mittels polyn. Regression
        double *calc_variances;			// berechnete Werte der Semivarainz pro lag mit Hilfe der exponentiellen Funktion
        double *variogram_variances;		// Semivarianzwerte der lags des Variogramms bis zum gesuchten Vergleichsindex.
        double tmp;				// temporary variable, commonly used for checking purposes
		
    
        if (Map->show_output){
            printf("Get variogram model ... ");
            fflush(stdout);
        }
    
        // allocate memory for the input dataset for the polynomic regression
        input_data_poly_reg = create_fmatrix(Map->variogram.numClasses, 2);
        if (input_data_poly_reg == NULL){
            longjmp(env, 1);
        }
    
        // allocate memory for the variance values of the variogram
        variogram_variances = create_fvector(Map->variogram.numClasses);
        if (variogram_variances == NULL){
            longjmp(env, 1);
        }   

        /*
            prepare the following values
                - distance_avg (x-Werte)
                - variance_avg (y-Werte)
            to perform a polynomic regression.
        */
        
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
            // transfer x-values:
            input_data_poly_reg[idx][0] = Map->variogram.classes[idx].distance_avg;
        
            // transfer y-values:
            input_data_poly_reg[idx][1] = Map->variogram.classes[idx].variance_avg; 
        
            // Übertrage y-Werte in Vektor:
            variogram_variances[idx] = Map->variogram.classes[idx].variance_avg; 
        }
    


        // Calculate the polynomic regression of 4th order.
        /* Is successful it returns a pointer to an array of length order +2 with the solutions
           Example:
           Order(n): 4
           
           array[0] to array[n]		...	contains the weights of the polynomic function from b0 to bn
           array[n+1] 			...	contains the coefficient of determination
        */
        Map->variogram.reg_function.solution = polynomial_regression(input_data_poly_reg, 
                                                                     Map->variogram.numClasses, 
                                                                     Map->variogram.reg_function.order);
        if (Map->variogram.reg_function.solution == NULL){
            longjmp(env, 2);
        }
        
        // Calculate on the basis of the regress function the predicted value for the semivariance 
        // ... over the average distances of the semivariances
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
             
            // start value is weight0 (b0):
            if ((isnan(Map->variogram.reg_function.solution[0])) || (isinf(Map->variogram.reg_function.solution[0]))){
                longjmp(env, 4);
            }
            else{
                Map->variogram.classes[idx].variance_avg_reg = Map->variogram.reg_function.solution[0];
            }
            
            // sum over the powers:
            for (jdx=1; jdx<=Map->variogram.reg_function.order; jdx++){
                    
                // check for nan and inf values within the predicted variance data:
                tmp = (Map->variogram.reg_function.solution[jdx] * pow(Map->variogram.classes[idx].distance_avg, jdx));
                if ((isnan(tmp)) || (isinf(tmp))){
                    longjmp(env, 3);
                }
                else{
                    Map->variogram.classes[idx].variance_avg_reg += tmp;
                }
            }
        }
        
        // Now found the part of the polynomic function you want to adjust the exponential model
        find_model_adjust_index(Map, variogram_variances, Map->variogram.numClasses);

        // allocate memory for the calculated variances on basis of the exponential model:
        calc_variances = create_fvector(Map->variogram.model_adjust_index);
        if (calc_variances == NULL){
            longjmp(env, 1);
        }

        // find sill and range for exponential model:
        for (sill=0; sill<100; sill++){
        
            for (range=0; range<=Map->variogram.classes[Map->variogram.model_adjust_index].distance_avg; range+=0.1){
            
                // Calculate the semivariance with the help of the exponential model:
                for (idx=0; idx<Map->variogram.model_adjust_index; idx++){
                
                    calc_variances[idx] = calc_covariance(Map->variogram.classes[idx].distance_avg, 
                                                          sill, 
                                                          Map->variogram.nugget, 
                                                          range);
                }
                
                // get the error by calculating the rsme:
                RSME = calc_RSME(calc_variances, variogram_variances, Map->variogram.model_adjust_index);                                 
                if (isnan(RSME) || (isinf(RSME))){
                    longjmp(env, 5);
                }
                   
                // take over the values of sill and range as long as they are smaller then the prevoius ones.
                if ((RSME < RSME_min) || (RSME_min < 0)){
            
                    RSME_min = RSME;
                    Map->variogram.sill = sill;
                    Map->variogram.range = range;
                    continue;
                }
                else{
                    continue;
                }
            }
        }
        
        
        // check previously if b0-weight is nan or inf:
        if ((isnan(Map->variogram.reg_function.solution[0])) || (isinf(Map->variogram.reg_function.solution[0]))){
            longjmp(env, 4);
        }
        
        // pass the value of the nugget to the model:
        if (Map->variogram.reg_function.solution[0] < 0){
            Map->variogram.nugget = 0.001;
        }
        else{
            Map->variogram.nugget = Map->variogram.reg_function.solution[0];
        }
        
        
        // deallocate the memory
        for (idx=0; idx<Map->variogram.numClasses; idx++){
            free(input_data_poly_reg[idx]);
        }

        free(input_data_poly_reg);    
        free(calc_variances);    
        free(variogram_variances);
        
        if (Map->show_output){
            printf("ok\n");
        }
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;                
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> The solution for the polynomic regression returns NULL!\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 3: fprintf(stderr, "ERROR: %s --> %d:\n >>> The vector of the predicted variances contains \"NAN\" or \"INF\" values!\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 4: fprintf(stderr, "ERROR: %s --> %d:\n >>> The value of the b0 weight is \"NAN\" or \"INF\"!\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 5: fprintf(stderr, "ERROR: %s --> %d:\n >>> The calculated value of the RSME is \"NAN\" or \"INF\"!\n", __FILE__, __LINE__); return EXIT_FAILURE;        
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int find_model_adjust_index(struct usr_map *Map, double *variogram_variances, int length){

    /*
        DESCRIPTION:
        Finds the (lag)-index of the variogram we want to adjust the exponential model with.
        
        BACKGROUND:
        any lag of the varigram is defined by a average distance and a average semivarianz.
        The progress of the semivariance follows a exponential function defined by:
        
        nugget + sill * (1-exp(-|distance|/a)), mit a = range/3;
        
        Now we have to find the index of that lag until we want to adjust the expontential model.
        
        INPUT:
        struct usr_map *Map		...	pointer to the map object
        double *variogram_variances	...	pointer an array with a copy of the variogram variances
        int length			...	lenght of that array.
        
    */

    int idx, jdx;
    int excno;
    jmp_buf env;
    
    
    if ((excno = setjmp(env)) == 0){
    
        int imax=0;
        int *density_variances;
        double y_max, interval;
    
    
        // vector of the destribution of the semivariances of the distance classes:
        density_variances = create_vector(Map->variogram.numClasses);
        if (density_variances == NULL){
            longjmp(env, 1);
        }
    
        // gives you the maximum value of the semivariance vector
        y_max = get_fvector_max(variogram_variances, Map->variogram.numClasses);
    
        // Calculates the width of an interval out of the maximum value and the number of intervals
        interval = y_max/Map->variogram.numClasses;
       
        // Count the number of semivarinaces and assign them to the vecor desity_variances
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
            for (jdx=0; jdx<Map->variogram.numClasses-1; jdx++){
            
                if ((variogram_variances[idx] > jdx*interval) && (variogram_variances[idx] < (jdx+1)*interval)){
                
                    density_variances[jdx]++;
                }
            }
        }
        
        // now find the maximum value within the density_variances vector
        // take the first occurence
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
            if (density_variances[idx] == get_dvector_max(density_variances, Map->variogram.numClasses)){
                
                imax = idx;
            }
        }

        // Now make the sum over the number of all semivariances until the the interval with the most values.
        for (idx=0; idx<=imax; idx++){
            Map->variogram.model_adjust_index += density_variances[idx];
        
        }
        
        free(density_variances);
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> There must be at least one variogram class.\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;        
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }  
}


// ##################################################################################################
// ##################################################################################################


int create_covariance_matrix(struct usr_map *Map){

    /*
        DESCRIPTION:
        
        Creates by using the determined semivariance model a covariance matrix.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object

        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
    */

    int idx, jdx;
    int excno;
    jmp_buf env;


    if ((excno = setjmp(env)) == 0){  

        double tmp;	 		// temporary value for checking purposes

        if (Map->show_output){
            printf("Calculate covariance matrix ... ");
            fflush(stdout);
        }

        // Reservieren der Kovarianzmatrix:   
        Map->covariance_matrix = create_fmatrix(Map->input_data.length+1, Map->input_data.length+1);
        if (Map->covariance_matrix == NULL){
            longjmp(env, 1);
        }
    
        // Berechne die Kovarianzen:
        for (idx=0; idx<Map->input_data.length+1; idx++){
            for (jdx=0; jdx<Map->input_data.length+1; jdx++){
            
                // Ist der zu berechnende Wert gleich der letzten Spalte und der letzten Zeile,
                // ... so setze ihn auf 0:
                if ((idx == (Map->input_data.length)) && (jdx == (Map->input_data.length))){
                
                    Map->covariance_matrix[idx][jdx] = 0;
                    continue;
                    
                }
                
                // Ist der zu berechnende Wert gleich der letzten Spalte, oder der letzten Zeile,
                // ... so setze ihn auf 1:                
                if ((idx == (Map->input_data.length)) || (jdx == (Map->input_data.length))){
                
                    Map->covariance_matrix[idx][jdx] = 1;
                    continue;
                }
                else{
                    // Ist die Distanz 0 , dann entspricht die Kovarianz dem nugget-Wert:
                    if (Map->distance_matrix[idx][jdx] < EPS){
                        
                        Map->covariance_matrix[idx][jdx] = Map->variogram.nugget;
                    }
                    else{
                        
                        tmp = calc_covariance(Map->distance_matrix[idx][jdx], 
                                              Map->variogram.sill, 
                                              Map->variogram.nugget,
                                              Map->variogram.range);
                        
                        if ((isnan(tmp)) || (isinf(tmp))){
                            longjmp(env, 2);
                        }
                        else{
                            Map->covariance_matrix[idx][jdx] = tmp;
                        }
                    }
                }
            }
        }
        
        if (Map->show_output){
            printf("ok!\n");
        }
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> There must be at least one variogram class.\n", __FILE__, __LINE__); return EXIT_FAILURE;                
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int create_inverted_covariance_matrix(struct usr_map *Map){

    /*
    
        DESCRIPTION
        Calculates the inverse of a matrix of type double by using Gauß-Jordan method.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object
        
        OUTOUT:
        on success:		...	EXIT_SUCCESS
        on failure:		...	EXIT_FAILURE
        
    */

    int idx, jdx, kdx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){
    
        double temp;
        double **matrix_temp;		// temporary matrix for calculation purposes.

        if (Map->show_output){
            printf("Calculating the inverted covariance-matrix ... ");
            fflush(stdout);
        }

        Map->covariance_matrix_inv = create_fmatrix(Map->input_data.length+1, Map->input_data.length+1);
        if (Map->covariance_matrix_inv == NULL){
            longjmp(env, 1);
        }
        
        matrix_temp = create_fmatrix(Map->input_data.length+1, Map->input_data.length+1);
        if (matrix_temp == NULL){
            longjmp(env, 1);
        }
   
        // Copy the content of the covariance matrix to the temporary matrix:
        for(idx=0; idx<Map->input_data.length+1; idx++){
            for(jdx=0; jdx<Map->input_data.length+1; jdx++){
	        if ((isnan(Map->covariance_matrix[idx][jdx])) || (isinf(Map->covariance_matrix[idx][jdx]))){
	            longjmp(env, 2);
	        }
	        else{
	            matrix_temp[idx][jdx] = Map->covariance_matrix[idx][jdx];
	        }
	    }
        }

        
        // Set the diagonal values of the inverted matrix to 1 else leave it at 0:
        for(idx=0; idx<Map->input_data.length+1; idx++){
            Map->covariance_matrix_inv[idx][idx] = 1;
        }


        // finally perform some calculations
        for(kdx=0 ;kdx<Map->input_data.length+1; kdx++){
    														
	    temp=matrix_temp[kdx][kdx];
											
            for(jdx=0; jdx<Map->input_data.length+1; jdx++){
        
	        matrix_temp[kdx][jdx] /= temp;
	        Map->covariance_matrix_inv[kdx][jdx] /= temp;
	    }
														
	    for(idx=0; idx<Map->input_data.length+1; idx++){
	
	        temp=matrix_temp[idx][kdx];
	    									
	        for(jdx=0; jdx<Map->input_data.length+1; jdx++){
	    												
		    if(idx == kdx){
		        break;
		    }									
		    matrix_temp[idx][jdx] -= matrix_temp[kdx][jdx] * temp;
								
		    Map->covariance_matrix_inv[idx][jdx] -= Map->covariance_matrix_inv[kdx][jdx] * temp;
	        }
	    }
        }
        
        for(idx=0; idx<Map->input_data.length+1; idx++){
            free(matrix_temp[idx]);
        }
        free(matrix_temp);
        
        if (Map->show_output){
            printf("ok\n");
        }
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            case 2: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The covarinace matrix contains \"NAN\" and \"INF\" values\n", __FILE__, __LINE__); return EXIT_FAILURE;                
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int interpolate_raster(struct usr_map *Map){

    /*
    
        DESCRIPTION:
        Calculates for any point of the raster who has no value an interpolated value.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object.
        
        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
    

    int idx, jdx, kdx;
    int excno;
    jmp_buf env;
    int err = EXIT_FAILURE;
    
    
    if ((excno = setjmp(env)) == 0){
    
        int value_cnt=1;
        double *cov_vector, *weights_vector;
        double sum;
        double check_tmp;			// just for check purposes
    
        cov_vector = create_fvector(Map->input_data.length+1);
        if (cov_vector == NULL){
            longjmp(env, 1);
        }
        
        weights_vector = create_fvector(Map->input_data.length+1);
        if (weights_vector == NULL){
            longjmp(env, 1);
        }
        
        
        if (Map->show_output){
            printf("interpolating ...         ");
            fflush(stdout);
        }
        
        // Now go to every point of the raster step by step:
        for (idx=0; idx<(Map->rows); idx++){
            for (jdx=0; jdx<(Map->cols); jdx++){
        
                if (Map->show_output){
                    printf("\b\b\b\b\b\b\b\b\b");
                    fflush(stdout);                   
                    printf(" %5.1f %% ", ((value_cnt*1.0)/(Map->rows*Map->cols*1.0)*100.0));               
                    fflush(stdout);
                } 
                
                value_cnt++;
                
                // If there is no value at the raster then interpolate:
                // default value for no value is -1
                if (Map->raster[idx][jdx].value < 0){
            
                    sum = 0;
            
                    // calculate for this point the distance and then the covariance to any other point with a value on the raster.
                    for (kdx=0; kdx<Map->input_data.length+1; kdx++){
                
                        if (kdx != Map->input_data.length){
                        
                            // calculate the distance:
                            check_tmp = calc_distance(Map->raster[idx][jdx].lat,
                                                      Map->raster[idx][jdx].lon, 
                                                      Map->input_data.data[kdx].lat,
                                                      Map->input_data.data[kdx].lon);
                                                   
                            // just check if the distance is nan or inf:                                                      
                            if ((isnan(check_tmp)) || (isinf(check_tmp))){
                                longjmp(env, 2);
                            }
                            else{
                                // calculate the covariance
                                check_tmp = calc_covariance(check_tmp,
                                                            Map->variogram.sill, 
                                                            Map->variogram.nugget,
                                                            Map->variogram.range);
                                                            
                                // just check if the covariance is nan or inf:
                                if ((isnan(check_tmp)) || (isinf(check_tmp))){
                                    longjmp(env, 3);
                                }
                                else{
                                    cov_vector[kdx] = check_tmp;
                                }                  
                            }  
                        }
                        else{
                            // set the last value to 1:
                            cov_vector[kdx] = 1;
                        }
                    }
                    
                    // now multiply the individual covarinance vector with the inverted covariance matrix:
                    err = multiplyMatrixVector(Map->covariance_matrix_inv, 
                                               cov_vector,
                                               weights_vector,
                                               Map->input_data.length+1, 
                                               Map->input_data.length+1);
                    (err == EXIT_FAILURE) ? longjmp(env, 4) : NULL;
                      
                                                          
                    if (Map->weights_correction){
                        
                        // correct negative weights:
                        err = correct_negative_weights(weights_vector, cov_vector, Map->input_data.length);
                        (err == EXIT_FAILURE) ? longjmp(env, 5) : NULL;
                    }
                    
                    // Wert des Rasterpunktes mit 0 initialisieren:
                    Map->raster[idx][jdx].value = 0;
                    
                    // Calculate the interpolated value, as the sum of the weighted precipitation values.
                    for (kdx=0; kdx<Map->input_data.length; kdx++){
                    
                        sum += (weights_vector[kdx] * Map->input_data.data[kdx].value);
                    }
                    
                    // assign value to raster
                    Map->raster[idx][jdx].value = sum;
                    
                }
                else{
                    // is there a value skip that point.
                    continue;
            
                }   
            }    
        }
        if (Map->show_output){
            printf("ok\n");
        }
        
        free(cov_vector);
        free(weights_vector);
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            case 2: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The calculated distance is \"NAN\" or \"INF\"\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 3: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The calculated covariance is \"NAN\" or \"INF\"\n", __FILE__, __LINE__); return EXIT_FAILURE; 
            case 4: fprintf(stderr, "\nERROR: %s --> %d:\n >>> Calculation of weights returned an error!\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 5: fprintf(stderr, "\nERROR: %s --> %d:\n >>> Correction of negative weights returned an error!\n", __FILE__, __LINE__); return EXIT_FAILURE;                              
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
};        


// ##################################################################################################
// ##################################################################################################


int correct_negative_weights(double *weights_vector, double *cov_vector, int length){

    /*
    
        DESCRIPTION:
        Sets negative weights to 0.
        Performs subsequently a restandardization to get the sum of all posiitive weights to 1.
        
        INPUT:
        double *weights_vector		...	pointer to the weights vector to correctify.
        double *cov_vector		...	pointer to the covariance vector.
        int length			...	length of these vectors.
        
        OUTPUT: (error code)
        on success			...	EXIT_SUCCESS
        on failure			...	EXIT_FAILURE
    
    */

    int idx;
    int excno;
    jmp_buf env;


    if ((excno = setjmp(env)) == 0){

        int cnt=0;
        double sum_weights=0;					// sum of all weights
        double sum_cov=0;					// sum of all covariances
        double absAvg_negWeights=0;				// absolute average of negative weights
        double avgCov_negWeights=0;				// average of neg. covariances
    

 
        for (idx=0; idx<length; idx++){
    
            // if the weight is negative...
            if (weights_vector[idx] < 0){
            
                // Calculate the absolute sum of these negative weights:
                sum_weights += fabs(weights_vector[idx]);
            
                // calculate the sum of the corresponding covariances
                sum_cov += cov_vector[idx];
            
                // raise the counter for the number of negative weights
                cnt++;
            }
        }
        // end this function if there are no negative weights.   
        if (cnt == 0){
    
            return EXIT_SUCCESS;
        }
        else{
            // Calculate the average of the sum of neg. weights and covariances
            absAvg_negWeights = (sum_weights/cnt);
            avgCov_negWeights = (sum_cov/cnt);
            
            if ((isnan(absAvg_negWeights)) || (isinf(absAvg_negWeights)) || 
                (isnan(avgCov_negWeights)) || (isinf(avgCov_negWeights))){
            
                longjmp(env, 1);   
            }      
        
            sum_weights = 0;
        
            for (idx=0; idx<length; idx++){
    
                // is the weight < 0, so set it to 0:
                if (weights_vector[idx] < 0){
                    weights_vector[idx] = 0;
                }
                // is the weight > 0 and the covariance < avgCov_negWeights and the weight < absAvg_negWeights ...
                else if ((weights_vector[idx] > 0) && 
                         (cov_vector[idx] < avgCov_negWeights) && 
                         (weights_vector[idx] < absAvg_negWeights)){
                
                    // set the weight to 0 ...
                    weights_vector[idx] = 0;            
                }
                else{
                    // is the weight not set to 0, than sum up this value
                    sum_weights += weights_vector[idx];
                }
            }
        
            // is the sum of weights > 0, ...
            if (sum_weights > 0){
    
                // ... restandardize the weights:
                for (idx=0; idx<length; idx++){
    
                    // but only the positive weights.
                    // the rest remains at 0
                    if (weights_vector[idx] > 0){

                        weights_vector[idx] = weights_vector[idx] / sum_weights;
                    }
                    else{
                        weights_vector[idx] = 0;
                    }
                }
            }
        }
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The averages of the negative weights or covariances are \"NAN\" or \"INF\"!\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "\nERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;               
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }    
}


// ##################################################################################################
// ##################################################################################################


double calc_distance(double latA, double lonA, double latB, double lonB){

    /*
        DESCRIPTION:
        Calculates the distance between 2 point on a sphere with the radius of the earth at 51°N.
        
        INPUT:
        double latA	...	latitude of point A in decimal degree.
        double lonA	...	longitude of point A in decimal degree.
        double latB	...	latitude of point B in decimal degree.
        double lonB	...	longitude of point B in decimal degree.
        
        OUTPUT:
        on success	...	distance between the two points in km
        on failure	...	NAN
    */ 
    
    jmp_buf env;
    int excno;
    
    if ((excno = setjmp(env)) == 0){
        
        if ((isnan(latA)) || (isnan(lonA)) || (isnan(latB)) || (isnan(lonB)) ||
            (isinf(latA)) || (isinf(lonA)) || (isinf(latB)) || (isinf(lonB))){
        
            longjmp(env, 1);
        }
        
        
        // point A: 
        double latA_rad = (latA/180.0) * M_PI;
        double lonA_rad = (lonA/180.0) * M_PI;

        // point B:
        double latB_rad = (latB/180.0) * M_PI;
        double lonB_rad = (lonB/180.0) * M_PI;

        return RADIUS_EARTH * acos( (sin(latA_rad) * sin(latB_rad) ) + ( cos(latA_rad) * cos(latB_rad) * ( cos(lonB_rad - lonA_rad) ) ) );
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> One of the coordinates is equal to \"NAN\" or \"INF\"\n", __FILE__, __LINE__); return NAN;                       
            case 2: fprintf(stderr, "\nERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return NAN;               
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NAN;
        }  
    }   
}


// ##################################################################################################
// ##################################################################################################


double calc_covariance(double distance, double sill, double nugget, double range){

    /*
        DESCRIPTION:
        Calculates the covariance of any point by unsing the determined variogram model,
        the distance, sill, nugget and range.
        
        INPUT:
        double distance	...	distance between two points
        double sill	...	sill of the variogram model
        double nugget	...	nugget of the variogram model
        double range	...	range of the variogram model
        
        OUTPUT:
        on success	...	covariance 
        on failure	...	NAN
    */
    
    jmp_buf env;
    int excno;
    
    if ((excno = setjmp(env)) == 0){   
    
        double z, n;
    
        // exponetial model:
        // nugget + sill *(1-exp(-|distance|/(range/3)))
    
        n = range / 3.0;		// 
        z = -1 * fabs(distance);	// distance must be negative
    
        return nugget + sill * (1 - exp(z/n));
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return NAN;                
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NAN;
        }
    }
}


// ##################################################################################################
// ##################################################################################################



int multiplyMatrixVector(double **matrix, double *vector_in, double *vector_out, int rows, int cols){

    /*
    
        DESCRIPTION:
        Multiplies a matrix of type double with a vector of type double.
        
        INPUT:
        double **matrix		...	pointer to the matrix you want to multiply with a vector.
        double *vector_in	...	pointer to the vector you want to multiply with a matrix.
        double *weights_vector	...	pointer to the result vector.
        int rows		...	number of rows of the matrix
        int cols		...	number ot columns of the matrix.
        
        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */

    int idx, jdx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){

        double sum=0;

        if (rows != cols){
            longjmp(env, 1);
        }
   
        // check for nan and inf in vector
        for (idx=0; idx<rows; idx++){
            if ((isnan(vector_in[idx])) || (isinf(vector_in[idx]))){
                longjmp(env, 2);
            }
        }
        
        // check for nan and inf in matrix
        for (idx=0; idx<rows; idx++){
            for (jdx=0; jdx<cols; jdx++){
                if ((isnan(matrix[idx][jdx])) || (isinf(matrix[idx][jdx]))){
                    longjmp(env, 3);
                }
            }
        }
   
        // multipy:
        for (idx=0; idx<rows; idx++){
                    
            vector_out[idx] = 0;
                
            for (jdx=0; jdx<cols; jdx++){
                
                vector_out[idx] += (matrix[idx][jdx] * vector_in[jdx]);
            }
        }
        return EXIT_SUCCESS;       
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The number of columns of the matrix must be equal to number of rows of the vector\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "\nERROR: %s --> %d:\n >>> Input vector contains \"NAN\" or \"INF\" values!\n", __FILE__, __LINE__); return EXIT_FAILURE; 
            case 3: fprintf(stderr, "\nERROR: %s --> %d:\n >>> Input matrix contains \"NAN\" or \"INF\" values!\n", __FILE__, __LINE__); return EXIT_FAILURE;                                
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }       
}


// ##################################################################################################
// ##################################################################################################


double calc_RSME(double *values1, double *values2, int length){


    /*
        DESCRIPTION
        Calculates the rsme error between two datasets of type double.
        
        Input:
        values1		...	Datenreihe 1
        values2		...	Datenreihe 2
    */

    int idx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){
    
        double qsum=0;		// sum of the powered differences.
    
        if (length <= 0){
            longjmp(env, 1);
        }
        
        for (idx=0; idx<=length; idx++){
    
            if (((isnan(values1[idx])) || (isinf(values1[idx]))) ||
               ((isnan(values2[idx])) || (isinf(values2[idx])))){
               
                   longjmp(env, 2);
            }
            else{
                qsum += pow((values1[idx] - values2[idx]),2);
            } 
        }
        return sqrt(qsum/length);
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The length must be greater then 0!\n", __FILE__, __LINE__); return NAN;
            case 2: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The datasets contains NAN or INF values!\n", __FILE__, __LINE__); return NAN;        
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NAN;
        }
    }
}


// ##################################################################################################
// ##################################################################################################


double get_fvector_max(double *values, int length){

    /*
    
        DESCRIPTION:
        Determines the maximum value of a vector of type double and length "length".
        
        INPUT:
        double *values	...	pointer to the vector of doubles.
        int length	...	length of the vector.
        
        OUTPUT:
        on success	...	maximum value of that vector
        on failure	...	NAN
        
    */

    
    int idx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){
        
        double maxValue = values[0];
    
    
        for (idx=0; idx<length; idx++){
    
            if (values[idx] > maxValue){
        
                maxValue = values[idx];
            }
        }
        return maxValue;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The length must be greater then 0!\n", __FILE__, __LINE__); return NAN;
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NAN;
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int get_dvector_max(int *values, int length){

    /*
    
        DESCRIPTION:
        Determines the maximum value of a vector of type int and length "length".
        
        INPUT:
        int *values	...	pointer to the vector of integers.
        int length	...	length of the vector.
        
        OUTPUT:
        on success	...	maximum value of that vector
        on failure	...	exit(errno)
        
    */
    
    int idx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){     
    
        if (length <= 0){
            longjmp(env, 1);
        }
    
        int maxValue = values[0];
    
        for (idx=0; idx<length; idx++){
    
            if (values[idx] > maxValue){
        
                maxValue = values[idx];
            }
        }
        return maxValue;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The length must be greater then 0!\n", __FILE__, __LINE__); exit(errno);
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); exit(errno);
        }  
    }
}


// ##################################################################################################
// ##################################################################################################


double get_fvector_min(double *values, int length){

    /*
    
        DESCRIPTION:
        Determines the minimum value of a vector of type double and length "length".
        
        INPUT:
        double *values	...	pointer to the vector of doubles.
        int length	...	length of the vector.
        
        OUTPUT:
        on success	...	minimum value of that vector
        on failure	...	NAN
        
    */

    
    int idx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){
        
        double minValue = values[0];
    
    
        for (idx=0; idx<length; idx++){
    
            if (values[idx] < minValue){
        
                minValue = values[idx];
            }
        }
        return minValue;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The length must be greater then 0!\n", __FILE__, __LINE__); return NAN;
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NAN;
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int get_dvector_min(int *values, int length){

    /*
    
        DESCRIPTION:
        Determines the minimum value of a vector of type int and length "length".
        
        INPUT:
        int *values	...	pointer to the vector of integers.
        int length	...	length of the vector.
        
        OUTPUT:
        on success	...	minimum value of that vector
        on failure	...	NAN
        
    */
    
    int idx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){     
    
        if (length <= 0){
            longjmp(env, 1);
        }
    
        int minValue = values[0];
    
        for (idx=0; idx<length; idx++){
    
            if (values[idx] < minValue){
        
                minValue = values[idx];
            }
        }
        return minValue;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> The length must be greater then 0!\n", __FILE__, __LINE__); exit(errno);
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); exit(errno);
        }
    }     
}


// ##################################################################################################
// ##################################################################################################


int outputRasterCSV(struct usr_data_point **raster, char *output_dir, char *filename, int rows, int cols, bool show_output){

    /*
    
        DESCRIPTION:
        This function outputs 3 csv-files.
        1.	csv-file with the values of the raster in dimensions of rows x cols.
        2.	csv-file with the latitude values of the values in dimensions of Map.rows x Map.cols.
        3.	csv-file with the longitude values of thevalues in dimensions of Map.rows x Map.cols.
        
        INPUT:
        struct usr_data_point **raster	...	pointer to the matrix you want to output of type struct usr_data_point.
        char *output_dir		...	directory you want to output the data
        char *filename			...	filename of the csv-file
        int rows			...	number of rows of the raster
        int cols			...	number of columns of the raster
        
    */



    int idx, jdx, kdx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){
    
        char valueText[20], latText[20], lonText[20];
        char path[100];
    
        FILE *fp_values, *fp_lat, *fp_lon;
    
    
        if ((rows<=0) || (cols<=0)){
            longjmp(env, 1);        
        }
    
        // file pointer to the values csv file:
        fp_values = fopen(strcat(strcpy(path, output_dir), filename),"w");
        if (fp_values == NULL){
            longjmp(env, 2);
        }
        
        // file pointer to the latitude csv file:
        fp_lat = fopen(strcat(strcpy(path, output_dir), "lat.csv"),"w");
        if (fp_lat == NULL){
            longjmp(env, 3);
        }
    
        // file pointer to the csv file for the longitude values:  
        fp_lon = fopen(strcat(strcpy(path, output_dir), "lon.csv"),"w");
        if (fp_lon == NULL){
            longjmp(env, 2);
        }

        // show output?
        if (show_output){
            printf("\nwriting csv files to:\n");
            printf(">>> %s\n", output_dir);
            
            printf("writing ... ");
            fflush(stdout);
        }
    
        
        for (idx=0; idx<rows; idx++){
            for (jdx=0; jdx<cols; jdx++){
            
                // pass the value to a field of type char:
                sprintf(valueText,"%.3f", raster[idx][jdx].value);
                
                // pass the latitude to a field of type char:
                sprintf(latText, "%.4f", raster[idx][jdx].lat);
                
                // pass the longitude to a field of type char:
                sprintf(lonText, "%.4f", raster[idx][jdx].lon);
                
                
                // check for points and change them to comma
                for (kdx=0; kdx<(int)strlen(valueText); kdx++){
                
                    if (valueText[kdx] == ','){
                    
                        valueText[kdx] = '.';
                        break;
                    }
                
                }
                // write that value into the file
                fprintf(fp_values,"%s",valueText);


                // check for points and change them to comma
                for (kdx=0; kdx<(int)strlen(latText); kdx++){
                
                    if (latText[kdx] == ','){
                    
                        latText[kdx] = '.';
                        break;
                    }
                }
                // write that latitude into the file               
                fprintf(fp_lat,"%s",latText);

                // check for points and change them to comma
                for (kdx=0; kdx<(int)strlen(lonText); kdx++){
                
                    if (lonText[kdx] == ','){
                    
                        lonText[kdx] = '.';
                        break;
                    }
                }
                // write that longitude into the file                
                fprintf(fp_lon,"%s",lonText);
                
                // add a semicolon to the line until ... 
                if (jdx != (cols-1)){
                
                    fprintf(fp_values,"%s",";");
                    fprintf(fp_lat,"%s",";");    
                    fprintf(fp_lon,"%s",";");
                    
                }
                else{
                    // ... we finish that line, than add a newline sign:
                    fprintf(fp_values, "%s", "\n");
                    fprintf(fp_lat, "%s", "\n");    
                    fprintf(fp_lon, "%s", "\n");                
                
                }
            }
        }
        
        // close all files:
        fclose(fp_values);
        fclose(fp_lat);
        fclose(fp_lon);
        
        if (show_output){
            printf("ok\n");
        }
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n The number of rows and columns must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\n The filepointer returns an error!\n>>> %s\n\n", __FILE__, __LINE__,strerror(errno)); return EXIT_FAILURE;
            case 3: fprintf(stderr, "ERROR: %s --> %d:\n %s\n\n", __FILE__, __LINE__,strerror(errno)); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int get_output_information(struct usr_map *Map){

    /*
    
        DESCRIPTION:
        Calculates/determines additional information to the output raster.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object
        
        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
    
    int idx, jdx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){
    
        double sum=0;
    
        // determine the maximum and minimum value of the raster:
        Map->output_data.maximum = Map->raster[0][0].value;
        Map->output_data.minimum = Map->raster[0][0].value; 
        Map->output_data.average = Map->raster[0][0].value; 
       
        for (idx=0; idx<Map->rows; idx++){
    
            for (jdx=0; jdx<Map->cols; jdx++){
        
                if (Map->raster[idx][jdx].value > Map->output_data.maximum){
            
                    Map->output_data.maximum = Map->raster[idx][jdx].value;
                }
            
                if (Map->raster[idx][jdx].value < Map->output_data.minimum){
            
                    Map->output_data.minimum = Map->raster[idx][jdx].value;
                
                }
            
                sum += Map->raster[idx][jdx].value;
                  
           }
    
        }
    
        Map->output_data.average = sum / (double)(Map->rows * Map->cols);
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n %s\n\n", __FILE__, __LINE__,strerror(errno)); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }    
}


// ##################################################################################################
// ##################################################################################################


int check_matrix(double **matrix, int rows, int cols, bool show_output){

    /*
        DESCRIPTION:
        Checks a matrix of type double if it contains nan or inf - values.
        If "show_output" set to true it outputs finally a summary of the results, otherwise it will
        only show results if nan- or inf values are included.
    
        INPUT:
        double **matrix		...	pointer to the matrix
        int rows		...	number of rows of that matrix
        int cols		...	number of cols of that matrix
        bool show_output	...	show results after the checks?
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
    
    jmp_buf env;
    int idx, jdx;
    int excno;
    
    if ((excno = setjmp(env)) == 0){    
    
    
        unsigned int cnt_nan = 0;
        unsigned int cnt_inf = 0;
        unsigned int cnt_neg = 0;
        unsigned int cnt_pos = 0;
    
        double max, min;
    
        if (show_output){
            printf("Check matrix:\n");
            printf("-----------------------\n");
        }
        
        max=matrix[0][0];
        min=matrix[0][0];        
        
        for (idx=0; idx<rows; idx++){
            for (jdx=0; jdx<cols; jdx++){
        
                if (isnan(matrix[idx][jdx])){
            
                    cnt_nan++;
                    continue;
                }
                else if (isinf(matrix[idx][jdx])){
                
                    cnt_inf++;
                    continue;
                }
                else if (matrix[idx][jdx] < 0){
                    
                    // count the neg. value
                    cnt_neg++;
                    
                    // check if the current value is higher then the current maximum
                    if (matrix[idx][jdx] > max){
                        max=matrix[idx][jdx];
                    }
                    
                    // check if the current value is lower then the current minimum                  
                    if (matrix[idx][jdx] < min){
                        min=matrix[idx][jdx];
                    }
                    
                    continue;
                }
                else if (matrix[idx][jdx] >= 0){
                
                    // count the pos. value
                    cnt_pos++;
                    
                    // check if the current value is higher then the current maximum
                    if (matrix[idx][jdx] > max){
                        max=matrix[idx][jdx];
                    }
                    
                    // check if the current value is lower then the current minimum                  
                    if (matrix[idx][jdx] < min){
                        min=matrix[idx][jdx];
                    }                    
                    continue;
                }
            }
        }
        
        if ((show_output) || (cnt_nan > 0) || (cnt_inf > 0)){

            printf("Summary:\n");
            printf("----------------\n");
               
            printf("Maximum: %.6f\n", max);
            printf("Minimum: %.6f\n", min);
    
            printf("Anzahl nan: %d\n", cnt_nan);
            printf("Anzahl inf: %d\n", cnt_inf);
            printf("Anzahl > 0: %d\n", cnt_pos);    
            printf("Anzahl < 0: %d\n", cnt_neg);  
      
            printf("-----------------------\n\n");
        }
        
        if ((cnt_nan > 0) || (cnt_inf > 0)){
            longjmp(env, 1);        
        }
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n The matrix contains \"NAN\" or \"INF\" values!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }
}


// ##################################################################################################
// ##################################################################################################

    
int show_variogram_data(struct usr_map *Map){

    /*
        DESCRIPTION:
        shows you a summary of the variogram data if the "show_output" attribut of the map object is set to true.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object.
        
        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
    
    jmp_buf env;
    int excno;
    int idx;    
    
    
    
    if ((excno = setjmp(env)) == 0){
    
        if (Map->show_output){

            printf("########################################################################################\n");    
            printf("#################################### SEMIVARIOGRAM #####################################\n\n");
       
            printf("--------------------------------------\n");
            printf("Solution of the polynomic function:\n");
            printf("Order:%3d\n", Map->variogram.reg_function.order);
            for (idx=0; idx<Map->variogram.reg_function.order+1; idx++){
                printf("b%d: %11.6f\n", idx, Map->variogram.reg_function.solution[idx]);
            }
            printf("R²: %8.3f\n", Map->variogram.reg_function.solution[Map->variogram.reg_function.order+1]);    
            printf("--------------------------------------\n\n");
        
            printf("Distance classes:\n");
            printf("----------------------------------------------------------------------------------------\n");    
            printf("%3s%12s%12s%15s%15s%15s\n","id", "lower Limit", "upper Limit", "Distance (AVG)", "Variance (AVG)", "Variance (REG)");
            for (idx=0; idx<Map->variogram.numClasses; idx++){
    
                printf("%3d%12d%12d%15.3f%15.3f%15.3f\n", idx,
                                                          Map->variogram.classes[idx].lowerLimit,
                                                          Map->variogram.classes[idx].upperLimit,
                                                          Map->variogram.classes[idx].distance_avg,
                                                          Map->variogram.classes[idx].variance_avg,
                                                          Map->variogram.classes[idx].variance_avg_reg);
        
            }
            printf("----------------------------------------------------------------------------------------\n\n");
  
            printf("--------------------------------------\n");
            printf("Model - fit-index: %d\n\n", Map->variogram.model_adjust_index);
            printf("Solution of the semivariance-model:\n");    
            printf("nugget: %.3f\n", Map->variogram.nugget);
            printf("sill: %.3f\n", Map->variogram.sill);
            printf("range: %.3f\n", Map->variogram.range);
            printf("--------------------------------------\n\n");
            printf("########################################################################################\n");
            printf("########################################################################################\n\n");
        }
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }          
}


// ##################################################################################################
// ##################################################################################################


int show_map_info(struct usr_map *Map){

    /*
    
        DESCRIPTION
        Show information to the current interpolated raster.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object
        
        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */

    jmp_buf env;
    int excno;
    
    if ((excno = setjmp(env)) == 0){
    
        if (Map->show_output){
            printf("\n\n");
            printf("############### INFORMATION ##############\n\n");
            printf("   Geogr. latitude: %.3f (minimum)\n",Map->minLat);
            printf("   Geogr. latitude: %.3f (maximum)\n",Map->maxLat);   
            printf("   Geogr. longitude: %.3f (minimum)\n",Map->minLon);
            printf("   Geogr. longitude: %.3f (maximum)\n\n",Map->maxLon);    
            printf("\n");    
            printf("   Number of raster points: %d\n", (Map->rows)*(Map->cols));
            printf("   Rows: %d\n", Map->rows);
            printf("   Columns: %d\n", Map->cols); 
            printf("\n");    
            printf("   Resolution (latitude): %.3f\n", Map->latRes);
            printf("   Resolution (longitude): %.3f\n", Map->lonRes); 
            printf("   Resolution (latitude, km): %.3f\n", Map->latMetRes);
            printf("   Resolution (longitude, km): %.3f\n", Map->lonMetRes);  
            printf("\n");    
            printf("   --------------input data---------------\n\n");
            printf("   Number of measured values: %d\n", Map->input_data.length);
            printf("   Maximum: %.3f\n", Map->input_data.maximum);
            printf("   Minimum: %.3f\n", Map->input_data.minimum);
            printf("   Average: %.3f\n", Map->input_data.average);
            printf("\n");
            printf("   --------------output data--------------\n\n");
            printf("   Maximum: %.3f\n", Map->output_data.maximum);
            printf("   Minimum: %.3f\n", Map->output_data.minimum);
            printf("   Average: %.3f\n", Map->output_data.average);
            printf("\n");    
            printf("##########################################\n\n");          
        }
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }    
}


// ##################################################################################################
// ##################################################################################################


int show_matrix(char *name, double **matrix, int rows, int cols, bool show_output){

    /*
    
        DESCRIPTION:
        Returns the matrix of type double on stdout within the defined limits.
        
        
        INPUT:
        double **matrix		...	pointer to the matrix to show
        int rows		...	rows to show
        int cols		...	columns to show
        
        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */
    
    
   
    int idx, jdx;
    int excno;
    jmp_buf env;
    
    
    
    if ((excno = setjmp(env)) == 0){
    
        if ((rows <= 0) || (cols <= 0)){
            longjmp(env, 1);
        }
        
        if (show_output){
            printf("%s\n", name);
    
            for (idx=0; idx<rows; idx++){
    
                for (jdx=0; jdx<cols; jdx++){
        
                    printf("%8.3f", matrix[idx][jdx]);   
        
                }
        
                printf("\n");
            }
            printf("\n");
        }
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> The given number of rows and columns are lower/equal to 0!\n", __FILE__, __LINE__); return EXIT_FAILURE;                
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }    
    }
}


// ##################################################################################################
// ##################################################################################################


int show_input_data(struct usr_map *Map){

     /*
     
        DESCRIPTION:
        shows the input dataset on stdout.
        
        INPUT:
        struct usr_map *Map	...	pointer to the map object.
        
        OUTPUT: (error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
        
    */   

    int idx;
    int excno;
    jmp_buf env;
    
    if ((excno = setjmp(env)) == 0){   
    
        if (Map->show_output){
            printf("%3s %-30s%10s%10s%10s%6s%6s\n","id", "name", "lat", "lon", "value", "row", "col");
        
            for (idx=0; idx<Map->input_data.length; idx++){
    
                printf("%3d %-30s%10.3f%10.3f%10.2f%6d%6d\n",idx,
                                                             Map->input_data.data[idx].name,
                                                             Map->input_data.data[idx].lat,
                                                             Map->input_data.data[idx].lon,
                                                             Map->input_data.data[idx].value,
                                                             Map->input_data.data[idx].row_idx,
                                                             Map->input_data.data[idx].col_idx);
            }
        }
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__,strerror(errno)); return EXIT_FAILURE;                
            default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        }
    }       
}


// ##################################################################################################
// ##################################################################################################


void free_raster(struct usr_map *Map){

    int idx;
    
    printf("\n");
    
    // check if the raster exists
    if (Map->raster != NULL){
        for (idx=0; idx<(Map->rows); idx++){
    
            free(Map->raster[idx]);
        }
        if (Map->show_output){
            free(Map->raster);
            printf("%-40s %s\n","raster:","deallocate memory successful!");
        }
    }
    
    //--------------------------------------------------------------------------------
       
    // check if distance matrix exists:
    if (Map->distance_matrix != NULL){
        for (idx=0; idx<Map->input_data.length; idx++){
    
            free(Map->distance_matrix[idx]);
        }
        if (Map->show_output){        
            free(Map->distance_matrix);
            printf("%-40s %s\n","distance matrix:","deallocate memory successful!");
        }
    }   
     
    //--------------------------------------------------------------------------------
          
    // check if covariance matrix exists:
    if (Map->covariance_matrix != NULL){
        for (idx=0; idx<Map->input_data.length+1; idx++){
    
            free(Map->covariance_matrix[idx]);
        }
        if (Map->show_output){                
            free(Map->covariance_matrix);
            printf("%-40s %s\n","covariance matrix:","deallocate memory successful!");
        }
    }
     
    //--------------------------------------------------------------------------------
           
    // check if inverted covariance matrix exists:
    if (Map->covariance_matrix_inv != NULL){    
        for (idx=0; idx<Map->input_data.length+1; idx++){
    
            free(Map->covariance_matrix_inv[idx]);
        }
        if (Map->show_output){                        
            free(Map->covariance_matrix_inv);
            printf("%-40s %s\n","inverted covariance matrix:","deallocate memory successful!");   
        }
    }
}


// ##################################################################################################
// ##################################################################################################


void free_vector(struct usr_map *Map){

    printf("\n");
    
    // check if input dataset exists
    if (Map->input_data.data != NULL){
        free(Map->input_data.data);
        
        if (Map->show_output){
            printf("%-40s %s\n","input dataset:","deallocate memory successful!");
        }
    }

    
    //--------------------------------------------------------------------------------
    
    // check if the variogram classes exists    
    if (Map->variogram.classes != NULL){
        free(Map->variogram.classes);
        
        if (Map->show_output){    
            printf("%-40s %s\n","variogram classes:","deallocate memory successful!");
        }
    }

    //--------------------------------------------------------------------------------    
    
    // check if the vector for the solution of the polynomic function exists?
    if (Map->variogram.reg_function.solution != NULL){
        free(Map->variogram.reg_function.solution);
        
        if (Map->show_output){    
            printf("%-40s %s\n","solution of polynomic function:", "deallocate memory successful!");
        }
    }
    
}



