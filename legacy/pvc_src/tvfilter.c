#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k,  i1,  i2;
float i1p,  i2p ; 
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int analysis_N, analysis_N2,   analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  achannelout=0 ;
int pitchflag=0; 
 float P = 1.0;
  FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float *previous_channel,  *F,  *previousF,  *F_lower,  *F_higher, *tempF ; 
float threshfac = .001,  threshfacdB=-60 ;
float	pm, fm, fs,  gain=1.;
double ar_dB ; 
float  temp,  temp2,  temp3 ;  
float getthresh();
float normamp[MAXIMUM_CHANNELS] ;
float diff ; 
float   IR,  dur=0., saved_dur;
float fundamental ;
float filtframenow=0. ; 
float analysis_fundamental, N_ratio,  sourceamp,  filtamp ;  
int limitcount=0,  bandrejecton=0 ; 
int ainchan ; 

//float fcentroid, old_fcentroid, scentroid,  old_scentroid,  sm ; 

int Mode__sampler_loop_0__autostop_1=0,  autostopflag=0,  wrap_0_fold_1_clip_2=0, 
	Onset_and_Release_Segment_Mode__off_0__on_1=0   ; 

float channelAmpSum, tempChannelAmpSum, filterChannelAmpSum,
	normgain, frameNormalizationAmpLimit, normalizationAmp, 
			Normalize_to__Input_Sound_0__Filter_1 =0 ; 

int LoopNormalizationFlag=0 ; 
float loopSmoothTime=0.0 ; 

float releasec,  minusreleasec,  attackc,  minusattackc ; 
float freleasec,  minusfreleasec,  fattackc,  minusfattackc ; 

//** FILTER VARIABLES
float filttnow=0., oldfilttnow,  filttinc,  filtf, filtfprop  ; 
int filtflow,  filtflowold=0,  filtfhigh,  imode=1. ; 

char tempstring[ STRING_SIZE ] ; 

int numFormants, *formantIndeces ;
float *formantCenterFreqs, *formantAmps, *formantBWs, *formantQs ; 

float dbThreshold=(-80), lowFreqLimit=20, highFreqLimit=20000, smoothingHzBW=-1.0, 
	smoothingBW_Q=-1.0, minBWasProportionOfPeakBW=0.1 ;  
int maxNumberOfFormants=1000 ; 


// PEAK LOOP SMOOTH TIME
struct func peakLoopSmoothTime ; 


// FRAME NORMALIZATION DECIBEL LIMIT
struct  func  frameNormalizationDecibelLimit ; 


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
float  filtcompthreshamp; 
struct  func  filtcompthresh ; 

//  FILTER DECIBELS OF COMPRESSION 
float filtcompamp,  filtcompnormamp ; 
struct  func  filtcompdecibels ; 

// ****

// FILTER TRANSPOSE IN SEMITONES
struct  func  ftrans ; 

// FILTER  SHIFTER
struct  func  fshift ; 

//  FILTER RELEASE
struct  func  frelease ; 

//  FILTER ATTACK
struct  func  fattack ; 

//  FILTER SMOOTHING
struct  func  smoothingBW ; 

//  FILTER WARP SHAPE
struct  func  warpshape ; 

//SHELF EQ
struct  func  dBlow;
struct  func  dBhi;
struct  func  freqlow;
struct  func  freqhi ;


//  CENTROID SHIFT CONROL
//struct  func  centroidshift ; 


//*****************INITIALIZE

// PEAK LOOP SMOOTH TIME
peakLoopSmoothTime.L = 1. ; peakLoopSmoothTime.n = 1. ; peakLoopSmoothTime.A[ 0 ] = 0.2 ; 



// FRAME NORMALIZATION DECIBEL LIMIT
frameNormalizationDecibelLimit.L = 1. ; 
	frameNormalizationDecibelLimit.n = 1. ; frameNormalizationDecibelLimit.A[ 0 ] = 0. ; 



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


// SHELF EQ
dBlow.L = 1. ; dBlow.n = 1. ; dBlow.A[ 0 ] = 0. ; 
dBhi.L = 1. ; dBhi.n = 1. ; dBhi.A[ 0 ] = 0. ; 
freqlow.L = 1. ; freqlow.n = 1. ; freqlow.A[ 0 ] = 200. ; 
freqhi.L = 1. ; freqhi.n = 1. ; freqhi.A[ 0 ] = 2000. ; 


