#include "globals.h"


void usage() ; 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{


int i,j,   i1,  i2 ;
float i1p,  i2p,  pm, fm,  fs ; 
float nyquist;
double atof();
int R=44100, N=0, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0 ;
float P = 1.0;

float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *buffer_filter, *channel, *channel_delay, 
		*channel_filter,  *output ;
float *previous_channel_filter,  *F,   *FF ; 
//float lowfreq=0., hifreq=22050. ; 

float funcMin, funcMax, funcAvg, maxDelayT=0. ;  

int maxNumOfDelayFrames, frameNowChannelDelayIndex, thisFrameDelay, sourceflag=0 ; 


int warpflag=0 ; 
float threshfac = .001,  threshfacdB=-96 ;
float releasec,  minusreleasec,  attackc,  minusattackc ; 
double ar_dB ; 
float	gain=1. ;
int bandrejecton=0 ; 
FILE *fopen();

float N_ratio,  analysis_fundamental ; 
int analysis_N,  analysis_N2 ; 
float channelAmpSum, tempChannelAmpSum, filterChannelAmpSum, frameNormalizationAmpLimit, normalizationAmp, Normalize_to__Input_Sound_0__Filter_1=0 ; 

int print_flag=0 ; 
int pitchflag=0; 
char ch;
// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 

float  temp;  
float getthresh();
float   IR,  dur=0.;

float dBnumerator ; 
float fundamental, factor,   filterampfloor,  filtamp ;  


float  compthreshamp,  compthreshdB,  compamp,  compdB ; 
float  expthreshamp,  expthreshdB, expamp,  expdB ; 




char tempstring[ STRING_SIZE ] ; 

// FUNCTION DELAY TIME SCALER
struct func Function_Delay_Time_Scaler ; 

// SOURCE
struct func SOURCE_dB ; 
struct func SOURCE_ptrans ; 
struct func SOURCE_fshift ; 
struct func SOURCE_delayT ; 

// FILTER OUTPUT TIME DELAY
struct func FILTER_OUTPUT_time_delay ;

// FILTER OUTPUT SHIFT ADDER
struct  func  FILTER_OUTPUT_harmadd ; 

// FILTER OUTPUT GAIN
struct  func  FILTER_OUTPUT_dBgain ; 

// FILTER OUTPUT PITCH SEMITONE TRANSPOSE
struct  func  FILTER_OUTPUT_ptrans ; 

// FILTER FREQ RESPONSE
struct  func  FILTER_freq_response ; 

// FILTER FREQ RESPONSE SEMITONE TRANSPOSE
struct  func  FILTER_FREQ_RESPONSE_ftrans ; 

// FILTER FREQ RESPONSE TRANSPOSE SHIFTER
struct  func  FILTER_FREQ_RESPONSE_fshift ; 

//  FILTER DECIBELS FLOOR
struct  func  FILTER_dB_source_floor ; 

//  FILTER FREQ RESPONSE WARPSHAPE
struct  func  FILTER_FREQ_RESPONSE_warpshape ; 

//  FILTER OUTPUT RESPONSE ATTACK
struct  func  FILTER_OUTPUT_attack ; 

//  FILTER OUTPUT RESPONSE  RELEASE
struct  func  FILTER_OUTPUT_release ; 

//  FILTER FREQ RESPONSE SMOOTHING
struct  func  smoothingBW ; 

// FRAME NORMALIZATION DECIBEL LIMIT
struct  func  frameNormalizationDecibelLimit ; 



//*****************INITIALIZE

// FUNCTION DELAY TIME SCALER
Function_Delay_Time_Scaler.L = 1. ; Function_Delay_Time_Scaler.n = 1. ; Function_Delay_Time_Scaler.A[ 0 ] = 1. ; 


// SOURCE
SOURCE_dB.L = 1. ; SOURCE_dB.n = 1. ; SOURCE_dB.A[ 0 ] = 0. ; 
SOURCE_ptrans.L = 1. ; SOURCE_ptrans.n = 1. ; SOURCE_ptrans.A[ 0 ] = 0. ; 
SOURCE_fshift.L = 1. ; SOURCE_fshift.n = 1. ; SOURCE_fshift.A[ 0 ] = 0. ; 
SOURCE_delayT.L = 1. ; SOURCE_delayT.n = 1. ; SOURCE_delayT.A[ 0 ] = 0. ; 

// FILTER OUTPUT TIME DELAY
FILTER_OUTPUT_time_delay.L = 1. ; FILTER_OUTPUT_time_delay.n = 1. ; FILTER_OUTPUT_time_delay.A[ 0 ] = 0. ; 


// FILTER OUTPUT SHIFT ADDER
FILTER_OUTPUT_harmadd.L = 1. ; FILTER_OUTPUT_harmadd.n = 1. ; FILTER_OUTPUT_harmadd.A[ 0 ] = 0. ; 

// FILTER OUTPUT GAIN
FILTER_OUTPUT_dBgain.L = 1. ;  FILTER_OUTPUT_dBgain.n = 1. ; FILTER_OUTPUT_dBgain.A[ 0 ] = 0. ; 

// FILTER OUTPUT PITCH SEMITONE TRANSPOSE
FILTER_OUTPUT_ptrans.L = 1. ; FILTER_OUTPUT_ptrans.n = 1. ; FILTER_OUTPUT_ptrans.A[ 0 ] = 0. ; 


// FILTER FREQ RESPONSE
FILTER_freq_response.L = 1. ; FILTER_freq_response.n = 0. ; FILTER_freq_response.A[ 0 ] = 0. ; 

// FILTER SEMITONE TRANSPOSE
FILTER_FREQ_RESPONSE_ftrans.L = 1. ; FILTER_FREQ_RESPONSE_ftrans.n = 1. ; FILTER_FREQ_RESPONSE_ftrans.A[ 0 ] = 0. ; 

// FILTER TRANSPOSE SHIFTER
FILTER_FREQ_RESPONSE_fshift.L = 1. ; FILTER_FREQ_RESPONSE_fshift.n = 1. ; FILTER_FREQ_RESPONSE_fshift.A[ 0 ] = 0. ; 

//  FLTER DECIBELS FLOOR
FILTER_dB_source_floor.L = 1. ; FILTER_dB_source_floor.n = 1. ; FILTER_dB_source_floor.A[ 0 ] = -96. ; 

//  FILTER FREQ RESPONSE WARPSHAPE
FILTER_FREQ_RESPONSE_warpshape.L = 1. ; 
	FILTER_FREQ_RESPONSE_warpshape.n = 1. ; FILTER_FREQ_RESPONSE_warpshape.A[ 0 ] = 0. ; 

//  FILTER OUTPUT RELEASE
FILTER_OUTPUT_release.L = 1. ; FILTER_OUTPUT_release.n = 1. ; FILTER_OUTPUT_release.A[ 0 ] = 0. ; 

//  FILTER OUTPUT ATTACK
FILTER_OUTPUT_attack.L = 1. ; FILTER_OUTPUT_attack.n = 1. ; FILTER_OUTPUT_attack.A[ 0 ] = 0. ; 

//  FILTER SMOOTHING
smoothingBW.L = 1. ; smoothingBW.n = 1. ; smoothingBW.A[ 0 ] = 0. ; 

// FRAME NORMALIZATION DECIBEL LIMIT
frameNormalizationDecibelLimit.L = 1. ; 
	frameNormalizationDecibelLimit.n = 1. ; frameNormalizationDecibelLimit.A[ 0 ] = 0. ; 


if( argc < 2 )usage() ; 

//CASE -> USAGE
    while( (ch= crack( argc, argv, 
	"=|_|a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|W|x|X||Z|",
 0 )) != CRACK_DONE_FLAG ) {

/*

CASE -> USAGE
 

*/

	switch(ch) {
	    case 'N':   N = (int) crackfloat( arg_option, ch );
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'I':   tfactor = crackfloat( arg_option, ch );
			break;

	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'P':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_ptrans.fp = crackstring( tempstring, & FILTER_OUTPUT_ptrans ); 
			break;

	    case 'h':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_time_delay.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_time_delay );
			break;



	    case 'a':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_harmadd.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_harmadd );
			break;

	    case 'A':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_dBgain.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_dBgain );
			break;

	   case 'q':	strcpy(afile, arg_option); break;

           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;



	    case 'B':   pitchflag = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'H':   dBlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'X':   dBhi = crackfloat( arg_option, ch ) ;
			break;
	    case 'm':   freqlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'R':   freqhi = crackfloat( arg_option, ch ) ;
			break;

            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;
            case 'Z':	print_flag = (int) crackfloat( arg_option, ch ) ; break;

	    case 'x':   strcpy(tempstring, arg_option);
			Function_Delay_Time_Scaler.fp = crackstring( tempstring, 
			    &Function_Delay_Time_Scaler );
			break;



	    case 'F':   strcpy(tempstring, arg_option);
			FILTER_freq_response.fp = crackstring( tempstring, 
			    &FILTER_freq_response );
			break;



	    case 'J':   strcpy(tempstring, arg_option);
			SOURCE_dB.fp = crackstring( tempstring, 
			    & SOURCE_dB );
			break;
 
	    case 'r':   strcpy(tempstring, arg_option);
			SOURCE_ptrans.fp = crackstring( tempstring, 
			    &SOURCE_ptrans );
			break;

	    case 'u':   strcpy(tempstring, arg_option);
			SOURCE_fshift.fp = crackstring( tempstring, 
			    &SOURCE_fshift );
			break;

	    case 'U':   strcpy(tempstring, arg_option);
			SOURCE_delayT.fp = crackstring( tempstring, 
			    &SOURCE_delayT );
			break;



            case 'G':	bandrejecton = (int) crackfloat( arg_option, ch ) ; break;


	    case 'T':   strcpy(tempstring, arg_option);
			FILTER_FREQ_RESPONSE_ftrans.fp = crackstring( tempstring, 
			    &FILTER_FREQ_RESPONSE_ftrans );
			break;
	    case 'V':   strcpy(tempstring, arg_option);
			FILTER_FREQ_RESPONSE_fshift.fp = crackstring( tempstring, 
			    & FILTER_FREQ_RESPONSE_fshift );
			break;
	    case 'S':   strcpy(tempstring, arg_option);
			FILTER_dB_source_floor.fp = crackstring( tempstring, 
			    & FILTER_dB_source_floor );
			break;

	    case 'L':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_release.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_attack.fp = crackstring( tempstring, 
			    & FILTER_OUTPUT_attack );
			break;

	    case 'W':   strcpy(tempstring, arg_option);
			FILTER_FREQ_RESPONSE_warpshape.fp = crackstring( tempstring, 
			    & FILTER_FREQ_RESPONSE_warpshape );
			break;


	    case 'n':   strcpy(tempstring, arg_option);
			frameNormalizationDecibelLimit.fp = crackstring( tempstring, 
			    & frameNormalizationDecibelLimit );
			break;

	    case 'v':   Normalize_to__Input_Sound_0__Filter_1 = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'Q':   strcpy(tempstring, arg_option);
			smoothingBW.fp = crackstring( tempstring, 
			    &smoothingBW );
			break;


	    case 'c':   compthreshdB = crackfloat( arg_option, ch ) ;
			break;
	    case 'd':   compdB = crackfloat( arg_option, ch ) ;
			break;
	    case 'E':   expthreshdB = crackfloat( arg_option, ch ) ;
			break;
	    case 'g':   expdB = crackfloat( arg_option, ch ) ;
			break;




	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	    case 's':	sflag = 1;
			break;
	}
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "FILTER", 69 ) ; 
prline( 69,  "-" ) ; 




    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
 
    if( (SOURCE_dB.A[0] > -96.0) || ( SOURCE_dB.n != 1.) ) sourceflag = 1 ; 

   findFuncMinMaxAvg( &FILTER_OUTPUT_time_delay, &funcMin, &funcMax, &funcAvg ) ; 
   maxDelayT = funcMax ; 

    if( sourceflag == 1 ) {
	findFuncMinMaxAvg( &SOURCE_delayT, &funcMin, &funcMax, &funcAvg ) ; 
	if( funcMax > maxDelayT) maxDelayT = funcMax ; 
    } ; 

   ringTime = maxDelayT ; 

// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
    setupfiles(argc, argv) ; 

    endchan = beginchan + ochan ; 
    
// ******** FILTER FILE SETUPS
if( FILTER_freq_response.n < 1. ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A FILTER FUNCTION. BYE.\n" ) ;
    exit(EXIT_FAILURE);
}

    analysis_N = FILTER_freq_response.n - 2 ;
    analysis_N2 = analysis_N>>1 ; 

// ******** SET N TO ANALYSIS N IF 0
//    if( N <= 0 ) N = analysis_N ; 

    if( N != analysis_N )
    {
	prt( "\n\n=========== WARNING ==============\n"); 
	pri( analysis_N, "INPUT FREQRESPONSE ANALYSIS FFT SIZE" ) ; 
	pri( N, "FLITER FFT SIZE" ) ; 
	prt( "------> FFT SIZES DO NOT MATCH. <--------") ;
	prt( "CHANGING FILTER FFT SIZE TO MATCH INPUT FREQRESPONSE SIZE."); 
    } ;



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
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    IR = (float) I / (float) R ; 
    fundamental = (float) R / (float) N ; 
    factor = (float) R / ((float) D * TWOPI) ; 
    dBnumerator = (float) pow( (double) 10.0, (double) -3.9 ) ; 
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    
    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 


 //*****************
