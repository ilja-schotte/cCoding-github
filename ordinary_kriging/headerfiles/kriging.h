
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


int fill_raster_with_default_data(struct usr_map *zgr);
int input_csv_data(struct usr_map *zgr, char *input_datafile);
int fill_raster_with_input_data(struct usr_map *zgr);
int create_variogram(struct usr_map *zgr);
int interpolate_raster(struct usr_map *zgr);
int multiplyMatrix(double **matrixA, double **matrixB, int rows, int cols);
int correct_negative_weights(double *weights_vector, double *cov_vector, int length);
int outputRasterCSV(struct usr_data_point **raster, char *output_dir, char *filename, int rows, int cols);
int get_output_information(struct usr_map *Map);
int get_variogram_model(struct usr_map *zgr);
int *create_vector(int length);
int get_max(int *values, int length);
int get_min(int *values, int length);


double *multiplyMatrixVector(double **matrix, double *vector_in, int rows, int cols);          
double calc_distance(double latA, double lonA, double latB, double lonB);
double calc_covariance(double distance, double sill, double nugget, double range);
double calc_RSME(double *values1, double *values2, int length);
double get_fmax(double *values, int length);
double get_fmin(double *values, int length);
int create_covariance_matrix(struct usr_map *Map);
double **inverseOfMatrix(double **matrix, int rows, int cols);
int create_distance_matrix(struct usr_map *Map);
double **create_fmatrix(int rows, int cols);
double *create_fvector(int length);

               
void show_raster(struct usr_data_point **zgr, int *rows, int *cols);
void show_map_info(struct usr_map *zgr);
void show_input_data(struct usr_map *zgr);
void show_variogram_data(struct usr_map *zgr);
void show_matrix(double **matrix, int rows, int cols);
int check_matrix(double **matrix, int rows, int cols, bool show_output);
void outputMatrixCSV(double **matrix, char *filename, int rows, int cols);
int find_model_adjust_index(struct usr_map *Map, double *variogram_variances, int length);

void free_raster(struct usr_map *zgr);
void free_vector(struct usr_map *zgr);


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
    jmp_buf env;    
    
    // ####################################### FUNCTION #############################################
    void allocate_memory(struct usr_data_point ***raster, int rows, int cols){

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
    }
    
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: allocate_memory(raster, rows, cols); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n Failure during the attempt to allocate memory\n--> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
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
    jmp_buf env;
    double cols;
    
    // ####################################### FUNCTION #############################################
    void complete_conf(struct usr_map *Map, int argc, char **argv){

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
        
    }
    
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: complete_conf(Map, argc, argv); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n The number of rows must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n The number of maxLon and minLon must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR: %s --> %d:\n The number of maxLat and minLat must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 4: fprintf(stderr, "ERROR: %s --> %d:\n The calculated number of columns of the output raster is \"NAN\" or \"INF\"!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;        
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
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
    jmp_buf env;
    
    // ####################################### FUNCTION #############################################    
    void set_default_data(struct usr_map *Map){

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
    }
    
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: set_default_data(Map); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n The number of rows must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n The number of columns must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR: %s --> %d:\n The resolution of longitude must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 4: fprintf(stderr, "ERROR: %s --> %d:\n The resolution of latitude must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;                            
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
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
    
    double sum = 0; 
    double *values_array;
    
    char inputRow[255];
    char inputDecimal[20];
    char path[100];
    
    // ####################################### FUNCTION #############################################
    void read_input_data(struct usr_map *Map, char *input_datafile){
        
        
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
        }
    }
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: read_input_data(Map, input_datafile); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n Failure when opening the csv-file:\n>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n The counted number of datapoints is 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR: %s --> %d:\n Failure when allocating the memory for the input dataset:\n>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
    }    
}


// ##################################################################################################
// ##################################################################################################




