#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#define WIDTH 132

int findFuncStats( 
	struct func *p, 
	int 	numberOfValues, 
	float 	*low,  
	float 	*high,  
	float 	*range,  
	float 	*average,
	float 	*median, 
	float 	*mode, 
	float 	*standarddeviation,
	float 	*sum,
	float 	*begin, 
	float 	*end,
	float 	*middle, 
	int 		printflag,
	int		freqUnitsFlag
) 
{
	int	i, ii,  pd(), count=0, notdone ;
	float	t, *value ;  
	FILE	*fopen();  
	float *temp_array, temp, tempsum;
	  
	 
	int modeFoundFlag ;
	  

	float thislow, thishigh, thisrange, thisaverage, thismedian, thismode, thisbegin, thisend, thismiddle ; 

	numberOfValues = p->n ; 

//pri( numberOfValues, "numberOfValues" ) ; 

	fvec( value, numberOfValues ) ; 
	
//prt( "h1" ) ; 

	if( p->n <= 1.){
		// ONE VALUE - NO FUNCTION FILE
		*low = *high = *average = *sum = *median = *mode = *begin = *end = *middle = p->A[ 0 ] ; 
		*standarddeviation = *range = 0. ; 
		return(1) ; 	
	}else{
		// FUNCTION FILE
		// READ IN AND STAT VALUES.
		i = 0 ;
//prt( "h2" ) ; 

		if(  p->L == 0. ){

//prt( "h3" ) ; 
			rewind( p->fp ) ;	 

			// ASCII FILE
			while( fscanf( p->fp,  "%f",  &t ) == 1 ){
				value[ count ] = t ; count++ ; i += 1 ;
//fprintf( stderr, "\ncount: %d %f", count, value[ count - 1 ] ) ; 
			} 

		}else{

//prt( "h4" ) ; 

			rewind( p->fp ) ;	 

	    	// FLOAT FILE
			while( fread( &t, sizeof(float), 1, p->fp ) == 1 ){
				value[ count ] = t; count++ ; i += 1 ;
			} 

		} ;

	} ; 

//prt( "h5" ) ; 


	fvec( temp_array, numberOfValues ) ;  

//prt( "h6" ) ; 
	
	*sum = 0. ; 
	for(i = 0; i < numberOfValues; i++){
		if(i == 0){
			*low = value[i] ;
			*high = value[i] ;  
		}else{
			if(value[i] < *low) *low = value[i] ; 
			if(value[i] > *high) *high = value[i] ; 
		}; 
		*sum += value[i]; 
	}; 

//prt( "h7" ) ; 

	*average = *sum / (float) numberOfValues ;  
	*begin = value[0] ; *end = value[ numberOfValues - 1] ; *middle = value[ numberOfValues / 2 ] ;  
	*range = *high - *low ;  

	for(i = 0; i < numberOfValues; i++)temp_array[i] = value[i] ; 

	notdone = 1 ; ii = 0 ; 

//prt( "h8" ) ; 

	// SORT
	while( notdone ){ 
		notdone = 0 ; 
		for( i = ii ; i  < (numberOfValues - 1 - ii); i += 2 ){
		    if( temp_array[i] > temp_array[i + 1] ){
				// SWITCH
				temp = temp_array[i] ; temp_array[i] = temp_array[i + 1] ; temp_array[i + 1] = temp ; 
				notdone = 1 ; 
		    }
	    }
	    if( ii == 0 ) ii = 1 ; else ii = 0 ; 
	} ; 

//prt( "h9" ) ; 


//	for(i = 0; i < numberOfValues; i++) prf( temp_array[i], "temp_array[i]" ) ; 

	temp = (float) numberOfValues / 2. ;
	*median = (temp - floor(temp)) > 0. ? temp_array[ (int) floor(temp) ] : 
				0.5 * (temp_array[ (int)(temp - 1.) ] + temp_array[ (int) temp ]) ; 



	// FIND MODE
/*
	modeFound = 0 ; 
	numhistdivisions = 100 ; 

	while(modeFound != 1){

		for(i = 0; i < numhistdivisions; i++) {
			histogramcounts[i] = 0 ; histogramdivisionsums[i] = 0. ; 
		}; 


		histdivision = (*high - *low) / (float) numhistdivisions ; 

		for(i = 0; i < numberOfValues; i++){
			for(k = 0, lowdivision = *low, highdivision = *low + histdivision; k < numhistdivisions; 
								k++, lowdivision += histdivision, highdivision += histdivision){ 
if( highdivision  == *high ) prf( highdivision, "highdivision" ) ; 

				if( lowdivision == *low ){
					if( (temp_array[i] >= lowdivision) && (temp_array[i] <= highdivision) ){
						 histogramcounts[k]++ ; histogramdivisionsums[k] += temp_array[i] ; 						
					} ; 
				}else{
					if( (temp_array[i] > lowdivision) && (temp_array[i] <= highdivision) ){
						 histogramcounts[k]++ ; histogramdivisionsums[k] += temp_array[i] ; 						
					} ; 						
				} ; 
			} ; 
		} ; 

		// SORT COUNTS
		notdone = 1 ; ii = 0 ; 
		while( notdone ){ 
			notdone = 0 ; 
			for( i = ii ; i  < (numhistdivisions - 1 - ii); i += 2 ){
		    		if( histogramcounts[i] > histogramcounts[i + 1] ){
					// SWITCH
					k = histogramcounts[i] ; histogramcounts[i] = histogramcounts[i + 1] ; histogramcounts[i + 1] = k ; 
					temp = histogramdivisionsums[i] ; histogramdivisionsums[i] = histogramdivisionsums[i + 1] ; histogramdivisionsums[i + 1] = temp ;
					notdone = 1 ; 
				} ; 
		    	}
	    		if( ii == 0 ) ii = 1 ; else ii = 0 ; 
	    	}

for(i = 0; i < numhistdivisions; i++)prf( histogramcounts[i], "histogramcounts" ); 
for(i = 0; i < numhistdivisions; i++)prf( histogramdivisionsums[i], "histogramdivisionsums" ); 

		if( (histogramcounts[numhistdivisions - 1] > histogramcounts[numhistdivisions - 2]) || (numhistdivisions <= 2) ){
			*mode = histogramdivisionsums[numhistdivisions - 1] / (float) histogramcounts[numhistdivisions - 1] ; 
			modeFound = 1 ; 
		}else{
			numhistdivisions-- ; 
		} ;  
	} ; 


	prf( *mode, "mode" ) ; 	

*/

	*mode = findMode( temp_array, numberOfValues, 0., &modeFoundFlag ) ; 

// *****

	for(i = 0; i < numberOfValues; i++) temp_array[i] = pow( value[i] - *average, 2. ) ; 
	tempsum = 0. ; 
	for(i = 0; i < numberOfValues; i++) tempsum += temp_array[i] ; 
	*standarddeviation = sqrt( tempsum / (float) numberOfValues ) ;

//prt( "h10" ) ; 




	if( printflag == 1 ){


		pri( (int) numberOfValues,  "NUMBER OF VALUES" ) ; 
		prline( WIDTH, "-" ) ; 
		fprintf( stderr, "\nLOW         HIGH        RANGE       MEAN        MEDIAN      MODE        ") ; 
		fprintf( stderr, "STD. DEV.   SUM         BEGIN        END         MIDDLE" ) ; 
		prline( WIDTH, "." ) ; 
		if( freqUnitsFlag == 0 ) fprintf( stderr, "\nFREQUENCY:" ) ;
		if( freqUnitsFlag == 1 ) fprintf( stderr, "\nOCTAVE.DECIMAL:" ) ;
		if( freqUnitsFlag == 2 ) fprintf( stderr, "\nSEMITONE DEVIATION FROM REFERENCE:" ) ;
		if( freqUnitsFlag == 3 ) fprintf( stderr, "\nINVERTED SEMITONE DEVIATION FROM REFERENCE:" ) ;
		if( freqUnitsFlag == 4 ) fprintf( stderr, "\nMIDI:" ) ;
		if( freqUnitsFlag == 5 ) fprintf( stderr, "\nOCTAVE.PITCHCLASS:" ) ;
		if( modeFoundFlag == 1 )
			fprintf( stderr, 
				"\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f",
				*low, *high, *range, *average, *median, *mode, *standarddeviation, *sum, *begin, *end, *middle 
			) ; 
		else
			fprintf( stderr, 
				"\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  **********  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f",
				*low, *high, *range, *average, *median, *standarddeviation, *sum, *begin, *end, *middle 
			) ; 
		
		if( freqUnitsFlag == 0 ){ // HERTZ
			// MIDI
			thislow = Hz_to_MIDI( *low ); 
			thishigh = Hz_to_MIDI( *high ) ;
			thisrange = thishigh - thislow ;
			thisaverage = Hz_to_MIDI( *average ); 
			thismedian = Hz_to_MIDI( *median ); 
			thismode = Hz_to_MIDI( *mode ) ; 
			thisbegin = Hz_to_MIDI( *begin ); 
			thisend = Hz_to_MIDI(*end) ; 
			thismiddle = Hz_to_MIDI(*middle) ; 
			fprintf( stderr, "\nMIDI:" ) ; 
			if( modeFoundFlag == 1 )
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  **********   **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisrange, thisaverage, thismedian, thismode, thisbegin, thisend, thismiddle  
				) ; 
			else
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  **********  **********   **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisrange, thisaverage, thismedian, thisbegin, thisend, thismiddle  
				) ; 
				

			// OCTAVE.PITCHCLASS
			thislow = Hz_to_OPPC( *low ); 
			thishigh = Hz_to_OPPC(*high) ; 
			thisaverage = Hz_to_OPPC(*average) ;
			thismedian = Hz_to_OPPC(*median) ;
			thismode =  Hz_to_OPPC(*mode) ;
			thisbegin = Hz_to_OPPC(*begin) ; 
			thisend = Hz_to_OPPC(*end) ; 
			thismiddle = Hz_to_OPPC(*middle) ; 
			fprintf( stderr, "\nOCTAVE.PITCHCLASS:" ) ; 
			if( modeFoundFlag == 1 )
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  **********  %-10.4f  %-10.4f  %-10.4f  **********  **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisaverage, thismedian, thismode, thisbegin, thisend, thismiddle  
				) ; 
			else
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  **********  %-10.4f  %-10.4f  **********  **********  **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisaverage, thismedian, thisbegin, thisend, thismiddle  
				) ; 


		} ; 

		if( freqUnitsFlag == 4 ){ // MIDI
			// FREQUENCY
			thislow = MIDI_to_Hz( *low ); 
			thishigh = MIDI_to_Hz( *high ) ;
			thisrange = thishigh - thislow ;
			thisaverage = MIDI_to_Hz( *average ); 
			thismedian = MIDI_to_Hz( *median ); 
			thismode = MIDI_to_Hz( *mode ) ; 
			thisbegin = MIDI_to_Hz( *begin ); 
			thisend = MIDI_to_Hz(*end) ; 
			thismiddle = MIDI_to_Hz(*middle) ; 
			fprintf( stderr, "\nFREQUENCY:" ) ; 
			if( modeFoundFlag == 1 )
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  **********   **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisrange, thisaverage, thismedian, thismode, thisbegin, thisend, thismiddle  
				) ; 
			else
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  **********  **********   **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisrange, thisaverage, thismedian, thisbegin, thisend, thismiddle  
				) ; 
			// OCTAVE.PITCHCLASS
			thislow = Hz_to_OPPC( MIDI_to_Hz( *low ) ); 
			thishigh = Hz_to_OPPC( MIDI_to_Hz( *high) ) ; 
			thisaverage = Hz_to_OPPC( MIDI_to_Hz( *average) ) ;
			thismedian = Hz_to_OPPC( MIDI_to_Hz( *median) ) ;
			thismode =  Hz_to_OPPC( MIDI_to_Hz( *mode) ) ;
			thisbegin = Hz_to_OPPC( MIDI_to_Hz( *begin) ) ; 
			thisend = Hz_to_OPPC( MIDI_to_Hz( *end) ) ; 
			thismiddle = Hz_to_OPPC( MIDI_to_Hz( *middle) ) ; 
			fprintf( stderr, "\nOCTAVE.PITCHCLASS:" ) ; 
			if( modeFoundFlag == 1 )
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  **********  %-10.4f  %-10.4f  %-10.4f  **********  **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisaverage, thismedian, thismode, thisbegin, thisend, thismiddle  
				) ; 
			else			
				fprintf( stderr, 
					"\n%-10.4f  %-10.4f  **********  %-10.4f  %-10.4f  **********  **********  **********   %-10.4f  %-10.4f  %-10.4f",
					thislow, thishigh, thisaverage, thismedian, thisbegin, thisend, thismiddle  
				) ; 
		} ; 



	} ; 
 
//prt( "h11" ) ; 

    return( 1  );

}

