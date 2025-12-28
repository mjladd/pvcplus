#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>



void getGlobalFunctionValues(
    float values[],  
    int numTones,
    float dur,
    float delayTimes[],
    float timeRateScalers[], 
    struct func *function
)
{
    int toneNumber ;
    float thisTime ; 

    // EVALUATE THE FUNCTION "function" FOR the "numTones" NUMBER OF 
    // DElAY TIMES COMING FROM THE "delayTime" functions, 
    // PUTTING THE VALUES IN values.  

    for( toneNumber = 0; toneNumber < numTones; toneNumber++ ){
        thisTime = t - delayTimes[ toneNumber ] ; 
        if( thisTime > 0. ) thisTime *= timeRateScalers[ toneNumber ]  ; 
        values[ toneNumber ] = function->A[ 0 ] = fval( function, dur, thisTime ); 
    } ; 

} ; 
