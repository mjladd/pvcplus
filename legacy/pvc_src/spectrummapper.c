#include "globals.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SMOOTHING_WINDOW_AS_PROPORTION_OF_CENTROID 1./50.
#define FORMANT_OVERLAP_TOLERANCE_PROP .5

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; } ;

int findIntersectPointOfLines(
	float linkFromSegYintercept,
	float linkFromSegSlope,
	float linkToSegYintercept,
	float linkToSegSlope,
	float *xIntersect,
	float *yIntersect
){

	fprintf( stderr, "\n linkFromSegYintercept: %f linkFromSegSlope: %f linkToSegYintercept: %f linkToSegSlope: %f",
		linkFromSegYintercept, linkFromSegSlope, linkToSegYintercept, linkToSegSlope ) ; 


	if( linkFromSegSlope == linkToSegSlope ){
		return(0) ; 
	}else{
		*xIntersect = (linkToSegYintercept - linkFromSegYintercept) / 
			(linkFromSegSlope - linkToSegSlope) ;
		*yIntersect = (linkFromSegSlope * *xIntersect) + linkFromSegYintercept ;
		return(1); 
	} ;
} ;

int computeAmpAndFreqCorrelationFactor_NEW(
	int i0,
	int i1,
	float *dBabsoluteDiffMean, 
	float *midiAbsoluteDiffMean,
	float formantTime[],
	float formantCF[],
	float formantAmp[],
	int segmentLengths[], 
	int segmentBeginDataIndices[], 
	int segmentEndDataIndices[] 
){
	float temp ; 
	float shortdBmean, longdBmean, shortMidiMean, longMidiMean ; 
	float numVals ;
	float dBmeanDiff, midiMeanDiff ;  
	float shortdB, longdB, shortMidi, longMidi ;  
	int shortSegDataIndex, longSegDataIndex, 
		shortSegBeginDataIndex, shortSegEndDataIndex, 
		longSegBeginDataIndex, longSegEndDataIndex,
		slopeSegLength, accelerationSegLength,
		i, j, k, l ;
	float dBabsoluteSlopeDiffSum=0., midiAbsoluteSlopeDiffSum=0. ;
	float shortdBslopeSum, longdBslopeSum, shortMidiSlopeSum, longMidiSlopeSum    ; 
	float shortdBaccelerationSum, longdBaccelerationslopeSum, 
		shortMidiAccelerationSum, longMidiAccelerationSum    ; 
	float dBaccelerationDiff=0., midiAccelerationDiff=0. ;
	float v0, v1 ;
	float t0b, t0e, t1b, t1e ;
	float tbegin, tend ; 
	int foundFlag, i0b, i0e, i1b, i1e ; 

	fprintf( stderr, "\nSEGMENT LENGTHS: %d\t%d", 
		segmentLengths[ i0 ], segmentLengths[ i1 ] ); 

	t0b = formantTime[ segmentBeginDataIndices[i0] ] ;
	t0e = formantTime[ segmentEndDataIndices[i0] ] ;
	t1b = formantTime[ segmentBeginDataIndices[i1] ] ;
	t1e = formantTime[ segmentEndDataIndices[i1] ] ;

	fprintf( stderr, "\n t0b: %f t0e: %f t1b: %f t1e: %f", t0b, t0e, t1b, t1e ) ; 
	   
	tbegin = t1b >= t0b ? t1b : t0b ;
	tend = t0e <= t1e ? t0e : t1e ;

	fprintf( stderr, "\ntbegin: %f\ttend: %f", tbegin, tend ) ; 

	i0b = segmentBeginDataIndices[ i0 ] ;
	i1b = segmentBeginDataIndices[ i1 ] ;
	i0e = segmentEndDataIndices[ i0 ] ;
	i1e = segmentEndDataIndices[ i1 ] ;
 
	foundFlag = 0 ;
 	while( foundFlag == 0 ){
		if( formantTime[ i0b ] >= tbegin ) foundFlag = 1 ; else i0b += 1 ; } ; 
	foundFlag = 0 ;
 	while( foundFlag == 0 ){
		if( formantTime[ i1b ] >= tbegin ) foundFlag = 1 ; else i1b += 1 ; } ; 
	foundFlag = 0 ;
 	while( foundFlag == 0 ){
		if( formantTime[ i0e ] <= tend ) foundFlag = 1 ; else i0e -= 1 ; } ; 
	foundFlag = 0 ;
	while( foundFlag == 0 ){
		if( formantTime[ i1e ] <= tend ) foundFlag = 1 ; else i1e -= 1 ; } ; 

	while( (i0e - i0b) > (i1e - i1b) ) i0e -= 1 ;  
	while( (i1e - i1b) > (i0e - i0b) ) i1e -= 1 ; ;
	numVals = (float)((i0e - i0b) + 1) ;  

	// FIND AVERAGES
	shortdBmean = 0. ; longdBmean = 0. ; shortMidiMean = 0. ; longMidiMean = 0. ; 
	for(i = i0b, j = i1b; i <= i0e; i++, j++ ){
		temp = ( formantAmp[ i ] < dB_to_amp( -96. ) ) ? -96. : amp_to_dB( formantAmp[ i ] ) ;
		shortdBmean += temp ; 
		temp = ( formantAmp[ j ] < dB_to_amp( -96. ) ) ? -96. : amp_to_dB( formantAmp[ j ] ) ;
		longdBmean += temp ; 
		shortMidiMean += Hz_to_MIDI( formantCF[ i ] ) ;
		longMidiMean += Hz_to_MIDI( formantCF[ j ] ) ;
	} ;  
	shortdBmean /= numVals ; longdBmean /= numVals ; 
	shortMidiMean /= numVals ; longMidiMean /= numVals ;
	dBmeanDiff = shortdBmean - longdBmean ;
	midiMeanDiff = shortMidiMean - longMidiMean ;


	*dBabsoluteDiffMean = 0. ; *midiAbsoluteDiffMean = 0. ; 
	for(i = i0b, j = i1b; i <= i0e; i++, j++ ){
		shortdB = ( formantAmp[ i ] < dB_to_amp( -96. ) ) ? -96. : amp_to_dB( formantAmp[ i ] ) ;
		longdB = ( formantAmp[ j ] < dB_to_amp( -96. ) ) ? -96. : amp_to_dB( formantAmp[ j ] ) ;
		*dBabsoluteDiffMean += 
		   fabs(shortdB - dBmeanDiff - longdB)  ;
		*midiAbsoluteDiffMean += 
		   fabs( Hz_to_MIDI( formantCF[ i ] ) - midiMeanDiff - Hz_to_MIDI( formantCF[ j ] ) ) ;
	} ;	
	*dBabsoluteDiffMean /= numVals ; *midiAbsoluteDiffMean /= numVals ; 

	return(1) ; 
} ;



float linearLeastSquaresProjection(
	float	X,
	float	x[], 
	float	y[],
	int 	n,
	float 	*yIntercept,
	float	*slope

){
	double first=1, double_n, xSum, ySum, sumOfXtimesY, Y, xSquaredSum, xSumSquared,
		yMean, xMean, thisSlope, thisYintercept ;
	int i ; 



	double_n = (double) n ; 

//fprintf( stderr, "\n double_n: %f", double_n ); 
/*
for(i = 0; i < n; i++ ){
	fprintf( stderr, "\nx[ i ]: %f y[ i ]: %f", x[i], y[i] ); 

}; 
*/
	xSum = 0. ; ySum = 0. ;
	for(i = 0; i < n; i++){ xSum += (double) x[i] ; ySum += (double) y[i] ; }; 

	sumOfXtimesY = 0. ; 
	for(i = 0; i < n; i++) sumOfXtimesY += ((double) x[i] * (double) y[i]) ;

	xSquaredSum = 0. ; 
	for(i = 0; i < n; i++) xSquaredSum += ((double) x[i] * (double) x[i]) ;

	xSumSquared = 0. ; 
	for(i = 0; i < n; i++) xSumSquared += (double) x[i] ;
	xSumSquared *= xSumSquared ; 
	
	thisSlope = (float)(( sumOfXtimesY - ( ( xSum * ySum ) / double_n ) ) / 
					( xSquaredSum - (xSumSquared / double_n ) )) ; 

	xMean = 0 ; yMean = 0. ; 
	for(i = 0; i < n; i++){ xMean += (double) x[i] ; yMean += (double) y[i] ; } ;
	xMean /= double_n ; yMean /= double_n ;

	thisYintercept = (float)(yMean - (thisSlope * xMean)) ;

	Y = ((thisSlope * X) + thisYintercept) ;

	*slope = (float) thisSlope ; *yIntercept = (float) thisYintercept ; 

	return( (float) Y ) ; 

} ;



float makeLeastSquaresTargetCF(
	int direction,
	float groupTime, // FORMANT GROUP FRAME TIME
	int numberOfFrontLegFormants,
	int numberOfBackLegFormants,
	float formantCF[],
	float formantTime[],
	int forwardLegOfSegmentIndices[],
	int backwardLegOfSegmentIndices[],
	float scratchX[],
	float scratchY[],
	int maxLength
){

	float targetFormantCF ;
	float scaler, x1, x2, y1, y2 ;
	int i, j, k, thisLength, numberOfAddedFrontLegFormants, 
		scratchIndex, numberOfSpacesLeft, legStartIndex, 
		backLegStartIndex, frontLegStartIndex ;
	float yIntercept, slope ;

/*
fprintf( stderr, "\n direction: %d, groupTime: %f, numberOfFrontLegFormants: %d, numberOfBackLegFormants: %d", 
direction, groupTime, numberOfFrontLegFormants, numberOfBackLegFormants ) ; 
*/
	// *** MAKE TARGET CF *******
	if( direction == 1 ){  
		// FRONT LEG

		if( numberOfFrontLegFormants == 1 ){
			// ONE FORMANT (PEAK)
	         		targetFormantCF = formantCF[ forwardLegOfSegmentIndices[ 0 ]  ] ;

		}else{
			// MULTIPLE FRONT LEG FORMANTS
			// FILL ARRAYS
		      	if( numberOfFrontLegFormants >= maxLength ){
				// USE END OF LEG FOR maxLength VALUES. 
				thisLength = maxLength ; 
				frontLegStartIndex = numberOfFrontLegFormants - thisLength ;
		      	} else {
				// USE THE ENTIRE FRONT LEG.
				thisLength = numberOfFrontLegFormants ; frontLegStartIndex = 0 ; 
		      	} ;
			// FILL SCRATCH ARRAYS
		      	for( i = 0, j = frontLegStartIndex; i < thisLength; i++, j++ ){
		         		scratchX[ i ] = 
		            		formantTime[ forwardLegOfSegmentIndices[ j ] ] ;
		         		scratchY[ i ] = 
		            		formantCF[ forwardLegOfSegmentIndices[ j ] ] ;
		      	}  ;
			// CALL LEAST SQUARES FOR VALUE. 
		      	targetFormantCF = linearLeastSquaresProjection(
				groupTime, scratchX,  scratchY, thisLength, &yIntercept, &slope
		      	) ;
		} ;
	} else {
		// BACK LEG
	   	if( numberOfBackLegFormants >= maxLength ){
			// ALL BACK LEG FORMANTS
			thisLength = maxLength ; numberOfAddedFrontLegFormants = 0 ;  
			backLegStartIndex = numberOfBackLegFormants - thisLength ;
	   	} else {
			// NOT ENOUGH IN BACK LEG; USE ALL IN BACK LEG
			thisLength = numberOfBackLegFormants ;
			backLegStartIndex = 0 ;
			numberOfSpacesLeft = maxLength - thisLength ; 
			if( (numberOfFrontLegFormants - 1) >= numberOfSpacesLeft ){
				// FILL ALL REMAINING OPEN SPACES FROM FRONT LEG FORMANTS.
				numberOfAddedFrontLegFormants = numberOfSpacesLeft ;
				frontLegStartIndex = numberOfAddedFrontLegFormants ;			 
			}else{
				// FILL SOME SPACES FROM WHAT IS AVAILABLE FROM FRONT LEG.
				numberOfAddedFrontLegFormants = numberOfFrontLegFormants - 1 ;
				frontLegStartIndex = numberOfFrontLegFormants - 1 ;
			} ;
			// FILL ARRAYS
			scratchIndex = 0 ; 
			if( numberOfAddedFrontLegFormants > 0 ){
			   // FROM FRONT LEG FIRST, IF ANY
			   for(i = 0, j = frontLegStartIndex; i < numberOfAddedFrontLegFormants; i++, j--){
		         	      scratchX[ scratchIndex ] = formantTime[ forwardLegOfSegmentIndices[ j ] ] ;
		         	      scratchY[ scratchIndex ] = formantCF[ forwardLegOfSegmentIndices[ j ] ] ;
			      scratchIndex++ ;
			   } ;
			} ;
			// NOW FROM BACK LEG	
			for(i = 0, j = backLegStartIndex; i < thisLength; i++, j++ ){
		         	   scratchX[ scratchIndex ] = formantTime[ backwardLegOfSegmentIndices[ j ] ] ;
		         	   scratchY[ scratchIndex ] = formantCF[ backwardLegOfSegmentIndices[ j ] ] ;
			   scratchIndex++ ;
			} ;	
		} ;
		if( scratchIndex == 1 ){
			// ONLY ONE USE PREVIOUS
			targetFormantCF = formantCF[ backwardLegOfSegmentIndices[ 0 ]  ] ;
		}else{
			targetFormantCF = linearLeastSquaresProjection(
				groupTime, scratchX,  scratchY, scratchIndex, &yIntercept, &slope
		      	) ;
		} ;
	} ;
	// END OF ROUTINE FOR MAKING TARGET FREQ
	return( targetFormantCF ) ; 
} ;




void printSegmentData(
	char string[1000],
	int index,
	float formantTime[],
	float formantCF[],
	float formantAmp[],
	float formantBW[],
	float formantQ[],

	int segmentLengths[], 
	int segmentBeginDataIndices[], 
	int segmentEndDataIndices[], 
	int segmentSwitches[],

	float segmentBeginTimes[], 
	float segmentEndTimes[], 

	float segmentPeakAmps[]
){
	fprintf( stderr, "\n******** %s *******", string ) ; 
	fprintf( stderr, "\nSEGMENT INDEX: %d\tLENGTH: %d", index, segmentLengths[ index ] ) ;
	fprintf( stderr, "\n\tBEGIN-END TIMES: %f - %f",
		segmentBeginTimes[index], segmentEndTimes[index] 
	) ;
	fprintf( stderr, "\n\tBEGIN-END FORMANT FREQUENCIES: %f - %f",
		formantCF[ segmentBeginDataIndices[index] ], formantCF[ segmentEndDataIndices[index] ] 
	) ;

} ;

float findPeakAmp(
    float A[],
    int N2
 
) ; 

float find_dBchangePerMillisecond(
	int direction,
	float formantAmp[],
	float formantTime[],
	int proposedFormantIndex,
	int forwardLegOfSegmentIndices[],
	int numberOfFrontLegFormants,
	int backwardLegOfSegmentIndices[],
	int numberOfBackLegFormants
){

	float x0, x1, y0, y1 ;
	float dB0, dB1 ; 
	float dBChangePerMillisecond, duration, changeIndB ; 

	x1 = formantTime[ proposedFormantIndex ] ;
	y1 = formantAmp[ proposedFormantIndex ] ;

	if( direction == 1 ){
		// FORWARD
		x0 = formantTime[ forwardLegOfSegmentIndices[ numberOfFrontLegFormants - 1 ] ] ;
		y0 = formantAmp[ forwardLegOfSegmentIndices[ numberOfFrontLegFormants - 1 ] ] ;
	}else{
		// BACKWARD
		x0 = formantTime[ backwardLegOfSegmentIndices[ numberOfBackLegFormants - 1 ] ] ;
		y0 = formantAmp[ backwardLegOfSegmentIndices[ numberOfBackLegFormants - 1 ] ] ;
	} ;
        duration = fabs( x1 - x0 ) ;
	dB0 =  y0 < amp_to_dB( -96. ) ? -96. : amp_to_dB( y0 ) ; 
	dB1 =  y1 < amp_to_dB( -96. ) ? -96. : amp_to_dB( y1 ) ; 

	changeIndB = dB1 - dB0 ;

	dBChangePerMillisecond = (duration > 0.) ? 
		.001 * (changeIndB / duration) : 0. ;

/*
	fprintf( stderr, 
          "\n IN ROUTINE DURATION: %f, CHANGE IN DECIBELS PER MILLISECOND: %f ", 
               duration, dBChangePerMillisecond  ) ; 
*/
	return( dBChangePerMillisecond ) ; 

} ; 

float findFrequencyChangePerMillisecond(
	int direction,
	float formantCF[],
	float formantTime[],
	int proposedFormantIndex,
	int forwardLegOfSegmentIndices[],
	int numberOfFrontLegFormants,
	int backwardLegOfSegmentIndices[],
	int numberOfBackLegFormants
){
	float x0, x1, y0, y1 ;
	float changeInFrequency, frequencyChangePerMillisecond, 
		semitoneChangePerSecond, duration,
		changeInSemitones ; 

	x1 = formantTime[ proposedFormantIndex ] ;
	y1 = formantCF[ proposedFormantIndex ] ;

	if( direction == 1 ){
		// FORWARD
		x0 = formantTime[ forwardLegOfSegmentIndices[ numberOfFrontLegFormants - 1 ] ] ;
		y0 = formantCF[ forwardLegOfSegmentIndices[ numberOfFrontLegFormants - 1 ] ] ;
	}else{
		// BACKWARD
		x0 = formantTime[ backwardLegOfSegmentIndices[ numberOfBackLegFormants - 1 ] ] ;
		y0 = formantCF[ backwardLegOfSegmentIndices[ numberOfBackLegFormants - 1 ] ] ;
	} ;
        duration = fabs( x1 - x0 ) ; 
//	changeInSemitones = Hz_to_MIDI( y1 ) - Hz_to_MIDI( y0 ) ;
	changeInFrequency = y1 - y0 ;

//	semitoneChangePerSecond = (duration > 0.) ? (changeInSemitones / duration) : 0. ;
	frequencyChangePerMillisecond = (duration > 0.) ? 
		.001 * (changeInFrequency / duration) : 0. ;

/*
	fprintf( stderr, 
          "\n IN ROUTINE DURATION: %f, CHANGE IN SEMITONES PER SECOND: %f ", 
               duration, semitoneChangePerSecond  ) ; 
*/
	return( changeInFrequency ) ; 

} ;






