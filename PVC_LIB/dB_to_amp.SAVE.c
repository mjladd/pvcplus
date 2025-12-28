#include <stdio.h>
#include <math.h>
#include "pv.h"

float dB_to_amp( float dB ){
    
    static int first=1;
    static float *A,   mult1  ;
    float y1,  y2,  prop,  frac,  val,  temp,  div ; 
    int iy1,  i ; 
    if( first ){
	// MAKE ARRAY
	fvec( A,  5000 ) ;
	// FILL ARRAY
	for( i = 0 ; i < 5000; i++ ){ 
	    temp = ( 192. * ( (float) i / 4999. ) ) - 96. ; 
	    A[ i ] = pow( (double) 10.0, (double) (temp / 20.) );	
	}
	first = 0 ;
	mult1 = 4998. / 192. ; 
    }
        
    
    prop = mult1 * ( dB + 96. ) ;
    if( prop > 4998. ) {
	prop = 4998. ; 
	fprintf( stderr,  "\n%f GAIN EXCEEDS TABLE!",  dB ) ; 
    }
    if( prop < 0. ) {
	prop = 0. ;  
	fprintf( stderr,  "\n%f GAIN EXCEEDS TABLE!",  dB ) ; 
    }
    iy1 = (int) prop ; 
    frac = prop - (float) iy1 ; 
    y1 = A[ iy1 ] ; 
    y2 = A[ iy1  + 1] ; 
    val = y1 + (frac * (y2 - y1)) ; 
    return( val ) ; 

}
