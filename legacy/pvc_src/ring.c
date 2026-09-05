#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j ;
float nyquist,  basefreq ;
double atof(),  DD ;
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0 ;
float P = 1.0;
FILE *fopen();
char ch,  tempstring[ STRING_SIZE ] ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel, *output ;
float threshfac = .001,  threshfacdB=-96 ;
float  gain,    f  ;
float  *previous_channel, *previous_buffer, *next_channel,  *next_buffer,
    *feedback_channel,  *feedback_buffer,  *bufferout, 
    *binfreq, *phasediff,      temp,  temp2, temp3,   
    source_pm,  feedback_pm,  source_gain,  feedback_gain, master_gain; 
float envattack,  envrelease,  minusattack,  minusrelease  ;  
float getthresh();

float low, hi, avg, median ; int length ; 


double ar_dB ; 

float feedbackthresh,  feedt,  minusfeedbacklowpass, feedlowpass, feedlevel, 
       *freqdither,  freqdithernow ; 
float minusfeedbackalowpass, feedalowpass,  
       *dBdither,  dBdithernow ; 
int	feedback_thresh_mode=1 ; 

float fundamental,  factor ; 
float   IR, DR,  dur=0.;

float  FEEDBACK_dBhitemp,  FEEDBACK_dBlowtemp ; 
float prebalancesum,  postbalancesum, balancelimitdB=0,  balancelimitamp=0  ; 
int limitcount=0,  balanceflag=0 ; 

//**
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
  

//**
//**
// FEEDBACK LOOP EQ HIGH SHELF: DB
struct  func  FEEDBACK_dBhi  ;  

// FEEDBACK LOOP  EQ HIGH SHELF: FREQ
struct  func   FEEDBACK_freqhi ;   
  
// FEEDBACK LOOP  EQ LOW SHELF: DB
struct  func  FEEDBACK_dBlow  ;  

// FEEDBACK LOOP  EQ LOW SHELF: FREQ
struct  func   FEEDBACK_freqlow ;   
  
// FEEDBACK LOOP EQ DECAY TIME
struct  func   FEEDBACK_decay_time  ;


//**
 
 


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


//**** FEEDBACK 
// FEEDBACK GAIN
struct  func  feedback_dBgain ; 

// FEEDBACK FREQUENCY SHIFT ADDER
struct  func  feedback_harmadd ; 

// FEEDBACK PITCH MULTIPLIER
struct  func  feedback_ptrans ; 

// FEEDBACK THRESHOLD
struct  func  feedback_threshdB ;

// FEEDBACK DECAY TIME IN SECONDS
struct  func  feedback_level ;

// RANDOM STUFF

// FEEDBACK RANDOM FREQ DEVIATION PROPORTION
struct  func  feedback_dither ;


// FEEDBACK RANDOM FREQ DEVIATION RESPONSE TIME
struct  func  feedback_lowpass ;

// FEEDBACK RANDOM AMP DEVIATION DB FLOOR
struct  func  feedback_dBdither ;


// FEEDBACK RANDOM AMP DEVIATION RESPONSE TIME
struct  func  feedback_alowpass ;



//  RELEASE
struct  func  release ; 

//  ATTACK
struct  func  attack ; 

//#include "underflow.h"


//*****************INITIALIZE


//**
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
  

//**
//**
// FEEDBACK EQ HIGH SHELF: DB
FEEDBACK_dBhi.L = 1. ;  FEEDBACK_dBhi.n = 1. ; FEEDBACK_dBhi.A[ 0 ] = 0. ; 

// FEEDBACK EQ HIGH SHELF: FREQ
FEEDBACK_freqhi.L = 1. ;  FEEDBACK_freqhi.n = 1. ; FEEDBACK_freqhi.A[ 0 ] = 2000. ; 
  
// FEEDBACK EQ LOW SHELF: DB
FEEDBACK_dBlow.L = 1. ;  FEEDBACK_dBlow.n = 1. ; FEEDBACK_dBlow.A[ 0 ] = 200. ; 

// FEEDBACK EQ LOW SHELF: FREQ
FEEDBACK_freqlow.L = 1. ;  FEEDBACK_freqlow.n = 1. ; FEEDBACK_freqlow.A[ 0 ] = 0. ; 
  
