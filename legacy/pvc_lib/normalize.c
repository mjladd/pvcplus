#include <stdio.h>
#include <math.h>
#include "pv.h"

int normalize( 
    float SP[],  
    int Nplus2,  
    float peakamp
    )
{    

    int i ;
    int flag;  
    
// GAINSCALE ALL BINS RELATIVE TO THE PEAKAMP WHICH 
// BECOMES 1.0. ONCE SCALED, BINS WITH AMP GREATER THAN 1.0 ARE
// LIMITED TO 1.

	flag = 0 ; 
// NORMALIZE
	for( i = 1; i < Nplus2 ; i+= 2){
	    SP[i - 1] = SP[i - 1] /peakamp ; 
	    if( SP[i - 1] > 1.0 ){
// LIMIT
		SP[i - 1] = 1.0 ; 

		flag = 1. ; 
	    }
	} 

	
	return( flag ) ;

  
}


int gainscale( 
    float SP[],  
    int Nplus2,  
    float amp
    )
{    

    int i ;
    
// GAINSCALE ALL BINS BY AMP

// NORMALIZE
	for( i = 1; i < Nplus2 ; i+= 2){

	    SP[i - 1] = SP[i - 1] * amp ; 

	} 

	
	return( 1 ) ;

  
}
