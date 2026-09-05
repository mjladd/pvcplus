#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,  i1,  i2;
float i1p,  i2p ; 
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int analysis_N, analysis_N2,   analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  achannelout=0 ;
int pitchflag=0; 
 float P = 1.0;
  FILE *fopen();
char ch;

float funcMin, funcMax, funcAvg, maxDelayT=0., timeRange, thisDelayT, 
	interpTdelayFilterAmp ; 
int maxNumOfDelayFrames, frameNowChannelDelayIndex, thisFrameDelay; 

float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float *previous_channel_filter, *channel_filter, *F, *Ffreq,  *previousF,  *tempF, *FtimeDelay, *channel_delay,
		*F_lower,  *F_higher,  *previous_ranfreqv ; 
float threshfac = .001,  threshfacdB=-60 ;
float	pm, fm, fs,  gain=1.;
double ar_dB ; 
float  temp,  temp2,  temp3,  temp4 ;  
float getthresh();
float normamp[MAXIMUM_CHANNELS] ;
 
float   IR,  dur=0.;
float fundamental ;
 
float analysis_fundamental, N_ratio,  sourceamp,  filtamp ;  
int bandrejecton=0 ; 
int ainchan ; 
// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 

float  ranfreqsmoothc,  minusranfreqsmoothc ; 
int ranfreqdevswitch=0 ; 

float fdmdiff,   fsdiff,  fdevminus ; 

int Mode__sampler_loop_0__autostop_1=0,  autostopflag=0,  wrap_0_fold_1_clip_2=0, 
	LoopNormalizationFlag=0, Onset_and_Release_Segment_Mode__off_0__on_1=0   ; 
float loopSmoothTime ; 

float releasec,  minusreleasec,  attackc,  minusattackc ; 
float freleasec,  minusfreleasec,  fattackc,  minusfattackc ; 


//** FILTER VARIABLES
float filttnow=0., oldfilttnow,  filttinc; 
 


char tempstring[ STRING_SIZE ] ; 
char	crackTempString[ STRING_SIZE ]="EMPTY\0" ; 


// PEAK LOOP SMOOTH TIME
struct func peakLoopSmoothTime ; 


// FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 

//  BIN AMP SOURCE dB
struct  func  sourcedB ; 

//  RELEASE
struct  func  release ; 

//  ATTACK
struct  func  attack ; 

// FILTER
struct  func  filter ; 

// ****
//  FILTER  RATE
struct  func  filtrate ; 

// FILTER TIME POINT ORIGIN
struct  func  filttorigin ; 

// FILTER TIME WINDOW LOWER BOUNDARY
struct  func  filtwinlow ; 

// FILTER TIME WINDOW UPPER BOUNDARY
struct  func  filtwinhi ; 

//  FILTER COMPRESSION THRESHOLD
struct  func  filtcompthresh ;
float    filtcompthreshamp; 
int compflag=0 ; 

//  FILTER DECIBELS OF COMPRESSION 
struct  func  filtcompdecibels ;
float  filtcompamp,  filtcompnormamp ; 
// ****

//  FILTER EXPANSION THRESHOLD IN DB
struct  func  expthreshdB ;

//  FILTER EXPANSION THRESHOLD IN DB
struct  func  expdB ;
 



// FILTER TRANSPOSE IN SEMITONES
struct  func  ftrans ; 

// FILTER  SHIFTER
struct  func  fshift ; 

//  FILTER RELEASE
struct  func  frelease ; 

//  FILTER ATTACK
struct  func  fattack ; 

//  FILTER WARP SHAPE
struct  func  warpshape ; 

//  FILTER SMOOTHING
struct  func  smoothingBW ; 

//  FILTER DEVIATION BASE
struct  func  fdevbase ;
//  FILTER DEVIATION PEAK
struct  func  fdevpeak ;

// **** RANDOM FREQ DEVIATION RESPONSE TIME
struct  func  ranfreqdevresponse ; 

//  FILTER DEVIATION SHIFT BASE 
struct  func  fshiftbase ;
//  FILTER DEVIATION SHIFT PEAK
struct  func  fshiftpeak ;
//  FILTER DEVIATION WARP SHAPE
struct  func  freqwarpshape ;
//  FILTER DEVIATION MASTER CONTROL
struct  func  fdevcontrol ;


// **** FREQ DEVIATION MODE
struct  func  freqdevmode ; 

// **** TIME DELAY: MASTER DEVIATION CONTROL
struct  func  FILTER_OUTPUT_timeDelayDevControl ; 

// TIME DELAY BASE
struct func FILTER_OUTPUT_timeDelayBase ; 

// TIME DELAY PEAK
struct func FILTER_OUTPUT_timeDelayPeak ; 

//  FILTER TIME DELAY DEVIATION WARP SHAPE
struct  func  timedelaywarpshape ;



//*****************INITIALIZE

// PEAK LOOP SMOOTH TIME
peakLoopSmoothTime.L = 1. ; peakLoopSmoothTime.n = 1. ; peakLoopSmoothTime.A[ 0 ] = 0.2 ; 



// COMB FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH MULTIPLIER
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 

//  SOURCE DECIBELS
sourcedB.L = 1. ; sourcedB.n = 1. ; sourcedB.A[ 0 ] = -96. ; 

//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 


// FILTER
filter.L = 1. ; filter.n = 0. ; filter.A[ 0 ] = 0. ; 

// *****
//  FILTER  RATE
filtrate.L = 1. ; filtrate.n = 1. ; filtrate.A[ 0 ] = 1. ; 

// FILTER TIME ORIGIN
filttorigin.L = 1. ; filttorigin.n = 1. ; filttorigin.A[ 0 ] = 0. ; 

// FILTER TIME WINDOW LOWER BOUNDARY
filtwinlow.L = 1. ; filtwinlow.n = 1. ; filtwinlow.A[ 0 ] = 0. ; 

// FILTER TIME WINDOW UPPER BOUNDARY
filtwinhi.L = 1. ; filtwinhi.n = 1. ; filtwinhi.A[ 0 ] = -1. ; 

// *****
// FILTER TRANSPOSE MULTIPLIER
ftrans.L = 1. ; ftrans.n = 1. ; ftrans.A[ 0 ] = 0. ; 

// FILTER TRANSPOSE SHIFTER
fshift.L = 1. ; fshift.n = 1. ; fshift.A[ 0 ] = 0. ; 

//  FILTER RELEASE
frelease.L = 1. ; frelease.n = 1. ; frelease.A[ 0 ] = 0. ; 

//  FILTER ATTACK
fattack.L = 1. ; fattack.n = 1. ; fattack.A[ 0 ] = 0. ; 

//  FILTER WARP SHAPE
warpshape.L = 1. ; warpshape.n = 1. ; warpshape.A[ 0 ] = 0. ; 

//  FILTER SMOOTHING
smoothingBW.L = 1. ; smoothingBW.n = 1. ; smoothingBW.A[ 0 ] = 0. ; 

//  FILTER DEVIATION BASE
fdevbase.L = 1. ; fdevbase.n = 1. ; fdevbase.A[ 0 ] = 0. ; 

//  FILTER DEVIATION PEAK
fdevpeak.L = 1. ; fdevpeak.n = 1. ; fdevpeak.A[ 0 ] = 0. ; 

//  FILTER DEVIATION SHIFT BASE 
fshiftbase.L = 1. ; fshiftbase.n = 1. ; fshiftbase.A[ 0 ] = 0. ; 