double **create_fmatrix(int rows, int cols){

    /*
        Erstellt eine 2D-Matrix vom Datentyp "double" der Dimensionen rows x cols.
        
        Rückgabewert: 
        Erfolg: Ein Doppelzeiger des Datentyps "double" auf das Feld [0][0] der 2D-Matrix.
        Fehler: NULL.        
    */

    int idx;
    double **zgr;
    
    
    zgr = (double **) calloc(rows, sizeof(double*));
    if (zgr == NULL){
    
        printf("Speicherfehler!\n");
        printf("Beim Reservieren einer Zeigerzeile von Datentyp: \"double\" ist ein Fehler aufgetreten!\n");
        printf("Funktion: **create_fmatrix\n");
        printf("Rueckgabe: NULL\n"); 
                 
        return NULL;
    }
    else{       
        for (idx=0; idx<rows; idx++){
        
            zgr[idx] = (double *) calloc(cols, sizeof(double));        
            if (zgr[idx] == NULL){
            
                printf("Speicherfehler!\n");
                printf("Beim Reservieren einer Spalte von Datentyp \"double\" ist ein Fehler aufgetreten!\n");
                printf("Funktion: **create_fmatrix\n");
                printf("Rueckgabe: NULL\n");            
            
                return NULL;
            
            }
        }
        return zgr;
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

    
    // ####################################### FUNCTION #############################################   
    double *allocate(int length){
    
        double *zgr;
        
        zgr = (double *) calloc(length, sizeof(double));
        if (zgr == NULL){
            longjmp(env, 1);
        }
        else{
            return zgr;
        }   
    }
    // ##############################################################################################
    switch(setjmp(env)){
        case 0: return allocate(length);
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n>>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return NULL;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NULL;
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

    
    // ####################################### FUNCTION #############################################   
    int *allocate(int length){
    
        int *zgr;
        
        zgr = (int *) calloc(length, sizeof(int));
        if (zgr == NULL){
            longjmp(env, 1);
        }
        else{
            return zgr;
        }   
    }
    // ##############################################################################################
    switch(setjmp(env)){
        case 0: return allocate(length);
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n>>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return NULL;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NULL;
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
    jmp_buf env;
    
    // ####################################### FUNCTION #############################################
    void fill_raster(struct usr_map *Map){    
    
        int min_row_idx, min_col_idx;
        double mlat, mlon, distance;
        double rlat, rlat_rad, rlon, rlon_rad;
        double min_distance, min_mlat, min_mlon, min_rlat, min_rlon;
        
    
        // output?
        if (Map->show_output){
            printf("Uebertrage Messpunkte auf Raster ... ");
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
    }    
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: fill_raster(Map); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n The length of the input dataset is 0\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
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
    int idx, kdx;
    
    // ####################################### FUNCTION #############################################
    void create_matrix(struct usr_map *Map){     
    
        // output ?
        if (Map->show_output){
            printf("Berechne die Distanzmatrix ... ");
            fflush(stdout);
        }
    
        // Allocate memory for a square distance matrix of "Map.input_data.length" x "Map.input_data.length"
        Map->distance_matrix = create_fmatrix(Map->input_data.length, Map->input_data.length);
        if (Map->distance_matrix == NULL){
            longjmp(env, 1);    
        }
        else{
    
            for (idx=0; idx<Map->input_data.length; idx++){
                for (kdx=0; kdx<Map->input_data.length; kdx++){
            
                    // Is the difference between the longitude and latitude value smaler then EPS, the distance must be 0:
                    if ((fabs(Map->input_data.data[idx].lat - Map->input_data.data[kdx].lat) < EPS) && 
                        (fabs(Map->input_data.data[idx].lon - Map->input_data.data[kdx].lon) < EPS)){
                
                        Map->distance_matrix[idx][kdx] = 0.0;
                    }
                    else{
                        // calculate the distance between these points:                
                        Map->distance_matrix[idx][kdx] = calc_distance(Map->input_data.data[idx].lat, 
                                                                       Map->input_data.data[idx].lon, 
                                                                       Map->input_data.data[kdx].lat, 
                                                                       Map->input_data.data[kdx].lon);
                    }
                }
            }
            // output ?
            if (Map->show_output){
                printf("ok!\n");
            }       
        }
    }
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: create_matrix(Map); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
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
    
    // ####################################### FUNCTION #############################################    
    void create(struct usr_map *Map){
    
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
            printf("Erstelle Variogramm ... ");
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
    }
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: create(Map); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> There must be at least one variogram class.\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;        
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
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
    jmp_buf env;
    
    // ####################################### FUNCTION #############################################    
    void get_model(struct usr_map *Map){    
    
    
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
    }
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: get_model(Map); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;                
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> The solution for the polynomic regression returns NULL!\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR: %s --> %d:\n >>> The vector of the predicted variances contains \"NAN\" or \"INF\" values!\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 4: fprintf(stderr, "ERROR: %s --> %d:\n >>> The value of the b0 weight is \"NAN\" or \"INF\"!\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 5: fprintf(stderr, "ERROR: %s --> %d:\n >>> The calculated value of the RSME is \"NAN\" or \"INF\"!\n", __FILE__, __LINE__); return EXIT_FAILURE;        
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
    }
}



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
    jmp_buf env;
    
    // ####################################### FUNCTION #############################################    
    void get_index(struct usr_map *Map){
    
    
    
        int imax=0;
        int *density_variances;
        double y_max, interval;
    
    
        // vector of the destribution of the semivariances of the distance classes:
        density_variances = create_vector(Map->variogram.numClasses);
        if (density_variances == NULL){
            longjmp(env, 1);
        }
    
        // gives you the maximum value of the semivariance vector
        y_max = get_fmax(variogram_variances, Map->variogram.numClasses);
    
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
        
            if (density_variances[idx] == get_max(density_variances, Map->variogram.numClasses)){
                
                imax = idx;
            }
        }

        // Now make the sum over the number of all semivariances until the the interval with the most values.
        for (idx=0; idx<=imax; idx++){
            Map->variogram.model_adjust_index += density_variances[idx];
        
        }
        
        free(density_variances);
    }
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: get_index(Map); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> There must be at least one variogram class.\n", __FILE__, __LINE__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;        
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
    }   
}



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
    jmp_buf env;

    // ####################################### FUNCTION #############################################    
    void create_matrix(struct usr_map *Map){   

        double **covariance_matrix;
        double tmp;	 		// temporary value for checking purposes

        if (Map->show_output){
            printf("Berechne die Kovarianzmatrix ... ");
            fflush(stdout);
        }

        // Reservieren der Kovarianzmatrix:   
        covariance_matrix = create_fmatrix(Map->input_data.length+1, Map->input_data.length+1);
        if (covariance_matrix == NULL){
            longjmp(env, 1);
        }
    
        // Berechne die Kovarianzen:
        for (idx=0; idx<Map->input_data.length+1; idx++){
            for (jdx=0; jdx<Map->input_data.length+1; jdx++){
            
                // Ist der zu berechnende Wert gleich der letzten Spalte und der letzten Zeile,
                // ... so setze ihn auf 0:
                if ((idx == (Map->input_data.length)) && (jdx == (Map->input_data.length))){
                
                    covariance_matrix[idx][jdx] = 0;
                    continue;
                    
                }
                // Ist der zu berechnende Wert gleich der letzten Spalte, oder der letzten Zeile,
                // ... so setze ihn auf 1:                
                if ((idx == (Map->input_data.length)) || (jdx == (Map->input_data.length))){
                
                    covariance_matrix[idx][jdx] = 1;
                    continue;
                }
                else{
                    // Ist die Distanz 0 , dann entspricht die Kovarianz dem nugget-Wert:
                    if (covariance_matrix[idx][jdx] < EPS){
                        
                        covariance_matrix[idx][jdx] = Map->variogram.nugget;
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
                            covariance_matrix[idx][jdx] = tmp;
                        }
                    }
                }
            }
        }
        
        Map->covariance_matrix = covariance_matrix;
        
        if (Map->show_output){
            printf("ok!\n");
        }
    }
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: create_matrix(Map); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> There must be at least one variogram class.\n", __FILE__, __LINE__); return EXIT_FAILURE;                
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
    }
}


