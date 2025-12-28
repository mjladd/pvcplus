#include <stdio.h>
#include <math.h>
#include "pv.h"

int compand( 
    float SP[],  
    int Nplus2,  
    float compthreshamp, 
    float  compamp, 
    float expthreshamp, 
    float expamp 
    )
{    

    int i ;
    float peakamp ;  
    
// IF A BIN'S AMPLITUDE IS GREATER THAN THE COMPRESSION THRESHOLD, 
// REDUCE THE AMOUNT LYING ABOVE THE THRESHOLD BY THE 
// COMPRESSION AMPLITUDE

// IF A BIN'S AMPLITUDE IS LOWER THAN THE EXAPNSION THRESHOLD, 
// REDUCE THE AMOUNT LYING BELOW THE THRESHOLD BY THE 
// EXPANSION AMPLITUDE

// NORMALIZE THE OUTPUT


// NORMALIZE IT FIRST
	peakamp = -99999. ; 
	
	for( i = 0; i < Nplus2 ; i+= 2) if( SP[i] > peakamp ) peakamp = SP[i] ; 
	
	if( peakamp > 0. )
	    for( i = 0; i < Nplus2 ; i+= 2) SP[i]  = SP[i] / peakamp ; 
	else{
	    // ZERO!
	    fprintf( stderr, "\n\n CANNOT NORMALIZE SPECTRUM! PEAKAMP = 0! BYE\n\n" ) ; 
	    exit( 0 ) ;
	}


	peakamp = -99999. ; 
	for( i = 0; i < Nplus2 ; i+= 2){

	    if( SP[i] >  compthreshamp ){

		// COMPRESS
		SP[i] = (compthreshamp + (compamp * (SP[i] - compthreshamp))) ; 

	    }else if( SP[i] <  expthreshamp ){

		// EXPAND
		SP[i] = (expthreshamp - ( expamp  * (expthreshamp - SP[i]))) ; 
		if( SP[i] < 0. ) SP[i] = 0. ; 

	    }
	    
	    // FIND PEAKAMP
	    if( SP[i] > peakamp ) peakamp = SP[i] ;
	} 


	if( peakamp > 0. )
	    for( i = 0; i < Nplus2 ; i+= 2) SP[i]  = SP[i] / peakamp ; 
	else{
	    // ZERO!
	    fprintf( stderr, "\n\n CANNOT NORMALIZE SPECTRUM! PEAKAMP = 0! BYE\n\n" ) ; 
	    exit( 0 ) ; 
	}

	
	return( 1 ) ;

  
}
