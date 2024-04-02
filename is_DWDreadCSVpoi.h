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


struct MeasuredData{

    char MDate[24][11];			// measured date			"dd.mm.yyyy"
    char MTime[24][6];			// measured time			"HH:MM"
    unsigned char *N[24];		// cloud cover				%
    float *DP[24];			// dew point				°C		
    float *TT[24];			// temperature				°C
    float *VV[24];			// visibility				km
    char *FX[24];			// maximum gust in last hour		km/h
    char *FF[24];			// wind speed last 10 min		km/h	
    unsigned short *DD[24];		// wind direction			°
    float *RRh[24];			// precipitation last hour		mm
    char *WW[24];			// present weather			0 to 99
    char *RH[24];			// relative humidity			%
    unsigned short *PP[24];		// Pressure reduced to sea level 	hPa
    unsigned short *SND[24];		// snowdepth				cm
    unsigned char *SD[24];		// sunshine duration last hour		min 
    
};

 
struct DWDWeatherReportPoi{

    char filetype[100];			// datatype (format) Examples: ".csv", ".nc", ...
    char filename[100];			// filename of the file you want to read
    char path[100]; 			// path where you can find that file
    char stationId[10];			// id of the corresponding station
    FILE *filePointer;			// file pointer to read the file
    int number_parameters;		// number of all parameters you want of the csv file
    char wanted_parameters[50][100];	// array of wanted parameters of the csv file

    struct MeasuredData groundData; 
};






int read_main_arguments(int argc, char *argv[], struct DWDWeatherReportPoi *Dataset);	// Checks the number of passed arguments and reads the current filename depending on the passed filetype.	
int open_CSVfile(struct DWDWeatherReportPoi *Dataset);					// Opens the file under the given path and filename and passes a filepointer to the dataset object.
int close_CSVfile(struct DWDWeatherReportPoi *Dataset);					// Closes the file by closing the corresponding filepointer to this file.
int allocate_cmatrix(char **ptr, int rows, int cols);					// allocates a rows x cols matrix of datatype char.



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
    
        Dataset->filePointer = fopen(strcat(file_tmp, Dataset->filename), "r");
        
        if (Dataset->filePointer == NULL){
            longjmp(env, 1);
        }
        else{
            fprintf(stdout, ">> Read file: %s\n",file_tmp);
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
    int idx;
    
    // ########################################### functions ###########################################          
    void read_data(struct DWDWeatherReportPoi *Dataset){
    
        int row_length = (int)USHRT_MAX;	// USHRT_MAX = 65535
        char row[row_length];
        
        
    

    
    }
    // #################################################################################################
    
    switch(setjmp(env)){
        case 0: read_data(Dataset); return EXIT_SUCCESS;
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
    
        if (fclose(Dataset->filePointer) == EXIT_SUCCESS){
        
            fprintf(stdout, ">> file closed: %s%s\n",Dataset->path, Dataset->filename);            
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


int allocate_cmatrix(char **ptr, int rows, int cols){

    /*
        DESCRIPTION:
        Allocates memory for a matrix of rows x columns fields. 
    
        INPUT:
        void *ptr	...	pointer of type char to the allocated memory.
        char *datatype	...	datatype of the allocated memory.
        int rows	...	number of rows.
        int cols	...	number of columns.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - rows and columns must be greater than 0
    */
    
    jmp_buf env;
    int idx;
    
    // ########################################### functions ###########################################      
    void allocate(char **ptr, int rows, int cols){
    
        // check if rows and cols are greater than 0
        if ((rows <= 0) || (cols <= 0)){
            longjmp(env, 1);
        }
        
        ptr = (char **) malloc(cols * sizeof(char*));
        if (ptr == NULL){
            longjmp(env, 2);
        }
        else{
            for (idx=0; idx<cols; idx++){
                ptr[idx] = (char *) malloc(rows * sizeof(char));
                if (ptr[idx] == NULL){
                    longjmp(env, 2);
                }
            }
        }
    }
    // #################################################################################################    
    
    switch(setjmp(env)){
        case 0: allocate(ptr, rows, cols); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "ERROR: (%s -> %s)\n>>> The size of rows and columns must be greater than 0.\n\n", __FILE__, __func__); return EXIT_FAILURE;       
        case 2: fprintf(stderr, "ERROR: (%s -> %s)\n>>> %s\n", __FILE__, __func__, strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! (%s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }

}














