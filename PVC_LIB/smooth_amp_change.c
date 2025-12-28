#include <stdio.h>
#include <math.h>
#include "pv.h"

int smooth_amp_change( 
    float A[], 
    float old_A[], 
    int N2, 
    float att,
    float matt, 
    float rel, 
    float mrel
     
    )

{    

    int j ; 

	// SMOOTH THE AMP CHANGE MULTIPLIERS (HALF SIZE ARRAYS)
	//*********ATTACK/RELEASE:

  if( (rel != 0.) || (att != 0.) ){

	for( j = 0; j < N2; j++ ){

	    if((A[j] < old_A[j]) ){
		    // RELEASE
		    A[j] =  
			(rel * old_A[j] ) + ( mrel * A[j] ); 
	    } else {
		    // ATTACK
		    A[j] =  
			(att * old_A[j] ) + ( matt * A[j] ); 

	    }
	 
		
	}
  }
  
  for( j = 0; j < N2; j++ ) old_A[j] = A[j] ;

  return( 1 ) ; 

}