//  FILTER COMPRESSION THRESHOLD
filtcompthresh.L = 1. ; filtcompthresh.n = 1. ; filtcompthresh.A[ 0 ] = 0. ; 

//  FILTER DECIBELS OF COMPRESSION 
filtcompdecibels.L = 1. ; filtcompdecibels.n = 1. ; filtcompdecibels.A[ 0 ] = 0. ; 

//  CENTROID SHIFT CONROL
//centroidshift.L = 1. ; centroidshift.n = 1. ; centroidshift.A[ 0 ] = 0. ; 

strcpy( routine, "tvfilter" ) ; 


if( argc < 2 )usage() ; 

//	AVAILABLE: 'j' 'k' 'O' 'U' 'y' 'u'

    while( (ch= crack( argc, argv, 
    "a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|N|n|o|p|P|q|Q|r|R|s|S|t|T|V|v|w|W|x|X|Y|z|Z|_|=|",
     0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = (int) crackfloat(arg_option, ch); // crackfloat(arg_option, ch)
			break;
	    case 'M':   Nw = (int) crackfloat(arg_option, ch);
			break;
	    case 'w':   window_type = (int) crackfloat(arg_option, ch) ;
			break;
	    case 'D':   frames_per_sec = crackfloat(arg_option, ch);
			break;
	    case 'I':   tfactor = crackfloat(arg_option, ch);
			break;

	    case 'B':   pitchflag = (int) crackfloat(arg_option, ch) ;
			break;

	    case 'b':   begint = crackfloat(arg_option, ch) ;
			break;
	    case 'e':   endt = crackfloat(arg_option, ch) ;
			break;

	    case 'C':   channelout = (int) crackfloat(arg_option, ch) ;
			break;

	    case 'K':   achannelout = (int) crackfloat(arg_option, ch) ;
			break;

	    case 'H':   strcpy(tempstring, arg_option);
			dBlow.fp = crackstring( tempstring, &dBlow );
			break;
	    case 'X':   strcpy(tempstring, arg_option);
			dBhi.fp = crackstring( tempstring, &dBhi );
			break;
	    case 'm':   strcpy(tempstring, arg_option);
			freqlow.fp = crackstring( tempstring, &freqlow );
			break;
	    case 'R':   strcpy(tempstring, arg_option);
			freqhi.fp = crackstring( tempstring, &freqhi );
			break;

	    case 'j':   LoopNormalizationFlag = (int) crackfloat(arg_option, ch) ; 
			break;

	    case 'k':   strcpy(tempstring, arg_option);
			peakLoopSmoothTime.fp = crackstring( tempstring, &peakLoopSmoothTime );
			break;


           case '_':	autoplayreps = (int) crackfloat(arg_option, ch) ; break;

           case '=':	rescalev = crackfloat(arg_option, ch) ; break;

	    case 'd':   Mode__sampler_loop_0__autostop_1 = (int) crackfloat(arg_option, ch) ;
			break;



            case 'p':	quiet = (int) crackfloat(arg_option, ch) ; break;
            case 'i':	ampstatinc = crackfloat(arg_option, ch) ; break;

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
 
            case 'q':	bandrejecton = (int) crackfloat(arg_option, ch) ; break;


	    case 'f':   strcpy(tempstring, arg_option);
			smoothingBW.fp = crackstring( tempstring, 
			    &smoothingBW );
			break;

// *****
	    case 'u':	imode = (int) crackfloat(arg_option, ch);
			break;
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
	    case 'o':	wrap_0_fold_1_clip_2 = (int) crackfloat(arg_option, ch);
			break;
	    case 'r':	Onset_and_Release_Segment_Mode__off_0__on_1 = (int) crackfloat(arg_option, ch);
			break;



	    case 'E':   strcpy(tempstring, arg_option);
			filtcompthresh.fp = crackstring( tempstring, 
			    &filtcompthresh);
			break;
	    case 'c':   strcpy(tempstring, arg_option);
			filtcompdecibels.fp = crackstring( tempstring, 
			    &filtcompdecibels);
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

	    case 'n':   strcpy(tempstring, arg_option);
			frameNormalizationDecibelLimit.fp = crackstring( tempstring, 
			    & frameNormalizationDecibelLimit );
			break;

	    case 'v':   Normalize_to__Input_Sound_0__Filter_1 = (int) crackfloat(arg_option, ch) ;
			break;



	    case 't':   threshfacdB = crackfloat(arg_option, ch);
			break;
	    case 's':   sflag = 1;
			break;
	} 
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "TV (TIME VARYING) FILTER", 69 ) ; 
prline( 69,  "-" ) ; 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
 




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
    dur = saved_dur = (endt - begint) * (float) I / (float) D ; 



// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( (ptrans.n  != 1.) || (harmadd.n  != 1.) || 
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.)  ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
    
    // READ IN FFT HEADER VALUES
    if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &k,  normamp, &filter, 1 ) == -1){
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
prp( &frameNormalizationDecibelLimit, "FRAME NORMALIZATION DECIBEL LIMIT" ) ; 

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
    if( filtwinlow.A[0] < 0.0 ) filtwinlow.A[0] = 0.0 ; 
    if( filtwinhi.A[0] < 0.0 ) filtwinhi.A[0] = analysis_dur ; 



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


pri( imode,  "INPUT ANALYSIS: DATA MODE" ) ; 
prp( &filtrate,  "FILTER RATE MULTIPLIER"  ) ; 
prp( &filttorigin,  "FILTER TIME POINT-ORIGIN"  ) ; 
prp( &filtwinlow,  "FILTER TIME WINDOW LOWER BOUNDARY"  ) ; 
prp( &filtwinhi,  "FILTER TIME WINDOW UPPER BOUNDARY"  ) ; 

if( Mode__sampler_loop_0__autostop_1 == 0){
	prt( "SET TO SAMPLER LOOP MODE" ) ;
	if( wrap_0_fold_1_clip_2 == 0 )prt( "LOOP METHOD: WRAP" ) ; 
	if( wrap_0_fold_1_clip_2 == 1 )prt( "LOOP METHOD: FOLD" ) ; 
	if( wrap_0_fold_1_clip_2 == 2 )prt( "LOOP METHOD: CLIP/LIMIT" ) ; 
	prp( &peakLoopSmoothTime, "LOOP SMOOTH TIME" ) ; 
	if( Onset_and_Release_Segment_Mode__off_0__on_1 == 1) prt( "USING ONSET/RELEASE MODE" ) ;
}else 
	prt( "SET TO AUTOSTOP MODE" ) ; 



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
prp( &freqlow, "LOW SHELF FREQUENCY" ) ; 
prp( &dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prp( &freqhi, "HIGH SHELF FREQUENCY" ) ; 
prp( &dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
prp( &warpshape, "INPUT SPECTRUM WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 
prp( &smoothingBW, "RESPONSE SMOOTHING (Q) FACTOR IN FREQUENCY OR OCTAVES" ) ; 

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
    fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

// ALLOCATE FILTER SPACE
    fvec( F,  analysis_N+2  ) ;	/* filter array */
    fvec( previousF, analysis_N+2 ) ;	/* previous filter channels */
    fvec( tempF,  analysis_N+2  ) ; 

    fvec( F_lower,  analysis_N+2  ) ;	/* lower filter array */
    fvec( F_higher,  analysis_N+2  ) ;	/* higher filter array */



    fvec( formantCenterFreqs, (analysis_N/2) + 1 ) ; fvec( formantAmps, (analysis_N/2) + 1 ) ; 
    fvec( formantBWs, (analysis_N/2) + 1 ) ; fvec( formantQs, (analysis_N/2) + 1 ) ; 
    ivec( formantIndeces, (analysis_N/2) + 1 ) ; 



// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

//*********************************************
//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

    dur = saved_dur ; 

    // **** SET UP ANALYSIS CHANNEL
	ainchan = (achannelout == 0) ? channow : (achannelout - 1) ; 
    // SETUP FILTER CONTROL VALUES
	filttorigin.A[ 0 ] = fval( &filttorigin, dur, 0. );
	filttnow = filttorigin.A[ 0 ] ; 

	oldfilttnow = filttnow ; 

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
//    if(dBhi != 0. )fprintf(stderr," \nUSING FILTER EQ ......" ) ;
    fprintf(stderr," \n") ; 
    if( bandrejecton )prt( "INVERTING RESPONSE......." ) ; 

//*************
	    // SET AT DATA BEGIN
		fseek( filter.fp,  sizeof(float) * FFT_HEADER_SIZE,  SEEK_SET ) ; 
//*************
	
 //*********************************************
// LOOP FOR FRAMES
//*********************************************

   while ( (!eof)  && (autostopflag == 0) ) {
	in += D ;
	on += I ;
	timenow( dur ) ;

    eof = shiftin( input, Nw, D ) ;
    fold( input, Wanal, Nw, buffer, N, in ) ;
    rfft( buffer, N2, FORWARD ) ;
    convert( buffer, channel, N2, D, R ) ;

	// SETUP PREVIOUS CHANNEL
    if( !frame_count )for(i = 0; i < (N + 2); i++)
	    previous_channel[ i ] = channel[ i ] ; 



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


//******************************************************************************
// MAKE THE FILTER FRAME
//******************************************************

	makeInterpolatedFilterFrame ( &filter, F_lower, F_higher, F,
			iframes_per_sec, (analysis_N + 2), filttnow, ainchan, analysis_chan
	) ; 

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

//******************************************************

//*************NORMALIZE THE INPUT SPECTRUM
    normalize(  F,  (analysis_N + 2),   normamp[ainchan]  ) ;


    //********** FIND THE SUM OF THE AMPS FOR NORMALIZATION
    filterChannelAmpSum = 0 ; 
    for( i = 1; i < (N + 2); i+= 2 ){
	filterChannelAmpSum += F[i - 1] ; 
    }


//**************COMPRESS THE INPUT SPECTRUM

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

    if((filtcompthreshamp < 1.0) && (filtcompamp < 1.0) ){

	if(!frame_count)fprintf(stderr," \nUSING FILTER SPECTRUM COMPRESSION......" ) ;
	compress( F,  (analysis_N + 2),  filtcompthreshamp, filtcompamp,  filtcompnormamp ) ; 
     }

    

//*************** IF BAND REJECT, THEN INVERT THE RESPONSE
    if( bandrejecton )invertresponse( F,  (analysis_N + 2), (bandrejecton - 1) ) ; 

//*************WARP THE INPUT SPECTRUM
	warpshape.A[ 0 ] = fval( &warpshape, dur, t );
    spectmagwarp( F,  (analysis_N +2), warpshape.A[ 0 ], 0 ) ;

//*************EQUALIZE THE INPUT SPECTRUM
		dBlow.A[ 0 ] =  fval( &dBlow, dur, t );
		dBhi.A[ 0 ] =  fval( &dBhi, dur, t );
		freqlow.A[ 0 ] =  fval( &freqlow, dur, t );
		freqhi.A[ 0 ] =  fval( &freqhi, dur, t );
    eq( F,  (analysis_N + 2),  dBlow.A[ 0 ],  dBhi.A[ 0 ], freqlow.A[ 0 ],  freqhi.A[ 0 ],  fundamental, 1,  0, 0 ) ; 

     //*********** SMOOTH THE SPECTRUM
	smoothingBW.A[ 0 ] = fval( &smoothingBW, dur, t );
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


	    // FRAME NORMALIZATION DECIBEL LIMIT
		frameNormalizationDecibelLimit.A[ 0 ] = 
			fval( & frameNormalizationDecibelLimit, dur, t );
		frameNormalizationAmpLimit = dB_to_amp( frameNormalizationDecibelLimit.A[ 0 ] ); 



//*************************

    //********** FIND THE SUM OF THE AMPS FOR NORMALIZATION
    channelAmpSum = 0 ; 
    for( i = 1; i < (N + 2); i+= 2 ){
	channelAmpSum += channel[i - 1] ; 
    }



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
		    temp =  (temp / (fm) )  ; // TRANSPOSE THE BIN
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
		    
		    // CHANGE AMP
		    channel[i - 1] *=  ((filtamp * temp) + sourceamp );  

		//****** END FILTER

		}


	//  NORMALIZE AND REPLACE INTO CHANNEL.
		// FIND NORMALIZATION FACTOR.
	tempChannelAmpSum = 0. ; 	
	for( i = 0; i < (N + 2); i+= 2 ){
		tempChannelAmpSum += channel[i] ;
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
			channel[i] = channel[i] * normalizationAmp ; 	
	} ; 



    // SMOOTH THE CHANGES TO THE SPECTRUM
    smooth( channel, previous_channel, (N + 2), attackc, minusattackc, releasec, minusreleasec ) ; 



		 for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){

// SHIFT BY -a AND TRANSPOSE BY -P

		    temp = pm * (channel[i] + harmadd.A[ 0 ]) ;
		    
		// ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		    if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
		    else channel[i] = temp ; 



		    channel[i - 1] = channel[i - 1] * gain ;  

		}
		

        synt = getthresh( channel, (N + 2), threshfac );


// ** OSCIL BANK OR OVERLAP/ADD OUT
	if ( obank ) {
	    noscbank(channel, N2, R, Nw, I, P, output);
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;
	} else {
	    unconvert( channel, buffer, N2, I, R ) ;

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


    fprintf(stderr,"\nFILTER : RESYNTHESIS COMPLETED\n");


    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
    if( sourcedB.n != 1. ) fclose(sourcedB.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;
    if( filter.n != 1. ) fclose(filter.fp ) ;
    if( filtrate.n != 1. ) fclose(filtrate.fp ) ;
    if( filttorigin.n != 1. ) fclose(filttorigin.fp ) ;
    if( filtwinlow.n != 1. ) fclose(filtwinlow.fp ) ;
    if( filtwinhi.n != 1. ) fclose(filtwinhi.fp ) ;
    if( ftrans.n != 1. ) fclose(ftrans.fp ) ;
    if( fshift.n != 1. ) fclose(fshift.fp ) ;
    if( frelease.n != 1. ) fclose(frelease.fp ) ;
    if( fattack.n != 1. ) fclose(fattack.fp ) ;
    if( warpshape.n != 1. ) fclose(warpshape.fp ) ;
    if( smoothingBW.n != 1. ) fclose(smoothingBW.fp ) ;
    if( dBlow.n != 1. ) fclose(dBlow.fp ) ;
    if( dBhi.n != 1. ) fclose(dBhi.fp ) ;
    if( freqlow.n != 1. ) fclose(freqlow.fp ) ;
    if( freqhi.n != 1. ) fclose(freqhi.fp ) ;
    if( filtcompthresh.n != 1. ) fclose(filtcompthresh.fp ) ;
    if( filtcompdecibels.n != 1. ) fclose(filtcompdecibels.fp ) ;


    exit(EXIT_SUCCESS) ;
}


void usage()
{
    fprintf(stderr, "%s",
	"tvfilter:  time-varying  cross-synthetic, phase vocoder filter\n"
	"tvfilter   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"
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
	"	o:	DATA time window: boundary flag  [0]\n"
	"		   0 = wrap time into window\n"
	"		   1 = fold time into window bounds\n"
	"		   2 = clip or limit time to nearest window boundary\n"
	"	r:	DATA time window: trigger entry mode flag [0]\n"
	"		   Begin or trigger use of time window boundaries with first entry\n"
	"		   of time point into window bounds. 0 = off, 1 = on\n"
	"		    (upper boundary < 0. defaults to end of file)\n"
	"	Y:	FILTER rate multiplier (func) [1.]\n"
	"		    (1. = rate of original, 2. = twice as fast, etc.)\n"
	"		    (negative = reverse,  0 = stationary) \n"
	"	d:	DATA auto stop: 0 = off,  1 = on [0]\n"
	"		    (When on,  auto stop will terminate synthesis\n"
	"		      when a time boundary is crossed.)\n"

	"	j:	Loop Normalization Flag [0]\n"	"	k:	peak Loop Smooth Time [.2]\n"


	"	E:	FILTER-spectrum compression threshold (in decibels) (func) [0] \n"
	"	c:	FILTER-spectrum decibels of compression (func) [0] \n"

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

	"	     FILTER FREQUENCY RESPONSE -- SHELF EQ: (pre-processing)\n"
	"	H:	FILTER SHELF EQ: Low shelf gain in dB  (func) [0.] \n"
	"	X:	FILTER SHELF EQ: High shelf gain in dB  (func) [0.] \n"
	"	m:	FILTER SHELF EQ: Low shelf frequency in Hz  (func) [200.] \n"
	"	R:	FILTER SHELF EQ: High shelf frequency in Hz  (func) [2000.] \n"

	"	t:	oscillator resynthesis threshold [.001]\n"

	"		FRAME NORMALIZATION: \n"
	"	n:	Frame Normalization Decibel Limit: (0-?) \n"
	"		Scale output frame amps to match or approach input frame amps\n"
	"		using the (sum of input amps)/(sum of output amps) limited to\n" 
	"		the Decibel limit. 0 dB prevents normalization. [0]\n"   

	"	v:	Frame Normalization Reference: \n"
	"		0 = Input Sound, 1 = Filter Analysis [0]\n"


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

