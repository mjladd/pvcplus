#include "globals.h"

// 4-23-2011

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k,  itemp ;
float nyquist;
double atof();
  int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  achannelout ;
int analysis_N,  analysis_D, analysis_R, analysis_chan,   niframes ;  
float analysis_dur,  iframes_per_sec ; 
float P = 1.0;
FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *previous_buffer, *channel,  
    *C_buffer, *C_channel, *output, *channelMix, *bufferMix ;
float *previous_channel,  *F,  *Fbuffer,  *F_lower,  *F_higher ; 
float threshfac = .001,  threshfacdB=-96 ;
float	pm, fm, panc,  pans, panpot, 
    A_dB,  B_dB,  C_dB,    gain=1. ;
float  temp,  temp2,  temp3 ;  
float getthresh();
float peakbinamp = 0.,  avgbinamp=0.,  peakamp ;
float   IR,  dur=0.;
float fundamental ;
float warpshape=0. ; 
int smooth__polar_0__Cartesian_1=0 ; 
float envattack,  envrelease,  minusattack,  minusrelease  ;  
double ar_dB ; 

float real, imag ; 


int frameNormalizeFlag, smoothingFlag ; 

float A_peak, B_peak,  AB_peak,  normamp,  normamps[16] ; 

float panwarp_A=0.,  panwarp_B=0. ; 
int ainchan,  exflag ; 

int Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1=0 ; 
float vibratoPeriodDurationNow=1. ; 


// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 

float windowedFilttnow, loopTimeDirectionSign=1.0 ; 
int Mode__sampler_loop_0__autostop_1=0,  autostopflag=0,  wrap_0_fold_1_clip_2=0, useTimeWindowFlag=0, 
	Onset_and_Release_Segment_Mode__off_0__on_1=0, useEndSegmentStageFlag=0   ; 

float channelAmpSum, tempChannelAmpSum, filterChannelAmpSum,
	normgain, frameNormalizationAmpLimit, normalizationAmp, 
			Normalize_to__Input_Sound_0__Filter_1=0 ; 



// ** FILTER VARIABLES
float filttnow=0., oldfilttnow,  filttinc,  filtf, filtfprop  ; 
int filtflow,  filtflowold=0,  filtfhigh ; 

SF_INFO 	inputSFinfo ; 


char tempstring[ STRING_SIZE ] ; 

// FRAME NORMALIZATION DECIBEL LIMIT
struct  func  frameNormalizationDecibelLimit ; 

// COMB FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 

// FILTER
struct  func  filter ; 
 


//  FILTER  RATE
struct  func  filtrate ; 

// FILTER TIME POINT ORIGIN
struct  func  filttorigin ; 

// FILTER TIME WINDOW LOWER BOUNDARY
struct  func  filtwinlow ; 

// FILTER TIME WINDOW UPPER BOUNDARY
struct  func  filtwinhi ; 







//  SOUND A dB
struct  func  Sound_A_dB ; 

//  SOUND B dB
struct  func  Sound_B_dB ; 

//  CONVOLUTION dB
struct  func  convolve_dB ; 

//  RELEASE
struct  func  release ; 

//  ATTACK
struct  func  attack ; 

//  CONVOLUTION PANPOT
struct  func  pan ; 

// *****************INITIALIZE


// FRAME NORMALIZATION DECIBEL LIMIT
frameNormalizationDecibelLimit.L = 1. ; 
	frameNormalizationDecibelLimit.n = 1. ; frameNormalizationDecibelLimit.A[ 0 ] = 0. ; 


// COMB FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH MULTIPLIER
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 

// FILTER
filter.L = 1. ; filter.n = 0. ; filter.A[ 0 ] = 0. ; 

//  SOUND A dB
Sound_A_dB.L = 1. ; Sound_A_dB.n = 1. ; Sound_A_dB.A[ 0 ] = -0. ; 

//  SOUND B dB
Sound_B_dB.L = 1. ; Sound_B_dB.n = 1. ; Sound_B_dB.A[ 0 ] = -0. ; 

//  CONVOLUTION dB
convolve_dB.L = 1. ; convolve_dB.n = 1. ; convolve_dB.A[ 0 ] = -0. ; 

//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 

//  CONVOLUTION PANPOT
pan.L = 1. ; pan.n = 1. ; pan.A[ 0 ] = 0  ; 

