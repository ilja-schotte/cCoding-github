
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <math.h>
    #include <stdbool.h>
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


int set_config(struct usr_map *Map, int argc, char **argv);					// performs some calculations regarding to the resolution
int create_maps_raster(struct usr_data_point ***raster, int rows, int cols);
int fill_raster_with_default_data(struct usr_map *Map);
int input_csv_data(struct usr_map *Map, char *input_datafile);
int show_input_data(struct usr_map *Map);
int fill_raster_with_input_data(struct usr_map *Map);
int interpolate_raster(struct usr_map *Map);

double calc_distance(double latA, double lonA, double latB, double lonB);			// calculates the distance between two points on a sphere.


void free_raster(struct usr_map *Map);
void free_vector(struct usr_map *Map);


// ##################################################################################################
// ##################################### Definition: Funktionen #####################################


int set_config(struct usr_map *Map, int argc, char **argv){

    /*
        DESCRIPTION:
        Performs some calculations to complete the intern configuration.
    
        INPUT:
        struct usr_map *Map	...	pointer to the "map" object of datatype "struct usr_map".
    
        OUTPUT:(error code)
        on success		...	EXIT_SUCCESS
        on failure		...	EXIT_FAILURE
       
    */

    int idx;
    int excno;
    jmp_buf env;
    double cols;
    
    // ####################################### FUNCTION #############################################
    if ((excno = setjmp(env)) == 0){
    
        // read the arguments:
        for (idx=0; idx<argc; idx++){
        
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
        
        return EXIT_SUCCESS;
    }
    else{
        switch(excno){
            case 1: fprintf(stderr, "ERROR: %s --> %d:\nThe number of rows must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\nThe number of maxLon and minLon must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 3: fprintf(stderr, "ERROR: %s --> %d:\nThe number of maxLat and minLat must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 4: fprintf(stderr, "ERROR: %s --> %d:\nThe calculated number of columns of the output raster is \"NAN\" or \"INF\"!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;        
            default: fprintf(stderr, "ERROR: %s --> %d:\nWoops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
        }
    }
}


// ##################################################################################################
// ##################################################################################################


int create_maps_raster(struct usr_data_point ***raster, int rows, int cols){

    /*
    
        DESCRIPTION:
        Initialize an raster of dimensions "Map->rows" x "Map->cols" of type struct usr_data_point.
        
        INPUT:
        struct usr_data_point ***rasters	...	pointer to the maps 2d raster
        int rows				...	number of rows
        int cols				...	number of cols
    
        OUTPUT:(error code)
        on success				...	EXIT_SUCCESS
        on failure				...	EXIT_FAILURE       
     
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
    
    // ##############################################################################################

    switch(excno){
        case 1: fprintf(stderr, "ERROR: %s --> %d:\nFailure during the attempt to allocate memory\n--> %s\n\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr, "ERROR: %s --> %d:\nWoops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
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
            case 1: fprintf(stderr, "ERROR: %s --> %d:\nThe number of rows must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 2: fprintf(stderr, "ERROR: %s --> %d:\nThe number of columns must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 3: fprintf(stderr, "ERROR: %s --> %d:\nThe resolution of longitude must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;
            case 4: fprintf(stderr, "ERROR: %s --> %d:\nThe resolution of latitude must be greater then 0!\n\n", __FILE__, __LINE__); return EXIT_FAILURE;                            
            default: fprintf(stderr, "ERROR: %s --> %d:\nWoops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;       
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
    int excno;

    FILE *fp;
    int rows = 0;
    int idx, jdx;
    
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
    else{
        // ##############################################################################################
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
    

    int idx, jdx, kdx, ldx;
    int excno;
    jmp_buf env;
    
    double dist_nenner, i_value, dist_zaehler, weight, value;
    
    if ((excno = setjmp(env)) == 0){   
    
        for (idx=0; idx<Map->rows; idx++){
        
            printf("%.6f\n", ((double)idx/(double)Map->rows)*100.0);
            
            for (jdx=0; jdx<Map->cols; jdx++){
            
                // interpolate if value of raster point is lower then 0:
                if (Map->raster[idx][jdx].value < 0){
                
                    Map->raster[idx][jdx].value = 0;
                    
                    // over all input data points
                    for (kdx=0; kdx<Map->input_data.length; kdx++){
                    
                        // distanz zu den messpunkten
                        dist_zaehler = 1/pow(calc_distance(Map->raster[idx][jdx].lat, Map->raster[idx][jdx].lon, 
                                                           Map->input_data.data[kdx].lat, Map->input_data.data[kdx].lon), 2);
                        
                        dist_nenner = 0;                  
                        for (ldx=0; ldx<Map->input_data.length; ldx++){
                            dist_nenner += 1/pow(calc_distance(Map->raster[idx][jdx].lat, Map->raster[idx][jdx].lon, 
                                                               Map->input_data.data[kdx].lat, Map->input_data.data[kdx].lon), 2);    
                        }
                        
                        Map->raster[idx][jdx].value += (dist_zaehler / dist_nenner) * Map->input_data.data[kdx].value;
                    }
                }
                else{
                    continue;
                }
            }
        }
        return EXIT_SUCCESS;
    }
    else{
        // ##############################################################################################
        switch(excno){
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n >>> %s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;                              
            default: fprintf(stderr, "\nERROR: %s --> %d:\n Woops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return EXIT_FAILURE;
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
            case 1: fprintf(stderr, "\nERROR: %s --> %d:\n>>> One of the coordinates is equal to \"NAN\" or \"INF\"\n", __FILE__, __LINE__); return NAN;                       
            case 2: fprintf(stderr, "\nERROR: %s --> %d:\n>>> %s\n", __FILE__, __LINE__, strerror(errno)); return NAN;               
            default: fprintf(stderr, "\nERROR: %s --> %d:\nWoops! Somethings nasty has happend!\n%s\n", __FILE__, __LINE__, strerror(errno)); return NAN;
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
}



