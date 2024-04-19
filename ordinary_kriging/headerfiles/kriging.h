
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
double **create_covariance_matrix(struct usr_map *zgr, double **matrix, int rows, int cols);
double **inverseOfMatrix(double **matrix, int rows, int cols);
int create_distance_matrix(struct usr_map *Map);
double **create_fmatrix(int rows, int cols);
double *create_fvector(int length);

               
void show_raster(struct usr_data_point **zgr, int *rows, int *cols);
void show_map_info(struct usr_map *zgr);
void show_input_data(struct usr_map *zgr);
void show_variogram_data(struct usr_map *zgr);
void show_matrix(double **matrix, int rows, int cols);
void check_matrix(double **matrix, int rows, int cols);
void outputMatrixCSV(double **matrix, char *filename, int rows, int cols);
void find_model_adjust_index(struct usr_map *zgr, double *variogram_variances, int length);

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



double *create_fvector(int length){

    /*
        Erstellt einen Vektor vom Datentyp "double" der Laenge "length".
        
        Rückgabewert: 
        Erfolg: Ein Zeiger des Datentyps "double" auf das Feld [0] des Vektors.
        Fehler: NULL.       
    */

    int idx;
    double *zgr;
    
    
    zgr = (double *) calloc(length, sizeof(double));
    if (zgr == NULL){
    
        printf("Speicherfehler!\n");
        printf("Beim Reservieren eines Vektors von Datentyp: \"double\" ist ein Fehler aufgetreten!\n");
        printf("Funktion: *create_fvector\n");
        printf("Rueckgabe: NULL\n");
                  
        return NULL;
    }
    else{
        return zgr;
    }   
}



// ##################################################################################################



