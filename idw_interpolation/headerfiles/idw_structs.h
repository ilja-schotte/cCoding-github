
struct usr_dataset{

    int length;
    double maximum;
    double minimum;
    double average;
    struct usr_data_point *data;

};


// Definition: Strukturen
struct usr_data_point{

    int row_idx;
    int col_idx;
    
    double lat;
    double lon;
    double value;
    
    char name[100];

};

struct usr_config{

    char output_dir[100];
    char input_dir[100];
    char input_datafile[100];
    char output_datafile[100];

};


struct usr_map{

    // Ausmaße der Karte:
    // geogr. Breite
    double minLat;
    double maxLat;
    
    // geogr. Länge
    double minLon;
    double maxLon;
    
    // Anzahl der Felder des Rasters:
    unsigned int rows;
    unsigned int cols;
    
    // Auflösung Dezimalgrad:
    double latRes;
    double lonRes;
    
    // Auflösung in km:
    double latMetRes;
    double lonMetRes;
    
    // show output during calculations on console:
    bool show_output;
    
    // Konfiguration:
    struct usr_config config;
    
    // Eingabe-Daten:
    struct usr_dataset input_data;
    
    // Ausgabe-Daten:
    struct usr_dataset output_data;
    
    // Eingabe-Raster:
    struct usr_data_point **raster;
 
};
