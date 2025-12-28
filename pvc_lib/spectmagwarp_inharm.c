#include <stdio.h>
#include <math.h>
#include "pv.h"

int spectmagwarp_inharm( 
    float SP[],
    float F[],   
    int Nplus2,  
    float warpshape
   )
{    

    int i ; 
    float temp ;
    float middB ;
    float peakbinamp=0;   
    float curve( float V1, float  V2, float  n, float   x) ;
    
// WARP THE SPECTRUM USING CURVE AND THE WARPSHAPE 
// 


    if( warpshape != 0.){
	//  WARP AGAINST PEAKAMP.
	// FIND PEAK AMP
	peakbinamp = -99999999. ; 
	for( i = 1; i < Nplus2; i+= 2){
	    if( F[i - 1] != -1. )
		if( peakbinamp < SP[i - 1] ) 
			peakbinamp = SP[i - 1];
	}
	if( peakbinamp <= 0. ){
	    // NO AMP TO WARP
	    return(0) ; 
	}else{
	// NOW WARP IT 
	    for( i = 1; i < Nplus2; i+= 2)
		if( F[i - 1] != -1. )
		    SP[i - 1] = 
			curve( 0.,  peakbinamp, (SP[i - 1]/peakbinamp),  warpshape   ) ;
	    return( 1 ) ;  
	}

    }else{
	// LINEAR WARP -- SKIP
	return(0) ; 
    }
}
