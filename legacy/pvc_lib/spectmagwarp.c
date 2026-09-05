#include <stdio.h>
#include <math.h>
#include "pv.h"

int spectmagwarp( 
    float SP[],  
    int Nplus2,  
    float warpshape, 
    int normflag
    )
{    

    int i ; 
    
    
    float peakbinamp=0;   
    float curve( float V1, float  V2, float  n, float   x) ;
    
// WARP THE SPECTRUM USING CURVE AND THE WARPSHAPE 
// 


    if( normflag ){
	// NORMALIZE MODE: NORMALIZE THE OUTPUT ARRAY TO A PEAK AMP OF 1.0
	 // FIND PEAK AMP
	peakbinamp = -99999999. ; 
	for( i = 1; i < Nplus2; i+= 2){
	    if( peakbinamp < SP[i - 1] ) peakbinamp = SP[i - 1];
	}

	if( peakbinamp <= 0. ){
	   //fprintf( stderr, "\n\n(IN SPECTMAGWARP): CANNOT NORMALIZE, PEAK BIN AMP = 0.   ABORT. BYE\n" ) ;
	    // NO AMP TO WARP
	    return(0) ; 
	}
	// NORMALIZE THEM
	for( i = 1; i < Nplus2 ; i+= 2){
	    SP[i - 1] = SP[i - 1] / peakbinamp ; 
	} 
	peakbinamp = 1.0 ; // 1.0 BECOMES PEAK

	// NOW WARP IT 
	if( warpshape == 0. ){
		// LINEAR WARP -- SKIP
	    	    return(0) ; 
	}else{
	    for( i = 1; i < Nplus2; i+= 2)
		SP[i - 1] = curve( 0.,  peakbinamp, SP[i - 1],  warpshape   ) ;
	}
	return( 1 ) ;  

    }else if( warpshape != 0.){
	// REGULAR MODE: DO NOT NORMALIZE, WARP AGAINST PEAKAMP.
	// FIND PEAK AMP
	peakbinamp = -99999999. ; 
	for( i = 1; i < Nplus2; i+= 2){
	    if( peakbinamp < SP[i - 1] ) peakbinamp = SP[i - 1];
	}
	if( peakbinamp <= 0. ){
	    // NO AMP TO WARP
	    return(0) ; 
	}else{
	// NOW WARP IT 
	    for( i = 1; i < Nplus2; i+= 2)
		SP[i - 1] = curve( 0.,  peakbinamp, (SP[i - 1]/peakbinamp),  warpshape   ) ;
	    return( 1 ) ;  
	}

    }else{
	// LINEAR WARP -- SKIP
	return(0) ; 
    }
}