// FEEDBACK LOOP EQ DECAY TIME
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

//**** FEEDBACK 

// FEEDBACK GAIN
feedback_dBgain.L = 1. ;  feedback_dBgain.n = 1. ; feedback_dBgain.A[ 0 ] = 0. ; 

// FEEDBACK FREQUENCY SHIFT ADDER
feedback_harmadd.L = 1. ; feedback_harmadd.n = 1. ; feedback_harmadd.A[ 0 ] = 0. ; 

// FEEDBACK PITCH MULTIPLIER
feedback_ptrans.L = 1. ; feedback_ptrans.n = 1. ; feedback_ptrans.A[ 0 ] = 0. ; 

// FEEDBACK THRESHOLD
feedback_threshdB.L = 1. ; feedback_threshdB.n = 1. ; feedback_threshdB.A[ 0 ] = -96. ; 

// FEEDBACK DECAY TIME IN SECONDS
feedback_level.L = 1. ; feedback_level.n = 1. ; feedback_level.A[ 0 ] = 0. ; 

// FEEDBACK RANDOM FREQ DEVIATION PROPORTION
feedback_dither.L = 1. ; feedback_dither.n = 1. ; feedback_dither.A[ 0 ] = 0. ; 

// FEEDBACK RANDOM FREQ DEVIATION LOWPASS FILTER COEFFICIENT
feedback_lowpass.L = 1. ; feedback_lowpass.n = 1. ; feedback_lowpass.A[ 0 ] = 0. ; 

// FEEDBACK RANDOM AMP DEVIATION DB FLOOR
feedback_dBdither.L = 1. ; feedback_dBdither.n = 1. ; feedback_dBdither.A[ 0 ] = 0. ; 

// FEEDBACK RANDOM FREQ DEVIATION LOWPASS FILTER COEFFICIENT
feedback_alowpass.L = 1. ; feedback_alowpass.n = 1. ; feedback_alowpass.A[ 0 ] = 0. ; 


//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 


if( argc < 2 )usage() ; 


     while( (ch= crack( argc, argv,
    "R|N|M|P|D|t|I|F|f|s|a|A|Y|w|i|W|n|_|=|O|p|C|e|E|U|T|j|z|H|V|X|q|c|G|k|S|K|Z|Q|x|T|m|d|B|L|l|b|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = atoi(arg_option);
			break;
	    case 'M':   Nw = atoi(arg_option);
			break;
	    case 'W':   window_type = atoi(arg_option) ;
			break;
	    case 'D':   frames_per_sec = atof(arg_option);
			break;
	    case 'I':   tfactor = atof(arg_option);
			break;

	    case 'b':   begint = atof(arg_option) ;
			break;
	    case 'e':   endt = atof(arg_option) ;
			break;

	    case 'C':   channelout = atoi(arg_option) ;
			break;

	    case 't':   threshfacdB = atof(arg_option);
			break;
//MASTER
	    case 'A':   strcpy(tempstring, arg_option);
			master_dBgain.fp = crackstring( tempstring, &master_dBgain );
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



// FEEDBACK

	    case 'E':   balancelimitdB = atof(arg_option) ;
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
	    case 'V':   feedback_thresh_mode = atoi(arg_option) ;
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

	    case 'x':   strcpy(tempstring, arg_option);
			feedback_dBdither.fp = crackstring( tempstring, &feedback_dBdither );
			break;
	    case 'q':   strcpy(tempstring, arg_option);
			feedback_alowpass.fp = crackstring( tempstring, &feedback_alowpass );
			break;

           case '_':	autoplayreps = atoi(arg_option) ; break;

           case '=':	rescalev = atof(arg_option) ; break;



			// FEEDBACK HIGH SHELF EQ
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


	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;

            case 'w':	quiet = atoi(arg_option) ; 
			break;
            case 'i':	ampstatinc = atof(arg_option) ; 
			break;

} }





prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "RING: REVERBERATOR/RESONATOR", 69 ) ; 
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

