#ifdef __unix__
    #include <stdio.h>
    #include <setjmp.h>
    #include <stdlib.h>
    #include <string.h>
    #include <errno.h>
    #include <limits.h>
#endif 


/* ##########################################################################################################
   Author:	Schotte, Ilja
   
   SHORT DESCRIPTION:
   Opens csv files downloaded from https://opendata.dwd.de/weather/weather_reports/poi/.
   Reads all wanted meteorologic parameters and copies these information into a private struct. 
*/

struct file{

    FILE *filePointer;			// file pointer to read the file
    unsigned short numRows;		// Number of rows of the CSV file.
    unsigned short numCols;		// Number of columns of the CSV file.
    char **rawRows;			// Matrix filled with the raw content of each rows of the CSV file.
    char headerParameterRow;		// Row of the parameters your looking for.
    char firstDataRow;			// First row of hourly data.

};


struct MeasuredData{

    struct file fileContent;		// Content of the CSV file.
    char ***rawDataMatrix;		// matrix filled with all data from the csv file.
    char ***formatedDataMatrix;		// Matrix filled with the formated data from the CSV file.
    
};

 
struct DWDWeatherReportPoi{

    char filetype[100];			// datatype (format) Examples: ".csv", ".nc", ...
    char filename[100];			// filename of the file you want to read
    char path[100]; 			// path where you can find that file
    char stationId[10];			// id of the corresponding station
    unsigned char number_parameters;	// number of all parameters you want to import from the CSV file.
    char *wanted_parameters[30][2];	// array of wanted parameters of the csv file

    struct MeasuredData groundData; 
};






int read_main_arguments(int argc, char *argv[], struct DWDWeatherReportPoi *Dataset);	// Checks the number of passed arguments and reads the current filename depending on the passed filetype.	
int open_CSVfile(struct DWDWeatherReportPoi *Dataset);					// Opens the file under the given path and filename and passes a filepointer to the dataset object.
int close_CSVfile(struct DWDWeatherReportPoi *Dataset);					// Closes the file by closing the corresponding filepointer to this file.
int allocate_cmatrix(char **ptr, int rows, int cols);					// allocates a rows x cols matrix of datatype char.
int show_dataMatrix(struct DWDWeatherReportPoi *Dataset);				// shows the dataMatrix with the observation data imported from the csv file.


// ###############################################################################################################################################################################
// ###############################################################################################################################################################################
// ############################################################################# functions #######################################################################################


