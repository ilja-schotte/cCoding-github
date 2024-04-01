#ifdef __unix__
    #include <stdio.h>
    #include <setjmp.h>
    #include <stdlib.h>
    #include <string.h>
    #include <errno.h>
#endif 


/* ##########################################################################################################
   Author:	Schotte, Ilja
   
   SHORT DESCRIPTION:
   Opens csv files downloaded from https://opendata.dwd.de/weather/weather_reports/poi/.
   Reads all wanted meteorologic parameters and copies these information into a private struct. 
*/


struct MeasuredData{

    char MDate[24][11];	// char: 	"dd.mm.yyyy"
    char MTime[24][6];	// char: 	"HH:MM"
    unsigned char N;	// int:		0 to 100 %
    float DP;		// float:	°C		
    float TT;		// float:	°C
    float VV;		// float:	km
    char MG;		// float:	km/h
    unsigned short WD;	// ushort:	0 to 350°
    float RRh;		// float:	mm
    char WW;		// int:		0 to 99
    char RH;		// int:		0 to 100%
    unsigned short SND;	// Snow Depth		0 ... n cm
    unsigned char SD;	// Sunshine Duration	0 ... 60 min 

}

 
struct DWDWeatherReportPoi{

    char filetype[100];		// datatype (format) Examples: ".csv", ".nc", ...
    char filename[100];		// filename of the file you want to read
    char path[100]; 		// path where you can find that file
    char stationId[10];		// id of the corresponding station
    FILE *filePointer;		// file pointer to read the file

    struct MeasuredData 
};






int read_main_arguments(int argc, char *argv[], struct DWDWeatherReportPoi *Dataset);	// Checks the number of passed arguments and reads the current filename depending on the passed filetype.	
int open_file(struct DWDWeatherReportPoi *Dataset);					// Opens the file under the given path and filename and passes a filepointer to the dataset object. 





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


int open_file(struct DWDWeatherReportPoi *Dataset){

    /*
        DESCRIPTION:
        Trys to open the given file under passed filepath and filename and passes the filepointer to the dataset object.
    
        INPUT:
        char *path	...	path to the file to open.
        char *filename	...	filename of the file to open.
    
        OUTPUT:
        Outputs an error code:
        success:	...	1 (EXIT_SUCCESS)
        failure:	...	0 (EXIT_FAILURE)
        
        CHECKS:
        - 
    */

    jmp_buf env;
    
    void open_file(struct DWDWeatherReportPoi *Dataset){
    
        Dataset->filePointer = fopen(strcat(Dataset->path, Dataset->filename), "r");
        
        if (Dataset->filePointer == NULL){
            longjmp(env, 1);
        }
        else{
            fprintf(stdout, ">> Read file: %s\n",Dataset->path);
        }
    }
    switch(setjmp(env)){
        case 0: open_file(Dataset); return EXIT_SUCCESS;
        case 1: fprintf(stderr, "%s\n",strerror(errno)); return EXIT_FAILURE;
        default: fprintf(stderr,"Woops! ( %s -> %s)\n>>> Something unexpected has happend.\n\n", __FILE__, __func__); return EXIT_FAILURE;
    }
    



}



