//prf( ringTime, "ringTime" ) ; 
// &FEEDBACK_decay_time, &feedback_level 



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
    fundamental = (float) R / (float) N ; 
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    factor = R/(D*TWOPI);
    DD = (double) D ; 
// REVERB TIME STUFF
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    DR = (float) D / (float) R ; 
    IR = (float) I / (float) R ;     

    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 

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

prp( &master_dBgain,  "MASTER GAIN (in dB)"  ) ; 
prline( 1,  "*" ) ; 
prp( &source_dBgain,  "SOURCE GAIN (in dB)"  ) ; 
prp( &source_ptrans,  "SOURCE PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &source_harmadd,  "SOURCE FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1, "*" ) ;  
prp( &feedback_dBgain,  "FEEDBACK GAIN (in dB)"  ) ; 
prp( &feedback_ptrans,  "FEEDBACK PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &feedback_harmadd,  "FEEDBACK FREQUENCY SHIFT (in Hz)"  ) ; 
prp( &feedback_threshdB,  "FEEDBACK THRESHOLD (in dB)"  ) ; 
pri( feedback_thresh_mode,  "REVERB THRESHOLD PASS MODE (0 = below, 1 = above)" ) ; 
prp( &FEEDBACK_decay_time,  "REVERB INSIDE LOOP FILTER DECAY TIME (in seconds) "  ) ;
prp( &feedback_level,  "REVERB DECAY TIME (in seconds)"  ) ; 
prp( &feedback_dither,  "FEEDBACK: RANDOM FREQUENCY DEVIATION (in Hz)"  ) ; 
prp( &feedback_lowpass,  "FEEDBACK: RANDOM FREQUENCY DEVIATION  RESPONSE TIME (in seconds)"  ) ; 
prp( &feedback_dBdither,  "FEEDBACK: RANDOM AMPLITUDE DEVIATION FLOOR (in dB)"  ) ; 
prp( &feedback_alowpass,  "FEEDBACK: RANDOM AMPLITUDE DEVIATION LOWPASS RESPONSE TIME (in seconds)"  ) ; 
prline( 1, "*" ) ;  
prp( &attack,  "REVERB: ENVELOPE ATTACK TIME (in seconds)"  ) ; 
prp( &release,  "REVERB: ENVELOPE RELEASE TIME (in seconds)"  ) ; 
prline( 1, "*" ) ;  

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

prf( rescalev, "DECIBEL RESCALE VALUE" ) ; 



prline( 69, "*" ) ;  
   fprintf( stderr,  "\n\n" ) ; 
    // *******





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

    fvec( dBdither, N2+1 ) ;	/* previous dB deviation */

    fvec( bufferout, N ) ;		/* FFT buffer */

// MAKE THRESH AMP
    threshfac = dB_to_amp( threshfacdB ) ;	

// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

// MAKE BIN FREQ ARRAY
    for(j = 0; j < (N2 + 1); j++){
	binfreq[j] = fundamental * (float) j ; 
    }

// ZERO ARRAYS
    for(j = 0; j < (N2 + 1); j++){
	freqdither[j] = 0. ; dBdither[j] = 0. ; 
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

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; 



    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;
	
//*********************************************
// LOOP FOR FRAMES
//*********************************************

    while ( !eof ) {
	in += D ;
	on += I ;
	timenow( dur ) ;

	 	  eof = shiftin( input, Nw, D ) ;
	  	  fold( input, Wanal, Nw, buffer, N, in ) ;
	  	  rfft( buffer, N2, FORWARD ) ;


	// SETUP PREVIOUS CHANNEL
//	    if( !frame_count )for(i = 0; i < (N + 2); i++)
//		    previous_channel[ i ] = channel[ i ] ; 
//*************************
// GET THE VALUES
//*************************
	    // MASTER
		master_dBgain.A[ 0 ] = 
		    fval( &master_dBgain, dur, t );
		master_gain =  dB_to_amp( master_dBgain.A[ 0 ] );	


	    // SOURCE
		source_dBgain.A[ 0 ] = 
		    fval( &source_dBgain, dur, t );
		source_harmadd.A[ 0 ] = 
		    fval( &source_harmadd, dur, t );
		source_ptrans.A[ 0 ] = 
		    fval( &source_ptrans, dur, t ) ;

		source_pm = semitones_to_mult( source_ptrans.A[ 0 ] ) ;
		source_gain = dB_to_amp( source_dBgain.A[ 0 ] );	

	    // FEEDBACK
		feedback_dBgain.A[ 0 ] = 
		    fval( &feedback_dBgain, dur, t );
		feedback_harmadd.A[ 0 ] = 
		    fval( &feedback_harmadd, dur, t );
		feedback_ptrans.A[ 0 ] = 
		    fval( &feedback_ptrans, dur, t ) ;

		feedback_level.A[ 0 ] = 
		    fval( &feedback_level, dur, t );
		smooth_setup( feedback_level.A[ 0 ], &feedlevel, &temp, IR ) ; 


		// RANDOM DEVIATION
		// FREQ
		feedback_dither.A[ 0 ] = 
		    fval( &feedback_dither, dur, t ) ;
//		freqdithernow = feedback_dither.A[ 0 ] * fundamental ; 

		feedback_lowpass.A[ 0 ] = 
		    fval( &feedback_lowpass, dur, t );
		smooth_setup( feedback_lowpass.A[ 0 ], &feedlowpass, &minusfeedbacklowpass, IR ) ; 

		// AMP
		feedback_dBdither.A[ 0 ] = 
		    fval( &feedback_dBdither, dur, t ) ;
		dBdithernow = feedback_dBdither.A[ 0 ] / 2.  ; 

		feedback_alowpass.A[ 0 ] = 
		    fval( &feedback_alowpass, dur, t );
		smooth_setup( feedback_alowpass.A[ 0 ], &feedalowpass, &minusfeedbackalowpass, IR ) ; 


		feedback_threshdB.A[ 0 ] =  
		    fval( &feedback_threshdB, dur, t ) ;


		feedback_pm = semitones_to_mult( feedback_ptrans.A[ 0 ] ) ;
		feedback_gain =dB_to_amp( feedback_dBgain.A[ 0 ] );	
		// MAKE FEEDBACK THRESH AMP
		feedbackthresh = dB_to_amp( feedback_threshdB.A[ 0 ] ); 	


		release.A[ 0 ] = fval( &release, dur, t );
		smooth_setup( release.A[ 0 ], &envrelease, &minusrelease, IR ) ; 

		attack.A[ 0 ] = fval( &attack, dur, t );
		smooth_setup( attack.A[ 0 ], &envattack, &minusattack, IR ) ; 

//****************************
//*** FILTER ANALYSIS FOR INPUT TO FEEDBACK x(n) "filtered"		

	    // COPY BUFFER INTO FEEDBACK BUFFER
		 for( i = 0; i < N; i++ ) feedback_buffer[i] = buffer[i]; 

	    // LEAN CONVERT FEEDBACK BUFFER
	    leanconvert( feedback_buffer, feedback_channel,   N2, D, R ) ; 

//**** THRESHOLD ENVELOPE GOES HERE
		// GET FEEDBACK THRESHOLD 
		feedt = getthresh( feedback_channel, (N + 2), feedbackthresh );


		    if(  feedback_thresh_mode == 1 ){


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


		    }else{


		      // LOOP FOR BINS
		      for( i = 1, j = 0;  i < (N + 2); i += 2,  j++ ){

			//******** ZERO AMPS BELOW THRESHOLD RELATIVE TO THIS FRAME
			if( feedback_channel[ i - 1 ] > feedt ){ 
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













	    //**** LEAN UNCONVERT FEEDBACK CHANNEL TO BUFFER FOR ADDING IN
		    leanunconvert( feedback_channel, feedback_buffer,  N2, D, R ) ; 


//** ADD IN PREVIOUS BUFFER
		 for( i = 0; i < N; i++ ){
		    feedback_buffer[i] = 
			    feedback_buffer[i] +
			   (feedlevel * next_buffer[i] ); 
		 }		
		// CONVERT  FEEDBACK BUFFER TO AMP AND PHASE
		leanconvert( feedback_buffer, feedback_channel,   N2, D, R ) ; 


		// FEEDBACK EQ
		FEEDBACK_dBhi.A[ 0 ] =  fval( &FEEDBACK_dBhi, dur, t ) ;
		FEEDBACK_freqhi.A[ 0 ] =  fval( &FEEDBACK_freqhi, dur, t ) ;
		FEEDBACK_dBlow.A[ 0 ] =  fval( &FEEDBACK_dBlow, dur, t ) ;
		FEEDBACK_freqlow.A[ 0 ] =  fval( &FEEDBACK_freqlow, dur, t ) ;
		FEEDBACK_decay_time.A[ 0 ] =  fval( &FEEDBACK_decay_time, dur, t ) ;
		    // MODIFY FEEDBACK EQ DB BY FRAME RATE
		    FEEDBACK_dBhitemp = (FEEDBACK_dBhi.A[ 0 ] * IR ) / FEEDBACK_decay_time.A[ 0 ] ; 
		    FEEDBACK_dBlowtemp = (FEEDBACK_dBlow.A[ 0 ] * IR ) / FEEDBACK_decay_time.A[ 0 ] ; 


// DECAYING SIGNAL BALANCE
    // FIND SUM OF ALL MAGNITUDES
		if( (FEEDBACK_dBhitemp != 0.) && (balanceflag)){
		    prebalancesum = 0 ; 
		    for( i = 0; i < (N + 2); i+= 2) prebalancesum += feedback_channel[i] ; 
		}
		
		
				// INSIDE FEEDBACK LOOP EQ
		eq( feedback_channel,  (N + 2),  
		    FEEDBACK_dBlowtemp,    FEEDBACK_dBhitemp, 
		    FEEDBACK_freqlow.A[ 0 ],  FEEDBACK_freqhi.A[ 0 ],  
		    fundamental, 1,  0, 0 ) ; 


// EXPERIMENTAL DECAYING SIGNAL BALANCE
    // FIND SUM OF ALL MAGNITUDES
		if( balanceflag ){

		  if( (FEEDBACK_dBhitemp != 0.) || (FEEDBACK_dBlowtemp != 0.)){
		    if( !frame_count )fprintf( stderr, "\n\n..USING BALANCE ON DECAY FILTER AND/OR EQ....." ) ; 
			    postbalancesum = 0 ; 
		    for( i = 0; i < (N + 2); i+= 2) postbalancesum += feedback_channel[i] ; 
		
    // ******* NOW BALANCE
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
		if( obank ){
	  	    convert1( feedback_buffer, feedback_channel, N2, D, R ) ;
		}else{
	  	    leanconvert( feedback_buffer, feedback_channel, N2, D, R ) ;
		}
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
 




//** HERE!
//***** $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	//********* FEEDBACK CHANGES
// ** WORK ZONE
		//*** RANDOM FREQ DITHER
		    if(feedback_dither.A[ 0 ] != 0.){
			// DITHER
			temp = randf( -(feedback_dither.A[ 0 ]), feedback_dither.A[ 0 ] ) ;

			freqdither[ j ] = (feedlowpass * freqdither[ j ]) +
			    (minusfeedbacklowpass * temp) ;
			    
			    
			feedback_channel[i] = feedback_channel[i] + freqdither[ j ] ; 
		    }




		//*** RANDOM DB DITHER
		    if(dBdithernow != 0.){
			// DITHER 
			temp = randf( dBdithernow, -(dBdithernow) ) ;

			dBdither[ j ] = (feedalowpass * dBdither[ j ]) +
			    (minusfeedbackalowpass * temp) ;
			    
			feedback_channel[i - 1] = 
			    feedback_channel[i - 1] * dB_to_amp( dBdither[ j ] ) ; 
		    }
			    

// ** END WORK ZONE
//***** $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$








		// SET FREQ
		    temp = feedback_pm  * (feedback_harmadd.A[ 0 ]  + feedback_channel[i]) ;
		    if((temp > 0.) && (temp < nyquist)) feedback_channel[i] = temp ;
		    else feedback_channel[i - 1] = 0. ;  


		// ADD GAIN
		    feedback_channel[i - 1] = feedback_channel[i - 1] * feedback_gain * master_gain ;  
		    
    //****		    
	//********* SOURCE CHANGES

		    temp = source_pm  * (source_harmadd.A[ 0 ]  + channel[i]) ;
		    if((temp > 0.) && (temp < nyquist)) channel[i] = temp ;
		    else channel[i - 1] = 0. ;  

		    // ADD GAIN
		    channel[i - 1] = channel[i - 1] * source_gain * master_gain ;  
		    
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

    if( INPUT_dBhi.n != 1. ) fclose(INPUT_dBhi.fp ) ;
    if( INPUT_freqhi.n != 1. ) fclose(INPUT_freqhi.fp ) ;
    if( INPUT_dBlow.n != 1. ) fclose(INPUT_dBlow.fp ) ;
    if( INPUT_freqlow.n != 1. ) fclose(INPUT_freqlow.fp ) ;
    if( OUTPUT_dBhi.n != 1. ) fclose(OUTPUT_dBhi.fp ) ;
    if( OUTPUT_freqhi.n != 1. ) fclose(OUTPUT_freqhi.fp ) ;
    if( OUTPUT_dBlow.n != 1. ) fclose(OUTPUT_dBlow.fp ) ;
    if( OUTPUT_freqlow.n != 1. ) fclose(OUTPUT_freqlow.fp ) ;
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
    if( feedback_dBdither.n != 1. ) fclose(feedback_dBdither.fp ) ;
    if( feedback_alowpass.n != 1. ) fclose(feedback_alowpass.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;

   
    fprintf(stderr,"\nRING: RESYNTHESIS COMPLETED\n");
    exit(EXIT_SUCCESS) ;

}

void usage()
{
    fprintf(stderr, "%s",
	"ring:  phase vocoder reverberator/resonator \n"
	"ring   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	N:	FFT length (must be a power of 2) [1024]\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	W:	window type: 0 = hamming,  1 = rectangular  \n"
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
	"	f:	SOURCE frequency shift factor (bin frequency adder, before -p )(func) [0.] \n"
	"	p:	SOURCE pitch transposition in semitones (func) [0]\n"

	"	    *** y(n) ****\n"
	"	F:	REVERB gain in decibels (func)[0.] \n"
	"	H:	REVERB frequency shift factor (bin frequency adder, before -P )(func) [0.] \n"
	"	P:	REVERB pitch transposition in semitones (func) [0]\n"
	"	Z:	REVERB decay time in seconds (func) [0.] \n"
	"	j:      REVERB random frequency deviation in Hz (0 -> ?) (func) [0.]\n"
	"	K:      REVERB random frequency deviation lowpass \n"
	"			filter response time  (func) [0.]\n"

	"	x:      REVERB random amplitude deviation floor in dB (0 -> -96) (func) [0.]\n"
	"	q:      REVERB random amplitude deviation lowpass \n"
	"			filter response time  (func) [0.]\n"

	"	    *** x(n) **** INPUT  AND EQ FILTER (pre-transpose/shift) **\n"
	"	z:	REVERB (input) threshold in dB (0. > v >= -96.) (func) [-96.] \n"
	"	V:	REVERB threshold pass mode [1] \n"
	"		  (1 = pass bins > threshold,  0 = pass bins < threshold)\n"
	"	l:      REVERB (input) envelope attack time  (func) [0.]\n"
	"	L:      REVERB (input) envelope release time   (func) [0.]\n"

	"	O:	REVERB (input) EQ: Low shelf gain in dB (func) [0.] \n"
	"	Y:	REVERB (input) EQ: High shelf gain in dB (func) [0.] \n"
	"	d:	REVERB (input) EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	n:	REVERB (input) EQ: High shelf frequency in Hz (func) [2000.] \n"

	"	    *** y(n-1) **** FEEDBACK EQ FILTER  (pre-transpose/shift)  **\n"
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


	"	    *** y(n) **** OUTPUT EQ FILTER  (pre-transpose/shift)  **\n"
	"	k:	REVERB (output) EQ: Low shelf gain in dB (func) [0.] \n"
	"	c:	REVERB (output) EQ: High shelf gain in dB (func) [0.] \n"
	"	s:	REVERB (output) EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	G:	REVERB (output) EQ: High shelf frequency in Hz (func) [2000.] \n"


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