#include <stdio.h>
#include <math.h>
#include "pv.h"

int eq( 
    float SP[],  
    int N,  
    float dBlow, 
    float dBhi, 
    float freqlow, 
    float freqhi, 
    float fundamental,
    float pmult, 
    float freqadd,  
    int normflag
    )
{    

    int i, j,   ilow,  ihigh ; 
    float temp1, temp2,  temp3 ;
    float peakbinamp=0;
        
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
    if( !frame_count )prt("........USING EQ.........") ; 


    // DIFFERENCE BETWEEN LOW AND HIGH 

    // MODIFY SHELF FREQUENCIES BY FREQ ADDER AND TRANSPOSITION MULTIPLIER
	freqlow = (freqlow - freqadd) / pmult ; 
	freqhi = (freqhi - freqadd) / pmult ; 

    // FIND INDECES FOR EQ REGIONS FROM FREQUENCIES
    ilow = 1 + (2 * (int) ( (freqlow / fundamental) + .5 )) ; 
	if(ilow < 0) ilow = 1 ;  if(ilow > N ) ilow = (N - 1) ;  
    ihigh = 1 + (2 * (int) ( (freqhi / fundamental) + .5 )) ; 
	if(ihigh < 0) ihigh = 1 ;  if(ihigh > N ) ihigh = (N - 1) ;  
    

    // GAIN LOW SHELF
	//temp1 = pow( (double) 10.0, (double) (dBlow / 20.) );	
	temp1 = dB_to_amp( dBlow  );	
    for( i = 1; i < ilow; i+= 2) SP[i - 1] = SP[i - 1]  * temp1 ;
	
    // GAIN TRANSITION REGION BETWEEN SHELF FREQUENCIES
	temp1 = dBhi - dBlow ; // SHELF DB DIFFERENCE
	temp2 = (float) ((ihigh - ilow) / 2) ; // NUMBER OF BINS IN TRANSITION REGION
	temp3 = temp1 / temp2 ; // DB PORTION PER BIN
    
    for( i = ilow,  j = 0 ; i < ihigh; i+= 2,  j++ ){
	temp1 = dBlow + (temp3 * (float) j) ; 
	//temp1 = pow( (double) 10.0, (double) (temp1 / 20.) );	
	temp1 = dB_to_amp( temp1  );	
	SP[i - 1] = SP[i - 1]  * temp1 ;
    }


    // GAIN HIGH SHELF
	//temp1 = pow( (double) 10.0, (double) (dBhi / 20.) );	
	temp1 = dB_to_amp( dBhi  );	
    for( i = ihigh; i < N; i+= 2)SP[i - 1] = SP[i - 1]  * temp1 ;

 
  }else if(dBhi != 0.){
    // NO DIFFERENCE BETWEEN LOW AND HIGH 
      // GAIN BY COMMON LOW/HIGH DB


    if( !frame_count )prt("........USING EQ (GAIN ONLY MODE).........") ; 

    // MAKE AMP 
    //temp1 = pow( (double) 10.0, (double) (dBhi / 20.) );	
	temp1 = dB_to_amp( dBhi  );	
    for( i = 1; i < N; i+= 2) SP[i - 1] = SP[i - 1]  * temp1 ;  

  }else{
      // SKIP IT - ALL GAIN AT 0 dB
  }

// NORMALIZE IF REQUESTED
    if( normflag ){
  // FIND PEAK
	if( !frame_count )prt("........NORMALIZING.........") ; 
	peakbinamp=0. ;
	for( i = 1; i < N; i+= 2){
	    if( peakbinamp < SP[i - 1] ) peakbinamp = SP[i - 1]; 
	}
	if( peakbinamp <= 0. ){
	    fprintf( stderr, "\n\n(IN EQ) PEAK BIN AMP = 0.   ABORT. BYE\n" ) ;
	    exit(0) ; 
	}
	for( i = 1; i < N ; i+= 2){
	    SP[i - 1] = SP[i - 1] / peakbinamp ; 
	} 

	return( 1 ) ;	
    }else{
	 return( 1 ) ; 
    }

}
