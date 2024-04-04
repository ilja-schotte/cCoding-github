#ifdef __unix__
    #include <stdio.h>
    #include "is_DWDreadCSVpoi.h"
#endif









int main(int argc, char *argv[]){

    int idx, jdx;
    int err;
    
    struct DWDWeatherReportPoi Dataset = {
        .filetype = ".csv",
        .path = "./",
        .number_parameters = 15,
        .wanted_parameters = {
            {"Datum", "date"},
            {"Uhrzeit (UTC)", "time"},
            {"Wolkenbedeckung", "NN"},
            {"Taupunkttemperatur (2m)", "TD"},
            {"Temperatur (2m)", "TT"},
            {"Sichtweite", "VV"},
            {"Windboen (letzte Stunde)", "FXH"},
            {"Windrichtung", "DD"},
            {"Windgeschwindigkeit", "FF"},
            {"Niederschlag (letzte Stunde)", "RR"},
            {"aktuelles Wetter", "WW"},
            {"Druck (auf Meereshoehe)", "PP"},
            {"Relative Feuchte", "RH"},
            {"Schneehoehe", "SND"},
            {"Sonnenscheindauer (letzte Stunde)", "SD"}
        },
        .groundData = {
            .fileContent = {
                .numRows = 0,
                .numCols = 0,
                .headerParameterRow = 2,
                .firstDataRow = 3,
            },
        },
    };
    
    
    
    
    
    err = read_main_arguments(argc, argv, &Dataset);
    if (err == EXIT_FAILURE){
        return 1;
    }
    
    // Open CSV file:
    err = open_CSVfile(&Dataset);
    if (err == EXIT_FAILURE){
        return 1;
    }
    
    // read CSV file:
    err = read_CSVdata(&Dataset);
    if (err == EXIT_FAILURE){
        return 1;
    }    
    
    // close CSV file:
    err = close_CSVfile(&Dataset);
    if (err == EXIT_FAILURE){
        return 1;
    }
    
    //err = show_dataMatrix(&Dataset);
    //if (err == EXIT_FAILURE){
    //    return 1;
    //}
    
    
    // free rows of raw file content
    for (idx=0; idx<Dataset.groundData.fileContent.numRows; idx++){
        free(Dataset.groundData.fileContent.rawRows[idx]);
    }
    free(Dataset.groundData.fileContent.rawRows);
    
    // free formated input dataset
    for (idx=0; idx<Dataset.groundData.fileContent.numRows; idx++){
    
        for (jdx=0; jdx<Dataset.groundData.fileContent.numCols; jdx++){
        
            free(Dataset.groundData.rawDataMatrix[idx][jdx]);
            
        }
        free(Dataset.groundData.rawDataMatrix[idx]);
    }
    free(Dataset.groundData.rawDataMatrix);    
    
    
    
    // free formated input dataset
    for (idx=0; idx<Dataset.groundData.fileContent.numRows; idx++){
    
        for (jdx=0; jdx<Dataset.number_parameters; jdx++){
        
            free(Dataset.groundData.formatedDataMatrix[idx][jdx]);
            
        }
        free(Dataset.groundData.formatedDataMatrix[idx]);
    }
    free(Dataset.groundData.formatedDataMatrix);
    

    return 0;

}










