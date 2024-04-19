
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <string.h>
    #include <stdbool.h>
#endif

#define NO_VALUE -1.0
#define M_PI 3.14159265358979323846
#define EPS 1.0E-3

struct usr_variogram{

    struct usr_vario_class *classes;	// Feld von Abstandsklassen
    int numClasses;			// Anzahl der Abstandsklassen
    int distInterval;			// Abstandsintervall der Abstandsklassen
    double sill;			// Schwelle: Grenzwert ab dem sich nur noch zufällige Schwankungen der Semivarianz ergeben.
    double nugget;			// Grundrauschen der Semivarianz
    double range;			// Reichweite: Ist die Distanz ab der der Grenzwert (sill) erreicht wird.

};


struct usr_vario_class{

    int lowerLimit;			// Untere Grenze der Abstandsklasse
    int upperLimit;			// Obere Grenze der Abstandsklasse
    int num_variance_values;		// Anzahl der Semivarianzwerte in dieser Abstandsklasse
    double variance_values[255];	// Feld an Semivarianzen der Abstandsklasse
    double variance_avg;		// Mittelwert der Semivarianzwerte dieser Abstandsklasse 
};




struct usr_dataset{

    int length;
    struct usr_data_point *data;

};



// Definition: Strukturen
struct usr_data_point{

    int row_idx;
    int col_idx;
    
    double lat;
    double lon;
    double value;
    
    char name[30];

};

struct usr_config{

    char output_dir[100];
    char input_dir[100];

};


struct usr_map{

    // Ausmaße der Karte:
    // geogr. Breite
    double minLat;
    double maxLat;
    
    // geogr. Länge
    double minLon;
    double maxLon;
    
    // Maximale Entfernung zweier Punkte auf der Karte:
    int maxDist;
    
    // Anzahl der Felder des Rasters:
    int rows;
    int cols;
    
    // Auflösung Dezimalgrad:
    double latRes;
    double lonRes;
    
    // Auflösung in km:
    double latMetRes;
    double lonMetRes;
    
    // Nachträgliche Korrektur der Gewichte beim Interpolieren?
    bool weights_correction;
    
    // Konfiguration:
    struct usr_config config;
    
    // Eingabe-Daten:
    struct usr_dataset input_data;
    
    // Eingabe-Raster:
    struct usr_data_point **raster_in;
    
    // Ausgabe-Raster:
    struct usr_data_point **raster_out;
    
    // Variogramm:
    struct usr_variogram variogram;
    
    // Distanzmatrix (Distanz eines jeden Messpunktes zum anderen Messpunkt):
    double **distance_matrix;
    
    // Kovarianzmatrix (Kovarianzen eines jeden Messpunktes zum anderen Messpunkt):
    double **covariance_matrix; 
    
    // Inverse der Kovarianzmatrix:
    double **covariance_matrix_inv;  
};






// Deklaration: Funktion
struct usr_data_point **create_2D_raster(int rows, int cols);

int fill_raster_with_default_data(struct usr_map *zgr);
int input_csv_data(struct usr_map *zgr);
int fill_raster_with_input_data(struct usr_map *zgr);
int create_variogram(struct usr_map *zgr);
int interpolate_raster(struct usr_map *zgr);
int multiplyMatrix(double **matrixA, double **matrixB, int rows, int cols);

double *multiplyMatrixVector(double **matrix, double *vector_in, int rows, int cols);          
double calc_distance(double latA, double lonA, double latB, double lonB);
double calc_covariance(double distance);
double determinantOfMatrix(double **matrix, int rang);
double **create_covariance_matrix(struct usr_map *zgr, double **matrix, int rows, int cols);
double **inverseOfMatrix(double **matrix, int rows, int cols);
double **create_distance_matrix(struct usr_dataset dataset, int length);
double **create_matrix(int rows, int cols);
double *create_vector(int length);
                 
void show_raster(struct usr_data_point **zgr, int *rows, int *cols);
void show_map_info(struct usr_map *zgr);
void show_input_data(struct usr_map *zgr);
void show_variogram_data(struct usr_map *zgr);
void show_matrix(double **matrix, int rows, int cols);
void check_matrix(double **matrix, int rows, int cols);
void correct_negative_weights(double *weights_vector, int length);
void outputMatrixCSV(double **matrix, char *filename, int rows, int cols);
void outputRasterCSV(struct usr_data_point **raster, char *output_dir, char *filename, int rows, int cols);


void free_raster(struct usr_map *zgr);
void free_vector(struct usr_map *zgr);




// ###########################################################################
// ###########################################################################




