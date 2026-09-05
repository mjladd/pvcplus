#include <stdio.h>
#include <math.h>
#include "pv.h"

int invertresponse( 
    float SP[],  
    int N, 
    int normflag
    )
{    

    int i ;
    int flag;
    float a,  peakamp,  normamp ; 
    static float athresh ; 
    static int first=1 ; 
    
    
    
// INVERT THE MAGNITUDES, CONVERTING FIRST TO DB AND THEN BACK
// IF normflag != 0, FIND THE PEAKAMP AND INVERT VALUES BETWEEN
// 0 AND THE PEAK. 
    if( first ) athresh = dB_to_amp( -96. ) ; 

    if(  normflag != 0 ){
	// FIND PEAK AMP
	peakamp = -999999999. ; 
	for( i = 0; i < N ; i+= 2)
	    if( SP[i] > peakamp ) peakamp = SP[i] ; 

	if( peakamp <= 0.){
		//prt( "CANNOT INVERT RESPONSE!,  PEAK AMPLITUDE == 0" ) ; 
		return(0) ; 
	    }else{
		normamp = 1. / peakamp ; 

	    }
    }else{
	// INVERTED AGAINST PEAK AMP OF 1.0
	peakamp = 1.0 ; 
	normamp = 1.0 ; 
    }   

 
    // NOW INVERT
	for( i = 0; i < N ; i+= 2 ){
	    a = SP[i] * normamp  ;
	    // CONVERT TO DB
	    if( a <= athresh )
		a = -96. ;
	    else  
		a = (float) (20. * log10( (double) a )) ; 
	    // INVERT IN DB DOMAIN
	    a = -96. - a ; 
	    // CONVERT TO AMP
	    a =  dB_to_amp( a ) ; 
	    // REPLACE INTO ARRAY
	    SP[i] = a * peakamp ; 


	}

	return( flag ) ;

  
}
