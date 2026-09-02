#include <stdio.h>
#include <math.h>
#include "pv.h"

int smooth_zero( 
    float A[], 
    float old_A[], 
    int N, 
    float att,
    float matt, 
    float rel, 
    float mrel, 
    float F[], 
    float gain     
    )

{    

    int i ; 

	//*********ATTACK/RELEASE:

  if( (rel != 0.) || (att != 0.) ){

	for( i = 0; i < N; i+= 2 ){


	    if( A[i] <= (4. * gain * F[i]) ){
		if((A[i] < old_A[i] ) ){
		    // RELEASE
		    A[i] =  
			(rel * old_A[i] ) + ( mrel * A[i] ); 
		} else {
		    // ATTACK
		    A[i] =  
			(att * old_A[i] ) + ( matt * A[i] ); 

		}
	 
	    }	
	}
  }
  
  for( i = 0; i < N; i+= 2 ) old_A[i] = A[i] ;

  return( 1 ) ; 

}
