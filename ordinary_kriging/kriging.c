
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <string.h>
    #include <stdbool.h>
    #include "./headerfiles/kriging_structs.h"
    #include "./headerfiles/kriging.h"
#endif


/* ##########################################################################################

Author: Schotte, Ilja
Latest Update: 19.04.2023

DESCRIPTION:
Performs an ordinary kriging method on daily precipitation sums.

ARGUMENTS:
Basically this function performs a simple ordinary kriging. But it is possible
that some weights in later calculations are getting negativ. So in this case it is possible
to call this function with:

-c	...	This function performs corrections of negativ weights later on.
		Basically it is recommended to call this argument.
		
###########################################################################################*/




int main(int argc, char **argv){


    int idx;
    int err;
    
    // ####################################################################################
    // ############ declaration and initialize of object "Map" ############################
    // use this section for configuration purposes:
    
    struct usr_map Map = {.minLat = 47.000,						// minLat ("minimum latitude") : min. decimal degree (latitude) 
                          .maxLat = 55.000,						// maxLat ("maximum latitude") : max. decimal degree (latitude)
                          .minLon = 5.000,						// minLon ("minimum longitude") : min. decimal degree (longitude) 
                          .maxLon = 16.000,						// maxLon ("maximum longitude") : max. decimal degree (longitude)
                          .latRes = 0,							// 0,04545 => resolution of 5 km / 0,00909 => resolution of approx 1km
                          .lonRes = 0,							// 0,04545 => resolution of 5 km / 0,00909 => resolution of approx 1km
                          .latMetRes = 0,						// resolution between two points (geogr. latitude)
                          .lonMetRes = 0,						// resolution between two points (geogr. longitude)
                          .weights_correction = false,					// subsequently correction of negative weights
                          .show_output = false,						// show output during calculations
                          .rows = 900,							// 900 => resolution of 1 km in horizontal direction
                          .cols = 0,							// will be subsequently calculated 
                          .variogram = {.distInterval = 50,				// width of each distance-interval of the variogram
                                        .maxDistance = 900,				// maximum distance
                                        .nugget = 0.001,
                                        .sill = 0,
                                        .range = 0,
                                        .model_adjust_index = 0,
                                        .reg_function = {.order = 4}			// order of regression function
                                        },						
                          .config = {.output_dir = {"./output/"},			// output directory 
                                     .output_datafile = {"interpolRaster.csv"}, 	// outputfile without correction
                                     .output_datafile_cor = {"interpolRaster_c.csv"},	// ouputtfile with correction
                                     .input_dir = {"./input/"},				// input directory 			
                                     .input_datafile = {"tagessummen_177.csv"}},	// dataset of the sums of daily precipiation
                         };
    
    
    // ####################################################################################    
    // ####################################################################################
    
    // read the arguments of main and performs some calculations to get the number of columns of the raster.
    err = set_config(&Map, argc, argv);
    (err == EXIT_FAILURE) ? exit(err) : NULL;
    
    // initialize the raster of the map:
    err = create_maps_raster(&(Map.raster), Map.rows, Map.cols);
    (err == EXIT_FAILURE) ? exit(err) : NULL;

    // fill the raster points with information:
    // - coordinates
    // - index
    err = fill_raster_with_default_data(&Map);
    (err == EXIT_FAILURE) ? exit(err) : NULL;
   
    // Raed the input dataset out of the gives csv file::
    err = input_csv_data(&Map, Map.config.input_datafile);
    (err == EXIT_FAILURE) ? exit(err) : NULL;
           
    // Zeige den Input-Datensatz:
    //show_input_data(&Map);
                
    // Ordne die Messpunkte den Rasterpunkten zu:
    err = fill_raster_with_input_data(&Map);
    (err == EXIT_FAILURE) ? exit(err) : NULL;
    
    // Erstelle eine Abstandsmatrix:
    err = create_distance_matrix(&Map);
    (err == EXIT_FAILURE) ? exit(err) : NULL;
    
    //check the matrix for nan and inf value and get the max and min value:
    err = check_matrix(Map.distance_matrix, Map.input_data.length, Map.input_data.length, Map.show_output);
    (err == EXIT_FAILURE) ? exit(err) : NULL;
                
    // Erstelle aus den Messwerten ein Variogramm:
    err = create_variogram(&Map);
    (err == EXIT_FAILURE) ? exit(err) : NULL;

    // Determine out of the semivariances the covariance model:
    err = get_variogram_model(&Map);
    (err == EXIT_FAILURE) ? exit(err) : NULL; 
                       
    // Zeige Informationen zu Variogramm:
    show_variogram_data(&Map);
    
    // Erstelle die Kovarianzmatrix:
    err = create_covariance_matrix(&Map);
    (err == EXIT_FAILURE) ? exit(err) : NULL;

    //check the matrix for nan and inf value and get the max and min value:                                                     
    err = check_matrix(Map.covariance_matrix, Map.input_data.length+1, Map.input_data.length+1, Map.show_output);
    (err == EXIT_FAILURE) ? exit(err) : NULL;    
    exit(0);
    
    //show_matrix(Map.covariance_matrix, 20, 20);
                
    //outputMatrixCSV(Map.covariance_matrix, "output_covMatrix.csv", Map.input_data.length+1, Map.input_data.length+1);
                
    // Berechne die Determinante der Kovarianzmatrix
    //printf("%.3f\n",determinantOfMatrix(Map.covariance_matrix, Map.input_data.length));          
                               
                
    // Berechne die Inverse der Kovarianzmatrix:
    Map.covariance_matrix_inv = inverseOfMatrix(Map.covariance_matrix, 
                                                Map.input_data.length+1, 
                                                 Map.input_data.length+1);
                                
    check_matrix(Map.covariance_matrix_inv, 
                 Map.input_data.length+1, 
                 Map.input_data.length+1,
                 Map.show_output);
                             
    //show_matrix(Map.covariance_matrix_inv, 20, 20);
                
    //outputMatrixCSV(Map.covariance_matrix_inv, "output_covMatrix_inv.csv", Map.input_data.length+1, Map.input_data.length+1);
                
                
    // Multipliziere die Kovarianzmatrix mit ihrer Inversen um zur Probe die Einheitsmatrix zu erhalten.
    /*multiplyMatrix(Map.covariance_matrix, 
                     Map.covariance_matrix_inv, 
                     Map.input_data.length+1, 
                     Map.input_data.length+1);*/
                
    // Interpoliere nun das Raster:
    interpolate_raster(&Map);
                
    // Bestimme Metadaten des Ausgabeprodukts:
    get_output_information(&Map);
                
    // Zeige Informationen zum Raster:
    show_map_info(&Map);
                
    if (Map.weights_correction){        
        outputRasterCSV(Map.raster, Map.config.output_dir, Map.config.output_datafile_cor, Map.rows, Map.cols);
    }
    else{
        outputRasterCSV(Map.raster, Map.config.output_dir, Map.config.output_datafile, Map.rows, Map.cols);
    }
                
    free_raster(&Map);
    free_vector(&Map);               
                
    return 0;
}









