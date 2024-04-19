
#include <stdio.h>
#include <math.h>


#define M_PI 3.14159265358979323846
#define KM 111.319


int main(){

    double mySiteLat = 52.40730742459821;
    double mySiteLon = 13.05489924477713;
    double othSiteLat = 52.51869407973752;
    double othSiteLon = 13.488572241699332;

    double mySiteLat_rad = (52.40730742459821/180) * M_PI;
    double mySiteLon_rad = (13.05489924477713/180) * M_PI;
    double othSiteLat_rad = (52.51869407973752/180) * M_PI;
    double othSiteLon_rad = (13.488572241699332/180) * M_PI;   

    double dist_py, dist_ci;

    dist_py = sqrt(pow((mySiteLat - othSiteLat),2) + pow((mySiteLon - othSiteLon),2)) * KM;

    dist_ci = 6371.0008 * acos((sin(mySiteLat_rad) * sin(othSiteLat_rad)) + (cos(mySiteLat_rad) * cos(othSiteLat_rad) * (cos(othSiteLon_rad - mySiteLon_rad))));

    printf("Pythagoras: %.3f\nKreis: %.3f\n", dist_py, dist_ci);

    return 0;

}