//  FILTER DEVIATION SHIFT PEAK
fshiftpeak.L = 1. ; fshiftpeak.n = 1. ; fshiftpeak.A[ 0 ] = 0. ; 

//  FILTER DEVIATION WARP SHAPE
freqwarpshape.L = 1. ; freqwarpshape.n = 1. ; freqwarpshape.A[ 0 ] = 0. ; 

//  FILTER DEVIATION MASTER CONTROL
fdevcontrol.L = 1. ; fdevcontrol.n = 1. ; fdevcontrol.A[ 0 ] = 0. ; 

// **** FREQ DEVIATION MODE
freqdevmode.L = 1. ; freqdevmode.n = 1. ; freqdevmode.A[ 0 ] = 0. ; 

// **** RANDOM FREQ DEVIATION RESPONSE TIME
ranfreqdevresponse.L = 1. ; ranfreqdevresponse.n = 1. ; ranfreqdevresponse.A[ 0 ] = 0. ; 

// **** TIME DELAY: MASTER DEVIATION CONTROL
FILTER_OUTPUT_timeDelayDevControl.L = 1. ; FILTER_OUTPUT_timeDelayDevControl.n = 1. ; FILTER_OUTPUT_timeDelayDevControl.A[ 0 ] = 1. ; 


// TIME DELAY BASE
FILTER_OUTPUT_timeDelayBase.L = 1. ; FILTER_OUTPUT_timeDelayBase.n = 1. ; FILTER_OUTPUT_timeDelayBase.A[ 0 ] = 0. ; 

// TIME DELAY PEAK
FILTER_OUTPUT_timeDelayPeak.L = 1. ; FILTER_OUTPUT_timeDelayPeak.n = 1. ; FILTER_OUTPUT_timeDelayPeak.A[ 0 ] = 0. ; 

//  FILTER TIME DELAY DEVIATION WARP SHAPE
timedelaywarpshape.L = 1. ; timedelaywarpshape.n = 1. ; timedelaywarpshape.A[ 0 ] = 0. ; 

//  FILTER COMPRESSION THRESHOLD
filtcompthresh.L = 1. ; filtcompthresh.n = 1. ; filtcompthresh.A[ 0 ] = 0. ; 

//  FILTER DECIBELS OF COMPRESSION 
filtcompdecibels.L = 1. ; filtcompdecibels.n = 1. ; filtcompdecibels.A[ 0 ] = 0. ; 

//  FILTER EXPANSION THRESHOLD IN DB
expthreshdB.L = 1. ; expthreshdB.n = 1. ; expthreshdB.A[ 0 ] = -200. ; 

//  FILTER EXPANSION THRESHOLD IN DB
expdB.L = 1. ; expdB.n = 1. ; expdB.A[ 0 ] = 0. ; 

strcpy( routine, "tvfiltdeviator" ) ; 


if( argc < 2 )usage() ; 

    while( (ch= crack( argc, argv, 
    "/|@|!|:|~|_|=|a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|W|x|X|y|Y|z|Z|", //
     0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
// NEW
   case '/':	strcpy( crackTempString, arg_option) ; 
                switch( crackTempString[0] )
                {  // a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|
                   // r|R|s|S|t|T|u|U|v|V|w|X|x|y|Y|z|Z|_|1|2|3|4|5|6|7|8|9|~|=|@|!|:|/|0|


	    			case '/':   strcpy(tempstring, arg_option);
					timedelaywarpshape.fp = crackstring( tempstring, 
			    			&timedelaywarpshape );
					break;
                   case 'a': strcpy(tempstring, arg_option);
                             FILTER_OUTPUT_timeDelayBase.fp = 
		        crackstring( &crackTempString[1], & FILTER_OUTPUT_timeDelayBase);  
                             break;



                } ;
                break;

// END NEW






	    case 'N':   N = (int) atoi(arg_option); // crackfloat( arg_option, ch )
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'I':   tfactor = crackfloat( arg_option, ch );
			break;

	    case 'B':   pitchflag = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'u':   LoopNormalizationFlag = (int) crackfloat( arg_option, ch ) ;
			break;

	    case '~':   strcpy(tempstring, arg_option);
			peakLoopSmoothTime.fp = crackstring( tempstring, 
			    &peakLoopSmoothTime );
			break;


	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'K':   achannelout = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'H':   dBlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'X':   dBhi = crackfloat( arg_option, ch ) ;
			break;
	    case 'm':   freqlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'R':   freqhi = crackfloat( arg_option, ch ) ;
			break;

           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;

	    case 'd':   Mode__sampler_loop_0__autostop_1 = (int) crackfloat( arg_option, ch ) ;
			break;



            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;

	    case 'P':   strcpy(tempstring, arg_option);
			ptrans.fp = crackstring( tempstring, &ptrans ); 
			break;
	    case 'a':   strcpy(tempstring, arg_option);
			harmadd.fp = crackstring( tempstring, 
			    &harmadd );
			break;
	    case 'A':   strcpy(tempstring, arg_option);
			dBgain.fp = crackstring( tempstring, 
			    &dBgain );
			break;

	    case 'F':   strcpy(tempstring, arg_option);
			filter.fp = crackstring_bin_only( tempstring, 
			    &filter );
			break;
 
            case 'q':	bandrejecton = (int) crackfloat( arg_option, ch ) ; break;


	    case 'f':   strcpy(tempstring, arg_option);
			smoothingBW.fp = crackstring( tempstring, 
			    &smoothingBW );
			break;

// *****
	    case 'Q':   strcpy(tempstring, arg_option);
			filttorigin.fp = crackstring( tempstring, 
			    &filttorigin );
			break;
	    case 'Y':   strcpy(tempstring, arg_option);
			filtrate.fp = crackstring( tempstring, 
			    &filtrate );
			break;

	    case 'g':   strcpy(tempstring, arg_option);
			filtwinlow.fp = crackstring( tempstring, 
			    &filtwinlow );
			break;
	    case 'G':   strcpy(tempstring, arg_option);
			filtwinhi.fp = crackstring( tempstring, 
			    &filtwinhi );
			break;

	    case 'v':	Onset_and_Release_Segment_Mode__off_0__on_1 = (int) crackfloat( arg_option, ch );
			break;
	    case 'x':	wrap_0_fold_1_clip_2 = (int) crackfloat( arg_option, ch );
			break;

	    case 'E':   strcpy(tempstring, arg_option);
			filtcompthresh.fp = crackstring( tempstring, 
			    &filtcompthresh );
			break;
	    case 'c':   strcpy(tempstring, arg_option);
			filtcompdecibels.fp = crackstring( tempstring, 
			    &filtcompdecibels );
			break;




// *****

	    case 'T':   strcpy(tempstring, arg_option);
			ftrans.fp = crackstring( tempstring, 
			    &ftrans );
			break;
	    case 'V':   strcpy(tempstring, arg_option);
			fshift.fp = crackstring( tempstring, 
			    &fshift );
			break;
	    case 'Z':   strcpy(tempstring, arg_option);
			frelease.fp = crackstring( tempstring, 
			    &frelease );
			break;
	    case 'z':   strcpy(tempstring, arg_option);
			fattack.fp = crackstring( tempstring, 
			    &fattack );
			break;

   //***********


	    case 'h':   strcpy(tempstring, arg_option);
			fdevbase.fp = crackstring( tempstring, 
			    &fdevbase );
			break;
	    case 'j':   strcpy(tempstring, arg_option);
			fdevpeak.fp = crackstring( tempstring, 
			    &fdevpeak );
			break;



	    case 'U':	strcpy(tempstring, arg_option);
			freqdevmode.fp = crackstring( tempstring, 
			    &freqdevmode );
			break;
	    case 'y':    strcpy(tempstring, arg_option);
			ranfreqdevresponse.fp = crackstring( tempstring, 
			    &ranfreqdevresponse );
			break;

	    case 'J':   strcpy(tempstring, arg_option);
			fshiftbase.fp = crackstring( tempstring, 
			    &fshiftbase );
			break;
	    case 'k':   strcpy(tempstring, arg_option);
			fshiftpeak.fp = crackstring( tempstring, 
			    &fshiftpeak );
			break;

	    case 'n':   strcpy(tempstring, arg_option);
			freqwarpshape.fp = crackstring( tempstring, 
			    &freqwarpshape );
			break;

	    case 'O':   strcpy(tempstring, arg_option);
			fdevcontrol.fp = crackstring( tempstring, 
			    &fdevcontrol );
			break;

     //***********

	    case '@':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_timeDelayDevControl.fp = crackstring( tempstring, 
			    & FILTER_OUTPUT_timeDelayDevControl );
			break;
	    case ':':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_timeDelayPeak.fp = crackstring( tempstring, 
			    & FILTER_OUTPUT_timeDelayPeak );
			break;




	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;

	    case 'S':   strcpy(tempstring, arg_option);
			sourcedB.fp = crackstring( tempstring, 
			    &sourcedB );
			break;


	    case 'W':   strcpy(tempstring, arg_option);
			warpshape.fp = crackstring( tempstring, 
			    &warpshape );
			break;

	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	    case 's':   sflag = 1;
			break;
	} 
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "TVFILTDEVIATOR", 69 ) ; 
prline( 69,  "-" ) ; 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
 

//    sourceflag = ( (SOURCE_dB.A[0] > -96.0) || ( SOURCE_dB.n != 1.) ) ? 1 : 0 ; 


   // FIND MAX DELAY TIME
    findFuncMinMaxAvg( & FILTER_OUTPUT_timeDelayBase, &funcMin, &funcMax, &funcAvg ); 
    if( funcMax > maxDelayT ) maxDelayT = funcMax ; 
    findFuncMinMaxAvg( & FILTER_OUTPUT_timeDelayPeak, &funcMin, &funcMax, &funcAvg ); 
    if( funcMax > maxDelayT ) maxDelayT = funcMax ; 
  
//    if( sourceflag == 1 ) {
//	findFuncMinMaxAvg( &SOURCE_delayT, &funcMin, &funcMax, &funcAvg ) ; 
//	if( funcMax > maxDelayT) maxDelayT = funcMax ; 
//    } ; 

    ringTime = maxDelayT ; 





// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
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

    if(tfactor <= 0.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY A TIME FACTOR > 0. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 1.\n\n" ) ; 
	tfactor = 1. ; 
    }
    I = (int) ((float) D * tfactor ) ; 

//******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
    if( Nw < I ){
	// INCREASE WINDOW SIZE TO ACCOMODATE INTERPOLATION
	Nw = 2 ; while( Nw <= I )Nw *= 2 ;
	prt( "\n----> INCREASING WINDOW SIZE TO ACCOMODATE TIME RESYNTHESIS INTERPOLATION. <---" ) ;
	pri( Nw,  "NEW WINDOW SIZE" ) ; 
    }
//*********************************
    IR = (float) I / (float) R ; 
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    fundamental = (float) R / (float) N ; 
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	

    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 


// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( (ptrans.n  != 1.) || (harmadd.n  != 1.) || 
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.)  ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }



    // FREQ DEVIATION MODE SETUP
    if( freqdevmode.n != 1. ){
	// FILE MODE
	ranfreqdevswitch = 2 ; 
    }else if( freqdevmode.A[ 0 ] == 0. ){
	// RESPONSE MODE
	ranfreqdevswitch = 0 ; 
    }else if( freqdevmode.A[ 0 ] == 1. ){ 
	// RANDOM MODE
	ranfreqdevswitch = 1 ; 
    }else{
	// ILLEGAL MODE
	prt( "\n\n\tILLEGAL FREQUENCY DEVIATION MODE. BYE!\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
    } 


     // READ IN FFT HEADER VALUES
    if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &window_type,  normamp, &filter, 1 ) == -1){
        fprintf( stderr, "CHECK YOUR ANALYSIS FILE.\t\t. . . BYE.\n\n\n" ) ; exit(EXIT_FAILURE) ; 
    } ; 


    for( i = 0; i < analysis_chan; i++){
	prf( normamp[i], "normamp" ) ; 
    }


    analysis_N2 = analysis_N>>1 ; 
 		
