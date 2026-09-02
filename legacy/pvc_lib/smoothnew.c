#include <stdio.h>
#include <math.h>
#include "pv.h"

int smoothnew( 
    float channel[], 
    int N,  
    float detectenv[], 
    float newenv[],  
    int N2,  
    float detect, 
    float mdetect, 
    float attackc, 
    float minusattackc, 
    float releasec, 
    float minusreleasec )

{    

    int i,  j ;
    
    static int first=1 ; 
    static float sixtydB ; 
    float temp ; 
    
    if( first ){
	
	sixtydB = dB_to_amp( -60 ) ; 
    
	first = 0 ; 
    } 

    // MAKE THE DETECTED ENVELOPE VALUES
	for( i = 0, j = 0; i < N; i += 2,  j++ ){
	    detectenv[j] = 
		(detect * detectenv[j] ) + 
		    ( mdetect * channel[i] );
	}


   // MAKE THE NEW ENVELOPE VALUES
  if( (releasec != 0.) || (attackc != 0.) ){

	for( i = 0, j = 0; i < N; i += 2,  j++ ){

		if((detectenv[j] < newenv[j]) ){
		    // RELEASE
		    newenv[j] =  
			( releasec * newenv[j] ) + ( minusreleasec * detectenv[j] ); 
		} else {
		    // ATTACK
		    newenv[j] =  
			( attackc * newenv[j] ) + ( minusattackc * detectenv[j] ); 

		}
	 
		
	}
  }

    // MODIFY THE VALUES WITH THE PROPORTIONAL MULTIPLIER OF newenv / detectenv
    for( i = 0, j = 0; i < N; i += 2,  j++ ){
	if( detectenv[j] > 0. ){
	    // MAKE MULTIPLIER
	    temp = channel[ i ] / sixtydB ; // (1 to 0 )
	    temp = curve( 0., 1.,  temp,  -12. ) ; 


	    temp = 1. + temp * (1. - ( newenv[j] / detectenv[j] ) ) ; 
	    
	    channel[ i ] *= temp ; 
	}
	else
	    channel[ i ] = 0. ; 
    }
    
    
  return( 1 ) ; 

}