//  FILTER  RATE
filtrate.L = 1. ; filtrate.n = 1. ; filtrate.A[ 0 ] = 1. ; 

// FILTER
filttorigin.L = 1. ; filttorigin.n = 1. ; filttorigin.A[ 0 ] = 0. ; 

// FILTER TIME WINDOW LOWER BOUNDARY
filtwinlow.L = 1. ; filtwinlow.n = 1. ; filtwinlow.A[ 0 ] = 0. ; 

// FILTER TIME WINDOW UPPER BOUNDARY
filtwinhi.L = 1. ; filtwinhi.n = 1. ; filtwinhi.A[ 0 ] = -1. ; 




if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, 
"_||=|a|A|b|B|c|C|d|D|e|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|p|P|q|Q|r|R|s|S|t|T|v|V|w|W|x|X|y|Y|z|Z|",
0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {

	    case 'k':   smooth__polar_0__Cartesian_1 = (int) crackfloat( arg_option, ch );
			break;


	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'I':   tfactor = crackfloat( arg_option, ch );
			break;

	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
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

	    case 'y':   Mode__sampler_loop_0__autostop_1 = (int) crackfloat( arg_option, ch ) ;
			break;

            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; 
			break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; 
			break;

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

// ****
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

	    case 'r':	Onset_and_Release_Segment_Mode__off_0__on_1 = (int) crackfloat( arg_option, ch );
			break;
	    case 'o':	wrap_0_fold_1_clip_2 = (int) crackfloat( arg_option, ch );
			break;


	    case 'n':   strcpy(tempstring, arg_option);
			frameNormalizationDecibelLimit.fp = crackstring( tempstring, 
			    &frameNormalizationDecibelLimit );
			break;

	    case 'v':   Normalize_to__Input_Sound_0__Filter_1 = (int) crackfloat( arg_option, ch ) ;
			break;

// ****
           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;



	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;

	    case 'B':   strcpy(tempstring, arg_option);
			Sound_B_dB.fp = crackstring( tempstring, 
			    &Sound_B_dB );
			break;
	    case 'q':   strcpy(tempstring, arg_option);
			Sound_A_dB.fp = crackstring( tempstring, 
			    &Sound_A_dB );
			break;
	    case 'Z':   strcpy(tempstring, arg_option);
			convolve_dB.fp = crackstring( tempstring, 
			    &convolve_dB );
			break;

	    case 'S':   strcpy(tempstring, arg_option);
			pan.fp = crackstring( tempstring, 
			    &pan );
			break;

	    case 'j':	panwarp_A = crackfloat( arg_option, ch );
			break;
	    case 'J':	panwarp_B = crackfloat( arg_option, ch );
			break;

	    case 't':	threshfacdB = crackfloat( arg_option, ch );
			break;
} }


prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "CONVOLVER: SHORT-TERM FFT SPECTRAL MULTIPLICATION", 69 ) ; 
prline( 69,  "-" ) ; 

// ********** INSERT CHANNEL AND SETUP CHANGE
    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    } ; 
 



    prbanner( "(SOUND A)",  69 ) ; 

// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
    setupfiles(argc, argv) ; 

    endchan = beginchan + ochan ; 


// .......... END INSERT

// ** SOUND B
if( filter.n == 0. ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A SOUND B - INPUT FILE. BYE.\n" ) ;
    exit(0); 
} 

    // READ IN FFT HEADER VALUES
    //readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan,  normamps, &filter, 1 ) ; 
//    readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &itemp,  normamps, &filter ) ; 

    // READ IN FFT HEADER VALUES
    if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &itemp,  normamps, &filter, 1 ) == -1){
        fprintf( stderr, "CHECK YOUR ANALYSIS FILE.\t\t. . . BYE.\n\n\n" ) ; exit(EXIT_FAILURE) ; 
    } ; 