void get_formants_2( 
	int *numFormants, 
	float formantCenterFreqs[],  
	float formantAmps[], 
	float formantBWs[],
	float formantQs[],
	int formantIndices[],
	int formantLowStopBandIndices[], 
	int formantHighStopBandIndices[], 
	float F[], 
	int N,              
	float lowFreqLimit, 
	float highFreqLimit, 
	float minimumFormantDB,
	float Formant_Selection_Threshold__0_to_1,
	char freqStasisPlotFile[],
	int CorrelateWithFreqStasisFlag,
	float nyquist,


	float amps[], 
	float freqs[], 
	float newAmps[], 
	float AmpsDerivative[], 
	float testAmpsSave[], 
	float freqStasis[], 
	float triWindow[],
	float winArray[], 
	float symmetryFactor[], 
	float v[], 
	float w[], 
	float formantAmpsCopy[], 
	float tempList[],
	float peakAmps[], 
	float avgAmps[], 
	float diffdBs[],

	int HammingWindowSize,
	float HammingWindow[] 




){


float centroidFreq, sumOfAmps, strongestFreq, peakAmp, baseAmp, windowBW, freqsDiffSum, freqsDiffMax,
		avgBinFreqDiff, vv, thisDBthreshold, 
	newAmpsPeakAmp, scaleFactor, peakAmpIn_v_and_w, vSum, wSum, diffProp ; 
int includesflag, tempListLengthNow, sign ;
float thisMaxAmp, sum, sum1, sum2, meanBW, maxBW ;
float TWOPI ;  
 
int lowIndex, highIndex, thisHammingIndex, thisLowIndex, thisHighIndex ;
float cf, fundamental, ampSum, thisHammingSum, thisAmp, HammingFloatIndex  ; 



static int first=1 ; 

float lowdBdiff, highdBdiff, lowFreqDiff, highFreqDiff ; 

float thisCFBinCF, thisCF, thisCFdB, dBdiff, freqDiff, thisFreqDiff, thisFreq ; 
int formantIndex, thisIndex, found ; 

float baseDB, lowDB, highDB, lowFreq, highFreq, temp ; 	

int thisFormant, indexOfFormant, numBins ; 	    

float *tempFreqStasisSpectrum ; 
float stdDev, freqThreshold, freqDiffAvg ; 
FILE *fopen(), *adata;

int avgWinSize, n, c, exitFlag ; 
int N2, i, j, k, l, indexOfPeak ; 

static int lastN=0 ; 

float originalPeakAmp ; 



    TWOPI = 8.*atan(1.) ;


        N2 = N>>1 ; 


	lastN = N ; 

// prt( "H0"); 

    // SEPARATE AMPS FROM FREQS.
    for( i = 0, j = 0; i < N; i += 2, j++ ){
	amps[ j ] = F[ i ] ; freqs[ j ] = F[i + 1] ; 
   } ; 

   // FIND STRONGEST FREQ
    peakAmp = -999999.0 ;
    for( i = 0; i < N2; i++){
	if( amps[i] > peakAmp ){
	    peakAmp = amps[i] ; strongestFreq = freqs[i] ;  ; 
	} ; 
    } ;      
// prt( "H1"); 

    // NORMALIZE
    if( peakAmp < 1.0 ){
	for( i = 0; i < N2; i++) amps[i] /= peakAmp ; 
    } ; 

    // SAVE PEAK AMP FOR RESCALE OF FINAL AMPS
    originalPeakAmp = peakAmp ; 

	// TRANSFER
	for(i = 0; i < N2; i++) newAmps[i] = amps[i] ; 


    // FIND CENTROID
    centroidFreq = 0.0 ; sumOfAmps = 0.0 ; 
    for( i = 0; i < N2; i++) {
	centroidFreq += (freqs[i] * newAmps[i]) ; 
	sumOfAmps += newAmps[i] ; 
    } ; 
    centroidFreq = centroidFreq / sumOfAmps ; 
// prt( "H2"); 
 
	// CREATE BW: IF SMOOTHING SETTING EXISTS, SET IT; OTHERWISE, CREATE ONE 
//    windowBW = strongestFreq / 10 ; 
    windowBW = centroidFreq * SMOOTHING_WINDOW_AS_PROPORTION_OF_CENTROID ; 



	// CREATE AVERAGE BIN FREQ DIFFERENCE
    freqsDiffSum = 0.0 ; freqsDiffMax = -99999999.0 ; 
    for( i = 1; i < N2; i++ ) {
	freqDiff = freqs[i] - freqs[i - 1] ; 
	freqsDiffSum += freqDiff ; 
	if( freqDiff > freqsDiffMax ) freqsDiffMax = freqDiff ; 
    } ; 
    avgBinFreqDiff = freqsDiffSum / (float) (N2 - 1) ; 


//prf( avgBinFreqDiff, "avgBinFreqDiff" ) ; 


	// FREQ STASIS
    freqDiffAvg = 0.0 ;  
    for(i = 0, j = 0; i < N2; i++, j++ ){
	if(i == 0) lowFreqDiff = 999999999.0 ; 
	else lowFreqDiff = fabs( (freqs[i] - freqs[i - 1]) ) ;  
	if(i == (N2 - 1)) highFreqDiff = 999999999.0 ; 
	else  highFreqDiff = fabs( (freqs[i] - freqs[i + 1]) ) ;  
	if(lowFreqDiff < highFreqDiff) freqDiff = lowFreqDiff ; else freqDiff = highFreqDiff ;  
	freqStasis[i] = freqDiff ; freqDiffAvg += freqDiff ; 
    } ; 
    freqDiffAvg /= (float) j ; 
// prt( "H34"); 

    // STD DEV
    stdDev = 0.0 ; 
    for(i = 0; i < N2; i++) stdDev += pow( (freqStasis[i] - freqDiffAvg), 2.0); 
    stdDev = sqrt( stdDev / (float) N2 ) ; 
// prt( "H35"); 

    freqThreshold = freqDiffAvg - (0.5 * stdDev) ; 
    for(i = 0; i < N2; i++){ 
	if( freqStasis[i] > (freqDiffAvg - stdDev) ) freqStasis[i] = 0.0 ; 
	else freqStasis[i] = 1.0 - (freqStasis[i] / (freqDiffAvg - stdDev) ) ;  
	freqStasis[i] = curve( 0.0, 1.0, freqStasis[i], -12 ) ; 
    }; 
// prt( "H36"); 

    if( strcmp( freqStasisPlotFile, "") != 0 ){
	fvec( tempFreqStasisSpectrum, N+2 ) ; 
	for(i = 0, j = 0; i < (N2 + 1); i++, j += 2){
	   tempFreqStasisSpectrum[j] = freqStasis[i] ;
	   tempFreqStasisSpectrum[j + 1] = freqs[i] ;
	} ; 
	writeSpectrumPlotFile( freqStasisPlotFile, tempFreqStasisSpectrum, (N + 2), 0 ) ; 
    }; 
// prt( "H30"); 

	// ********


	// CREATE AVERAGE WINDOW SIZE
    avgWinSize = (int)(windowBW / avgBinFreqDiff) ; 
    if( fmod( (double) avgWinSize, 2.0 ) == 0.0 ) avgWinSize = avgWinSize + 1 ; 
    if( avgWinSize < 3) avgWinSize = 3 ;
    n = (avgWinSize - 1) / 2 ; 
// prt( "H31"); 

	// FIND PEAK AMP
    peakAmp = -999999.0 ; 
    for( i = 0; i < N2; i++ ){
        for( n = 0; n < avgWinSize; n++ ){
	    k = n + i - (avgWinSize / 2) ; 
	    if( k < 0 ) k = 0 ; if( k > (N2 - 1)) k = N2 - 1; 
	    winArray[n] = newAmps[ k ] ; 
	} ; 

	for( j = 0, HammingFloatIndex = 0. ; j < avgWinSize; 
			j++, HammingFloatIndex += ((float) (HammingWindowSize - 1) / (float)(avgWinSize - 1) ) ) 
				winArray[j] = HammingWindow[(int)(HammingFloatIndex + 0.5)] * winArray[j] ; 

		vv = 0.0 ; 
		for( j = 0; j < avgWinSize; j++ ) vv += winArray[j] ; 
		vv = vv / (float) avgWinSize ;   
		newAmps[ i ] = vv ;
		if( newAmps[ i ] > peakAmp) peakAmp = newAmps[ i ] ;  
    } ; 
// prt( "H32"); 
    
    for( i = 0; i < N2; i++) newAmps[ i ] = newAmps[ i ] / peakAmp ; 


// prt( "H33"); 



// prt( "H42");

	// *** MAKE PEAK AMPS AND AVERAGE AMP GRAPHS.
	fundamental = (nyquist * 2.) / (float) N ; 
//prf( fundamental, "fundamental" ) ; 
	temp = ( (centroidFreq * 2.) - (centroidFreq * 0.5) ) * .5  ; 
	for(i = 0; i < N2; i++){
		cf = fundamental * (float)i ; 
//		lowIndex = (int)(0.5 + ((cf * 5./4.) / fundamental)) ; 
//		highIndex = (int)(0.5 + ((cf * 4./5.) / fundamental)) ;  
		lowIndex = (int)(0.5 + ((cf - temp) / fundamental)) ; 
		highIndex = (int)(0.5 + ((cf + temp) / fundamental)) ;  
		ampSum = 0. ; thisHammingSum = 0. ;  peakAmp = -999999.0 ; k = 0 ; 
		for(j = lowIndex; j <= highIndex; j++){
			if( (j >= 0) && (j < (N2 - 1)) ){
				thisHammingIndex =  
					(int)( ((float) HammingWindowSize - .001) * 
						((float)(j - lowIndex) / (float)((highIndex - lowIndex) + 1))) ;
				thisHammingSum +=  HammingWindow[thisHammingIndex] ; 
				thisAmp = newAmps[j] * HammingWindow[thisHammingIndex] ; 
				ampSum += thisAmp ; 
				if( thisAmp > peakAmp) peakAmp = thisAmp ; 
			} ; 
		} ; 
		peakAmps[i] = peakAmp ; avgAmps[i] = ampSum / thisHammingSum ; 
//prf( peakAmps[i], "peakAmps[i]" ) ; 
		diffdBs[i] = amp_to_dB( peakAmps[i] ) - amp_to_dB( avgAmps[i] ) ; 
// if( diffdBs[i] < dB_to_amp(-96.) ) diffAmps[i] = dB_to_amp(-96.) ; 
	} ; 

// prt( "H43");


 
	// FIND SYMMETRIES *********

    for( i = 0; i < N2; i++) symmetryFactor[ i ] = 0.0 ; // ZERO/NULL
    peakAmp = -99999.0 ; 
    for( i = 0; i < N2; i++) if( newAmps[ i ] > peakAmp) peakAmp = newAmps[ i ] ;


// prt( "H41"); 

    k = (avgWinSize - 1) / 2 ; 
    for( i = k; i <= ((N2 - 1) - k); i++ ){
		scaleFactor = newAmps[ i ] / peakAmp ;
		peakAmpIn_v_and_w = -9999999.0 ; vSum = 0.0 ; wSum = 0.0 ; 
		for( n = 0; n < k; n++ ){
	    		j = (i - 1) - n ; if( j < 0 ) j = 0 ; if( j > (N2 - 1)) j = N2 - 1; 
	    		v[n] = newAmps[ j ] ; 
	    		j = (i + 1) + n ; if( j < 0 ) j = 0 ; if( j > (N2 - 1)) j = N2 - 1; 
	    		w[n] = newAmps[ j ] ;
	    		if( v[n] > peakAmpIn_v_and_w ) peakAmpIn_v_and_w = v[n] ; 
	    		if( w[n] > peakAmpIn_v_and_w ) peakAmpIn_v_and_w = w[n] ;
	    		vSum += v[n] ; wSum += w[n] ;  
		}; 
// prt( "H42"); 

		if( newAmps[i] > peakAmpIn_v_and_w ){
	    		if( v < w) {
				symmetryFactor[ i ] = (scaleFactor * vSum ) / wSum ;
	    		} else {
				symmetryFactor[ i ] = (scaleFactor * wSum ) / vSum ;
	    		} ; 

		} else {
			symmetryFactor[i] = 0.0 ; 
			// THIS WAS symmetryFactor.add( 0.0 ) WHICH DOES NOT MAKE SENSE.
		} ; 
// prt( "H43"); 


    } ; 


// prt( "H5"); 

	// FIND FORMANT FREQS AND AMPS
    *numFormants = 0 ;  
	for( i = 1; i <= (N2 - 2); i++ ) { 
		if( (freqs[i] >= lowFreqLimit) && (freqs[i] <= highFreqLimit) &&
			(newAmps[i] > newAmps[i-1]) && (newAmps[i] > newAmps[i+1]) &&
				(newAmps[i] >= dB_to_amp( minimumFormantDB ) )
		)
		{
	    		formantIndex = i; 
	    		for(sign = -1; sign <= 1; sign +=2){
	        			n = formantIndex + sign; found = 0; 
	        			while(found == 0){
	    	    			if((newAmps[n] - newAmps[n + sign]) > 0.0){
						// AMPS STILL DECLINING
						n += sign ; 
						// STOP IF BOTTOM OR TOP REACHED. 
						if( (n < 0) || (n > (N2 - 1)) ) found = 1 ; 
		    			}else{
						// END OF DECLINE
						found = 1; 
		    			} ; 
				} ; 
				if(sign == -1){
					lowDB = amp_to_dB( newAmps[n] ) ; thisLowIndex = n ;  
	            		}else{ 
					highDB = amp_to_dB( newAmps[n] ) ; thisHighIndex = n ; 
				} ; 
	    		}; 
	    		thisCFdB = amp_to_dB( newAmps[formantIndex] );
	    		thisCF = freqs[formantIndex] ; 

//fprintf( stderr, "formantIndex: %d, thisCF: %d, thisCFdB: %d, lowDB: %d, highDB: %d\n",
//(int)formantIndex, (int)thisCF, (int)thisCFdB, (int)lowDB, (int)highDB ) ;  


			// GET DB THRESHOLD FOR THIS CF

			thisDBthreshold = Formant_Selection_Threshold__0_to_1 * diffdBs[i] ; 
			if( 
				(  ((thisCFdB - lowDB) > thisDBthreshold) && ((thisCFdB - highDB) >
 				(thisDBthreshold * (1. - FORMANT_OVERLAP_TOLERANCE_PROP )))  )
						||
				(  ((thisCFdB - lowDB) > (thisDBthreshold * (1. - 
						FORMANT_OVERLAP_TOLERANCE_PROP ))) 
				&& ((thisCFdB - highDB) > thisDBthreshold)  )
	    		){
					// ACCEPT AS  A FORMANT
					formantCenterFreqs[ *numFormants  ] = freqs[formantIndex] ; 
					formantAmps[ *numFormants  ] = amps[formantIndex] ;
					formantIndices[ *numFormants  ] = formantIndex ;
					formantLowStopBandIndices[ *numFormants ] = thisLowIndex ;
					formantHighStopBandIndices[ *numFormants ] = thisHighIndex ;
					*numFormants += 1 ; 
	    		} ; 
		} ; 
    } ; 


// prt( "H6"); 


	// MAKE PROVISIONAL Q AND BW.
   j = 0 ;  
	for( i = 0; i < *numFormants; i++){
 		formantIndex = formantIndices[i] ; thisCF = freqs[ formantIndex ] ;
		thisCFdB = amp_to_dB( amps[formantIndex] );  
		dBdiff = 0.0 ; freqDiff = 0.0  ; 

//		lowFreq = freqs[ formantLowStopBandIndices[i] ] ; 
//		highFreq = freqs[ formantHighStopBandIndices[i] ] ; 
		lowFreq = avgBinFreqDiff * (float) formantLowStopBandIndices[i] ; 
		highFreq = avgBinFreqDiff * (float) formantHighStopBandIndices[i] ; 

/*
		if( (lowFreq > thisCF) || (highFreq < thisCF) ){
			fprintf( stderr, 
		   "\n\n thisCF : %d, lowFreq: %d, highFreq: %d, lowIndex: %d, highIndex: %d\n",
		        (int) thisCF, (int)lowFreq, (int)highFreq, formantLowStopBandIndices[i],
			 formantHighStopBandIndices[i] ) ; 

			for( j = formantLowStopBandIndices[i]; j <= formantHighStopBandIndices[i]; j++ ){
				fprintf( stderr, "\nINDEX: %d, FREQ: %d, dB: %d",
					j, (int) freqs[ j ], (int)amp_to_dB(amps[ j ] ) ) ; 
				if( j == formantIndex ) fprintf( stderr, " *" ) ; 
			} ;


		} ;
*/

		lowDB = amp_to_dB(amps[ formantLowStopBandIndices[i] ]) ; 
		highDB = amp_to_dB(amps[ formantHighStopBandIndices[i] ]) ; 
	
	
		// CREATE A BASE DB TAKEN FROM THE POINT ALONG THE DIFFERENCE BETWEEN LOW AND HIGH DB 
		// LEVELS AT THE POSITION OF THE CF LYING BETWEEN THE LOW AND HIGH FREQS. 
		thisCFBinCF = (float) formantIndex * avgBinFreqDiff ;
		baseDB = 
	    		lowDB + ( (highDB - lowDB) * ((thisCFBinCF - lowFreq) / (highFreq - lowFreq)) ) ; 

		// MAKE Q 		BW / CF
		dBdiff = thisCFdB - baseDB ; 
		formantBWs[i] = (3. / dBdiff) * (highFreq - lowFreq) ; 
		formantQs[i] =  thisCFBinCF / formantBWs[i] ; 

/*
		if( formantBWs[i] <= 0. )
			fprintf( stderr, 
				"\n\nHERE i %d: lowFreq: %d, highFreq: %d, thisCF: %d lowDB: %d highDB: %d baseDB: %d thisCFdB: %d dBdiff: %d\n",
				i, (int) lowFreq, (int)highFreq, (int)thisCF, (int)lowDB, (int)highDB, (int)baseDB, (int)thisCFdB, (int)dBdiff 
			); 


		if( formantBWs[i] <= 0. ){
			fprintf( stderr, "\n\nBANDWIDTH: %f\n", formantBWs[i] ) ; 
			for( j = formantLowStopBandIndices[i]; j <= formantHighStopBandIndices[i]; j++ ){
				fprintf( stderr, "\nINDEX: %d, FREQ: %d, dB: %d",
					j, (int)freqs[ j ], (int)amp_to_dB(amps[ j ] ) ) ; 
				if( j == formantIndex ) fprintf( stderr, " *" ) ; 
			} ;

		} ;
*/

	} ;  

// prt( "H7"); 



    // OMIT FORMANTS THAT DO NOT CORRELATE WITH FREQUENCY STASIS
    if( CorrelateWithFreqStasisFlag == 1){
	j = 0 ;  
	for( i = 0; i < *numFormants; i++){
	    if( freqStasis[ formantIndices[i] ] > 0.0 ){
	        formantAmps[ j ] = formantAmps[ i ] ; 
	        formantCenterFreqs[ j ] = formantCenterFreqs[ i ] ; 
	        formantIndices[ j ] = formantIndices[ i ] ; 
		formantLowStopBandIndices[ j ] = formantLowStopBandIndices[ i ] ; 
		formantHighStopBandIndices[ j ] = formantHighStopBandIndices[ i ] ; 
	        formantBWs[ j ] = formantBWs[ i ] ; 
	        formantQs[ j ] = formantQs[ i ] ; 
	        j++ ; 
	    } ; 
       } ; 
       *numFormants = j ; 
    } ; 



   // RESCALE TO ORIGINAL AMP LEVELS
   for( i = 0; i < *numFormants; i++){
      formantAmps[ i ] = originalPeakAmp * formantAmps[ i ] ;
   } ; 
} ; 




void usage(); 



