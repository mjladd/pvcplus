#include <stdio.h>
#include <math.h>
#include "pv.h"

float AnalyzePartials(

     float A[], 
    int N, 
    float fundamental,
    float analysisFrequency,
    float bandwidth,
    int numberOfPartials, 
    float FrequencyOfPartials[],
    float AmplitudeOfPartials[]

 
 ){
    
    int i, j,   i1,  i2 ; 
    float binfreq, partanalfreq, lowfreq, hifreq, freqbw, peakbinamp, 
          binamp, strongbinfreq ; 
    
   
	// MAKE THE FREQ BW DEVIATION FROM PARTIAL
	freqbw = analysisFrequency * bandwidth * 0.5 ; 
	// LOOP FOR PARTIALS
	for(j = 0; j < numberOfPartials; j++){

		partanalfreq = analysisFrequency * (float) (j + 1) ; // PARTIAL FREQUENCY
		lowfreq = partanalfreq - freqbw ; hifreq = partanalfreq + freqbw ; // FREQ BOUNDARIES OF BAND
		
	  	// TURN FREQ BOUNDS INTO INDECES
		i1 = 1 +  ( 2 * (int) ( lowfreq / fundamental ) ) ; 
		i2 = 1 +  ( 2 * (int) ( hifreq / fundamental ) ) ; 

		// PROTECTIONS
		if( i2 >= N ) i2 = N - 1 ; 
		if( i2 == i1 ) i2 = i1 + 2 ; 


		peakbinamp = -999999.0 ; 


		for( i = i1; i <= i2; i+= 2){ 
		    binfreq = A[i] ; // FREQ OF BIN
	            binamp = A[i - 1] ; // AMP OF BIN

		   // FIND PEAK BIN AMP
		   if( binamp > peakbinamp ) { peakbinamp = binamp ; strongbinfreq = binfreq ; } ; 
		}


		// LOAD FREQ AND AMPS INTO ANALYSIS ARRAY

		FrequencyOfPartials[j] = strongbinfreq ; 
		AmplitudeOfPartials[j] = peakbinamp ; 


	}


    return( 1 ) ; 


}