// ************ TEST SIZE TO SEE IF DATA FILE IS POSSIBLY WRONG
    temp = ( ((filter.n  - (float) FFT_HEADER_SIZE ) / (float) (analysis_N + 2) ) != 0 ) ; 
    temp2 = temp - (float) ( (int) temp ) ; 
    if( (analysis_N <= 0) || (analysis_D <= 0) || (analysis_R <= 0) || 
	    (analysis_chan <= 0) || ( temp2 != 0. ) ){
	prt( "YOUR SOUND B ANALYSIS FILE HAS QUESTIONABLE DATA. BYE." ) ; 
	    pri( analysis_N,  "INPUT ANALYSIS: FFT SIZE" ) ; 
	    pri( analysis_R,  "INPUT ANALYSIS: SAMPLE RATE" ) ; 
	    pri( analysis_D,  "INPUT ANALYSIS: DECIMATION" ) ; 
	    pri( analysis_chan,  "INPUT ANALYSIS: NUMBER OF CHANNELS" ) ; 
	    pri( filter.n,  "INPUT ANALYSIS: FILE SIZE" ) ; 
	exit(0) ; 
    }

 		
fprintf( stderr,"\n*******************************************\n\n" ) ; 
    niframes = (((filter.n - (float) FFT_HEADER_SIZE) / (float) (analysis_N + 2))) / analysis_chan ; 
    analysis_dur = (float) (niframes) / ((float) analysis_R / (float) analysis_D ) ; 
    iframes_per_sec = (float) analysis_R /  (float) analysis_D ; 
    pri( niframes,  "NUMBER OF FRAMES IN ANALYSIS" ) ; 

	// FILTER WINDOW BOUNDARIES
    // -1 FLAGS 
    if( filtwinlow.A[0] < 0.0 ) filtwinlow.A[0] = 0.0 ; 
    if( filtwinhi.A[0] < 0.0 ) filtwinhi.A[0] = analysis_dur ; 

// **************************************************************

	
// **************************************************************

prbanner( "SOUND B:  ANALYSIS FILE",  69 ) ; 
prf( analysis_dur,	  "INPUT ANALYSIS: DURATION" ) ; 
pri( analysis_N,  "SOUND B ANALYSIS: FFT SIZE" ) ; 
pri( analysis_R,  "SOUND B ANALYSIS: SAMPLE RATE" ) ; 
prf( iframes_per_sec,  "SOUND B ANALYSIS: FRAMES PER SECOND" ) ; 
pri( analysis_D,  "SOUND B ANALYSIS:      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
pri( analysis_chan,  "SOUND B ANALYSIS: NUMBER OF CHANNELS" ) ;
    
    if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............REDO ANALYSIS.  BYE.\n\n" ) ; exit(0) ; 
    }






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


    if( (frameNormalizationDecibelLimit.n > 1) || (frameNormalizationDecibelLimit.A[0] > 0. ) ) frameNormalizeFlag = 1 ; 
    else frameNormalizeFlag = 0 ; 

    if( (release.n > 1.) || (attack.n > 1.) || (release.A[0] > 0.) || (attack.A[0] > 0.) ) smoothingFlag = 1 ; 
    else smoothingFlag = 0 ; 


// **************************************************************
// **************************************************************
 
// **************************************************************
    N = analysis_N;
    D = analysis_D;
    frames_per_sec =  iframes_per_sec ; 

// **** SET UPS *****
    if(isr == analysis_R){
	// SAMPLE RATES MATCH
	R = isr ; // SAMPLE RATE EQUALS INPUT FILE
    }else{
	// NO MATCH
	prf( isr, "\nSOUND A SAMPLE RATE" ) ; 
	prf( analysis_R, "SOUND B SAMPLE RATE" ) ; 
	prt( "\nYOUR SAMPLE RATES DO NOT MATCH,. ....BYE.\n\n" ) ;  exit(0) ; 
    }


    if(tfactor <= 0.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY A TIME FACTOR > 0. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 1.\n\n" ) ; 
	tfactor = 1. ; 
    }
    I = (int) ((float) D * tfactor ) ; 

// ******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
    if( Nw < I ){
	// INCREASE WINDOW SIZE TO ACCOMODATE INTERPOLATION
	Nw = 2 ; while( Nw <= I )Nw *= 2 ;
	prt( "\n----> INCREASING WINDOW SIZE TO ACCOMODATE TIME RESYNTHESIS INTERPOLATION. <---" ) ;
	pri( Nw,  "NEW WINDOW SIZE" ) ; 
    }
// *********************************
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    fundamental = (float) R / (float) N ; 
// DECAY TIME STUFF
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    IR = (float) I / (float) R ;

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
    
// ***************** PRINT VALUES
prf( dur, "OUTPUT FILE: DURATION" ) ; 

prbanner( "SOUND A: ANALYSIS PARAMETERS",  69 ) ; 
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

