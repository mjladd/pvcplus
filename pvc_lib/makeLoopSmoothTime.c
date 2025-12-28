#include <stdio.h>
#include <math.h>
#include "pv.h"

float makeLoopSmoothTime (
    float filttnow,
    int wrap_0_fold_1_clip_2,
    int Mode__sampler_loop_0__autostop_1, 
    float dur,  
    struct func *filtwinlow,    
    struct func *filtwinhi,
    struct func *peakLoopSmoothTime
){
    float halfLoopTime, loopSmoothTime, thisPeakLoopSmoothTime ; 


    if( Mode__sampler_loop_0__autostop_1 == 0 ){
	    thisPeakLoopSmoothTime  = peakLoopSmoothTime->A[ 0 ] =  fval( peakLoopSmoothTime, dur, t );

		// MAKE SMOOTH VALUES FOR LOOP ENDS
		if( (wrap_0_fold_1_clip_2 != 2) && 
	    		(filttnow >= (filtwinlow->A[ 0 ] - thisPeakLoopSmoothTime) ) && 
	        		(filttnow <= (filtwinhi->A[ 0 ] + thisPeakLoopSmoothTime) ) ){
	
	    		halfLoopTime = 0.5 * fabs(filtwinhi->A[ 0 ] - filtwinlow->A[ 0 ]) ;
	    		loopSmoothTime = thisPeakLoopSmoothTime - 
			    fabs(halfLoopTime -  
				fabs( (filttnow - filtwinlow->A[ 0 ]) - halfLoopTime ) ) ; 
	    		if( loopSmoothTime < 0.0 ) loopSmoothTime = 0.0  ;
		}else{
			loopSmoothTime = 0.0 ;
		} ; 
    }else{
		loopSmoothTime = 0.0 ; 
    }; 
    return( loopSmoothTime ) ; 
} ; 