// SET UP SOME FILTER FILE VALUES
    analysis_fundamental = nyquist / (float) (analysis_N/2) ; 
    // MAKE RATIO OF ANALYSIS N TO SOURCE N FOR FILTER POSITIONING
    N_ratio = (float) analysis_N / (float) N ; 
    
//*****************
// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( 
	(FILTER_OUTPUT_ptrans.n  != 1.) || (FILTER_OUTPUT_harmadd.n  != 1.) || 
	    (FILTER_OUTPUT_ptrans.A[0] != 0.) || (FILTER_OUTPUT_harmadd.A[0] != 0.) ||
		(SOURCE_fshift.A[0] != 0.) || (SOURCE_fshift.n != 1.) || 
			(SOURCE_ptrans.A[0] != 0.) || (SOURCE_ptrans.n != 1.) ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
    
//***************** PRINT VALUES
prf( dur, "OUTPUT FILE: DURATION" ) ; 
//*****************
prbanner( "FREQUENCY RESPONSE",  69 ) ; 
pri( analysis_N,  "FREQRESPONSE: FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( analysis_fundamental, "FREQRESPONSE:     FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
//*****************

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



prp( &SOURCE_dB, "SOURCE: GAIN (in dB)" ); 
prp( &SOURCE_ptrans, "SOURCE: PITCH TRANSPOSITION (in semitones)" ); 
prp( &SOURCE_fshift, "SOURCE: FREQ SHIFT" ); 
prp( &SOURCE_delayT, "SOURCE: TIME DELAY" ); 


prp( &FILTER_OUTPUT_dBgain,  "FILTER: GAIN (in dB)"  ) ; 
prp( & FILTER_OUTPUT_ptrans,  "FILTER: PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &FILTER_OUTPUT_harmadd,  "FILTER: FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
if( bandrejecton ){
    prt( "FILTER TYPE: ............REJECT RESPONSE" ) ;
}else{
    prt( "FILTER TYPE: ............PASS RESPONSE" ) ;
}
prp( &FILTER_FREQ_RESPONSE_ftrans,  "FILTER FREQ RESPONSE: PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( & FILTER_FREQ_RESPONSE_fshift,  "FILTER FREQ RESPONSE: FREQUENCY SHIFT (in Hz)"  ) ; 
prp( & FILTER_dB_source_floor,  "FILTER: SOURCE SIGNAL FLOOR (in dB)"  ) ; 
prline( 1,  "*" ) ; 
prp( & FILTER_OUTPUT_attack,  "FILTER OUTPUT: ENVELOPE ATTACK TIME (in seconds)"  ) ; 
prp( & FILTER_OUTPUT_release,  "FILTER OUTPUT: ENVELOPE RELEASE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prp( & FILTER_FREQ_RESPONSE_warpshape, "FILTER FREQ RESPONSE: WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 
prp( &FILTER_OUTPUT_time_delay, "FILTER: TIME DELAY" ) ; 


prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
// *******
prf( compthreshdB,  "COMPRESSION THRESHOLD DECIBELS" ) ; 
prf( compdB,  "COMPRESSION DECIBELS" ) ; 
prf( expthreshdB,  "EXPANSION THRESHOLD DECIBELS" ) ; 
prf( expdB,  "EXPANSION DECIBELS" ) ; 
prline( 1,  "*" ) ; 
prp( &smoothingBW, "RESPONSE SMOOTHING (Q) FACTOR (IN FREQUENCY OR (NEGATIVE) OCTAVES)" ) ; 
prp( & frameNormalizationDecibelLimit, "FRAME NORMALIZATION DECIBEL LIMIT" ) ; 
if( Normalize_to__Input_Sound_0__Filter_1 == 0 )prt( "NORMALIZE TO INPUT SOUND" ); 
else prt( "NORMALIZE TO FILTER FRAME" ) ; 

prf( rescalev, "DECIBEL RESCALE VALUE" ) ; 

prp( &Function_Delay_Time_Scaler, "FUNCTION DELAY TIME SCALER" ) ; 

// **
prline( 1,  "*" ) ; 

prline( 1,  "*" ) ;


//***************** SET UP ARRAYS


    
    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( buffer_filter, N ) ;		/* FFT buffer */

    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( channel_filter, N+2 ) ; 
    fvec( output, Nw ) ;	/* output buffer */
    fvec( previous_channel_filter, N+2 ) ;	/* previous analysis channels */

// ADD DELAYED CHANNEL ARRAY HERE
    maxNumOfDelayFrames = 1 + (int)((maxDelayT * frames_per_sec) + 0.5) ; 
    fvec( channel_delay, maxNumOfDelayFrames * (N + 2) ) ; 

// MAKE THRESH AMP
    //threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	
    threshfac = dB_to_amp( threshfacdB ) ; 
    
// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

// ALLOCATE FILTER SPACE
fvec( F,  analysis_N+2  ) ;	/* filter array */
fvec( FF,  analysis_N+2  ) ;	/* filter array */
    if( FILTER_FREQ_RESPONSE_warpshape.n > 1 ) warpflag = 1 ; 

//************FIRST TIME: FILL ARRAY FROM FILE
    fillfunc( &FILTER_freq_response, F, (analysis_N + 2) ) ;     

// MAKE FILTER AMP SUM
   filterChannelAmpSum = 0. ; 	
   for( i = 0; i < (analysis_N+2) ; i+= 2 ) filterChannelAmpSum += F[i] ; 



//*************EQ AND NORMALIZE
    if(dBlow !=  dBhi)prt("...............EQUALIZING AND NORMALIZING INPUT FREQUENCY RESPONSE....." ) ; 
    else prt("...............NORMALIZING INPUT FREQUENCY RESPONSE....." ) ;
    eq( F,  (analysis_N + 2),  dBlow,  dBhi, freqlow,  freqhi,  analysis_fundamental, 1,  0, 1 ) ; 

//************ COMPANSION
    // FIND COMPRESSION AND EXPANSION THRESHOLDS AND AMPS
    compthreshamp = dB_to_amp( compthreshdB ) ; 
    compamp = dB_to_amp( compdB ) ; 
    expthreshamp = dB_to_amp( expthreshdB ) ; 
    expamp = 1. / dB_to_amp( expdB ) ; 

    if( compthreshamp <= expthreshamp ){
	fprintf( stderr, "\n\nCOMPRESSION THRESHOLD IS BELOW EXPANSION THRESHOLD!\n\nCOMPANSION SKIPPED!\n\n" ) ;
    }else{
	if( (compamp < 1.) || (expamp > 1. ) ){
	    if( (compamp <= 1.) && (expamp >= 1.) ){
		prt( "...............USING COMPRESSION" ) ; 
 
		 compand( F, (analysis_N + 2), compthreshamp, compamp, expthreshamp, expamp  ) ;
 
	    }else{
		prf( compdB,  "COMPRESSION DECIBELS" ) ; 
		prf( expdB,  "EXPANSION DECIBELS" ) ; 
		prt( "ILLEGAL COMPRESSION/EXPANSION VALUES" ) ;
		prt( "...................NO COMPRESSION" ) ; 
	    }
	}else{
	    prt( "...................NO COMPRESSION" ) ; 
	}  
    }
    
//*********** PRINT TO TERMINAL IF DESIRED ******
    if(print_flag)tprintspec( F, (analysis_N + 2), analysis_fundamental,  print_flag) ;


//************ CONVERT FREQUENCIES TO PHASE DIFFERENCES
    for(i = 1,  j = 0; i < (analysis_N + 2); i += 2,  j++) 
	F[ i ] = (F[ i ] - ((float) j * analysis_fundamental)) / factor ; 


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
		  convert( buffer, channel, N2, D, R ) ;	 

	// TRANSFER channel INTO CIRCULAR DELAY LINE
	frameNowChannelDelayIndex = frame_count ; 
	while( frameNowChannelDelayIndex >= maxNumOfDelayFrames ) 
		frameNowChannelDelayIndex -=  maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2) ; i++ ) 
		channel_delay[ (frameNowChannelDelayIndex * (N + 2)) + i] = channel[i] ; 


	// FILL channel_filter WITH APPROPRIATE TIME DELAYED FRAME. 
	// FILTER TIME DELAY 
	FILTER_OUTPUT_time_delay.A[ 0 ] = fval( &FILTER_OUTPUT_time_delay, dur, t );
 	thisFrameDelay = frameNowChannelDelayIndex - 
			(int)((FILTER_OUTPUT_time_delay.A[ 0 ] * frames_per_sec) + 0.5) ; 
	while( thisFrameDelay < 0) thisFrameDelay += maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2); i++ ) channel_filter[ i ] = channel_delay[ (thisFrameDelay * (N + 2)) + i] ; 


	// SETUP PREVIOUS CHANNEL
	    if( !frame_count )for(i = 0; i < (N + 2); i++)
		    previous_channel_filter[ i ] = channel_filter[ i ] ; 

