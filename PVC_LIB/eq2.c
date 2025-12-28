#include <stdio.h>
#include <math.h>
#include "pv.h"

int eq2( 
    float SP[],  
    int Nplus2,  
    float dBlow, 
    float dBhi, 
    float freqlow, 
    float freqhi, 
    float fundamental,
    float channel_freqdev[],   
    int normflag
    )
{    

    int i, j ; 
    float temp1,  freq ;
    float peakbinamp,  lowamp, hiamp,  
	dBdiff,  freqdiff,  dBdiffdiv ;
    

    
// EQUALIZE THE EVEN NUMBERED ARRAY VALUES WITH THE 
// LO/HI RESPONSE CURVE

    // TEST FOR LOW-HIGH CONTRADICTION
  if(freqlow > freqhi){
	// LOW FREQ ABOVE HIGH. ABORT
	prline( 34, "?" ) ;
	prline( 1, "*" ) ; 
	prt( "SHELF EQ: ====>   YOUR LOW SHELF FREQUENCY IS > THE HIGH SHELF FREQUENCY. <====" ) ;
	prf( freqlow,  "LOW SHELF FREQUENCY" ) ; 
	prf( freqhi,  "HIGH SHELF FREQUENCY" ) ; 
	prt( "............................................EXITING. BYE." ) ; 
	prline( 69, "!" ) ; 
	prline( 1, "*" ) ;
	exit(0) ;  
 }

 if( (dBhi - dBlow) != 0. ){ 
    if( !frame_count ){
	prt("........USING EQ.........") ; 
    } ; 

    // SET UP AMPS
    	lowamp = dB_to_amp( dBlow ) ;	
   	hiamp = dB_to_amp( dBhi );	
	dBdiff = dBhi - dBlow ;
	freqdiff = freqhi - freqlow ; 
	dBdiffdiv = dBdiff / freqdiff ; 
	
	
    // LOOP FOR BINS
    for( i = 1,  j = 0; i < Nplus2; i+= 2,  j++ ){
	freq = 
	    (( (float) j * fundamental) + channel_freqdev[ i - 1] )
			 * channel_freqdev[ i ] ; 
	if( freq <=  freqlow ){
	// LOW SHELF
	    SP[ i - 1 ] *= lowamp ; 
	}else if( freq >=  freqhi ){
	// HI SHELF
	    SP[ i - 1 ] *= hiamp ; 
	}else{
	// TRANSITION
	    temp1 = dBlow + dBdiff * ( (freq - freqlow) / freqdiff ) ; 
   	    temp1 = (float) pow( (double) 10.0, (double) (temp1 / 20.) ) ;
	    SP[ i - 1 ] *=  temp1 ;  
	}
    }

 }else if(dBhi != 0.){
    // NO DIFFERENCE BETWEEN LOW AND HIGH 
      // GAIN BY COMMON LOW/HIGH DB


    if( !frame_count )prt("........USING EQ (GAIN ONLY MODE).........") ; 

    // MAKE AMP 
    temp1 = dB_to_amp( dBhi );	
    for( i = 1; i < Nplus2; i+= 2) SP[i - 1] = SP[i - 1]  * temp1 ;  

  }else{
      // SKIP IT - ALL GAIN AT 0 dB
  }

// NORMALIZE IF REQUESTED
    if( normflag ){
  // FIND PEAK

    if( !frame_count )prt("........NORMALIZING AFTER EQ.........") ; 
	peakbinamp=0;
	for( i = 1; i < Nplus2; i+= 2){
	    if( peakbinamp < SP[i - 1] ) peakbinamp = SP[i - 1]; 
	}
	if( peakbinamp <= 0. ){
	    fprintf( stderr, "\n\n(IN EQ) PEAK BIN AMP = 0.   ABORT. BYE\n" ) ;
	    exit(0) ; 
	}
	for( i = 1; i < Nplus2 ; i+= 2){
	    SP[i - 1] = SP[i - 1] / peakbinamp ; 
	} 

	return( 1 ) ;
    }else{
	 return( 1 ) ; 
    }

}
