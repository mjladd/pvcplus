#include <stdio.h>
#include <math.h>
#include "pv.h"
 
int smoothspec( float *F, int N2plus1, float octavesOrFreqBW,  int R )
{


// AVERAGE THE SPECTRUM VALUES WITH WINDOWS THE SIZE OF
// octavesOrFreqBW

    static float fund, *T ; 
    static int first=1 ; 

    int  i, j, k,  l, m,   lowbin,  hibin, octavesOrFreqFlag, binsHalfBand ;
    float pma, t,  pmb,   norm,  temp,  temp2,  peakampF,  peakampT, sum ;


    //fprintf( stderr, "\n\n.......SMOOTHING  THE PEAK SPECTRUM..." ) ; 

    if( first == 1 ) {
	fvec(T, N2plus1) ; // TEMP SPACE
	fund = (float) R / (float) (N2plus1 * 2) ; 
	first = 0 ; 
    }


    if( octavesOrFreqBW < 0.) { // OCTAVE UNITS
        octavesOrFreqFlag = 0 ; octavesOrFreqBW = fabs( octavesOrFreqBW ) ; 

        t = octavesOrFreqBW / 2. ; 
        pma = pow( (double) 2.,  (double) t ) ; 
        pmb = pow( (double) 2.,  (double) (-t) ) ; 
    } else {
	octavesOrFreqFlag = 1 ; 
    } ; 	


    // FIND PEAK AMP
    peakampF = -99999999. ; 
    for(j = 0,  i = 0; j < N2plus1; j++,  i += 2) 
	if( F[i] > peakampF ) peakampF = F[i] ; 
    
    
   	// LOOP FOR BINS
    for(i = 1,  j = 0; j < N2plus1; i += 2,  j++ ){

	if( octavesOrFreqFlag == 0 ){
	    lowbin = (int) (.5 + (pmb * j)) ; hibin = (int) (.5 + (pma * j)) ;
	}else{
	    binsHalfBand = (int) (((octavesOrFreqBW * 0.5) / fund) + 0.5) ; 
	    lowbin = j - binsHalfBand ; hibin = j + binsHalfBand ; 
	} ; 

	if(lowbin < 0)lowbin = 0;
	if(hibin > N2plus1 )hibin = N2plus1;
	T[j] = 0; temp = 0 ; sum = 0. ; 

	for(k = lowbin,  m = 0; k <= hibin; k++,  m++){
//	    temp = halfCosWindow( fabs( (float) (k - j) ) / (float) binsHalfBand ) ; 

//	    T[j] +=  (F[ k*2 ] * temp ) ; sum += temp ; 
	    T[j] +=  F[ k*2 ]  ; 

	}	
//	T[j] = T[j] / sum ; 
	T[j] = T[j] / (float) m ; 

    }
    

    // FIND PEAK AMP IN T
    peakampT = -99999999. ; 
    for(j = 0,  i = 0; j < N2plus1; j++,  i += 2) 
	if( T[j] > peakampT ) peakampT = T[j] ; 

    if( peakampT > 0. ) norm = peakampF / peakampT ; 
    else norm = 1. ; 
    
    // REPLACE VALUES IN F
    for(j = 0,  i = 0; j < N2plus1; j++,  i += 2) F[i] = T[j] * norm ; 


    return( 1 ) ; 
}