// ******************************************************************************
int main( argc, argv )
    int argc ; char *argv[] ;
{

int segmentsReviewed=0 ;

float 
	slope_LinkFromCF, yIntercept_LinkFromCF,
	slope_LinkFromAmp, yIntercept_LinkFromAmp,
	slope_LinkFromBW, yIntercept_LinkFromBW,
	slope_LinkFromQ, yIntercept_LinkFromQ,

	slope_LinkToCF, yIntercept_LinkToCF,
	slope_LinkToAmp, yIntercept_LinkToAmp,
	slope_LinkToBW, yIntercept_LinkToBW,
	slope_LinkToQ, yIntercept_LinkToQ

 ; 

float upRamp, downRamp ; 

float previousAbsoluteDistance, linkFromSegYintercept, linkFromSegSlope, linkToSegYintercept, linkToSegSlope,
   xIntersect, yIntersect ;
int intersectionSuccessFlag, linkToLength, linkFromLength ;


float yIntercept, slope ;

int minLength=3 ;

float maximumFrequencyLinkage=100., previousDistance, distance, absoluteFreqDiff ; 

float binaryOutputArray[ 6 ] ;

int printRateModulus ; 

float segmentLinkingTolerancePercentage=1 ; 
float halfLinkTimePoint, beginSegTargetCF, endSegTargetCF ; 


int nextSegIndexBegin, thisSegIndexEnd ; 
float rampUp, rampDown ; 
float thisOnsetDuration, thisReleaseDuration ;

float *thisSegTime, *thisSegCF, *thisSegAmp, *thisSegdB, *thisSegBW, *thisSegQ ;  
int thisSegLength ; 

float linkageTime=0.02 ;
int closestSegment, totalLinks=0, totalSegmentsFollowingLinking=0, *segmentWriteFlag ; 

float frequencyChangePerMillisecond ; 
float outputGroupBeginTime, outputGroupTimeShift ;
int thisLength ; 

int testResult ;
float minimumOverlapInPercent=0. ;

float s0b, s0e, s1b, s1e, s0dur, s1dur ; 
float *shortdBslope, *longdBslope, *shortMidiSlope, *longMidiSlope ;

float *scratchX, *scratchY, *thisSegmentFormantCF ;
int maxLength=10, nextSeg, numberOfSteps, thisSeg ;

float dBabsoluteDiffMean, midiAbsoluteDiffMean ;

float dBabsoluteDiffMeanMinMaxAvg[3], midiAbsoluteDiffMeanMinMaxAvg[3] ;
int numCorrSegsOut=0, numCorrComparisons=0 ;

float dBaccelerationAverage, semitoneAccelerationAverage ;

float dBaccelerationAverageMinMaxAvg[3], semitoneAccelerationAverageMinMaxAvg[3] ;


int numberOfBridgingFramesNow=0, totalBridgesUsed=0, maximumAllowedBridgingFrames=0 ;
float low, hi, avg, length, median, timeShift ;
float dBchangePerMillisecond, 
	MaximumDecibelRisePerMillisecond=90, MaximumDecibelFallPerMillisecond=90 ;

 
int nextGroupIsInAdjacentFrame,
	groupIndexForSelectedFormant, indexOfClosestFormantNow, 
	indexOfClosestFormantAbove, indexOfClosestFormantBelow, proposedFormantIndex,
	numberOfFrontLegFormants, numberOfBackLegFormants, segmentLength, formantIndex ;  
float MaximumFrequencyChangePerMillisecond=12.0 ;
float segmentLegCFaverageNow ;
float semitoneChangePerSecond, frequencyAcceleration, maximumSpeed=50, minimumSpeed=-50, 
	maximumAcceleration=20, minimumAcceleration=-20 ; 

float formantGroupFrameTime ; 

float peakFormantAmp, thisDifference, smallestDifferenceYet ; 
int peakSegIndex, peakFormantIndex, groupIndexForNewFormant, previousGroupIndex, 
	indexOfPreviousFormantInLeg, stillLookingForMoreSegmentFormants,
		direction, acceptableSlope ; 

float formantFrequencyDifference ; 
int *formantFrame, totalNumberOfFormants=0, remainingFormants, *formantGroup ; 
float *formantTime, *formantCF, *formantAmp, *formantBW, *formantQ ;
int *formantSegmentNumber ;

float formantTimeMinMaxAvg[3], formantCFMinMaxAvg[3], formantAmpMinMaxAvg[3], formantBWMinMaxAvg[3], formantQMinMaxAvg[3],
	segmentBridgeLengthsMinMaxAvg[3] ;

float segmentLengthMinMaxAvg[3] ;

int *segmentLengths, *segmentBeginDataIndices, *segmentEndDataIndices, *segmentSwitches,
   *linkToSegment, *linkFromSegment ;

float *segmentBeginTimes, *segmentEndTimes, *segmentPeakAmps ;

float targetFormantCF, targetFormantChangeScaler ; 
int *indexOfPrecedingFormant, *indexofFollowingFormant ; 
int *formantTimePointGroupIndex ; 
int *frameGroupBeginIndex, *frameGroupEndIndex ; 
int *formantSwitch, *peakFormantSwitch ;
long numberOfSegmentsFilePosition, segmentLengthFilePosition, filePositionNow ; 

int indexOfSegmentWithGreatestAmp, indexOfLongestSegment, 
	indexOfGuideSegment, maximumNumberOfOutputGroups=30, 
	numberOfOutputGroups=0, longestSegmentLength ;
int formantsStillRemain, outputSegmentLength, totalFormantsInAllSegments=0, 
	numberOfSegments, numberOfSegmentsFoundBeforeSelection=0, numberOfSegmentsFoundAfterSelection=0, totalSegmentsLimit=-1 ; 
int *forwardLegOfSegmentIndices, *backwardLegOfSegmentIndices, *thisSegment, *thisSegmentNumberOfBridgeFrames ; 
int *numberOfBridgeFramesToForwardLegIndex, *numberOfBridgeFramesToBackwardLegIndex ;  
float * thisSegmentEnvelope ; 

int *formantGroupBeginIndex, *formantGroupEndIndex ; 

float formantAmplitudeMinimumThreshold, formantAmplitudeMaximumThreshold,
minimumDecibels=-200.0, maximumDecibels=0.0 ;
int minimumSegmentLength=1, maximumSegmentLength=(-1) ; 
float minimumSegmentDuration=0., maximumSegmentDuration=0., segmentDuration ; 


float timeNow ; 
int frameGroupIndex, formantGroupFrameNow, *formantGroupFrame, formantGroupIndex, 
	numberOfFrames=0, numberOfGroups=0, maximumFormantsFoundInAnyGroup=0 ; 

int nn,  sec,  min ; 
float oldt ; 
float time, lastPassifierPrintTime, lastPassifierSecondsPrint ;
int i,j, k, l, jj, ii,   exflag, numberOfBridgeFrames;
float nyquist,   fundamental, frameDuration, bridgeProportion ;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0 ;
float P = 1.0, coef=0.0 ;
FILE *fopen(), *fp,  *adata, *ASCIIfrequencyScatterPlot, *BinaryDataPointsFile, 
	*ASCIIfrequencySegmentsPlot,
	*intermediaryASCIIfrequencySegmentsPlot, *ASCIIfrequencySegmentsPlotPeakPoints, 
	*ASCIIfrequencySegmentsPlotRejects, *intermediaryBinaryFrequencySegmentsFile,
	*postCorrelationASCIIfrequencySegmentsPlot,
	*BinaryFrequencySegmentsFile ;
char ch ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,   
	*previous_channel, *output ;
float *F, FreqStasisSpectrumAmpSum=0.0, *AmplitudeSpectrum, 
		*AmplitudeSpectrumForFormantSelection, *OutputSpectrum, *SD, channel_Save, threshold ;
float *PeakWindowedAmps, *AvgWindowedAmps ; 
int numSaveFrames, saveFramesCount ;  
float	gain=1., stdDev  ;
float  temp,  temp2,  temp3,  temp4,  temp5,  temp6  ;
float getthresh();
float peakbinamp = 0.,  avgbinamp=0.,  peakamp,  dur, minDev, maxDev ;
float freqChangePerMillisecond=1.0 ; 

int numFormants, *formantIndices, *formantLowStopBandIndices, *formantHighStopBandIndices ;


float *formantCenterFreqs, *formantAmps, *formantBWs, *formantQs ; 

float formantData[ 6 ] ; 

float minimumFormantDB=-96. ;  

int CorrelateWithFreqStasisFlag=0 ; 

char intermediaryASCIIfrequencySegmentsPlotFileName[ STRING_SIZE ]="/tmp/sp",
	ASCIIfrequencySegmentsPlotFileName[ STRING_SIZE ]="",
	BinaryFrequencySegmentsFileName[ STRING_SIZE ]="" ; 

char freqStasisPlotFile[ STRING_SIZE ]="" ; 

char ASCIIfrequencyScatterPlotFile[ STRING_SIZE ]="" ; 



float *barPlot, minFormantAmp, minFormantdB  ; 
int peakFormantFreq ; 

float peakdB, avgdB, diffdB, dBadjust ; 

int lowIndex, highIndex, thisHammingIndex, HammingWindowSize=1024 ; 
float ampSum, thisHammingSum, peakAmp, *peakAvgProps, *HammingWindow, 
	cf, peakAveragePropIndB ; 

int add_onset_and_release_points__no_0__append_1__impose_2=0 ;
float onset_duration=-1.0, release_duration=-1.0 ;


float *amps; 
float *freqs; 
float *newAmps; 
float *AmpsDerivative; 
float *testAmpsSave; 
float *freqStasis; 
float *triWindow;
float *winArray; 
float *symmetryFactor; 
float *v; 
float *w; 
float *formantAmpsCopy; 
float *tempList; // tempListLengthNow
float *peakAmps; 
float *avgAmps; 
float *diffdBs; 

float testX[6]={0, 1, 2, 3, 4, 5}, testY[6]={0, 1, 2, 3, 4, 5} ;

int numberOfOnsetFrames, numberOfReleaseFrames ;

float thisSegmentTime, thisCF, thisAmp, thisdB, thisBW, thisQ, previousSegmentTime, thisDuration ; 
float previousCF, previousAmp, previousBW, previousQ ; 

int numClipped=0 ; 
int Formants_N ; 
int   print_flag=0, buffer_count=0 ; 
float   IR, DR,  n=2048 ;
char tempstring[ 5000 ] ; 

// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 
int eqnormbypassflag=0 ; 

// LOW FREQ LIMIT
struct func lowFreqLimit ; 

// LOW FREQ LIMIT
struct func highFreqLimit ; 

// FILTER
struct  func  freqresponse ; 


// LOW FREQ LIMIT
lowFreqLimit.L = 1. ; lowFreqLimit.n = 0. ; lowFreqLimit.A[ 0 ] = 0. ; 

// HIGH FREQ LIMIT
highFreqLimit.L = 1. ; highFreqLimit.n = 0. ; highFreqLimit.A[ 0 ] = nyquist ; 

// FILTER
freqresponse.L = 1. ; freqresponse.n = 0. ; freqresponse.A[ 0 ] = 0. ; 

float Formant_Selection_Threshold__0_to_1=0.5 ; 



if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, 
	"a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|u|U|v|V|w|W|X|", 0  )) 
		!= CRACK_DONE_FLAG ) {
	switch(ch) { // K x T y Y z Z


	    case 'S':   strcpy(BinaryFrequencySegmentsFileName, arg_option); 
			break;


	    case 'W':   maximumFrequencyLinkage =  crackfloat( arg_option, ch ) ;
			break;

	    case 'r':   linkageTime =  crackfloat( arg_option, ch ) ;
			break;

//	    case 'T':   segmentLinkingTolerancePercentage =  crackfloat( arg_option, ch ) ;
//			break;

	    case 'V':   MaximumDecibelRisePerMillisecond =  crackfloat( arg_option, ch ) ;
			break;
	    case 'v':   MaximumDecibelFallPerMillisecond =  crackfloat( arg_option, ch ) ;
			break;

	    case 'n':   timeShift =  crackfloat( arg_option, ch ) ;
			break;

//	    case 'K':   maximumAllowedBridgingFrames = (int) crackfloat( arg_option, ch ) ;
//			break;


	    case 'u':   onset_duration = crackfloat( arg_option, ch ) ;
			break;

	    case 'U':   release_duration = crackfloat( arg_option, ch ) ;
			break;


	    case 'F':   add_onset_and_release_points__no_0__append_1__impose_2 = 
				(int) crackfloat( arg_option, ch );
			break;

	    case 'p':   MaximumFrequencyChangePerMillisecond = crackfloat( arg_option, ch ) ;
			break;

	    case 'c':   minimumDecibels = crackfloat( arg_option, ch );
			break;
	    case 'E':   maximumDecibels = crackfloat( arg_option, ch );
			break;


	    case 'o':   minimumSegmentLength = (int) crackfloat( arg_option, ch );
			break;
	    case 'O':   maximumSegmentLength = (int) crackfloat( arg_option, ch );
			break;

	    case 'q':   minimumSegmentDuration = crackfloat( arg_option, ch );
			break;
	    case 'Q':   maximumSegmentDuration = crackfloat( arg_option, ch );
			break;


	    case 'f':   strcpy(ASCIIfrequencyScatterPlotFile, arg_option) ; 
			break;

	    case 'N':   N = (int) crackfloat( arg_option, ch );
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;


	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;




	    case 'B':   eqnormbypassflag = (int) crackfloat( arg_option, ch ) ;
			break;




	    case 'H':   dBlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'X':   dBhi = crackfloat( arg_option, ch ) ;
			break;
	    case 'm':   freqlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'R':   freqhi = crackfloat( arg_option, ch ) ;
			break;


// *****

	    case 'L':   strcpy(tempstring, arg_option);
			lowFreqLimit.fp = crackstring( tempstring, 
			    &lowFreqLimit );
			break;
	    case 'j':   strcpy(tempstring, arg_option);
			highFreqLimit.fp = crackstring( tempstring, 
			    &highFreqLimit );
			break;
	    case 'A':   minimumFormantDB = crackfloat( arg_option, ch ) ;
			break;



	    case 'g':   Formant_Selection_Threshold__0_to_1 = crackfloat( arg_option, ch ) ;
			break;

	    case 'a':   strcpy(ASCIIfrequencySegmentsPlotFileName, arg_option); 
			break;



	

// ******


	    case 'P':   print_flag = (int) crackfloat( arg_option, ch ) ;
			break;
	}
    }



            temp = linearLeastSquaresProjection(
               0.,
               testX,  
               testY, 
               6,
               &linkToSegYintercept,
               &linkToSegSlope
            ); 
fprintf( stderr, "\n linkToSegYintercept: %f linkToSegSlope: %f", linkToSegYintercept, linkToSegSlope ) ; 




prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "SPECTRUM MAPPER", 69 ) ; 
prline( 69,  "-" ) ; 

prf( MaximumFrequencyChangePerMillisecond, "MAXIMUM FREQUENCY CHANGE PER MILLISECOND" ) ; 

prf( MaximumDecibelRisePerMillisecond, "MAXIMUM DECIBEL RISE PER MILLISECOND" );  
prf( MaximumDecibelFallPerMillisecond,  "MINIMUM DECIBEL FALL PER MILLISECOND" ); 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
    // SET NO OUTPUT FLAG
    outputoff=1;

// GET INPUT HEADER INFO
    setupfiles(argc, argv) ; 

    endchan = beginchan + ochan ; 

     
// **** SET UPS *****
    R = isr ; // SAMPLE RATE EQUALS INPUT FILE
    if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 200.\n\n" ) ; 
	frames_per_sec = 200 ; 
    }
    D = (int) ((float) R / frames_per_sec) ; 
    frameDuration = 1.0 / (float) frames_per_sec ;

   prf(  frameDuration, "FRAME DURATION" ) ;  
//*****
//******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
//*********************************


    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
    fundamental =  ((float) R / (float) N) ; 
    obank = P != 0. ;
    if( P == 0.0 ) {P = 1.0;}
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    DR = (float) D / (float) R ; 
    // COMPUTE THE DURATION
    dur = (endt - begint) ; 

//	if( highFreqLimit == 0. ) highFreqLimit = nyquist ; 

    
//***************** PRINT VALUES
prf( dur, "ANALYSIS SEGMENT DURATION" ) ; 

