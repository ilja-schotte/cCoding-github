
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <string.h>
    #include <stdbool.h>
    #include "./headerfiles/idw_structs.h"
    #include "./headerfiles/idw.h"
#endif


/* ##########################################################################################

Author: 	Schotte, Ilja
Latest Update:	22.04.2023
Compiled with:	gcc v7.5.0


DESCRIPTION:
Performs an inverse distance weighted method on daily precipitation sums.

ARGUMENTS:
Basically this function performs an "inverse distance weighted" interpolation.

-o	...	This function shows no output during its calculations.
		You can enable an extensive output by using this parameter
		
###########################################################################################*/




int main(int argc, char **argv){


    int idx;
    int err;
    
    // ####################################################################################
    // ############ declaration and initializing of object "Map" ############################
    // use this section for configuration purposes:
    
    struct usr_map Map = {.minLat = 47.000,						// minLat ("minimum latitude") : min. decimal degree (latitude) 
                          .maxLat = 55.000,						// maxLat ("maximum latitude") : max. decimal degree (latitude)
                          .minLon = 5.000,						// minLon ("minimum longitude") : min. decimal degree (longitude) 
                          .maxLon = 16.000,						// maxLon ("maximum longitude") : max. decimal degree (longitude)
                          .latRes = 0,							// 0,04545 => resolution of 5 km / 0,00909 => resolution of approx 1km
                          .lonRes = 0,							// 0,04545 => resolution of 5 km / 0,00909 => resolution of approx 1km
                          .latMetRes = 0,						// resolution between two points (geogr. latitude)
                          .lonMetRes = 0,						// resolution between two points (geogr. longitude)
                          .show_output = false,						// show output during calculations
                          .rows = 900,							// 900 => resolution of 1 km in horizontal direction
                          .cols = 0,							// will be subsequently calculated 						
                          .config = {.output_dir = {"./output/"},			// output directory 
                                     .output_datafile = {"interpolRaster.csv"}, 	// outputfile without correction
                                     .input_dir = {"./input/"},				// input directory 			
                                     .input_datafile = {"tagessummen_177.csv"}},	// dataset of the sums of daily precipiation
                         .input_data.data = NULL,
                         .raster = NULL,
                         };
    
    
    // ####################################################################################    
    // ####################################################################################
    
    // read the arguments of main and performs some calculations to get the number of columns of the raster.
    err = set_config(&Map, argc, argv);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;
    
    
    // initialize the raster of the map:
    err = create_maps_raster(&(Map.raster), Map.rows, Map.cols);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;

    // fill the raster points with information:
    // - coordinates
    // - index
    err = fill_raster_with_default_data(&Map);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;
   
   
    // Read the input dataset out of the given csv file:
    err = input_csv_data(&Map, Map.config.input_datafile);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;
                
    // Ordne die Messpunkte den Rasterpunkten zu:
    err = fill_raster_with_input_data(&Map);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;
    
    // show the input dataset:
    err = show_input_data(&Map);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;    

    // Interpoliere nun das Raster:
    err = interpolate_raster(&Map);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;
                  
    /*// Bestimme Metadaten des Ausgabeprodukts:
    err = get_output_information(&Map);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;
                
    // show information regarding to the interpolated raster if Map.show_output is set to true
    err = show_map_info(&Map);
    (err == EXIT_FAILURE) ? ({
        free_raster(&Map);
        free_vector(&Map);
        exit(err);
    }) : NULL;
    
       
    // the output depends on if correction of negative weights was selected or not:
    if (Map.weights_correction){        
        outputRasterCSV(Map.raster, Map.config.output_dir, Map.config.output_datafile_cor, Map.rows, Map.cols, Map.show_output);
    }
    else{
        outputRasterCSV(Map.raster, Map.config.output_dir, Map.config.output_datafile, Map.rows, Map.cols, Map.show_output);
    }
    */
    // clean up:
    free_raster(&Map);
    free_vector(&Map);             
                
    return 0;
}