int read_main_arguments(int argc, char *argv[], struct DWDWeatherReportPoi *Dataset){

    /*
        DESCRIPTION:
        1. Checks the number and values of the given arguments.
        2. Reads the first valid filename of the passed arguments and passes it to the dataset object.
    
        INPUT:
        int argc				...	number of given arguments
        char *char[]				...	values of the given arguments
        struct DWDWeatherReportPoi *Dataset	...	corresponding dataset to the file.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - checks if the number of given arguments is greater than two.
        - checks if a valid filename is under the given arguments.
    */
    
    int idx;
    jmp_buf env;
    int filetypeLength = (int)strlen(Dataset->filetype);
    int argumentLength = 0;
    
    
    // ########################################### functions ###########################################  
    void read_arguments(int argc, char *argv[], struct DWDWeatherReportPoi *Dataset){
    
        char *lastChars;
    
        // check for number of arguments:
        if (argc > 1){
        
            for (idx=0; idx<argc; idx++){
                
                // get argument length:
                argumentLength = (int)strlen(argv[idx]);
                
                if (argumentLength > filetypeLength){
                    
                    // get last chars of argument:
                    lastChars = &argv[idx][argumentLength-filetypeLength];
                    
                    // check if the last chars of arguemnt corresponds to user defined filetype:
                    if (strcmp(lastChars, Dataset->filetype) == 0){
                        strcpy(Dataset->filename, argv[idx]);
                        break;
                    }
                    else{
                        // last chars of argument doesnt correspond to user defined filetype.
                        continue;
                    }
                }
                else{
                    // Length of argument ist to short.
                    continue;
                }
            }
            if ((int)strlen(Dataset->filename) == 0){
                // No appropriate filename found.
                longjmp(env, 1);
            }
        }
        else{
            // too few arguments.
            longjmp(env, 1);
        }
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: read_arguments(argc, argv, Dataset); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: There must be at least one valid filename argument for main().\n"); return EXIT_FAILURE;       
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// #####################################################################################################
// #####################################################################################################


int open_CSVfile(struct DWDWeatherReportPoi *Dataset){

    /*
        DESCRIPTION:
        Trys to open the given file under passed filepath and filename and passes the filepointer to the dataset object.
    
        INPUT:
        struct DWDWeatherReportPoi *Dataset	...	pointer to the corresponding dataset.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - 
    */

    jmp_buf env;
    char file_tmp[100];
    
    // ########################################### functions ###########################################     
    void open_file(struct DWDWeatherReportPoi *Dataset){
    
        // copy the path of the file to the temporary path variable.
        strcpy(file_tmp, Dataset->path);
    
        Dataset->groundData.fileContent.filePointer = fopen(strcat(file_tmp, Dataset->filename), "r");
        
        if (Dataset->groundData.fileContent.filePointer == NULL){
            longjmp(env, 1);
        }
        else{
            fprintf(stdout, ">> File opened: %s\n",file_tmp);
        }
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: open_file(Dataset); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: (%s -> %s)\n>>> %s\n", __FILE__, __func__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// #####################################################################################################
// #####################################################################################################


int read_CSVdata(struct DWDWeatherReportPoi *Dataset){

    /*
        DESCRIPTION:
        Reads the csv file and copies the wanted data into the DWDWeatherReportPoi.MeasuredData object.
    
        INPUT:
        struct DWDWeatherReportPoi *Dataset	...	pointer to the corresponding dataset.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - 
    */
    
    jmp_buf env;
    int idx, jdx;
    
    // ########################################### functions ###########################################          
    void read_data(struct DWDWeatherReportPoi *Dataset){
    
        int row_length = (int)USHRT_MAX;	// USHRT_MAX = 65535
        int cols_cnt = 0; 
        char row[row_length];
        char *token_tmp;			// temporary token for temporary usecases.
        char string_tmp[row_length];

        
        // get number of rows of the CSV file.
        while (fgets(row, row_length, Dataset->groundData.fileContent.filePointer)){
            Dataset->groundData.fileContent.numRows++;
        }
        // Number of rows shouldnt be 0 or less;
        if ((Dataset->groundData.fileContent.numRows) == 0){
            longjmp(env, 1);
        }
        
        
        // Allocate memory for the raw unformated rows strings of the CSV file:
        Dataset->groundData.fileContent.rawRows = (char**) malloc(Dataset->groundData.fileContent.numRows * sizeof(char*));
        if (Dataset->groundData.fileContent.rawRows == NULL){
            longjmp(env, 2);
        }
        else{
            for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
                Dataset->groundData.fileContent.rawRows[idx] = (char*) malloc(row_length * sizeof(char));
                if (Dataset->groundData.fileContent.rawRows[idx] == NULL){
                    longjmp(env, 2);
                }
            }
        }
        
        // Set file pointer to the start of the csv file if it is not already.
        if (ftell(Dataset->groundData.fileContent.filePointer) != 0){
            fseek(Dataset->groundData.fileContent.filePointer,0,SEEK_SET);
        }
        
        // copy row content of the CSV file to the allocated memory.
        for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
        
            if (fgets(Dataset->groundData.fileContent.rawRows[idx], row_length, Dataset->groundData.fileContent.filePointer) != NULL){
                continue;
            }
            else{
                longjmp(env, 2);
            }
        } 
        

        
        // get number of columns of the CSV file.
        for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
        
            // work with a copy of that string
            strcpy(string_tmp, Dataset->groundData.fileContent.rawRows[idx]);
            
            // check over all rows the number of columns:
            token_tmp = strtok(string_tmp, ";");
            
            cols_cnt = 0;
            while (token_tmp != NULL){
            
                cols_cnt ++;
                token_tmp = strtok(NULL, ";");
            }
            // if there are differences take always the max number:
            Dataset->groundData.fileContent.numCols = (cols_cnt > Dataset->groundData.fileContent.numCols) ? cols_cnt : Dataset->groundData.fileContent.numCols;
        }
        if (Dataset->groundData.fileContent.numCols <= 0){
            longjmp(env, 4);
        }

        
        
        
        // Allocate the data matrix to import the raw data from the file.
        Dataset->groundData.rawDataMatrix = (char***) malloc(Dataset->groundData.fileContent.numRows * sizeof(char**));
        if (Dataset->groundData.rawDataMatrix == NULL){
            longjmp(env, 2);
        }
        else{
            for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
            
                Dataset->groundData.rawDataMatrix[idx] = (char**) malloc(Dataset->groundData.fileContent.numCols * sizeof(char*));
                if (Dataset->groundData.rawDataMatrix[idx] == NULL){
                    longjmp(env, 2);
                }
                else{
                    for (jdx=0; jdx<Dataset->groundData.fileContent.numCols; jdx++){
                    
                        Dataset->groundData.rawDataMatrix[idx][jdx] = (char*) malloc(100 * sizeof(char));
                        if (Dataset->groundData.rawDataMatrix[idx][jdx] == NULL){
                            longjmp(env, 2);
                        }
                    }
                }
            }
        }
        
        
        // Allocate the final data matrix for the formated data:
        Dataset->groundData.formatedDataMatrix = (char***) malloc(Dataset->groundData.fileContent.numRows * sizeof(char**));
        if (Dataset->groundData.formatedDataMatrix == NULL){
            longjmp(env, 2);
        }
        else{
            for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
            
                Dataset->groundData.formatedDataMatrix[idx] = (char**) malloc(Dataset->number_parameters * sizeof(char*));
                if (Dataset->groundData.formatedDataMatrix[idx] == NULL){
                    longjmp(env, 2);
                }
                else{
                    for (jdx=0; jdx<Dataset->number_parameters; jdx++){
                    
                        Dataset->groundData.formatedDataMatrix[idx][jdx] = (char*) malloc(10 * sizeof(char));
                        if (Dataset->groundData.formatedDataMatrix[idx][jdx] == NULL){
                            longjmp(env, 2);
                        }
                    }
                }
            }
        }

        
        // copy all data from file to the allocated raw data matrix:
        for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
        
            jdx=0;
        
            // work with just a copy of that string:
            strcpy(string_tmp, Dataset->groundData.fileContent.rawRows[idx]);
        
            token_tmp = strtok(string_tmp, ";");
            
            while (token_tmp != NULL){
            
                strcpy(Dataset->groundData.rawDataMatrix[idx][jdx], token_tmp);
                jdx ++;
                token_tmp = strtok(NULL, ";");
            }
        }
        
        for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
        
            /*for (jdx=0; jdx<Dataset->groundData.fileContent.numCols; jdx++){
            
                printf("%10s",Dataset->groundData.rawDataMatrix[idx][jdx]);
            
            }*/
            printf("%10s",Dataset->groundData.rawDataMatrix[idx][2]);
            printf("\n");
        }
        
        
        
        
        
        
        fprintf(stdout, ">> File read: %s%s\n", Dataset->path, Dataset->filename);
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: read_data(Dataset); return EXIT_SUCCESS;
        case 1: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The number of rows of the CSV file shouldnt be 0.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        case 2: fprintf(stderr, "ERROR: (%s -> %s)\n>>> %s\n", __FILE__, __func__, strerror(errno)); return EXIT_FAILURE;
        case 3: fprintf(stderr, "ERROR: (%s -> %s)\n>>> Import of station id from the CSV file failed!\n\
                                                        The length of the imported stations id is equal to 0.\n", __FILE__, __func__); return EXIT_FAILURE; 
        case 4: fprintf(stderr,"ERROR: (%s -> %s)\n>>> The number of columns of the CSV file shouldnt be 0.\n\n", __FILE__, __func__); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// #####################################################################################################
