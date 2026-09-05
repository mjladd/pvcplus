#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j, k,  i1,  i2  ;
float i1p,  i2p ; 
float nyquist,  basefreq ;
double atof(),  DD ;
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int analysis_N,  analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 
float tfactor = 1.,  frames_per_sec=200; 
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  achannelout=0 ;
float P = 1.0;
FILE *fopen();
char ch,  tempstring[ STRING_SIZE ] ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel, *output ;
float threshfac = .001,  threshfacdB=-96 ;

float  *previous_channel, *previous_buffer, *next_channel,  *next_buffer,
    *feedback_channel,  *feedback_buffer, *bufferout, *F,  
	*tempF, *previousF, *F_lower,  *F_higher,    
    *binfreq, *phasediff,      temp,  temp2,   
    source_pm,  feedback_pm, fm, fs,   source_gain,  feedback_gain ; 
float envattack,  envrelease,  minusattack,  minusrelease  ;  
float getthresh();
float normamp[MAXIMUM_CHANNELS] ;
int ainchan ; 

float low, hi, avg, median ; int length ; 

int Mode__sampler_loop_0__autostop_1=0,  autostopflag=0,  wrap_0_fold_1_clip_2=0, Onset_and_Release_Segment_Mode__off_0__on_1=0, LoopNormalizationFlag=0   ; 

float loopSmoothTime=0.0, loopSmoothTimec, minusloopSmoothTimec ; 


double ar_dB,  temp2double, temp3double ; 

float feedbackthresh,  feedt,  minusfeedbacklowpass, feedlowpass, feedlevel, 
       *freqdither ; 

float fundamental,  factor,  sourceamp,  filtamp ; 
int filtflag=0 ; 

float   IR, DR,  dur=0., saved_dur ;
int pitchflag=0; 
//****

float  FEEDBACK_dBhitemp,  FEEDBACK_dBlowtemp ; 
float prebalancesum,  postbalancesum, balancelimitdB=0,  balancelimitamp=0  ; 
int limitcount=0,  balanceflag=0 ; 

float filttnow=0.,  oldfilttnow, filttinc; 
 

float analysis_fundamental, N_ratio ;  

char	crackTempString[ STRING_SIZE ]="EMPTY\0" ; 


//**
// PEAK LOOP SMOOTH TIME
struct func peakLoopSmoothTime ; 

// INPUT EQ HIGH SHELF: DB
struct  func  INPUT_dBhi  ;  

// INPUT EQ HIGH SHELF: FREQ
struct  func   INPUT_freqhi ;   
  
// INPUT EQ LOW SHELF: DB
struct  func  INPUT_dBlow  ;  

// INPUT EQ LOW SHELF: FREQ
struct  func   INPUT_freqlow ;   
  
//**
// OUTPUT EQ HIGH SHELF: DB
struct  func  OUTPUT_dBhi  ;  

// OUTPUT EQ HIGH SHELF: FREQ
struct  func   OUTPUT_freqhi ;   
  
// OUTPUT EQ LOW SHELF: DB
struct  func  OUTPUT_dBlow  ;  

// OUTPUT EQ LOW SHELF: FREQ
struct  func   OUTPUT_freqlow ;   
  
struct func spectrum_warpshape ; 


//**

// REVERB INPUT FILTER
struct  func  filter ; 

// REVERB INPUT FILTER SEMITONE TRANSPOSE
struct  func  ftrans ; 

// REVERB INPUT FILTER TRANSPOSE SHIFTER
struct  func  fshift ; 

//  REVERB INPUT FILTER SOURCE DECIBELS
struct  func  sourcedB ; 

// REVERB INPUT FILTER DECAY TIME
struct  func   filter_decay_time  ;



//**
// FEEDBACK LOOP EQ HIGH SHELF: DB
struct  func  FEEDBACK_dBhi  ;  

// FEEDBACK LOOP  EQ HIGH SHELF: FREQ
struct  func   FEEDBACK_freqhi ;   
  
// FEEDBACK LOOP  EQ LOW SHELF: DB
struct  func  FEEDBACK_dBlow  ;  

// FEEDBACK LOOP  EQ LOW SHELF: FREQ
struct  func   FEEDBACK_freqlow ;   
  
  

// REVERB LOOP EQ DECAY TIME
struct  func   FEEDBACK_decay_time  ;

 


//****


// MASTER GAIN
struct  func  master_dBgain ; 

//**** SOURCE 
// SOURCE GAIN
struct  func  source_dBgain ; 

// SOURCE FREQUENCY SHIFT ADDER
struct  func  source_harmadd ; 

// SOURCE PITCH MULTIPLIER
struct  func  source_ptrans ; 


//**** REVERB 
// REVERB GAIN
struct  func  feedback_dBgain ; 

// REVERB FREQUENCY SHIFT ADDER
struct  func  feedback_harmadd ; 

// REVERB PITCH MULTIPLIER
struct  func  feedback_ptrans ; 

// REVERB THRESHOLD
struct  func  feedback_threshdB ;

// FEEDBACK DECAY TIME IN SECONDS
struct  func  feedback_level ;

// REVERB RANDOM FREQ DEVIATION PROPORTION
struct  func  feedback_dither ;


// FEEDBACK RANDOM FREQ DEVIATION RESPONSE TIME
struct  func  feedback_lowpass ;



//  RELEASE
struct  func  release ; 

//  ATTACK
struct  func  attack ; 


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
float  filtcompthresh_in_dB=0,   filtcompthreshamp; 
int compflag=0 ; 

//  FILTER DECIBELS OF COMPRESSION 
float  filtcompdecibels=0,  filtcompamp,  filtcompnormamp ; 
// ****



//#include "underflow.h"



//*****************INITIALIZE

spectrum_warpshape.L = 1. ; spectrum_warpshape.n = 1. ; spectrum_warpshape.A[ 0 ] = 0 ; 



// PEAK LOOP SMOOTH TIME
peakLoopSmoothTime.L = 1. ; peakLoopSmoothTime.n = 1. ; peakLoopSmoothTime.A[ 0 ] = 0.2 ; 


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
//**@@@


// INPUT EQ HIGH SHELF: DB
INPUT_dBhi.L = 1. ;  INPUT_dBhi.n = 1. ; INPUT_dBhi.A[ 0 ] = 0. ; 

// INPUT EQ HIGH SHELF: FREQ
INPUT_freqhi.L = 1. ;  INPUT_freqhi.n = 1. ; INPUT_freqhi.A[ 0 ] = 2000. ; 
  
// INPUT EQ LOW SHELF: DB
INPUT_dBlow.L = 1. ;  INPUT_dBlow.n = 1. ; INPUT_dBlow.A[ 0 ] = 200. ; 

// INPUT EQ LOW SHELF: FREQ
INPUT_freqlow.L = 1. ;  INPUT_freqlow.n = 1. ; INPUT_freqlow.A[ 0 ] = 0. ; 
  

//**

//**
// OUTPUT EQ HIGH SHELF: DB
OUTPUT_dBhi.L = 1. ;  OUTPUT_dBhi.n = 1. ; OUTPUT_dBhi.A[ 0 ] = 0. ; 

// OUTPUT EQ HIGH SHELF: FREQ
OUTPUT_freqhi.L = 1. ;  OUTPUT_freqhi.n = 1. ; OUTPUT_freqhi.A[ 0 ] = 2000. ; 
  
// OUTPUT EQ LOW SHELF: DB
OUTPUT_dBlow.L = 1. ;  OUTPUT_dBlow.n = 1. ; OUTPUT_dBlow.A[ 0 ] = 200. ; 

