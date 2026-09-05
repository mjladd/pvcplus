#include <stdio.h>
#include <math.h>
#include "pv.h"


int writeSpectrumPlotFile(
    char spectrumOutputFile[],
    float F[],
    int N,
    int decibelsFlag
)
{
    FILE *fopen(), *file ;     
    int i, k, f, lowFreq, highFreq, freqRange ;
    float lowAmp, highAmp, outVal ;  

//    prs( spectrumOutputFile, "DECIBELS SPECTRUM PLOT FILE" ) ; 
    file  = fopen( spectrumOutputFile,  "w+") ;

    for( i = 1; i <= (N - 2); i += 2 ){
	lowFreq = (int)F[i] ; highFreq = (int)F[i + 2] ; 
	if( lowFreq != highFreq ){
	    lowAmp = F[i - 1] ; highAmp = F[i - 1 + 2] ;
	    freqRange = highFreq - lowFreq + 1 ; 
	    for( f = lowFreq, k = 0; f < highFreq; f++, k++ ){
		outVal = lowAmp + ( ((float)k / (float) freqRange) * (highAmp - lowAmp) ); 		 
		if(decibelsFlag == 1){
		    outVal = amp_to_dB( outVal ) ; if( outVal < -90.0) outVal = -90. ;   
		}; 
		fwrite( &outVal, sizeof(float), 1, file ) ;
	    } ; 

	} ; 
    } ; 
    fclose( file ) ; 
		
	return( 1 ); 
}; 