int *create_vector(int length){

    /*
        Erstellt einen Vektor vom Datentyp "integer" der Laenge "length".
        
        Rückgabewert: 
        Erfolg: Ein Zeiger des Datentyps "integer" auf das Feld [0] des Vektors.
        Fehler: NULL.       
    */

    int idx;
    int *zgr;
    
    
    zgr = (int *) calloc(length, sizeof(int));
    if (zgr == NULL){
    
        printf("Speicherfehler!\n");
        printf("Beim Reservieren eines Vektors von Datentyp: \"double\" ist ein Fehler aufgetreten!\n");
        printf("Funktion: *create_vector\n");
        printf("Rueckgabe: NULL\n"); 
                  
        return NULL;
    }
    else{
        return zgr;
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
        1. Definiert die Abstandsklassen:
        -> Maximaler Abstand zweier Punkte in Deutschland 900 km.
        -> Einteilung in 50 km Klassen -> 18 Klassen.
        
        2. Berechnung der mittleren Distanz aller Punkte einer Abstandsklasse.
        3. Berechnung der mittleren Varianz aller Punkte einer Abstandsklasse.
        
        Rückgabewert:
        Erfolg: True "1"
        Fehler: False "0"
        
    */
    
    int idx, kdx, ndx;
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
    
    printf("Erstelle Variogramm ... ");
    fflush(stdout);
    
    // Abstandsklassen erstellen (max. Abstand / Intervallgröße):
    Map->variogram.numClasses = (int)(Map->variogram.maxDistance / Map->variogram.distInterval);

    // Reservieren eines Vektors des Datentyps "usr_vario_class" in die Informationen der Variogrammklassen eingetragen werden sollen.
    Map->variogram.classes = (struct usr_vario_class *) malloc(Map->variogram.numClasses * sizeof(struct usr_vario_class));
    if (Map->variogram.classes == NULL){
    
        printf("Speicherfehler: Variogram\n");
        return false;
    
    }
    else{
        // Attribute der Abstandsklassen erstellen (lowerLimit / upperLimit):
        for (idx=1; idx<=(Map->variogram.numClasses); idx++){
        
            Map->variogram.classes[idx-1].lowerLimit = (int)((idx-1)*Map->variogram.distInterval);
            Map->variogram.classes[idx-1].upperLimit = (int)((idx)*Map->variogram.distInterval);
            Map->variogram.classes[idx-1].num_variance_values = 0;
            Map->variogram.classes[idx-1].num_distance_values = 0;
            Map->variogram.classes[idx-1].variance_avg = 0;
            Map->variogram.classes[idx-1].distance_avg = 0;
        
        }
        
        // Berechnen der mittleren Semivarianz für jede Abstandsklasse über alle Punkte:
        // Nimm eine Station aus dem Input-Datensatz:
        for (idx=0; idx<Map->input_data.length; idx++){
                        
            
            // Prüfe nun über die Abstandsklassen, welche Stationen dazugehören und bilde davon die Semivarianz
            // Nimm eine Abstandsklasse:
            for (kdx=0; kdx<Map->variogram.numClasses; kdx++){
            
                // Setze den Zwischenwert, Zähler und den Wert der Semivarianz der Abstandsklasse auf 0;
                sumPowValue = 0;	// Summe der quadratischen Abweichungen
                numValue = 0;		// Anzahl der quadratischen Abweichungen
                semiVarianz = 0;	// Semivarianz der Abstandsklasse für diese Station
                sumDistance = 0;	// Summe der Distanzen 
            
                // Vergleiche den Messwert dieser Station mit jedem anderen:
                // Nimm nun eine Vergleichsstation aus dem Input-Datensatz:
                for (ndx=0; ndx<Map->input_data.length; ndx++){
                
                    // Der Messwert einer Station, soll nicht mit sich selbst verglichen werden:
                    if (idx != ndx){
                
                        // Berechne nun den Abstand zwischen der Station und der Vergleichsstation:
                        distance = calc_distance(Map->input_data.data[idx].lat, 
                                                 Map->input_data.data[idx].lon, 
                                                 Map->input_data.data[ndx].lat,
                                                 Map->input_data.data[ndx].lon);
                        
                        // Liegt die Distanz der beiden Stationen in der aktuell ausgewählten Abstandsklasse, ...
                        // ... so füge sie der Berechnung der Semivarianz hinzu:
                        if ((Map->variogram.classes[kdx].lowerLimit < distance) && (distance <= Map->variogram.classes[kdx].upperLimit)){
                        
                            sumPowValue += pow((Map->input_data.data[idx].value - Map->input_data.data[ndx].value),2);
                            
                            sumDistance += distance;
                            
                            numValue ++;
                            continue;
                            
                        }
                        else{
                        // Liegt die Distanz der beiden Stationen nicht in der aktuell ausgewählten Abstandsklasse, ...
                        // so nimm die nächste Station:
                            continue;
                        }
                
                    }
                    else{
                    // Handelt es sich um die gleiche Station, so nimm die nächste Station:
                        continue;
                    }
                }
                
                // Wurden alle Vergleichsstationen durchlaufen, so berechne den Semivarianzwert der Abstanzklasse und füge ihn der Abstandsklasse hinzu, wenn er größer 0 ist:
                // 1. Prüfe zuerste, ob der Wert der Summe der quatratischen Abweichungen > 0 ist, um einer Division durch 0 zuvor zu kommen: 
                if (sumPowValue > 0){
                
                    // 2. Berechne die Semivarianz:
                    semiVarianz = (double)(sumPowValue / (2*numValue));
                    
                    // 3. Berechne die durchschnittl. Distanz dieser Klasse für diese Station
                    avgDistance = (double)sumDistance / numValue;
                    
                    if ((semiVarianz > 0) && (avgDistance > 0)){
                
                        // Füge den Semivarianzwert der Klasse hinzu:
                        Map->variogram.classes[kdx].variance_values[Map->variogram.classes[kdx].num_variance_values++] = semiVarianz;
                        
                        // Füge die durchschnittl. Distanz dieser Klasse für diese Station hinzu: 
                        Map->variogram.classes[kdx].distance_values[Map->variogram.classes[kdx].num_distance_values++] = avgDistance;                      
                    }
                    else{
                        continue;
                    }
                }
                // Weiter mit der nächsten Abstandsklasse:
                continue; 
            }
        }
                    
        // Berechne die Mittelwerte der Semivarianzen und Distanzen einer jeden Abstandsklasse:
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
            sumClassVarianz = 0.0;
            sumClassDistance = 0.0;
        
            // Besitzt die Abstandsklasse mindestenz eine Semivarianz bzw. Durchschnittsdistanz:
            if ((Map->variogram.classes[idx].num_variance_values > 0) && (Map->variogram.classes[idx].num_variance_values > 0)){
        
                for (kdx=0; kdx<Map->variogram.classes[idx].num_variance_values; kdx++){
            
                    sumClassVarianz += Map->variogram.classes[idx].variance_values[kdx];
                    sumClassDistance += Map->variogram.classes[idx].distance_values[kdx];
                }
                
                Map->variogram.classes[idx].variance_avg = sumClassVarianz / (double)(Map->variogram.classes[idx].num_variance_values);
                Map->variogram.classes[idx].distance_avg = sumClassDistance / (double)(Map->variogram.classes[idx].num_distance_values);
            }
            // Sonst setze den Mittelwert der Semivarianz auf 0.0:  
            else{
            
                Map->variogram.classes[kdx].variance_avg = sumClassVarianz;
                Map->variogram.classes[kdx].distance_avg = sumClassDistance;                
                continue;
            }
        }
        
        printf("ok.\n");
           
        return true;
        
    }
}



