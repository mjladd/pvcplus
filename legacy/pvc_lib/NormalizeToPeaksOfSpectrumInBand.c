#include <stdio.h>
#include <math.h>
#include "pv.h"

void NormalizeToPeaksOfSpectrumInBand
( 
    int normalizeToPeaksFlag, 
    float F[],
    int N,
    int numFormants, 
    int formantIndices[],
    int formantLowStopBandIndices[], 
    int formantHighStopBandIndices[], 
    float expansionIndex
){
    int N2, lowIndex, highIndex, i, j, k, indexRange, halfRangeIndex, formant  ; 
    float lowGain, highGain, upRamp, downRamp, peakAmp, lowAmp=0.0, highAmp=0.0;
    float inputPeakAmp ; 

    N2 = N / 2; 

    // NORMALIZE

    inputPeakAmp = -99999999.0 ; 
    for( i = 1; i < N; i += 2)
	if( F[i - 1] > inputPeakAmp) inputPeakAmp = F[i - 1] ; 
    if( inputPeakAmp != 1. ) 
		for( i = 0; i < N; i += 2)F[i] /= inputPeakAmp ;  

// ***




// ****



    for(formant = 0; formant <= numFormants; formant++ ){

	if(formant == 0){
		// BOTTOM
	    lowIndex = 0 ; highIndex = formantIndices[formant] * 2 ; 
	    
	    if( normalizeToPeaksFlag == 1){
		peakAmp = -999999.0 ;
		k = ( (highIndex - lowIndex + 1) / 4) ; 
		for( j = lowIndex; j < k; j += 2)
		    if( F[j] > peakAmp ) peakAmp = F[j] ; 
		if( peakAmp > 0.0){
		    lowGain = 1.0 / peakAmp ; lowAmp = 0.0 ; 
		}else{
		    lowGain = 1.0 ; lowAmp = 1.0 ; 
		} ;  
		highGain = 1.0 / F[ highIndex ] ;  
	    }else{
		lowGain = 1.0 ; lowAmp = 0.0 ; highGain = 1.0; highAmp = 0.0 ;  
	    };
//	    lowFreq = highFreq = F[ (formantIndices[formant] * 2) + 1 ] ;  

	}else if(formant == numFormants){
		// TOP
	    lowIndex = formantIndices[formant - 1] * 2 ; highIndex = N - 2 ;

	    if( normalizeToPeaksFlag == 1){
		lowGain = 1.0 / F[ lowIndex ] ; 
		peakAmp = -999999.0 ;
		k = highIndex - ( (highIndex - lowIndex + 1) / 4) ; 
		for( j = k; j < highIndex; j += 2)
		    if( F[j] > peakAmp )peakAmp = F[j] ; 
		if( peakAmp > 0.0){
		    highGain = 1.0 / peakAmp ; highAmp = 0.0 ; 
		}else{
		    highGain = 1.0 ; highAmp = 1.0 ; 
		} ;  
	    }else{
		lowGain = 1.0 ; lowAmp = 0.0 ; highGain = 1.0; highAmp = 0.0 ;  
	    } ; 

	}else{
		// ALL OTHERS BETWEEN
	    lowAmp = highAmp = 0.0 ; 
	    lowIndex = formantIndices[formant - 1] * 2 ; highIndex = formantIndices[formant] * 2 ; 
	    if( normalizeToPeaksFlag == 1){
	        lowGain = 1.0 / F[ lowIndex ] ; highGain = 1.0 / F[ highIndex ] ;  
	    }else{
		lowGain = 1.0 ; highGain = 1.0 ; 
	    } ; 

	} ; 

	indexRange = (highIndex - lowIndex + 2) / 2 ; 
	halfRangeIndex = (int)( ((float) indexRange / (float) 2) + 0.5 ) ; 


	for( j = lowIndex, k = 0; j < highIndex; j += 2, k++ ){
	    upRamp = (float) k / (float) indexRange ;   
	    downRamp = 1.0 - upRamp ;
	    upRamp = curve( 0., 1., upRamp, expansionIndex ) ; 
	    downRamp = curve( 0., 1., downRamp, expansionIndex ) ;   		    
	    F[ j ] = (F[j] * ( (upRamp * highGain) + (downRamp * lowGain) )) + 
		(lowAmp * downRamp) + (highAmp * upRamp) ; 
	    if( F[j] > 1.0 ) F[j] = 1.0 ; 
	
	}; 



    } ; 

    if( (inputPeakAmp > 0.) && (inputPeakAmp != 1.) ){
        for( i = 0; i < N; i += 2) F[i] *= inputPeakAmp ;  
    } ; 



} ; 