// OUTPUT EQ LOW SHELF: FREQ
OUTPUT_freqlow.L = 1. ;  OUTPUT_freqlow.n = 1. ; OUTPUT_freqlow.A[ 0 ] = 0. ; 
  
//**@@@
// REVERB INPUT FILTER
filter.L = 1. ; filter.n = 0. ; filter.A[ 0 ] = 0. ; 

// REVERB INPUT FILTER SEMITONE TRANSPOSE
ftrans.L = 1. ; ftrans.n = 1. ; ftrans.A[ 0 ] = 0. ; 

// REVERB INPUT FILTER TRANSPOSE SHIFTER
fshift.L = 1. ; fshift.n = 1. ; fshift.A[ 0 ] = 0. ; 

// REVERB INPUT SOURCE DECIBELS
sourcedB.L = 1. ; sourcedB.n = 1. ; sourcedB.A[ 0 ] = -96. ; 

// REVERB INPUT FILTER DECAY TIME
filter_decay_time.L = 1. ; filter_decay_time.n = 1. ; filter_decay_time.A[ 0 ] = 1. ; 



//**

// FEEDBACK EQ HIGH SHELF: DB
FEEDBACK_dBhi.L = 1. ;  FEEDBACK_dBhi.n = 1. ; FEEDBACK_dBhi.A[ 0 ] = 0. ; 

// FEEDBACK EQ HIGH SHELF: FREQ
FEEDBACK_freqhi.L = 1. ;  FEEDBACK_freqhi.n = 1. ; FEEDBACK_freqhi.A[ 0 ] = 2000. ; 
  
// FEEDBACK EQ LOW SHELF: DB
FEEDBACK_dBlow.L = 1. ;  FEEDBACK_dBlow.n = 1. ; FEEDBACK_dBlow.A[ 0 ] = 200. ; 

// FEEDBACK EQ LOW SHELF: FREQ
FEEDBACK_freqlow.L = 1. ;  FEEDBACK_freqlow.n = 1. ; FEEDBACK_freqlow.A[ 0 ] = 0. ; 
  
// REVERB LOOP EQ DECAY TIME
FEEDBACK_decay_time.L = 1. ;  FEEDBACK_decay_time.n = 1. ; FEEDBACK_decay_time.A[ 0 ] = 1. ; 

//**

 
// MASTER GAIN
master_dBgain.L = 1. ;  master_dBgain.n = 1. ; master_dBgain.A[ 0 ] = 0. ; 

//**** SOURCE 

// SOURCE GAIN
source_dBgain.L = 1. ;  source_dBgain.n = 1. ; source_dBgain.A[ 0 ] = 0. ; 

// SOURCE FREQUENCY SHIFT ADDER
source_harmadd.L = 1. ; source_harmadd.n = 1. ; source_harmadd.A[ 0 ] = 0. ; 

// SOURCE PITCH MULTIPLIER
source_ptrans.L = 1. ; source_ptrans.n = 1. ; source_ptrans.A[ 0 ] = 0. ; 

//**** REVERB 

// REVERB GAIN
feedback_dBgain.L = 1. ;  feedback_dBgain.n = 1. ; feedback_dBgain.A[ 0 ] = 0. ; 

// REVERB FREQUENCY SHIFT ADDER
feedback_harmadd.L = 1. ; feedback_harmadd.n = 1. ; feedback_harmadd.A[ 0 ] = 0. ; 

// REVERB PITCH MULTIPLIER
feedback_ptrans.L = 1. ; feedback_ptrans.n = 1. ; feedback_ptrans.A[ 0 ] = 0. ; 

// REVERB THRESHOLD
feedback_threshdB.L = 1. ; feedback_threshdB.n = 1. ; feedback_threshdB.A[ 0 ] = -96. ; 

// FEEDBACK DECAY TIME IN SECONDS
feedback_level.L = 1. ; feedback_level.n = 1. ; feedback_level.A[ 0 ] = 0. ; 

// REVERB RANDOM FREQ DEVIATION PROPORTION
feedback_dither.L = 1. ; feedback_dither.n = 1. ; feedback_dither.A[ 0 ] = 0. ; 

// REVERB RANDOM FREQ DEVIATION LOWPASS FILTER COEFFICIENT
feedback_lowpass.L = 1. ; feedback_lowpass.n = 1. ; feedback_lowpass.A[ 0 ] = 0. ; 



//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 

strcpy( routine, "ringtvfilter" ) ; 

