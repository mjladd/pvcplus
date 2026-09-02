#include <stdio.h>
#include <math.h>
#include "pv.h"

int findFilterTimeAndConstrainByWindow( 
	float filttinc, 
	struct func *filttorigin, 
	struct func *filtrate,
	int Onset_and_Release_Segment_Mode__off_0__on_1,
	int Mode__sampler_loop_0__autostop_1, 
	int *autostopflag,
	int wrap_0_fold_1_clip_2,
	float *filttnow,
	float *oldfilttnow, 
	struct func *filtwinlow, 
	struct func *filtwinhi,
    	float *dur,
	float analysis_dur,
	int vibratoPeriodsUnitsRateFlag,
	float vibratoPeriodDuration
)
{
    float temp, prop, warpedProp, propFrac, winSize, NewMinusOldDiff, newDur ;
    float filtTimeLeftNow, filtTimeLeftProp ;
    int boundariesResetExitCode ; // 0: NO RESET, 1: RESET  

    static int saveFiltTimeLeftNowFlag ; 
    static float startFiltTimeLeft ; 
    int iprop ; 
    static float testDur ;
    static float remainingTime ;  
    static float remainingFiltTime ;
    static int timeDirectionNow ;
    static int endSegmentFlag ; 
    static float loopTimeDirectionInverter=1.0 ;
    static int useTimeWindowFlag ;
    static float thisFiltRate ;
	static int wrappedFlag ;
	static float previousfilttnow ;
	static float filttdiff ;       

    if( frame_count == 0 ){
	// RESET 
	loopTimeDirectionInverter = 1.0 ;
	previousfilttnow = *filttnow ; 
	endSegmentFlag = 0 ; 
	useTimeWindowFlag = 0 ; 
	filttorigin->A[ 0 ] = fval( filttorigin, *dur, 0. );
	*filttnow = filttorigin->A[ 0 ] ; // ACCUMULATED FILTER TIME POINT IN SECONDS
	saveFiltTimeLeftNowFlag = 1 ;  
   }; 
	
	wrappedFlag = 0 ; 

	*oldfilttnow = *filttnow ; //INITIALIZE OLD FILTER TIME POINT IN SECONDS


    // INTIALIZE RESET TO 0
    boundariesResetExitCode = 0 ; 

// ********** INCREMENT FILTER TIME TO NEXT FRAME
    // GET NEW RATE
    thisFiltRate = filtrate->A[ 0 ] = fval( filtrate, *dur, t );
//prf( thisFiltRate, "BEFORE: thisFiltRate" ) ; 

    // RESCALE RATE FROM VIBRATO PERIODS TO SECONDS
//prf( vibratoPeriodDuration, "BEFORE: vibratoPeriodDuration" ) ; 
    if( vibratoPeriodsUnitsRateFlag == 1 ) thisFiltRate *= vibratoPeriodDuration ; 

//prf( thisFiltRate, "AFTER: thisFiltRate" ) ; 

    // SAVE OLD ORIGIN
    temp = filttorigin->A[ 0 ] ; 
    // GET NEW ORIGIN
    filttorigin->A[ 0 ] = fval( filttorigin, *dur, t );
    // GET CHANGE IN ORIGIN
    temp = filttorigin->A[ 0 ] - temp ;

    // SUMMED CHANGE
    temp =  (temp  + (filttinc * thisFiltRate )) ; 

    // ADD CHANGE IN ORIGIN AND CURRENT RATE, INVERTED BY LOOP, TO NEXT FRAME POSITION
    *filttnow += ( temp * loopTimeDirectionInverter ) ;

    // DIRECTION
    if( temp >= 0.0 ) timeDirectionNow = 1 ; else timeDirectionNow = -1 ; 

    // *******

    // SAVE DIFFERENCE BETWEEN NEW AND OLD FILT TIME
    NewMinusOldDiff = *filttnow - *oldfilttnow ; 
 
    // TIME DIRECTION NOW

    // GET LOW/HI WINDOW BOUNDARIES
    filtwinlow->A[ 0 ] = fval( filtwinlow, *dur, t );
    filtwinhi->A[ 0 ] = fval( filtwinhi, *dur, t );

    // CONSTRAIN BOUNDARIES TO ANALYSIS TIME (0 TO ANALYSIS DURATION).
    if(filtwinlow->A[ 0 ] < 0.)filtwinlow->A[ 0 ] = 0. ; 
    if(filtwinlow->A[ 0 ] > analysis_dur)filtwinlow->A[ 0 ] = analysis_dur ;  
    if(filtwinhi->A[ 0 ] > analysis_dur)filtwinhi->A[ 0 ] = analysis_dur ; 
    if(filtwinhi->A[ 0 ] < 0.)filtwinhi->A[ 0 ] = 0. ;
 

  
    // SWITCH LOW AND HIGH IF THEY HAVE CROSSED.
    if( filtwinhi->A[ 0 ] < filtwinlow->A[ 0 ] ){
        temp = filtwinhi->A[ 0 ] ; filtwinhi->A[ 0 ] = filtwinlow->A[ 0 ] ; filtwinlow->A[ 0 ] = temp ;
    } ; 

    // IF NOT IN WINDOW.......THEN
    if( (useTimeWindowFlag != 1) && ( endSegmentFlag != 1 ) ){
    	// IF FILT TIME IS NOW INSIDE WINDOW (TRIGGER) OR.....
    	// ONSET/RELEASE MODE IS OFF, THEN SET FLAG TO USE WINDOW. (ALWAYS) 

    	if( 
	    ( (*filttnow > filtwinlow->A[ 0 ]) && (*filttnow < filtwinhi->A[ 0 ]) ) 
    		||
	    ( Onset_and_Release_Segment_Mode__off_0__on_1 == 0 ) 

	){
	    useTimeWindowFlag = 1 ; 
	    prf( t, "* LOOPED AT" ) ; 
	    fprintf( stderr, "\n\tLOOP BOUNDARIES: %f <-> %f  ", filtwinlow->A[ 0 ], filtwinhi->A[ 0 ] ) ; 

	} ; 

    } ; 

	// IF IN LOOP THAT USES ONSET AND RELEASE...
    if( (useTimeWindowFlag == 1) && // IN WINDOW
	    (Onset_and_Release_Segment_Mode__off_0__on_1 == 1) && // ONSET/RELEASE MODE
		(Mode__sampler_loop_0__autostop_1 == 0) &&
		    (loopTimeDirectionInverter == 1.0) && // TIME IS NOT REVERSED FROM LOOP FOLDING 
		    	(endSegmentFlag != 1)
    ){
		// TEST FOR START OF RELEASE 
		if( timeDirectionNow == -1) testDur = filtwinhi->A[ 0 ] ;
		else testDur = analysis_dur - filtwinlow->A[ 0 ] ;
		if( wrap_0_fold_1_clip_2 == 1 ) testDur += (filtwinhi->A[ 0 ] - filtwinlow->A[ 0 ]) ;  

		remainingTime = *dur - t ; 
//fprintf( stderr, "remainingTime: %f\t testDur: %f\n", remainingTime, testDur ) ; 
		if( remainingTime <= testDur ){
	    		// LOOP WINDOW OFF -- EFFECTIVE RELEASE MODE TO END (OR BEGINNING WHEN TIME IS REVERSED).
	    		useTimeWindowFlag = 0 ; endSegmentFlag = 1 ;  boundariesResetExitCode = -1 ;
	    		prf( t, "* RELEASED AT" ) ; 
		};  
    } ; 
	
    if( (useTimeWindowFlag == 1) && ( endSegmentFlag != 1 ) ){

	// TAKE AUTO-STOP ROUTE IF ON.
		if( Mode__sampler_loop_0__autostop_1 == 1 ) {
	    		// ON
	    		if( (*filttnow > filtwinhi->A[ 0 ]) || (*filttnow < filtwinlow->A[ 0 ]) ){
				// STOP
				*autostopflag = 1 ;
//	prt( "\n\n\t========>  WARNING! <=========\n") ;
//	prt( "AUTOSTOP EXIT: CONTROL FUNCTIONS WILL NOT COMPLETE THEIR DURATION.\n" ) ; 
	    	} ; 
	} else {
	    // WRAP, FOLD, OR CLIP TIME.
	    if( wrap_0_fold_1_clip_2 == 0 ){ // WRAP
	        winSize = filtwinhi->A[ 0 ] - filtwinlow->A[ 0 ] ; 
			wrappedFlag = 0. ; 
			filttdiff = *filttnow - previousfilttnow ; 

	        while( *filttnow > filtwinhi->A[ 0 ] ){ 
				*filttnow -= winSize ; 
				wrappedFlag = 1 ; 
                 	boundariesResetExitCode = 1 ; 
		   } ; 
		   if( (wrappedFlag == 1) && (filttdiff > 0.) ) {
				prf( t, "* <-- WRAPPED AT" ) ; 
		   } ; 			
	        while( *filttnow < filtwinlow->A[ 0 ] ){ 
				*filttnow += winSize ; 
				wrappedFlag = 1 ; 
                 	boundariesResetExitCode = 1 ; 
		   } ; 
		   if( (wrappedFlag == 1) && (filttdiff < 0.) ) {
				prf( t, "* --> WRAPPED AT" ) ; 
		   } ; 			

	    } else if( wrap_0_fold_1_clip_2 == 1 ){ // FOLD
			winSize = filtwinhi->A[ 0 ] - filtwinlow->A[ 0 ] ; 
			if( *filttnow > filtwinhi->A[ 0 ] ){
		    		prop = (*filttnow - filtwinhi->A[ 0 ]) / (2.0 * winSize) ;
	    	    		iprop = (int) prop ; 
		    		propFrac = prop - (float) iprop ;
		    		*filttnow = filtwinlow->A[ 0 ] + 
				(winSize * fabs( (float) (-1.0 + (propFrac * 2.0)))) ;
		    		if( (-1.0 + (propFrac * 2.0)) < 0.0 ) {
					 if( loopTimeDirectionInverter == 1.0 ) prf( t, "* <-- * FOLDED AT" ) ;
					loopTimeDirectionInverter = -1.0 ; // *= -1.0 ;
                 		boundariesResetExitCode = 1 ; 
		    		} ;  
		    		*oldfilttnow = *filttnow - NewMinusOldDiff ;
			} ; 
			if( *filttnow < filtwinlow->A[ 0 ] ){
		    		prop = (filtwinlow->A[ 0 ] - *filttnow) / (2.0 * winSize) ;
		    		iprop = (int) prop ; 
		    		propFrac = prop - (float) iprop ;
		    		*filttnow = filtwinhi->A[ 0 ] - 
					(winSize * fabs( (float) (1.0 - (propFrac * 2.0)))) ; 
		    		if( (1.0 - (propFrac * 2.0)) > 0.0 ){ 
					if( loopTimeDirectionInverter == -1.0 ) prf( t, "* --> *  FOLDED AT" ) ; 
					loopTimeDirectionInverter = 1.0 ;  // *= -1.0 ;
                 		boundariesResetExitCode = 1 ; 
		    		} ;  
		    		*oldfilttnow = *filttnow - NewMinusOldDiff ;
			} ; 

	    } else if( wrap_0_fold_1_clip_2 == 2 ){ // CLIP
			if( *filttnow > filtwinhi->A[ 0 ] ){
		    		*filttnow = filtwinhi->A[ 0 ] ; 
		    		*oldfilttnow = *filttnow - NewMinusOldDiff ; 
			} ; 
			if( *filttnow < filtwinlow->A[ 0 ] ){
		    		*filttnow = filtwinlow->A[ 0 ] ;
		    		*oldfilttnow = *filttnow - NewMinusOldDiff ; 
			} ; 
	    } else {
			pri( wrap_0_fold_1_clip_2, "ILLEGAL WRAP-FOLD-OR-CLIP FLAG. FLAG " ) ; 
			exit(-1); 
	    } ; 
		previousfilttnow = *filttnow ; 
	} ;  
   } ; 

    // CONSTRAIN TIME TO ANALYSIS TIME (0 TO ANALYSIS DURATION).
    if( *filttnow > analysis_dur ) { *filttnow = analysis_dur ; *oldfilttnow = analysis_dur ; } ;  
    if( *filttnow < 0. ) { *filttnow = 0. ; *oldfilttnow = 0. ; } ;  

    // IF IN RELEASE MODE, CHANGE DURATION TO LEAVE AT LEAST AS MUCH TIME AS 
    // NEEDED TO REACH RESPECTIVE END OR BEGIN. 

    if( endSegmentFlag == 1 ){
		if( timeDirectionNow == 1 ){
			// FORWARD RELEASE TO END
	    		filtTimeLeftNow = analysis_dur - *filttnow ; 
	    		if( saveFiltTimeLeftNowFlag == 1 ) {
				startFiltTimeLeft = filtTimeLeftNow ; saveFiltTimeLeftNowFlag = 0 ; 
	    		} ; 
	    		filtTimeLeftProp = (1.0 - (filtTimeLeftNow / startFiltTimeLeft)) ;
	    		filtTimeLeftProp = curve(0., 1., filtTimeLeftProp, 7) ; 
	    		*dur = *dur + (filtTimeLeftProp * ((analysis_dur - *filttnow) - (*dur - t))) ; 
		} ; 
		if( timeDirectionNow == -1 ){
			// BACKWARD RELEASE TO BEGIN
	    		filtTimeLeftNow = *filttnow ; 
	    		if( saveFiltTimeLeftNowFlag == 1 ) {
				startFiltTimeLeft = filtTimeLeftNow ; saveFiltTimeLeftNowFlag = 0 ; 
	    		} ; 
	    		filtTimeLeftProp = 1.0 - (filtTimeLeftNow / startFiltTimeLeft) ; 
	    		filtTimeLeftProp = curve(0., 1., filtTimeLeftProp, 7) ; 
	    		*dur = *dur + (filtTimeLeftProp * (*filttnow - (*dur - t))) ; 
		} ; 

    } ; 

    
    // SET TO STOP IF IN RELEASE STAGE AND HAVE REACHED BEGINNING OR END.
    if( (endSegmentFlag == 1) && 
	    ((*filttnow == 0.) || (*filttnow == analysis_dur)) &&
		(ringTime == 0.0)	 
    ){
		*autostopflag = 1 ; 
	}; 

    return( boundariesResetExitCode ) ; 


}