// ##################################################################################################


double **inverseOfMatrix(double **matrix, int rows, int cols){

    /*
        Berechnet die Inverse einer Matrix mit Hilfe des Gauß-Jordan Algorithmuses
        
        Rückgabewert:
        Erfolg: Doppelzeiger vom Datentyp "double" auf die Inversematrix der Dimensionen rows x cols.
        Fehler: NULL
    */

    int idx, jdx, kdx;
    double temp;
    double **matrix_inv, **matrix_temp;

    printf("Berechne die Inverse der Kovarianzmatrix ... ");
    fflush(stdout);

    matrix_inv = create_fmatrix(rows, cols);
    matrix_temp = create_fmatrix(rows, cols);    
    if ((matrix_inv == NULL) || (matrix_temp == NULL)){
    
        printf("Speicherfehler! Inverse Kovarianzmatrix.\n");
        return NULL;
    
    }   
    else{
    
        // Kopiere den Inhalt der Kovarianzmatrix in die temporäre Matrix:
        for(idx=0; idx<rows; idx++){
    				
            for(jdx=0; jdx<cols; jdx++){
            					
	        matrix_temp[idx][jdx] = matrix[idx][jdx];
	    }
        } 
    
        
        // Setze die Diagonale der Inversen auf 1, sonst 0:
        for(idx=0; idx<rows; idx++){
    				
            for(jdx=0; jdx<cols; jdx++){								
	
	        if(idx == jdx){										
	            matrix_inv[idx][jdx] = 1;
	        }									
	        else{
	            matrix_inv[idx][jdx] = 0;
	        }
	    }
        }


        // 
        for(kdx=0 ;kdx<rows; kdx++){
    														
	    temp=matrix_temp[kdx][kdx];
											
            for(jdx=0; jdx<cols; jdx++){
        
	        matrix_temp[kdx][jdx] /= temp;
	        matrix_inv[kdx][jdx] /= temp;
	    }
														
	    for(idx=0; idx<rows; idx++){
	
	        temp=matrix_temp[idx][kdx];
	    									
	        for(jdx=0; jdx<cols; jdx++){
	    												
		    if(idx == kdx){
		        break;
		    }									
		    matrix_temp[idx][jdx] -= matrix_temp[kdx][jdx] * temp;
								
		    matrix_inv[idx][jdx] -= matrix_inv[kdx][jdx] * temp;
	        }
	    }
        }
        
        for(idx=0; idx<rows; idx++){
    				
            free(matrix_temp[idx]);
        }         
        free(matrix_temp);
        
        
        printf("ok\n");        
        return matrix_inv;
    }
}