if( argc < 2 )usage() ; 

    while( (ch= crack( argc, argv,
    "~|_|=|@|!|:|/|a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|", 

    0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
// NEW

   case '/':	strcpy( crackTempString, arg_option) ; 
                switch( crackTempString[0] )
                {  // a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|
                   // r|R|s|S|t|T|u|U|v|V|w|X|x|y|Y|z|Z|_|1|2|3|4|5|6|7|8|9|~|=|@|!|:|/|0|

	    			case '/':   window_type = 
						(int) crackfloat( &crackTempString[1], ch ) ;
					break;
	    			case 'Q':	wrap_0_fold_1_clip_2 = 
						(int) crackfloat( &crackTempString[1], ch );
					break;


                } ;
                break;





// END NEW
	    case 'N':   N = (int) crackfloat( arg_option, ch ); // crackfloat( arg_option, ch )
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
//	    case '/':   window_type = (int) crackfloat( arg_option, ch ) ;
//			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'I':   tfactor = crackfloat( arg_option, ch );
			break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

	    case '@':   Mode__sampler_loop_0__autostop_1 = (int) crackfloat( arg_option, ch ) ;
			break;


	    case '~':   achannelout = (int) crackfloat( arg_option, ch ) ;
			break;

         case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;

	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;

	    case 'A':   LoopNormalizationFlag = (int) crackfloat( arg_option, ch ) ; 
			break;
// SOURCE
	    case 'S':   strcpy(tempstring, arg_option);
			source_dBgain.fp = crackstring( tempstring, &source_dBgain );
			break;
	    case 'p':   strcpy(tempstring, arg_option);
			source_ptrans.fp = crackstring( tempstring, &source_ptrans); 
			break;
	    case 'f':   strcpy(tempstring, arg_option);
			source_harmadd.fp = crackstring( tempstring, &source_harmadd );
			break;
	    case 'B':   pitchflag = (int) crackfloat( arg_option, ch ) ;
			break;

// REVERB

	    case 'E':   balancelimitdB = crackfloat( arg_option, ch ) ;
			break;

	    case 'F':   strcpy(tempstring, arg_option);
			feedback_dBgain.fp = crackstring( tempstring, &feedback_dBgain );
			break;
	    case 'P':   strcpy(tempstring, arg_option);
			feedback_ptrans.fp = crackstring( tempstring, &feedback_ptrans); 
			break;
	    case 'H':   strcpy(tempstring, arg_option);
			feedback_harmadd.fp = crackstring( tempstring, &feedback_harmadd );
			break;
	    case 'z':   strcpy(tempstring, arg_option);
			feedback_threshdB.fp = crackstring( tempstring, &feedback_threshdB );
			break;

	    case 'Z':   strcpy(tempstring, arg_option);
			feedback_level.fp = crackstring( tempstring, &feedback_level );
			break;
	    case 'j':   strcpy(tempstring, arg_option);
			feedback_dither.fp = crackstring( tempstring, &feedback_dither );
			break;
	    case 'K':   strcpy(tempstring, arg_option);
			feedback_lowpass.fp = crackstring( tempstring, &feedback_lowpass );
			break;

  	    case 'Q':   strcpy(tempstring, arg_option);
			FEEDBACK_dBhi.fp = crackstring( tempstring, &FEEDBACK_dBhi );
			break;
	    case 'm':   strcpy(tempstring, arg_option);
			FEEDBACK_freqhi.fp = crackstring( tempstring, &FEEDBACK_freqhi );
			break;
			// FEEDBACK LOW SHELF EQ
  	    case 'X':   strcpy(tempstring, arg_option);
			FEEDBACK_dBlow.fp = crackstring( tempstring, &FEEDBACK_dBlow );
			break;
	    case 'U':   strcpy(tempstring, arg_option);
			FEEDBACK_freqlow.fp = crackstring( tempstring, &FEEDBACK_freqlow );
			break;


	    case 'T':   strcpy(tempstring, arg_option);
			FEEDBACK_decay_time.fp = crackstring( tempstring, &FEEDBACK_decay_time );
			break;

	
			// INPUT HIGH SHELF EQ
  	    case 'Y':   strcpy(tempstring, arg_option);
			INPUT_dBhi.fp = crackstring( tempstring, &INPUT_dBhi );
			break;
	    case 'n':   strcpy(tempstring, arg_option);
			INPUT_freqhi.fp = crackstring( tempstring, &INPUT_freqhi );
			break;
			// INPUT LOW SHELF EQ
  	    case 'O':   strcpy(tempstring, arg_option);
			INPUT_dBlow.fp = crackstring( tempstring, &INPUT_dBlow );
			break;
	    case 'd':   strcpy(tempstring, arg_option);
			INPUT_freqlow.fp = crackstring( tempstring, &INPUT_freqlow );
			break;
	

			// OUTPUT HIGH SHELF EQ
  	    case 'c':   strcpy(tempstring, arg_option);
			OUTPUT_dBhi.fp = crackstring( tempstring, &OUTPUT_dBhi );
			break;
	    case 'G':   strcpy(tempstring, arg_option);
			OUTPUT_freqhi.fp = crackstring( tempstring, &OUTPUT_freqhi );
			break;
			// OUTPUT LOW SHELF EQ
  	    case 'k':   strcpy(tempstring, arg_option);
			OUTPUT_dBlow.fp = crackstring( tempstring, &OUTPUT_dBlow );
			break;
	    case 's':   strcpy(tempstring, arg_option);
			OUTPUT_freqlow.fp = crackstring( tempstring, &OUTPUT_freqlow );
			break;



	    case 'y':   strcpy(tempstring, arg_option);
			filter.fp = crackstring_bin_only( tempstring, 
			    &filter );
			break;


// *****
	    case 'h':   strcpy(tempstring, arg_option);
			filttorigin.fp = crackstring( tempstring, 
			    &filttorigin );
			break;
	    case 'R':   strcpy(tempstring, arg_option);
			filtrate.fp = crackstring( tempstring, 
			    &filtrate );
			break;

	    case 'g':   strcpy(tempstring, arg_option);
			filtwinlow.fp = crackstring( tempstring, 
			    &filtwinlow );
			break;
	    case 'J':   strcpy(tempstring, arg_option);
			filtwinhi.fp = crackstring( tempstring, 
			    &filtwinhi );
			break;
	    case ':':	Onset_and_Release_Segment_Mode__off_0__on_1 = (int) crackfloat( arg_option, ch );
			break;

	    case 'a':   strcpy(tempstring, arg_option);
			peakLoopSmoothTime.fp = crackstring( tempstring, &peakLoopSmoothTime );
			break;


	    case 'W':   filtcompthresh_in_dB = crackfloat( arg_option, ch ) ;
			break;
	    case 'v':   filtcompdecibels = crackfloat( arg_option, ch ) ;
			break;

// *****




	    case 'o':   filtflag = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'u':   strcpy(tempstring, arg_option);
			ftrans.fp = crackstring( tempstring, 
			    &ftrans );
			break;
	    case 'V':   strcpy(tempstring, arg_option);
			fshift.fp = crackstring( tempstring, 
			    &fshift );
			break;
	    case 'x':   strcpy(tempstring, arg_option);
			sourcedB.fp = crackstring( tempstring, 
			    &sourcedB );
			break;
	    case 'q':   strcpy(tempstring, arg_option);
			spectrum_warpshape.fp = crackstring( tempstring, &spectrum_warpshape );
			break;


	    case 'r':   strcpy(tempstring, arg_option);
			filter_decay_time.fp = crackstring( tempstring, &filter_decay_time );
			break;




	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;

            case 'w':	quiet = (int) crackfloat( arg_option, ch ) ; 
			break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; 
			break;
	} 
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "RINGTVFILTER: REVERBERATOR/RESONATOR WITH TIME-VARYING FILTER", 69 ) ; 
prline( 69,  "-" ) ; 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }

    funcStats( &feedback_level, &low, &hi, &avg, &length, &median ) ; 
    ringTime = hi ; 

    prf( ringTime, "ringTime" ) ; 

 
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
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
    basefreq = (float) (R / N) ; 
     	// OSC BANK
    P = 1. ; obank = 1 ;  
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    fundamental = (float) R/ (float) N;
    factor = R/(D*TWOPI);
    DD = (double) D ; 
// REVERB TIME STUFF
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    DR = (float) D / (float) R ; 
    IR = (float) I / (float) R ;

    // COMPUTE THE DURATION
    dur = saved_dur = (endt - begint) * (float) I / (float) D ; 

    // FIND BALANCE LIMIT AMP FOR BALANCE GAIN LIMITING
    balancelimitamp = (float) pow( (double) 10.0, (double) ((balancelimitdB)  / 20.) );
   if( balancelimitdB <= 0. ) balanceflag = 0 ; else balanceflag = 1 ; 


// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
/*
    if( 
	(source_ptrans.n  != 1.) || (source_harmadd.n  != 1.) || 
	    (source_ptrans.A[0] != 0.) || (source_harmadd.A[0] != 0.) ||   
		  (feedback_ptrans.n  != 1.) || (feedback_harmadd.n  != 1.) || 
		    (feedback_ptrans.A[0] != 0.) || (feedback_harmadd.A[0] != 0.) ||
			(feedback_dither.A[ 0 ] != 0.)  
		) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
    
*/
	P = 1. ; obank = 1 ;  

    // READ IN FFT HEADER VALUES
    if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &k,  normamp, &filter, 1 ) == -1){
        fprintf( stderr, "CHECK YOUR ANALYSIS FILE.\t\t. . . BYE.\n\n\n" ) ; exit(EXIT_FAILURE) ; 
    } ; 


//***************** PRINT VALUES
prf( dur, "OUTPUT FILE: DURATION" ) ; 

prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 

