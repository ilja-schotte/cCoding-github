
#ifdef __unix__
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
#endif











int main(int argc, char *argv[]){

    int idx, jdx, kdx, ldx;
    
    double x_avg=0, x2_avg=0, x3_avg=0, y_avg=0;
    double x_dev, x2_dev, x3_dev, y_dev;
    
    
    double orig_data[13][2] = {{1, 14},
                               {0, 8},
                               {3, 30},
                               {3, 48},
                               {4, 49},
                               {2, 24},
                               {5, 81},
                               {10, 45},
                               {9, 83},
                               {6, 99},
                               {1, 8},
                               {10, 35},
                               {8, 100}};
                          
    double data[13][4];
    double z_standard[13][4];
    double covarianzPred[3][3];
    double interCorrPred[3][3];

    
    
    printf("Originaldaten:\n");
    printf("-------------------------------\n\n");
                   
    printf("%3s%9s%9s\n","idx", "x", "y");               
    for (idx=0; idx<13; idx++){
    
        printf("%3d %8.3f %8.3f\n", idx, orig_data[idx][0], orig_data[idx][1]);
    
    }
    printf("-------------------------------\n\n\n");


    
    printf("Berechnung von x, x² und x³\n");
    printf("-------------------------------\n\n");
    
    printf("%3s%9s%10s%10s%9s\n","idx", "x", "x²", "x³", "y");                       
    for (idx=0; idx<13; idx++){
    
        data[idx][0] = orig_data[idx][0];
        data[idx][1] = pow(orig_data[idx][0],2);
        data[idx][2] = pow(orig_data[idx][0],3);
        data[idx][3] = orig_data[idx][1];
        
        printf("%3d %8.3f %8.3f %8.3f %8.3f\n", idx, data[idx][0], data[idx][1], data[idx][2], data[idx][3]);
    }



    printf("Berechnung der z-Standardisierung von x, x², x³ und y\n");
    printf("-------------------------------\n\n");

    // Zuerst muss das arith. Mittel und die Standardabweichung für jede Spalte berechnet werden.    
    for (idx=0; idx<13; idx++){
    
        x_avg += data[idx][0];
        x2_avg += data[idx][1];
        x3_avg += data[idx][2];
        y_avg += data[idx][3];
    
    }
    x_avg /= 13;
    x2_avg /= 13;
    x3_avg /= 13;
    y_avg /= 13;
    
    for (idx=0; idx<13; idx++){
    
        x_dev += pow((data[idx][0] - x_avg),2);
        x2_dev += pow((data[idx][1] - x2_avg),2);
        x3_dev += pow((data[idx][2] - x3_avg),2);
        y_dev += pow((data[idx][3] - y_avg),2);
    }
    
    x_dev = sqrt(x_dev /= 13);
    x2_dev = sqrt(x2_dev /= 13);
    x3_dev = sqrt(x3_dev /= 13);
    y_dev = sqrt(y_dev /= 13);    
    
    // Berechnung der z-standardisierung:
    
    for (idx=0; idx<13; idx++){
    
        z_standard[idx][0] = (data[idx][0] - x_avg) / x_dev;
        z_standard[idx][1] = (data[idx][1] - x2_avg) / x2_dev;        
        z_standard[idx][2] = (data[idx][2] - x3_avg) / x3_dev;
        z_standard[idx][3] = (data[idx][3] - y_avg) / y_dev;                
    }

    printf("%3s%9s%10s%10s%9s\n","idx", "z(x)", "z(x²)", "z(x³)", "z(y)");                       
    for (idx=0; idx<13; idx++){
    
        printf("%3d %8.3f %8.3f %8.3f %8.3f\n", idx, z_standard[idx][0], z_standard[idx][1], z_standard[idx][2], z_standard[idx][3]);
    }
    
    
    
    
    printf("Berechnung der Korrelationsmatrix der Prädiktoren x, x², x³\n");
    printf("-------------------------------\n\n");   

    // https://www.youtube.com/watch?v=uBRXqlZr11I&ab_channel=Methodenlehre
    // https://www.youtube.com/watch?v=cLr0zg8-bgQ&ab_channel=Methodenlehre

    for (idx=0; idx<3; idx++){
    
        for (jdx=0; jdx<3; jdx++){
        
            for (kdx=0; kdx<13; kdx++){
                covarianzPred[idx][jdx] += (data[kdx][idx] - x_avg) * (data[jdx][kdx] - x_avg);
            }
        }
    }

    for (idx=0; idx<3; idx++){
    
        printf("%.3f %.3f %.3f\n", interCorrPred[idx][0], interCorrPred[idx][1], interCorrPred[idx][2]);
    
    }



    return 0;

}
