#include <stdio.h>
#include <math.h>
#include "pv.h"

int smooth( 
    float A[], 
    float old_A[], 
    int N, 
    float att,
    float matt, 
    float rel, 
    float mrel
     
    )

{    
    static int first=1 ;
    static float ampthresh ;  
    int i ;
    float peakamp,   threshnow,  temp,  trel,  tmrel,  tatt,  tmatt ;  
    
    
    if( first ){

	ampthresh = dB_to_amp( -30 ) ;
	first = 0 ;  
    }
	//******** FIND THE PEAK AMP
    peakamp = 0. ; 
    for( i = 0; i < N; i+= 2 )if( A[i] > peakamp ) peakamp = A[i] ; 


	// MAKE RELATIVE THRESHOLD
	threshnow = ampthresh * peakamp   ; 

	//*********ATTACK/RELEASE:

  if( (rel != 0.) || (att != 0.) ){

	for( i = 0; i < N; i+= 2 ){

	    // IF BIN AMP IS BELOW THRESHOLD, THEN TURN DOWN 
	    if(A[i] < threshnow ){

		// BELOW THRESHOLD
		// MAKE GRADIENT VALUE
		temp = curve( 0.,  1.,  (A[i] / threshnow),  -6. ) ; 



		// MODIFY COEF ON PAST AMP
		trel = rel * temp ; tatt = att * temp ; 
		// MAKE OTHER FROM IT
		tmrel = 1. - trel ; tmatt = 1. - tatt ; 
	    }else{

		// NO CHANGE
		trel = rel; tmrel = mrel;  tatt = att ; tmatt = matt ; 
	    }

		if((A[i] < old_A[i]) ){
		    // RELEASE
		    A[i] =  
			(trel * old_A[i] ) + ( tmrel * A[i] ); 
		} else {
		    // ATTACK
		    A[i] =  
			(tatt * old_A[i] ) + ( tmatt * A[i] ); 

		}
	 
		
	}
  }
  
  for( i = 0; i < N; i+= 2 ) old_A[i] = A[i] ;

  return( 1 ) ; 

}