pri( frames_per_sec,  "FRAMES/SECOND" ) ; 
pri( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
pri( I,  "      INTERPOLATION SAMPLES (samples between resynthesis frames)" ) ; 
prline( 1,  "*" ) ; 
prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (in dB)" ) ; 
prline( 1,  "*" ) ; 

prp( &source_dBgain,  "SOURCE GAIN (in dB)"  ) ; 
prp( &source_ptrans,  "SOURCE PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &source_harmadd,  "SOURCE FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
prp( &feedback_dBgain,  "REVERB GAIN (in dB)"  ) ; 
prp( &FEEDBACK_decay_time,  "REVERB DECAY TIME (in seconds) "  ) ;
prp( &feedback_ptrans,  "REVERB PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &feedback_harmadd,  "REVERB FREQUENCY SHIFT (in Hz)"  ) ; 
prp( &feedback_threshdB,  "REVERB THRESHOLD (in dB)"  ) ; 
prp( &feedback_level,  "REVERB COEFFICIENT (in Hz)"  ) ; 
prp( &feedback_dither,  "REVERB FREQUENCY DEVIATION PROPORTION"  ) ; 
prp( &feedback_lowpass,  "REVERB FREQUENCY DEVIATION LOWPASS COEFFICIENT"  ) ; 
prline( 1,  "*" ) ; 
prp( &attack,  "REVERB: ENVELOPE ATTACK TIME (in seconds)"  ) ; 
prp( &release,  "REVERB: ENVELOPE RELEASE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prp( &FEEDBACK_dBhi,  "FEEDBACK (IN LOOP) EQ  HIGH SHELF dB"  ) ;
prp( &FEEDBACK_freqhi,  "FEEDBACK (IN LOOP) EQ  HIGH SHELF FREQ"  ) ;
prp( &FEEDBACK_dBlow,  "FEEDBACK (IN LOOP) EQ  LOW SHELF dB"  ) ;
prp( &FEEDBACK_freqlow,  "FEEDBACK (IN LOOP) EQ  LOW SHELF FREQ"  ) ;

prline( 1, "*" ) ;  

prp( &INPUT_dBhi,  "FEEDBACK INPUT EQ  HIGH SHELF dB"  ) ;
prp( &INPUT_freqhi,  "FEEDBACK INPUT EQ  HIGH SHELF FREQ"  ) ;
prp( &INPUT_dBlow,  "FEEDBACK INPUT EQ  LOW SHELF dB"  ) ;
prp( &INPUT_freqlow,  "FEEDBACK INPUT EQ  LOW SHELF FREQ"  ) ;

prline( 1, "*" ) ;  

prp( &OUTPUT_dBhi,  "FEEDBACK OUTPUT EQ  HIGH SHELF dB"  ) ;
prp( &OUTPUT_freqhi,  "FEEDBACK OUTPUT EQ  HIGH SHELF FREQ"  ) ;
prp( &OUTPUT_dBlow,  "FEEDBACK OUTPUT EQ  LOW SHELF dB"  ) ;
prp( &OUTPUT_freqlow,  "FEEDBACK OUTPUT EQ  LOW SHELF FREQ"  ) ;

prline( 1, "*" ) ;  
prp( &ftrans,  "FILTER TRANSPOSITION (in semitones) "  ) ;
prp( &fshift,  "FILTER SHIFT (in Hz) "  ) ;
prp( &sourcedB,  "FILTER: SOURCE SIGNAL FLOOR (in dB)"  ) ;
prp( &filter_decay_time,  "FEEDBACK FILTER DECAY TIME (in seconds) "  ) ;
prline( 1, "*" ) ;  
prp( &filtrate,  "FILTER RATE MULTIPLIER"  ) ; 
prp( &filttorigin,  "FILTER TIME POINT-ORIGIN"  ) ; 
prp( &filtwinlow,  "FILTER TIME WINDOW LOWER BOUNDARY"  ) ; 
prp( &filtwinhi,  "FILTER TIME WINDOW UPPER BOUNDARY"  ) ; 

prp( &spectrum_warpshape, "FILTER SPECTRUM WARPSHAPE INDEX" ) ; 

if( Mode__sampler_loop_0__autostop_1 == 0){
	prt( "SET TO SAMPLER LOOP MODE" ) ;
	if( wrap_0_fold_1_clip_2 == 0 )prt( "LOOP METHOD: WRAP" ) ; 
	if( wrap_0_fold_1_clip_2 == 1 )prt( "LOOP METHOD: FOLD" ) ; 
	if( wrap_0_fold_1_clip_2 == 2 )prt( "LOOP METHOD: CLIP/LIMIT" ) ; 
	prp( &peakLoopSmoothTime, "LOOP SMOOTH TIME" ) ; 
	if( Onset_and_Release_Segment_Mode__off_0__on_1 == 1) prt( "USING ONSET/RELEASE MODE" ) ;
}else 
	prt( "SET TO AUTOSTOP MODE" ) ; 

prline( 69, "*" ) ;  
//fprintf( stderr, "\nfiltcompthresh_in_dB: %f\n", filtcompthresh_in_dB ) ; 
prf( filtcompthresh_in_dB,  "FILTER COMPRESSION THRESHOLD (in decibels)"  ) ; 
prf( filtcompdecibels,  "FILTER COMPRESSION LEVEL (in decibels)"  ) ; 
prline( 69, "*" ) ;  

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

prp( &ftrans,  "FILTER TRANSPOSITION (semitones)"  ) ; 
prp( &fshift,  "FILTER FREQUENCY SHIFT (Hz)"  ) ; 
//prp( &fattack,  "FILTER ENVELOPE ATTACK TIME (seconds)"  ) ; 
//prp( &frelease,  "FILTER ENVELOPE RELEASE TIME (seconds)"  ) ; 
prf( filtcompthresh_in_dB,  "FILTER COMPRESSION THRESHOLD (dB)"  ) ; 
prf( filtcompdecibels,  "FILTER DEGREE OF COMPRESSION (dB)"  ) ; 
prline( 1,  "*" ) ; 

fprintf( stderr,"\n*******************************************\n\n" ) ; 


//***********
// SETUP FILTER CONTROL VALUES
	         // FILTER INCREMENT IN SECONDS
    filttinc =  (float) D / (float) R ; 
//***********

 //*****************
// SET UP SOME FILTER FILE VALUES
    analysis_fundamental = nyquist / (float) (analysis_N/2) ; 
    // MAKE RATIO OF ANALYSIS N TO SOURCE N FOR FILTER POSITIONING
    N_ratio = (float) analysis_N / (float) N ; 
    
//*****************

// MAKE COMPRESSION STUFF
    if( filtcompthresh_in_dB > 0.){
	fprintf(stderr," \n\n***** COMPRESSION THRESHOLD MUST BE < 0dB. BYE.\n\n" ) ;
	exit(EXIT_FAILURE) ; 
    }
    if( filtcompdecibels > 0.){
	fprintf(stderr," \n\n***** COMPRESSION DECIBELS MUST BE < 0dB. BYE.\n\n" ) ;
	exit(EXIT_FAILURE) ; 
    }
    filtcompthreshamp = pow( (double) 10.0, (double) (filtcompthresh_in_dB/20.) );
    filtcompamp =  pow( (double) 10.0, (double) (filtcompdecibels/20.) );

    filtcompnormamp = 
	1. / (filtcompthreshamp + (filtcompamp * (1. - filtcompthreshamp))) ; 

    if((filtcompthreshamp < 1.0) && (filtcompamp < 1.0) ){
	compflag = 1;
    }
//*****************




    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */

    fvec( output, Nw ) ;	/* output buffer */

    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */

    fvec( feedback_buffer, N ) ;		/* FFT buffer */
    fvec( feedback_channel, N+2 ) ;	/* analysis channels */

    fvec( previous_buffer, N ) ;	/* previous analysis buffer */
    fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

    fvec( next_buffer, N ) ;	/* next analysis buffer */
    fvec( next_channel, N+2 ) ;	/* next analysis channels */

    fvec( phasediff, N+2 ) ;	/* previous analysis buffer */


    fvec( binfreq, N2+1 ) ;	/* center frequency of analysis */
    fvec( freqdither, N2+1 ) ;	/* previous frequency deviation */

    fvec( bufferout, N ) ;		/* FFT buffer */

    // ALLOCATE FILTER SPACE
    fvec( F,  analysis_N+2  ) ;	/* filter array */
    fvec( tempF,  analysis_N+2  ) ;	
    fvec( previousF,  analysis_N+2  ) ;	


    fvec( F_lower,  analysis_N+2  ) ;	/* lower filter array */
    fvec( F_higher,  analysis_N+2  ) ;	/* higher filter array */





// MAKE THRESH AMP
    threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	

// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

// MAKE BIN FREQ ARRAY
    for(j = 0; j < (N2 + 1); j++){
	binfreq[j] = fundamental * (float) j ; 
    }


//********** MAKE SOME ANNOUNCEMENTS
    FEEDBACK_dBhi.A[ 0 ] = fval( &FEEDBACK_dBhi, dur, t );
    if(FEEDBACK_dBhi.A[ 0 ] != 0. )fprintf(stderr," \nUSING INSIDE-LOOP, FEEDBACK EQ ......" ) ;
    INPUT_dBhi.A[ 0 ] = fval( &INPUT_dBhi, dur, t );
    if(INPUT_dBhi.A[ 0 ] != 0. )fprintf(stderr," \nUSING FEEDBACK INPUT EQ ......" ) ;

//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

    dur = saved_dur ; 

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


    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;

//********** MAKE SOME ANNOUNCEMENTS
    if( compflag == 1 )fprintf(stderr," \nUSING FILTER SPECTRUM COMPRESSION......" ) ;
    fprintf(stderr," \n") ; 
	
//*********************************************
// LOOP FOR FRAMES
//*********************************************

    while ( (!eof)  ) { //  && (autostopflag == 0)
	in += D ;
	on += I ;
	timenow( dur ) ;
		if( sflag ){
			if( fread(channel, sizeof(float) , N+2, stdin) == 0 )
				eof = 1;
		}
		else {
	 	  eof = shiftin( input, Nw, D ) ;
	  	  fold( input, Wanal, Nw, buffer, N, in ) ;
	  	  rfft( buffer, N2, FORWARD ) ;
		}

	// SETUP PREVIOUS CHANNEL
//	    if( !frame_count )for(i = 0; i < (N + 2); i++)
//		    previous_channel[ i ] = channel[ i ] ; 


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

	
        smooth_setup( loopSmoothTime, &loopSmoothTimec, &minusloopSmoothTimec, IR ) ; 


//*************
//*************


//******************************************************************************
// MAKE THE FILTER FRAME
//******************************************************

	makeInterpolatedFilterFrame ( &filter, F_lower, F_higher, F,
			iframes_per_sec, (analysis_N + 2), filttnow, ainchan, analysis_chan
	) ; 

	if( frame_count == 0 )
	    for( i = 0; i < (analysis_N + 2); i++ ) previousF[i] = F[i] ; 

	// SAVE THE OLD TIME POINT
	oldfilttnow = filttnow ; 


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

    // SMOOTH THE CHANGES TO THE SPECTRUM
    smooth( F, previousF, (analysis_N + 2), 
		loopSmoothTimec, minusloopSmoothTimec, loopSmoothTimec, minusloopSmoothTimec ) ; 

    for( i = 0; i < (analysis_N + 2); i++ ) previousF[i] = F[i] ; 


		
//******************************************************
//*************NORMALIZE THE INPUT SPECTRUM
    normalize(  F,  (analysis_N + 2),   normamp[ainchan]  ) ;

//**************COMPRESS THE INPUT SPECTRUM
   if( compflag == 1 )
	compress( F,  (analysis_N + 2),  filtcompthreshamp, filtcompamp,  filtcompnormamp ) ; 

//*************WARP THE INPUT SPECTRUM
    spectrum_warpshape.A[ 0 ] =  fval( &spectrum_warpshape, dur, t ) ;
    spectmagwarp( F,  (analysis_N + 2), spectrum_warpshape.A[ 0 ], 0 ) ;



//*************************
// GET THE VALUES
//*************************


	    // SOURCE
		source_dBgain.A[ 0 ] = 
		    fval( &source_dBgain, dur, t );
		    source_gain = dB_to_amp( source_dBgain.A[ 0 ] ) ; 
		source_harmadd.A[ 0 ] = 
		    fval( &source_harmadd, dur, t );
		source_ptrans.A[ 0 ] = 
		    fval( &source_ptrans, dur, t ) ;
		    source_pm = semitones_to_mult( source_ptrans.A[ 0 ] ) ; 

	    // REVERB
		feedback_dBgain.A[ 0 ] = 
		    fval( &feedback_dBgain, dur, t );
		    feedback_gain = dB_to_amp( feedback_dBgain.A[ 0 ] ) ; 
		feedback_harmadd.A[ 0 ] = 
		    fval( &feedback_harmadd, dur, t );
		feedback_ptrans.A[ 0 ] = 
		    fval( &feedback_ptrans, dur, t ) ;

		feedback_level.A[ 0 ] = 
		    fval( &feedback_level, dur, t );
		// RECOMPUTE FOR INTERPOLATION
		if(feedback_level.A[ 0 ] <= 0.){
		    feedlevel = 0. ; 
		}else{
		    feedlevel = 
			pow( (double) ar_dB,  
			    (double) (IR / feedback_level.A[ 0 ] ) ) ; 
		}


		feedback_dither.A[ 0 ] = 
		    fval( &feedback_dither, dur, t );

		feedback_lowpass.A[ 0 ] = 
		    fval( &feedback_lowpass, dur, t );
		// RECOMPUTE FOR INTERPOLATION
		if(feedback_lowpass.A[ 0 ] <= 0.){
		    feedlowpass = 0 ; minusfeedbacklowpass =  1.  ; 
		}else{
		    feedlowpass = 
			pow( (double) ar_dB,  
			    (double) (IR / feedback_lowpass.A[ 0 ] ) ) ; 
		    minusfeedbacklowpass =  1. - feedlowpass ; 
		}

		feedback_threshdB.A[ 0 ] =  
		    fval( &feedback_threshdB, dur, t ) ;
		    feedbackthresh = dB_to_amp( feedback_threshdB.A[ 0 ] ) ; 

		feedback_pm = (float)
		    pow( (double) 2.0, (double) ( feedback_ptrans.A[ 0 ] / 12.0 ) ) ;


		release.A[ 0 ] = fval( &release, dur, t );
		smooth_setup( release.A[ 0 ], &envrelease, &minusrelease, IR ) ; 
		attack.A[ 0 ] = fval( &attack, dur, t );
		smooth_setup( attack.A[ 0 ], &envattack, &minusattack, IR ) ; 


//****************************
//*** FILTER ANALYSIS FOR INPUT TO REVERB x(n) "filtered"		

	    // COPY BUFFER INTO REVERB BUFFER
		 for( i = 0; i < N; i++ ) feedback_buffer[i] = buffer[i]; 

	    // LEAN CONVERT REVERB BUFFER
	    leanconvert( feedback_buffer, feedback_channel,   N2, D, R ) ; 

//**** THRESHOLD ENVELOPE GOES HERE
		// GET FEEDBACK THRESHOLD 
		feedt = getthresh( feedback_channel, (N + 2), feedbackthresh );

		// LOOP FOR BINS
		for( i = 1, j = 0;  i < (N + 2); i += 2,  j++ ){
		    //******** ZERO AMPS BELOW THRESHOLD RELATIVE TO THIS FRAME
		    if( feedback_channel[ i - 1 ] < feedt ){ 
		    // RELEASE
			feedback_channel[ i - 1 ] = 
			    (envrelease * previous_channel[i - 1] ) ; 

		    } else {
		//************ATTACK AND RELEASE

		      if((feedback_channel[i - 1] < previous_channel[i - 1]) ){
		    // RELEASE
			feedback_channel[ i - 1 ] = 
			    (envrelease * previous_channel[i - 1] )
				+
			   ( minusrelease * feedback_channel[ i - 1 ] ); 
			
		       } else {
			// ATTACK
			feedback_channel[ i - 1 ] = 
			    (envattack * previous_channel[i - 1] )
				+
			   ( minusattack * feedback_channel[ i - 1 ] ); 

		       }

		    }
		    // SAVE MAGNITUDE
		    previous_channel[i - 1] = feedback_channel[i - 1] ; 

		}




	    // APPLY INPUT EQ TO FEEDBACK CHANNEL 
		INPUT_dBhi.A[ 0 ] =  fval( &INPUT_dBhi, dur, t ) ;
		INPUT_freqhi.A[ 0 ] =  fval( &INPUT_freqhi, dur, t ) ;
		INPUT_dBlow.A[ 0 ] =  fval( &INPUT_dBlow, dur, t ) ;
		INPUT_freqlow.A[ 0 ] =  fval( &INPUT_freqlow, dur, t ) ;

		eq( feedback_channel,  (N + 2),  
		    INPUT_dBlow.A[ 0 ],    INPUT_dBhi.A[ 0 ], 
		    INPUT_freqlow.A[ 0 ],  INPUT_freqhi.A[ 0 ],  
		    fundamental, 1,  0, 0 ) ; 


	    // ************* APPLY FILTER TO REVERB INPUT
	    // FIRST MAKE THE VALUES
		ftrans.A[ 0 ] = fval( &ftrans, dur, t );
		    fm = semitones_to_mult( ftrans.A[ 0 ] ) ; 
		  // IF PITCH TRANSPOSITION FOR INPUT ONLY, DIVIDE OUT OF FILTER CHANGE
		    if( !pitchflag ) fm = fm / feedback_pm ; 

		fs = fshift.A[ 0 ] = fval( &fshift, dur, t );
		    // IF FEEDBACK FREQ SHIFT FOR INPUT ONLY, SUBTRACT FROM FILTER CHANGE
		    if( !pitchflag ) fs = fs - feedback_harmadd.A[ 0 ] ; 
		sourcedB.A[ 0 ] = fval( &sourcedB, dur, t );
		    sourceamp = dB_to_amp( sourcedB.A[ 0 ]  ) ; 
			filtamp = 1. - sourceamp ;  

	   //*** NOW FILTER IT
	      if((filtflag == 0) && (sourcedB.A[ 0 ] != 0.)){
	    //***** 		
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
    			temp = F[ 0 ]  ; 

		    }else if( i2 > (analysis_N - 2)  ){ 
			//OVER THE ARRAY
    			temp = F[ analysis_N - 2 ]  ; 

		    }else{
			// IN ARRAY
			temp = ( F[ i1 ] * i1p ) + (F[ i2 ] * i2p ) ; 

		    }
			// MAKE NEW AMP
		    feedback_channel[i - 1] *= ((filtamp * temp) + sourceamp );  
		}
	      }

	    //**** LEAN UNCONVERT REVERB CHANNEL TO BUFFER FOR ADDING IN
		    leanunconvert( feedback_channel, feedback_buffer,  N2, D, R ) ; 


//** ADD IN PREVIOUS BUFFER
		 for( i = 0; i < N; i++ ){
		    feedback_buffer[i] = 
			    feedback_buffer[i] +
			   (feedlevel * next_buffer[i] ); 
		 }		
		// CONVERT  REVERB BUFFER TO AMP AND PHASE
		leanconvert( feedback_buffer, feedback_channel,   N2, D, R ) ; 


		// FEEDBACK EQ
		FEEDBACK_dBhi.A[ 0 ] =  fval( &FEEDBACK_dBhi, dur, t ) ;
		FEEDBACK_freqhi.A[ 0 ] =  fval( &FEEDBACK_freqhi, dur, t ) ;
		FEEDBACK_dBlow.A[ 0 ] =  fval( &FEEDBACK_dBlow, dur, t ) ;
		FEEDBACK_freqlow.A[ 0 ] =  fval( &FEEDBACK_freqlow, dur, t ) ;
		FEEDBACK_decay_time.A[ 0 ] =  fval( &FEEDBACK_decay_time, dur, t ) ;

		if(FEEDBACK_decay_time.A[ 0 ] < IR){
		    FEEDBACK_dBhitemp = FEEDBACK_dBhi.A[ 0 ] ; 
		    // MAKE FACTOR
		    temp2double =  (double) 1. ; 
		}else{
		    // MODIFY FEEDBACK EQ DB BY FRAME RATE
		    FEEDBACK_dBhitemp = (FEEDBACK_dBhi.A[ 0 ] * IR ) / FEEDBACK_decay_time.A[ 0 ] ; 
		    FEEDBACK_dBlowtemp = (FEEDBACK_dBlow.A[ 0 ] * IR ) / FEEDBACK_decay_time.A[ 0 ] ; 
		    // MAKE FACTOR
		    temp2double =  (double) (IR / FEEDBACK_decay_time.A[ 0 ]) ; 
		}



// DECAYING SIGNAL BALANCE
    // FIND SUM OF ALL MAGNITUDES
		if(
		    (((filtflag != 0) && (sourcedB.A[ 0 ] != 0.)) 
			|| (FEEDBACK_dBhitemp != 0.))
			    && (balanceflag)
				    ){
		    prebalancesum = 0 ; 
		    for( i = 0; i < (N + 2); i+= 2) prebalancesum += feedback_channel[i] ; 
		}



		// INSIDE FEEDBACK LOOP EQ
		eq( feedback_channel,  (N + 2),  
		    FEEDBACK_dBlowtemp,    FEEDBACK_dBhitemp, 
		    FEEDBACK_freqlow.A[ 0 ],  FEEDBACK_freqhi.A[ 0 ],  
		    fundamental, 1,  0, 0 ) ; 



		// FILTER DECAY
		filter_decay_time.A[ 0 ] =  fval( &filter_decay_time, dur, t ) ;
		    // MAKE DECAY FACTOR
		if(filter_decay_time.A[ 0 ] < IR){
		    temp3double =  (double) 1. ; 
		}else{
		    temp3double =  (double) (IR / filter_decay_time.A[ 0 ]) ; 
		}


		

		
// EXTRA FILTER GOES HERE
	   //*** NOW FILTER IT
	      if((filtflag != 0) && (sourcedB.A[ 0 ] != 0.)){
		 for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){
		    // LOOP FOR BINS	    
	    // FIND THE INDECES WHICH RESULT AFTER SHIFT AND TRANSPOSE OF 
	    // THE FILTER
		    // APPLY FILTER TRANSPOSE
		    // SHIFT
		    temp = (i - 1) / 2 ; 
		    temp = temp - ( fshift.A[ 0 ] / fundamental ) ; 
		    // TRANSPOSE
		    temp =  (temp / fm )  ; 
		    i2p = temp - (float)((int) temp ) ; 
		    i1p = 1. - i2p ; 
		    i1 = 2 * (int) temp ; 
		    i2 = i1 + 2 ; 
		    
		    if( i1 < 0 ){ //UNDER THE ARRAY
			i1 = 0; i2 = i1 + 2 ; i1p = 1. ; i2p = 0. ; 
		    }
		    if( i2 > N ){ //OVER THE ARRAY
			i2 = N - 2 ; i1 = i2 - 2 ; i2p = 1. ; i1p = 0. ; 
		    }

	    // MAKE NEW AMPLITUDE FOR THIS BIN
		// FIRST MAKE INTERPOLATED FILTER AMP
		    temp = ( F[ i1 ] * i1p ) + (F[ i2 ] * i2p ) ; 
			// MAKE NEW AMP IN RANGE ABOVE SOURCE LEVEL
		    temp =  (filtamp * temp) + sourceamp ;  
		// MODIFY FOR DECAY TIME
		    temp = pow( (double) temp,  temp3double ) ; 
		// NOW APPLY
		    feedback_channel[i - 1] = temp * feedback_channel[i - 1] ; 

		}
	      }

// EXPERIMENTAL DECAYING SIGNAL BALANCE
    // FIND SUM OF ALL MAGNITUDES


	if( balanceflag ){
		if(
		    ((filtflag != 0) && (sourcedB.A[ 0 ] != 0.)) 
			|| (FEEDBACK_dBhitemp != 0.)
				    ){
		    if( !frame_count )fprintf( stderr, "\n\n..USING BALANCE ON DECAY FILTER AND/OR EQ....." ) ; 
			    postbalancesum = 0 ; 
		    for( i = 0; i < (N + 2); i+= 2) postbalancesum += feedback_channel[i] ; 
		
    // NOW BALANCE
		    if( (prebalancesum > 0.) && (postbalancesum > 0.) ){
			temp = prebalancesum / postbalancesum ; 
		    // BALANCE LIMITER
		    if( temp > balancelimitamp ) {
			temp = balancelimitamp ; limitcount++ ;  
		    }			
		    for( i = 0; i < (N + 2); i+= 2) feedback_channel[i] *= temp  ; 
		    
		    }
		}

	}



		for( i = 1, j = 0;  i < (N + 2); i += 2,  j++ ){
			// MAKE NEW PHASE DIFFERENCE
		    phasediff[ i ] = feedback_channel[ i ] - previous_channel[ i ] ; 
			// ADVANCE PHASE TO NEXT FRAME 
		    next_channel[ i ] = feedback_channel[ i ]   + phasediff[ i ] ; 
			// SETUP MAGNITUDE IN NEXT CHANNEL
		    next_channel[i - 1] = feedback_channel[i - 1] ; 
		    			
			
			// SAVE PREVIOUS PHASE
		    previous_channel[i] = feedback_channel[i] ; 


		}		  



//**** CONVERT NEXT CHANNEL TO BUFFER FOR ADDING NEXT TIME AROUND
		    leanunconvert( next_channel, next_buffer,  N2, D, R ) ; 

//**** PREPARE OUTPUT

//**** NOW CONVERT  THE FEEDBACK BUFFER 
	  	  convert1( feedback_buffer, feedback_channel, N2, D, R ) ;

//****** FILTER FEEDBACK WITH OUTPUT SETTINGS
		OUTPUT_dBhi.A[ 0 ] =  fval( &OUTPUT_dBhi, dur, t ) ;
		OUTPUT_freqhi.A[ 0 ] =  fval( &OUTPUT_freqhi, dur, t ) ;
		OUTPUT_dBlow.A[ 0 ] =  fval( &OUTPUT_dBlow, dur, t ) ;
		OUTPUT_freqlow.A[ 0 ] =  fval( &OUTPUT_freqlow, dur, t ) ;

		eq( feedback_channel,  (N + 2),  
		    OUTPUT_dBlow.A[ 0 ],    OUTPUT_dBhi.A[ 0 ], 
		    OUTPUT_freqlow.A[ 0 ],  OUTPUT_freqhi.A[ 0 ],  
		    fundamental, 1,  0, 0 ) ; 


//**** NOW A FULL CONVERT OF THE CURRENT BUFFER
		if( obank ){
	  	    convert( buffer, channel, N2, D, R ) ;
		}else{
	  	    leanconvert( buffer, channel, N2, D, R ) ;
		}
//****************************

// CHANGE THINGS

		 for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){
 
	//********* FEEDBACK CHANGES

		//*** RANDOM FREQ DITHER
		    if(feedback_dither.A[ 0 ] != 0.){
			// DITHER
			temp = randf( -(feedback_dither.A[ 0 ]) * binfreq[j], 
				(feedback_dither.A[ 0 ]) * binfreq[j] ) ;

			temp = (feedlowpass * freqdither[ j ]) +
			    (minusfeedbacklowpass * temp) ;
			  // SAVE OLD
			  freqdither[ j ] = temp ;  
			    
			feedback_channel[i] = feedback_channel[i] + temp ; 
		    }

		    temp = feedback_pm  * (feedback_harmadd.A[ 0 ]  + feedback_channel[i]) ;
		    if((temp > 0.) && (temp < nyquist)) feedback_channel[i] = temp ;
		    else feedback_channel[i - 1] = 0. ;  


		// ADD GAIN
		    feedback_channel[i - 1] = feedback_channel[i - 1] * feedback_gain ;  
		    
    //****		    
	//********* SOURCE CHANGES

		    temp = source_pm  * (source_harmadd.A[ 0 ]  + channel[i]) ;
		    if((temp > 0.) && (temp < nyquist)) channel[i] = temp ;
		    else channel[i - 1] = 0. ;  

		    // ADD GAIN
		    channel[i - 1] = channel[i - 1] * source_gain ;  
		    
    //****		    

		}

		
// ** OSCIL BANK OR OVERLAP/ADD OUT
	if ( obank ) {
	    // OBANK
           synt = getthresh( channel, (N + 2), threshfac );
	   temp = getthresh( feedback_channel, (N + 2), threshfac );

	    if(temp > synt) synt = temp ; 
	    noscbank2(channel, N2, R, Nw, I, P,  output,  feedback_channel, N2 );
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;

	} else {
	    // OVERLAP/ADD
    	    leanunconvert( channel, buffer, N2, I, R ) ;
	    leanunconvert( feedback_channel, feedback_buffer, N2, I, R ) ;
	    for( i = 0; i < N; i++ )bufferout[i] = buffer[i] + feedback_buffer[i] ; 

	    rfft( bufferout, N2, INVERSE ) ;
	    overlapadd( bufferout, N, Wsyn, output, Nw, on ) ;
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

    temp = (float) limitcount / (float) (ochan * frame_count) ; 
    prf( temp,  "\nBALANCE LIMITER USAGE: PROPORTION OF TIME USED" ) ; 
    prf( (1. - temp),  "\nBALANCE  USAGE: PROPORTION OF TIME USED" ) ; 

    if( filtrate.n != 1. ) fclose(filtrate.fp ) ;
    if( filttorigin.n != 1. ) fclose(filttorigin.fp ) ;
    if( filtwinlow.n != 1. ) fclose(filtwinlow.fp ) ;
    if( filtwinhi.n != 1. ) fclose(filtwinhi.fp ) ;
    if( INPUT_dBhi.n != 1. ) fclose(INPUT_dBhi.fp ) ;
    if( INPUT_freqhi.n != 1. ) fclose(INPUT_freqhi.fp ) ;
    if( INPUT_dBlow.n != 1. ) fclose(INPUT_dBlow.fp ) ;
    if( INPUT_freqlow.n != 1. ) fclose(INPUT_freqlow.fp ) ;
    if( OUTPUT_dBhi.n != 1. ) fclose(OUTPUT_dBhi.fp ) ;
    if( OUTPUT_freqhi.n != 1. ) fclose(OUTPUT_freqhi.fp ) ;
    if( OUTPUT_dBlow.n != 1. ) fclose(OUTPUT_dBlow.fp ) ;
    if( OUTPUT_freqlow.n != 1. ) fclose(OUTPUT_freqlow.fp ) ;
    if( filter.n != 1. ) fclose(filter.fp ) ;
    if( ftrans.n != 1. ) fclose(ftrans.fp ) ;
    if( fshift.n != 1. ) fclose(fshift.fp ) ;
    if( sourcedB.n != 1. ) fclose(sourcedB.fp ) ;
    if( filter_decay_time.n != 1. ) fclose(filter_decay_time.fp ) ;
    if( FEEDBACK_dBhi.n != 1. ) fclose(FEEDBACK_dBhi.fp ) ;
    if( FEEDBACK_freqhi.n != 1. ) fclose(FEEDBACK_freqhi.fp ) ;
    if( FEEDBACK_dBlow.n != 1. ) fclose(FEEDBACK_dBlow.fp ) ;
    if( FEEDBACK_freqlow.n != 1. ) fclose(FEEDBACK_freqlow.fp ) ;
    if( FEEDBACK_decay_time.n != 1. ) fclose(FEEDBACK_decay_time.fp ) ;
    if( master_dBgain.n != 1. ) fclose(master_dBgain.fp ) ;
    if( source_dBgain.n != 1. ) fclose(source_dBgain.fp ) ;
    if( source_harmadd.n != 1. ) fclose(source_harmadd.fp ) ;
    if( source_ptrans.n != 1. ) fclose(source_ptrans.fp ) ;
    if( feedback_dBgain.n != 1. ) fclose(feedback_dBgain.fp ) ;
    if( feedback_harmadd.n != 1. ) fclose(feedback_harmadd.fp ) ;
    if( feedback_ptrans.n != 1. ) fclose(feedback_ptrans.fp ) ;
    if( feedback_threshdB.n != 1. ) fclose(feedback_threshdB.fp ) ;
    if( feedback_level.n != 1. ) fclose(feedback_level.fp ) ;
    if( feedback_dither.n != 1. ) fclose(feedback_dither.fp ) ;
    if( feedback_lowpass.n != 1. ) fclose(feedback_lowpass.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;

    
     
    fprintf(stderr,"\nRINGTVFILTER: RESYNTHESIS COMPLETED\n");
    exit(EXIT_SUCCESS) ;

}



void usage()
{
    fprintf(stderr, "%s",
	"ringtvfilter:  phase vocoder reverberator/resonator \n"
	"		    with switchable input/feedback time-varying filter \n"
	"ringtvfilter   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	N:	FFT length (must be a power of 2) [1024]\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	//:	window type: 0 = hamming,  1 = rectangular  \n"
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

	"	A:	MASTER (source + reverb) gain in decibels (func)[0.] \n"

	"	S:	SOURCE gain in decibels (func)[0.] \n"
	"	f:	SOURCE frequency shift factor (bin frequency adder, before -P )(func) [0.] \n"
	"	p:	SOURCE pitch transposition in semitones (func) [0]\n"
	"	B:	TRANSPOSITION/SHIFT application FLAG [0]\n"
	"		 Apply -P and -a to:\n"
	"		  source only -- prefilter (0),\n"  
	"		  or to  source and filter -- postfilter (1)\n"  


	"	    *** y(n) ****\n"
	"	F:	REVERB gain in decibels (func)[0.] \n"
	"	H:	REVERB frequency shift factor (bin frequency adder, before -P )(func) [0.] \n"
	"	P:	REVERB pitch transposition in semitones (func) [0]\n"
	"	Z:	REVERB decay time in seconds (func) [0.] \n"
	"	j:	REVERB random frequency deviation proportion (0 -> ?) (func) [0.]\n"
	"	K:	REVERB random frequency deviation lowpass \n"
	"			filter response time  (func) [0.]\n"

	"	    *** x(n) **** INPUT  AND EQ FILTER **\n"
	"	z:	REVERB (input) threshold in dB (0. > v >= -96.) (func) [-96.] \n"
	"	l:	REVERB (input) envelope attack time  (func) [0.]\n"
	"	L:	REVERB (input) envelope release time   (func) [0.]\n"
	"	O:	REVERB (input) EQ: Low shelf gain in dB (func)[0.] \n"
	"	Y:	REVERB (input) EQ: High shelf gain in dB (func) [0.] \n"
	"	d:	REVERB (input) EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	n:	REVERB (input) EQ: High shelf frequency in Hz (func) [2000.] \n"

	"	    *** y(n-1) **** FEEDBACK EQ FILTER **\n"
	"	T:	REVERB (feedback) EQ: decay time (func) [1.] \n"

	"	E:	REVERB (feedback): \n"
	"		    signal balance gain limiter level (0 to 96dB) [0.] \n"
	"		    (reverb EQ filter output is balanced against input)\n"
	"		     0 = balance off (balance gain limited to 0 dB),  \n"
	"		     96 = balance full-on (balance gain limited to 96 dB, unlimited)\n"

 	"	X:	REVERB (feedback) EQ: Low shelf gain in dB (func) [0.] \n"
	"	Q:	REVERB (feedback) EQ: High shelf gain in dB (func) [0.] \n"
	"	U:	REVERB (feedback) EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	m:	REVERB (feedback) EQ: High shelf frequency in Hz (func) [2000.] \n"

	"	    *** y(n) **** OUTPUT EQ FILTER **\n"
	"	k:	REVERB (output) EQ: Low shelf gain in dB (func) [0.] \n"
	"	c:	REVERB (output) EQ: High shelf gain in dB (func) [0.] \n"
	"	s:	REVERB (output) EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	G:	REVERB (output) EQ: High shelf frequency in Hz (func) [2000.] \n"

	"	    *******  FILTER **\n"
	"	y:	FILTER frequency response file \n"
	"	~:	FILTER analysis channel (1 -> ?) (0 = all) [0] \n"


	"	a:	FILTER data access mode [1]\n"
	"		   (0 = explicit time: Use -h )\n"
	"		   (1 = rate mode: Use -h as begin point, and -R as rate control) \n"
	"	h:	FILTER time point (func) [0.] \n"
	"	g:	FILTER time window: lower boundary (func) [0.] \n"
	"	J:	FILTER time window: upper boundary (func) [end of file] \n"
	"	/Q:	DATA time window: boundary flag  [0]\n"
	"		   0 = wrap time into window\n"
	"		   1 = fold time into window bounds\n"
	"		   2 = clip or limit time to nearest window boundary\n"
	"	::	DATA time window: trigger entry mode flag [0]\n"
	"		   Begin or trigger use of time window boundaries with first entry\n"
	"		   of time point into window bounds. 0 = off, 1 = on\n"
	"		    (upper boundary < 0. defaults to end of file)\n"
	"	R:	FILTER rate multiplier (func) [1.]\n"
	"		    (1. = rate of original, 2. = twice as fast, etc.)\n"
	"		    (negative = reverse,  0 = stationary) \n"

	"	W:	FILTER-spectrum compression threshold (in decibels) [0] \n"
	"	v:	FILTER-spectrum decibels of compression [0] \n"

	"	@:	DATA auto stop: 0 = off,  1 = on [0]\n"
	"		    (When on,  auto stop will terminate synthesis\n"
	"		      when a time boundary is crossed.)\n"



	"	o:	FILTER placement: for transfer function: y(n) = a * x(n) + b * y(n-1) \n"
	"		    0 = a,  1 = b [0] \n"
	"	u:	FILTER frequency response transposition in semitones (func) [0]\n"
	"	V:	FILTER frequency response shifter (before -U) (func) [0.] \n"
	"	x:	FILTER source decibels (reduces filtered signal in proportion)(func) [0.] \n"
	"	q:	FILTER warp index for reshaping frequency response (func) [0.] \n"
	"		    values > 0 close down or sharpen response, < 0 open it up\n"
	"	r:	FILTER (feedback) : decay time (func) [1.] \n"

	"	t:	oscillator resynthesis threshold in dB \n"
	"		    (both reverb and source) [-96]\n"
	"	w:	amplitude reports print mode: 0 = off, 1 = on [0]\n" 
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