// ##################################################################################################



int interpolate_raster(struct usr_map *Map){

    /*
        Berechnet für jeden Punkt des Kartenrasters der keinen Messwert besitzt 
        einen Interpolationswert.
        
        Rückgabewert:
        Erfolg: True "1"
        Fehler: False "0"
    */
    

    int idx, jdx, kdx, value_cnt=1;
    double *cov_vector, *weights_vector;
    double sum;
    
    cov_vector = create_fvector(Map->input_data.length+1);
    if (cov_vector == NULL){
    
        printf("Speicherfehler! Kovarianzvektor eines Rasterpunkts.\n");
        return false;
    
    }
    else{
    
        printf("Interpoliere ...         ");
        fflush(stdout);
        
        // Geh über jeden Punkt des Input-Rasters:
        for (idx=0; idx<(Map->rows); idx++){
            for (jdx=0; jdx<(Map->cols); jdx++){
        
                
                printf("\b\b\b\b\b\b\b\b\b");
                fflush(stdout);                   
                printf(" %5.1f %% ", ((value_cnt*1.0)/(Map->rows*Map->cols*1.0)*100.0));               
                fflush(stdout);  
                
                value_cnt++;
                
                // Liegt kein Messwert für den Rasterpunkt vor...
                if (Map->raster[idx][jdx].value < 0){
            
                    sum = 0;
            
                    // Berechne nun für diesen Punkt die Distanz zu jedem Messpunkt der Input-Daten.
                    for (kdx=0; kdx<Map->input_data.length+1; kdx++){
                
                        if (kdx != Map->input_data.length){
                            cov_vector[kdx] = calc_covariance(calc_distance(Map->raster[idx][jdx].lat,
                                                                            Map->raster[idx][jdx].lon, 
                                                                            Map->input_data.data[kdx].lat,
                                                                            Map->input_data.data[kdx].lon),
                                                                            Map->variogram.sill, 
                                                                            Map->variogram.nugget,
                                                                            Map->variogram.range);
                        }
                        else{
                            cov_vector[kdx] = 1;
                        }
                    }
                
                    weights_vector = multiplyMatrixVector(Map->covariance_matrix_inv, 
                                                          cov_vector, 
                                                          Map->input_data.length+1, 
                                                          Map->input_data.length+1);
                                                          
                   
                                                          
                    if (Map->weights_correction){
                    
                        correct_negative_weights(weights_vector, cov_vector, Map->input_data.length);
                    }
                    
                    // Wert des Rasterpunktes mit 0 initialisieren:
                    Map->raster[idx][jdx].value = 0;
                    
                    // Berechnen des Interpolationswertes, als die Summe über die Gewichte * Werte.
                    for (kdx=0; kdx<Map->input_data.length; kdx++){
                    
                        sum += (weights_vector[kdx] * Map->input_data.data[kdx].value);
                    }
                    
                    // Interpolierten Wert dem Rasterpunkt zuweisen:
                    Map->raster[idx][jdx].value = sum;
                    
                }
                else{
                    // Liegt ein Messwert vor, so geh zum nächsten Rasterpunkt
                    continue;
            
                }   
            }    
        }
        
        printf("ok\n");
        free(cov_vector);
        free(weights_vector);
        return true;
    }   

};



// ##################################################################################################



