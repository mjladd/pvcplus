#include <stdio.h>
#include <math.h>
#include "pv.h"

float find_spectralflatness(

	float *flatnessCoef, 
	float *flatnessCoefInDecibels,
	float A[],
	float previous_A[], 
    	int N, 
		// 0 = amplitude, 1 = change in amplitude, 2 = change in frequency
	int methodFlag, 
    	float lowf, 
    	float hif, 
    	float fundamental, 
    	float *old_value,
	float amplitudeThreshold

 ){
    
    int i,  i1,  i2 ; 
    double value; 
    double arithmeticMean, geometricMean, count, omittedFromCount ;    

    // TURN FREQ BOUNDS INTO INDECES


	i1 = 1 +  ( 2 * (int) ( lowf / fundamental ) ) ; 
	i2 = 1 +  ( 2 * (int) ( hif / fundamental ) ) ; 
	if( i2 >= N ) i2 = N - 1 ; 
	


	if( i2 == i1 ) i2 = i1 + 2 ; 

	arithmeticMean = 0. ; geometricMean = 0. ; omittedFromCount = 0. ; count = 0. ; 
	if( frame_count == 0 ) *old_value = amplitudeThreshold ; 
	else *old_value = *flatnessCoef ;

	for( i = i1; i <= i2; i+= 2){ 

		if( methodFlag == 0 ) value = A[i - 1] ;
		else if( methodFlag == 1 ) value = fabs( A[i - 1] - previous_A[i - 1] ) ; 
		else if( methodFlag == 2 ) value = fabs( A[i] - previous_A[i] ) ; 

	    	arithmeticMean = arithmeticMean  + value ; 
		geometricMean = geometricMean + log( value ) ;
	    	count = count + 1. ; 
	}

	if( count > 0. ){
		arithmeticMean = arithmeticMean / count ;	
		geometricMean = exp( geometricMean / count ) ;
	} ; 

	if( arithmeticMean > 0. ) *flatnessCoef = geometricMean / arithmeticMean ;
	else *flatnessCoef = 0. ; 

	if( *flatnessCoef < amplitudeThreshold ) *flatnessCoef = amplitudeThreshold ; 
	*flatnessCoefInDecibels = amp_to_dB( *flatnessCoef ); 
	
	return(1.0) ; 

}