// #####################################################################################################


int close_CSVfile(struct DWDWeatherReportPoi *Dataset){

    /*
        DESCRIPTION:
        Closes the file by closing the filepointer to the csv file.
    
        INPUT:
        struct DWDWeatherReportPoi *Dataset	...	pointer to the corresponding dataset.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - 
    */

    jmp_buf env;

    // ########################################### functions ###########################################      
    void close_file(struct DWDWeatherReportPoi *Dataset){
    
        if (fclose(Dataset->groundData.fileContent.filePointer) == EXIT_SUCCESS){
        
            fprintf(stdout, ">> File closed: %s%s\n",Dataset->path, Dataset->filename);            
        }
        else{
            longjmp(env, 1);
        }
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: close_file(Dataset); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: (%s -> %s)\n>>> %s\n", __FILE__, __func__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
}


// #####################################################################################################
// #####################################################################################################


int show_dataMatrix(struct DWDWeatherReportPoi *Dataset){

    /*
        DESCRIPTION:
        Shows the data matrix with the observation data imported from the csv file.
    
        INPUT:
        struct DWDWeatherReportPoi *Dataset	...	pointer to the corresponding dataset.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - 
    */

    int idx, jdx; 
    jmp_buf env;
    
    // ########################################### functions ########################################### 
    
    void show(struct DWDWeatherReportPoi *Dataset){
    
        // rows
        for (idx=0; idx<Dataset->groundData.fileContent.numRows; idx++){
        
            // columns:
            for (jdx=0; jdx<Dataset->number_parameters; jdx++){
            
                printf("%8s",Dataset->groundData.formatedDataMatrix[idx][jdx]);
            }
            printf("\n");
        }
    }
    // ################################################################################################# 
       
    switch(setjmp(env)){
        case 0: show(Dataset); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: (%s -> %s)\n>>> %s\n", __FILE__, __func__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;          

    }
}


// #####################################################################################################
// #####################################################################################################