int correct_negative_weights(double *weights_vector, double *cov_vector, int length){

    /*
        Setzt negative Gewichtungen des Gewichtungsvektors auf 0.
        Führt anschließend eine Restandardisierung durch um die Summe aller pos. Gewichte 
        anschließend wieder auf 1 zu bringen.
        
        Rückgabewert:
        Erfolg: True "1"
    
    */



    int idx, cnt=0;
    double sum_weights=0, sum_cov=0;
    double absAvg_negWeights=0;				// absolten Durchschnittswertes aller negativen Gewichte
    double avgCov_negWeights=0;				// durchschnittl.
    

    // 1. Berechne den absoluten Durchschnittswert der negativen Abweichungen (absAvg_negWeights) über den "weights_vector"
    //    lam_avg_neg = sum(|-lam_0|, |-lam_1|, ... , |-lam_n|)/n
    
    // 2. Berechne die durschnittl. Kovarianz zwischen dem Interpolationspunkt und den Messpunkten mit neg. Gewichten:
    //    cov_avg_neg = sum(cov(u - ui))/n
    //    d.h. 2.1. Berechne die Distanz zwischen dem zu interpolierenden Punkt und den Messpunkten mit neg. Gewichten
    //         2.2. Berechne aus diesen Abständen die Kovarianz und bilde davon den Mittelwert 
    for (idx=0; idx<length; idx++){
    
        if (weights_vector[idx] < 0){
            
            // Bilde die Summe der absoluten neg. Gewichtungen:
            sum_weights += fabs(weights_vector[idx]);
            
            // Bilde die Summe der Kovarianzen bei neg. Gewichtung:
            sum_cov += cov_vector[idx];
            
            // Erhöhe den Counter für die Anzahl der neg. Gewichtungen
            cnt++;
        }
    }
    // Sind keine neg. Gewichte vorhanden, müss alle Gewichtungen größer sein als absAvg_negWeights, da...
    // absAvg_negWeights = 0 und und alle Gewichtung > 0    
    if (cnt == 0){
    
        return true;
    }
    // Berechne den absoluten Durchschnittswert der negativen Abweichungen nur, wenn min. 1 negativer Wert vorhanden war:
    else{
        
        absAvg_negWeights = (sum_weights/cnt);
        avgCov_negWeights = (sum_cov/cnt);
        
        // 3. Korrigiere die Gewichte nun folgendermaßen:
        //   1. neue Gewichtung = alte Gewichtung
        //   2. if alte Gewichtung < 0 dann neue Gewichtung = 0
        //   3. if (alte Gewichtung > 0) && 
        //         (Kovarianz zwischen zu interpolierenden Punkt und Messwert < cov_avg_neg) && 
        //         (alte Gewichtung < absAvg_negWeights) dann lam_new = 0       
        
        sum_weights = 0;
        
        for (idx=0; idx<length; idx++){
    
            // Ist die Gewichtung < 0, dann setzte es 0:
            if (weights_vector[idx] < 0){
                weights_vector[idx] = 0;
            }
            // Ist die Gewichtung > 0, aber ...
            else if ((weights_vector[idx] > 0) && 
                     (cov_vector[idx] < avgCov_negWeights) && 
                     (weights_vector[idx] < absAvg_negWeights)){
                
                weights_vector[idx] = 0;            
            }
            else{
                // Wird die Gewichtung nicht auf 0 gesetzt, so summiere den Wert auf:
                sum_weights += weights_vector[idx];
            }
        }
        
        // Ist die Summe der pos. Gewichtungen > 0, ...
        // sprich: Es wurden nicht alle Gewichtungen auf 0 gesetzt, dann ...
        if (sum_weights > 0){
    
            // ... Restandardisiere die Gewichte:
            for (idx=0; idx<length; idx++){
    
                // Es sollen nur die positiven Gewichtungen restandardisiert werden.
                // Der Rest bleibt 0:
                if (weights_vector[idx] > 0){

                    weights_vector[idx] = weights_vector[idx] / sum_weights;
                }
                else{
                    weights_vector[idx] = 0;
                }
            }
        }
        return true;
    }
}



// ##################################################################################################



double calc_distance(double latA, double lonA, double latB, double lonB){

    /*
        Berechnet die Entfernung zwischen 2 Punkten auf einer Kugelfläche mit einem Radius von 6365.265 km.
        Der Radius entspricht dem Erdradius auf 51° N.
        Die Input-Argumente muessen in Dezimalgrad vorliegen.
    */ 
    
    
    // Punkt A: 
    double latA_rad = (latA/180.0) * M_PI;
    double lonA_rad = (lonA/180.0) * M_PI;

    // Punkt B:
    double latB_rad = (latB/180.0) * M_PI;
    double lonB_rad = (lonB/180.0) * M_PI;

    return RADIUS_EARTH * acos( (sin(latA_rad) * sin(latB_rad) ) + ( cos(latA_rad) * cos(latB_rad) * ( cos(lonB_rad - lonA_rad) ) ) );

}



// ##################################################################################################



double calc_covariance(double distance, double sill, double nugget, double range){

    /*
        Berechnet die Kovarianz eines Punktes mittels gewähltem Variogrammmodell,
        der Distanz, sill, nugget und range.
        
        Rückgabewert:
        Erfolg: Kovarianz vom Datentyp "double"
    */
    
    double z, n;
    
    // Exponentielles Modell:
    // nugget + sill *(1-exp(-|distance|/(range/3)))
    
    n = range / 3.0;
    z = -1 * abs(distance);
    
    return nugget + sill * (1 - exp(z/n));


}



// ##################################################################################################



