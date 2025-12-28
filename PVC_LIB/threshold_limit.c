#include <stdio.h>
#include <math.h>
#include "pv.h"

int threshold_limit( 
    float SP[],  
    int Nplus2,  
    float noise_thresh_limit_dB
    )
{    

    int i ;
    int flag;
    
    float peakamp,  peakdB ;  
    float noise_thresh_limit_amp ; 
     

//	fprintf( stderr, "\n(IN)noise_thresh_limit_dB = %f", noise_thresh_limit_dB ) ;    


    noise_thresh_limit_amp = dB_to_amp( noise_thresh_limit_dB ) ; 

    
    peakamp = -9999999999. ; flag = 0 ; 

// SET TO ZERO THE BINS WHICH ARE MORE THAN THE THRESHOLD
    
// FIRST FIND PEAK AMP IN ARRAY

	for( i = 1; i < Nplus2 ; i+= 2){
	    if( SP[i - 1] > peakamp ){
		peakamp = SP[i - 1] ; flag = 1 ; 
	    }
	}
	if( flag == 0 ){
	    fprintf( stderr, "\n\nWARNING!  NOISE SAMPLE HAS ZERO AMPLITUDE! BYE \n" ) ; 
	    exit( 0 ) ; 
	}

    peakdB = 20. * log10f( peakamp ) ; 
    
//	fprintf( stderr, "\npeakdB =  %f, noise_thresh_limit_dB = %f", peakdB,  noise_thresh_limit_dB ) ;    
	noise_thresh_limit_amp *= peakamp ; 
//	fprintf( stderr, "\npeakamp =  %f, noise_thresh_limit_amp = %f", peakamp,  noise_thresh_limit_amp ) ;    

    // IF BIN AMP IS 
	for( i = 1; i < Nplus2 ; i+= 2 )
	    if( SP[i - 1] > noise_thresh_limit_amp ) SP[i - 1] = 0. ; 
	
	return( flag ) ;

  
}