// ##################################################################################################



int get_variogram_model(struct usr_map *Map){

    /*
        Findet anhand des Variograms das dazu passende Modell:
        
        1: Finde eine Polynomfunktion 4. Grades, die den Verlauf der Semivarianzwerte über die lags gut wiedergibt.
        2: Finde mit Hilfe dieser Polynomfunktion einen Wert für:
           - sill
           - range
       
        Bereite die ermittelten Werte für:
           - distance_avg (x-Werte)
           - variance_avg (y-Werte)
        so auf, dass eine polynominale Rekurssion durchgeführt werden kann:
    */
    
    int idx, jdx;
    int cnt_neg_slope;
    int cnt_raising_slope;
    int itemp;
    int *density_variances;  
      
    double sill, range, RSME, interval;		//
    double RSME_min=-1;				// Minimumwert des RSME
    double y_max;				// Maximumwert der Vorhersagewerte der Semivarianzen
    double **input_data_poly_reg;		// Doppelzeiger auf Matrix mit Eingabedaten.
    double *diff_variance_avg_reg;		// abgeleitete Werte der Vorhersagewerte der Semivarianz mittels polyn. Regression
    double *calc_variances;			// berechnete Werte der Semivarainz pro lag mit Hilfe der exponentiellen Funktion
    double *variogram_variances;		// Semivarianzwerte der lags des Variogramms bis zum gesuchten Vergleichsindex.
		
    
    printf("Bestimme das Variogrammmodell ... ");
    
    // Reserviere Speicher für die Eingabedaten:
    input_data_poly_reg = create_fmatrix(Map->variogram.numClasses, 2);
    
    // Reserviere Speicher für die Varianzwerte des Variogramms:
    variogram_variances = create_fvector(Map->variogram.numClasses);
    

    // Bereite die ermittelten Werte für:
    // - distance_avg (x-Werte)
    // - variance_avg (y-Werte)
    // so auf, dass eine polynominale Rekurssion durchgeführt werden kann:
    // Übertrage die Eingabedaten in ein Array:
    for (idx=0; idx<Map->variogram.numClasses; idx++){
        
        // Übertrage x-Werte:
        input_data_poly_reg[idx][0] = Map->variogram.classes[idx].distance_avg;
        
        // Übertrage y-Werte:
        input_data_poly_reg[idx][1] = Map->variogram.classes[idx].variance_avg; 
        
        // Übertrage y-Werte in Vektor:
        variogram_variances[idx] = Map->variogram.classes[idx].variance_avg; 
    }
    


    // Berechne nun die polynimiale Refgressionsfunktion 4. Grades:
    // liefert bei Erfolg einen Zeiger auf die Gewichte der Regressionsfunktion:
    Map->variogram.reg_function.solution = polynomial_regression(input_data_poly_reg, 
                                                                 Map->variogram.numClasses, 
                                                                 Map->variogram.reg_function.order);
                                                                 
                                                                 
                                                                
    if (Map->variogram.reg_function.solution == NULL){
        return false;
    }
    else{
        
        // Berechne mit Hilfe der Regressionsfunktion die Vorhersagewerte der Semivarianz ...
        // ... über die Durchschnittsentfernungen der Semivarianzklassen des Variogramms.
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
            // Übergib den Startwert / Schnittpunkt mit y-Achse (b0)
            Map->variogram.classes[idx].variance_avg_reg = Map->variogram.reg_function.solution[0];
            
            // Summiere dann über die Potenzen auf:
            for (jdx=1; jdx<=Map->variogram.reg_function.order; jdx++){
            
                Map->variogram.classes[idx].variance_avg_reg += (Map->variogram.reg_function.solution[jdx] * pow(Map->variogram.classes[idx].distance_avg, jdx));

            }
        }
        

        // Finde den Bereich (Index) des Semivariogramms, an den die exponentielle Funktion angepasst werden soll.
        find_model_adjust_index(Map, variogram_variances, Map->variogram.numClasses);

        // Reserviere Speicher für die berechneten Werte der Semivarianz mit Hilfe des exponentiellen Modells:
        calc_variances = create_fvector(Map->variogram.model_adjust_index);

        // Ziel ist es nun, den sill und die range für ein exponentielles Modell zu finden.
        // Dies geschieht, indem so lange der sill und die range verändert wird, bis der RSME minimal wird.
        // Bestimme nun über diese Werte den RSME_min:
        for (sill=0; sill<100; sill++){
        
            for (range=0; range<=Map->variogram.classes[Map->variogram.model_adjust_index].distance_avg; range+=0.1){
            
                // Berechne die Semivarianz mit Hilfe des exponentiellen Modells:
                for (idx=0; idx<Map->variogram.model_adjust_index; idx++){
                
                    calc_variances[idx] = calc_covariance(Map->variogram.classes[idx].distance_avg, 
                                                          sill, 
                                                          Map->variogram.nugget, 
                                                          range);
                }
                                                     
                RSME = calc_RSME(calc_variances, variogram_variances, Map->variogram.model_adjust_index);                                 
                    
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
        
        
        // Übergib dem Modell den Wert des nuggets:
        // nugget negativ: -> 0.001:
        // sonst: -> b0, Regressionsfunktion
        if (Map->variogram.reg_function.solution[0] < 0){
            Map->variogram.nugget = 0.001;
        }
        else{
            Map->variogram.nugget = Map->variogram.reg_function.solution[0];
        }
        
        
        // Speicher der Eingabedaten freigeben:
        for (idx=0; idx<Map->variogram.numClasses; idx++){
            free(input_data_poly_reg[idx]);
        }

        free(input_data_poly_reg);    
        free(calc_variances);    
        free(variogram_variances);
    
        printf("ok\n");
        return true;
    }
}



// ##################################################################################################



void find_model_adjust_index(struct usr_map *Map, double *variogram_variances, int length){

    /*
        Findet den (lag)-Index des Variogramms bis zu dem das exponentielle Modell angepasst werden soll.
        
        Hintergrund:
        Im Variogramm ist jedem lag, eine mittlere Distanz und Semivarianz zugeordnet.
        Der Verlauf entspricht im Groben einem exponetiellen Verlauf der Form:
        
        nugget + sill * (1-exp(-|distance|/a)), mit a = range/3;
        
        Es ist nun erforderlich den Index des lags zu finden, bis zu dem dieses Modell angepasst werden soll.
        Die Semivarianzwerte des Variogramms zeigen gerade bei hohen Distanzen Extremwerte der Semivarianz, die
        bei einer Anpassung des Modells nicht berücksichtigt werden sollen.
        
    */

    int idx, jdx, imax=0;
    int *density_variances;
    double y_max, interval;
    
    
    // Vektor für die Verteilung der Semivarianzen in den einzelnen Intervallen:
    density_variances = create_vector(Map->variogram.numClasses);
    
    // Liefert den Maximumwert der Semivarianzen des Variogramms:
    y_max = get_fmax(variogram_variances, Map->variogram.numClasses);
    
    // Berechne die Intervallgröße mit dem Maximalwert der Semivarianz und der Anzahl der lags.
    interval = y_max/Map->variogram.numClasses;
       
    // Bestimme nun, wie viele Semivarianzen den entsprechenden Intervallen zuzuordnen sind und ...
    // ... ordne sie dem Vektor "density_variances" zu.
    for (idx=0; idx<Map->variogram.numClasses; idx++){
        
        for (jdx=0; jdx<Map->variogram.numClasses-1; jdx++){
            
            if ((variogram_variances[idx] > jdx*interval) && (variogram_variances[idx] < (jdx+1)*interval)){
                
                density_variances[jdx]++;
            }
        }
    }
        
    // Prüfe nun, an welchem Index des Density_vektors der Maximumwert auftritt.
    // Sollte er mehrfach auftreten, so nimm das letzte Auftreten;
    for (idx=0; idx<Map->variogram.numClasses; idx++){
        
        if (density_variances[idx] == get_max(density_variances, Map->variogram.numClasses)){
                
            imax = idx;
        }
    }

    // Bilde nun die Summe über die Anzahl der Semivarianzwerte bis zum Interval mit den meisten Werten.
    for (idx=0; idx<=imax; idx++){
        
        Map->variogram.model_adjust_index += density_variances[idx];
        
    }


    free(density_variances);
}



// ##################################################################################################



double **create_covariance_matrix(struct usr_map *Map, double **matrix, int rows, int cols){

    /*
        Berechnet anhand der Distanz-Matrix und dem definierten Semivarianzmodell
        die Kovarianzmatrix der Dimensionen rows x cols.
        
        Rückgabewert:
        Erfolg: Ein Doppelzeiger vom Datentyp "double" auf das Feld [0][0] der Kovarianzmatrix mit den Dimensionen rows x cols.
        Fehler: NULL
        
    */

    int idx, kdx;
    double **covariance_matrix;



    printf("Berechne die Kovarianzmatrix ... ");
    fflush(stdout);

    // Reservieren der Kovarianzmatrix:   
    covariance_matrix = create_fmatrix(rows, cols);
    
    if (covariance_matrix == NULL){
    
        printf("Speicherfehler: Kovarianzmatrix\n");
        return NULL;
    
    }
    else{
        // Berechne die Kovarianzen:
        for (idx=0; idx<rows; idx++){
                
            for (kdx=0; kdx<cols; kdx++){
                // Ist der zu berechnende Wert gleich der letzten Spalte und der letzten Zeile,
                // ... so setze ihn auf 0:                

                if ((idx == (rows-1)) && (kdx == (cols-1))){
                
                    covariance_matrix[idx][kdx] = 0;
                    continue;
                    
                }
                // Ist der zu berechnende Wert gleich der letzten Spalte, oder der letzten Zeile,
                // ... so setze ihn auf 1:                
                if ((idx == (rows-1)) || (kdx == (cols-1))){
                
                    covariance_matrix[idx][kdx] = 1;
                    continue;
                }
                else{
                    // Ist die Distanz 0 , dann entspricht die Kovarianz dem nugget-Wert:
                    if (matrix[idx][kdx] < EPS){
                        
                        covariance_matrix[idx][kdx] = Map->variogram.nugget;
                    }
                    else{
                        
                        covariance_matrix[idx][kdx] = calc_covariance(matrix[idx][kdx], 
                                                                      Map->variogram.sill, 
                                                                      Map->variogram.nugget,
                                                                      Map->variogram.range);
                        
                    }
                }
            }
        }
         
        printf("ok!\n");
        return covariance_matrix;
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
        Berechnet den RSME zwischen zwei Datenreihen über die Länge "length".
        
        Input:
        values1		...	Datenreihe 1
        values2		...	Datenreihe 2
    */

    int idx;
    
    double qsum=0;		// Summe der quadrierten Abweichungen
    
    
    for (idx=0; idx<=length; idx++){
    
        qsum += pow((values1[idx] - values2[idx]),2);
    
    }
    
    return sqrt(qsum/length);

}

double get_fmax(double *values, int length){

    /*
        Bestimmt den Maximaumwert einer Datenreihe des Typ "double".
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



void check_matrix(double **matrix, int rows, int cols){

    /*
        Prüft Matrizen auf nan-Werte und liefert eine Übersicht der Anzahl von pos. wie neg. Werten.
    */
    
    int idx, jdx;
    
    int cnt_nan = 0;
    int cnt_neg = 0;
    int cnt_pos = 0;
    
    double max, min;
    
    
    printf("Prüfe Matrix:\n");
    printf("-----------------------\n");
    
    for (idx=0; idx<rows; idx++){
    
        for (jdx=0; jdx<cols; jdx++){
        
            if (isnan(matrix[idx][jdx])){
            
                cnt_nan++;
                continue;
            }
            else if (matrix[idx][jdx] < 0){
            
                cnt_neg++;
                continue;
            }
            else if (matrix[idx][jdx] > 0){
            
                cnt_pos++;
                continue;
            }
        }
    }
    

    printf("Zusammenfassung:\n");
    printf("----------------\n");
    max=matrix[0][0];
    min=matrix[0][0]; 
       
    for (idx=0; idx<rows; idx++){
    
        for (jdx=0; jdx<cols; jdx++){
        
            if (matrix[idx][jdx] > max){
            
                max=matrix[idx][jdx];
            }
            if (matrix[idx][jdx] < min){
            
                min=matrix[idx][jdx];
            }
        }
    }    
    printf("Maximum: %.6f\n", max);
    printf("Minimum: %.6f\n", min);
    
    printf("Anzahl nan: %d\n", cnt_nan);
    printf("Anzahl > 0: %d\n", cnt_pos);    
    printf("Anzahl < 0: %d\n", cnt_neg);  
      
    printf("-----------------------\n\n");
}



    
void show_variogram_data(struct usr_map *Map){

    /*
        Liefert eine Ausgabe des Variogramms.
    */

    int idx;
    printf("########################################################################################\n");    
    printf("#################################### SEMIVARIOGRAMM ####################################\n\n");
   
    printf("--------------------------------------\n");
    printf("Lösungen der Regressionsfunktion:\n");
    printf("Grad:%3d\n", Map->variogram.reg_function.order);
    for (idx=0; idx<Map->variogram.reg_function.order+1; idx++){
        printf("b%d: %11.6f\n", idx, Map->variogram.reg_function.solution[idx]);
    }
    printf("R²: %8.3f\n", Map->variogram.reg_function.solution[Map->variogram.reg_function.order+1]);    
    printf("--------------------------------------\n\n");
        
    printf("Abstandsklassen:\n");
    printf("----------------------------------------------------------------------------------------\n");    
    printf("%3s%10s%10s%15s%15s%15s\n","id", "u.Grenze", "o.Grenze", "Distanz (AVG)", "Varianz (AVG)", "Varianz (REG)");
    for (idx=0; idx<Map->variogram.numClasses; idx++){
    
        printf("%3d%10d%10d%15.3f%15.3f%15.3f\n", idx,
                                      Map->variogram.classes[idx].lowerLimit,
                                      Map->variogram.classes[idx].upperLimit,
                                      Map->variogram.classes[idx].distance_avg,
                                      Map->variogram.classes[idx].variance_avg,
                                      Map->variogram.classes[idx].variance_avg_reg);
        
    }
    printf("----------------------------------------------------------------------------------------\n\n");
  
    printf("--------------------------------------\n");
    printf("Modell - Anpassungsindex: %d\n\n", Map->variogram.model_adjust_index);
    printf("Lösung des Semivarianzmodells:\n");    
    printf("nugget: %.3f\n", Map->variogram.nugget);
    printf("sill: %.3f\n", Map->variogram.sill);
    printf("range: %.3f\n", Map->variogram.range);
    printf("--------------------------------------\n\n");
    printf("########################################################################################\n");
    printf("########################################################################################\n\n");        
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