int main(int argc, char *argv[]){



    bool weights_correction = false;
    int idx;

    // Einlesen der Parameter ################################################
    
    for (idx=0; idx<argc; idx++){
        
        // Korrektur der negativen Gewichte beim Interpolieren?
        if (!strcmp(argv[idx],"-p")){
            weights_correction = true;
        }
    }

    // #######################################################################



    struct usr_data_point **zgr_in, **zgr_out;   
    
    // Deklarieren/Initialisieren des "Objekts" Map;
    struct usr_map Map = {.minLat = 47.000,				// minLat ("minimum latitude") : min. Dezimalgrad (Breite) 
                          .maxLat = 55.000,				// maxLat ("maximum latitude") : max. Dezimalgrad (Breite)
                          .minLon = 5.000,				// minLon ("minimum longitude") : min. Dezimalgrad (Länge) 
                          .maxLon = 16.000,				// maxLon ("maximum longitude") : max. Dezimalgrad (Länge)
                          .latRes = 0,					// 0,04545 => Auflösung von ca. 5km / 0,00909 => Auflösung von ca. 1km
                          .lonRes = 0,					// 0,04545 => Auflösung von ca. 5km / 0,00909 => Auflösung von ca. 1km
                          .latMetRes = 0,				// Auflösung zwischen zwei benachbarten Punkten (geogr. Breite)
                          .lonMetRes = 0,				// Auflösung zwischen zwei benachbarten Punkten (geogr. Länge)
                          .weights_correction = weights_correction,	// Nachträgliche Korrektur der negativen Gewichte beim Interpolieren?
                          .rows = 900,					// 900 => Auflösung con 1km
                          .cols = 0,
                          .variogram = {.distInterval = 50},		// Intervallbreite der Abstandsklassen.
                          .config = {.output_dir = {"./output/"},
                                     .input_dir = {"./input/"}},
                          .maxDist = 900				// Maximale Distanz zweier Punkte in Deutschland.
                         };
    
    Map.cols = ceil(Map.rows * (int)(Map.maxLon - Map.minLon) / (Map.maxLat - Map.minLat));

    

    
    
    Map.raster_in = create_2D_raster(Map.rows, Map.cols);
    Map.raster_out = create_2D_raster(Map.rows, Map.cols);
    
    if (Map.raster_in == NULL){
    
        printf("Speicherfehler beim reservieren der Raster-Matrix (in).\n");
        return 1;
    }
    else{
        // Fülle die punkte der Matrix mit Koordinaten:
        if (fill_raster_with_default_data(&Map)){
        
            // Zeige Informationen zum Raster_in:
            show_map_info(&Map);
            
            
            // Lese die DAten aus der CSV-DAtei ein:
            if (input_csv_data(&Map)){
            
                // Zeige den Input-Datensatz:
                //show_input_data(&Map);
                
                // Ordne die Messpunkte den Rasterpunkten zu:
                fill_raster_with_input_data(&Map);
                
                // Erstelle eine Abstandsmatrix:
                Map.distance_matrix = create_distance_matrix(Map.input_data, 
                                                             Map.input_data.length);
                                                             
                check_matrix(Map.distance_matrix, Map.input_data.length, Map.input_data.length);
                
                // Erstelle aus den Messwerten ein Variogramm und die dazugehörige Funktion:
                create_variogram(&Map);
                
                // Zeige Informationen zu Variogramm:
                show_variogram_data(&Map);
                
                // Erstelle die Kovarianzmatrix:
                Map.covariance_matrix = create_covariance_matrix(&Map, 
                                                                 Map.distance_matrix, 
                                                                 Map.input_data.length+1, 
                                                                 Map.input_data.length+1);
                                                                   
                check_matrix(Map.covariance_matrix, Map.input_data.length+1, Map.input_data.length+1);
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
                             Map.input_data.length+1);
                             
                //show_matrix(Map.covariance_matrix_inv, 20, 20);
                
                //outputMatrixCSV(Map.covariance_matrix_inv, "output_covMatrix_inv.csv", Map.input_data.length+1, Map.input_data.length+1);
                
                
                // Multipliziere die Kovarianzmatrix mit ihrer Inversen um zur Probe die Einheitsmatrix zu erhalten.
                /*multiplyMatrix(Map.covariance_matrix, 
                               Map.covariance_matrix_inv, 
                               Map.input_data.length+1, 
                               Map.input_data.length+1);*/
                
                // Interpoliere nun das Raster.
                interpolate_raster(&Map);
                outputRasterCSV(Map.raster_out, Map.config.output_dir, "interpolRaster.csv", Map.rows, Map.cols);
                
                
                // Zeige das Raster_in:
                //show_raster(Map.raster_in, &(Map.rows), &(Map.cols));
                
                free_raster(&Map);
                free_vector(&Map);               
                
                return 0;
            }
            else{
                free_raster(&Map);
                free_vector(&Map);
                return 1;
            }
        }
        else{
            free_raster(&Map);
            free_vector(&Map);
            return 1;
        }
    }
}
 







// Definition: Funktionen
struct usr_data_point **create_2D_raster(int rows, int cols){

    int idx;
    
    struct usr_data_point **zgr;
    
    
    zgr = (struct usr_data_point **) malloc(rows * sizeof(struct usr_data_point*));
    if (zgr == NULL){
    
        return NULL;
    }
    else{
    
        for (idx=0; idx<rows; idx++){
        
            zgr[idx] = (struct usr_data_point *) malloc(cols * sizeof(struct usr_data_point));        
            if (zgr[idx] == NULL){
            
                return NULL;
            
            }
        }
        printf("Matrix reserviert!\n");
        return zgr;
    }
    return NULL;    
}



double **create_matrix(int rows, int cols){

    /*
        Erzeugt einen Doppelzeiger auf ein Feld der Zeilen "rows" und Spalten "cols".
    */

    int idx;
    
    double **zgr;
    
    
    zgr = (double **) malloc(rows * sizeof(double*));
    if (zgr == NULL){
    
        return NULL;
    }
    else{
    
        for (idx=0; idx<cols; idx++){
        
            zgr[idx] = (double *) malloc(cols * sizeof(double));        
            if (zgr[idx] == NULL){
            
                return NULL;
            
            }
        }
        return zgr;
    }  
}


double *create_vector(int length){

    /*
        Erstellt einen Zeiger auf einen Vektor der Länge "length".   
    */

    int idx;
    
    double *zgr;
    
    
    zgr = (double *) malloc(length * sizeof(double));
    if (zgr == NULL){
    
        return NULL;
    }
    else{
        return zgr;
    }   
}





