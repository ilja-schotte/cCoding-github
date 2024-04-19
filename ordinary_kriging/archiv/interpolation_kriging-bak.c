
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <string.h>
    #include <stdbool.h>
#endif

#define NO_VALUE -1.0
#define M_PI 3.14159265358979323846

struct usr_variogram{

    struct usr_vario_class *classes;	// Feld von Abstandsklassen
    int numClasses;			// Anzahl der Abstandsklassen
    int distInterval;			// Abstandsintervall der Abstandsklassen

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
    
    // Eingabe-Daten:
    struct usr_dataset input_data;
    
    // Eingabe-Raster:
    struct usr_data_point **raster_in;
    
    // Ausgabe-Raster:
    struct usr_data_point **raster_out;
    
    // Variogramm:
    struct usr_variogram variogram;
};






// Deklaration: Funktion
struct usr_data_point **create_2D_raster(int *rows, int *cols);

int fill_raster_with_default_data(struct usr_map *zgr);
int input_csv_data(struct usr_map *zgr);
int fill_raster_with_input_data(struct usr_map *zgr);
int create_variogram(struct usr_map *zgr);
double calc_distance(double latA, double lonA, double latB, double lonB);


                 
void show_raster(struct usr_data_point **zgr, int *rows, int *cols);
void show_map_info(struct usr_map *zgr);
void show_input_data(struct usr_map *zgr);
void show_variogram_data(struct usr_map *zgr);

void free_raster(struct usr_map *zgr);
void free_vector(struct usr_map *zgr);


int main(int argc, char *argv[]){

    struct usr_data_point **zgr_in, **zgr_out;   
    
    // Deklarieren/Initialisieren des "Objekts" Map;
    struct usr_map Map = {.minLat = 47.000,	// minLat ("minimum latitude") : min. Dezimalgrad (Breite) 
                          .maxLat = 55.000,	// maxLat ("maximum latitude") : max. Dezimalgrad (Breite)
                          .minLon = 5.000,	// minLon ("minimum longitude") : min. Dezimalgrad (Länge) 
                          .maxLon = 16.000,	// maxLon ("maximum longitude") : max. Dezimalgrad (Länge)
                          .latRes = 0,		// 0,04545 => Auflösung von ca. 5km / 0,00909 => Auflösung von ca. 1km
                          .lonRes = 0,		// 0,04545 => Auflösung von ca. 5km / 0,00909 => Auflösung von ca. 1km
                          .latMetRes = 0,	// Auflösung zwischen zwei benachbarten Punkten (geogr. Breite)
                          .lonMetRes = 0,	// Auflösung zwischen zwei benachbarten Punkten (geogr. Länge)
                          .rows = 900,		// 900 => Auflösung con 1km
                          .cols = 0,
                          .variogram = {.distInterval = 100},	// Intervallbreite der Abstandsklassen.
                          .maxDist = 900	// Maximale Distanz zweier Punkte in Deutschland.
                         };

    Map.cols = ceil(Map.rows * (int)(Map.maxLon - Map.minLon) / (Map.maxLat - Map.minLat));

    

    

    
    
    Map.raster_in = create_2D_raster(&(Map.rows), &(Map.cols));
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
            
                // Ordne die Messpunkte den Rasterpunkten zu:
                fill_raster_with_input_data(&Map);
                
                // Erstelle aus den Messwerten ein Variogramm und die dazugehörige Funktion:
                create_variogram(&Map);
                
                // Zeige Informationen zu Variogramm:
                show_variogram_data(&Map);
                
                // Zeige den Input-Datensatz:
                //show_input_data(&Map); 
                               
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
struct usr_data_point **create_2D_raster(int *rows, int *cols){

    int idx;
    
    struct usr_data_point **zgr;
    
    
    zgr = (struct usr_data_point **) malloc(*rows * sizeof(struct usr_data_point*));
    if (zgr == NULL){
    
        return NULL;
    }
    else{
    
        for (idx=0; idx<*rows; idx++){
        
            zgr[idx] = (struct usr_data_point *) malloc(*cols * sizeof(struct usr_data_point));        
            if (zgr[idx] == NULL){
            
                return NULL;
            
            }
        }
        printf("Matrix reserviert!\n");
        return zgr;
    }
    return NULL;    
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
    
    
    fp = fopen("tagessummen.csv", "r");
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
        
            // Füllen der Feld-Indizes:
            Map->raster_in[idx][kdx].row_idx = idx;
            Map->raster_in[idx][kdx].col_idx = kdx;
            
            // Füllen der Koordinaten:
            Map->raster_in[idx][kdx].lon = (Map->minLon) + kdx*(Map->lonRes);
            Map->raster_in[idx][kdx].lat = (Map->maxLat) - idx*(Map->latRes);
            
            // Füllen des Wertes:
            Map->raster_in[idx][kdx].value = NO_VALUE;
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
    
    
    // Abstandsklassen erstellen (max. Abstand / Intervallgröße):
    Map->variogram.numClasses = (int)(Map->maxDist / Map->variogram.distInterval);

    
    Map->variogram.classes = (struct usr_vario_class *) malloc(Map->variogram.numClasses * sizeof(struct usr_vario_class));
    if (Map->variogram.classes == NULL){
    
        printf("Speicherfehler!\n");
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
                    
        // Zeige die Samivarianzwerte:
        for (kdx=0; kdx<Map->variogram.numClasses; kdx++){
            printf("(%d, %d): ",Map->variogram.classes[kdx].lowerLimit, Map->variogram.classes[kdx].upperLimit);
            for (ndx=0; ndx<Map->variogram.classes[kdx].num_variance_values; ndx++){
                printf("%.3f ", Map->variogram.classes[kdx].variance_values[ndx]);
            }
            printf("(%d)\n", Map->variogram.classes[kdx].num_variance_values);
        }
    }
    
    return true;
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
    
    for (idx=0; idx<(Map->rows); idx++){
    
        free(Map->raster_in[idx]);
    
    }
    
    free(Map->raster_in);
    printf("Eingaberaster: Speicher freigegeben!\n");
    return;
}



void free_vector(struct usr_map *Map){

    free(Map->input_data.data);
    printf("Eingabedaten: Speicher freigegeben!\n");
    
    
    free(Map->variogram.classes);
    printf("Variogrammklassen: Speicher freigegeben!\n");
}





