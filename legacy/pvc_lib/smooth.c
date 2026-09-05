#include <stdio.h>
#include <math.h>
#include "pv.h"

int smooth( 
    float A[], 
    float old_A[], 
    int Nplus2, 
    float att,
    float matt, 
    float rel, 
    float mrel
     
    )

{    

    int i ; 

   if( frame_count == 0 ){
	for(i = 0; i < Nplus2; i++ ) old_A[i] = A[i] ; 
//	prt( "FIRST FRAME: FILLING PREVIOUS ARRAY." ) ; 
//	for( i = 0 ; i < 100; i++ ) 
//		prf( old_A[i], "old_A[i]" ) ; 
    } ; 

	//*********ATTACK/RELEASE:

  if( (rel != 0.) || (att != 0.) ){

	for( i = 0; i < Nplus2; i+= 2 ){

//	    if( A[i] > 0.){

		if((A[i] < old_A[i]) ){
		    // RELEASE
		    A[i] =  
			(rel * old_A[i] ) + ( mrel * A[i] ); 
		} else {
		    // ATTACK
		    A[i] =  
			(att * old_A[i] ) + ( matt * A[i] ); 

		}
//	    }
	 
		
	}
  }
  
  for( i = 0; i < Nplus2; i+= 2 ) old_A[i] = A[i] ;

  return( 1 ) ; 

}
