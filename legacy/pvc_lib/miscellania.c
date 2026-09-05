#include <stdio.h>
#include <math.h>
#include "pv.h"

float amp_to_dB( float amp ){
    
    return( 20. * log10( (double) amp ) ) ; 

}


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
        
    div = 1 ; 
    if( dB > 96. ){
	while( dB > 96. ){
	    dB = dB - 6. ; div = div * 2. ; 
	}
    }else if( dB < -96. ){
	while( dB < -96. ){
	    dB = dB + 6. ; div = div * .5 ; 
	}
    }
    
    prop = mult1 * ( dB + 96. ) ;
    iy1 = (int) prop ; 
    frac = prop - (float) iy1 ; 
    y1 = A[ iy1 ] ; 
    y2 = A[ iy1  + 1] ; 
    val = y1 + (frac * (y2 - y1)) ; 
    return( val * div ) ; 

}

float semitones_to_mult( float semidev ){
    
    static int first=1;
    static float *A,  mult  ;
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

float smooth_setup( float t, float *c, float *minusc, float IR ){
    
  static int first=1 ; 
  static float ar_dB ; 
  
  if( first ){  
    ar_dB =  
	(double) pow( (double) 10.0, (double) ( -60. / 20.) );
	first = 0 ; 
  }
    		if(t <= 0.){
		    *c = 0. ; *minusc = 1. ; 

		}else{
		    // LOWPASS
		    *c = pow( (double) ar_dB, (double) (IR / t ) ) ; 
		    *minusc = 1. - *c ; 
		}
	return(1.) ; 

}

float smooth_one_value(

    float A, 
    float old_A, 
    float att,
    float matt, 
    float rel, 
    float mrel

){
    
              if((A < old_A) ){
                    // RELEASE
                    A = (rel * old_A ) + ( mrel * A ); 
            } else {
                    // ATTACK
                    A = (att * old_A ) + ( matt * A ); 

            }
 
 
    return( A ) ; 
}
