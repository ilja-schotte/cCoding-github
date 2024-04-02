#ifdef __unix__
    #include <stdio.h>
    #include "is_DWDreadCSVpoi.h"
#endif









int main(int argc, char *argv[]){


    int err;
    
    struct DWDWeatherReportPoi Dataset = {
        .filetype = ".csv",
        .path = "./",
        .number_parameters = 15,
        .wanted_parameters = {"Datum",
                              "Uhrzeit (UTC)",
                              "Wolkenbedeckung",
                              "Taupunkttemperatur (2m)",
                              "Temperatur (2m)",
                              "Sichtweite",
                              "Windboen (letzte Stunde)",
                              "Windrichtung",
                              "Windgeschwindigkeit",
                              "Niederschlag (letzte Stunde)",
                              "aktuelles Wetter",
                              "Druck (auf Meereshoehe)"
                              "Relative Feuchte",
                              "Schneehoehe",
                              "Sonnenscheindauer (letzte Stunde)"},
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
    
    // close CSV file:
    err = close_CSVfile(&Dataset);
    if (err == EXIT_FAILURE){
        return 1;
    }    

    return 0;

}

















