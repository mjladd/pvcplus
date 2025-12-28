#include <stdio.h>
#include <math.h>
#include "pv.h"

float semitones_to_mult( float semidev ){
    
    static int first=1;
    static float *A,  mindev,  maxdev,  mult  ;
    float y1,  y2,  prop,  frac,  val,  temp ; 
    int iy1,  i ; 
    if( first ){
	// MAKE ARRAY
	fvec( A,  5000 ) ;
	// FILL ARRAY
	for( i = 0 ; i < 5000; i++ ){ 
	    temp = ( 144. * ( (float) i / 4999. ) ) - 72. ; 
	    A[ i ] = ((float) pow( (double) 2.0, (double) ( temp / 12.0 ) )) ;
	}
	first = 0 ;
	mult = 4999. / 144. ; 
    }
    
    prop = mult * ( semidev + 72. ) ;
    if( prop > 4998. ) prop = 4998. ; 
    if( prop < 0. )  prop = 0. ;  
    iy1 = (int) prop ; 
    frac = prop - (float) iy1 ; 
    y1 = A[ iy1 ] ; 
    y2 = A[ iy1  + 1] ; 
    val = y1 + (frac * (y2 - y1)) ; 
    return( val ) ; 

}
