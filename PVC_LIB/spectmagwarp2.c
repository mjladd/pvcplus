#include <stdio.h>
#include <math.h>
#include "pv.h"

int spectmagwarp2( 
    float SP[],  
    float SP_return[],  
    int Nplus2,  
    float warpshape, 
    int normflag
    )
{    

    int i ; 
    float temp ;
    float middB ;
    float peakbinamp=0;   
    float curve( float V1, float  V2, float  n, float   x) ;
    
// WARP THE SPECTRUM USING CURVE AND THE WARPSHAPE VALUE, RETURN IN THE RETURN ARRAY 
//  

    if( normflag ){
	// NORMALIZE MODE
	 // FIND PEAK AMP
	peakbinamp = -99999999. ; 
	for( i = 0; i < Nplus2; i += 2){
	    if( peakbinamp < SP[ i ] ) peakbinamp = SP[ i ];
	}

	if( peakbinamp <= 0. ){
	   //fprintf( stderr, "\n\n(IN SPECTMAGWARP2): CANNOT NORMALIZE, PEAK BIN AMP = 0.   ABORT. BYE\n" ) ;
	    // NO AMP TO WARP
	    for( i = 0; i < Nplus2; i += 2) 
		SP_return[ i ] = SP[ i ] ; 
	    return(0) ; 
	}
	// NORMALIZE THEM
	for( i = 0; i < Nplus2; i+= 2){
	    SP_return[ i ] = SP[ i ] / peakbinamp ; 
	} 
	peakbinamp = 1.0 ; // 1.0 BECOMES PEAK

	// NOW WARP IT 
	if( warpshape == 0. ){
		// LINEAR WARP -- SKIP
		for( i = 0; i < Nplus2; i += 2) 
		    SP_return[ i ] = SP[ i ] ; 
	    	return(0) ; 
	}else{
	    for( i = 0; i < Nplus2; i += 2)
		SP_return[ i ] = curve( 0.,  peakbinamp, SP[ i ],  warpshape   ) ;
	}
	return( 1 ) ;  

    }else if( warpshape != 0.){
	// REGULAR MODE
	// FIND PEAK AMP
	peakbinamp = -99999999. ; 
	for( i = 0; i < Nplus2; i += 2){
	    if( peakbinamp < SP[ i ] ) peakbinamp = SP[ i ];
	}
	if( peakbinamp <= 0. ){
	    // NO AMP TO WARP
	    for( i = 0; i < Nplus2; i += 2) 
		SP_return[ i ] = SP[ i ] ; 
	    return(0) ; 
	}else{
	// NOW WARP IT 
	    for( i = 0; i < Nplus2; i += 2)
		SP_return[ i ] = curve( 0.,  peakbinamp, (SP[ i ]/peakbinamp),  warpshape   ) ;
	    return( 1 ) ;  
	}

    }else{
	// LINEAR WARP -- SKIP
	    for( i = 0; i < Nplus2; i += 2) 
		SP_return[ i ] = SP[ i ] ; 
	return(0) ; 
    }
}
