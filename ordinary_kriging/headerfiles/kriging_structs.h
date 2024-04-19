
struct usr_function{

    int order;				// Grad der Funktion.
    double *solution;			// Zeiger auf Vektor der Länge (order + 2)

};

struct usr_variogram{

    struct usr_vario_class *classes;	// Feld von Abstandsklassen
    int numClasses;			// Anzahl der Abstandsklassen
    int model_adjust_index;		// Index der Eingabedaten. Bis zu diesem Index der Eingabedaten wird das gesuchte Semivarianzmodell angepasst.
    double distInterval;		// Abstandsintervall der Abstandsklassen
    double maxDistance;			// Maximale Ausdehnung (km) übder die die Abstandsklassen gebildet werden sollen.
    double sill;			// Schwelle: Grenzwert ab dem sich nur noch zufällige Schwankungen der Semivarianz ergeben.
    double nugget;			// Grundrauschen der Semivarianz
    double range;			// Reichweite: Ist die Distanz ab der der Grenzwert (sill) erreicht wird.
    struct usr_function reg_function;	// polynomiale Regressionsfunktion zur Ermittlung des Variogrammmodells.

};


struct usr_vario_class{

    int lowerLimit;			// Untere Grenze der Abstandsklasse
    int upperLimit;			// Obere Grenze der Abstandsklasse
    int num_variance_values;		// Anzahl der Semivarianzwerte in dieser Abstandsklasse
    int num_distance_values;		// Anzahl der Distanzen in der Abstanzklasse
    double variance_values[1000];	// Feld an Semivarianzen der Abstandsklasse
    double distance_values[1000];	// Feld der Distanzen der Abstandsklasse
    double variance_avg;		// Mittelwert der Semivarianzwerte dieser Abstandsklasse
    double variance_avg_reg;		// Vorhersagewert der Semivarianz mittels polynomialer Regression dieser Abstandsklasse (lag)
    double distance_avg; 		// gemittelte Distanz dieser Abstandsklasse (lag)
};

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
    char output_datafile_cor[100];

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
    
    // Nachträgliche Korrektur der Gewichte beim Interpolieren?
    bool weights_correction;
    
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
    
    // Variogramm:
    struct usr_variogram variogram;
    
    // Distanzmatrix (Distanz eines jeden Messpunktes zum anderen Messpunkt):
    double **distance_matrix;
    
    // Kovarianzmatrix (Kovarianzen eines jeden Messpunktes zum anderen Messpunkt):
    double **covariance_matrix; 
    
    // Inverse der Kovarianzmatrix:
    double **covariance_matrix_inv;  
};
