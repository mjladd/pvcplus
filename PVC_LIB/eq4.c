#include <stdio.h>
#include <math.h>
#include "pv.h"

int eq4( 

    float SP[], 
    int N,  
    float dBlow,  
    float dBhi, 
    float freqlow, 
    float freqhi, 
    float eqcf, 
    float dBcf, 
    float eqwarp
     ) 		
{    



    int i, j ; 
    float temp1,  freq ;
    float lowamp, hiamp, lowfreqdiff,  hifreqdiff;
    int lowflag,  hiflag ; 

    
// EQUALIZE THE EVEN NUMBERED ARRAY VALUES WITH THE 
// BP RESPONSE CURVE


 if( (dBhi != 0.) || (dBlow != 0.) || (dBcf != 0.) ){ 
    if( !frame_count )prt("........USING BANDPASS EQ.........") ; 

    // SET UP AMPS AND FREQS
	lowamp = dB_to_amp( dBlow ) ;	
   	hiamp = dB_to_amp( dBhi );	
	lowfreqdiff = eqcf - freqlow ; 
	hifreqdiff = freqhi - eqcf ; 
	if( dBlow < dBcf ) lowflag = 1 ; 
	else lowflag = 0 ; 
	if( dBhi < dBcf ) hiflag = 1 ; 
	else hiflag = 0 ; 

	if( eqcf < freqlow ){
	    // CF BELOW LOWPASSBAND
	    prf( eqcf,  "EQ CENTER FREQ" ) ; 
	    prf( freqlow,  "EQ LOW PASSBAND FREQ" ) ; 
	    prt( "YOUR EQ CENTER FREQ IS BELOW THE LOW PASSBAND FREQ" ) ; 
	    prt( "ABORT!" ) ; 
	}
	if( freqhi < eqcf ){
	    // HIGH PASSBAND BELOW CF
	    prf( eqcf,  "EQ CENTER FREQ" ) ; 
	    prf( freqhi,  "EQ HIGH PASSBAND FREQ" ) ; 
	    prt( "YOUR EQ CENTER FREQ IS BELOW THE HIGH PASSBAND FREQ" ) ; 
	    prt( "ABORT!" ) ; 
	}

	
	
    // LOOP FOR BINS
    for( i = 1; i < N; i+= 2 ){
	freq = SP[ i ] ; 
	if( freq < freqlow ){
	    // BELOW LOW PASSBAND
	    SP[i - 1] *= lowamp ;
	}else if( freq > freqhi ){
	    // ABOVE HIGH PASSBAND
	    SP[i - 1] *= hiamp ;
	}else if( freq < eqcf ){
	    // LOWER TRANSITION
	    // TRANSITION CF TO LOW
	    temp1 = (freq - freqlow) / lowfreqdiff  ;  // 0-1

	    if( lowflag )
		temp1 = curve( dBlow, dBcf, temp1, eqwarp ) ; // dBlow to 0
	    else
		temp1 = curve( dBlow, dBcf, temp1, (-1. * eqwarp) ) ; // dBlow to 0
	    
	    SP[i - 1] *= ( dB_to_amp( temp1 )) ;
	}else{
	    // UPPER TRANSITION
	    temp1 = (freqhi - freq) / hifreqdiff  ;  // 0-1
	    if( hiflag )
		temp1 = curve( dBhi, dBcf, temp1, eqwarp ) ; // dBhi to 0
	    else
		temp1 = curve( dBhi, dBcf, temp1, (-1. * eqwarp) ) ; // dBhi to 0
		SP[i - 1] *= ( dB_to_amp( temp1 )) ;
	} 

    }

 }
    return( 1 ) ; 
    
}