int input_csv_data(struct usr_map *Map){

    /*
        Ließt die Input-Daten aus einer CSV-Datei und füllt damit ein dyn. Feld vom typ struct usr_data_point.
    */

    FILE *fp;
    int rows = 0;
    int idx, kdx;
    
    char inputRow[255];
    char inputDecimal[20];
    char path[100];
    
    
    fp = fopen(strcat(strcpy(path,Map->config.input_dir), "tagessummen.csv"), "r");
    if (fp == NULL){
    
        printf("Dateifehler!\n");
        return false;
    
    }
    else{
        // Bestimme die Anzahl der Zeilen im CSV file:
        while (fgets(inputRow, 255, fp)){
            rows++;
        }
        
        // Länge des Input_Datensatzes übergeben:
        Map->input_data.length = rows-1;
        
        // Reserviere nun Speicher der Anzahl der Zeilen der CSV Datei und vom Typ usr_data_point.
        Map->input_data.data = (struct usr_data_point*) malloc(rows * sizeof(struct usr_data_point));
    
       
        // Einlesen der Zeilen und füllen der Felder struct usr_data_point:
        idx = 0;
        fseek(fp, 0, SEEK_SET);
        while (fgets(inputRow, 255, fp)){
            
            if (idx > 0){
                // Einlesen des Stationsnamens:
                strcpy(Map->input_data.data[idx-1].name, strtok(inputRow, ";"));
            
            
                // Einlesen der geogr. Breite:
                strcpy(inputDecimal, strtok(NULL, ";"));
                for (kdx=0; kdx<(int)strlen(inputDecimal); kdx++){
            
                    if (inputDecimal[kdx] == ','){
                
                        inputDecimal[kdx] = '.';
                        break;
                    }
                }
                Map->input_data.data[idx-1].lat = atof(inputDecimal);
            
            
                // Einlesen der geogr. Länge:            
                strcpy(inputDecimal, strtok(NULL, ";"));
                for (kdx=0; kdx<(int)strlen(inputDecimal); kdx++){
            
                    if (inputDecimal[kdx] == ','){
                
                        inputDecimal[kdx] = '.';
                        break;
                    }
                }
                Map->input_data.data[idx-1].lon = atof(inputDecimal);  
            
                // Einlesen des Messwerts:         
                strcpy(inputDecimal, strtok(NULL, ";"));
                for (kdx=0; kdx<(int)strlen(inputDecimal); kdx++){
            
                    if (inputDecimal[kdx] == ','){
                
                        inputDecimal[kdx] = '.';
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
        
        fclose(fp);
        return true;   
    }
}



int fill_raster_with_default_data(struct usr_map *Map){

    /*
        In jedem Feld des Rasters liegt eine Struktur vom Typ struct usr_data_point.
    
        Diese Funktion füllt die Strukturen vom Typ struct usr_data_point 
        mit Informationen:
        - lat		...	geogr. Länge des Felds
        - lon		...	geogr. Breite des Felds
        - row_idx	...	Zeilenindex des Felds
        - col_idx	...	Spaltenindex des Felds
        
        Diese Funktion berechnet die Auflösung der Karte in:
        - Dezimalgrad
        - Kilometer
    */


    int idx, kdx;

    
    // Berechne die Auflösung der Matrix in Dezimalgrad:
    Map->lonRes = (Map->maxLon - Map->minLon) / (Map->cols-1);
    Map->latRes= (Map->maxLat - Map->minLat) / (Map->rows-1);
    
    // Berechne die Auflösung der Matrix in Kilometer:   
    // Map->latMetRes = Map->latRes * GRAD_KM;		// => Grad in Meter umrechnen
    // Map->lonMetRes = Map->lonRes * GRAD_KM;
    Map->latMetRes = calc_distance(0.0, 0.0, Map->latRes, 0);
    Map->lonMetRes = calc_distance(0.0, 0.0, 0.0, Map->lonRes);

    // Fülle die Felder der Matrix vom Typ usr_data_point mit Werten:
    for (idx=0; idx<Map->rows; idx++){
    
        for (kdx=0; kdx<Map->cols; kdx++){
        
            // Füllen der Feld-Indizes (in):
            Map->raster_in[idx][kdx].row_idx = idx;
            Map->raster_in[idx][kdx].col_idx = kdx;
            
            // Füllen der Feld-Indizes (out):
            Map->raster_out[idx][kdx].row_idx = idx;
            Map->raster_out[idx][kdx].col_idx = kdx;            
            
            
            // Füllen der Koordinaten (in):
            Map->raster_in[idx][kdx].lon = (Map->minLon) + kdx*(Map->lonRes);
            Map->raster_in[idx][kdx].lat = (Map->maxLat) - idx*(Map->latRes);
            
            // Füllen der Koordinaten (out):
            Map->raster_out[idx][kdx].lon = (Map->minLon) + kdx*(Map->lonRes);
            Map->raster_out[idx][kdx].lat = (Map->maxLat) - idx*(Map->latRes);            
            
            
            // Füllen des Wertes (in):
            Map->raster_in[idx][kdx].value = NO_VALUE;
            
            // Füllen des Wertes (out):            
            Map->raster_out[idx][kdx].value = NO_VALUE;            
        }
    
    }
    
    return true;
}



int fill_raster_with_input_data(struct usr_map *Map){


    int idx, kdx, ndx;
    double mlat, mlon, distance;
    double rlat, rlat_rad, rlon, rlon_rad;
    double min_distance, min_mlat, min_mlon, min_rlat, min_rlon;
    

    int min_row_idx, min_col_idx;
    
    printf("Uebertrage Messpunkte auf Raster ...\n");
    for (idx=0; idx<Map->input_data.length; idx++){
    
        // Setze den Defaultwert der Minimum-Entfernung:
        min_distance = 44000.00;
        
        // Breite und Länge der Station:
        mlat = Map->input_data.data[idx].lat;
        mlon = Map->input_data.data[idx].lon;
        
        for (kdx=0; kdx<Map->rows; kdx++){
            for (ndx=0; ndx<Map->cols; ndx++){
            
                // Breite und Länge des Rasterpunkts:
                rlat = Map->raster_in[kdx][ndx].lat;
                rlon = Map->raster_in[kdx][ndx].lon;
                            
                // Distanzberechnung auf einer Kugel Mit dem Erdradius auf 51°N.
                distance = calc_distance(mlat, mlon, rlat, rlon);
                
                if (distance < min_distance){                  
                    min_distance = distance;
                    min_mlat = mlat;
                    min_mlon = mlon;
                    min_rlat = rlat;
                    min_rlon = rlon;
                    min_row_idx = Map->raster_in[kdx][ndx].row_idx;
                    min_col_idx = Map->raster_in[kdx][ndx].col_idx;
                }
            }
        }
        
        //printf("%s %.3f\n", Map->input_data.data[idx].name, min_distance);
        
        // Rasterpunkt den Stationsnamen übertragen:
        strcpy(Map->raster_in[min_row_idx][min_col_idx].name, Map->input_data.data[idx].name);
        Map->raster_in[min_row_idx][min_col_idx].value = Map->input_data.data[idx].value;
        
        // Inputdaten die Indizes des zugeordneten Rasterpunkts übergeben;
        Map->input_data.data[idx].row_idx = min_row_idx;
        Map->input_data.data[idx].col_idx = min_col_idx;
    }
    printf("Fertig.\n");
    
    return true;

}





double **create_distance_matrix(struct usr_dataset dataset, int length){


    /* 
        Berechnet anhand eines Datensatzes von Messpunkten mit Breiten- und 
        Längengraden die Abstände eines jeden Punktes zu einander.
    */
    
    int idx, kdx;
    double distance;
    double **distance_matrix;
    
    
    
    printf("Berechne die Distanzmatrix ... ");
    
    distance_matrix = create_matrix(length, length);
    if (distance_matrix == NULL){
    
        printf("Speicherfehler: Distanzmatrix\n");
        return NULL;
    
    }
    else{
    
        for (idx=0; idx<length; idx++){
        
            for (kdx=0; kdx<length; kdx++){
            
                // Wenn die Differenz der Breiten und Längengrade kleiner als EPS sind, dann ist die Distanz 0:
                if ((fabs(dataset.data[idx].lat - dataset.data[kdx].lat) < EPS) && 
                    (fabs(dataset.data[idx].lon - dataset.data[kdx].lon) < EPS)){
                
                    distance_matrix[idx][kdx] = 0.0;
                }
                else{
                // Sonst: Berechne die Distanz:                
                    distance_matrix[idx][kdx] = calc_distance(dataset.data[idx].lat, 
                                                              dataset.data[idx].lon, 
                                                              dataset.data[kdx].lat, 
                                                              dataset.data[kdx].lon);
                }
            }
        }
        printf("ok!\n");
        return distance_matrix;
                
    }
}






int create_variogram(struct usr_map *Map){

    /*
        1. Definieren von Abstandsklassen:
        -> Maximaler Abstand zweier Punkte in Deutschland 900 km.
        -> Einteilung in 50 km Klassen -> 18 Klassen.
    */
    
    int idx, kdx, ndx;
    int numValue;
    double semiVarianz;
    double sumPowValue;		// Summe der quadratischen abweichungen pro Abstandsklasse.
    double distance;
    double sumClassVarianz;
    
    
    // Abstandsklassen erstellen (max. Abstand / Intervallgröße):
    Map->variogram.numClasses = (int)(Map->maxDist / Map->variogram.distInterval);

    
    Map->variogram.classes = (struct usr_vario_class *) malloc(Map->variogram.numClasses * sizeof(struct usr_vario_class));
    if (Map->variogram.classes == NULL){
    
        printf("Speicherfehler: Variogram\n");
        return false;
    
    }
    else{
        // Attribute der Abstandsklassen erstellen (lowerLimit / upperLimit):
        for (idx=1; idx<=(Map->variogram.numClasses); idx++){
        
            Map->variogram.classes[idx-1].lowerLimit = (int)((idx-1)*Map->variogram.distInterval);
            Map->variogram.classes[idx-1].upperLimit = (int)((idx)*Map->variogram.distInterval);
            Map->variogram.classes[idx-1].num_variance_values = 0;
            Map->variogram.classes[idx-1].variance_avg = 0;
        
        }
        
        // Berechnen der mittleren Semivarianz für jede Abstandsklasse über alle Punkte:
        // Nimm eine Station aus dem Input-Datensatz:
        for (idx=0; idx<Map->input_data.length; idx++){
                        
            
            // Prüfe nun über die Abstandsklassen, welche Stationen dazugehören und bilde davon die Semivarianz
            // Nimm eine Abstandsklasse:
            for (kdx=0; kdx<Map->variogram.numClasses; kdx++){
            
                // Setze den Zwischenwert, Zähler und den Wert der Semivarianz der Abstandsklasse auf 0;
                sumPowValue = 0;	// Summe der quadratischen Abweichungen
                numValue = 0;		// Anzahl der quadratischen Abweichungen
                semiVarianz = 0;	// Semivarianz der Abstandsklasse für diese Station
            
            
                // Vergleiche den Messwert dieser Station mit jedem anderen:
                // Nimm nun eine Vergleichsstation aus dem Input-Datensatz:
                for (ndx=0; ndx<Map->input_data.length; ndx++){
                
                    // Der Messwert einer Station, soll nicht mit sich selbst verglichen werden:
                    if (idx != ndx){
                
                        // Berechne nun den Abstand zwischen der Station und der Vergleichsstation:
                        distance = calc_distance(Map->input_data.data[idx].lat, 
                                                 Map->input_data.data[idx].lon, 
                                                 Map->input_data.data[ndx].lat,
                                                 Map->input_data.data[ndx].lon);
                        
                        // Liegt die Distanz der beiden Stationen in der aktuell ausgewählten Abstandsklasse, ...
                        // ... so füge sie der Berechnung der Semivarianz hinzu:
                        if ((Map->variogram.classes[kdx].lowerLimit < distance) && (distance <= Map->variogram.classes[kdx].upperLimit)){
                        
                            sumPowValue += pow((Map->input_data.data[idx].value - Map->input_data.data[ndx].value),2);
                            numValue ++;
                            continue;
                            
                        }
                        else{
                        // Liegt die Distanz der beiden Stationen nicht in der aktuell ausgewählten Abstandsklasse, ...
                        // so nimm die nächste Station:
                            continue;
                        }
                
                    }
                    else{
                    // Handelt es sich um die gleiche Station, so nimm die nächste Station:
                        continue;
                    }
                }
                
                // Wurden alle Vergleichsstationen durchlaufen, so berechne den Semivarianzwert der Abstanzklasse und füge ihn der Abstandsklasse hinzu, wenn er größer 0 ist:
                // 1. Prüfe zuerste, ob der Wert der Summe der quatratischen Abweichungen > 0 ist, um einer Division durch 0 zuvor zu kommen: 
                if (sumPowValue > 0){
                
                    // 2. Berechne die Semivarianz:
                    semiVarianz = (double)(sumPowValue / (2*numValue));
                    
                    if (semiVarianz > 0){
                
                        Map->variogram.classes[kdx].variance_values[Map->variogram.classes[kdx].num_variance_values++] = semiVarianz;
                        //Map->variogram.classes[kdx].num_variance_values++;
                    }
                    else{
                        continue;
                    }
                }
                // Weiter mit der nächsten Abstandsklasse:
                continue; 
            }
        }
                    
        // Berechne die Mittelwerte der Semivarianzen einer jeden Abstandsklasse:
        for (idx=0; idx<Map->variogram.numClasses; idx++){
        
            sumClassVarianz = 0.0;
        
            // Besitzt die Abstandsklasse mindestenz eine Semivarianz:
            if (Map->variogram.classes[idx].num_variance_values > 0){
        
                for (kdx=0; kdx<Map->variogram.classes[idx].num_variance_values; kdx++){
            
                    sumClassVarianz += Map->variogram.classes[idx].variance_values[kdx];
            
                }
                
                Map->variogram.classes[idx].variance_avg = sumClassVarianz / (double)(Map->variogram.classes[idx].num_variance_values);
            }
            // Sonst setze den Mittelwert der Semivarianz auf 0.0:  
            else{
            
                Map->variogram.classes[kdx].variance_avg = sumClassVarianz;
                continue;
            }
        }
        
        
        // Hier braucht es eine Funktion, die den sill, nugget und range-Wert bestimmt.
        Map->variogram.sill = 20.0;
        Map->variogram.nugget = 1.123477337;
        Map->variogram.range =  550; 	//(km)      
        
    }
    
    return true;
}



double **create_covariance_matrix(struct usr_map *Map, double **matrix, int rows, int cols){

    /*
        Berechnet anhand der Distanz-Matrix die Kovarianzmatrix.
    */

    int idx, kdx;
    double **covariance_matrix;



    printf("Berechne die Kovarianzmatrix ... ");
    
    covariance_matrix = create_matrix(rows, cols);
    if (covariance_matrix == NULL){
    
        printf("Speicherfehler: Kovarianzmatrix\n");
        return false;
    
    }
    else{
    
        // Berechne die Kovarianzen:
        for (idx=0; idx<rows; idx++){
                
            for (kdx=0; kdx<cols; kdx++){
            
                // Ist der zu berechnende Wert gleich der letzten Spalte und der letzten Zeile,
                // ... so setze ihn auf 0:                

                if ((idx == (rows-1)) && (kdx == (cols-1))){
                
                    covariance_matrix[idx][kdx] = 0;
                    continue;
                    
                }
                // Ist der zu berechnende Wert gleich der letzten Spalte, oder der letzten Zeile,
                // ... so setze ihn auf 1:                
                if ((idx == (rows-1)) || (kdx == (cols-1))){
                
                    covariance_matrix[idx][kdx] = 1;
                    continue;
                }
                else{
                    // Logarithmisch:
                    // Ist die Distanz 0 , dann ist die Kovarianz auch 0:
                    if (matrix[idx][kdx] < EPS){
                        
                        covariance_matrix[idx][kdx] = Map->variogram.nugget;
                    }
                    else{
                        
                        covariance_matrix[idx][kdx] = calc_covariance(matrix[idx][kdx]);
                        
                    }
                }
            }
        }
         
        printf("ok!\n");
        return covariance_matrix;
    }
}


double determinantOfMatrix(double **matrix, int rang){

    int idx, jdx, kdx;
    
    double sum = 0;
    
    // Determinanten der Lower- und Upper- Diagonalmatrizen:
    double DetL = 1.0;
    double DetU = 1.0;
    double Det;
    
    double **L, **U;

    
    // Erstelle die Upper- und Lower-Diagonalmatrizen:
    L = (double **) malloc(rang * sizeof(double *));
    if (L == NULL){
    
        printf("Speicherfehler! Lower-Diagonalmatrize (Zeilen)\n");
        return false;
    
    }
    U = (double **) malloc(rang * sizeof(double *));
    if (L == NULL){
    
        printf("Speicherfehler! Lower-Diagonalmatrize (Zeilen)\n");
        return false;
    
    }
    else{
    
        for (idx=0; idx<rang; idx++){
        
            L[idx] = (double *) malloc(rang * sizeof(double));
            U[idx] = (double *) malloc(rang * sizeof(double));            
        
            if ((L[idx] == NULL) || (U[idx] == NULL)){
            
                printf("Speicherfehler! Diagonalmatrize (Spalten)\n");
                return false;
            
            }
        }
        
        // Berechnung der Determinanten:
        for (idx=0; idx<rang; idx++){
    
            U[idx][idx] = 1;
        }

        for (jdx=0; jdx<rang; jdx++){
    
	    for (idx=jdx; idx<rang; idx++){
	
	        sum=0;
	    
	        for (kdx=0; kdx<jdx; kdx++){
	    
		    sum = sum + L[idx][kdx] * U[kdx][jdx];
			
	        }
	    
	        L[idx][jdx] = matrix[idx][jdx] - sum;
	    }

	    for (idx=jdx; idx<rang; idx++){
	
	        sum = 0;
	    
	        for(kdx=0; kdx<jdx; kdx++){
	    
	            sum = sum + L[jdx][kdx] * U[kdx][idx];
	            
	        }
	        
	        if (L[jdx][jdx] == 0){
	    
	            printf("det(L) nahe 0!\n Es kann nicht durch 0 dividiert werden ...\n");
	            exit(EXIT_FAILURE);
	        }
	        U[jdx][idx] = (matrix[jdx][idx] - sum) / L[jdx][jdx];
	    }
        }
        
        
        // Multiplizieren der Spaltendiagonalen:
        for (idx=0; idx<rang; idx++){
    
            DetL = DetL * L[idx][idx];
            DetU = DetU * U[idx][idx];

        }
            
    
        // Speicher der Diagonalmatrizen freigeben:
        for (idx=0; idx<rang; idx++){
        
            free(L[idx]);
            free(U[idx]);
        }
        free(L);
        free(U);
        
        
        // Berechnen der Determinanten aus det(L) * det(U)
        Det = DetL*DetU;
        
        
        if (isnan(Det)){
            printf("Fehler! Determinante ist nan.");
            return 0;
        }
        else{
            return Det;
        }
    }
    
    return 0;
}




double **inverseOfMatrix(double **matrix, int rows, int cols){

    /*
        Berechnet die Inverse mit Hilfe des Gauß-Jordan Algorithmuses
    */

    int idx, jdx, kdx;
    double temp;
    double **matrix_inv, **matrix_temp;

    printf("Berechne die Inverse der Kovarianzmatrix ... ");

    matrix_inv = create_matrix(rows, cols);
    if (matrix_inv == NULL){
    
        printf("Speicherfehler! Inverse Kovarianzmatrix.\n");
        return NULL;
    
    }
    matrix_temp = create_matrix(rows, cols);
    if (matrix_inv == NULL){
    
        printf("Speicherfehler! temporäre Kovarianzmatrix.\n");
        return NULL;
    
    }    
    else{
    
        // Kopiere den Inhalt der Kovarianzmatrix in die temporäre Matrix:
        for(idx=0; idx<rows; idx++){
    				
            for(jdx=0; jdx<cols; jdx++){
            					
	        matrix_temp[idx][jdx] = matrix[idx][jdx];
	    }
        } 
    
        
        // Setze die Diagonale der Inversen auf 1, sonst 0:
        for(idx=0; idx<rows; idx++){
    				
            for(jdx=0; jdx<cols; jdx++){								
	
	        if(idx == jdx){										
	            matrix_inv[idx][jdx] = 1;
	        }									
	        else{
	            matrix_inv[idx][jdx] = 0;
	        }
	    }
        }


        // 
        for(kdx=0 ;kdx<rows; kdx++){
    														
	    temp=matrix_temp[kdx][kdx];
											
            for(jdx=0; jdx<cols; jdx++){
        
	        matrix_temp[kdx][jdx] /= temp;
	        matrix_inv[kdx][jdx] /= temp;
	    }
														
	    for(idx=0; idx<rows; idx++){
	
	        temp=matrix_temp[idx][kdx];
	    									
	        for(jdx=0; jdx<cols; jdx++){
	    												
		    if(idx == kdx){
		        break;
		    }									
		    matrix_temp[idx][jdx] -= matrix_temp[kdx][jdx] * temp;
								
		    matrix_inv[idx][jdx] -= matrix_inv[kdx][jdx] * temp;
	        }
	    }
        }
        
        for(idx=0; idx<rows; idx++){
    				
            free(matrix_temp[idx]);
        }         
        free(matrix_temp);
        
        
        printf("ok\n");        
        return matrix_inv;
    }
}





int interpolate_raster(struct usr_map *Map){

    /*
        Quellen: https://youtu.be/vhVDcCNNMWE?si=3oJl7ghWn3U0SECQ
    
        Es gilt:
        sig ... sigma (sill)
        a   ... nugget
        r   ... range
        h   ... Abstand
    
        1: Berechne die Distanz (h) eines jeden Punktes zum gesuchten Punkt.
           
        2: Berechne aus diesem Abstandsvektor, die Kovarianzen.
        3: Multipliziere den Kovarianzvektor mit der Inversen der Kovarianzmatrix
        4: Multipliziere die daraus resultierenden Gewichtungen mit den Werten der Punkte ...
           ... und summiere dies auf.
           
    */
    

    int idx, jdx, kdx;
    double *cov_vector, *weights_vector;
    double sum;
    
    cov_vector = create_vector(Map->input_data.length+1);
    if (cov_vector == NULL){
    
        printf("Speicherfehler\n");
        return false;
    
    }
    else{
    
        printf("Interpoliere ... ");
        
        // Geh über jeden Punkt des Input-Rasters:
        for (idx=0; idx<(Map->rows); idx++){
            for (jdx=0; jdx<(Map->cols); jdx++){
        
                // Liegt kein Messwert für den Rasterpunkt vor...
                if (Map->raster_in[idx][jdx].value < 0){
            
                    sum = 0;
            
                    // Berechne nun für diesen Punkt die Distanz zu jedem Messpunkt der Input-Daten.
                    for (kdx=0; kdx<Map->input_data.length+1; kdx++){
                
                        if (kdx != Map->input_data.length){
                            cov_vector[kdx] = calc_covariance(calc_distance(Map->raster_out[idx][jdx].lat,
                                                                            Map->raster_out[idx][jdx].lon, 
                                                                            Map->input_data.data[kdx].lat,
                                                                            Map->input_data.data[kdx].lon));
                        }
                        else{
                            cov_vector[kdx] = 1;
                        }
                    }
                
                    weights_vector = multiplyMatrixVector(Map->covariance_matrix_inv, 
                                                          cov_vector, 
                                                          Map->input_data.length+1, 
                                                          Map->input_data.length+1);
                                                          
                                                          
                    if (Map->weights_correction){
                    
                        correct_negative_weights(weights_vector, Map->input_data.length+1);
                        return false;
                    }
                    /*
                        
                        2. Berechne die durschnittl. Kovarianz zwischen dem Interpolationspunkt und den Messpunkten mit neg. Gewichten:
                           cov_avg_neg = sum(u - ui)/n
                           d.h. 2.1. Berechne die Distanz zwischen dem zu interpolierenden Punkt und den Messpunkten mit neg. Gewichten
                                2.2. Berechne aus diesen Abständen die Kovarianz und bilde davon den Mittelwert
                        3. Korrigiere die Gewichte nun folgendermaßen:
                           1. neue Gewichtung = alte Gewichtung
                           2. if alte Gewichtung < 0 dann neue Gewichtung = 0
                           3. if (alte Gewichtung > 0) && (Kovarianz zwischen zu interpolierenden Punkt und Messwert < cov_avg_neg) && lam < lam_avg_neg
                              dann lam_new = 0
                    
                    
                    */
                    
                    // Wert des Rasterpunktes mit 0 initialisieren:
                    Map->raster_out[idx][jdx].value = 0;
                    
                    // Berechnen des Interpolationswertes, als die Summe über die Gewichte * Werte.
                    for (kdx=0; kdx<Map->input_data.length; kdx++){
                
                        sum += (weights_vector[kdx] * Map->input_data.data[kdx].value);
                    }
                    
                    // Interpolierten Wert dem Rasterpunkt zuweisen:
                    Map->raster_out[idx][jdx].value = sum;
                    
                }
                else{
            
                    Map->raster_out[idx][jdx] = Map->raster_in[idx][jdx];
            
                }   
            }    
        }
        
        printf("ok\n");
        free(cov_vector);
        free(weights_vector);
        return true;
    }   

};





void correct_negative_weights(double *weights_vector, int length){

    int idx, cnt=0;
    double sum=0;
    double avg_abs_neg_weights;				// absolten Durchschnittswertes aller negativen Gewichte
    

    // 1. Berechne den absoluten Durchschnittswert der negativen Abweichungen (lam_avg_neg) über den "weights_vector"
    //    lam_avg_neg = sum(|-lam_0|, |-lam_1|, ... , |-lam_n|)/n
    for (idx=0; idx<length; idx++){
    
        if (weights_vector[idx] < 0){
            sum += fabs(weights_vector[idx]);
            cnt++;
        }
    }
    avg_abs_neg_weights = (sum/cnt);
    printf("%.3f\n", avg_abs_neg_weights);
    
    
    

}





double calc_distance(double latA, double lonA, double latB, double lonB){

    // Berechnet die Entfernung zwischen 2 Punkten auf einer Kugelfläche mit einem Radius von 6365.265 km.
    // Der Radius entspricht dem Erdradius auf 51° N.
    // Die Input-Argumente muessen in Dezimalgrad vorliegen.

    const double RADIUS_EARTH = 6365.265;

    // Punkt A: 
    double latA_rad = (latA/180.0) * M_PI;
    double lonA_rad = (lonA/180.0) * M_PI;

    // Punkt B:
    double latB_rad = (latB/180.0) * M_PI;
    double lonB_rad = (lonB/180.0) * M_PI;

    return (double)RADIUS_EARTH * acos( (sin(latA_rad) * sin(latB_rad) ) + ( cos(latA_rad) * cos(latB_rad) * ( cos(lonB_rad - lonA_rad) ) ) );

}



double calc_covariance(double distance){

    /*
        Berechnet die Kovarianz eines Punktes anhand des Abstands zu einem anderen Punkt.
        Bei Änderung der Funktion, bitte hier korrigieren:
    */

    return (7.3086*(log(distance))) - 27.468;


}

double *multiplyMatrixVector(double **matrix, double *vector_in, int rows, int cols){

    /*
        Multipliziert eine Matrize mit einem Vektor.
    */

    int idx, jdx, kdx;

    double *vector_out;
    double sum=0;
    
    
    
    vector_out = create_vector(rows);
    if (vector_out == NULL){
    
        printf("Speicherfehler!\n");
        return false;
    }
    else{
        
        // Multiplikation:
        for (idx=0; idx<rows; idx++){
                    
            vector_out[idx] = 0;
                
            for (jdx=0; jdx<cols; jdx++){
                
                vector_out[idx] += (matrix[idx][jdx] * vector_in[jdx]);
                    
            }
            
        }
        
        return vector_out;
    }
    
    return NULL;

}






int multiplyMatrix(double **matrixA, double **matrixB, int rows, int cols){

    /*
        Multipliziert zwei Matrizen miteinander.
    */


    int idx, jdx, kdx;
    
    double **matrixP;
    double sum=0;
    
    
    
    matrixP = create_matrix(rows, cols);
    if (matrixP == NULL){
    
        printf("Speicherfehler!\n");
        return false;
    }
    else{
        
        // Multiplikation:
        for (idx=0; idx<rows; idx++){
        
            for (jdx=0; jdx<cols; jdx++){
            
                matrixP[idx][jdx] = 0;
                
                for (kdx=0; kdx<cols; kdx++){
                
                    matrixP[idx][jdx] += (matrixB[kdx][jdx] * matrixA[idx][kdx]);
                    
                }
            }
        }
                        
        
        //show_matrix(matrixP, 20, 20);
        
        for (idx=0; idx<rows; idx++){
        
            free(matrixP[idx]);
        
        }
        free(matrixP);
    }
    
    return true;
    
}




void outputMatrixCSV(double **matrix, char *filename, int rows, int cols){


    int idx, jdx, kdx;
    char zahlText[20];
    FILE *fp;


    fp = fopen(filename,"w");
    if (fp == NULL){
    
        printf("Dateifehler! Matrix (output)\n");
    
    }
    else{
    
        for (idx=0; idx<rows; idx++){
        
            for (jdx=0; jdx<cols; jdx++){
            
                sprintf(zahlText,"%.8f", matrix[idx][jdx]);
                
                for (kdx=0; kdx<(int)strlen(zahlText); kdx++){
                
                    if (zahlText[kdx] == '.'){
                    
                        zahlText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp,"%s;",zahlText);
            }
            fprintf(fp,"%s","\n");
        }
    }


    fclose(fp);

}


void outputRasterCSV(struct usr_data_point **raster, char *output_dir, char *filename, int rows, int cols){

    /*
        Ausgabe:
        - CSV-Datei in Form eines "rows x cols" Rasters mit den interpolierten Werten.
        - CSV-Datei in Form eines "rows x cols" Rasters mit den Werten der geogr. Breite eines jeden Rasterpunktes.
        - CSV-Datei in Form eines "rows x cols" Rasters mit den Werten der geogr. Länge eines jeden Rasterpunktes.
    */



    int idx, jdx, kdx;
    char valueText[20], latText[20], lonText[20];
    char path[100];

    
    FILE *fp_values, *fp_lat, *fp_lon;


    // Filepointer für die Rasterwerte:
    fp_values = fopen(strcat(strcpy(path, output_dir), filename),"w");
    
    // Filepointer für die Breitengrade der Rasterpunkte
    fp_lat = fopen(strcat(strcpy(path, output_dir), "lat.csv"),"w");
       
    // Filepointer für die Längengrade der Rasterpunkte    
    fp_lon = fopen(strcat(strcpy(path, output_dir), "lon.csv"),"w");
    
    
    
    
    if ((fp_values == NULL) || (fp_lat == NULL) || (fp_lon == NULL)){
    
        printf("Dateifehler!\nDefinierter Filepointer ist NULL.\n");
    
    }
    
    else{
    
        printf("Schreibe csv-Ausgabedateien ... ");
    
        for (idx=0; idx<rows; idx++){
        
            for (jdx=0; jdx<cols; jdx++){
            
                // Lese den Wert in ein Textfeld ein:
                sprintf(valueText,"%.3f", raster[idx][jdx].value);
                
                // Lese die geogr. Breite in ein Textfeld ein:
                sprintf(latText, "%.4f", raster[idx][jdx].lat);
                
                // Lese die geogr. Länge in ein Textfeld ein:
                sprintf(lonText, "%.4f", raster[idx][jdx].lon);
                
                
                
                
                // Schreibe die CSV-Datei der interpolierten Werte:
                for (kdx=0; kdx<(int)strlen(valueText); kdx++){
                
                    if (valueText[kdx] == '.'){
                    
                        valueText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp_values,"%s",valueText);

                // Schreibe die CSV-Datei der geogr. Breite:
                for (kdx=0; kdx<(int)strlen(latText); kdx++){
                
                    if (latText[kdx] == '.'){
                    
                        latText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp_lat,"%s",latText);

                // Schreibe die CSV-Datei der geogr. Breite:
                for (kdx=0; kdx<(int)strlen(lonText); kdx++){
                
                    if (lonText[kdx] == '.'){
                    
                        lonText[kdx] == ',';
                        break;
                    }
                
                }
                fprintf(fp_lon,"%s",lonText);
                
                if (jdx != (cols-1)){
                
                    fprintf(fp_values,"%s",";");
                    fprintf(fp_lat,"%s",";");    
                    fprintf(fp_lon,"%s",";");
                    
                }
                else{
                
                    fprintf(fp_values, "%s", "\n");
                    fprintf(fp_lat, "%s", "\n");    
                    fprintf(fp_lon, "%s", "\n");                
                
                }
            }
        }
        
        fclose(fp_values);
        fclose(fp_lat);
        fclose(fp_lon);
        
        printf("ok\n");
    }
}




void check_matrix(double **matrix, int rows, int cols){

    /*
        Prüft Matrizen auf Fehler:
        1. Prüfung auf nan Werte
    
    */
    
    int idx, jdx;
    
    int cnt_nan = 0;
    bool found_nan = false;
    
    double max, min;
    
    
    printf("Prüfe Matrix:\n");
    printf("-----------------------\n");
    printf("Prüfe auf nan ... ");
    
    for (idx=0; idx<rows; idx++){
    
        for (jdx=0; jdx<cols; jdx++){
        
            if (isnan(matrix[idx][jdx])){
            
                cnt_nan++;
                found_nan = true;
            }
        }
    }
    
    if (found_nan){
    
        printf("nicht bestanden!\n");
        printf("Es wurden %d NaN Werte gefunden!\n", cnt_nan);
    
    }
    else{
    
         printf("ok!\n");       
    
    }
    
    printf("\n");
    
    printf("-----------------------\n");    
    printf("Zusammenfassung:\n");

    max=matrix[0][0];
    min=matrix[0][0]; 
       
    for (idx=0; idx<rows; idx++){
    
        for (jdx=0; jdx<cols; jdx++){
        
            if (matrix[idx][jdx] > max){
            
                max=matrix[idx][jdx];
            }
            if (matrix[idx][jdx] < min){
            
                min=matrix[idx][jdx];
            }
        }
    }    
    printf("Maximum: %.6f\n", max);
    printf("Minimum: %.6f\n", min);    
    
    printf("-----------------------\n");
}



    
void show_variogram_data(struct usr_map *Map){

    int idx;
    
    
    printf("Abstandsklassen:\n");
    printf("%3s%10s%10s%15s\n","id", "u.Grenze", "o.Grenze", "Varianz (AVG)");
    for (idx=0; idx<Map->variogram.numClasses; idx++){
    
        printf("%3d%10d%10d%15.3f\n", idx,
                                      Map->variogram.classes[idx].lowerLimit,
                                      Map->variogram.classes[idx].upperLimit,
                                      Map->variogram.classes[idx].variance_avg);
        
    }
    printf("-----------------------\n");
    printf("nugget: %.3f\n", Map->variogram.nugget);
    printf("sill: %.3f\n", Map->variogram.sill);
    printf("range: %.3f\n", Map->variogram.range);
    printf("-----------------------\n\n");

}


void show_map_info(struct usr_map *Map){

    printf("############### INFORMATION ##############\n\n");
    printf("   Geogr. Breite: %.3f (Minimum)\n",Map->minLat);
    printf("   Geogr. Breite: %.3f (Maximum)\n",Map->maxLat);   
    printf("   Geogr. Laenge: %.3f (Minimum)\n",Map->minLon);
    printf("   Geogr. Laenge: %.3f (Maximum)\n\n",Map->maxLon);    
    
    printf("   Anzahl der Felder: %d\n", (Map->rows)*(Map->cols));
    printf("   Zeilen: %d\n", Map->rows);
    printf("   Spalten: %d\n\n", Map->cols); 
    
    printf("   Aufloesung (Breite): %.3f\n", Map->latRes);
    printf("   Aufloesung (Laenge): %.3f\n", Map->lonRes); 
    printf("   Aufloesung (Breite, km): %.3f\n", Map->latMetRes);
    printf("   Aufloesung (Laenge, km): %.3f\n\n", Map->lonMetRes);  
    
    printf("   Genutzter Speicher: %lu Byte\n\n", sizeof(*Map));  
    printf("##########################################\n\n");          
}





void show_matrix(double **matrix, int rows, int cols){

    /*
        Gibt die übergebene Matrix auf stdout aus.    
    */

    int idx, jdx;

    for (idx=0; idx<rows; idx++){
    
        for (jdx=0; jdx<cols; jdx++){
        
            printf("%8.3f", matrix[idx][jdx]);   
        
        }
        
        printf("\n");
    }
    printf("\n");
}





void show_input_data(struct usr_map *Map){

    int idx;
    
    
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


void show_raster(struct usr_data_point **raster, int *rows, int *cols){

    int idx, kdx;
    
    
    for (idx=0; idx<*rows; idx++){
    
        for (kdx=0; kdx<*cols; kdx++){
        
            //geogr_laenge
            //printf("%.2f ", raster[idx][kdx].lon);
            
            //geogr_breite          
            //printf("%.2f ", raster[idx][kdx].lat);
            
            //werte          
            printf("%.0f ", raster[idx][kdx].value);           
        }
        printf("\n");
    
    }

}


void free_raster(struct usr_map *Map){

    int idx;
    
    
    // Leeren es Eingaberasters:
    for (idx=0; idx<(Map->rows); idx++){
    
        free(Map->raster_in[idx]);
    
    }
    
    free(Map->raster_in);
    printf("Eingaberaster: Speicher freigegeben!\n");
    
    
    //Leeren der Distanzmatrix:
    for (idx=0; idx<Map->input_data.length; idx++){
    
        free(Map->distance_matrix[idx]);
    }
    free(Map->distance_matrix);
    printf("Distanzmatrix: Speicher freigegeben!\n");   
    
    // Leeren der Kovarianzmatrix:
    for (idx=0; idx<Map->input_data.length+1; idx++){
    
        free(Map->covariance_matrix[idx]);
    }
    free(Map->covariance_matrix);
    printf("Kovarianzmatrix: Speicher freigegeben!\n");

     // Leeren der Inversen der Kovarianzmatrix:
    for (idx=0; idx<Map->input_data.length+1; idx++){
    
        free(Map->covariance_matrix_inv[idx]);
    }
    free(Map->covariance_matrix_inv);
    printf("Inverse der Kovarianzmatrix: Speicher freigegeben!\n");   
    
    return;
}



void free_vector(struct usr_map *Map){

    free(Map->input_data.data);
    printf("Eingabedaten: Speicher freigegeben!\n");
    
    
    free(Map->variogram.classes);
    printf("Variogrammklassen: Speicher freigegeben!\n");
}