prbanner( "SPECTRUM MAPPER ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 

pri( frames_per_sec,  "FRAMES/SECOND" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
prline( 1,  "*" ) ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
if(eqnormbypassflag == 0)prt( "EQ WITH NORMALIZATION: ON" ) ; 
    else prt( "EQ WITH NORMALIZATION: OFF" ) ;
prt( "*...........................................*" ) ;  


prp( &lowFreqLimit, "FORMANT ANALYSIS: LOW FREQUENCY LIMIT" ) ; 
prp( &highFreqLimit, "FORMANT ANALYSIS: HIGH FREQUENCY LIMIT" ) ; 

prf( minimumFormantDB, "FORMANT ANALYSIS: MINIMUM FORMANT AMPLITUDE IN DB" ) ; 
prf( Formant_Selection_Threshold__0_to_1, "0-1 Formant Selection/Rejection Threshold" ) ; 


	
//if( CorrelateWithFreqStasisFlag == 1)prt( "CORRELATING FORMANTS WITH FREQUENCY STASIS" ) ;



// *******

fvec( Wanal, Nw ) ;		/* analysis window */
fvec( Wsyn, Nw ) ;		/* synthesis window */
fvec( input, Nw ) ;		/* input buffer */
fvec( Hwin, Nw ) ;		/* plain Hamming window */
fvec( winput, Nw ) ;	/* windowed input buffer */
fvec( buffer, N ) ;		/* FFT buffer */
fvec( channel, N+2 ) ;	/* analysis channels */
fvec( previous_channel, N+2 ) ;	/* analysis channels */
fvec( output, Nw ) ;	/* output buffer */


//fvec( peakAmps, N2 ) ; fvec( avgAmps, N2 ) ;

// MAKE SPECTRUM AVERAGE/PEAK/MINIMUM ARRAY
fvec( AmplitudeSpectrum, N+2 ) ;	/* spectrum average or peak array */

fvec( OutputSpectrum, N+2 ) ; 

fvec( HammingWindow, HammingWindowSize ) ; 

fvec( amps, N2+1 ) ; 
fvec( freqs, N2+1 ) ; 
fvec( newAmps, N2+1 ) ; 
fvec( AmpsDerivative, N2+1 ) ; 
fvec( testAmpsSave, N2+1 ) ; 
fvec( freqStasis, N2+1 ) ; 
fvec( triWindow, N2+1 ) ;
fvec( winArray, N2+1 ) ; 
fvec( symmetryFactor, N2+1 ) ; 
fvec( v, N2+1 ) ; 
fvec( w, N2+1 ) ; 
fvec( formantAmpsCopy, N2+1 ) ; 
fvec( tempList, N2+1 ) ;
fvec( peakAmps, N2+1 ) ; 
fvec( avgAmps, N2+1 ) ; 
fvec( diffdBs, N2+1 ) ; 



// FIRST MAKE HAMMING WINDOW 
fvec( HammingWindow, HammingWindowSize ) ; 
for ( i = 0 ; i < HammingWindowSize ; i++ ){
	HammingWindow[i] = 0.54 - (0.46 * cos( (double)(TWOPI * (float) i / (float)(HammingWindowSize - 1)  ) ) ) ;
} ;


fvec( formantCenterFreqs, N2+1 ) ; fvec( formantAmps, N2+1 ) ; 
fvec( formantBWs, N2+1 ) ; fvec( formantQs, N2+1 ) ; 
ivec( formantIndices, N2+1 ) ; 
ivec( formantLowStopBandIndices, N2+1 ) ; ivec( formantHighStopBandIndices, N2+1 ) ; 


// OPEN SCATTER PLOT ASCII FILE FOR WRITING
ASCIIfrequencyScatterPlot = fopen( ASCIIfrequencyScatterPlotFile, "w" ) ; 

// OPEN BINARY DATA POINTS FILE FOR WRITING
BinaryDataPointsFile = fopen( "/tmp/tempDataPointsFile", "w" ) ; 

fseek( BinaryDataPointsFile, 0, SEEK_SET ) ;  


// OPEN INPUT  AND OUTPUT FILES
openfiles() ; 


minDev = 999999999999.0 ; maxDev = -9999999.0 ; avg = 0.0 ; 


//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0  ; 

// pd( 200 );


    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;

// pd( 201 );

	oldt = 0. ; 
	sec = 0 ;
	min = 0 ;  
	nn = 0 ;

// pd( 202 );


	lastPassifierPrintTime = -0.000001 ;	
	lastPassifierSecondsPrint = -0.000001  ; 
//*********************************************
// LOOP FOR FRAMES
//*********************************************

    while ( !eof ) {
	in += D ;
	on += I ;
	timenow( dur ) ;

//	time = ((float) frame_count / (float) frames_per_sec) ; 
	if( frame_count == 0 ) time = 0. ; 
	else time = (float)(frame_count * D) / (float) R  ; 


        if( frame_count == 0 ) {
		prf( begint, "BEGIN TIME" ) ; 
		prf( endt, "END TIME" ) ; 
		prf( dur, "ANALYSIS DURATION" ) ;
		prt( "\n" );
		prt( "TIME IN SECONDS:\n") ;   
	} ; 

	if( time > lastPassifierPrintTime ){
		if( time > lastPassifierSecondsPrint ){
			fprintf( stderr, "( %d )", (int) time ) ; lastPassifierSecondsPrint += 1.0  ;
		} else{
			fprintf( stderr, "*" ) ; 
		} ;  
		lastPassifierPrintTime += 0.1 ;
	}; 
	


//	fprintf( stderr, "\nTIME: %f %f", time, t ) ; 


        lowFreqLimit.A[ 0 ] = fval( &lowFreqLimit, dur, time );
        highFreqLimit.A[ 0 ] = fval( &highFreqLimit, dur, time );



	eof = shiftin( input, Nw, D ) ;
	fold( input, Wanal, Nw, buffer, N, in ) ;
	rfft( buffer, N2, FORWARD ) ;
	convert( buffer, channel, N2, D, R ) ;


	if( frame_count == 0 ) 
		for( i = 0; i < (N + 2); i++ ) previous_channel[i] =  channel[i] ;  



//*****************
// MODIFICATIONS LOOP
//*****************




	buffer_count++ ;
	for( i = 0; i < (N + 2); i++ ) previous_channel[i] = 
			((1.0 - coef) * channel[i])  + (coef * previous_channel[i]) ;  

    	// TRANSFER TO OUTPUT SPECTRUM
	for( i = 0; i < (N + 2); i++ ) OutputSpectrum[i] = channel[i] ; 

    	// IF FLAG OFF, EQUALIZE AND NORMALIZE THE OUPUT SPECTRUM
	if( eqnormbypassflag != 1 ){
    		eq( OutputSpectrum,  (N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1, 0, 0 ) ; 
	} ; 

	temp = findPeakAmp( OutputSpectrum, (N + 2) );  

//	fprintf( stderr, "\nPEAK AMP: %f    %f dB ", temp, amp_to_dB( temp ) ) ;  


	get_formants_2( 
		&numFormants,  
		formantCenterFreqs, formantAmps, formantBWs, formantQs, 
		formantIndices, formantLowStopBandIndices, formantHighStopBandIndices, 
		OutputSpectrum, (N + 2), 
		lowFreqLimit.A[ 0 ], 
		highFreqLimit.A[ 0 ], 
		minimumFormantDB,
		Formant_Selection_Threshold__0_to_1, freqStasisPlotFile, CorrelateWithFreqStasisFlag, nyquist, 

		amps, 
		freqs, 
		newAmps, 
		AmpsDerivative, 
		testAmpsSave, 
		freqStasis, 
		triWindow,
		winArray, 
		symmetryFactor, 
		v, 
		w, 
		formantAmpsCopy, 
		tempList,
		peakAmps, 
		avgAmps, 
		diffdBs,

		HammingWindowSize,
		HammingWindow

	) ; 
// fprintf( stderr, "\nNUMBER OF FORMANTS: %d", numFormants ) ; 
	numberOfFrames++ ;

	if( numFormants > 0 ) numberOfGroups++ ;

	for( i = 0; i < numFormants; i++ ){ 
		sprintf( tempstring, "%f %f\n\n", time, formantCenterFreqs[ i ] ) ;
		fprintf( ASCIIfrequencyScatterPlot, "%s", tempstring ) ; 
	}; 

	if( numFormants > maximumFormantsFoundInAnyGroup )
		maximumFormantsFoundInAnyGroup = numFormants ; 



	// WRITE FORMANTS TO TEMP FILE
	for( i = 0; i < numFormants; i++ ){

		formantData[ 0 ] = frame_count ; // TIME
		formantData[ 1 ] = time ; // TIME
		formantData[ 2 ] = formantCenterFreqs[ i ] ; // CF
		formantData[ 3 ] = formantAmps[ i ] ; // AMP
		formantData[ 4 ] = formantBWs[ i ]  ; // BW
		formantData[ 5 ] = formantQs[ i ] ; // Q

		fwrite( &formantData[ 0 ], sizeof(float), 1, BinaryDataPointsFile ) ;
		fwrite( &formantData[ 1 ], sizeof(float), 1, BinaryDataPointsFile ) ;
		fwrite( &formantData[ 2 ], sizeof(float), 1, BinaryDataPointsFile ) ;
		fwrite( &formantData[ 3 ], sizeof(float), 1, BinaryDataPointsFile ) ;
		fwrite( &formantData[ 4 ], sizeof(float), 1, BinaryDataPointsFile ) ;
		fwrite( &formantData[ 5 ], sizeof(float), 1, BinaryDataPointsFile ) ;
		totalNumberOfFormants ++ ;


/*
		fprintf( stderr, 
			"\n%d %f %f %f %f %f ", 
			(int) formantData[ 0 ], formantData[ 1 ], 
				formantData[ 2 ], formantData[ 3 ], formantData[ 4 ], 
					formantData[ 5 ] ) ;	
*/

	} ;	

//	


		frame_count++ ; 


	// FRAMES LOOP END
    }

    	prt( "\n . . . . . DONE!" ) ; 

// pd( 2 );
	// CHANNELS LOOP END
} 



// OPEN BINARY DATA POINTS FILE FOR WRITING


    // CLOSE  INPUT FILE
fclose(ifd) ;  


fprintf( stderr, "\n\nNUMBER OF FORMANT POINTS: %d\n", totalNumberOfFormants ) ; 


fclose( ASCIIfrequencyScatterPlot ) ;



fprintf( stderr, "\n\nNUMBER OF FRAMES: %d", numberOfFrames ); 

fclose( BinaryDataPointsFile ) ; 



// OPEN BINARY DATA POINTS FILE FOR WRITING
BinaryDataPointsFile = fopen( "/tmp/tempDataPointsFile", "r" ) ; 


// MAKE ARRAYS TO HOLD FORMANT POINTS PARAMETERS
ivec( formantFrame, totalNumberOfFormants ) ; 
fvec( formantTime, totalNumberOfFormants ) ; 
fvec( formantCF, totalNumberOfFormants ) ; 
fvec( formantAmp, totalNumberOfFormants ) ; 
fvec( formantBW, totalNumberOfFormants ) ; 
fvec( formantQ, totalNumberOfFormants ) ; 
ivec( formantGroup, totalNumberOfFormants ) ; 
ivec( formantSwitch, totalNumberOfFormants ) ; 
ivec( peakFormantSwitch, totalNumberOfFormants ) ; 


ivec( formantGroupFrame, numberOfGroups ) ; 

ivec( formantGroupBeginIndex, numberOfGroups ) ; 
ivec( formantGroupEndIndex, numberOfGroups ) ; 

ivec( forwardLegOfSegmentIndices, numberOfFrames ) ; 
ivec( backwardLegOfSegmentIndices, numberOfFrames ) ; 
ivec( numberOfBridgeFramesToForwardLegIndex, numberOfFrames ) ; 
ivec( numberOfBridgeFramesToBackwardLegIndex, numberOfFrames ) ; 



ivec( thisSegment, numberOfFrames ) ; 
fvec( thisSegmentEnvelope, numberOfFrames ) ; 
ivec( thisSegmentNumberOfBridgeFrames, numberOfFrames ) ; 


timeNow = -1.0 ; 
formantGroupIndex = -1 ; 
peakAmp = 0.0 ; 
formantGroupFrameNow = -1 ; 

// READ IN DATA AND INITIALIZE ARRAYS
for( i = 0; i < totalNumberOfFormants; i++ ){

	fread( &temp, sizeof( float ), 1, BinaryDataPointsFile ) ; 
	formantFrame[ i ] = (int) temp ;
	fread( &temp, sizeof( float ), 1, BinaryDataPointsFile ) ; 
	formantTime[ i ] = temp ;
	fread( &temp, sizeof( float ), 1, BinaryDataPointsFile ) ; 
	formantCF[ i ] = temp ;
	fread( &temp, sizeof( float ), 1, BinaryDataPointsFile ) ; 
	formantAmp[ i ] = temp ;
	fread( &temp, sizeof( float ), 1, BinaryDataPointsFile ) ; 
	formantBW[ i ] = temp ;
	fread( &temp, sizeof( float ), 1, BinaryDataPointsFile ) ; 
	formantQ[ i ] = temp ;

	if( i == 0 ){
		formantTimeMinMaxAvg[0] = formantTime[ i ] ;
		formantCFMinMaxAvg[0] = formantCF[ i ] ;
		formantAmpMinMaxAvg[0] = formantAmp[ i ] ;
		formantBWMinMaxAvg[0] = formantBW[ i ] ;
		formantQMinMaxAvg[0] = formantQ[ i ] ;
		formantTimeMinMaxAvg[1] = formantTime[ i ] ;
		formantCFMinMaxAvg[1] = formantCF[ i ] ;
		formantAmpMinMaxAvg[1] = formantAmp[ i ] ;
		formantBWMinMaxAvg[1] = formantBW[ i ] ;
		formantQMinMaxAvg[1] = formantQ[ i ] ;
		formantTimeMinMaxAvg[2] = formantTime[ i ] ;
		formantCFMinMaxAvg[2] = formantCF[ i ] ;
		formantAmpMinMaxAvg[2] = formantAmp[ i ] ;
		formantBWMinMaxAvg[2] = formantBW[ i ] ;
		formantQMinMaxAvg[2] = formantQ[ i ] ;
	}else{
		if( formantTime[ i ] < formantTimeMinMaxAvg[ 0 ] )
			formantTimeMinMaxAvg[ 0 ] = formantTime[ i ] ;
		if( formantCF[ i ] < formantCFMinMaxAvg[ 0 ] )
			formantCFMinMaxAvg[ 0 ] = formantCF[ i ] ;
		if( formantAmp[ i ] < formantAmpMinMaxAvg[ 0 ] )
			formantAmpMinMaxAvg[ 0 ] = formantAmp[ i ] ;
		if( formantBW[ i ] < formantBWMinMaxAvg[ 0 ] )
			formantBWMinMaxAvg[ 0 ] = formantBW[ i ] ;
		if( formantQ[ i ] < formantQMinMaxAvg[ 0 ] )
			formantQMinMaxAvg[ 0 ] = formantQ[ i ] ;

		if( formantTime[ i ] > formantTimeMinMaxAvg[ 1 ] )
			formantTimeMinMaxAvg[ 1 ] = formantTime[ i ] ;
		if( formantCF[ i ] > formantCFMinMaxAvg[ 1 ] )
			formantCFMinMaxAvg[ 1 ] = formantCF[ i ] ;
		if( formantAmp[ i ] > formantAmpMinMaxAvg[ 1 ] )
			formantAmpMinMaxAvg[ 1 ] = formantAmp[ i ] ;
		if( formantBW[ i ] > formantBWMinMaxAvg[ 1 ] )
			formantBWMinMaxAvg[ 1 ] = formantBW[ i ] ;
		if( formantQ[ i ] > formantQMinMaxAvg[ 1 ] )
			formantQMinMaxAvg[ 1 ] = formantQ[ i ] ;

		formantTimeMinMaxAvg[2] += formantTime[ i ] ;
		formantCFMinMaxAvg[2] += formantCF[ i ] ;
		formantAmpMinMaxAvg[2] += formantAmp[ i ] ;
		formantBWMinMaxAvg[2] += formantBW[ i ] ;
		formantQMinMaxAvg[2] += formantQ[ i ] ;
	} ; 


	if( formantFrame[ i ] != formantGroupFrameNow ){
		formantGroupIndex++ ; 
		formantGroupFrameNow = formantFrame[ i ] ; 
		formantGroupFrame[ formantGroupIndex ] = formantGroupFrameNow ;
		formantGroupBeginIndex[ formantGroupIndex ] = i ;
		if( formantGroupIndex != 0 ) formantGroupEndIndex[ formantGroupIndex - 1 ] = i - 1 ;
	} ; 	

	formantGroup[ i ] = formantGroupIndex ;
	formantSwitch[ i ] = 1 ; peakFormantSwitch[ i ] = 1 ; 
	if( formantAmp[ i ] > peakAmp ) peakAmp = formantAmp[ i ] ;


/*
	fprintf( stderr, 
		"\n%d: %d, %d %f %f %f %f %f", i, 
		formantFrame[ i ], formantGroup[ i ], formantTime[ i ], formantCF[ i ], 
			formantAmp[ i ], formantBW[ i ], formantQ[ i ] 
			  ) ;	
*/

} ; 

formantTimeMinMaxAvg[2] /= (float) totalNumberOfFormants ;
formantCFMinMaxAvg[2] /= (float) totalNumberOfFormants ;
formantAmpMinMaxAvg[2] /= (float) totalNumberOfFormants ;
formantBWMinMaxAvg[2] /= (float) totalNumberOfFormants ;
formantQMinMaxAvg[2] /= (float) totalNumberOfFormants ;



//pd(1000) ; 

formantGroupEndIndex[ numberOfGroups - 1 ] = totalNumberOfFormants - 1 ; 

//pd(1001) ;

// NORMALIZE AMPLITUDES FOR ALL FORMANTS
for( i = 0; i < totalNumberOfFormants; i++ ){
	formantAmp[ i ] = formantAmp[ i ] / peakAmp ; 
} ;

//pd(1002) ;
fclose( BinaryDataPointsFile ) ; 

fvec( scratchX, numberOfGroups ) ;
fvec( scratchY, numberOfGroups ) ; 


//pd(1003) ;
// 
if( maximumSegmentLength <= 0 ) maximumSegmentLength = numberOfFrames ;
if( minimumSegmentLength <= 0 ) minimumSegmentLength = 1 ;

if( maximumSegmentDuration <= 0. ) maximumSegmentDuration = dur * 2.0  ;
if( minimumSegmentDuration <= 0. ) minimumSegmentDuration = 0. ;

if( add_onset_and_release_points__no_0__append_1__impose_2 > 0 ){
	if( onset_duration < 0.0 ) onset_duration = 1.0 / (float) frames_per_sec ;
	if( release_duration < 0.0 ) release_duration = 1.0 / (float) frames_per_sec ;
}else{
	onset_duration = 0.0 ; release_duration = 0.0 ;
} ;

//pd(1004) ;
segmentBridgeLengthsMinMaxAvg[2] = 0. ;

//totalSegmentsLimit = ??? ; 

// TURN OFF ALL FORMANTS WITH AMPLITUDE BELOW THE SPECIFIED THRESHOLD
formantAmplitudeMinimumThreshold = dB_to_amp( minimumDecibels ) ; 
formantAmplitudeMaximumThreshold = dB_to_amp( maximumDecibels ) ; 
for( i = 0; i < totalNumberOfFormants; i++ ){
	if( (formantAmp[ i ] <= formantAmplitudeMinimumThreshold) ||
	 (formantAmp[ i ] >= formantAmplitudeMaximumThreshold) ) {
		formantSwitch[ i ] = 0 ;
//		fprintf( stderr, "\nFORMANT: %d TURNED OFF", i);   
	} ;
} ;

// LOOP FOR LOOKING FOR A FORMANT TO SEQUENCE 
formantsStillRemain = 1 ;  

// PLOT FILE FOR SEGMENTS
intermediaryASCIIfrequencySegmentsPlot = fopen( intermediaryASCIIfrequencySegmentsPlotFileName, "w" ) ; 

ASCIIfrequencySegmentsPlot = fopen( ASCIIfrequencySegmentsPlotFileName, "w" ) ; 
BinaryFrequencySegmentsFile = fopen( BinaryFrequencySegmentsFileName, "w+" ) ; 






sprintf( tempstring, "%sPeakPoints", intermediaryASCIIfrequencySegmentsPlotFileName ) ;
ASCIIfrequencySegmentsPlotPeakPoints = fopen( tempstring, "w" ) ; 

sprintf( tempstring, "%sRejects", intermediaryASCIIfrequencySegmentsPlotFileName ) ;
ASCIIfrequencySegmentsPlotRejects = fopen( tempstring, "w" ) ; 


// BINARY FILE FOR TEMPORARY STORAGE OF SEGMENTS
intermediaryBinaryFrequencySegmentsFile = fopen( "/tmp/intermediaryBinaryFrequencySegmentsFile", "wb" ) ; 
// WRITE DUMMY NUMBER OF SEGMENTS
fwrite( &numberOfSegmentsFoundAfterSelection, sizeof(int), 1, intermediaryBinaryFrequencySegmentsFile ) ;



// WHILE LOOP FOR FINDING/CONSTRUCTING ALL (OR THE MINIMUM DESIRED) SEGMENTS.
while( (formantsStillRemain == 1) && 
	(( numberOfSegmentsFoundAfterSelection < totalSegmentsLimit ) || (totalSegmentsLimit == -1)) 
){
   formantsStillRemain = 0 ; // TURN OFF FORMANT-FOUND FLAG BEFORE SEARCH FOR FORMANT
   peakFormantAmp = 0. ; 
	
   // **** FIND THE STRONGEST FORMANT NOT YET USED. *** 
   // *** DO THIS BY LOOKING AT EVERY FORMANT . . . 
   remainingFormants = totalNumberOfFormants ;
   for(i = 0; i < totalNumberOfFormants; i++){ 
      // AND IF THE FORMANT IS STILL ON . . . 
      if( (formantSwitch[ i ] == 1) && (peakFormantSwitch[ i ] == 1) ){
         // (FORMANT IS ON)
         // THEN COMPARE ITS AMP TO THE PREVIOUSLY FOUND PEAK. 
         // IF IT IS STRONGER . . . 

         if(  formantAmp[ i ] > peakFormantAmp ){
            // (STRONGER PEAK THAN PREVIOUS)  
            // THEN SAVE ITS AMP AND INDEX 
            // AS THE STRONGEST PEAK FOUND AMONGST THE REMAINING FORMANTS.
            peakFormantAmp = formantAmp[ i ] ; peakFormantIndex = i ; 
            // SET FLAG TO INDICATE THAT A FORMANT HAS BEEN FOUND. 
            formantsStillRemain = 1 ;
         } ; 
      }else{
         remainingFormants-- ;
      } ;
   } ; // END OF LOOP FOR LOOKING AT EVERY FORMANT

   // PASSIFIER PRINT
   if( fmodf((float) remainingFormants, 500. ) == 0. ){
      temp = ((float) remainingFormants / (float) totalNumberOfFormants ) ;
      k = (int)(100. * temp) + 1 ;
      fprintf( stderr, "\n%d%% REMAINING - ", (int) (temp * 100.0) ) ;
      for( i = 0; i < k; i++ ) fprintf( stderr, "*" ) ; 
       
   } ;

   // *** IF A PEAK FORMANT WAS FOUND IN ALL THE DATA, THEN ATTEMPT CONSTRUCTION OF
   // A LEG ON EACH SIDE OF IT.
   if( formantsStillRemain == 1 ){
      peakFormantSwitch[ peakFormantIndex ] = 0 ; 
//pd(1008) ;

//fprintf( stderr, "\nFORMANT FOUND FOR PEAK, INDEX: %d", peakFormantIndex ) ; 
      // (YES ONE WAS FOUND). 
      // HERE
//      formantSwitch[ peakFormantIndex ] = 0 ; // TURNING OFF SEGMENT PEAK FORMANT

      // SET FIRST LEG IN FRONT AND BACK SEGMENTS TO THE PEAK FORMANT.
      forwardLegOfSegmentIndices[ 0 ] = peakFormantIndex ;
      backwardLegOfSegmentIndices[ 0 ] = peakFormantIndex ;
      // HERE
      segmentLegCFaverageNow = formantCF[ peakFormantIndex ] ;

      // SET NUMBER OF LEG FORMANTS TO ONE, i.e. THE PEAK FORMANT, FOR RESPECTIVE FRONT OR BACK LEG.
      numberOfFrontLegFormants = 1 ; numberOfBackLegFormants = 1 ;

      // LEG CONSTRUCTION: TWO PASSES FOR FRONT AND BACK LEGS. 
      for( direction = 1; direction >= -1; direction += -2 ){
//fprintf( stderr, "\nBEGIN OF LEGS LOOP, DIRECTION: %d", direction ) ; 
         // (DIRECTION 1 = FRONT LEG, -1 = BACK LEG, CONSTRUCTED BACKWARDS.) 
         // FIND THE LEG SEGMENT INDICES
         // SET FLAG FOR LEG FORMANT SEARCH
         stillLookingForMoreSegmentFormants = 1 ;
         numberOfBridgingFramesNow = 0 ; 
	
         // SAVE GROUP INDEX OF PEAK FORMANT, WHICH IS SYNONYMOUS WITH THAT
         // NOW SAVED IN THE RESPECTIVE LEGS OF INDICES.
         if( direction == 1 ) groupIndexForSelectedFormant = formantGroup[ forwardLegOfSegmentIndices[ 0 ] ] ;
         else groupIndexForSelectedFormant = formantGroup[ backwardLegOfSegmentIndices[ 0 ] ] ;

         // THEN STEP INCREMENTALLY FORWARD OR BACKWARD TO THE GROUP USING THE SAVED GROUP INDEX, 
         // RELATIVE TO THE DIRECTION/LEG.
         previousGroupIndex = groupIndexForSelectedFormant ;
         groupIndexForNewFormant = groupIndexForSelectedFormant + direction ;  
         // IDENTIFY THE INDEX OF THE PREVIOUS FORMANT (WHICH IS ACTUALLY THE PEAK FORMANT), AS
         //  TAKEN FROM THE INITIALIZED LEG OF INDICES. 
         if( direction == 1 ) indexOfPreviousFormantInLeg = forwardLegOfSegmentIndices[ 0 ] ; 
         else indexOfPreviousFormantInLeg = backwardLegOfSegmentIndices[ 0 ] ;

         // WHILE LOOP FOR BUILDING THIS LEG SEGMENT BY LOOKING AT FORMANT GROUPS . . . 
	k = 0 ;  
        while( 
            (stillLookingForMoreSegmentFormants == 1) && 
            (groupIndexForNewFormant < numberOfGroups) &&    // HAVE NOT HIT END OF DATA FORMANT GROUPS
            (groupIndexForNewFormant >= 0)      // NOR BEGIN OF DATA
         ){



//fprintf( stderr, "\nBEGIN OF WHILE LOOP FOR BUILDING THIS LEG SEGMENT BY LOOKING AT FORMANT GROUPS . . ." ) ; 
//fprintf( stderr, "\nWHILE LOOP COUNT: %d\t numberOfBridgingFramesNow: %d", k, numberOfBridgingFramesNow ) ; 

            // THIS FORMANT GROUP EXISTS AND IS IN AN ADJACENT FRAME. 

            // SAVE THE FREQUENCY OF THE FORMANT TO WHICH WE ARE ATTEMPTING TO CONNECT, WHICH MAY BE
            // THE PEAK FORMANT OR THE LAST IN THE LEG. 


           // EXAMINE FORMANTS IN THIS GROUP; LOOKING FOR ONE THAT IS CLOSEST TO THE PREVIOUS ONE FOUND IN THIS LEG.
           // INITIALIZE THE ONE FOUND TO A NONE-FOUND-YET FLAG OF -1
           indexOfClosestFormantAbove = -1, indexOfClosestFormantBelow = -1,

            // *** MAKE TARGET CF *******
            // FIRST GET THE FRAME TIME FOR THIS GROUP
            formantGroupFrameTime = formantTime[ formantGroupBeginIndex[ groupIndexForNewFormant ] ] ;
              // THEN MAKE BY CALL TO ROUTINE

//fprintf( stderr, "\nBEFORE ROUTINE CALL, groupIndexForNewFormant: %d, formantGroupFrameTime: %f", groupIndexForNewFormant, 
//		formantGroupFrameTime ); 
  

           if( numberOfBridgingFramesNow == 0 ){
	     targetFormantCF = makeLeastSquaresTargetCF(
			direction,
			formantGroupFrameTime, // FORMANT GROUP FRAME TIME
			numberOfFrontLegFormants,
			numberOfBackLegFormants,
			formantCF,
			formantTime,
			forwardLegOfSegmentIndices,
			backwardLegOfSegmentIndices,
			scratchX,
			scratchY,
			maxLength
		) ;

          } ;
 
	    // *** TIME POINT FORMANTS GROUP EXAMINATION LOOP ******
            //  (LOOP FOR EXAMINING FORMANTS IN THIS GROUP, WHICH IS THE NEXT ADJACENT GROUP.)
            for( i = formantGroupBeginIndex[ groupIndexForNewFormant ]; 
               i <= formantGroupEndIndex[ groupIndexForNewFormant ]; i++ ){ 
//fprintf( stderr, "\n i: %d, formantSwitch[ i ]: %d", i, formantSwitch[ i ] ) ; 

//fprintf( stderr, "\nBEGIN OF LOOP FOR EXAMINING FORMANTS IN THIS GROUP, i: %d", i ) ; 
               // i = FORMANT INDEX OF FORMANT IN GROUP. 
//pd(1011) ;
               formantFrequencyDifference = formantCF[ i ] - formantCF[ indexOfPreviousFormantInLeg ] ;

/*
fprintf( stderr, "\nformantCF[ i ]: %f, formantCF[ indexOfPreviousFormantInLeg ]: %f, formantFrequencyDifference: %f ", formantCF[ i ], formantCF[ indexOfPreviousFormantInLeg ], formantFrequencyDifference ) ; 
*/

	      if( formantFrequencyDifference >= 0 ){
                  // THIS FORMANT IS ABOVE
                  if( indexOfClosestFormantAbove == -1 ){
                     // THIS IS FIRST; SAVE IT
                     indexOfClosestFormantAbove = i ; 
                  }else{
                     // NOT FIRST; TEST WHETHER IT IS CLOSER.
                     if( formantFrequencyDifference < 
                         ( formantCF[ indexOfClosestFormantAbove ] - formantCF[ indexOfPreviousFormantInLeg ] ) ){
                        // YES IT IS; SAVE IT.
                        indexOfClosestFormantAbove = i ; 
                     } ;
                  } ;
               }else{
                  // THIS FORMANT IS BELOW
                  if( indexOfClosestFormantBelow == -1 ){
                     // THIS IS FIRST; SAVE IT
                     indexOfClosestFormantBelow = i ; 
                  }else{
                     // NOT FIRST; TEST WHETHER IT IS CLOSER.
                     if( fabs( formantFrequencyDifference ) < 
                         fabs( ( formantCF[ indexOfClosestFormantBelow ] - 
                                    formantCF[ indexOfPreviousFormantInLeg ]) ) ){
                        // YES IT IS; SAVE IT.
                        indexOfClosestFormantBelow = i ; 
                     } ;
                  } ;
               } ;

            } ; // END OF SEARCH FOR CLOSET FORMANTS ABOVE OR BELOW.

/*
fprintf( stderr, "\n indexOfClosestFormantAbove: %d, indexOfClosestFormantBelow: %d", 
indexOfClosestFormantAbove, indexOfClosestFormantBelow ) ; 
fprintf( stderr, "\nTIME: %f FORMANT FREQ ABOVE: %f, FORMANT FREQ BELOW: %f, TARGET FREQ: %f", 
formantTime[ indexOfClosestFormantAbove ], formantCF[ indexOfClosestFormantAbove ], 
formantCF[ indexOfClosestFormantBelow ], targetFormantCF ) ; 
*/         
            // FIND THE ONE THAT IS CLOSEST TO THE THEORETICAL TARGET FREQUENCY.
            // FIRST SET proposedFormantIndex TO NOT FOUND.
            proposedFormantIndex = -1 ; 
            if( indexOfClosestFormantAbove != -1 ){
               // ABOVE FOUND; BELOW FOUND?
               if( indexOfClosestFormantBelow != -1 ){
                  // BOTH ABOVE AND BELOW FOUND; TEST FOR CLOSER.
                  if( fabs(formantCF[ indexOfClosestFormantAbove ] - targetFormantCF) < 
                       fabs(formantCF[ indexOfClosestFormantBelow ] - targetFormantCF) ){
                     // ABOVE IS CLOSER
                     proposedFormantIndex = indexOfClosestFormantAbove ;
                  } else {
                    // BELOW IS CLOSER
                     proposedFormantIndex = indexOfClosestFormantBelow ;
                  } ;
               }else{
                  // ONLY ABOVE; SAVE IT.
                  proposedFormantIndex = indexOfClosestFormantAbove ;
               } ;
            }else{
               // ABOVE NOT FOUND; BELOW FOUND?
               if( indexOfClosestFormantBelow != -1 ){
                  // ONLY BELOW FOUND. SAVE IT.
                  proposedFormantIndex = indexOfClosestFormantBelow ;
               }else{

               } ;          

            } ;


//            fprintf( stderr, "\nPROPOSED INDEX: %d SWITCH: %d ", 
//			proposedFormantIndex, formantSwitch[ proposedFormantIndex ] ) ;
             // WAS A FORMANT FOUND?
            if( proposedFormantIndex != -1 ){
                // FOUND; IF IT IS STILL ON (AVAILABLE), THEN ...
               if( formantSwitch[ proposedFormantIndex ] == 1 ){
                  // IS ON. 
                  // IF CHANGE IN TRAJECTORY SLOPE IS LESS THAN THRESHOLD, THEN SAVE IT.
                  //SAVE IN RESPECTIVE LEG AND TURN OFF.


                  frequencyChangePerMillisecond = findFrequencyChangePerMillisecond(
                     direction,
                     formantCF,
                     formantTime,
                     proposedFormantIndex,
                     forwardLegOfSegmentIndices,
                     numberOfFrontLegFormants,
                     backwardLegOfSegmentIndices,
                     numberOfBackLegFormants
                  ) ;  

                  dBchangePerMillisecond = find_dBchangePerMillisecond(
                     direction,
                     formantAmp,
                     formantTime,
                     proposedFormantIndex,
                     forwardLegOfSegmentIndices,
                     numberOfFrontLegFormants,
                     backwardLegOfSegmentIndices,
                     numberOfBackLegFormants
                  ) ;

 
//	     fprintf( stderr, "\nHERE frequencyChangePerMillisecond: %f", frequencyChangePerMillisecond ) ; 
//	     fprintf( stderr, "\nHERE dBchangePerMillisecond: %f", dBchangePerMillisecond ) ; 
 
                  
                  if( 
                       (
                          (frequencyChangePerMillisecond < MaximumFrequencyChangePerMillisecond) && 
			(frequencyChangePerMillisecond > (-1.0 * MaximumFrequencyChangePerMillisecond)) 
			&&
                          (dBchangePerMillisecond <= MaximumDecibelRisePerMillisecond) &&
                          (dBchangePerMillisecond >= (-1. * MaximumDecibelFallPerMillisecond)) 
                       ) 

/*
                       ||
                       (
                          ((numberOfFrontLegFormants + numberOfBackLegFormants - 1) == 1) &&
                          ( fabs(formantCF[ proposedFormantIndex ] - targetFormantCF) 
                            <= (1. * fundamental))
                       )
*/

                  ){
                      // MEETS THRESHOLDS. SAVE.
/*
                     fprintf( stderr, "\nSAVING INDEX: %d, FREQ: %f, FRAME: %d, TIME: %f ", 
			proposedFormantIndex, formantCF[ proposedFormantIndex ], 
			formantFrame[ proposedFormantIndex ], formantTime[ proposedFormantIndex ] ) ; 
*/
                     if( direction == 1 ){
                         forwardLegOfSegmentIndices[ numberOfFrontLegFormants ] = proposedFormantIndex ;
                         // HERE
                         for(k = 0; k < numberOfFrontLegFormants; k++ ) 
                              segmentLegCFaverageNow += forwardLegOfSegmentIndices[ k ] ;
                         segmentLegCFaverageNow /= (float) numberOfFrontLegFormants ;

                         // HERE
//                         formantSwitch[ proposedFormantIndex ] = 0 ; // ???
                         numberOfBridgeFramesToForwardLegIndex[ numberOfFrontLegFormants ] = numberOfBridgingFramesNow ; 
                         numberOfFrontLegFormants++ ; 
                     }else{
                        backwardLegOfSegmentIndices[ numberOfBackLegFormants ] = proposedFormantIndex ;
                         // HERE
                         for(k = 0; k < numberOfBackLegFormants; k++ ) 
                            segmentLegCFaverageNow += backwardLegOfSegmentIndices[ k ] ;
                         segmentLegCFaverageNow /= (float) numberOfBackLegFormants ;

                        // HERE
//                        formantSwitch[ proposedFormantIndex ] = 0 ; // ???
                        numberOfBridgeFramesToBackwardLegIndex[ numberOfBackLegFormants ] = numberOfBridgingFramesNow ; 
                        numberOfBackLegFormants++ ; 
                     } ;
 
                    // SAVE INDEX OF PREVIOUS FORMANT
                     indexOfPreviousFormantInLeg =  proposedFormantIndex ;
                     // INCREMENT TO NEXT FORMANT GROUP
                     previousGroupIndex = groupIndexForNewFormant ;                 
                     groupIndexForNewFormant += direction ;
                     

                     if( numberOfBridgingFramesNow > 0 ){ 
//                        fprintf( stderr, "\nBRIDGED ACROSS %d FRAME(S) TO CONNECT TO THIS FORMANT",
//                         numberOfBridgingFramesNow ) ; 
                        if( totalBridgesUsed == 0 ){
                           segmentBridgeLengthsMinMaxAvg[0] = numberOfBridgingFramesNow ;
                           segmentBridgeLengthsMinMaxAvg[1] = numberOfBridgingFramesNow ;
                        }else{
                           if( numberOfBridgingFramesNow < segmentBridgeLengthsMinMaxAvg[0] )
                              segmentBridgeLengthsMinMaxAvg[0] = numberOfBridgingFramesNow ;
                           if( numberOfBridgingFramesNow > segmentBridgeLengthsMinMaxAvg[1] )
                              segmentBridgeLengthsMinMaxAvg[1] = numberOfBridgingFramesNow ;
                        } ;
                        segmentBridgeLengthsMinMaxAvg[2] += numberOfBridgingFramesNow ;
                        totalBridgesUsed++ ;
                     } ;
                     numberOfBridgingFramesNow = 0 ; 


                  } else {
                     // NOT FOUND
                     numberOfBridgingFramesNow++ ;
                     if( numberOfBridgingFramesNow > maximumAllowedBridgingFrames ){
                        stillLookingForMoreSegmentFormants = 0 ; 
                     } ;
                     groupIndexForNewFormant += direction ;
//                     fprintf( stderr, "\nFREQUENCY CHANGE RATE ISSUE: NOT SAVING INDEX . . . " ) ;
                  } ;

               } else {
                  // PROPOSED FORMANT NOT ON (NOT AVAILABLE)
                  numberOfBridgingFramesNow++ ;
                  if( numberOfBridgingFramesNow > maximumAllowedBridgingFrames ){
                     stillLookingForMoreSegmentFormants = 0 ;
                  } ;
                  groupIndexForNewFormant += direction ;
//                  fprintf( stderr, "\nNOT AVAILABLE: NOT SAVING INDEX . . . " ) ;
               } ;

            } else {
               // NOT FOUND
               numberOfBridgingFramesNow++ ;
               if( numberOfBridgingFramesNow > maximumAllowedBridgingFrames ){
                  stillLookingForMoreSegmentFormants = 0 ;
               } ;
               groupIndexForNewFormant += direction ;
//                  fprintf( stderr, "\nNONE FOUND: NOT SAVING INDEX . . . " ) ;
            } ; // END OF "WAS A PROPOSED FORMANT FOUND" TEST
k = k + 1 ; 
/*
fprintf( stderr, "\nEND OF LOOP FOR EXAMINING FORMANTS" ) ; 
fprintf( stderr, "\n stillLookingForMoreSegmentFormants: %d, groupIndexForNewFormant: %d",
	 stillLookingForMoreSegmentFormants, groupIndexForNewFormant ) ; 
fprintf( stderr, "\n groupIndexForNewFormant: %d", groupIndexForNewFormant ) ; 
fprintf( stderr, "\n numberOfFrontLegFormants: %d", numberOfFrontLegFormants ) ; 
fprintf( stderr, "\n numberOfBackLegFormants: %d", numberOfBackLegFormants ) ;
fprintf( stderr, "\n formantGroupFrame[ groupIndexForNewFormant ]: %d",
formantGroupFrame[ groupIndexForNewFormant ] ) ; 
fprintf( stderr, "\n formantGroupFrame[ groupIndexForSelectedFormant: %d", formantGroupFrame[ groupIndexForSelectedFormant ] ) ; 
*/


         } ; // *** END OF LOOP FOR EXAMINING FORMANTS IN TIME GROUP


      }; // END OF WHILE LOOP FOR BUILDING LEG SEGMENT 		

      
//fprintf( stderr, "\nLEG SEGMENT BUILT" ) ; 
   } ; // END OF FRONT/BACK SEGMENTS LOOP

//fprintf( stderr, "\nSEGMENT BUILT" ) ;  
/*
fprintf( stderr, "\nFRONT LEG INDICES AND SWITCH: "); 
for(i = 0; i < numberOfFrontLegFormants; i++)fprintf( stderr, " %d %d", 
       forwardLegOfSegmentIndices[ i ], formantSwitch[ forwardLegOfSegmentIndices[ i ] ] ) ; 
fprintf( stderr, "\nBACK LEG INDICES AND SWITCH: "); 
for(i = 0; i < numberOfBackLegFormants; i++)fprintf( stderr, " %d %d", 
       backwardLegOfSegmentIndices[ i ], formantSwitch[ backwardLegOfSegmentIndices[ i ] ] ) ; 
*/
 
   // **** CONSTRUCT SEGMENT OUT OF FRONT AND BACK SEGMENTS

   segmentLength = numberOfBackLegFormants + numberOfFrontLegFormants - 1 ;
   segmentDuration = formantTime[ forwardLegOfSegmentIndices[ numberOfFrontLegFormants - 1 ] ] - 
        formantTime[ backwardLegOfSegmentIndices[ numberOfBackLegFormants - 1 ] ] ;

/*
fprintf( stderr, "\nMIN DUR: %f, MAX DUR: %f, PROSPECTIVE SEGMENT DURATION: %f, LENGTH: %d\n", 
    minimumSegmentDuration, maximumSegmentDuration , segmentDuration, segmentLength ) ; 
   fprintf( stderr, "\nSEGMENT FOUND: %d", numberOfSegmentsFoundBeforeSelection ) ; 
*/
   numberOfSegmentsFoundBeforeSelection++ ;

   // TEST TO WRITE OR NOT WRITE OUT SEGMENT.

   if(1 == 1)
   {
      // ACCEPT SEGMENT
      numberOfSegmentsFoundAfterSelection++ ;
      // TURN OFF ALL FORMANTS IN THE ACCEPTED SEGMENT EXCEPT THE FIRST AND LAST, AS
      // THESE CAN BE LINKED TO BY OTHER SEGMENTS.
      for( i = 1 ; i < segmentLength - 1; i++ ) formantSwitch[ thisSegment[ i ] ] = 0 ; 
//      for( i = 0 ; i < segmentLength; i++ ) formantSwitch[ thisSegment[ i ] ] = 0 ; 


      if( add_onset_and_release_points__no_0__append_1__impose_2 == 1 ) 
              segmentDuration += ( onset_duration + release_duration ) ;

      if( numberOfSegmentsFoundAfterSelection == 1 ){
         segmentLengthMinMaxAvg[ 0 ] = segmentLength ;
         segmentLengthMinMaxAvg[ 1 ] = segmentLength ;
         segmentLengthMinMaxAvg[ 2 ] = segmentLength ;
      }else{
         if( segmentLength < segmentLengthMinMaxAvg[ 0 ] ) segmentLengthMinMaxAvg[ 0 ] = segmentLength ;
         if( segmentLength > segmentLengthMinMaxAvg[ 1 ] ) segmentLengthMinMaxAvg[ 1 ] = segmentLength ;
         segmentLengthMinMaxAvg[ 2 ] += segmentLength ;
      } ;

      // ZERO OUT
      for( i = 0 ; i < segmentLength; i++ ) thisSegmentNumberOfBridgeFrames[ i ] = 0 ; 

      // CONCATENATE REVERSE-OF-BACK-SEGMENT AND FRONT-SEGMENT-MINUS-FIRST-ENTRY INTO ARRAY.
      for( i = 0, k = (numberOfBackLegFormants - 1); i < numberOfBackLegFormants; i++, k-- ){ 
         thisSegment[ i ] = backwardLegOfSegmentIndices[ k ] ;
         if( i == 0 ) thisSegmentNumberOfBridgeFrames[ i ] = 0 ; 
         else thisSegmentNumberOfBridgeFrames[ i ] = numberOfBridgeFramesToBackwardLegIndex[k - 1] ;
      } ;
      for( i = numberOfBackLegFormants, k = 1; k < numberOfFrontLegFormants; i++, k++ ){
          thisSegment[ i ] = forwardLegOfSegmentIndices[ k ] ;
          thisSegmentNumberOfBridgeFrames[ i ] = numberOfBridgeFramesToBackwardLegIndex[ k ] ;
      } ; 

      // RAMP UP BEGIN AND END IF SPECIFIED.
      for(i = 0; i < segmentLength; i++ ) thisSegmentEnvelope[ i ] = 1.0 ; 
      if( add_onset_and_release_points__no_0__append_1__impose_2 == 2 ){
         if( segmentLength <= 2 ){
            for(i = 0; i < segmentLength; i++ ) thisSegmentEnvelope[ i ] = 0. ;
         } else {
            // ONSET
            temp = (onset_duration > (segmentDuration * 0.5)) ? segmentDuration * 0.5 : onset_duration ;
            i = 0 ; 
            while( (formantTime[ thisSegment[ i ] ] - formantTime[ thisSegment[ 0 ] ]) < temp ){
               thisSegmentEnvelope[ i ] *= 
                     ( (formantTime[ thisSegment[ i ] ] - formantTime[ thisSegment[ 0 ] ]) / temp) ; 
               i++ ;              
            } ;
            // RELEASE
            temp = (release_duration > (segmentDuration * 0.5)) ? segmentDuration * 0.5 : release_duration ;
            i = segmentLength - 1 ; 
            while( (formantTime[ thisSegment[ segmentLength - 1 ] ] - formantTime[ thisSegment[ i ] ]) < temp ){
               thisSegmentEnvelope[ i ] *= 
                     ( (formantTime[ thisSegment[ segmentLength - 1 ] ] - formantTime[ thisSegment[ i ] ]) / temp) ; 
               i++ ;                 
            } ;
         } ;

      } ;


        // WRITE OUT SEGMENT FREQUENCIES TO PLOT FILE
//	fprintf( stderr, "********\n" ) ; 


      // CALCULATE OUTPUT SEGMENT LENGTH (WITH ADDED ONSET, BRIDGE, AND RELEASE) FOR BINARY WRITE.
      outputSegmentLength = 0 ;  

	// WRITE SEPARATE POINT IN PLOT FILE FOR PLOTTING SEGMENT PEAK AMP POINT. 

      thisSegmentTime = timeShift + formantTime[ forwardLegOfSegmentIndices[0] ] ;
      thisCF = formantCF[ forwardLegOfSegmentIndices[0] ] ;
      thisAmp = formantAmp[ forwardLegOfSegmentIndices[0] ] ;
      thisdB = (thisAmp < dB_to_amp( -96. )) ? -96. : amp_to_dB( thisAmp ) ;   
      thisBW = formantBW[ forwardLegOfSegmentIndices[0] ] ; 
      thisQ = formantQ[ forwardLegOfSegmentIndices[0] ] ; 
 
      sprintf( tempstring, "%f %f %f %f %f %f\n\n", thisSegmentTime, thisCF, thisAmp, thisdB, thisBW, thisQ ) ;
      
      fprintf( ASCIIfrequencySegmentsPlotPeakPoints, "%s", tempstring ) ; 



      // ****** ONSET **********
      // SAVE FILE POSITION AND THEN WRITE BINARY FILE PLACEHOLDER FOR LENGTH OF NEXT SEGMENT.
      segmentLengthFilePosition = ftell( intermediaryBinaryFrequencySegmentsFile ) ; 
      fwrite( &outputSegmentLength, sizeof(int), 1, intermediaryBinaryFrequencySegmentsFile ) ;

      if( (add_onset_and_release_points__no_0__append_1__impose_2 == 1) && 
		(onset_duration > 0.0)  ){
            

         numberOfOnsetFrames = (int)(0.5 + (onset_duration * (float) frames_per_sec)) ;

         if( numberOfOnsetFrames > 0 ){
//               fprintf( stderr, "\nADDING ONSET OF %d FRAMES", numberOfOnsetFrames ) ; 
            for(i = 0, j = numberOfOnsetFrames; i < numberOfOnsetFrames; i++, j-- ){

               thisSegmentTime = 
                     timeShift + formantTime[ thisSegment[ 0 ] ] - ( (float) j * frameDuration ) ;
/*
               temp = formantTime[ thisSegment[ 0 ] ] - ( (float) j * frameDuration ) ;
               thisCF = linearLeastSquaresProjection(
                  temp, 
                  scratchX,  
                  scratchY, 
                  thisLength
               ) ;
*/
//             fprintf( stderr, "\nTIME: %f\tthisCF: %f, LENGTH: %d", temp, thisCF, thisLength ) ; 

               thisCF = formantCF[ thisSegment[ 0 ] ] ;
/*
               temp = -96. - dB_to_amp( formantAmp[ thisSegment[ segmentLength - 1 ] ] ); 
               thisAmp = formantAmp[ thisSegment[ 0 ] ] * 
		dB_to_amp( temp * (1.0 - ((float) i / (float) numberOfOnsetFrames) ) ) ;
*/
               thisAmp = formantAmp[ thisSegment[ 0 ] ] * ( (float) i / (float) numberOfOnsetFrames) ;

               thisdB = (thisAmp < dB_to_amp( -96. )) ? -96. : amp_to_dB( thisAmp ) ;   
               thisBW = formantBW[ thisSegment[ 0 ] ] ; 
               thisQ = formantQ[ thisSegment[ 0 ] ] ; 
 
	       sprintf( tempstring, "%f %f %f %f %f %f\n", 
                  thisSegmentTime, thisCF, thisAmp, thisdB, thisBW, thisQ ) ;
               fprintf( intermediaryASCIIfrequencySegmentsPlot, "%s", tempstring ) ; 

               fwrite( &thisSegmentTime, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
               fwrite( &thisCF, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;                  
               fwrite( &thisAmp, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;                  
               fwrite( &thisBW, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
               fwrite( &thisQ, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
               outputSegmentLength++ ; 

               previousSegmentTime = thisSegmentTime ;
               previousCF =  thisCF ;
               previousAmp = thisAmp ;
               previousBW = thisBW ;
               previousQ = thisQ ;
            } ;

         } ;
      } ; 

      // ****** CENTRAL SEGMENT **********
//      fprintf( stderr, "\nACCEPT" ) ;
//      fprintf( stderr, "\nSWITCHES:" ) ; 
 
//      fprintf( stderr, "\nthisSegmentEnvelope: " ) ; 
//      for(i = 0; i < segmentLength; i++ ) fprintf( stderr, "%f ", thisSegmentEnvelope[ i ] ); 
//      fprintf( stderr, "\n" ) ; 
    
      for(i = 0; i < segmentLength; i++ ){
  //       fprintf( stderr, " %d %d", thisSegment[ i ], formantSwitch[ thisSegment[ i ] ] ); 

         if( thisSegmentNumberOfBridgeFrames[ i ] > 0 ){
            // INTERPOLATE OVER LONGER BRIDGE SPAN.
//            fprintf( stderr, "\nADDING BRIDGE OF %d FRAMES", thisSegmentNumberOfBridgeFrames[ i ] ) ;
            for( j = 1; j < (thisSegmentNumberOfBridgeFrames[ i ] - 1); j++ ){
               bridgeProportion = (float) j / (float) thisSegmentNumberOfBridgeFrames[ i ] ;

               thisSegmentTime = timeShift + previousSegmentTime + (bridgeProportion * frameDuration) ; 
               thisCF = previousCF  + (bridgeProportion * (formantCF[ thisSegment[ i ] ] - previousCF)) ;
               thisAmp = ( previousAmp  + (bridgeProportion * (formantAmp[ thisSegment[ i ] ] - previousAmp)))  ;
               thisdB = (thisAmp < dB_to_amp( -96. )) ? -96. : amp_to_dB( thisAmp ) ; 
               thisBW = previousBW  + (bridgeProportion * (formantBW[ thisSegment[ i ] ] - previousBW)) ; 
               thisQ = previousQ  + (bridgeProportion * (formantQ[ thisSegment[ i ] ] - previousQ)) ; 


	      sprintf( tempstring, "%f %f %f %f %f %f\n", thisSegmentTime, thisCF, thisAmp * thisSegmentEnvelope[ i ],
		 thisdB, thisBW, thisQ ) ;
               fprintf( intermediaryASCIIfrequencySegmentsPlot, "%s", tempstring ) ; 

               fwrite( &thisSegmentTime, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
               fwrite( &thisCF, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;                  
               fwrite( &thisAmp, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;                  
               fwrite( &thisBW, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
               fwrite( &thisQ, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
               outputSegmentLength++ ;
            } ;
        } ;

        thisSegmentTime = timeShift + formantTime[ thisSegment[ i ] ] ; 
        thisCF = formantCF[ thisSegment[ i ] ] ;
        thisAmp = formantAmp[ thisSegment[ i ] ] ; 
        thisdB = (thisAmp < dB_to_amp( -96. )) ? -96. : amp_to_dB( thisAmp ) ; 
        thisBW = formantBW[ thisSegment[ i ] ] ; 
        thisQ = formantQ[ thisSegment[ i ] ] ; 

        sprintf( tempstring, "%f %f %f %f %f %f\n", thisSegmentTime, thisCF, thisAmp * thisSegmentEnvelope[ i ], 
           thisdB, thisBW, thisQ ) ;
        fprintf( intermediaryASCIIfrequencySegmentsPlot, "%s", tempstring ) ; 

        fwrite( &thisSegmentTime, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
        fwrite( &thisCF, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;         
        fwrite( &thisAmp, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;         
        fwrite( &thisBW, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
        fwrite( &thisQ, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
        outputSegmentLength++ ;

        previousSegmentTime = thisSegmentTime ;
      }; 
//      fprintf( stderr, "\n" ) ; 

      // ****** RELEASE **********
      if( (add_onset_and_release_points__no_0__append_1__impose_2 == 1) && 
		(release_duration > 0.0) )
      {
            // ADD DECAY RAMP FROM 0.
         numberOfReleaseFrames = (int)(0.5 + (release_duration * (float) frames_per_sec)) ;
         if( numberOfReleaseFrames > 0 ){
//            fprintf( stderr, "\nADDING RELEASE OF %d FRAMES", numberOfReleaseFrames ) ;
            for(l = 0, j = 1, i = (numberOfReleaseFrames - 1); l < numberOfReleaseFrames; j++, i--, l++ ){

              thisSegmentTime = timeShift + 
                 formantTime[ thisSegment[segmentLength - 1 ] ] + (frameDuration * (float) j ) ; 
/*
               thisCF = linearLeastSquaresProjection(
                  formantTime[ thisSegment[segmentLength - 1 ] ] + (frameDuration * (float) j ), 
                  scratchX,  
                  scratchY, 
                  thisLength
               ) ;
*/
              thisCF = formantCF[ thisSegment[segmentLength - 1 ] ] ;
/*
              temp = -96. - dB_to_amp( formantAmp[ thisSegment[ segmentLength - 1 ] ] ); 
              thisAmp = formantAmp[ thisSegment[ segmentLength - 1 ] ] * 
			dB_to_amp( temp * (1. - ( (float) i / (float) numberOfReleaseFrames ) ) )  ; 
*/
              thisAmp = formantAmp[ thisSegment[ segmentLength - 1 ] ] * 
			( (float) i / (float) numberOfReleaseFrames )  ; 


              thisdB = (thisAmp < dB_to_amp( -96. )) ? -96. : amp_to_dB( thisAmp ) ; 
              thisBW = formantBW[ thisSegment[ segmentLength - 1 ] ] ; 
              thisQ = formantQ[ thisSegment[ segmentLength - 1 ] ] ; 

	     sprintf( tempstring, "%f %f %f %f %f %f\n", thisSegmentTime, thisCF, thisAmp, thisdB, thisBW, thisQ ) ;
              fprintf( intermediaryASCIIfrequencySegmentsPlot, "%s", tempstring ) ; 

              fwrite( &thisSegmentTime, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
              fwrite( &thisCF, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;         
              fwrite( &thisAmp, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;         
              fwrite( &thisBW, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
              fwrite( &thisQ, sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
              outputSegmentLength++ ; 
           } ;
        } ;
      } ; 

//    fprintf( stderr, "\numberofreleaseframes LENGTH: %d", outputSegmentLength ) ;  
      totalFormantsInAllSegments += outputSegmentLength ;

 
      fprintf( intermediaryASCIIfrequencySegmentsPlot, "\n" ) ;

      // SAVE THIS FILE POSITION
      filePositionNow = ftell( intermediaryBinaryFrequencySegmentsFile ) ; 
//	fprintf( stderr, "\n filePositionNow: %ld", filePositionNow ); 
      // GO BACK AND WRITE THE NUMBER OF SEGMENTS
      fseek( intermediaryBinaryFrequencySegmentsFile, segmentLengthFilePosition, 0 ) ;
      fwrite( &outputSegmentLength, sizeof(int), 1, intermediaryBinaryFrequencySegmentsFile ) ;
      // RETURN TO THE NEXT POSITION
      fseek( intermediaryBinaryFrequencySegmentsFile, filePositionNow, 0 ) ;



/*
      for( i = (numberOfBackLegFormants - 1); i >= 0; i-- ){
          fprintf( stderr, "\nFORMANT GROUP INDEX: %d INDEX: %d TIME: %f FREQ: %d AMP: %f DB: %d Q: %d", 
		formantGroup[ backwardLegOfSegmentIndices[ i ] ], 
                backwardLegOfSegmentIndices[ i ], formantTime[ backwardLegOfSegmentIndices[ i ] ],
		(int) formantCF[ backwardLegOfSegmentIndices[ i ] ], 
			formantAmp[ backwardLegOfSegmentIndices[ i ] ], 
			(int) amp_to_dB( formantAmp[ backwardLegOfSegmentIndices[ i ] ] ), 
				(int) formantQ[ backwardLegOfSegmentIndices[ i ] ] 
         ) ; 

      } ; 


      fprintf( stderr, 
           "\n------\nFORMANT GROUP INDEX: %d INDEX: %d TIME: %f FREQ: %d AMP: %f DB: %d Q: %d <-- PEAK\n------", 
		formantGroup[ peakFormantIndex ], peakFormantIndex, formantTime[ peakFormantIndex ], 
			(int) formantCF[ peakFormantIndex ], 
			formantAmp[ peakFormantIndex ],
			(int) amp_to_dB( formantAmp[ peakFormantIndex ] ), (int) formantQ[ peakFormantIndex ] 
      ) ; 
      for( i = 0; i < numberOfFrontLegFormants; i++ ){
         fprintf( stderr, "\nFORMANT GROUP INDEX: %d INDEX: %d TIME: %f FREQ: %d AMP: %f DB: %d Q: %d", 
		formantGroup[ forwardLegOfSegmentIndices[ i ] ], forwardLegOfSegmentIndices[ i ],  
		formantTime[ forwardLegOfSegmentIndices[ i ] ],
		(int) formantCF[ forwardLegOfSegmentIndices[ i ] ], 
			formantAmp[ forwardLegOfSegmentIndices[ i ] ],
			(int) amp_to_dB( formantAmp[ forwardLegOfSegmentIndices[ i ] ] ),
				 (int) formantQ[ forwardLegOfSegmentIndices[ i ] ] 
         ) ; 
      } ; 
*/


   }else{
//      fprintf( stderr, "\nREJECT" ) ; 
      // WRITE OUT SEGMENT AS REJECT POINTS
      k = 0; 
      for(i = 0; i < numberOfFrontLegFormants; i++){
         thisSegment[ k ] = forwardLegOfSegmentIndices[ i ] ; k++ ;
      } ;
      for(i = 1; i < numberOfBackLegFormants; i++){
         thisSegment[ k ] = backwardLegOfSegmentIndices[ i ] ; k++ ;
      } ;

//      fprintf( stderr, "\nSWITCHES:" ) ; 
      for(i = 0; i < k; i++ ){
//         fprintf( stderr, " %d %d", thisSegment[ i ], formantSwitch[ thisSegment[ i ] ] ); 

        thisSegmentTime = timeShift + formantTime[ thisSegment[ i ] ] ; 
        thisCF = formantCF[ thisSegment[ i ] ] ;
        thisAmp = formantAmp[ thisSegment[ i ] ] ; 
        thisdB = (thisAmp < dB_to_amp( -96. )) ? -96. : amp_to_dB( thisAmp ) ; 
        thisBW = formantBW[ thisSegment[ i ] ] ; 
        thisQ = formantQ[ thisSegment[ i ] ] ; 

        sprintf( tempstring, "%f %f %f %f %f %f\n\n", thisSegmentTime, thisCF, thisAmp, thisdB, thisBW, thisQ ) ;
        fprintf( ASCIIfrequencySegmentsPlotRejects, "%s", tempstring ) ; 
 
      }; 
//      fprintf( stderr, "\n" ) ; 
   } ; // END OF TEST FOR A WRITEABLE SEGMENT


   // ???

}; 


// GO TO BEGIN OF BINARY FILE AND WRITE THE NUMBER OF SEGMENTS
fseek( intermediaryBinaryFrequencySegmentsFile, 0, 0 ) ; 
fwrite( &numberOfSegmentsFoundAfterSelection, sizeof(int), 1, intermediaryBinaryFrequencySegmentsFile ) ;

fclose( intermediaryBinaryFrequencySegmentsFile ) ;

fclose( intermediaryASCIIfrequencySegmentsPlot ) ; 
fclose( ASCIIfrequencySegmentsPlotPeakPoints ) ; 
fclose( ASCIIfrequencySegmentsPlotRejects ) ; 

// **** FREE MEMORY

free( Wanal ) ;
free( Wsyn ) ;
free( input ) ;
free( Hwin ) ;
free( winput ) ;
free( buffer ) ;
free( channel ) ;
free( previous_channel ) ;
free( output ) ;

free( AmplitudeSpectrum ) ;

free( OutputSpectrum ) ; 

free( HammingWindow ) ;

free( amps ) ; 
free( freqs ) ; 
free( newAmps ) ; 
free( AmpsDerivative ) ; 
free( testAmpsSave ) ; 
free( freqStasis ) ; 
free( triWindow ) ;
free( winArray ) ; 
free( symmetryFactor ) ; 
free( v ) ; 
free( w ) ; 
free( formantAmpsCopy ) ; 
free( tempList ) ;
free( peakAmps ) ; 
free( avgAmps ) ; 
free( diffdBs ) ; 

free( formantCenterFreqs ) ; free( formantAmps ) ; 
free( formantBWs ) ; free( formantQs ) ; 
free( formantIndices ) ; 
free( formantLowStopBandIndices ) ; free( formantHighStopBandIndices ) ; 

free( formantFrame ) ; 
free( formantTime ) ; 
free( formantCF ) ; 
free( formantAmp ) ; 
free( formantBW ) ; 
free( formantQ ) ; 
free( formantGroup ) ; 
free( formantSwitch ) ; 
free( formantGroupFrame ) ; 
free( formantGroupBeginIndex ) ; 
free( formantGroupEndIndex ) ; 
free( forwardLegOfSegmentIndices ) ; 
free( backwardLegOfSegmentIndices ) ; 
free( numberOfBridgeFramesToForwardLegIndex ) ; 
free( numberOfBridgeFramesToBackwardLegIndex ) ; 
free( thisSegment ) ; 
free( thisSegmentNumberOfBridgeFrames ) ; 



// **** END MEMORY FREE


intermediaryBinaryFrequencySegmentsFile = fopen( "/tmp/intermediaryBinaryFrequencySegmentsFile", "r" ) ; 
// READ IN NUMBER OF SEGMENTS
fread( &numberOfSegments, sizeof(int), 1, intermediaryBinaryFrequencySegmentsFile ) ;
//fprintf( stderr, "\nNUMBER OF SEGMENTS: %d", numberOfSegments ) ; 



fvec( formantTime, totalFormantsInAllSegments ) ; 
fvec( formantCF, totalFormantsInAllSegments ) ; 
fvec( formantAmp, totalFormantsInAllSegments ) ; 
fvec( formantBW, totalFormantsInAllSegments ) ; 
fvec( formantQ, totalFormantsInAllSegments ) ; 
ivec( formantSegmentNumber, totalFormantsInAllSegments ) ; 


ivec( segmentLengths, numberOfSegments ) ; 
ivec( segmentBeginDataIndices, numberOfSegments ) ; 
ivec( segmentEndDataIndices, numberOfSegments ) ; 
ivec( segmentSwitches,  numberOfSegments ) ;
ivec( linkToSegment, numberOfSegments ) ; 
ivec( linkFromSegment, numberOfSegments ) ; 

fvec( segmentBeginTimes, numberOfSegments ) ; 
fvec( segmentEndTimes, numberOfSegments ) ; 

fvec( segmentPeakAmps, numberOfSegments ) ;

for(i = 0; i < numberOfSegments; i++){
	segmentSwitches[i] = 1 ;
	linkToSegment[i] = -1 ; 
	linkFromSegment[i] = -1 ; 
} ;

formantIndex = 0 ;

for(i = 0; i < numberOfSegments; i++){
   fread( &segmentLength, sizeof(int), 1, intermediaryBinaryFrequencySegmentsFile ) ;
   segmentLengths[ i ] = segmentLength ;


   segmentBeginDataIndices[ i ] = formantIndex ;
   for(k = 0; k < segmentLength; k++){
      fread( &formantTime[ formantIndex ], sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
      fread( &formantCF[ formantIndex ], sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
      fread( &formantAmp[ formantIndex ], sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
      fread( &formantBW[ formantIndex ], sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;
      fread( &formantQ[ formantIndex ], sizeof(float), 1, intermediaryBinaryFrequencySegmentsFile ) ;

      formantSegmentNumber[ formantIndex ] = i ; 

      if( k == 0 ) peakAmp = formantAmp[ formantIndex ] ;  
      else if( formantAmp[ formantIndex ] > peakAmp ) peakAmp = formantAmp[ formantIndex ] ;   


      formantIndex++ ;
   } ;
   segmentEndDataIndices[ i ] = formantIndex - 1 ; 

   segmentBeginTimes[ i ] = formantTime[ segmentBeginDataIndices[ i ] ] ;
   segmentEndTimes[ i ] = formantTime[ segmentEndDataIndices[ i ] ] ;

   segmentPeakAmps[ i ] =  peakAmp ;


//   fprintf( stderr, "\n formantTime[ segmentBeginDataIndices[ i ] ]: %f", 
//	formantTime[ segmentBeginDataIndices[ i ] ] ) ; 


};

fprintf( stderr, "\nTOTAL PRE-LINKED SEGMENTS FOUND: %d", numberOfSegments ); 
fprintf( stderr, "\nSEARCHING FOR LINKABLE SEGMENTS" ) ; 
//prline( 100,   "-" ) ;
fprintf( stderr, "\n" ); 

printRateModulus =  numberOfSegments / 100 ;
                                                                                                            
// EXAMINE EVERY SEGMENT FOR POSSIBLE LINKING
k = printRateModulus ; 

for(i = 0; i < numberOfSegments; i++){
   if( segmentLengths[ i ] < minLength ){
      // TURN OFF SEGMENT AS TOO SHORT
      linkToSegment[ i ] = -2 ; linkFromSegment[ i ] = -2 ; 
   } ;
} ;

thisSeg = 0 ;
while( thisSeg != -1 ){
   // FIND THE SEGMENT WITH THE LONGEST LENGTH 
   // THAT IS STILL LINKABLE ON EITHER END.
   thisSeg = -1 ; // -1 MEANS NONE FOUND

   segmentsReviewed = 0 ; 
   // GO THROUGH ALL THE SEGMENTS . . . 
   for(i = 0; i < numberOfSegments; i++){
      if( (linkFromSegment[ i ] == -1) || (linkToSegment[ i ] == -1) ){
         segmentsReviewed ++ ;
         // SEGMENT HAS VIABLE LINK ENDS (i.e. NOT -2 (OFF) OR GREATER THAN -1 (LINK) ) AND COULD BE LINKED.
         if( thisSeg == -1 ){
            // FIRST ONE FOUND
            thisSeg = i ;
         }else{

            if( 
//                segmentPeakAmps[ i ] > segmentPeakAmps[ thisSeg ] 
                segmentLengths[ i ] > segmentLengths[ thisSeg ] 

            ){
               thisSeg = i ;
            } ;
         } ;
      } ;
   } ;


//  fprintf( stderr, "\n thisSeg: %d", thisSeg ); 

   if( k == 0 ){
      fprintf( stderr, "*" ) ; 
      k = printRateModulus ;
      temp = (100.0 * (float) segmentsReviewed / (float) numberOfSegments) ;
      if( fmodf( temp, 10.0 ) == 0.0 ) fprintf( stderr, " %d%%", (int) temp ) ; 

   } ; 
   k-- ; 

   if( 
      // IF A LINKABLE SEGMENT WAS FOUND . . . 
      thisSeg != -1
   ){

      // FIND AN END TO LINK
      // EXAMINE ALL SEGMENTS LOOKING FOR A LINKABLE ONE.
      closestSegment = -1 ; 
      // LOOP FOR SEGMENTS INTERROGATION.
      for(j = 0; j < numberOfSegments; j++ ){

         // NON DIRECTION-CONNECTED TESTS
         if( linkToSegment[ thisSeg ] == -1 ){
            // FORWARD LINKING
            absoluteFreqDiff = fabs(formantCF[ segmentEndDataIndices[ thisSeg ] ] -
               formantCF[ segmentBeginDataIndices[ j ] ]) ;
            if( 
               // NOT THE SAME
               (j != i) && 
                 // HAVE THE MINIMUM LENGTH
               (segmentLengths[ j ] >= minLength) &&  
                 // HAVE A BEGIN TIME AFTER END OF LINK-FROM LINK SEG
               (segmentBeginTimes[ j ] >= segmentEndTimes[ thisSeg ]) && 
                 // BEGIN BEFORE THE END OF THE LINKAGE TIME WINDOW
               (segmentBeginTimes[ j ] <= (segmentEndTimes[ thisSeg ] + linkageTime)) &&
                 // IS WITHIN FREQ CHANGE THRESHOLD
               (absoluteFreqDiff <= maximumFrequencyLinkage)
            ){
               // FIND DISTANCE FROM LINK-FROM TO LINK-TO SEGMENTS; SCALE THE FREQ DIFFERENCE BY THE 
               distance = sqrt( 
                  pow((linkageTime / maximumFrequencyLinkage) * 
                  (formantCF[ segmentBeginDataIndices[ j ] ] - formantCF[ segmentEndDataIndices[ thisSeg ] ]), 2. )
                  +
                  pow(formantTime[ segmentBeginDataIndices[ j ] ] - formantTime[ segmentEndDataIndices[ thisSeg ] ], 2. )
               ) ;
               if( closestSegment == -1 ){
                  closestSegment = j; previousDistance = distance ;
               }else{
                  if( distance < previousDistance ){
                     closestSegment = j ; previousDistance = distance ;
                  } ;
               } ;
            } ;

         }else{
            // BACKWARD LINKING
            absoluteFreqDiff = fabs(formantCF[ segmentBeginDataIndices[ thisSeg ] ] - 
               formantCF[ segmentEndDataIndices[ j ] ]) ;          
            if( 
               // NOT THE SAME
               (j != i) && 
                 // HAVE THE MINIMUM LENGTH
               (segmentLengths[ j ] >= minLength) &&  
               (segmentEndTimes[ j ] <= segmentBeginTimes[ thisSeg ]) && 
               (segmentBeginTimes[ j ] >= (segmentBeginTimes[ thisSeg ] - linkageTime)) &&
                 // IS WITHIN FREQ CHANGE THRESHOLD
               (absoluteFreqDiff <= maximumFrequencyLinkage)
            ){
               // FIND DISTANCE FROM LINK-FROM TO LINK-TO SEGMENTS; SCALE THE FREQ DIFFERENCE BY THE 
               distance = sqrt( 
                  pow((linkageTime / maximumFrequencyLinkage) * 
                  (formantCF[ segmentEndDataIndices[ j ] ] - formantCF[ segmentBeginDataIndices[ thisSeg ] ]), 2. )
                  +
                  pow(formantTime[ segmentEndDataIndices[ j ] ] - formantTime[ segmentBeginDataIndices[ thisSeg ] ], 2. )
               ) ;
               if( closestSegment == -1 ){
                  closestSegment = j; previousDistance = distance ;
               }else{
                  if( distance < previousDistance ){
                     closestSegment = j ; previousDistance = distance ;
                  } ;
               } ;
            } ;
         } ;
      } ;  // END OF LOOP FOR LINK TO SEGMENTS INTERROGATION.

      if( closestSegment != -1 ){
         // LINKABLE SEGMENT WAS FOUND; LINK IT! 
            if( linkToSegment[ thisSeg ] == -1 ){
               linkToSegment[ thisSeg ] = closestSegment ;  linkFromSegment[ closestSegment ] = thisSeg ; 
            }else{
               linkToSegment[ closestSegment ] = thisSeg ;  linkFromSegment[ thisSeg ] = closestSegment ; 
            } ;
      }else{
         // MARK WITH -2 THE END OF THIS SEGMENT AS TESTED AND UN-LINKABLE.
         if( linkToSegment[ thisSeg ] == -1 ) linkToSegment[ thisSeg ] = -2 ; 
         else linkFromSegment[ thisSeg ] = -2 ; 
      } ;
   } ;
} ; // END OF LOOP FOR LOOKING AT SEGMENTS TO LINK. 

// COUNT LINKS
for(i = 0; i < numberOfSegments; i++ ){
   if( linkToSegment[ i ] >= 0 )totalLinks++ ;
} ; 


fprintf( stderr, "\n\nTOTAL LINKS: %d", totalLinks ) ; 

fvec( thisSegTime, numberOfFrames ) ; 
fvec( thisSegCF, numberOfFrames ) ; 
fvec( thisSegAmp, numberOfFrames ) ; 
fvec( thisSegdB, numberOfFrames ) ; 
fvec( thisSegBW, numberOfFrames ) ; 
fvec( thisSegQ, numberOfFrames ) ; 


// BUILD AND WRITE OUT SEGMENTS 
ivec( segmentWriteFlag, numberOfSegments ) ; 

// SET WRITE FLAG TO NOT WRITTEN.
for( i = 0; i < numberOfSegments; i++ ) segmentWriteFlag[ i ] = -1 ; 

// LOOK AT EVERY SEGMENT
for( i = 0; i < numberOfSegments; i++ ){
   if( segmentWriteFlag[ i ] == -1) {
      // THIS SEGMENT HAS NOT BEEN WRITTEN.
      thisSegLength = 0 ; 
      thisSeg = i ;
//      fprintf( stderr, "\nSEGMENTS: " ) ; 
      while( thisSeg > -1 ){
//         fprintf( stderr, " %d", thisSeg ) ;
         // NOT YET WRITTEN
         // WRITE IT OUT
         for(k = segmentBeginDataIndices[ thisSeg ]; k <= segmentEndDataIndices[ thisSeg ]; k++){
            thisSegTime[ thisSegLength ] = timeShift + formantTime[ k ] ; 
            thisSegCF[ thisSegLength ] = formantCF[ k ] ;
            thisSegAmp[ thisSegLength ] = formantAmp[ k ] ; 
//fprintf( stderr, "\nV k: %d  thisSegAmp[ j ]: %f", k, thisSegAmp[ j ] ) ; 

            thisSegdB[ thisSegLength ] = 
                 (thisSegAmp[ thisSegLength ] < dB_to_amp( -96. )) ? -96. : 
                        amp_to_dB( thisSegAmp[ thisSegLength ] ) ; 
            thisSegBW[ thisSegLength ] = formantBW[ k ] ; 
            thisSegQ[ thisSegLength ] = formantQ[ k ] ; 
            thisSegLength++ ;

         } ;
         segmentWriteFlag[ thisSeg ] = 0 ; 
         // DOES IT LINK?
         if( linkToSegment[ thisSeg ]  > -1 ){
            // YES. WRITE OUT LINK VALUES
            nextSeg = linkToSegment[ thisSeg ] ;
            numberOfSteps = 
             (int)(( formantTime[ segmentBeginDataIndices[ nextSeg ] ] - 
                         formantTime[ segmentEndDataIndices[ thisSeg ] ] ) / frameDuration ) ;
            // LOOP FOR STEPS
            thisSegIndexEnd = segmentEndDataIndices[ thisSeg ] ;
            nextSegIndexBegin = segmentBeginDataIndices[ nextSeg ] ;

            // FIND THE SLOPE AND Y INTERCEPTS FOR THE LINK-FROM AND LINK-TO SEGMENTS
            // FOR THE CF, AMP, AND BW PARAMETERS.
            linkFromLength = (segmentLengths[ thisSeg ] > maxLength) ? maxLength : segmentLengths[ thisSeg ] ;
            linkToLength = (segmentLengths[ nextSeg ] > maxLength) ? maxLength : segmentLengths[ nextSeg ] ;
//fprintf( stderr, "\n linkFromLength: %d linkToLength: %d", linkFromLength, linkToLength ) ; 
            // FIND SLOPE AND Y INTERCEPTS.
            // LINK-FROM
            if( linkFromLength <= 1 ){
               yIntercept_LinkFromCF = formantCF[ thisSegIndexEnd ] ; slope_LinkFromCF = 0.0 ;
               yIntercept_LinkFromAmp = formantAmp[ thisSegIndexEnd ] ; slope_LinkFromAmp = 0.0 ;
               yIntercept_LinkFromBW = formantBW[ thisSegIndexEnd ] ; slope_LinkFromBW = 0.0 ;
               yIntercept_LinkFromQ = formantQ[ thisSegIndexEnd ] ; slope_LinkFromQ = 0.0 ;
            }else{
               l = thisSegIndexEnd - linkFromLength + 1 ;
               linearLeastSquaresProjection( 
                  0., &formantTime[ l ], &formantCF[ l ],
                  linkFromLength, &yIntercept_LinkFromCF, &slope_LinkFromCF
               ) ;
               linearLeastSquaresProjection( 
                  0., &formantTime[ l ], &formantAmp[ l ],
                  linkFromLength, &yIntercept_LinkFromAmp, &slope_LinkFromAmp
               ) ;
               linearLeastSquaresProjection( 
                  0., &formantTime[ l ], &formantBW[ l ],
                 linkFromLength, &yIntercept_LinkFromBW, &slope_LinkFromBW
               ) ;
               linearLeastSquaresProjection( 
                  0., &formantTime[ l ], &formantQ[ l ],
                  linkFromLength, &yIntercept_LinkFromQ, &slope_LinkFromQ
               ) ;
            } ;
            // LINK-TO
            if( linkToLength <= 1 ){
               yIntercept_LinkToCF = formantCF[ nextSegIndexBegin ] ; slope_LinkToCF = 0.0 ;
               yIntercept_LinkToAmp = formantAmp[ nextSegIndexBegin ] ; slope_LinkToAmp = 0.0 ;
               yIntercept_LinkToBW = formantBW[ nextSegIndexBegin ] ; slope_LinkToBW = 0.0 ;
               yIntercept_LinkToQ = formantQ[ nextSegIndexBegin ] ; slope_LinkToQ = 0.0 ;
            }else{
               linearLeastSquaresProjection( 
                  0., &formantTime[ nextSegIndexBegin ], &formantCF[ nextSegIndexBegin ],
                  linkToLength, &yIntercept_LinkToCF, &slope_LinkToCF
               ) ;
/*
for(k = 0; k < linkToLength; k++) 
fprintf( stderr, "\nk: %d formantAmp[ nextSegIndexBegin + k ]: %f formantTime[ nextSegIndexBegin + k ]: %f", 
   k, formantAmp[ nextSegIndexBegin + k], formantTime[ nextSegIndexBegin + k ] ) ; 
*/
               linearLeastSquaresProjection( 
                  0., &formantTime[ nextSegIndexBegin ], &formantAmp[ nextSegIndexBegin ],
                  linkToLength, &yIntercept_LinkToAmp, &slope_LinkToAmp
               ) ;
/*
fprintf( stderr, "\n A yIntercept_LinkFromAmp: %f yIntercept_LinkToAmp: %f slope_LinkFromAmp: %f slope_LinkToAmp %f", 
yIntercept_LinkFromAmp, yIntercept_LinkToAmp, slope_LinkFromAmp, slope_LinkToAmp ) ; 
*/
               linearLeastSquaresProjection( 
                  0., &formantTime[ nextSegIndexBegin ], &formantBW[ nextSegIndexBegin ],
                  linkToLength, &yIntercept_LinkToBW, &slope_LinkToBW
               ) ;
               linearLeastSquaresProjection( 
                  0., &formantTime[ nextSegIndexBegin ], &formantQ[ nextSegIndexBegin ],
                  linkToLength, &yIntercept_LinkToQ, &slope_LinkToQ
               ) ;
            } ;
            for(j = 1; j <= numberOfSteps; j++){ // HERE ???
              upRamp = (float) j / (float) numberOfSteps ; downRamp = 1.0 - upRamp ; 

              thisSegTime[ thisSegLength ] = timeShift + formantTime[ thisSegIndexEnd ] +
                 ((float) j * frameDuration) ; 

              thisSegCF[ thisSegLength ] = 
                 (  downRamp * ( (slope_LinkFromCF * thisSegTime[ thisSegLength ]) + yIntercept_LinkFromCF )  )
                    +
                 (  upRamp * ( (slope_LinkToCF * thisSegTime[ thisSegLength ]) + yIntercept_LinkToCF )  )
              ; 
              if(thisSegCF[ thisSegLength ] < 0. ) thisSegCF[ thisSegLength ] = 0. ;
            
/*
fprintf( stderr, "\n B yIntercept_LinkFromAmp: %f yIntercept_LinkToAmp: %f slope_LinkFromAmp: %f slope_LinkToAmp %f", 
yIntercept_LinkFromAmp, yIntercept_LinkToAmp, slope_LinkFromAmp, slope_LinkToAmp ) ; 
*/
              thisSegAmp[ thisSegLength ] = 
                 (  downRamp * ( (slope_LinkFromAmp * thisSegTime[ thisSegLength ]) + yIntercept_LinkFromAmp )  )
                    +
                 (  upRamp * ( (slope_LinkToAmp * thisSegTime[ thisSegLength ]) + yIntercept_LinkToAmp )  )
              ; 
               if( thisSegAmp[ thisSegLength ] < dB_to_amp( -96.0 ) ) 
                  thisSegAmp[ thisSegLength ] = dB_to_amp( -96.0 ) ;

//fprintf( stderr, "\nW j: %d  thisSegAmp[ j ]: %f thisSegLength: %d", j, thisSegAmp[ j ], thisSegLength ) ; 

              thisSegBW[ thisSegLength ] = 
                 (  downRamp * ( (slope_LinkFromBW * thisSegTime[ thisSegLength ]) + yIntercept_LinkFromBW )  )
                    +
                 (  upRamp * ( (slope_LinkToBW * thisSegTime[ thisSegLength ]) + yIntercept_LinkToBW )  )
              ; 
              thisSegQ[ thisSegLength ] = 
                 (  downRamp * ( (slope_LinkFromQ * thisSegTime[ thisSegLength ]) + yIntercept_LinkFromQ )  )
                    +
                 (  upRamp * ( (slope_LinkToQ * thisSegTime[ thisSegLength ]) + yIntercept_LinkToQ )  )
              ; 

              thisSegdB[ thisSegLength ] = (thisSegAmp[ thisSegLength ]
                        < dB_to_amp( -96. )) ? -96. : amp_to_dB( thisSegAmp[ thisSegLength ] ) ; 

              thisSegLength++ ;
           } ; 

        } ;
        // SET NEXT SEGMENT INDEX
        thisSeg = linkToSegment[ thisSeg ] ;
      } ;
/*
fprintf( stderr, "\nCONSTRUCTED SEGMENT LENGTH: %d", thisSegLength ) ; 

for(j = 0; j < thisSegLength; j++){
 fprintf( stderr, "\nT j: %d  thisSegAmp[ j ]: %f", j, thisSegAmp[ j ] ) ;
}; 
*/
      // IF RESULTING SEGMENT IS LONG ENOUGH AND SHORT ENOUGH, THEN . . . 
      segmentDuration = thisSegTime[thisSegLength - 1]  - thisSegTime[ 0 ] ; 
//      fprintf( stderr, "\nLENGTH: %d\tDURATION: %f", thisSegLength, segmentDuration ) ; 
      if( 
         (thisSegLength >= minimumSegmentLength) && 
         (segmentDuration >= minimumSegmentDuration) && 
         (thisSegLength <= maximumSegmentLength) && 
         (segmentDuration <= maximumSegmentDuration)
      ){
//         fprintf( stderr, "\nSUITABLE SEGMENT, WRITING . . . " ) ; 
         // WRITE OUT SAVED VALUES, ADDING ONSET/RELEASE ENVELOPE IF REQUESTED.
         // ADD ENVELOPE
         if( add_onset_and_release_points__no_0__append_1__impose_2 == 2 ){
            if( thisSegLength <= 2 ){
//fprintf( stderr, "\nG0");
               thisSegAmp[ 0 ] = 0. ; thisSegAmp[ thisSegLength - 1 ] = 0. ; 
               thisSegdB[ 0 ] = -96. ; thisSegdB[ thisSegLength - 1 ] = -96. ; 
            } else {
//fprintf( stderr, "\nG1");
               thisOnsetDuration = (onset_duration > (segmentDuration * 0.5)) ? 
                           segmentDuration * 0.5 : onset_duration ;
               thisReleaseDuration = (release_duration > (segmentDuration * 0.5)) ? 
                           segmentDuration * 0.5 : release_duration ;
               // APPLY ONSET AND RELEASE TO AMPS AND MAKE NEW DB

//fprintf( stderr, "\nBEFORE LOOP: thisSegLength: %d", thisSegLength ) ; 
               for(j = 0; j < thisSegLength; j++){
//fprintf( stderr, "\nG20");

                  if( (thisSegTime[ j ] - thisSegTime[ 0 ]) < thisOnsetDuration ){
                     // APPLY ONSET RAMP
//fprintf( stderr, "\nG21");
//fprintf( stderr, "\nX j: %d  thisSegAmp[ j ]: %f", j, thisSegAmp[ j ] ) ; 
                     thisSegAmp[ j ] *= ((thisSegTime[ j ] - thisSegTime[ 0 ]) / thisOnsetDuration) ;
                     thisSegdB[ j ] = (thisSegAmp[ j ] < dB_to_amp( -96. )) ? -96. :  dB_to_amp( thisSegAmp[ j ] ) ; 
                  } ;

                  if( (thisSegTime[ j ] - thisSegTime[ 0 ])  > (segmentDuration - thisReleaseDuration) ){
//fprintf( stderr, "\nG22");
// NEXT IS nan
/*
fprintf( stderr, "\nY j: %d thisSegAmp[ j ]: %f  thisSegTime[ thisSegLength - 1 ]: %f  thisSegTime[ j ]: %f thisReleaseDuration: %f", 
      j, thisSegAmp[ j ], thisSegTime[ thisSegLength - 1 ], thisSegTime[ j ], thisReleaseDuration ) ; 
*/

                     thisSegAmp[ j ] *= 
                        ( (thisSegTime[ thisSegLength - 1 ] - thisSegTime[ j ]) / thisReleaseDuration) ;
//fprintf( stderr, "\nG221");
/*
fprintf( stderr, "\nZ j: %d  thisSegAmp[ j ]: %f", j, thisSegAmp[ j ] ) ; 
                     thisSegdB[ j ] = (thisSegAmp[ j ] < dB_to_amp( -96. )) ? -96. :  dB_to_amp( thisSegAmp[ j ] ) ;
fprintf( stderr, "\nG222");
*/
                  } ;
               } ;
//fprintf( stderr, "\nG23");
            } ;
//fprintf( stderr, "\nG24");
         } ;      
//fprintf( stderr, "\nH0"); 
         // WRITE SEGMENT
         // WRITE LENGTH AS FLOAT TO BINARY FILE
         temp = (float) thisSegLength ;
         fwrite( &temp, sizeof(float), 1, BinaryFrequencySegmentsFile ) ; 
         for(j = 0; j < thisSegLength; j++ ){
               sprintf( tempstring, "%f %f %f %f %f %f\n", 
                     thisSegTime[ j ], thisSegCF[ j ], thisSegAmp[ j ], thisSegdB[ j ], thisSegBW[ j ], thisSegQ[ j ] ) ;
               fprintf( ASCIIfrequencySegmentsPlot, "%s", tempstring ) ;    

            // ??? WRITE A LINE OF BINARY OUTPUT.
            // FIRST PUT DATA INTO ARRAY FOR FASTER WRITING
            binaryOutputArray[ 0 ] = thisSegTime[ j ] ;
            binaryOutputArray[ 1 ] = thisSegCF[ j ] ;
            binaryOutputArray[ 2 ] = thisSegAmp[ j ] ;
            binaryOutputArray[ 3 ] = thisSegdB[ j ] ;
            binaryOutputArray[ 4 ] = thisSegBW[ j ] ;
            binaryOutputArray[ 5 ] = thisSegQ[ j ] ;

            fwrite( &binaryOutputArray, sizeof(float), 6, BinaryFrequencySegmentsFile ) ; 


         } ;
//fprintf( stderr, "\nH1"); 
         fprintf( ASCIIfrequencySegmentsPlot, "\n" ) ; 


         totalSegmentsFollowingLinking++ ;

      } ; // END OF TEST FOR MINIMUM/MAXIMUM LENGTH CHECK

   } ;
} ;

fclose( ASCIIfrequencySegmentsPlot ) ; 
fclose( BinaryFrequencySegmentsFile ) ; 


fprintf( stderr, "\nTOTAL SEGMENTS AFTER LINKING AND EXCLUDING: %d\t REDUCTION PROPORTION: %f\n",
	 totalSegmentsFollowingLinking, 
	(float) totalSegmentsFollowingLinking / (float) numberOfSegmentsFoundBeforeSelection 
) ; 

prbanner( "MIN, MAX, AND AVERAGE", 69 ) ; 
fprintf( stderr, "TIME\t\t\tMinimum: %f\tMaximum: %f\tAverage: %f", 
	formantTimeMinMaxAvg[ 0 ], formantTimeMinMaxAvg[ 1 ], formantTimeMinMaxAvg[ 2 ] ) ; 




fprintf( stderr, "\nCENTER FREQUENCY\tMinimum: %f\tMaximum: %f\tAverage: %f", 
	formantCFMinMaxAvg[ 0 ], formantCFMinMaxAvg[ 1 ], formantCFMinMaxAvg[ 2 ] ) ; 
fprintf( stderr, "\nAMPLITUDE (in dB)\tMinimum: %f\tMaximum: %f\tAverage: %f", 
	amp_to_dB( formantAmpMinMaxAvg[ 0 ] ), amp_to_dB( formantAmpMinMaxAvg[ 1 ] ), 
		amp_to_dB( formantAmpMinMaxAvg[ 2 ] ) ) ; 
fprintf( stderr, "\nBANDWIDTH\t\tMinimum: %f\tMaximum: %f\tAverage: %f", 
	formantBWMinMaxAvg[ 0 ], formantBWMinMaxAvg[ 1 ], formantBWMinMaxAvg[ 2 ] ) ; 
fprintf( stderr, "\nQ\t\t\tMinimum: %f\tMaximum: %f\tAverage: %f", 
	formantQMinMaxAvg[ 0 ], formantQMinMaxAvg[ 1 ], formantQMinMaxAvg[ 2 ] ) ; 
prline( 69,   "*" ) ; 

prline( 69,   "*" ) ; 
fprintf( stderr, "\nTOTAL CONSTRUCTED SEGMENTS: (pre-screening) %d\t(post-screening) %d", 
	numberOfSegmentsFoundBeforeSelection, totalSegmentsFollowingLinking ) ; 
if( numberOfSegmentsFoundAfterSelection > 0 ){
	segmentLengthMinMaxAvg[ 2 ] /= (float) numberOfSegmentsFoundAfterSelection ;
fprintf( stderr, "\nSEGMENT LENGTH\t\tMinimum: %d\tMaximum: %d\tAverage: %f",
	(int) segmentLengthMinMaxAvg[ 0 ], (int) segmentLengthMinMaxAvg[ 1 ], segmentLengthMinMaxAvg[ 2 ] ) ;
	if( totalBridgesUsed > 0 ){
		fprintf( stderr, "\nTOTAL SEGMENT BRIDGES: \t%d", totalBridgesUsed ) ;
		segmentBridgeLengthsMinMaxAvg[2] /= (float) totalBridgesUsed ;
		fprintf( stderr, "\nSEGMENT BRIDGE LENGTHS\t\tMinimum: %d\tMaximum: %d\tAverage: %f", 
			(int) segmentBridgeLengthsMinMaxAvg[ 0 ], (int)segmentBridgeLengthsMinMaxAvg[ 1 ], 
		segmentBridgeLengthsMinMaxAvg[ 2 ] ) ; 
	} ;
} ;


prline( 69,   "*" ) ; 

    
prline( 69,   "*" ) ; 
prs( ifile, "INPUT SOUND FILE" ) ;
prt( "DATA OUTPUT FILES:");  
if( strcmp( ASCIIfrequencyScatterPlotFile, "" ) != 0)
	prs( ASCIIfrequencyScatterPlotFile, "ASCII SCATTER PLOT FILE" ) ; 
if( strcmp( ASCIIfrequencySegmentsPlotFileName, "" ) != 0 ) 
    prs( ASCIIfrequencySegmentsPlotFileName, "ASCII FREQUENCY SEGMENTS PLOT FILE" ) ; 



prbanner( "SPECTRUM MAPPER : ANALYSIS COMPLETED", 69 ) ; 


exit(EXIT_SUCCESS) ;
}



void usage()
{
    fprintf(stderr, "%s",
	"spectrummapper:  spectrum formant trajectories mapper \n"
	"spectrummapper   [flags] [input sound file] [output spectrummapper file]\n"
	"	N:	"FFT_LENGTH 		// N
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout


	"	L:   low frequency limit [0.]\n"
	"	j:   high frequency limit [nyquist]\n"

	"	A:	minimum formant peak amplitude in dB [-96.]\n"

	"	g:   formant selection threshold, 0-1 [.5]\n"
	"			Higher thresholds produce fewer formants by selecting\n"
	"			for stronger spikes.\n" 

	"	B:	EQ with normalization (0 = yes, 1 = no) [0] \n"
	"		 (This is an administrative flag for silent use with \n"
	"		   noisefilter and compander. All other uses of freqresponse)\n"
	"		   normalize the output response.)\n"
	"	     "SHELF_EQ_HEADER
	"	H:	"SHELF_EQ_LOW_GAIN		// dBlow
	"	X:	"SHELF_EQ_HIGH_GAIN		// dBhi
	"	m:	"SHELF_EQ_LOW_FREQ		// freqlow
	"	R:	"SHELF_EQ_HIGH_FREQ		// freqhi

	"	    Segment Construction:\n"
	"	c:	minimum decibel limit [-200]\n"
	"	E:	maximum decibel limit [0]\n"
	"	o:	minimum segment length [1]\n"
	"	O:	maximum segment length [total number of frames]\n"

	"	F:	add onset and release points, 0 = no, 1 = append, 2 = impose [0] \n"	"	K:	maximum Allowed Bridging Frames [0]\n"	"	Q:	maximum Segment Duration [0.]\n"	"	T:	segment Linking Tolerance Percentage [1]\n"	"	U:	release duration \n"	"	V:	Maximum Decibel Rise Per Millisecond  [90]\n"	"	W:	maximum Frequency Linkage [100]\n"	"	n:	time Shift [0]\n"	"	p:	Maximum Frequency Change Per Millisecond [12]\n"	"	q:	minimum Segment Duration [0]\n"	"	r:	linkage Time [0.02]\n"	"	u:	onset duration\n"	"	v:	Maximum Decibel Fall Per Millisecond [90]\n"


	"	S:	Binary Frequency Segments File Name\n"	"	f:	ASCII Frequency Scatter Plot File\n"
	"	a:	ASCII frequency Segments Plot File Name\n"
	"	P:	"FREQUENCY_RESPONSE_PRINTOUT

	);
    exit(EXIT_SUCCESS);
} ;





float findPeakAmp(
    float A[],
    int N2
 
){
    float peakAmp ;
    int i ;  

   // FIND STRONGEST FREQ
    peakAmp = -999999.0 ;
    for( i = 0; i < N2; i += 2) if( A[i] > peakAmp ) peakAmp = A[i] ; 

    return( peakAmp ) ; 

} ; 


/* SUBROUTINE JUNKYARD

float makeTargetCF(
	int direction,
	float groupTime, // FORMANT GROUP FRAME TIME
	int numberOfFrontLegFormants,
	int numberOfBackLegFormants,
	float formantCF[],
	float formantTime[],
	int forwardLegOfSegmentIndices[],
	int backwardLegOfSegmentIndices[]
){

	float targetFormantCF ;
	float scaler, x1, x2, y1, y2 ;


//fprintf( stderr, "\n direction: %d, groupTime: %f, numberOfFrontLegFormants: %d, numberOfBackLegFormants: %d", 
//direction, groupTime, numberOfFrontLegFormants, numberOfBackLegFormants ) ; 

	// *** MAKE TARGET CF *******

	// IF ONE OR MORE FORMANTS EXIST IN THE LEG, THEN USE THE SLOPE OF THE 
	// LAST TWO TO CONSTRUCT THE PROJECTED TARGET. IF NO FORMANTS YET, THEN PROJECT
	// THE PEAK FORMANT AS THE NEXT. 
	if( direction == 1 ){  
		// FRONT LEG

		if( numberOfFrontLegFormants == 1 ){
			// ONE FORMANT (PEAK)
	         	targetFormantCF = formantCF[ forwardLegOfSegmentIndices[ 0 ]  ] ;

		}else{
			// MULTIPLE FORMANTS
                             x1 = formantTime[ forwardLegOfSegmentIndices[numberOfFrontLegFormants - 2] ] ;
                             x2 = formantTime[ forwardLegOfSegmentIndices[numberOfFrontLegFormants - 1] ] ;
                             y1 = formantCF[ forwardLegOfSegmentIndices[numberOfFrontLegFormants - 2] ] ;
                             y2 = formantCF[ forwardLegOfSegmentIndices[numberOfFrontLegFormants - 1] ] ;
			scaler = (groupTime - x2 ) / (x2 - x1) ; 
                            targetFormantCF = y2 + (scaler * (y2 - y1)) ; 

		} ;

	} else {

		// BACK LEG
	     	if( numberOfBackLegFormants == 1 ){
	        		// NO FORMANTS IN BACK LEG EXCEPT PEAK. USE PEAK FORMANT ALONE AS TARGET 
	        		// OR INTEGRATE, IF IT EXISTS, FIRST FRONT LEG FORMANT INTO TARGET CONSTRUCTION.
	        		if( numberOfFrontLegFormants == 0 ){
	           		// NO FRONT LEG FORMANTS TO INTEGRATE; USE PEAK AS TARGET.
	           		targetFormantCF = formantCF[ backwardLegOfSegmentIndices[ 0 ] ] ;
	        		}else{
	           		// USE SECOND FRONT LEG FORMANT (FIRST PAST PEAK) 
                                 // WITH PEAK FORMANT (FIRST IN BACK LEG) TO CONSTRUCT TARGET.
                                      x1 = formantTime[ forwardLegOfSegmentIndices[ 1 ] ] ;
                                      x2 = formantTime[ backwardLegOfSegmentIndices[ 0 ] ] ;
                                      y1 = formantCF[ forwardLegOfSegmentIndices[ 1 ] ] ;
                                      y2 = formantCF[ backwardLegOfSegmentIndices[ 0 ] ] ;
			         scaler = (groupTime - x2) / (x2 - x1) ; 
                                      targetFormantCF = y2 + (scaler * (y2 - y1)) ; 
			} ; 

		} else {
 
	        		// MULTIPLE FORMANTS.
			x1 = formantTime[ backwardLegOfSegmentIndices[numberOfBackLegFormants - 2] ] ;
			x2 = formantTime[ backwardLegOfSegmentIndices[numberOfBackLegFormants - 1] ] ;
			y1 = formantCF[ backwardLegOfSegmentIndices[numberOfBackLegFormants - 2] ] ;
			y2 = formantCF[ backwardLegOfSegmentIndices[numberOfBackLegFormants - 1]  ] ;
			scaler = (groupTime - x2) / (x2 - x1) ; 
			targetFormantCF = y2 + (scaler * (y2 - y1)) ; 
	       	} ;

	} ; // END OF ROUTINE FOR MAKING TARGET FREQ

   return( targetFormantCF ) ; 

} ;

int computeAmpAndFreqCorrelationFactor(
	int i0,
	int i1,
	float *dBSlopeAverage,
	float *semitoneSlopeAverage,
	float *dBaccelerationAverage,
	float *semitoneAccelerationAverage,
	float shortdBslope[],
	float longdBslope[],
	float shortMidiSlope[],
	float longMidiSlope[],
	float formantTime[],
	float formantCF[],
	float formantAmp[],

	int segmentLengths[], 
	int segmentBeginDataIndices[], 
	int segmentEndDataIndices[], 

	float segmentBeginTimes[], 
	float segmentEndTimes[] 
){
	int shortSegDataIndex, longSegDataIndex, 
		shortSegBeginDataIndex, shortSegEndDataIndex, 
		longSegBeginDataIndex, longSegEndDataIndex,
		slopeSegLength, accelerationSegLength,
		i, j, k, l ;
	float dBabsoluteSlopeDiffSum=0., midiAbsoluteSlopeDiffSum=0. ;
	float shortdBslopeSum, longdBslopeSum, shortMidiSlopeSum, longMidiSlopeSum    ; 
	float shortdBaccelerationSum, longdBaccelerationslopeSum, 
		shortMidiAccelerationSum, longMidiAccelerationSum    ; 
	float dBaccelerationDiff=0., midiAccelerationDiff=0. ;
	float v0, v1 ;
	float t0b, t0e, t1b, t1e ;
	float tbegin, tend ; 
	int foundFlag, i0b, i0e, i1b, i1e ; 

	fprintf( stderr, "\nSEGMENT LENGTHS: %d\t%d", 
		segmentLengths[ i0 ], segmentLengths[ i1 ] ); 

	t0b = formantTime[ segmentBeginDataIndices[i0] ] ;
	t0e = formantTime[ segmentEndDataIndices[i0] ] ;
	t1b = formantTime[ segmentBeginDataIndices[i1] ] ;
	t1e = formantTime[ segmentEndDataIndices[i1] ] ;

//	fprintf( stderr, "\n t0b: %f t0e: %f t1b: %f t1e: %f", t0b, t0e, t1b, t1e ) ; 
	   
	tbegin = t1b >= t0b ? t1b : t0b ;
	tend = t0e <= t1e ? t0e : t1e ;

//	fprintf( stderr, "\ntbegin: %f\ttend: %f", tbegin, tend ) ; 

	i0b = segmentBeginDataIndices[ i0 ] ;
	i1b = segmentBeginDataIndices[ i1 ] ;
	i0e = segmentEndDataIndices[ i0 ] ;
	i1e = segmentEndDataIndices[ i1 ] ;
//	fprintf( stderr, "\nBEFORE SHORTENING\n\ti0b: %d i0e: %d i1b: %d i1e: %d", i0b, i0e, i1b, i1e ) ; 
 
	foundFlag = 0 ;
 	while( foundFlag == 0 ){
		if( formantTime[ i0b ] >= tbegin ) foundFlag = 1 ; else i0b += 1 ; } ; 
	foundFlag = 0 ;
 	while( foundFlag == 0 ){
		if( formantTime[ i1b ] >= tbegin ) foundFlag = 1 ; else i1b += 1 ; } ; 
	foundFlag = 0 ;
 	while( foundFlag == 0 ){
		if( formantTime[ i0e ] <= tend ) foundFlag = 1 ; else i0e -= 1 ; } ; 
	foundFlag = 0 ;
	while( foundFlag == 0 ){
		if( formantTime[ i1e ] <= tend ) foundFlag = 1 ; else i1e -= 1 ; } ; 

//	fprintf( stderr, "\ni0b: %d i0e: %d i1b: %d i1e: %d", i0b, i0e, i1b, i1e ) ; 
	while( (i0e - i0b) > (i1e - i1b) ) i0e -= 1 ;  
	while( (i1e - i1b) > (i0e - i0b) ) i1e -= 1 ; ;
//	fprintf( stderr, "\ni0b: %d i0e: %d i1b: %d i1e: %d", i0b, i0e, i1b, i1e ) ; 
 

	
	// SET UP LENGTH OF SLOPE SEGMENTS ;
	slopeSegLength = (i0e - i0b) + 1 ; 
//	fprintf( stderr, "\nSLOPE SEG LENGTH: %d", slopeSegLength ) ; 

	// TRANSFER SEGMENT PIECES AS SLOPES INTO SCRATCH ARRAYS
	for( i = i0b + 1, j = i1b + 1, k = 0; i <= i0e; i++, j++, k++ )
	{

		if( formantAmp[ i ] < dB_to_amp( -96. ) ) v0 = -96. ;
		else v0 = amp_to_dB( formantAmp[ i ]  ); 
		if( formantAmp[i - 1] < dB_to_amp( -96. ) ) v1 = -96. ;
		else v1 = amp_to_dB( formantAmp[i - 1]  ); 
		shortdBslope[ k ] = v0 - v1 ;

		if( formantAmp[ j ] < dB_to_amp( -96. ) ) v0 = -96. ;
		else v0 = amp_to_dB( formantAmp[ j ]  ); 
		if( formantAmp[j - 1] < dB_to_amp( -96. ) ) v1 = -96. ;
		else v1 = amp_to_dB( formantAmp[j - 1]  ); 
		longdBslope[ k ] = v0 - v1 ;

		shortMidiSlope[ k ] = 
			Hz_to_MIDI( formantCF[ i ] ) - Hz_to_MIDI( formantCF[i - 1] ) ;		
		longMidiSlope[ k ] = 
			Hz_to_MIDI( formantCF[ j ] ) - Hz_to_MIDI( formantCF[j - 1] ) ;		
	} ;

//	for(i = 0; i < slopeSegLength; i++ ) fprintf( stderr, "\n longdBslope: %f dB", longdBslope[ i ] ) ; 
//	for(i = 0; i < slopeSegLength; i++ ) fprintf( stderr, "\n longMidiSlope: %f semitones",
//		 longMidiSlope[ i ] ) ; 
	// FIND SUMS ;
	dBabsoluteSlopeDiffSum = 0. ; midiAbsoluteSlopeDiffSum = 0. ;
	shortdBslopeSum = 0. ; longdBslopeSum = 0. ; shortMidiSlopeSum = 0. ; longMidiSlopeSum  = 0. ;  
	for(i = 0; i < slopeSegLength; i++ ){
		shortdBslopeSum += shortdBslope[ i ] ;
		longdBslopeSum += longdBslope[ i ] ;
		shortMidiSlopeSum += shortMidiSlope[ i ] ;
		longMidiSlopeSum += longMidiSlope[ i ] ;
	};
	// FIND SUM DIFFERENCES
	dBabsoluteSlopeDiffSum = longdBslopeSum - shortdBslopeSum ;
	midiAbsoluteSlopeDiffSum = longMidiSlopeSum - shortMidiSlopeSum ;

//	fprintf( stderr, "\nSUMS: dBabsoluteSlopeDiffSum: %f", dBabsoluteSlopeDiffSum ) ;  
//	fprintf( stderr, "\nSUMS: midiAbsoluteSlopeDiffSum: %f", midiAbsoluteSlopeDiffSum ) ;  

	// FIND AVERAGES	
	*dBSlopeAverage = dBabsoluteSlopeDiffSum / (float) slopeSegLength ;
	*semitoneSlopeAverage = midiAbsoluteSlopeDiffSum / (float) slopeSegLength ;

	// FIND ACCELERATION
	accelerationSegLength = slopeSegLength - 1 ; 
	if( accelerationSegLength >= 1 ){
		// MAKE VALUES USING SLOPE ARRAY SPACE.
		for( i = 1, j = 0; i < slopeSegLength; i++, j++ ){
			shortdBslope[i - 1] = shortdBslope[ i ] - shortdBslope[ j ] ;	
			longdBslope[i - 1] = longdBslope[ i ] - longdBslope[ j ] ;	
			shortMidiSlope[i - 1] = shortMidiSlope[ i ] - shortMidiSlope[ j ] ;	
			longMidiSlope[i - 1] = longMidiSlope[ i ] - longMidiSlope[ j ] ;	
		} ;
		fprintf( stderr, "\n accelerationSegLength: %d", accelerationSegLength ) ; 
		if( accelerationSegLength < 1 )fprintf( stderr, "\n ****** ERROR ******" ) ; 
		// FIND SUMS OF THE ABSOLUTE VALUES ;
		shortdBaccelerationSum = 0.; longdBaccelerationslopeSum = 0.;  
		   shortMidiAccelerationSum = 0.; longMidiAccelerationSum = 0.; 	
		for(i = 0; i < accelerationSegLength; i++ ){
			shortdBaccelerationSum += fabs( shortdBslope[ i ] ) ;	
			longdBaccelerationslopeSum += fabs( longdBslope[ i ] ) ;
			shortMidiAccelerationSum += fabs( shortMidiSlope[ i ] ) ;	
			longMidiAccelerationSum += fabs( longMidiSlope[ i ] ) ;	
		};
		// FIND SUM DIFFERENCES
		dBaccelerationDiff = fabs(longdBaccelerationslopeSum - shortdBaccelerationSum) ;
		midiAccelerationDiff = fabs(longMidiAccelerationSum - shortMidiAccelerationSum) ;

		// FIND AVERAGES		
		*dBaccelerationAverage = dBaccelerationDiff / (float) accelerationSegLength ;
		*semitoneAccelerationAverage = midiAccelerationDiff / (float) accelerationSegLength ;
	}else{
		*dBaccelerationAverage = 0. ;
		*semitoneAccelerationAverage = 0. ;
	} ; 


	return(1) ; 
} ;


*/
