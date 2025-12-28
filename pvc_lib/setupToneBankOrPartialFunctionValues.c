#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

void setupToneBankOrPartialFunctionValues(
    int numTones,
    float dur,
    float delayTimes[], 
    float timeRateScalers[],
    struct func functions[]
)
{
    // FOR EACH TONE OR BANK, GET THE FUNCTION VALUE FROM THE CORRESPONDING
    // FUNCTION IN THE ARRAY OF FUNCTIONS, USING THE CORRESPONDING 
    // FUNCTION-DERIVED OR CONSTANT DELAY TIME.

    int toneNumber ;
    float thisTime ; 

    for( toneNumber = 0; toneNumber < numTones; toneNumber++ ){
        thisTime = t - delayTimes[ toneNumber ] ; 
        if( thisTime > 0. ) thisTime *= timeRateScalers[ toneNumber ]  ; 
        functions[ toneNumber ].A[ 0 ] = fval( &functions[ toneNumber ], dur, thisTime ); 
    } ; 

} ; 