prp( &dBgain,  "MASTER GAIN (in dB)"  ) ; 
prp( &ptrans,  "PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &harmadd,  "FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
if( smoothingFlag == 1){
    if( smooth__polar_0__Cartesian_1 == 0 )prt( "POLAR SMOOTHING") ; else prt( "CARTESIAN SMOOTHING" ) ; 
    prt( "SMOOTHING IS ON" ) ; 
    prp( &attack,  "ENVELOPE ATTACK TIME (in seconds)"  ) ; 
    prp( &release,  "ENVELOPE RELEASE TIME (in seconds)"  ) ; 
}else{
	prt( "SMOOTHING IS OFF" ) ; 
} ; 


prline( 1,  "*" ) ; 


prp( &filtrate,  "SOUND B: RATE MULTIPLIER"  ) ; 
prp( &filttorigin,  "SOUND B: TIME POINT-ORIGIN"  ) ; 
prp( &filtwinlow,  "SOUND B: TIME WINDOW LOWER BOUNDARY"  ) ; 
prp( &filtwinhi,  "SOUND B: TIME WINDOW UPPER BOUNDARY"  ) ; 

if( Onset_and_Release_Segment_Mode__off_0__on_1 == 1) prt( "TIME WINDOW TRIGGER MODE: ON" ) ;
else prt( "TIME WINDOW TRIGGER MODE: OFF" ) ; 
if( wrap_0_fold_1_clip_2 == 0 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: WRAP" ) ; 
if( wrap_0_fold_1_clip_2 == 1 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: FOLD" ) ; 
if( wrap_0_fold_1_clip_2 == 2 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: CLIP/LIMIT" ) ; 
if( Mode__sampler_loop_0__autostop_1 == 1 )prt( "AUTOSTOP ON" ) ; 
if( Mode__sampler_loop_0__autostop_1 == 0 )prt( "AUTOSTOP OFF" ) ; 



if( frameNormalizeFlag == 1 ) {
	prt( "FRAME NORMALIZATION IS ON" ) ; 
	prp( &frameNormalizationDecibelLimit, "FRAME NORMALIZATION DECIBEL LIMIT" ) ; 
	if( Normalize_to__Input_Sound_0__Filter_1 == 0 ) prt ("NORMALIZE TO INPUT SOUND") ;
	else prt( "NORMALIZE TO SOUND ANALYSIS" ) ; 
}else{
	 prt( "FRAME NORMALIZATION IS OFF" ) ;  
} ; 

prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
// *******
prp( &Sound_A_dB,  "CONVOLUTION PANPOT: SOUND A DB"  ) ; 
prp( &Sound_B_dB,  "CONVOLUTION PANPOT: SOUND B DB"  ) ; 
prp( &convolve_dB,  "CONVOLUTION PANPOT: CONVOLUTION DB"  ) ; 
prline( 1,  "*" ) ; 

prp( &pan,  "CONVOLUTION PANPOT (-1 to +1)"  ) ; 
prf( panwarp_A,  "CONVOLUTION PANPOT: SOUND A DOMAIN WARP"  ) ; 
prf( panwarp_B,  "CONVOLUTION PANPOT: SOUND A DOMAIN WARP"  ) ; 
prline( 1,  "*" ) ; 


// ***************** SET UP ARRAYS


    pri( niframes,  "NUMBER OF FRAMES IN ANALYSIS" ) ; 
    fprintf( stderr,  "\nINPUT FILE WILL BE USED %f TIMES", 
	    (dur * ((float) R / (float) D) ) / (float) niframes )  ;

fprintf( stderr,"\n*******************************************\n\n" ) ; 

 
    temp2 = (float) R  / (float) I  ; 
fprintf( stderr,"\n******** CONVOLVE  OUPUT *******************" ) ; 
fprintf( stderr,"\nOUTPUT DURATION: %f seconds",  dur ) ; 
fprintf( stderr,"\nOUTPUT: %d frames/sec ", (int) temp2 ) ;  ; 
fprintf( stderr,"\nOUTPUT: %d total frames", (int) (temp2 * dur) ) ;

// **************

    

    fvec( Wanal, Nw ) ;		// analysis window 
    fvec( Wsyn, Nw ) ;		// synthesis window 
    fvec( input, Nw ) ;		// input buffer 
    fvec( Hwin, Nw ) ;		// plain Hamming window 
    fvec( winput, Nw ) ;	// windowed input buffer 
    fvec( buffer, N ) ;		// FFT buffer 
    fvec( previous_buffer, N ) ;		// FFT buffer 

    fvec( channel, N+2 ) ;	// analysis channels 
    fvec( channelMix, N+2 ) ;	// analysis channels 
    fvec( bufferMix, N ) ;		// FFT buffer 


    fvec( C_buffer, N ) ;		// CONVOLUTION FFT buffer 
    fvec( C_channel, N+2 ) ;	// CONVOLUTION analysis channels 

    fvec( output, Nw ) ;	// output buffer 
    fvec( previous_channel, N+2 ) ;	// previous analysis channels 

// ***********
// SETUP FILTER CONTROL VALUES
	         // FILTER INCREMENT IN SECONDS
    filttinc =  (float) D / (float) R ; 
// ***********

// MAKE THRESH AMP
    threshfac = dB_to_amp(threshfacdB );	
    

// ALLOCATE INPUT FILE SPACE
    fvec( F,  N+2  ) ;	// input array 
    fvec( Fbuffer,  N  ) ;	// input Cartesian buffer 

    fvec( F_lower,  analysis_N+2  ) ;	// lower filter array 
    fvec( F_higher,  analysis_N+2  ) ;	// higher filter array 

// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

// *********************************************
// LOOP FOR CHANNELS
// *********************************************

for(channow = 0, outchan = beginchan; channow < ochan; channow++, outchan++ ){

    // **** SET UP CHANNELS
    outchan = (channelout == 0) ? channow : (channelout - 1) ; 
    ainchan = (achannelout == 0) ? channow : (achannelout - 1) ; 

// SETUP FILTER CONTROL VALUES
    filttorigin.A[ 0 ] = fval( &filttorigin, dur, 0. );
    filttnow = filttorigin.A[ 0 ] ; // ACCUMULATED FILTER TIME POINT IN SECONDS

    prline( 69,   "=" ) ; 
    pri( (outchan+1), "SOUND A CHANNEL" ) ; 
    pri( (ainchan+1), "SOUND B CHANNEL" ) ; 

    
    
    // *****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; autostopflag = 0 ; 


    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;
	
// *************
    // SET AT DATA BEGIN
    fseek( filter.fp,  sizeof(float) * FFT_HEADER_SIZE,  SEEK_SET ) ; 
// *************

// *********************************************
// LOOP FOR FRAMES
// *********************************************

    while ( (!eof) && (autostopflag == 0) ) {
	in += D ;
	on += I ;
	timenow( dur ) ;


	eof = shiftin( input, Nw, D ) ;
	fold( input, Wanal, Nw, buffer, N, in ) ;
	rfft( buffer, N2, FORWARD ) ;


    	// ********** FIND THE SUM OF THE AMPS FOR NORMALIZATION
        if( frameNormalizeFlag == 1 ){
            // CONVERT TO POLAR (channel)
    	    leanconvert( buffer, channel, N2, D, R ) ;

    	    channelAmpSum = 0. ; 
    	    for( i = 1; i < N; i+= 2 ){
		channelAmpSum += channel[i - 1] ; 
    	    } ; 

            leanunconvert( C_channel, C_buffer, N2, D, R ) ;

        } ; 

// *************
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
          Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1, // 0
          vibratoPeriodDurationNow
	) ; 




//fprintf( stderr, "time: %f\t filttnow: %f\t oldfilttnow : %f\t, diff: %f\n", 
//	t, filttnow, oldfilttnow, (filttnow - oldfilttnow) ) ;  

// *************


// ******************************************************************************
// MAKE THE FILTER FRAME
// ******************************************************



	makeInterpolatedFilterFrame ( &filter, F_lower, F_higher, F,
			iframes_per_sec, analysis_N, filttnow, ainchan, analysis_chan
	) ; 


	// SAVE THE OLD TIME POINT
	oldfilttnow = filttnow ; 


		
// *****************************************************


    // ********** FIND THE SUM OF THE AMPS FOR NORMALIZATION
    filterChannelAmpSum = 0 ; 
    for( i = 1; i < N; i+= 2 ){
	filterChannelAmpSum += F[i - 1] ; 
    }




// *************MAKE CARTESIAN ARRAY (Fbuffer) FROM  FREQ/MAGNITUDES FOR BOTH 
    	unconvert1( F, Fbuffer, N2, I, R ) ;
	

// ************MULTIPLY BUFFERS TOGETHER
    // FIND GREATEST PEAK BETWEEN THE TWO
    A_peak = RIfindpeak( buffer, N ) ; 
    B_peak = RIfindpeak( Fbuffer, N ) ; 
    AB_peak = A_peak > B_peak ? A_peak : B_peak ; 
    normamp = AB_peak == 0 ? 1. : 1. / AB_peak ; 

    // GET LEVELS
    Sound_A_dB.A[ 0 ] = fval( &Sound_A_dB, dur, t );
    Sound_B_dB.A[ 0 ] = fval( &Sound_B_dB, dur, t );
    convolve_dB.A[ 0 ] = fval( &convolve_dB, dur, t );


    // GET PAN POINT
    pan.A[ 0 ] = fval( &pan, dur, t ) ;
    pans = (float) fabs((double) pan.A[ 0 ] ) ; 
    temp = pan.A[ 0 ] < 0. ? (panwarp_A) :  (panwarp_B) ; 
    pans = curve( 0.,  1.,  pans, temp ) ;  
        
    // MAKE AMP LEVELS FROM DECIBELS Q: WHY "/20" ????
    A_dB = dB_to_amp( Sound_A_dB.A[ 0 ]) * pans ;	
    B_dB = dB_to_amp( Sound_B_dB.A[ 0 ]) * pans ;	
    C_dB = dB_to_amp(convolve_dB.A[ 0 ]) * (1. - pans) ;	

    // MULTIPLY BUFFERS
    for(i = 0; i < N; i++){
	C_buffer[i] = normamp * C_dB *  buffer[i] * Fbuffer[i] ; 
    }


/*
	// COMPLEX MULTIPLY  
	C_buffer[0] = buffer[0] * Fbuffer[0] * normamp * C_dB ; 
	C_buffer[1] = buffer[1] * Fbuffer[1] * normamp * C_dB ; 
	for(i = 2, j = 3;  i < N; i += 2, j += 2){
		real = (buffer[i] * Fbuffer[i]) - (buffer[j] * Fbuffer[j]) ; 
		imag = (buffer[i] * Fbuffer[j]) + (buffer[j] * Fbuffer[i]) ; 
		C_buffer[i] = real * normamp * C_dB ; C_buffer[j] = imag * normamp * C_dB ; 

	} ; 
*/


    // ************ NOW LEANCONVERT TO POLAR (C_channel) FREQ/MAGNITUDES
    leanconvert( C_buffer, C_channel, N2, D, R ) ;
    


	// *************EQUALIZE THE  SPECTRUM
    eq( C_channel,  (N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1,  0, 0 ) ; 



// ************ NOW LEANUNCONVERT TO CARTESIAN
    leanunconvert( C_channel, C_buffer, N2, D, R ) ;


    dBgain.A[ 0 ] =  fval( &dBgain, dur, t );
	gain = dB_to_amp( dBgain.A[ 0 ] ) ; 


// *********** NOW MIX PROPORTIONS IN CARTESIAN FORM AND ADD GAIN
    if( pan.A[ 0 ] < 0. ){
        for(i = 0; i < N; i++){
	    buffer[i] = gain * (
		C_buffer[i]  + (A_dB * buffer[i] )
            ) ; 
	}
    }else{
        for(i = 0; i < N; i++){
	    buffer[i] = gain * (
		C_buffer[i]  + (B_dB * Fbuffer[i]) 
            ) ; 
	}
    }

    if( smoothingFlag == 1 ){
	        // SETUP PREVIOUS CHANNEL
        if( !frame_count )for(i = 0; i < N; i++ ) previous_buffer[ i ] = buffer[ i ] ; 
        // SMOOTH THE CHANGES TO THE SPECTRUM
        CartesianSmooth( buffer, previous_buffer, N, envattack, minusattack, envrelease, minusrelease ) ; 
    } ; 


    // FRAME NORMALIZATION
    frameNormalizationDecibelLimit.A[ 0 ] = 
			fval( &frameNormalizationDecibelLimit, dur, t );
    if( (frameNormalizeFlag == 1) || 
		( (smoothingFlag == 1) && (smooth__polar_0__Cartesian_1 == 0) ) ){

	// CONVERT TO POLAR
	leanconvert( buffer, channel, N2, D, R ) ;

        if( frameNormalizeFlag == 1 ){

            frameNormalizationAmpLimit = dB_to_amp( frameNormalizationDecibelLimit.A[ 0 ] ); 


		// FIND NORMALIZATION FACTOR.
	    tempChannelAmpSum = 0. ; 	
	    for( i = 0; i < N; i+= 2 ){
	    	tempChannelAmpSum += channel[i] ;
	    } ; 


	    if( tempChannelAmpSum > 0.0 ){
		if( Normalize_to__Input_Sound_0__Filter_1 == 0 ){
			normalizationAmp = channelAmpSum / tempChannelAmpSum ;
		}else {
			normalizationAmp = filterChannelAmpSum / tempChannelAmpSum ;
		} ; 


		if( normalizationAmp > frameNormalizationAmpLimit )
			normalizationAmp = frameNormalizationAmpLimit ; 

		for( i = 0; i < N; i+= 2 )
			channel[i] = channel[i] * normalizationAmp ; 	
	    } ; 

        } ; 

/*
	if( smoothingFlag == 1 ){

		release.A[ 0 ] = fval( &release, dur, t );
		smooth_setup( release.A[ 0 ], &envrelease, &minusrelease, IR ) ; 

		attack.A[ 0 ] = fval( &attack, dur, t );
		smooth_setup( attack.A[ 0 ], &envattack, &minusattack, IR ) ; 

	        // SETUP PREVIOUS CHANNEL
	        if( !frame_count )for(i = 0; i < N; i++ )
		    previous_channel[ i ] = channel[ i ] ; 

                // SMOOTH THE CHANGES TO THE SPECTRUM
                smooth( channel, previous_channel, (N + 2), envattack, minusattack, envrelease, minusrelease ) ; 

        } ; 
*/

	// CONVERT TO CARTESIAN
        leanunconvert( channel, buffer, N2, D, R ) ;

    } ; 





// ************* NOW CONVERT CARTESIAN MIX TO POLAR (channel)


        if( obank ) {
	  	  convert( buffer, channel, N2, D, R ) ;

// *************************
// GET THE VALUES
// *************************



//  SHIFT, GAIN, AND TRANSPOSITION

		harmadd.A[ 0 ] =  fval( &harmadd, dur, t );
		ptrans.A[ 0 ] = fval( &ptrans, dur, t );
		    pm = semitones_to_mult( ptrans.A[ 0 ] ) ;






// *************************


//prf( tempChannelAmpSum, "tempChannelAmpSum" ) ; 
//prf( channelAmpSum, "channelAmpSum" ) ; 
//prf( filterChannelAmpSum, "filterChannelAmpSum" ) ; 






// *****************
// MODIFICATIONS LOOP
// *****************





		 for( i = 1, j = 0; i < N; i+= 2, j++ ){

		    // SHIFT BY -a AND TRANSPOSE BY -P

		    temp = pm * (channel[i] + harmadd.A[ 0 ]) ;
		    
		    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		    if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
		   else channel[i] = temp ; 

//		  channel[i - 1] = channel[i - 1] * gain ;  

		}

        } ; 


        synt = getthresh( channel, N, threshfac );
// ** OSCIL BANK OR OVERLAP/ADD OUT
	if ( obank ) {
	    noscbank(channel, N2, R, Nw, I, P, output);
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;
	} else {
//	    unconvert( channel, buffer, N2, I, R ) ;

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
    fclose(ifd) ;  
    fprintf(stderr,"\nCONVOLVER : RESYNTHESIS COMPLETED\n");
 



    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
    if( filter.n != 1. ) fclose(filter.fp ) ;
    if( Sound_A_dB.n != 1. ) fclose(Sound_A_dB.fp ) ;
    if( Sound_B_dB.n != 1. ) fclose(Sound_B_dB.fp ) ;
    if( convolve_dB.n != 1. ) fclose(convolve_dB.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;
    if( pan.n != 1. ) fclose(pan.fp ) ;
    if( filtrate.n != 1. ) fclose(filtrate.fp ) ;
    if( filttorigin.n != 1. ) fclose(filttorigin.fp ) ;
    if( filtwinlow.n != 1. ) fclose(filtwinlow.fp ) ;
    if( filtwinhi.n != 1. ) fclose(filtwinhi.fp ) ;

    exit( 0 ) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"convolve:  phase vocoder spectral multiplier\n"
	"convolve   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	      (FFT and framerate/decimation are taken from Sound B analysis file)\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	w:	window type: 0 = hamming,  1 = rectangular  \n"
	"		    2 = Blackman,  3 = Bartlett triangular [0.]\n"
	"		    4-12 = Kaiser windows for alpha = 4-12,  respectively\n"
	"		    (representative sidelobe levels for alpha: \n"
	"		      4 = -30dB,  8 = -58 dB,  12 = -90 dB)\n"
	"	I:	time expansion/contraction factor  [1.] \n"
	"		  (duration = duration * factor, 1. = original time) \n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds (0. = end of file) [0.] \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"

	"	P:	pitch transposition of output spectrum in semitones (func) [0]\n"
	"	a:	frequency shift  of output spectrum \n"
	"		    (bin frequency adder, before -P )(func)[0.] \n"
	"	A:	gain in decibels of output (func) [0.] \n"

	"	F:	input analysis file - (sound B) \n"
	"	B:	Sound B gain in decibels (func) [0.] \n"
	"	q:	Sound A gain in decibels (func) [0.] \n"
	"	Z:	Convolution gain in decibels (func) [0.] \n"

	"	S:	CONVOLUTION PAN POT position between \n"
	"		    convolved sounds (-1 to 1) (func) [0] \n"
	"		    (-1 = A,  1 = B,  0 = convolution of A and B)\n"
	"	j:	CONVOLUTION PAN POT: Sound A domain warp [0.] \n"
	"	J:	CONVOLUTION PAN POT: Sound B domain warp [0.] \n"

	"	k:	data smoothing: 0 = polar, 1 = Cartesian [0]\n" 
	"	L:	envelope release time  (func) [0.]\n"
	"	l:	envelope attack time  (func) [0.]\n"

	"	K:	SOUND B channel (1 -> ?) (0 = all) [0] \n"
	"		   (0 = explicit time: Use -Q)\n"
	"		   (1 = rate mode: Use -Q as begin point, -Y as rate control) \n"
	"	Q:	SOUND B time point (func) [0.] \n"
	"	g:	SOUND B time window: lower boundary (func) [0.] \n"
	"	G:	SOUND B time window: upper boundary (func) [end of file] \n"

	"	o:	DATA time window: boundary flag  [0]\n"
	"		   0 = wrap time into window\n"
	"		   1 = fold time into window bounds\n"
	"		   2 = clip or limit time to nearest window boundary\n"
	"	r:	DATA time window: trigger entry mode flag [0]\n"
	"		   Begin or trigger use of time window boundaries with first entry\n"
	"		   of time point into window bounds. 0 = off, 1 = on\n"
	"		    (upper boundary < 0. defaults to end of file)\n"

	"	Y:	SOUND B rate multiplier (func) [1.]\n"
	"		    (1. = rate of original, 2. = twice as fast, etc.)\n"
	"		    (negative = reverse,  0 = stationary) \n"
	"	y:	DATA auto stop: 0 = off,  1 = on [0]\n"
	"		    (When on,  auto stop will terminate synthesis when a\n"
	"		     time boundary in the SOUND B analysis is crossed.)\n"


	"	     CONVOLUTION OUTPUT LOW/HIGH SHELF EQ:(pre transpose/shift)\n"
	"	H:	FILTER SHELF EQ: Low shelf gain in dB [0.] \n"
	"	X:	FILTER SHELF EQ: High shelf gain in dB [0.] \n"
	"	m:	FILTER SHELF EQ: Low shelf frequency in Hz [200.] \n"
	"	R:	FILTER SHELF EQ: High shelf frequency in Hz [2000.] \n"



	"		FRAME NORMALIZATION: \n"
	"	n:	Frame Normalization Decibel Limit: (0-?) \n"
	"		Scale output frame amps to match or approach input frame amps\n"
	"		using the (sum of input amps)/(sum of output amps) limited to\n" 
	"		the Decibel limit. 0 dB prevents normalization. [0]\n"   

	"	v:	Frame Normalization Reference: \n"
	"		0 = Input Sound, 1 = Filter Analysis [0]\n"


	"	t:	oscillator resynthesis threshold in decibels [-96.]\n"
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
    exit(0);
}


void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

