#include <stdio.h>
#include <math.h>
#include "pv.h"

#define SMOOTHING_WINDOW_AS_PROPORTION_OF_CENTROID 1./50.
#define FORMANT_OVERLAP_TOLERANCE_PROP .5

int get_formants( 
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
	float nyquist
){



float *amps, *freqs, *newAmps, * testAmpsSave, *AmpsDerivative,
	*triWindow, *winArray, *v, *w, *symmetryFactor, *selectedFormantsAmps,
	*formantAmpsCopy, *tempList, *peakAmps, *avgAmps, *HammingWindow, *diffdBs  ; 

float centroidFreq, sumOfAmps, strongestFreq, peakAmp, baseAmp, windowBW, freqsDiffSum, freqsDiffMax,
		avgBinFreqDiff, vv, thisDBthreshold, 
	newAmpsPeakAmp, scaleFactor, peakAmpIn_v_and_w, vSum, wSum, diffProp ; 
int includesflag, tempListLengthNow, sign ;
float thisMaxAmp, sum, sum1, sum2, meanBW, maxBW ;
float TWOPI ;  
int HammingWindowSize=1024 ;
 
int lowIndex, highIndex, thisHammingIndex, thisLowIndex, thisHighIndex ;
float cf, fundamental, ampSum, thisHammingSum, thisAmp, HammingFloatIndex  ; 



static int first=1 ; 

float lowdBdiff, highdBdiff, lowFreqDiff, highFreqDiff ; 

float thisCF, thisCFdB, dBdiff, freqDiff, thisFreqDiff, thisFreq ; 
int formantIndex, thisIndex, found ; 

float baseDB, lowDB, highDB, lowFreq, highFreq, temp ; 	

int thisFormant, indexOfFormant, numBins ; 	    

float *freqStasis, *tempFreqStasisSpectrum ; 
float stdDev, freqThreshold, freqDiffAvg ; 
FILE *fopen(), *adata;

int avgWinSize, n, c, exitFlag ; 
int N2, i, j, k, l, indexOfPeak ; 

static int lastN=0 ; 


//prf( lowFreqLimit,  "lowFreqLimit" ) ; 
//prf( highFreqLimit,  "highFreqLimit" ) ; 


    TWOPI = 8.*atan(1.) ;

	if( first == 1 ){
		// FIRST MAKE HAMMING WINDOW 
      	fvec( HammingWindow, HammingWindowSize ) ; 
		for ( i = 0 ; i < HammingWindowSize ; i++ ){
             	HammingWindow[i] = 0.54 - (0.46 * cos( (double)(TWOPI * (float) i / (float)(HammingWindowSize - 1)  ) ) ) ;
//prf( HammingWindow[i], "HammingWindow"  ) ; 
		} ;
	} ; 


//    if( N > lastN ){
        N2 = N>>1 ; 
	fvec( amps, N2 ) ; fvec( freqs, N2 ) ; fvec( newAmps, N2 ) ; 
	fvec( AmpsDerivative, N2 ) ; 
	fvec( testAmpsSave, N2 ) ; 
	fvec( freqStasis, N2 ) ; 
	fvec( triWindow, N2 ) ;
        	fvec( winArray, N2 ) ; 
	fvec( symmetryFactor, N2 ) ; 
	fvec( v, N2 ) ; fvec( w, N2 ) ; 
	fvec( formantAmpsCopy, N2 ) ; 
	fvec( tempList, N2 ) ; // tempListLengthNow
	fvec( peakAmps, N2 ) ; fvec( avgAmps, N2 ) ; fvec( diffdBs, N2 ) ; 

	lastN = N ; 
//    } ; 

//    N2 = N>>1 ; 

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
// prt( "H3"); 

    // STD DEV
    stdDev = 0.0 ; 
    for(i = 0; i < N2; i++) stdDev += pow( (freqStasis[i] - freqDiffAvg), 2.0); 
    stdDev = sqrt( stdDev / (float) N2 ) ; 

    freqThreshold = freqDiffAvg - (0.5 * stdDev) ; 
    for(i = 0; i < N2; i++){ 
	if( freqStasis[i] > (freqDiffAvg - stdDev) ) freqStasis[i] = 0.0 ; 
	else freqStasis[i] = 1.0 - (freqStasis[i] / (freqDiffAvg - stdDev) ) ;  
	freqStasis[i] = curve( 0.0, 1.0, freqStasis[i], -12 ) ; 
    }; 

    if( strcmp( freqStasisPlotFile, "") != 0 ){
	fvec( tempFreqStasisSpectrum, N ) ; 
	for(i = 0, j = 0; i < N2; i++, j += 2){
	   tempFreqStasisSpectrum[j] = freqStasis[i] ;
	   tempFreqStasisSpectrum[j + 1] = freqs[i] ;
	} ; 
	writeSpectrumPlotFile( freqStasisPlotFile, tempFreqStasisSpectrum, N, 0 ) ; 
    }; 

	// ********


	// CREATE AVERAGE WINDOW SIZE
    avgWinSize = (int)(windowBW / avgBinFreqDiff) ; 
    if( fmod( (double) avgWinSize, 2.0 ) == 0.0 ) avgWinSize = avgWinSize + 1 ; 
    if( avgWinSize < 3) avgWinSize = 3 ;
    n = (avgWinSize - 1) / 2 ; 

	// FIND PEAK AMP
    peakAmp = -999999.0 ; 
    for( i = 0; i < N2; i++ ){
        for( n = 0; n < avgWinSize; n++ ){
	    k = n + i - (avgWinSize / 2) ; 
	    if( k < 0 ) k = 0 ;
	    if( k > (N2 - 1)) k = N2 - 1;
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
    
    for( i = 0; i < N2; i++) newAmps[ i ] = newAmps[ i ] / peakAmp ; 




    // SECRET PLOT OF "old" amps
    adata = fopen( "/tmp/oldAmps", "w+" ) ; 
    for(i = 0; i < N2; i++){
	temp = amp_to_dB( amps[i] ) ; 
	if( temp < -96 ) temp = -96 ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 

    // SECRET PLOT OF newAmps
    adata = fopen( "/tmp/newAmps", "w+" ) ; 
    for(i = 0; i < N2; i++){
	temp = amp_to_dB( newAmps[i] ) ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 

    // SECRET PLOT OF oldAmps - newAmps
    adata = fopen( "/tmp/oldMinusNewAmps", "w+" ) ; 
    for(i = 0; i < N2; i++){
	temp = amp_to_dB( amps[i] ) - amp_to_dB( newAmps[i] ) ; 
	if( temp < -200 ) temp = -200 ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 




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


    // SECRET PLOT OF peakAmps
    adata = fopen( "/tmp/peakAmps", "w+" ) ; 
    for(i = 0; i < N2; i++){
	temp = amp_to_dB( peakAmps[i] ) ; 
	if( temp < -96 ) temp = -96 ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 
    // SECRET PLOT OF avgAmps
    adata = fopen( "/tmp/avgAmps", "w+" ) ; 
    for(i = 0; i < N2; i++){
	temp = amp_to_dB( avgAmps[i] ) ; 
	if( temp < -96 ) temp = -96 ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 
   // SECRET PLOT OF diffdBs
    adata = fopen( "/tmp/diffdBs", "w+" ) ; 
    for(i = 0; i < N2; i++){
	temp = diffdBs[i] ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 



// prt( "H40"); 
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

/*
	// FIND FORMANT FREQS AND AMPS
    *numFormants = 0 ;  
    for( i = 1; i <= (N2 - 2); i++ ) { 
//	if( (newAmps[i] > newAmps[i - 1]) && (newAmps[i] > newAmps[i + 1]) ) 
	if( (amps[i] > amps[i - 1]) && (amps[i] > amps[i + 1]) ) 
	{
	    if( (freqs[i] >= lowFreqLimit) && (freqs[i] <= highFreqLimit) )
	    {
		// CONSIDER AS  A FORMANT
		// REFINE POSITION AGAINST ORIGINAL AMPS AND PEAKS IN CASE OF SMOOTHING SLIPPAGE.  
		// FIND PEAK AMP AMONGST THIS BIN AND TWO ADJACENT BINS USING ORIGINAL AMPS.
		indexOfPeak = 0 ; peakAmp = -9999999.0 ; 
		for(k = 0; k < 3; k++ ){
		    j = (i - 1) + k; if( j < 0 ) j = 0 ; if( j >= (N2 - 2))j = N2 - 2; 
		    if( amps[j] > peakAmp){
			indexOfPeak = j ; peakAmp = amps[j] ; 
		    } ; 
		} ; 

//		if( indexOfPeak != i) 
//		fprintf( stderr, "READJUSTING PEAK POSITION => OLD PEAK: %d\tNEW PEAK: %d\n", 
//			i, indexOfPeak ) ; 

		// RECHECK PEAK TO SEE IF IT IS STILL PEAK 
		if( (indexOfPeak != 0) && (indexOfPeak != (N2 - 1)) ) {
		    // NEW PEAK; CHECK AGAIN
		    if( (amps[indexOfPeak] > amps[indexOfPeak - 1]) && 
				(amps[indexOfPeak] > amps[indexOfPeak + 1]) ) {		    
			formantCenterFreqs[ *numFormants  ] = freqs[indexOfPeak] ; 
			formantAmps[ *numFormants  ] = newAmps[indexOfPeak] ;
			formantIndices[ *numFormants  ] = indexOfPeak ;  
			*numFormants += 1 ; 
		    } ; 
		};  
	    } ; 
	} ; 
    } ; 
*/

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
					(  ((thisCFdB - lowDB) > (thisDBthreshold * (1. - FORMANT_OVERLAP_TOLERANCE_PROP ))) 
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

		lowFreq = freqs[ formantLowStopBandIndices[i] ] ; 
		highFreq = freqs[ formantHighStopBandIndices[i] ] ; 
		lowDB = amp_to_dB(amps[ formantLowStopBandIndices[i] ]) ; 
		highDB = amp_to_dB(amps[ formantHighStopBandIndices[i] ]) ; 
	
	
		// CREATE A BASE DB TAKEN FROM THE POINT ALONG THE DIFFERENCE BETWEEN LOW AND HIGH DB 
		// LEVELS AT THE POSITION OF THE CF LYING BETWEEN THE LOW AND HIGH FREQS. 
		baseDB = 
	    		lowDB + ( (highDB - lowDB) * ((thisCF - lowFreq) / (highFreq - lowFreq)) ) ; 

		// MAKE Q 		BW / CF
		dBdiff = thisCFdB - baseDB ; 
		formantBWs[i] = (3. / dBdiff) * (highFreq - lowFreq) ; 
		formantQs[i] =  thisCF / formantBWs[i] ; 

//		fprintf( stderr, 
//			"i: %d thisCF: %d lowDB: %d highDB: %d baseDB: %d thisCFdB: %d dBdiff: %d\n",
//			i, (int)thisCF, (int)lowDB, (int)highDB, (int)baseDB, (int)thisCFdB, (int)dBdiff 
//		); 
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


    free( amps ) ; 
    free( freqs ) ; 
    free( newAmps ) ; 
    free( AmpsDerivative ) ; 
    free( testAmpsSave ) ; 
    free( freqStasis ) ; 
    free( triWindow ) ;
    free( winArray ) ; 
    free( symmetryFactor ) ; 
    free( v ) ; free( w ) ; 
    free( formantAmpsCopy ) ; 
    free( tempList ) ; // tempListLengthNow
    free( peakAmps ) ; 
    free( avgAmps ) ; 
    free( diffdBs ) ; 





    return(1) ; 
} ; 