double *multiplyMatrixVector(double **matrix, double *vector_in, int rows, int cols){

    /*
        Multipliziert eine Matrize vom Datentyp "double" mit einem Vektor vom Datentyp "double".
        
        Rückgabewert:
        Erfolg: Zeiger vom Datentyp "double" auf das Feld [0][0] des Ergebnisvektors.
        Fehler: False
    */

    int idx, jdx, kdx;

    double *vector_out;
    double sum=0;
    
    // Reserviere den Ergebnisvektor:
    vector_out = create_fvector(rows);
    if (vector_out == NULL){
    
        printf("Speicherfehler!\n");
        return NULL;
    }
    else{
        
        // Multiplikation:
        for (idx=0; idx<rows; idx++){
                    
            vector_out[idx] = 0;
                
            for (jdx=0; jdx<cols; jdx++){
                
                vector_out[idx] += (matrix[idx][jdx] * vector_in[jdx]);
                    
            }
            
        }
        
        return vector_out;
    }
}



// ##################################################################################################



int multiplyMatrix(double **matrixA, double **matrixB, int rows, int cols){

    /*
        Multipliziert zwei Matrizen vom Typ "double" und der Dimensionen rows x cols miteinander.
        
        Rückgabewert:
        Erfolg: True "1"
        Fehler: False "0"
    */


    int idx, jdx, kdx;
    
    double **matrixP;
    double sum=0;
    
    
    
    matrixP = create_fmatrix(rows, cols);
    if (matrixP == NULL){
    
        printf("Speicherfehler!\n");
        return false;
    }
    else{
        
        // Multiplikation:
        for (idx=0; idx<rows; idx++){
        
            for (jdx=0; jdx<cols; jdx++){
            
                matrixP[idx][jdx] = 0;
                
                for (kdx=0; kdx<cols; kdx++){
                
                    matrixP[idx][jdx] += (matrixB[kdx][jdx] * matrixA[idx][kdx]);
                    
                }
            }
        }
                        
        
        //show_matrix(matrixP, 20, 20);
        
        for (idx=0; idx<rows; idx++){
        
            free(matrixP[idx]);
        
        }
        
        free(matrixP);
        return true;
    }   
}



double calc_RSME(double *values1, double *values2, int length){


    /*
        DESCRIPTION
        Calculates the rsme error between two datasets of type double.
        
        Input:
        values1		...	Datenreihe 1
        values2		...	Datenreihe 2
    */

    int idx;
    jmp_buf env;
    
    // ####################################### FUNCTION #############################################    
    double rsme(double *values1, double *values2, int length){    
    
    
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
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: return rsme(values1, values2, length);
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n >>> The length must be greater then 0!\n", __FILE__, __LINE__); return NAN;
        case 2: fprintf(stderr, "ERROR: %s --> %d:\n >>> The datasets contains NAN or INF values!\n", __FILE__, __LINE__); return NAN;        
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NAN;
    }
}

double get_fmax(double *values, int length){

    /*
        Return the maximum value of a given vector.
    */
    
    int idx;
    
    double maxValue = values[0];
    
    
    for (idx=0; idx<length; idx++){
    
        if (values[idx] > maxValue){
            
            maxValue = values[idx];
        }
    }
    
    return maxValue;
}


int get_max(int *values, int length){

    /*
        Bestimmt den Maximaumwert einer Datenreihe des Typ "integer".
    */
    
    int idx;
    
    int maxValue = values[0];
    
    
    for (idx=0; idx<length; idx++){
    
        if (values[idx] > maxValue){
            
            maxValue = values[idx];
        }
    }
    
    return maxValue;
}



double get_fmin(double *values, int length){

    /*
        Bestimmt den Maximaumwert einer Datenreihe.
    */
    
    int idx;
    
    double minValue = values[0];
    
    
    for (idx=0; idx<length; idx++){
    
        if (values[idx] < minValue){
        
            minValue = values[idx];
        }
    }
    return minValue;
    
}


int get_min(int *values, int length){

    /*
        Bestimmt den Maximaumwert einer Datenreihe.
    */
    
    int idx;
    
    int minValue = values[0];
    
    
    for (idx=0; idx<length; idx++){
    
        if (values[idx] < minValue){
        
            minValue = values[idx];
        }
    }
    return minValue;
    
}


