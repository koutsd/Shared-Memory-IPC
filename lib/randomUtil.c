#include <stdlib.h>
#include <math.h>
#include "../headers/randomUtil.h"

int uniformDist_rand(int rangeLow, int rangeHigh) {
    int range = rangeHigh - rangeLow + 1,
        x = RAND_MAX / range,
        myRand;

    do {
        myRand = rand(); 
    } while(x * range < myRand);
    
    return myRand/x + rangeLow;
}

double exponentialDist_rand(double l) {
    double u = rand() / (RAND_MAX + 1.0);

    return - log(1 - u) / l;
}