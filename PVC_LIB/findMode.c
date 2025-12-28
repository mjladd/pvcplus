#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

float findMode( 
	float	array[],
	int 	numberOfValues,
	float modesMergeWidth,
	int *modeFound
) 
{
	int	i, ii, k,  notdone ;
	static int *bincounts, lastpeaknumbins=0, lastPeakNumberOfValues=0 ; 
	static float *binsums, *binmodes, *temparray ; 
	int save ;  
	float floatSave ; 
	float binwidth, mode ; 
	int numbins ;
	float lowdivision, highdivision ;
	float low, high, temp, standarddeviation, mean, tempsum ;   

	
	// ENLARGE TEMP ARRAY SPACE IF NEEDED. 
	if( numberOfValues > lastPeakNumberOfValues){
		fvec( temparray, numberOfValues ) ; lastPeakNumberOfValues = numberOfValues ;  
	} ; 

	// TRANSFER ARRAY TO TEMP
	for(i = 0; i < numberOfValues; i++ ) temparray[i] = array[i] ; 

	// SET FLAG TO OFF.
	*modeFound = 0 ;


	// FIND LOW AND HIGH ARRAY VALUES
	low = high = temparray[0] ; 
	for(i = 1; i < numberOfValues; i++){
		if(temparray[i] > high) high = temparray[i] ; 	if(temparray[i] < low) low = temparray[i] ; 	
	} ; 

	// SET NUMBER OF HISTOGRAM BINS TO 100.
	numbins = 100. ;


	// LOOP FOR FINDING A SUITABLE MODE
	while( (*modeFound != 1) && (numbins > 2) ){

		// DIVIDE RANGE BY NUMBER OF BINS TO DETERMINE BIN WIDTH.
		binwidth = (high - low) / (float) numbins ; 

		// ENLARGE ARRAYS IF NOW GREATER.
		if( numbins > lastpeaknumbins ){
			ivec( bincounts, numbins )  ;  fvec( binsums, numbins )  ; fvec( binmodes, numbins ) ; 
			lastpeaknumbins = numbins ; 
		}; 

		// ZERO BIN HISTOGRAM DATA FOR THIS PASS
		for(i = 0; i < numbins; i++) { bincounts[i] = 0 ; binsums[i] = 0. ; binmodes[i] = 0. ;  }; 


		// FILL BINS WITH DATA
		for(i = 0; i < numberOfValues; i++){
			for(k = 0, lowdivision = low, highdivision = low + binwidth; 
					k < numbins; 
						k++, lowdivision += binwidth, highdivision += binwidth){ 
				if( k == 0 ){
					if( (temparray[i] >= lowdivision) && (temparray[i] <= highdivision) ){
						 bincounts[k]++ ; binsums[k] += temparray[i] ; 						
					} ; 
				}else{
					if( (temparray[i] > lowdivision) && (temparray[i] <= highdivision) ){
						 bincounts[k]++ ; binsums[k] += temparray[i] ; 						
					} ; 						
				} ; 
			} ; 
		} ; 

		// MAKE BIN MODE VALUES IF SUM IS NOT ZERO.
		for(i = 0; i < numbins; i++) 
			if( binsums[i] != 0. ) binmodes[i] = binsums[i] / (float) bincounts[i] ; 

		// MERGE ADJACENT BINS WITHIN MERGING DISTANCE
		if(modesMergeWidth > 0.){
			for(i = numbins - 1 ; i > 0 ; i--){
				if( fabs(binmodes[i] - binmodes[i - 1]) <= modesMergeWidth ){
					// MERGE
					bincounts[i - 1] = bincounts[i] + bincounts[i - 1] ; bincounts[i] = 0. ;
					binsums[i - 1] = binsums[i] + binsums[i - 1] ; binsums[i] = 0. ;  
					binmodes[i - 1] = binsums[i - 1] / (float) bincounts[i - 1]  ; binmodes[i] = 0. ;  
				} ; 
			} ; 
		} ; 

		// SORT BIN DATA COUNTS
		notdone = 1 ; 
		while( notdone == 1 ){ 
			notdone = 0 ; 
			for(ii = 0; ii < 2; ii++){
				for( i = ii ; i  < (numbins - 1); i += 2 ){
		    			if( bincounts[i] > bincounts[i + 1] ){
						// SWITCH
						save = bincounts[i] ; bincounts[i] = bincounts[i + 1] ; bincounts[i + 1] = save ; 
						floatSave = binsums[i] ; binsums[i] = binsums[i + 1] ; binsums[i + 1] = floatSave ;
						floatSave = binmodes[i] ; binmodes[i] = binmodes[i + 1] ; binmodes[i + 1] = floatSave ;
						notdone = 1 ; 
					} ; 
		    		} ; 
			}; 
	    	} ; 


/*
for(i = 0; i < numbins; i++)
	if(binsums[i] > 0.) 
	fprintf( stderr, "\n%d: bincounts: %d, binsums: %f, mode: %f", 
		i, (int)bincounts[i], binsums[i], binmodes[i] ); 
*/
		temp = (float) bincounts[numbins - 1] / (float) bincounts[numbins - 2] ; 

		// TEST FOR MODAL PROMINENCE
		if( temp > 1.2 ){
			mode = binsums[numbins - 1] / (float) bincounts[numbins - 1] ; 
			*modeFound = 1 ; 
		}else{
			numbins-- ; 
		} ;  
	} ; 

	return( mode ) ; 
}