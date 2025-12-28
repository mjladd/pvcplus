#include <stdio.h>
#include <math.h>
#include "pv.h"

#define WIDTH 132


void findArrayStats(
	float v[],
	int *numberOfValues,
	float *low,  
	float *hi,  
	float *range,  
	float *average,
	float *median, 
	float *mode, 
	float *standarddeviation,
	float *sum,
	float *begin, 
	float *end,
	float *middle, 
	int printflag
){
	int i, notdone, ii, modeFoundFlag ;
	float *temp_array, temp, tempsum ;

	fvec( temp_array, *numberOfValues ) ;  
	
	*sum = 0. ; 
	for(i = 0; i < *numberOfValues; i++){
		if(i == 0){
			*low = v[i] ;
			*hi = v[i] ;  
		}else{
			if(v[i] < *low) *low = v[i] ; 
			if(v[i] > *hi) *hi = v[i] ; 
		}; 
		*sum += v[i]; 
	}; 
	*average = *sum / (float) *numberOfValues ;  
	*begin = v[0] ; *end = v[ *numberOfValues - 1] ; *middle = v[ *numberOfValues / 2 ] ;  
	*range = *hi - *low ;  

	for(i = 0; i < *numberOfValues; i++)temp_array[i] = v[i] ; 

	notdone = 1 ; ii = 0 ; 

	while( notdone ){ 
		notdone = 0 ; 
		for( i = ii ; i  < (*numberOfValues - 1); i += 2 ){
		    	if( temp_array[i] > temp_array[i + 1] ){
				// SWITCH
				temp = temp_array[i] ; temp_array[i] = temp_array[i + 1] ; temp_array[i + 1] = temp ; 
				notdone = 1 ; 
		    	}
	    	}
	    
	    	if( ii == 0 ) ii = 1 ; else ii = 0 ; 
	} ; 

	temp = (float) *numberOfValues / 2. ;
	*median = (temp - floor(temp)) > 0. ? temp_array[ (int) floor(temp) ] : 
			0.5 * (temp_array[ (int)(temp - 1.) ] + temp_array[ (int) temp ]) ; 

	*mode = findMode( temp_array, *numberOfValues, 0., &modeFoundFlag ) ; 


	for(i = 0; i < *numberOfValues; i++) temp_array[i] = pow( v[i] - *average, 2. ) ; 
	tempsum = 0. ; 
	for(i = 0; i < *numberOfValues; i++) tempsum += temp_array[i] ; 
	*standarddeviation = sqrt( tempsum / (float) *numberOfValues ) ;

	if( printflag == 1 ){
//		prbanner( "STATISTICS ", WIDTH ) ;
		pri( (int) *numberOfValues,  "NUMBER OF VALUES" ) ; 
		prline( WIDTH, "-" ) ; 
		fprintf( stderr, "\nLOW         HIGH        RANGE       MEAN        MEDIAN      MODE        ") ; 
		fprintf( stderr, "STD. DEV.   SUM         BEGIN        END         MIDDLE" ) ; 
		prline( WIDTH, "." ) ; 
		if( modeFoundFlag == 1 )
			fprintf( stderr, "\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f",
				*low, *hi, *range, *average, *median, *mode, *standarddeviation, *sum, *begin, *end, *middle ) ; 
		else
			fprintf( stderr, "\n%-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f  **********  %-10.4f  %-10.4f  %-10.4f  %-10.4f  %-10.4f",
				*low, *hi, *range, *average, *median, *standarddeviation, *sum, *begin, *end, *middle ) ; 

	} ; 
 

}; 