// PRINT VALUES
prf( dur, "DURATION" ) ; 
pri( N,  "FFT SIZE" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
pri( R,  "SAMPLE RATE" ) ; 
pri( D,  "DECIMATION" ) ; 
pri( I,  "INTERPOLATION" ) ; 

prp( &dBgain,  "MASTER GAIN (dB)"  ) ; 
prp( &ptrans,  "PITCH TRANSPOSITION (semitones)"  ) ; 
prp( &harmadd,  "FREQUENCY SHIFT (Hz)"  ) ; 
prp( &attack,  " ENVELOPE ATTACK TIME (seconds)"  ) ; 
prp( &release,  " ENVELOPE RELEASE TIME (seconds)"  ) ; 
prp( &sourcedB,  "SOURCE SIGNAL FLOOR (dB)"  ) ; 


prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (dB)" ) ; 


//pr( &?,  "~"  ) ; 

    fprintf( stderr,  "\n\n" ) ; 
    // *******

//*******
// FIND DURATION OF INPUT FILE
    


    niframes = (((filter.n - (float) FFT_HEADER_SIZE ) / (float) (analysis_N + 2))) / analysis_chan ; 
    analysis_dur = (float) (niframes) / ((float) analysis_R / (float) analysis_D ) ; 
    iframes_per_sec = (float) analysis_R /  (float) analysis_D ; 
    pri( niframes,  "NUMBER OF FRAMES IN ANALYSIS" ) ; 

// FILTER WINDOW BOUNDARIES
    if( filtwinhi.A[ 0 ] < 0.) filtwinhi.A[ 0 ] = analysis_dur ;

    temp2 = (float) R  / (float) I  ; 
fprintf( stderr,"\n******** TVFILTER  OUTPUT *******************" ) ; 
fprintf( stderr,"\nOUTPUT DURATION: %f seconds",  dur ) ; 
fprintf( stderr,"\nOUTPUT: %d frames/sec ", (int) temp2 ) ; 
fprintf( stderr,"\nOUTPUT: %d total frames", (int) (temp2 * dur) ) ;


fprintf( stderr,"\n********** FILTER FILE ********************" ) ; 
prf( analysis_dur,	  "INPUT ANALYSIS: DURATION" ) ; 
pri( analysis_N,  "INPUT ANALYSIS: FFT SIZE" ) ; 
pri( analysis_R,  "INPUT ANALYSIS: SAMPLE RATE" ) ; 
pri( analysis_D,  "INPUT ANALYSIS: DECIMATION" ) ; 
pri( analysis_chan,  "INPUT ANALYSIS: NUMBER OF CHANNELS" ) ;
// ***** VERIFY ANALYSIS CHANNEL
    if( achannelout == 0 ){
	// ANALYSIS USES CHANNEL FOR CHANNEL
	if( analysis_chan < ichan) {
		// NOT ENOUGH ANALYSIS CHANNELS
		prbanner( "WARNING",  69 ) ; 
		prt( " =======> THERE ARE NOT ENOUGH  ANALYSIS CHANNELS TO MATCH INPUT SOUNDFILE <=====" ) ;
		    pri( analysis_chan,  "INPUT ANALYSIS: NUMBER OF CHANNELS" ) ; 
		    pri( ichan,  "INPUT SOUND FILE: NUMBER OF CHANNELS" ) ; 
		prt( " ANALYSIS CHANNEL WILL BE SET TO 1 AND APPLIED TO ALL INPUT CHANNELS.\n" ) ;
		prline( 69,  "-" ) ;
		achannelout = 1 ;
	}else{
		prt( "\n.....ANALYSIS AND INPUT SOUND FILE CHANNELS WILL BE PAIRED BY NUMBER.\n" ) ; 
	}  
    }else{
	// PARTICULAR ANALYSIS CHANNEL SELECTED
	if( (achannelout - 1) >= analysis_chan ){
	    // NOT AN ANALYSIS CHANNEL -- RESET TO 1 AND ANNOUNCE
	    prline( 1,  "*" ) ;
	    pri( achannelout,  "\nSELECTED ANALYSIS CHANNEL" ) ;
	    prt( "DOES NOT EXIST. WILL RESET INPUT ANALYSIS CHANNEL = 1" ) ;
	    prline( 1,  "*" ) ;
	    achannelout = 1 ;  
	}else{
	    // GOOD CHANNEL
	    prt( "\nTHE SELECTED INPUT ANALYSIS CHANNEL WILL BE APPLIED TO ALL CHANNELS." ) ; 
	    pri( achannelout,  "SELECTED INPUT ANALYSIS CHANNEL" ) ; 
	    prline( 1,  "*" ) ;
	}
    }


prp( &filtrate,  "FILTER RATE MULTIPLIER"  ) ; 
prp( &filttorigin,  "FILTER TIME POINT-ORIGIN"  ) ; 
prp( &filtwinlow,  "FILTER TIME WINDOW LOWER BOUNDARY"  ) ; 
prp( &filtwinhi,  "FILTER TIME WINDOW UPPER BOUNDARY"  ) ; 

if( Onset_and_Release_Segment_Mode__off_0__on_1 == 1) prt( "TIME WINDOW TRIGGER MODE: ON" ) ;
else prt( "TIME WINDOW TRIGGER MODE: OFF" ) ; 
if( wrap_0_fold_1_clip_2 == 0 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: WRAP" ) ; 
if( wrap_0_fold_1_clip_2 == 1 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: FOLD" ) ; 
if( wrap_0_fold_1_clip_2 == 2 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: CLIP/LIMIT" ) ; 
if( Mode__sampler_loop_0__autostop_1 == 1 )prt( "AUTOSTOP ON" ) ; 
if( Mode__sampler_loop_0__autostop_1 == 0 )prt( "AUTOSTOP OFF" ) ; 

if( bandrejecton ){
    prt( "FILTER TYPE: ............REJECT RESPONSE" ) ;
}else{
    prt( "FILTER TYPE: ............PASS RESPONSE" ) ;
}
prp( &ftrans,  "FILTER TRANSPOSITION (semitones)"  ) ; 
prp( &fshift,  "FILTER FREQUENCY SHIFT (Hz)"  ) ; 
prp( &fattack,  "FILTER ENVELOPE ATTACK TIME (seconds)"  ) ; 
prp( &frelease,  "FILTER ENVELOPE RELEASE TIME (seconds)"  ) ; 
prp( &filtcompthresh,  "FILTER COMPRESSION THRESHOLD (dB)"  ) ; 
prp( &filtcompdecibels,  "FILTER DEGREE OF COMPRESSION (dB)"  ) ; 


prline( 1,  "*" ) ; 


prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
prp( &warpshape, "INPUT SPECTRUM WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 
prp( &smoothingBW, "RESPONSE SMOOTHING (Q) FACTOR IN FREQUENCY OR (NEGATIVE) OCTAVES" ) ; 


prline( 1,  "*" ) ; 
prt( "************** FREQUENCY DEVIATION PARAMETERS ***************" ) ; 
if( ranfreqdevswitch == 0 ){
    prt( " FREQ DEVIATION: RESPONSE DRIVEN MODE" ) ; 
}else if( ranfreqdevswitch == 1 ){
    prt( " FREQ DEVIATION: RANDOM  MODE" ) ; 
    prp( &ranfreqdevresponse,  "FREQUENCY DEVIATION FUNCTION RESPONSE TIME" ) ; 
}else if( ranfreqdevswitch == 2 ){ 
    prt( " FREQ DEVIATION: FUNCTION FILE MODE" ) ; 
    prp( &ranfreqdevresponse,  "FREQUENCY DEVIATION FUNCTION RESPONSE TIME" ) ; 
}

prp( &fdevbase,  "BASE FREQUENCY DEVIATION (in semitones)"  ) ; 
prp( &fdevpeak,  "PEAK FREQUENCY DEVIATION (in semitones)"  ) ; 
prline( 1,  "*" ) ; 
prp( &fshiftbase,  "BASE FREQUENCY DEVIATION SHIFT (in Hz)"  ) ; 
prp( &fshiftpeak,  "PEAK FREQUENCY DEVIATION SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
prp( &fdevcontrol,  "MASTER FREQUENCY DEVIATION CONTROL (0-1)"  ) ; 
prp( &freqwarpshape, "INPUT SPECTRUM FREQUENCY DOMAIN WARPSHAPE INDEX" ) ; 


prp( & FILTER_OUTPUT_timeDelayBase, "BASE TIME DELAY (in seconds)" ) ; 
prp( & FILTER_OUTPUT_timeDelayPeak, "PEAK TIME DELAY (in seconds)" ) ; 
prp( & FILTER_OUTPUT_timeDelayDevControl,  "MASTER TIME DEVIATION CONTROL (0-1)"  ) ; 
prp( &timedelaywarpshape, "INPUT SPECTRUM TIME DOMAIN WARPSHAPE INDEX" ) ; 



fprintf( stderr,"\n*******************************************\n\n" ) ; 



//***********
// SETUP FILTER CONTROL VALUES
	         // FILTER INCREMENT IN SECONDS
    filttinc =  (float) D / (float) R ; 
//***********

// MAKE THRESH AMP
    threshfac = dB_to_amp( threshfacdB ) ; 
 //*****************
// SET UP SOME FILTER FILE VALUES
    analysis_fundamental = nyquist / (float) (analysis_N/2) ; 
    // MAKE RATIO OF ANALYSIS N TO SOURCE N FOR FILTER POSITIONING
    N_ratio = (float) analysis_N / (float) N ; 
    

    
    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */
    fvec( channel_filter, N+2 ) ;	/* previous analysis channels */
    fvec( previous_channel_filter, N+2 ) ;	/* previous analysis channels */

    fvec( previous_ranfreqv, N+2 ) ;	/* channels old random freq value  */

// ALLOCATE FILTER SPACE
    fvec( F,  analysis_N+2  ) ;	/* filter array */
    fvec( Ffreq,  analysis_N+2  ) ;	/* filter array */
    fvec( FtimeDelay, analysis_N+2 ) ; 
    fvec( previousF, analysis_N+2 ) ;	/* previous filter channels */
    fvec( tempF, analysis_N+2 ) ;


    fvec( F_lower,  analysis_N+2  ) ;	/* lower filter array */
    fvec( F_higher,  analysis_N+2  ) ;	/* higher filter array */


    // CHANNEL DELAYS
    maxNumOfDelayFrames = 1 + (int)((maxDelayT * frames_per_sec) + 0.5) ; 
    fvec( channel_delay, maxNumOfDelayFrames * (N + 2) ) ; 


// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

//*********************************************
//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

    // **** SET UP ANALYSIS CHANNEL
	ainchan = (achannelout == 0) ? channow : (achannelout - 1) ; 
    // SETUP FILTER CONTROL VALUES
	filttorigin.A[ 0 ] = fval( &filttorigin, dur, 0. );
	filttnow = filttorigin.A[ 0 ] ; // ACCUMULATED FILTER TIME POINT IN SECONDS

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 
pri( (ainchan+1), "INPUT ANALYSIS (FILTER) CHANNEL" ) ; 

    
    
    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; autostopflag = 0 ; 

    for( i = 0; i < (maxNumOfDelayFrames * (N + 2)); i++ ) channel_delay[i] = 0.  ; 


//**** SEED RANDOM
    srandom(1);


//***** ZERO OLD RAN VALUES
    for( i = 0; i < (N + 2); i++ ) {
	previous_ranfreqv[i] = 0. ; 
    }



    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;

//********** MAKE SOME ANNOUNCEMENTS
//    if(dBhi != 0. )fprintf(stderr," \nUSING FILTER EQ ......" ) ;
    if( compflag == 1 )fprintf(stderr," \nUSING FILTER SPECTRUM COMPRESSION......" ) ;
    fprintf(stderr," \n") ; 
    if( bandrejecton )prt( "INVERTING RESPONSE......." ) ; 

//*************
	    // SET AT DATA BEGIN
		fseek( filter.fp,  sizeof(float) * FFT_HEADER_SIZE,  SEEK_SET ) ; 
//*************
// pd(0) ; 
	
 //*********************************************
// LOOP FOR FRAMES
//*********************************************

   while ( (!eof)  && (autostopflag == 0) ) {
	in += D ;
	on += I ;
	timenow( dur ) ;
// pd(1) ; 
    eof = shiftin( input, Nw, D ) ;
    fold( input, Wanal, Nw, buffer, N, in ) ;
    rfft( buffer, N2, FORWARD ) ;
    convert( buffer, channel, N2, D, R ) ;
// pd(2) ; 
	// TRANSFER channel INTO CIRCULAR DELAY LINE
	frameNowChannelDelayIndex = frame_count ; 
	while( frameNowChannelDelayIndex >= maxNumOfDelayFrames ) 
		frameNowChannelDelayIndex -=  maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2) ; i++ ) 
		channel_delay[ (frameNowChannelDelayIndex * (N + 2)) + i] = channel[i] ; 

// pd(3) ; 

//*************
// FILTER TIME ISSUES


	// CONSTRAIN TIME POINT TO WINDOW OR ANALYSIS. 
	findFilterTimeAndConstrainByWindow(
	    filttinc,
	    &filttorigin, 
	    &filtrate,
	    Onset_and_Release_Segment_Mode__off_0__on_1,
	    Mode__sampler_loop_0__autostop_1, 
	    &autostopflag, 
	    wrap_0_fold_1_clip_2, 
	    &filttnow, 
	    &oldfilttnow, 
	    &filtwinlow, 
	    &filtwinhi,
	    &dur,
	    analysis_dur,
            0,
            1.
	) ; 


	loopSmoothTime = makeLoopSmoothTime(
	    filttnow,
	    wrap_0_fold_1_clip_2,
	    Mode__sampler_loop_0__autostop_1,
	    dur,  
	    &filtwinlow,    
	    &filtwinhi,
	    &peakLoopSmoothTime
	) ; 


//*************


//*************



//******************************************************************************
// MAKE THE FILTER FRAME
//******************************************************

	makeInterpolatedFilterFrame ( &filter, F_lower, F_higher, F,
			iframes_per_sec, analysis_N, filttnow, ainchan, analysis_chan
	) ; 

	// SAVE THE OLD TIME POINT
	oldfilttnow = filttnow ; 

		
//******************************************************

// NORMALIZE FOR LOOP
// ************

    normalizeLoopAmplitudes(
	LoopNormalizationFlag,
	Mode__sampler_loop_0__autostop_1,
	&filtwinlow, 
	&filtwinhi,
	&filter,
	F_lower, 
	F_higher,
	tempF,
	F,
	iframes_per_sec,
	(analysis_N + 2),
	ainchan,
	analysis_chan,
	analysis_dur, 
	filttnow
    ) ; 

//******************************************************




//*************NORMALIZE THE INPUT SPECTRUM
    normalize(  F,  (analysis_N + 2),   normamp[ainchan]  ) ;

//******************************************************
//**************COMPRESS THE INPUT SPECTRUM

// MAKE COMPRESSION STUFF

    filtcompthresh.A[ 0 ] =  fval( &filtcompthresh, dur, t );
    filtcompdecibels.A[ 0 ] =  fval( &filtcompdecibels, dur, t );


    if( filtcompthresh.A[ 0 ] > 0.){
	fprintf(stderr," \n\n***** COMPRESSION THRESHOLD MUST BE < 0dB. BYE.\n\n" ) ;
	exit(EXIT_FAILURE) ; 
    }
    if( filtcompdecibels.A[ 0 ] > 0.){
	fprintf(stderr," \n\n***** COMPRESSION DECIBELS MUST BE < 0dB. BYE.\n\n" ) ;
	exit(EXIT_FAILURE) ; 
    }

    filtcompthreshamp = pow( (double) 10.0, (double) (filtcompthresh.A[ 0 ]/20.) );
    filtcompamp =  pow( (double) 10.0, (double) (filtcompdecibels.A[ 0 ]/20.) );

    filtcompnormamp = 
	1. / (filtcompthreshamp + (filtcompamp * (1. - filtcompthreshamp))) ; 

    if( (filtcompthreshamp < 1.0) && (filtcompamp < 1.0) ){
	compflag = 1;
    }


/*

    if( compthreshamp <= expthreshamp ){
	fprintf( stderr, "\n\nCOMPRESSION THRESHOLD IS BELOW EXPANSION THRESHOLD!\n\nCOMPANSION SKIPPED!\n\n" ) ;
    }else{
	if( (compamp < 1.) || (expamp > 1. ) ){
	    if( (compamp <= 1.) && (expamp >= 1.) ){
		prt( "...............USING COMPRESSION" ) ; 
		 compand( F, (analysis_N + 2), filtcompthreshamp, filtcompamp, expthreshamp, expamp  ) ;
	    }else{
		prf( compdB.A[ 0 ],  "COMPRESSION DECIBELS" ) ; 
		prt( "ILLEGAL COMPRESSION/EXPANSION VALUES" ) ;
		prt( "...................NO COMPRESSION" ) ; 
	    }
	}else{
	    prt( "...................NO COMPRESSION" ) ; 
	}  
    }

*/


//******************************************************



   if( compflag == 1 )
	compress( F,  (analysis_N + 2),  filtcompthreshamp, filtcompamp,  filtcompnormamp ) ; 

//*************** IF BAND REJECT, THEN INVERT THE RESPONSE
    if( bandrejecton )invertresponse( F,  (analysis_N + 2), (bandrejecton - 1) ) ; 

//*************WARP THE INPUT SPECTRUM
	warpshape.A[ 0 ] = fval( &warpshape, dur, t );
    spectmagwarp( F,  (analysis_N + 2), warpshape.A[ 0 ], 0 ) ;

//*************EQUALIZE THE INPUT SPECTRUM
    eq( F,  (analysis_N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1,  0, 0 ) ; 

     //*********** SMOOTH THE SPECTRUM
	smoothingBW.A[ 0 ] = fval( & smoothingBW, dur, t );
    smoothspec( F, (analysis_N2 + 1), smoothingBW.A[ 0 ], R ) ; 


//***********SMOOTH THE CHANGES ON THE FILTER

		//RELEASE
    frelease.A[ 0 ] = fval( &frelease, dur, t );
    smooth_setup( frelease.A[ 0 ] + loopSmoothTime, &freleasec, &minusfreleasec, IR ) ; 

		// ATTACK
    fattack.A[ 0 ] = fval( &fattack, dur, t );
    smooth_setup( fattack.A[ 0 ] + loopSmoothTime, &fattackc, &minusfattackc, IR ) ; 


		// SMOOTH THE CHANGES TO THE FILTER
    smooth( F, previousF, (analysis_N + 2), fattackc, minusfattackc, freleasec, minusfreleasec ) ; 

//*** END FILTER SMOOTH ************************************* 

// COMPRESSION => REJECTION => FILTER WARPING => EQ => OCTAVE Q SMOOTH =>
//   =>  ATTACK/RELEASE => (OUTPUT FOR FILTERING) => 
// => FREQ DEVIATION WARPING => OUTPUT FOR FREQ DEVIATION

// ********** COPY F INTO Ffreq
    for(i = 0; i < analysis_N+2 ; i++)Ffreq[i] = F[i] ; 

// pd(4) ; 
// ********** COPY F INTO FtimeDelay
    for(i = 0; i < analysis_N+2; i++) FtimeDelay[i] = F[i] ; 

// pd(5) ; 

//*************FREQ WARP
	freqwarpshape.A[ 0 ] = fval( &freqwarpshape, dur, t );
    spectmagwarp( Ffreq,  N, freqwarpshape.A[ 0 ], 1 ) ;


//**************TIME DELAY WARP
	timedelaywarpshape.A[ 0 ] = fval( & timedelaywarpshape, dur, t );
    spectmagwarp( FtimeDelay,  N, timedelaywarpshape.A[ 0 ], 1 ) ;


//*************************
// GET THE VALUES
//*************************


//  SHIFT, GAIN, AND TRANSPOSITION

		harmadd.A[ 0 ] =  fval( &harmadd, dur, t );
		dBgain.A[ 0 ] =  fval( &dBgain, dur, t );
		    gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
		ptrans.A[ 0 ] = fval( &ptrans, dur, t );
		pm = semitones_to_mult( ptrans.A[ 0 ] ) ; 


		ftrans.A[ 0 ] = fval( &ftrans, dur, t );
		    fm = semitones_to_mult( ftrans.A[ 0 ] ) ; 
		fs = fshift.A[ 0 ] = fval( &fshift, dur, t );
		    // IF FREQ SHIFT FOR SOURCE ONLY, SUBTRACT FROM FILTER CHANGE
		    if( !pitchflag ) fs = fs - harmadd.A[ 0 ] ; 
		sourcedB.A[ 0 ] = fval( &sourcedB, dur, t );
		    sourceamp = dB_to_amp( sourcedB.A[ 0 ] ) ; 
		    filtamp = 1. - sourceamp ;  


		    // IF PITCH TRANSPOSITION FOR SOURCE ONLY, DIVIDE OUT OF FILTER CHANGE
		    if( !pitchflag ) fm = fm / pm ; 


		//RELEASE
		release.A[ 0 ] = fval( &release, dur, t );
		smooth_setup( release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 

		// ATTACK
		attack.A[ 0 ] = fval( &attack, dur, t );
		smooth_setup( attack.A[ 0 ], &attackc, &minusattackc, IR ) ; 



    //****** FREQ DEV ******

		// **** FREQ DEVIATION: SETUP 
		fdevbase.A[ 0 ] = fval( &fdevbase, dur, t );
		fdevpeak.A[ 0 ] = fval( &fdevpeak, dur, t );
		fdmdiff = fdevpeak.A[ 0 ] - fdevbase.A[ 0 ] ; 		    


		// **** FREQ DEVIATION: SETUP 
		fshiftbase.A[ 0 ] = fval( &fshiftbase, dur, t );
		fshiftpeak.A[ 0 ] = fval( &fshiftpeak, dur, t );
		fsdiff = fshiftpeak.A[ 0 ] - fshiftbase.A[ 0 ] ; 		    


		// *** RANDOM FREQ DEV RESPONSE TIME
		ranfreqdevresponse.A[ 0 ] = fval( &ranfreqdevresponse, dur, t );
		    smooth_setup( ranfreqdevresponse.A[ 0 ], 
			    &ranfreqsmoothc, &minusranfreqsmoothc, IR ) ; 

		// FREQ DEVIATION FILE MODE: GET FUNCTION VALUE
		freqdevmode.A[ 0 ] = fval( &freqdevmode, dur, t );

// pd(6) ; 
		// MAKE BASE, PEAK AND RANGE FOR TIME DELAY.
		FILTER_OUTPUT_timeDelayBase.A[ 0 ] = fval( & FILTER_OUTPUT_timeDelayBase, dur, t );
		FILTER_OUTPUT_timeDelayPeak.A[ 0 ] = fval( & FILTER_OUTPUT_timeDelayPeak, dur, t );
		timeRange = FILTER_OUTPUT_timeDelayPeak.A[ 0 ] - FILTER_OUTPUT_timeDelayBase.A[ 0 ] ; 	
		FILTER_OUTPUT_timeDelayDevControl.A[ 0 ] = fval( & FILTER_OUTPUT_timeDelayDevControl, dur, t );

// pd(7) ; 

		fdevcontrol.A[ 0 ] = fval( &fdevcontrol, dur, t );
		fdevminus = 1. - fdevcontrol.A[ 0 ] ; 


//*************************


//*****************
// MODIFICATIONS LOOP
//*****************


//********************* FILTER


		 for( i = 1,  j = 0; i < (N + 2); i+= 2,  j++ ){

		    // APPLY FILTER TRANSPOSE
		    // SHIFT
		    temp = (float) j ;  // THE 0-(N/2) BIN WE ARE ON (TO BE FILTERED)
		    temp = temp * N_ratio ; // SHIFT BY RATIO OF ANALYSIS N TO SOURCE N
		    temp = temp - ( fs / analysis_fundamental ) ; // SHIFT THE BIN
		    // TRANSPOSE
		    temp =  (temp / fm )  ; // TRANSPOSE THE BIN
		    i2p = temp - (float)((int) temp ) ; // FRACTIONAL BIN VALUE
		    i1p = 1. - i2p ;			// 1. - FRACTIONAL BIN VALUE
		    i1 = 2 * (int) temp ;		// THE LOWER BIN AMP INDEX
		    i2 = i1 + 2 ;			// THE UPPER BIN AMP INDEX

		    if( i1 < 0 ){			
			//UNDER THE ARRAY
 			i1 = 0; i2 = i1 + 2 ; i1p = 1. ; i2p = 0. ; 
   			temp = F[ 0 ]  ; 
    			temp2 = Ffreq[ 0 ]  ; 

		    }else if( i2 > (analysis_N - 2)  ){ 
			//OVER THE ARRAY
			i2 = N - 2 ; i1 = i2 - 2 ; i2p = 1. ; i1p = 0. ; 
    			temp = F[ analysis_N - 2 ]  ; 
    			temp2 = Ffreq[ analysis_N - 2 ]  ; 

		    }else{
			// IN ARRAY
			temp = ( F[ i1 ] * i1p ) + (F[ i2 ] * i2p ) ;
			temp2 = ( Ffreq[ i1 ] * i1p ) + (Ffreq[ i2 ] * i2p ) ;  

		    }
		    

		// PLACE DELAYED CHANNEL VALUES INTO channel_filter 
		// FOR THIS FtimeDelay RESPONSE AND BOUNDARIES.
// pd(8) ; 
		// FIRST MAKE INTERPOLATED TIM DELAY
		    interpTdelayFilterAmp = 
				( FtimeDelay[ i1 ] * i1p ) + (FtimeDelay[ i2 ] * i2p ) ; 

		    // FIND TIME DELAY
		    thisDelayT = FILTER_OUTPUT_timeDelayDevControl.A[ 0 ] * 
			(FILTER_OUTPUT_timeDelayBase.A[ 0 ] + (interpTdelayFilterAmp * timeRange) ) ; 
		    thisFrameDelay = frameNowChannelDelayIndex - (int)((thisDelayT * frames_per_sec) + 0.5) ; 
		    while( thisFrameDelay < 0) thisFrameDelay += maxNumOfDelayFrames ; 
		    channel_filter[i] = channel_delay[ (thisFrameDelay * (N + 2)) + i] ; 
		    channel_filter[i - 1] = channel_delay[ (thisFrameDelay * (N + 2)) + i - 1] ; 
// pd(9) ; 
		// SETUP PREVIOUS CHANNEL IF FIRST FRAME
		    if( !frame_count ) {
			previous_channel_filter[ i ] = channel[ i ] ; 
			previous_channel_filter[i - 1] = channel[i - 1] ; 
		    } ; 
// pd(10) ; 

		    // CHANGE AMP
		    channel_filter[i - 1] *=  ((filtamp * temp) + sourceamp );  


	    // MAKE NEW DEVIATED FREQUENCY FOR THIS BIN
		  // MAKE NEW DEVIATED FREQUENCY


		if( ranfreqdevswitch == 2 ){
		    // FILE MODE

		    // GET FUNCTION VALUE
		    temp3 = freqdevmode.A[ 0 ] ;

		    // SMOOTH  VALUE
		    temp3 = (ranfreqsmoothc * previous_ranfreqv[i - 1] )
					+ ( minusranfreqsmoothc * temp3 ); 
		    previous_ranfreqv[i - 1] = temp3 ; 

		    // MAKE FULLY DEVIATED FREQ
		    temp4 = semitones_to_mult(fdevbase.A[ 0 ] + (temp3 * fdmdiff))
			    * 
			(channel_filter[i] + (fshiftbase.A[ 0 ] + (temp3 * fsdiff)) ) ; 

		    

		    channel_filter[i] = channel_filter[i] + 
				(temp2 * fdevcontrol.A[ 0 ] * (temp4 - channel_filter[i])) ; 

	
		    
		}else if( ranfreqdevswitch == 1 ){
		    // RANDOM DEVIATION MODE


		    // MAKE SMOOTHED RANDOM VALUE
		    temp3 = (ranfreqsmoothc * previous_ranfreqv[i - 1] )
					+ ( minusranfreqsmoothc * randf( 0,  1. ) ); 
		    previous_ranfreqv[i - 1] = temp3 ; 

		    // MAKE FULLY DEVIATED FREQ
		    temp4 = semitones_to_mult(fdevbase.A[ 0 ] + (temp3 * fdmdiff))
			    * 
			(channel_filter[i] + (fshiftbase.A[ 0 ] + (temp3 * fsdiff)) ) ; 

		    

		    channel_filter[i] = channel_filter[i] + 
			(temp2 * fdevcontrol.A[ 0 ] * (temp4 - channel_filter[i])) ; 

	
		    
		}else if( ranfreqdevswitch == 0 ){
		    // RESPONSE MODE
		    channel_filter[i] = 
			( (fdevminus) + (fdevcontrol.A[ 0 ] 
			    * semitones_to_mult(fdevbase.A[ 0 ] + (temp2 * fdmdiff)))  )
			    * 
			(channel_filter[i] + 
			    (fdevcontrol.A[ 0 ] * (fshiftbase.A[ 0 ] + (temp2 * fsdiff)))
			 ) ; 
		}



		//****** END FILTER

		}

    // SMOOTH THE CHANGES TO THE SPECTRUM
    smooth( channel_filter, previous_channel_filter, (N + 2), attackc, minusattackc, releasec, minusreleasec ) ; 



		 for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){

// SHIFT BY -a AND TRANSPOSE BY -P

		    temp = pm * (channel_filter[i] + harmadd.A[ 0 ]) ;
		    
		// ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		    if((temp <= 0.) || (temp >= nyquist)) channel_filter[i - 1] = 0. ; 
		    else channel_filter[i] = temp ; 



		    channel_filter[i - 1] = channel_filter[i - 1] * gain ;  

		}
		

        synt = getthresh( channel_filter, N, threshfac );


// ** OSCIL BANK OR OVERLAP/ADD OUT
	if ( obank ) {
	    noscbank(channel_filter, N2, R, Nw, I, P, output);
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;
	} else {
	    unconvert( channel_filter, buffer, N2, I, R ) ;

	    rfft( buffer, N2, INVERSE ) ;
	    overlapadd( buffer, N, Wsyn, output, Nw, on ) ;
	     shiftout( output, Nw, I, on, 0 ) ;
	
	}

	frame_count++ ; 

    // FRAMES LOOP END

    }
    // FLUSH OUT AND CLOSE OUTPUT FILE
    shiftout( output, Nw, I, 1, 1 ) ;

    
// CHANNELS LOOP END
} 

    // CLOSE  INPUT FILE
    if(ifd)fclose(ifd);  


    fprintf(stderr,"\nTVFILTDEVIATOR : RESYNTHESIS COMPLETED\n");

  
 
 
    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;

   if( sourcedB.n != 1. ) fclose(sourcedB.fp ) ;
   if( release.n != 1. ) fclose(release.fp ) ;
   if( attack.n != 1. ) fclose(attack.fp ) ;
   if( frelease.n != 1. ) fclose(frelease.fp ) ;
   if( fattack.n != 1. ) fclose(fattack.fp ) ;
   if( filter.n != 1. ) fclose(filter.fp ) ;
   if( ftrans.n != 1. ) fclose(ftrans.fp ) ;
   if( fshift.n != 1. ) fclose(fshift.fp ) ;

   if( fdevbase.n != 1. ) fclose(fdevbase.fp ) ;
   if( fdevpeak.n != 1. ) fclose(fdevpeak.fp ) ;
   if( fdevpeak.n != 1. ) fclose(fdevpeak.fp ) ;
   if( fshiftbase.n != 1. ) fclose(fshiftbase.fp ) ;
   if( fshiftpeak.n != 1. ) fclose(fshiftpeak.fp ) ;
   if( freqwarpshape.n != 1. ) fclose(freqwarpshape.fp ) ;
   if( fdevcontrol.n != 1. ) fclose(fdevcontrol.fp ) ;

    if( filtrate.n != 1. ) fclose(filtrate.fp ) ;
     if( filttorigin.n != 1. ) fclose(filttorigin.fp ) ;
     if( filtwinlow.n != 1. ) fclose(filtwinlow.fp ) ;
     if( filtwinhi.n != 1. ) fclose(filtwinhi.fp ) ;
     if( warpshape.n != 1. ) fclose(warpshape.fp ) ;
     if( smoothingBW.n != 1. ) fclose(smoothingBW.fp ) ;
     if( filtcompthresh.n != 1. ) fclose(filtcompthresh.fp ) ;
     if( filtcompdecibels.n != 1. ) fclose(filtcompdecibels.fp ) ;


    exit(EXIT_SUCCESS) ;
}



void usage()
{
    fprintf(stderr, "%s",
	"tvfiltdeviator:  time-varying  cross-synthetic, phase vocoder filter\n"
	"	    with response-correlated frequency deviation\n"
	"tvfiltdeviator   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	N:	FFT length (must be a power of 2) [1024]\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	w:	window type: 0 = hamming,  1 = rectangular  \n"
	"		    2 = Blackman,  3 = Bartlett triangular [0.]\n"
	"		    4-12 = Kaiser windows for alpha = 4-12,  respectively\n"
	"		    (representative sidelobe levels for alpha: \n"
	"		      4 = -30dB,  8 = -58 dB,  12 = -90 dB)\n"
	"	D:	analysis frames per second [200]\n"
	"	I:	time expansion/contraction factor  [1.] \n"
	"		  (duration = duration * factor, 1. = original time) \n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds (0. = end of file) [0.] \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"

	"	P:	pitch transposition in semitones (func) [0]\n"
	"	B:	TRANSPOSITION/SHIFT application FLAG [0]\n"
	"		 Apply -P and -a to:\n"
	"		  source only -- prefliter (0),\n"  
	"		  or to  source and filter -- postfilter (1)\n"  

	"	a:	frequency shift factor (bin frequency adder, before -P )(func)[0.] \n"
	"	A:	gain in decibels (func) [0.] \n"

	"	F:	FILTER time_varying, frequency response file analysis \n"

	"	K:	FILTER analysis channel (1 -> ?) (0 = all) [0] \n"

	"	q:	FILTER: filtering method: \n"
	"		 0 = pass, 1 = invert response against 0 dB peak\n"
	"		    2 = invert response against peak amp in frame [0] \n"

	"	u:	FILTER data access mode [1]\n"
	"		   (0 = explicit time: Use -Q)\n"
	"		   (1 = rate mode: Use -Q as begin point, -Y as rate control) \n"
	"	Q:	FILTER time point (func) [0.] \n"
	"	g:	FILTER time window: lower boundary (func) [0.] \n"
	"	G:	FILTER time window: upper boundary (func) [end of file] \n"
	"	x:	DATA time window: boundary flag  [0]\n"
	"		   0 = wrap time into window\n"
	"		   1 = fold time into window bounds\n"
	"		   2 = clip or limit time to nearest window boundary\n"
	"	v:	DATA time window: trigger entry mode flag [0]\n"
	"		   Begin or trigger use of time window boundaries with first entry\n"
	"		   of time point into window bounds. 0 = off, 1 = on\n"
	"		    (upper boundary < 0. defaults to end of file)\n"
	"		    (upper boundary < 0. defaults to end of file\n)"
	"	Y:	FILTER rate multiplier (func) [1.]\n"
	"		    (1. = rate of original, 2. = twice as fast, etc.)\n"
	"		    (negative = reverse,  0 = stationary) \n"
	"	d:	DATA auto stop: 0 = off,  1 = on [0]\n"
	"		    (When on,  auto stop will terminate synthesis\n"
	"		      when a time boundary is crossed.)\n"

	"	    FILTER COMPRESSION:\n"
	"	E:	FILTER-spectrum compression threshold (in decibels) [0] \n"
	"	c:	FILTER-spectrum decibels of compression [0] \n"

	"	T:	FILTER frequency response transposition in semitones (func) [0]\n"
	"	V:	FILTER  response shifter (func) [0.] \n"
	"	Z:      FILTER envelope envelope release time   (func) [0.]\n"
	"	z:      FILTER envelope envelope attack time   (func) [0.]\n"

	"	S:	FILTER source decibels (reduces filtered signal in proportion)(func) [-96] \n"
	"	W:	FILTER warp index for reshaping filter frequency response (func) [0.] \n"
	"		    values > 0 expand the dynamic range, \n"
	"		    values < 0 compress the dynamic range \n"

	"	l:      envelope attack time  (func) [0.]\n"
	"	L:      envelope release time   (func) [0.]\n"

	"	f:	"FILTER_FREQ_RESPONSE_SMOOTHING_BW

	"	//:	filter time delay deviation warpshape (func) [0]\n"
	"	/a:	frequency shift adder (func) [0]\n"
	"	@:	FILTER OUTPUT time Delay Deviation Control (func) [1]\n"
	"	~:	peak loop smooth time (func)  [.2]\n"


	"	PROCESSING SEQUENCE:\n"
	"	     COMPRESSION/EXPANSION => REJECTION => FILTER WARPING => EQ => \n"
	"	     OCTAVE Q SMOOTH =>  ATTACK/RELEASE => (OUTPUT FOR FILTERING) =>\n" 
	"	       FREQ DEVIATION WARPING => (OUTPUT FOR FREQ DEVIATION)\n"


	"	h:      FREQUENCY DEVIATION: base deviation in semitones (func) [0.]\n"
	"	j:      FREQUENCY DEVIATION: peak deviation in semitones (func) [0.]\n"
	"	U:	RANDOM FREQUENCY DEVIATION MODE:\n"
	"		    0 = response mode,  1 = random mode,  \"file name\" = file mode [0]\n"
	"	y:	FREQUENCY DEVIATION FUNCTION: response time in seconds (func) [0.]\n"
	"		    (random and file modes only)\n"
	"	J:      FREQUENCY DEVIATION: base frequency shift (func) [0.]\n"
	"	k:      FREQUENCY DEVIATION: peak frequency shift (func) [0.]\n"
	"	n:	FREQUENCY RESPONSE WARP:  index for reshaping frequency response [0.] \n"
	"		    in frequency processing (frequency deviation domain)\n"
	"		    values > 0 close down response, < 0 open it up\n"
	"	O:	FREQUENCY DEVIATION: master deviation control (0 (off) - 1) (func) [1.] \n"



	"	     FILTER FREQUENCY RESPONSE -- SHELF EQ: (pre-processing)\n"
	"	H:	FILTER SHELF EQ: Low shelf gain in dB [0.] \n"
	"	X:	FILTER SHELF EQ: High shelf gain in dB [0.] \n"
	"	m:	FILTER SHELF EQ: Low shelf frequency in Hz [200.] \n"
	"	R:	FILTER SHELF EQ: High shelf frequency in Hz [2000.] \n"

	"	t:	oscillator resynthesis threshold [.001]\n"
	"	p:	amplitude reports print mode: 0 = off, 1 = on [0]\n" 
	"	i:	time interval between amplitude reports [.25]\n" 

	"	_:	 AUTO OUTPUT SOUND FILE PLAY:\n"
	"		    0  = off \n"
	"		    -1 = interactive: Prompt for each play.\n"
	"		    -2 = interactive: Play once, then prompt for more.\n"
        "                   1 or greater = Auto-repeat for specified repetitions. [0]\n" 

	"	=:	 PEAK RESCALE LEVEL 0 to -96 dB \n"
	"		    1 = Rescale to level of input file.\n" 
	"		    2 = Bypass rescaling. [ 1 ]\n" 

	);
    exit(EXIT_SUCCESS);
}


void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
