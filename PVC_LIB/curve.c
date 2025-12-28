#include <stdio.h>
#include <math.h>
#include "pv.h"

float curve(float  V1,float  V2,float  n, float   warp )
{
    float v ; 
    // CURVE RETURNS  THE (0-1) NORMALIZED VALUE n
    //  WARPED BY THE EXPONENTIAL CURVE THROUGH warp, 
    // MAPPED INTO THE RANGE BETWEEN V1 AND V2

    if( warp == 0. ){
	v =  V1 + (V2 - V1) * n ;
    }else{
	v =  V1 + (V2 - V1) * (1. - exp((double)(n * warp)))/(1. - exp( (double) warp)) ; 
    }

    return( v ) ; 
}

/*
float curve(float  V1,float  V2,float  n, float   warp ){
    
    static int first=1;
    static float *A,  mult  ;
    float y1,  y2,  prop,  frac,  val, val1,  val2,  temp,  n_warp,  temp_warp ; 
    int iy1,  i ; 

    if( first ){
	// MAKE ARRAY
	fvec( A,  5000 ) ;
	// FILL ARRAY
	for( i = 0 ; i < 5000; i++ ){ 
	    temp =  ( 50. * ( (float) i / 4999. ) ) - 25. ; 
	    A[ i ] =  1. - (float) exp(  (double) temp ) ;
	}
	first = 0 ;
//	mult = 4998.99999 / 50. ; 
	mult = 4999. / 50. ; 
  	
    }

//    if( (n < 0.) || (n > 1.) ){
//	// BAD CURVE INDEX POINT
//	fprintf( stderr, "\n\n--> %f IS A BAD CURVE INDEX POINT!\n\n",  n ) ; 
//	exit( 0 ) ; 
//    }

   if( warp == 0. ){
	// STRAIGHT LINE
	val =  V1 + (V2 - V1) * n ;
    }else if( (warp > 25.) || (warp < -25.) || (n > 1.0) ){ 
	// OUTSIDE TABLE OR 0-1 INDEX TO IT: DO THE HARD WAY
	val =  V1 + (V2 - V1) * (1. - (float) exp((double)(n * warp)))/(1. - exp( (double) warp)) ; 
    }else{
	// TABLE
	
	prop = mult * ( warp + 25. ) ;
	iy1 = (int) prop ; 
	frac = prop - (float) iy1 ; 
	y1 = A[ iy1 ] ; 
	y2 = A[ iy1  + 1] ; 
	val1 = y1 + (frac * (y2 - y1)) ; 

	prop = mult * ( (warp * n) + 25. ) ;
	iy1 = (int) prop ; 
	frac = prop - (float) iy1 ; 
	y1 = A[ iy1 ] ; 
	y2 = A[ iy1  + 1] ; 
	val2 = y1 + (frac * (y2 - y1)) ; 

	val =  V1 + (V2 - V1) * ( val2 ) / ( val1 ) ; 
	
 	
    }

   return( val ) ; 

}

*/