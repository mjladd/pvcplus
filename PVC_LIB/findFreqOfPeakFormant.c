#include <stdio.h>
#include <math.h>
#include "pv.h"

int findFreqOfPeakFormant( 
    float SP[],  
    int N, 
    float *lowf, 
    float *hif,
    float *fundamental, 
    float *peakamp,
    float *freq
)
{    

    int i, i1, i2, first ;
    int flag;  
    
    first = 1 ; 

    // TURN FREQ BOUNDS INTO INDECES

//fprintf( stderr, "\nN = %d, lowf = %f, hif = %f, fundamental = %f", N,  *lowf, *hif, *fundamental ) ; 

    i1 = 1 +  ( 2 * (int) ( *lowf / *fundamental ) ) ; 
    i2 = 1 +  ( 2 * (int) ( *hif / *fundamental ) ) ; 
    if( i2 >= N ) i2 = N - 1 ; 

    if( i2 == i1 ) i2 = i1 + 2 ; 

//fprintf( stderr, "i1: %d i2: %d\n", i1, i2 ) ; 

// FIND FREQ OF THE STRONGEST BIN.

	for( i = i1; i <= i2 ; i+= 2){
            if( first == 1 ){
		*peakamp = SP[i - 1] ; 
		*freq = SP[i] ;
		first = 0 ;  
            }else{
		if( SP[i - 1] > *peakamp )
		{
			*peakamp = SP[i - 1] ; 
			*freq = SP[i] ; 
		} ; 
            } ; 
	} ; 

	return( *freq ) ; 

}