//*************************
// GET THE VALUES
//*************************


//  SHIFT, GAIN, AND TRANSPOSITION
          
		Function_Delay_Time_Scaler.A[ 0 ] =  fval( &Function_Delay_Time_Scaler, dur, t );


		FILTER_OUTPUT_harmadd.A[ 0 ] =  fval( &FILTER_OUTPUT_harmadd, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		FILTER_OUTPUT_dBgain.A[ 0 ] =  fval( &FILTER_OUTPUT_dBgain, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		    gain = dB_to_amp( FILTER_OUTPUT_dBgain.A[ 0 ]  ) ; 	
		FILTER_OUTPUT_ptrans.A[ 0 ] = fval( &FILTER_OUTPUT_ptrans, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		    pm = semitones_to_mult( FILTER_OUTPUT_ptrans.A[ 0 ] ) ; 


		fs = FILTER_FREQ_RESPONSE_fshift.A[ 0 ] = fval( & FILTER_FREQ_RESPONSE_fshift, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		    // IF FREQ SHIFT FOR SOURCE ONLY, SUBTRACT FROM FILTER CHANGE
		    if( !pitchflag ) fs = fs - FILTER_OUTPUT_harmadd.A[ 0 ] ; 
		FILTER_FREQ_RESPONSE_ftrans.A[ 0 ] = fval( &FILTER_FREQ_RESPONSE_ftrans, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		    fm = semitones_to_mult( FILTER_FREQ_RESPONSE_ftrans.A[ 0 ] ) ;
		    // IF PITCH TRANSPOSITION FOR SOURCE ONLY, DIVIDE OUT OF FILTER CHANGE
		    if( !pitchflag ) fm = fm / pm ; 

		FILTER_dB_source_floor.A[ 0 ] = fval( & FILTER_dB_source_floor, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		    filterampfloor = dB_to_amp( FILTER_dB_source_floor.A[ 0 ]  ) ; 	
		    filtamp = 1. - filterampfloor ;  



		// FILTER WARP
		FILTER_FREQ_RESPONSE_warpshape.A[ 0 ] = fval( & FILTER_FREQ_RESPONSE_warpshape, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );

		// ATTACK
		FILTER_OUTPUT_attack.A[ 0 ] = fval( &FILTER_OUTPUT_attack, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		smooth_setup( FILTER_OUTPUT_attack.A[ 0 ], &attackc, &minusattackc, IR ) ;


		//RELEASE
		FILTER_OUTPUT_release.A[ 0 ] = fval( &FILTER_OUTPUT_release, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		smooth_setup( FILTER_OUTPUT_release.A[ 0 ], &releasec, &minusreleasec, IR ) ;





	    // FRAME NORMALIZATION DECIBEL LIMIT
		frameNormalizationDecibelLimit.A[ 0 ] = 
			fval( &frameNormalizationDecibelLimit, dur,  
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
		frameNormalizationAmpLimit = dB_to_amp( frameNormalizationDecibelLimit.A[ 0 ] ); 

//*** MODIFICATIONS
    
    //*************WARP AND BAND REJECT
    spectmagwarp2( F, FF,   (analysis_N + 2), FILTER_FREQ_RESPONSE_warpshape.A[ 0 ], 1 ) ;


     //*********** SMOOTH THE SPECTRUM
	smoothingBW.A[ 0 ] = fval( &smoothingBW, dur, 
               t - (Function_Delay_Time_Scaler.A[ 0 ] * FILTER_OUTPUT_time_delay.A[ 0 ] ) );
    smoothspec( FF, (analysis_N2 + 1), smoothingBW.A[ 0 ], R ) ; 

    //*************** IF BAND REJECT, THEN INVERT THE RESPONSE
    if( bandrejecton ) invertresponse( FF,  (analysis_N + 2),  0  ) ; 

 
    //********** FIND THE SUM OF THE AMPS FOR NORMALIZATION
    channelAmpSum = 0 ; 
    for( i = 1; i < (N + 2); i+= 2 ){
	channelAmpSum += channel_filter[i - 1] ; 
    }
 
 

//*****************
// MODIFICATIONS LOOP
//*****************


		for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){


//***************************************************
	    // FIND THE INDECES WHICH RESULT AFTER SHIFT AND TRANSPOSE OF 
	    // THE FILTER
		    // APPLY FILTER TRANSPOSE
		    // SHIFT
		    temp = (float) j ; // THE 0-(N/2) BIN WE ARE ON (TO BE FILTERED)
		    temp = temp * N_ratio ; // SHIFT BY RATIO OF ANALYSIS N TO SOURCE N
		    temp = temp -  ( fs / analysis_fundamental ) ; // SHIFT BY RATIO OF ANALYSIS N TO SOURCE N
		    // TRANSPOSE
		    temp =  temp / (fm)  ; // TRANSPOSE THE BIN
		    i2p = temp - (float)((int) temp ) ; // FRACTIONAL BIN VALUE
		    i1p = 1. - i2p ; // 1. - FRACTIONAL BIN VALUE
		    i1 = 2 * (int) temp ; // THE LOWER BIN AMP INDEX
		    i2 = i1 + 2 ; // THE UPPER BIN AMP INDEX
		    
		    if( i1 < 0 ){			
			//UNDER THE ARRAY
    			temp = FF[ 0 ]  ; 

		    }else if( i2 > (analysis_N - 2)  ){ 
			//OVER THE ARRAY
    			temp = FF[ analysis_N - 2 ]  ; 

		    }else{
			// IN ARRAY
			temp = ( FF[ i1 ] * i1p ) + (FF[ i2 ] * i2p ) ; 

		    }
//***************************************************
		    
		   // CHANGE AMP
			channel_filter[i - 1] *=  ((filtamp * temp) + filterampfloor );  

	}


    
	//  NORMALIZE AND REPLACE INTO CHANNEL.
		// FIND NORMALIZATION FACTOR.
	tempChannelAmpSum = 0. ; 	
	for( i = 0; i < (N + 2); i+= 2 ){
		tempChannelAmpSum += channel_filter[i] ;
	} ; 
		// NORMALIZE AND TRANSFER
	if( (tempChannelAmpSum > 0.0) && (frameNormalizationAmpLimit != 1.0 ) ){
		if( Normalize_to__Input_Sound_0__Filter_1 == 0 ){
			normalizationAmp = channelAmpSum / tempChannelAmpSum ;
		}else {
			normalizationAmp = filterChannelAmpSum / tempChannelAmpSum ;
		} ; 

		if( normalizationAmp > frameNormalizationAmpLimit )
			normalizationAmp = frameNormalizationAmpLimit ; 
		for( i = 0; i < (N + 2); i+= 2 )
			channel_filter[i] = channel_filter[i] * normalizationAmp ; 	
	} ; 





    // SMOOTH THE CHANGES TO THE SPECTRUM
    smooth( channel_filter, previous_channel_filter, (N + 2), attackc, minusattackc, releasec, minusreleasec ) ; 

	for( i = 1; i < (N + 2); i+= 2 ){

	    // SHIFT BY -a AND TRANSPOSE BY -P

	    temp = pm * (channel_filter[i] + FILTER_OUTPUT_harmadd.A[ 0 ]) ;
		    
	    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
	    if((temp <= 0.) || (temp >= nyquist)) channel_filter[i - 1] = 0. ; 
	    else channel_filter[i] = temp ; 
	    
	    channel_filter[i - 1] = channel_filter[i - 1] * gain ;  

	}

    // SOURCE
    if( sourceflag == 1){

	SOURCE_delayT.A[ 0 ] = fval( & SOURCE_delayT, dur, t );
 	thisFrameDelay = frameNowChannelDelayIndex - 
			(int)((SOURCE_delayT.A[ 0 ] * frames_per_sec) + 0.5) ; 
	while( thisFrameDelay < 0) thisFrameDelay += maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2); i++ ) channel[ i ] = channel_delay[ (thisFrameDelay * (N + 2)) + i] ; 

	SOURCE_dB.A[ 0 ] =  fval( & SOURCE_dB, dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * SOURCE_delayT.A[ 0 ]) );
	SOURCE_ptrans.A[ 0 ] =  fval( & SOURCE_ptrans, dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * SOURCE_delayT.A[ 0 ]) );
	    pm = semitones_to_mult( SOURCE_ptrans.A[ 0 ] ) ; 
		
	SOURCE_fshift.A[ 0 ] =  fval( & SOURCE_fshift, dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * SOURCE_delayT.A[ 0 ]) );

	for(i = 0; i < (N + 2); i += 2 ){
	    channel[i] *= dB_to_amp( SOURCE_dB.A[ 0 ] ) ; 
	    channel[i + 1] = (channel[i + 1] * pm) + SOURCE_fshift.A[ 0 ] ;
	    if( channel[i + 1] < 0. ){
		channel[i + 1] = 0;  channel[i] = 0. ; 
	    } ;   
	} ; 
    }; 




    if(sourceflag == 1){
	synt = getthresh( channel, N + 2, threshfac );
	temp = getthresh( channel_filter, N + 2, threshfac );
	if(temp > synt) synt = temp ; 
    }else{
	temp = getthresh( channel_filter, N + 2, threshfac );
    }



// ** OSCIL BANK OR OVERLAP/ADD OUT


    if(sourceflag == 1){ 
	// WITH BOTH SOURCE AND FILTER OUTPUT
        if ( obank ) { 
	    // PITCH CHANGE IN ONE OR THE OTHER: OSCILLATOR BANK FOR BOTH
	    noscbank2(channel_filter, N2, R, Nw, I, P,  output,  channel, N2 );
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;
	} else {
	    // OVERLAP-ADD
	    unconvert1( channel_filter, buffer_filter, N2, I, R ) ;
	    unconvert( channel, buffer, N2, I, R ) ;
	    // COMBINE BUFFERS
	    for(i = 0; i < N2; i++ ) buffer[i] += buffer_filter[i] ; 	    
	    rfft( buffer, N2, INVERSE ) ;
	    overlapadd( buffer, N, Wsyn, output, Nw, on ) ;
    	    shiftout( output, Nw, I, on, 0 ) ;
	
	}
    }else {
	// FILTER OUTPUT ONLY
	if ( obank ) {
	    // OSCIL BANK
	    noscbank(channel_filter, N2, R, Nw, I, P, output);
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;
	} else {
	    // OVERLAP-ADD
	    unconvert1( channel_filter, buffer, N2, I, R ) ;
	    rfft( buffer, N2, INVERSE ) ;
	    overlapadd( buffer, N, Wsyn, output, Nw, on ) ;
	    shiftout( output, Nw, I, on, 0 ) ;
	
	}
    } ; 

    frame_count++ ; 

    // FRAMES LOOP END

    }
    // FLUSH OUT AND CLOSE OUTPUT FILE
    shiftout( output, Nw, I, 1, 1 ) ;

    
// CHANNELS LOOP END
} 

    // CLOSE  INPUT FILE
    if(ifd)fclose(ifd);  


    fprintf(stderr,"\n\nFILTER : RESYNTHESIS COMPLETED\n");


    if( FILTER_OUTPUT_harmadd.n != 1. ) fclose(FILTER_OUTPUT_harmadd.fp ) ;
    if( FILTER_OUTPUT_dBgain.n != 1. ) fclose(FILTER_OUTPUT_dBgain.fp ) ;
    if( FILTER_OUTPUT_ptrans.n != 1. ) fclose(FILTER_OUTPUT_ptrans.fp ) ;
    if( FILTER_freq_response.n != 1. ) fclose(FILTER_freq_response.fp ) ;
    if( FILTER_FREQ_RESPONSE_ftrans.n != 1. ) fclose(FILTER_FREQ_RESPONSE_ftrans.fp ) ;
    if( FILTER_FREQ_RESPONSE_fshift.n != 1. ) fclose(FILTER_FREQ_RESPONSE_fshift.fp ) ;
    if( FILTER_dB_source_floor.n != 1. ) fclose(FILTER_dB_source_floor.fp ) ;
    if( FILTER_FREQ_RESPONSE_warpshape.n != 1. ) fclose(FILTER_FREQ_RESPONSE_warpshape.fp ) ;
    if( FILTER_OUTPUT_release.n != 1. ) fclose(FILTER_OUTPUT_release.fp ) ;
    if( FILTER_OUTPUT_attack.n != 1. ) fclose(FILTER_OUTPUT_attack.fp ) ;
    if( smoothingBW.n != 1. ) fclose(smoothingBW.fp ) ;



    exit(EXIT_SUCCESS) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"filter:  fixed-spectrum,  phase vocoder filter\n"
	"filter   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	N:	"FFT_LENGTH 		// N
	"		FFT length match input freqresponse analysis length.\n"
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec
	"	I:	"TIME_FACTOR		// tfactor

	"	P:	"PITCH_TRANS		// FILTER_OUTPUT_ptrans
	"	a:	"FREQ_SHIFT		// FILTER_OUTPUT_harmadd

	"	B:	"TRANSPOSITION_SHIFT  

	"	A:	"DB_GAIN

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout

	"	    "SOURCE_HEADER
	"	r:	"SOURCE_PITCH_TRANS
	"	u:	"SOURCE_FREQ_SHIFT
	"	J:	"SOURCE_DB_GAIN
	"	U:	"SOURCE_TIME_DELAY


	"	F:	FILTER: single channel frequency response file \n"
	"	G:	FILTER: filtering method: (POST everything)\n"
	"		 0 = pass, 1 = reject (inverted response) [0] \n"

	"	Z:	"FREQUENCY_RESPONSE_PRINTOUT


	"	T:	"FILTER_FREQ_RESPONSE_TRANSP
	"	V:	"FILTER_FREQ_RESPONSE_SHIFT
	"	S:	"FILTER_DB_FLOOR


	"	     "FILTER_SHELF_EQ_HEADER
	"		(PRE-compression/expansion and transposition/shift)\n"
	"	H:	"FILTER_SHELF_EQ_LOW_GAIN
	"	X:	"FILTER_SHELF_EQ_HIGH_GAIN
	"	m:	"FILTER_SHELF_EQ_LOW_FREQ
	"	R:	"FILTER_SHELF_EQ_HIGH_FREQ

	"	Z:	FILTER FREQUENCY RESPONSE: COMPRESSION/EXPANSION:\n"
	"		(POST-EQ and PRE-warp)\n"
	"	c:	compression threshold in decibels (0 to -96) [0]\n"
	"	d:	decibels of compression (0 to -96) [0]\n"
	"	E:	expansion threshold in decibels (0 to -96) [-96]\n"
	"	g:	decibels of expansion (0 to -96) [0]\n"

	"	W:	FILTER FREQUENCY RESPONSE: \n"
	"		warp index for reshaping magnitude response (func) [0.]\n"
	"		(POST-compression/expansion and transposition/shift)\n"
	"		    values > 0 expand the response's dynamic range, \n"
	"		    values < 0 compress the the response's dynamic range \n"

	"		FILTER FREQUENCY RESPONSE PROCESSING CHAIN:\n"
	"		    EQ -> COMPANDING -> WARP -> INVERSION -> SMOOTHING -> NORMALIZATION\n"

	"	Q:	"FILTER_FREQ_RESPONSE_SMOOTHING_BW

	"	h:	FILTER: time delay in seconds (func) [0]\n"
	"	x:	function delay time scaler (func) [0]\n"


	"	    "FRAME_NORMALIZATION
	"	n:	"FRAME_NORMALIZATION_DB_LIMIT   

	"	v:	"FRAME_NORMALIZATION_REFERENCE

	"	l:      "AMP_ATT_TIME
	"	L:      "AMP_RELEASE_TIME

	"	p:	"AMP_REPORTS 
	"	i:	"AMP_REPORTS_TIME_INTERVAL 

	"	_:	 "AUTO_PLAY 
	
	"	q:	Optional Analysis Sound File Source (for playback comparison only) [none]\n"


	"	=:	 "RESCALE_LEVEL 


	"	t:	"RESYNTH_THRESHOLD
	); // DONE


    exit(EXIT_SUCCESS);





}



void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