void outputMatrixCSV(double **matrix, char *filename, int rows, int cols){

    /*
        Gibt eine Matrix vom Datentyp "double" als csv-Datei aus.
        
        Rückgabewert:
        Erfolg: True "1"
        Fehler: False "0"
    
    */

    int idx, jdx, kdx;
    char zahlText[20];
    FILE *fp;


    fp = fopen(filename,"w");
    if (fp == NULL){
    
        printf("Dateifehler! Matrix (output)\n");
    }
    else{
    
        for (idx=0; idx<rows; idx++){
        
            for (jdx=0; jdx<cols; jdx++){
            
                sprintf(zahlText,"%.8f", matrix[idx][jdx]);
                
                for (kdx=0; kdx<(int)strlen(zahlText); kdx++){
                
                    if (zahlText[kdx] == '.'){
                    
                        zahlText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp,"%s;",zahlText);
            }
            fprintf(fp,"%s","\n");
        }
        
        fclose(fp);
    }


    

}


int outputRasterCSV(struct usr_data_point **raster, char *output_dir, char *filename, int rows, int cols){

    /*
        Ausgabe:
        - CSV-Datei in Form eines "rows x cols" Rasters mit den interpolierten Werten.
        - CSV-Datei in Form eines "rows x cols" Rasters mit den Werten der geogr. Breite eines jeden Rasterpunktes.
        - CSV-Datei in Form eines "rows x cols" Rasters mit den Werten der geogr. Länge eines jeden Rasterpunktes.
        
        Rückgabewert:
        Erfolg: True "1"
        Fehler: False "0" 
        
    */



    int idx, jdx, kdx;
    char valueText[20], latText[20], lonText[20];
    char path[100];

    
    FILE *fp_values, *fp_lat, *fp_lon;


    // Filepointer für die Rasterwerte:
    fp_values = fopen(strcat(strcpy(path, output_dir), filename),"w");
    
    // Filepointer für die Breitengrade der Rasterpunkte
    fp_lat = fopen(strcat(strcpy(path, output_dir), "lat.csv"),"w");
       
    // Filepointer für die Längengrade der Rasterpunkte    
    fp_lon = fopen(strcat(strcpy(path, output_dir), "lon.csv"),"w");
    
    
    
    
    if ((fp_values == NULL) || (fp_lat == NULL) || (fp_lon == NULL)){
    
        printf("Dateifehler!\nDefinierter Filepointer ist NULL.\n");
        return false;
    }
    
    else{
    
        printf("Schreibe csv-Ausgabedateien ... ");
        fflush(stdout);
    
        for (idx=0; idx<rows; idx++){
        
            for (jdx=0; jdx<cols; jdx++){
            
                // Lese den Wert in ein Textfeld ein:
                sprintf(valueText,"%.3f", raster[idx][jdx].value);
                
                // Lese die geogr. Breite in ein Textfeld ein:
                sprintf(latText, "%.4f", raster[idx][jdx].lat);
                
                // Lese die geogr. Länge in ein Textfeld ein:
                sprintf(lonText, "%.4f", raster[idx][jdx].lon);
                
                
                
                
                // Schreibe die CSV-Datei der interpolierten Werte:
                for (kdx=0; kdx<(int)strlen(valueText); kdx++){
                
                    if (valueText[kdx] == '.'){
                    
                        valueText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp_values,"%s",valueText);

                // Schreibe die CSV-Datei der geogr. Breite:
                for (kdx=0; kdx<(int)strlen(latText); kdx++){
                
                    if (latText[kdx] == '.'){
                    
                        latText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp_lat,"%s",latText);

                // Schreibe die CSV-Datei der geogr. Breite:
                for (kdx=0; kdx<(int)strlen(lonText); kdx++){
                
                    if (lonText[kdx] == '.'){
                    
                        lonText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp_lon,"%s",lonText);
                
                if (jdx != (cols-1)){
                
                    fprintf(fp_values,"%s",";");
                    fprintf(fp_lat,"%s",";");    
                    fprintf(fp_lon,"%s",";");
                    
                }
                else{
                
                    fprintf(fp_values, "%s", "\n");
                    fprintf(fp_lat, "%s", "\n");    
                    fprintf(fp_lon, "%s", "\n");                
                
                }
            }
        }
        
        fclose(fp_values);
        fclose(fp_lat);
        fclose(fp_lon);
        
        printf("ok\n");
        return true;
    }
}



int get_output_information(struct usr_map *Map){

    /*
        Bestimmt und schreibt Metadaten zum interpolierten Raster in das "Map"-Objekt:
        
        Rückgabewert:
        Erfolg: True "1"
    */
    
    int idx, jdx;
    double sum=0;
    
    // Bestimmung des Maximum- und Minimumwerts.
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
    return true;
}



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
    
    // ####################################### FUNCTION #############################################
    void check(double **matrix, int rows, int cols, bool show_output){     
    
    
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
                else if (matrix[idx][jdx] > 0){
                
                    // count the pos. value
                    cnt_pos++;
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
        
    }
    // ##############################################################################################

    switch(setjmp(env)){
        case 0: check(matrix, rows, cols, show_output); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: %s --> %d:\n The matrix contains \"NAN\" or \"INF\" values!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
        default: fprintf(stderr, "ERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
    }
}


// ##################################################################################################
// ##################################################################################################

    
void show_variogram_data(struct usr_map *Map){

    /*
        DESCRIPTION:
        shows you a summary of the variogram data if the "show_output" attribut of the map object is set to true.
    */

    if (Map->show_output){

        int idx;
    
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
}




void show_map_info(struct usr_map *Map){

    /*
        Zeigt Informationen zum aktuellen Objekt.
    */

    printf("############### INFORMATION ##############\n\n");
    printf("   Geogr. Breite: %.3f (Minimum)\n",Map->minLat);
    printf("   Geogr. Breite: %.3f (Maximum)\n",Map->maxLat);   
    printf("   Geogr. Laenge: %.3f (Minimum)\n",Map->minLon);
    printf("   Geogr. Laenge: %.3f (Maximum)\n\n",Map->maxLon);    
    printf("\n");    
    printf("   Anzahl der Felder: %d\n", (Map->rows)*(Map->cols));
    printf("   Zeilen: %d\n", Map->rows);
    printf("   Spalten: %d\n", Map->cols); 
    printf("\n");    
    printf("   Aufloesung (Breite): %.3f\n", Map->latRes);
    printf("   Aufloesung (Laenge): %.3f\n", Map->lonRes); 
    printf("   Aufloesung (Breite, km): %.3f\n", Map->latMetRes);
    printf("   Aufloesung (Laenge, km): %.3f\n", Map->lonMetRes);  
    printf("\n");    
    printf("   --------------Inputdaten---------------\n\n");
    printf("   Anzahl der Stationswerte: %d\n", Map->input_data.length);
    printf("   Maximumwert: %.3f\n", Map->input_data.maximum);
    printf("   Minimumwert: %.3f\n", Map->input_data.minimum);
    printf("   Mittelwert: %.3f\n", Map->input_data.average);
    printf("\n");
    printf("   --------------Outputdaten--------------\n\n");
    printf("   Maximumwert: %.3f\n", Map->output_data.maximum);
    printf("   Minimumwert: %.3f\n", Map->output_data.minimum);
    printf("   Mittelwert: %.3f\n", Map->output_data.average);
    printf("\n");    
    printf("##########################################\n\n");          
}





void show_matrix(double **matrix, int rows, int cols){

    /*
        Gibt die übergebene Matrix auf stdout aus.    
    */

    int idx, jdx;

    for (idx=0; idx<rows; idx++){
    
        for (jdx=0; jdx<cols; jdx++){
        
            printf("%8.3f", matrix[idx][jdx]);   
        
        }
        
        printf("\n");
    }
    printf("\n");
}





void show_input_data(struct usr_map *Map){

     /*
        Gibt die Eingabedaten aus.    
    */   

    int idx;
    
    
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



void show_raster(struct usr_data_point **raster, int *rows, int *cols){

    /*
        Gibt ein Raster vom Datentyp "usr_data_point" in den Grenzen rows x cols aus.
    */

    int idx, kdx;
    
    for (idx=0; idx<*rows; idx++){
    
        for (kdx=0; kdx<*cols; kdx++){
        
            //geogr_laenge
            //printf("%.2f ", raster[idx][kdx].lon);
            
            //geogr_breite          
            //printf("%.2f ", raster[idx][kdx].lat);
            
            //werte          
            printf("%.0f ", raster[idx][kdx].value);           
        }
        printf("\n");
    
    }

}


void free_raster(struct usr_map *Map){

    int idx;
    
    
    // Leeren es Eingaberasters:
    for (idx=0; idx<(Map->rows); idx++){
    
        free(Map->raster[idx]);
    
    }
    
    free(Map->raster);
    printf("Eingaberaster: Speicher freigegeben!\n");
    
    
    //Leeren der Distanzmatrix:
    for (idx=0; idx<Map->input_data.length; idx++){
    
        free(Map->distance_matrix[idx]);
    }
    free(Map->distance_matrix);
    printf("Distanzmatrix: Speicher freigegeben!\n");   
    
    // Leeren der Kovarianzmatrix:
    for (idx=0; idx<Map->input_data.length+1; idx++){
    
        free(Map->covariance_matrix[idx]);
    }
    free(Map->covariance_matrix);
    printf("Kovarianzmatrix: Speicher freigegeben!\n");

     // Leeren der Inversen der Kovarianzmatrix:
    for (idx=0; idx<Map->input_data.length+1; idx++){
    
        free(Map->covariance_matrix_inv[idx]);
    }
    free(Map->covariance_matrix_inv);
    printf("Inverse der Kovarianzmatrix: Speicher freigegeben!\n");   
    
    return;
}



void free_vector(struct usr_map *Map){

    free(Map->input_data.data);
    printf("Eingabedaten: Speicher freigegeben!\n");
    
    
    free(Map->variogram.classes);
    printf("Variogrammklassen: Speicher freigegeben!\n");
    
    free(Map->variogram.reg_function.solution);
    printf("Variogramm -> Regressionsfunktion: Speicher freigegeben\n");
}



