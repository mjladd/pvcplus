#include <stdio.h>
#include <math.h>
#include "pv.h"

void normalizeLoopAmplitudes(
    int LoopNormalizationFlag,
    int Mode__sampler_loop_0__autostop_1, 
    struct func *filtwinlow, 
    struct func *filtwinhi,
    struct func *filter,
    float F_lower[], 
    float F_higher[],
    float tempChannel[],
    float channel[],
    float iframes_per_sec,
    int analysis_N,
    int ainchan,
    int analysis_chan,
    float analysis_dur, 
    float filttnow
)
{
    static int firstLowBoundaryChannelAmp ; 
    static int firstHighBoundaryChannelAmpSum ;
    static int findNewPeakBoundaryAmpSum ; 
    static float lowBoundaryAmpSum ; 
    static float highBoundaryAmpSum ; 
    static float peakBoundaryAmpSum ; 
    static float channelAmpSum ;
    static float prop ;   
    static float lowGainScaleInDB ; 
    static float highGainScaleInDB ;    
    static float gainScale ;
    static int i ;
    static float last_filtwinlow=-1. ;
    static float last_filtwinhi=-1. ;


    if( t == 0. ){
	firstLowBoundaryChannelAmp = 1 ; 
	firstHighBoundaryChannelAmpSum = 1 ; 
	findNewPeakBoundaryAmpSum = 1 ;
  }; 

	if( (Mode__sampler_loop_0__autostop_1 == 0) && (LoopNormalizationFlag == 1) ){
		// NORMALIZATION ON
		// FIND BOUNDARY AMP SUMS
		if( (firstLowBoundaryChannelAmp == 1) || (filtwinlow->A[ 0 ] != last_filtwinlow)){
//fprintf( stderr, "RESETTING IN LOOP NORMALIZATION AT %f\n", t ) ; 
	    		// GET FRAME FOR LOW BOUNDARY
	    		makeInterpolatedFilterFrame( filter, F_lower, F_higher, tempChannel,
				iframes_per_sec, analysis_N, filtwinlow->A[ 0 ], ainchan, analysis_chan
	    			) ; 
	    		// MAKE LOW BOUNDARY AMP SUM.
	    		lowBoundaryAmpSum = 0.0 ; 
	    		for(i = 0; i < analysis_N; i += 2) lowBoundaryAmpSum += tempChannel[i] ; 
	    		if( filtwinlow->n == 1 )  firstLowBoundaryChannelAmp = 0 ; 
	    		findNewPeakBoundaryAmpSum = 1 ; 
		}; 
		if( (firstHighBoundaryChannelAmpSum == 1) || (filtwinhi->A[ 0 ] != last_filtwinhi) ){
//fprintf( stderr, "RESETTING IN LOOP NORMALIZATION AT %f\n", t ) ; 
			// GET FRAME FOR HIGH BOUNDARY
	    		makeInterpolatedFilterFrame( filter, F_lower, F_higher, tempChannel,
				iframes_per_sec, analysis_N, filtwinhi->A[ 0 ], ainchan, analysis_chan
	    			) ; 
	    		// MAKE HIGH BOUNDARY AMP SUM.
	    		highBoundaryAmpSum = 0.0 ; 
	    		for(i = 0; i < analysis_N; i += 2) highBoundaryAmpSum += tempChannel[i] ; 
	    		if( filtwinhi->n == 1 ) firstHighBoundaryChannelAmpSum = 0 ; 
	    		findNewPeakBoundaryAmpSum = 1 ; 
		}; 
		if( findNewPeakBoundaryAmpSum == 1){
	    		if( lowBoundaryAmpSum > highBoundaryAmpSum ) peakBoundaryAmpSum = lowBoundaryAmpSum ; 
	    		else peakBoundaryAmpSum = highBoundaryAmpSum ; 
	    		findNewPeakBoundaryAmpSum = 0 ; 
		}; 


		// FIND AMP SUM
		channelAmpSum = 0.0 ; 
		for( i = 0; i < analysis_N; i += 2) channelAmpSum += channel[i]; 

		// NORMALIZE
		if( channelAmpSum > 0.0 ){
			if( filttnow <= filtwinlow->A[ 0 ] ){
	     		// ONSET SEGMENT
	        		prop = filttnow / filtwinlow->A[ 0 ] ; 
	        		lowGainScaleInDB = 0.0 ; 
				highGainScaleInDB = amp_to_dB( peakBoundaryAmpSum / lowBoundaryAmpSum )  ; 
	        		gainScale = 
					dB_to_amp( lowGainScaleInDB + (prop * (highGainScaleInDB - lowGainScaleInDB)) ) ; 
	    		}else if( filttnow >= filtwinhi->A[ 0 ] ){
	        		// RELEASE SEGMENT
	        		prop = ((filttnow - filtwinhi->A[ 0 ]) / (analysis_dur - filtwinhi->A[ 0 ])) ; 
	        		lowGainScaleInDB = amp_to_dB( peakBoundaryAmpSum / highBoundaryAmpSum ) ; 
				highGainScaleInDB = 0.0 ; 
	        		gainScale = 
					dB_to_amp( lowGainScaleInDB + (prop * (highGainScaleInDB - lowGainScaleInDB)) ) ; 
	    		}else{
	        		// LOOP
	        		gainScale = peakBoundaryAmpSum / channelAmpSum ; 
	    		}; 
	
	    		for( i = 0; i < analysis_N; i += 2) channel[i] *= gainScale ; 
		}; 
    };     

	last_filtwinlow = filtwinlow->A[ 0 ] ; last_filtwinhi = filtwinhi->A[ 0 ] ; 

} ; 
