#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k, kk, l,   i1,  i2;
float i1p,  i2p ; 
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int analysis_N,  analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  achannelout=0 ;
 float P = 1.0;
  FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float *previous_channel,  *F_lower,  *F_higher,  *delay_F ; 
float threshfac = .001,  threshfacdB=-60 ;
float	pm, fm, fs,  gain=1.;
double ar_dB ; 
float  temp,  temp2,  temp3 ;  
float getthresh();
float normamp[MAXIMUM_CHANNELS],  normamppk ;
float diff ; 
float   IR,  dur=0., funcDur ;
float low, hi, avg, median ; 
int length ; 
float fundamental ;
float filtframenow=0. ; 
float analysis_fundamental ;  
float prebalancesum,  postbalancesum, balancelimitdB=0,  balancelimitamp=0  ; 
int ainchan ; 
// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 
// GROUP DELAY EQ
float  delay_dBlow=0, delay_dBhi=0,  delay_freqlow=200, delay_freqhi=2000  ; 

float releasec,  minusreleasec,  attackc,  minusattackc ; 
float  fsmoothc,  minusfsmoothc,  minusransmoothv, ransmoothv ; 
//** FILTER VARIABLES
float filttnow, twindiff,  *filtbint,  *binranv,  filttinc,  filtf, *filtfprop,  
	*binTimeDelays, *filtbindBprop  ; 
int *filtflow,  filtfhigh,  imode=1.,  earliestf ; 


float dwin,  factor,  maxdelay  ;
int *done ;  

int print_flag=0 ; 
int bandrejecton=0 ; 
float delay_warpshape=0,  binamp ; 
float N_ratio,  delay_analysis_fundamental ; 
int  delay_analysis_N ;  
char tempstring[ STRING_SIZE ] ; 
float max,  min,  timeDelayForThisBin ; 

// FUNCTION DELAY TIME SCALER
struct func function_delay_time_scaler ; 

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

//  FREQUENCY CHANGE RESPONSE TIME
struct  func  fsmooth ; 

// DATA
struct  func  filter ; 

// ****
//  DATA  RATE
struct  func  filtrate ; 

// DATA TIME POINT ORIGIN
struct  func  filttorigin ; 


// MAX DELAY TIME
struct  func  maxdelayt ; 

// DELAY FILTER
struct  func  delayfilter ; 

// DELAY FILTER SEMITONE TRANSPOSE
struct  func  delayftrans ; 

// DELAY FILTER TRANSPOSE SHIFTER
struct  func  delayfshift ; 


// SPECTRUM WARPSHAPE INDEX
struct  func  warpshape ; 

//  ransmooth
struct  func  ransmooth ; 
int ranswitch=0 ; 

// ranTbandwidth
struct  func  ranTbandwidth ; 


//  0 DELAY DECIBELS
struct  func  zerodelay_dB ; 

//  MAX DELAY DECIBELS
struct  func  maxdelay_dB ; 

//  DELAY DECIBELS CURVE INDEX
struct  func  delay_dB_warp ; 

//  FILTER COMPRESSION THRESHOLD
float  filtcompthresh=0,   filtcompthreshamp; 
int compflag=0 ; 

//  FILTER DECIBELS OF COMPRESSION 
float  filtcompdecibels=0,  filtcompamp,  filtcompnormamp ; 
// ****


//*****************INITIALIZE
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

//  FREQUENCY CHANGE RESPONSE TIME
fsmooth.L = 1. ; fsmooth.n = 1. ; fsmooth.A[ 0 ] = 0. ; 

// FILTER
filter.L = 1. ; filter.n = 0. ; filter.A[ 0 ] = 0. ; 

// *****
//  FILTER  RATE
filtrate.L = 1. ; filtrate.n = 1. ; filtrate.A[ 0 ] = 1. ; 

// FILTER TIME ORIGIN
filttorigin.L = 1. ; filttorigin.n = 1. ; filttorigin.A[ 0 ] = 0. ; 


// MAX DELAY TIME
maxdelayt.L = 1. ; maxdelayt.n = 1. ; maxdelayt.A[ 0 ] = 0. ; 


// DELAY FILTER
delayfilter.L = 1. ; delayfilter.n = 0. ; delayfilter.A[ 0 ] = 0. ; 

// DELAY FILTER SEMITONE TRANSPOSE
delayftrans.L = 1. ; delayftrans.n = 1. ; delayftrans.A[ 0 ] = 0. ; 

// DELAY FILTER TRANSPOSE SHIFTER
delayfshift.L = 1. ; delayfshift.n = 1. ; delayfshift.A[ 0 ] = 0. ; 

//  0 DELAY DECIBELS
zerodelay_dB.L = 1. ; zerodelay_dB.n = 1. ; zerodelay_dB.A[ 0 ] = 0. ; 

//  MAX DELAY DECIBELS
maxdelay_dB.L = 1. ; maxdelay_dB.n = 1. ; maxdelay_dB.A[ 0 ] = 0. ; 

//  DELAY DECIBELS CURVE INDEX
delay_dB_warp.L = 1. ; delay_dB_warp.n = 1. ; delay_dB_warp.A[ 0 ] = 0. ; 

// SPECTRUM WARPSHAPE INDEX
warpshape.L = 1. ; warpshape.n = 1. ; warpshape.A[ 0 ] = 0. ; 

//  RANSMOOTH
ransmooth.L = 1. ; ransmooth.n = 1. ; ransmooth.A[ 0 ] = 0. ; 


// ranTbandwidth RANDOM TIME BANDWIDTH
ranTbandwidth.L = 1. ; ranTbandwidth.n = 1. ; ranTbandwidth.A[ 0 ] = 0. ; 

if( argc < 2 )usage() ; 


    while( (ch = crack( argc, argv,
    "_|=|a|A|b|B|c|C|d|D|e|E|f|F|h|H|i|I|K|l|L|m|M|N|p|P|q|Q|r|R|s|S|t|T|v|V|w|W|x|X|y|Y|z|Z|",     0  )) != CRACK_DONE_FLAG ) {
	switch(ch) { // x

	    case 'x':   strcpy(tempstring, arg_option);
			function_delay_time_scaler.fp = crackstring( tempstring, &function_delay_time_scaler ); 
			break;

	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'S':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;

	    case 'd':   dur = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

            case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;


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


	    case 'F':   strcpy(tempstring, arg_option); fixTildeInFilename( tempstring ) ;
			filter.fp = crackstring( tempstring, 
			    &filter );
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



	    case 'T':   strcpy(tempstring, arg_option);
			maxdelayt.fp = crackstring( tempstring, 
			    &maxdelayt );
			break;

// ***** NEW
	    case 'B':   strcpy(tempstring, arg_option);
			delayfilter.fp = crackstring( tempstring, 
			    &delayfilter );
			break;

	    case 'b':   strcpy(tempstring, arg_option);
			delayftrans.fp = crackstring( tempstring, 
			    &delayftrans );
			break;

	    case 'e':   strcpy(tempstring, arg_option);
			delayfshift.fp = crackstring( tempstring, 
			    &delayfshift );
			break;


	    case 'w':   delay_warpshape = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'V':   strcpy(tempstring, arg_option);
			zerodelay_dB.fp = crackstring( tempstring, 
			    &zerodelay_dB );
			break;

	    case 'y':   strcpy(tempstring, arg_option);
			maxdelay_dB.fp = crackstring( tempstring, 
			    &maxdelay_dB );
			break;

	    case 'z':   strcpy(tempstring, arg_option);
			delay_dB_warp.fp = crackstring( tempstring, 
			    &delay_dB_warp );
			break;

            case 'Z':	print_flag = (int) crackfloat( arg_option, ch ) ; break;




//*****



	    case 'v':   strcpy(tempstring, arg_option);
			ranswitch = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'r':   strcpy(tempstring, arg_option);
			ransmooth.fp = crackstring( tempstring, 
			    &ransmooth );
			break;
	    case 'q':   strcpy(tempstring, arg_option);
			ranTbandwidth.fp = crackstring( tempstring, 
			    & ranTbandwidth );
			break;



	    case 'E':   filtcompthresh = crackfloat( arg_option, ch ) ;
			break;
	    case 'c':   filtcompdecibels = crackfloat( arg_option, ch ) ;
			break;

// *****


	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;

	    case 'f':   strcpy(tempstring, arg_option);
			fsmooth.fp = crackstring( tempstring, 
			    &fsmooth );
			break;

	    case 'W':   strcpy(tempstring, arg_option);
			warpshape.fp = crackstring( tempstring, 
			    &warpshape );
			break;

	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	} 
 }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "TIME DELAY RESYNTHESIS", 69 ) ; 
prline( 69,  "-" ) ; 

    // READ IN FFT HEADER VALUES
    if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &i,  normamp, &filter, 1 ) == -1){
        fprintf( stderr, "CHECK YOUR ANALYSIS FILE.\t\t. . . BYE.\n\n\n" ) ; exit(EXIT_FAILURE) ; 
    } ; 



    //COPY PEAKAMPS INTO INPUT FILE PEAKAMPS
     for(k = 0; k < analysis_chan; k++)
	ipeakamp[ k ] = normamp[k] ; 
 
    
    // MAKE THE MASTER PEAK AMP
    normamppk = 0 ; 
    for(k = 0; k < analysis_chan; k++) 
	if( normamp[k] > normamppk) normamppk  = normamp[k]; 


//*******
// FIND DURATION OF INPUT FILE

    niframes = (((filter.n - (float) FFT_HEADER_SIZE) / (float) (analysis_N + 2))) / analysis_chan ; 
    analysis_dur = (float) (niframes) / ((float) analysis_R / (float) analysis_D ) ; 
    iframes_per_sec = (float) analysis_R /  (float) analysis_D ; 
    pri( niframes,  "NUMBER OF FRAMES IN ANALYSIS" ) ; 

//*******

    N = analysis_N; isr = R = analysis_R; D = analysis_D ; ichan = analysis_chan ; 
    idur = analysis_dur ; isr = R ; 

    if(channelout == 0){
	channelflag = 0 ; 
    } else{
	channelflag = 1 ; 
    }
 

// **** SET UPS *****
      if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 200.\n\n" ) ; 
	frames_per_sec = 200 ; 
    }
    I = D = (int) ((float) R / frames_per_sec) ; 


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
    factor = (float) R / ((float) D * TWOPI) ; 
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	

// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( (ptrans.n  != 1.) || (harmadd.n  != 1.) || 
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.)  ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
  

  
//*************************************************
// GROUP DELAY FILTER SETUP AREA
//*************************************************
// ******** FILTER FILE SETUPS
if( delayfilter.n < 1. ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A DELAY FILTER FUNCTION. BYE.\n" ) ;
    exit(EXIT_FAILURE);
}
    delay_analysis_N = delayfilter.n - 2 ; // @@@
 //*****************
// SET UP SOME DELAY FILTER FILE VALUES
    delay_analysis_fundamental = nyquist / (float) (delay_analysis_N / 2) ; 
    // MAKE RATIO OF ANALYSIS N TO SOURCE N FOR FILTER POSITIONING
    N_ratio = (float) delay_analysis_N / (float) N ; 
    
//*****************

//********************************************* @@@
// GROUP DELAY FILTER SETUPS

// ALLOCATE FILTER SPACE
fvec( delay_F,  delay_analysis_N+2  ) ;	/* GROUP DELAY FILTER */

//************FIRST TIME: FILL GROUP DELAY FILTER ARRAY FROM FILE
    fillfunc( &delayfilter, delay_F, (delay_analysis_N + 2)  ) ;     

// FIND THE MAX DELAY
    maxdelay = -999999. ; 
    for( i = 1; i < (delay_analysis_N + 2) ; i += 2 )
	if( delay_F[ i ] > maxdelay ) maxdelay = delay_F[ i ] ; 

    funcStats( &maxdelayt, &low, &hi, &avg, &length, &median ) ; 
    ringTime = maxdelay *= hi ; 

prf( ringTime, "ringTime" ) ; 
//********************************************* @@@

// DURATION
   if( dur <= 0.0 ) {
	dur = analysis_dur ; 
	prt( "(OUTPUT DURATION <= 0.0; RESETTING TO DURATION OF ANALYSIS.)" );  
   } ; 

// SET DURATION FOR FUNCTIONS TO ORIGINAL TIME.

    funcDur = dur ; 

    outdur = dur = dur + ringTime ; 
prf( dur, "OUTPUT DURATION" ) ; 



// SET UP OUTPUT FILE ACCORDING TO ANALYSIS RATE AND
    outfile_setup(argc, argv) ; // @@@





//*************************************************
//*************************************************
 		
// PRINT VALUES
//*****************
prbanner( "GROUP DELAY FILTER",  69 ) ; 
pri( delay_analysis_N,  "GROUP DELAY FILTER: FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( delay_analysis_fundamental, "GROUP DELAY FILTER:     FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 

prp( &delayftrans,  "GROUP DELAY FILTER: PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &delayfshift,  "GROUP DELAY FILTER: FREQUENCY SHIFT (in Hz)"  ) ; 
prf( delay_warpshape, "GROUP DELAY FILTER: WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 

prp( &zerodelay_dB, "GROUP DELAY FILTER: ZERO DELAY DECIBELS" ) ; 
prp( &maxdelay_dB, "GROUP DELAY FILTER: MAXIMUM DELAY DECIBELS" ) ; 
prp( &delay_dB_warp, "GROUP DELAY FILTER: DELAY DECIBELS CURVE INDEX" ) ; 
//*****************


prf( dur, "DURATION" ) ; 
pri( N,  "FFT SIZE" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
pri( R,  "SAMPLE RATE" ) ; 
pri( D,  "DECIMATION" ) ; 
pri( I,  "INTERPOLATION" ) ; 

prp( &function_delay_time_scaler, "FUNCTION DELAY TIME SCALER (0-1)" ) ; 

prp( &dBgain,  "MASTER GAIN (dB)"  ) ; 
prp( &ptrans,  "PITCH TRANSPOSITION (semitones)"  ) ; 
prp( &harmadd,  "FREQUENCY SHIFT (Hz)"  ) ; 
prp( &attack,  " ENVELOPE ATTACK TIME (seconds)"  ) ; 
prp( &release,  " ENVELOPE RELEASE TIME (seconds)"  ) ; 
prp( &fsmooth,  " FREQUENCY CHANGE RESPONSE TIME (seconds)"  ) ; 


prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (dB)" ) ; 


//pr( &?,  "~"  ) ; 

    fprintf( stderr,  "\n\n" ) ; 
    // *******

// FILTER WINDOW BOUNDARIES
//    if( filtwinhi.A[ 0 ] < 0.) filtwinhi.A[ 0 ] = analysis_dur ;




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
    if( channelout == 0 ){
	prt( "RESYNTHESIZING ALL CHANNELS" );
    } else{
	if( (channelout - 1) >= analysis_chan ){
	    // BAD CHANNEL
	    pri(  channelout, "\nCHANNEL" );
	    prt( " IS NOT AN ALLOWED CHANNEL. WILL CHANGE TO 1.\n" ) ; 
	}
	pri( channelout, "RESYNTHESIZING  CHANNEL" );
	
    }

prp( &filtrate,  "FILTER RATE MULTIPLIER"  ) ; 
prp( &filttorigin,  "FILTER TIME POINT-ORIGIN"  ) ; 
prf( filtcompthresh,  "FILTER COMPRESSION THRESHOLD (dB)"  ) ; 
prf( filtcompdecibels,  "FILTER DEGREE OF COMPRESSION (dB)"  ) ; 
prline( 1,  "*" ) ; 

prp( &maxdelayt,  "  DELAY WINDOW SIZE MULTIPLIER "  ) ; 
prp( & ranTbandwidth,  " DIFFUSION: RANDOM VARIATION BANDWIDTH (seconds)"  ) ; 
prp( &ransmooth,  " DIFFUSION: RANDOM VARIATION RESPONSE TIME (seconds)"  ) ; 
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
    
//*****************

// MAKE COMPRESSION STUFF
    if( filtcompthresh > 0.){
	fprintf(stderr," \n\n***** COMPRESSION THRESHOLD MUST BE <= 0dB. BYE.\n\n" ) ;
	exit(EXIT_FAILURE) ; 
    }
    if( filtcompdecibels > 0.){
	fprintf(stderr," \n\n***** COMPRESSION DECIBELS MUST BE <= 0dB. BYE.\n\n" ) ;
	exit(EXIT_FAILURE) ; 
    }
    filtcompthreshamp = dB_to_amp(filtcompthresh ); 

    filtcompamp = dB_to_amp( filtcompdecibels ); 
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
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */
    fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

    fvec( F_lower,  analysis_N+2  ) ;	/* lower filter array */
    fvec( F_higher,  analysis_N+2  ) ;	/* higher filter array */

    fvec( binranv,  (analysis_N+2) / 2  ) ;	/* bin random values */
    ivec( filtflow,  (analysis_N+2) / 2  ) ;	/* bin's low filter frame  */
    fvec( filtfprop,  (analysis_N+2) / 2  ) ;	/* bin's proportional position between frames  */
    fvec( binTimeDelays,  (analysis_N+2) / 2  ) ;	
    fvec( filtbindBprop,  (analysis_N+2) / 2  ) ;	/* bin's proportional dB curve position   */
    ivec( done,  (analysis_N+2) / 2  ) ;	/* bin flag for new values  */

    fvec( filtbint, N+2 ) ;	/* RESULTING DELAY AND AMP MULTIPLIER FOR BINS   */

//*********************************************
// INITIALIZE BIN TIMEPOINT RANDOMIZATION VALUES

    for( i = 1; i < (analysis_N + 2) / 2; i++) {
		if( ranswitch != 0 ) binranv[ i ] = randf( -1., 1. ) ;
		else binranv[ i ] = 0. ; 
    }  




//*************GROUP DELAY FILTER  EQ AND NORMALIZE
    if(delay_dBlow !=  delay_dBhi)prt("...............EQUALIZING AND NORMALIZING INPUT FREQUENCY RESPONSE....." ) ; 
    prt("...............NORMALIZING INPUT FREQUENCY RESPONSE....." ) ;
    eq( delay_F,  (delay_analysis_N + 2),  delay_dBlow,  delay_dBhi, delay_freqlow,  delay_freqhi,  delay_analysis_fundamental, 1,  0, 1 ) ; 

//************* GROUP DELAY FILTER  WARP
    if(delay_warpshape != 0) prt(".......WARPING GROUP DELAY FILTER RESPONSE.." ) ; 
    spectmagwarp( delay_F,  (delay_analysis_N + 2), delay_warpshape, 1 ) ;

//*********** PRINT TO TERMINAL IF DESIRED ******
    if(print_flag)tprintspec_groupdelay( delay_F, (delay_analysis_N + 2), delay_analysis_fundamental,  print_flag) ;

//*********************************************


//*********************************************

//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(channow = 0, outchan = beginchan; channow < ochan; channow++, outchan++ ){

    // **** SET UP ANALYSIS CHANNEL
	ainchan = (achannelout == 0) ? channow : (achannelout - 1) ; 
    // SETUP FILTER CONTROL VALUES
	filttorigin.A[ 0 ] = fval( &filttorigin, funcDur, 0. );
	filttnow = filttorigin.A[ 0 ] ; // ACCUMULATED FILTER TIME POINT IN SECONDS

prline( 69,   "=" ) ; 
pri( (ainchan+1), "INPUT ANALYSIS CHANNEL" ) ; 

    
    
    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; 

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

	
 //*********************************************
// LOOP FOR FRAMES
//*********************************************

   while ( t < dur ) {
	in += D ;
	on += I ;
	timenow( dur ) ;


//*************************
// GET THE VALUES
//*************************

//pd(2); 

//  SHIFT, GAIN, AND TRANSPOSITION

//		harmadd.A[ 0 ] =  fval( &harmadd, funcDur, t );
//pd(20); 
//		dBgain.A[ 0 ] =  fval( &dBgain, funcDur, t );
//		    gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
//pd(22); 
//		ptrans.A[ 0 ] = fval( &ptrans, funcDur, t );
//		pm = semitones_to_mult( ptrans.A[ 0 ] ) ;


		function_delay_time_scaler.A[ 0 ] = fval( &function_delay_time_scaler, funcDur, t );


	//** GROUP DELAY FILTER
			fs = delayfshift.A[ 0 ] = fval( &delayfshift, funcDur, t );
		    // IF FREQ SHIFT FOR SOURCE ONLY, SUBTRACT FROM FILTER CHANGE
		delayftrans.A[ 0 ] = fval( &delayftrans, funcDur, t );
		    fm = semitones_to_mult( delayftrans.A[ 0 ] ) ;
		    // IF PITCH TRANSPOSITION FOR SOURCE ONLY, DIVIDE OUT OF FILTER CHANGE

	//** 



		//RELEASE
//		release.A[ 0 ] = fval( &release, funcDur, t );
//		smooth_setup( release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 

		// ATTACK
//		attack.A[ 0 ] = fval( &attack, funcDur, t );
//		smooth_setup( attack.A[ 0 ], &attackc, &minusattackc, IR ) ; 


		// FRQ SMOOTH
//		fsmooth.A[ 0 ] = fval( &fsmooth, funcDur, t );
//		smooth_setup( fsmooth.A[ 0 ], &fsmoothc, &minusfsmoothc, IR ) ; 


    // FIND THE CURRENT SMOOTHING FACTOR
      ransmooth.A[ 0 ] = fval( &ransmooth, funcDur, t );
	 smooth_setup( ransmooth.A[ 0 ], &ransmoothv, &minusransmoothv, IR ) ; 

   // FIND THE CURRENT RANDOM TIME BANDWIDTH
		ranTbandwidth.A[ 0 ] = fval( & ranTbandwidth, funcDur, t );


//  TIME POINT DITHER WINDOW SIZE
	 maxdelayt.A[ 0 ] = fval( &maxdelayt, funcDur, t ) ; 
	dwin = maxdelayt.A[ 0 ] ; 

//fprintf( stderr, "\ndwin: %f", dwin ) ; 

//pd(3); 


//	 zerodelay_dB.A[ 0 ] = fval( &zerodelay_dB, funcDur, t ) ; 
//	 maxdelay_dB.A[ 0 ] = fval( &maxdelay_dB, funcDur, t ) ; 
// 	 delay_dB_warp.A[ 0 ] = fval( &delay_dB_warp, funcDur, t ) ; 


//*********************************************************

//*********************************************************
// FILTER TIME ISSUES

// ********** INCREMENT FILTER TIME TO NEXT FRAME
    // GET NEW RATE
    filtrate.A[ 0 ] = fval( &filtrate, funcDur, t );
    // SAVE OLD ORIGIN
    temp = filttorigin.A[ 0 ] ; 
    // GET NEW ORIGIN
    filttorigin.A[ 0 ] = fval( &filttorigin, funcDur, t );
    // GET CHANGE IN ORIGIN
    temp = filttorigin.A[ 0 ] - temp ;

    // SUMMED CHANGE
    temp =  (temp  + (filttinc * filtrate.A[ 0 ] )) ; 

    // ADD CHANGES FROM ORIGIN SHIFT AND THE CURRENT RATE TO THE NEXT FRAME POSITION.
    filttnow += temp ;



//pd(4); 

//******************************************************************************
// MAKE EACH BIN'S FILTER TIME


    for( i = 1,  j = 0; i < (analysis_N + 2); i +=2,  j++ ){
        // SET DONE FLAGS TO 0
        done[ j ] = 0 ; 
        // MAKE THE NEW RANDOM VALUE FOR THIS BIN
        if( ranswitch != 0 ) binranv[ j ] = 
            minusransmoothv * randf( -1., 1. )  +
            ransmoothv * binranv[ j ] ;  
            // ******FIND THE GROUP DELAY TIME MULTIPLIER FOR THIS BIN

            // FIND THE INDECES WHICH RESULT AFTER SHIFT AND TRANSPOSE OF 
            // THE GROUP DELAY FILTER
            // APPLY FILTER TRANSPOSE
            // SHIFT
            temp = (float) j ; // THE 0-(N/2) BIN WE ARE ON (TO BE FILTERED)
            temp = temp * N_ratio ; // SHIFT BY RATIO OF ANALYSIS N TO SOURCE N
            temp = temp -  ( fs / fundamental ) ; // SHIFT BY RATIO OF ANALYSIS N TO SOURCE N
            // TRANSPOSE
            temp =  temp / fm   ; // TRANSPOSE THE BIN
            i2p = temp - (float)((int) temp ) ; // FRACTIONAL BIN VALUE
            i1p = 1. - i2p ; // 1. - FRACTIONAL BIN VALUE
            i1 = 2 * (int) temp ; // THE LOWER BIN AMP INDEX
            i2 = i1 + 2 ; // THE UPPER BIN AMP INDEX
		    
            if( i1 < 0 ){			
                //UNDER THE ARRAY
                timeDelayForThisBin = delay_F[ 1 ] ; // TIME DELAY
                filtbint[ i - 1 ] = delay_F[ 0 ] ; 	// AMP MULTIPLIER		

            }else if( i2 > (delay_analysis_N - 2)  ){ 
                //OVER THE ARRAY
                timeDelayForThisBin = delay_F[ delay_analysis_N - 1 ]  ; // TIME DELAY
                filtbint[ i - 1 ] = delay_F[ delay_analysis_N - 2 ] ; // AMP MULTIPLIER			

            }else{
                // IN ARRAY
                // TIME DELAY
                timeDelayForThisBin = ( delay_F[ i1 + 1 ] * i1p ) + (delay_F[ i2 + 1 ] * i2p ) ;  
                    // AMP MULTIPLIER
                filtbint[ i - 1 ] = ( delay_F[ i1  ] * i1p ) + (delay_F[ i2  ] * i2p ) ; 			
            }
		    


            // *********

            // USE THE RANDOM VALUE TO FIND THE  TIME  PAST NOW
            // THIS BIN

            binTimeDelays[ j ]  = (timeDelayForThisBin  * dwin ) + (ranTbandwidth.A[ 0 ] * binranv[ j ]) ; 

            // BEFORE REPOSITIONING THE BIN TIME,
            // MAKE THE BIN'S PROPORTIONAL DECIBEL CURVE VALUE
            if(dwin <= 0.) filtbindBprop[ j ] = 0. ; 
            else filtbindBprop[ j ] = binTimeDelays[ j ] / ( maxdelay * dwin ) ; 

            // MAKE THE ACTUAL TIME FOR THIS BIN BY ADDING TIME NOW
            filtbint[ i ] = (-1. * binTimeDelays[ j ]) + filttnow ; 


            // MAKE THE BIN'S LOW FILTER FRAME
            filtf = iframes_per_sec * filtbint[ i ] ; // VIRTUAL FRAME 
            filtflow[ j ] = (int) filtf ; // LOWER FRAME
            if( filtflow[ j ] < 0 ) filtflow[ j ] = 0 ; 
            if( filtflow[ j ] > (niframes - 2) ) filtflow[ j ] = niframes - 2 ; 
            // MAKE THE BIN'S PROPORTIONAL POSITION BETWEEN FRAMES
            filtfprop[ j ] =  filtf - (float) filtflow[ j ] ;

        }
            
//******************************************************************************
// MAKE THE FILTER FRAME
//******************************************************

        // LOOP FOR BINS
        for( i = 1,  j = 0; i < (analysis_N + 2); i +=2,  j++ ){
            // LOOP FOR FRAMES
            if( done[ j ] == 0 ){
                // NEW FRAME PAIR
                filtfhigh = filtflow[ j ] + 1 ; // HIGHER FRAME

                // READ IN THE LOWER AND HIGHER FRAMES
	    
                // POSITION TO LOWER FRAME
                k = ((FFT_HEADER_SIZE + 
			    ((ainchan + (analysis_chan * filtflow[ j ]) ) 
				* (analysis_N + 2) ) ) * sizeof(float)) ;
                fseek( filter.fp,  k,  SEEK_SET ) ; 
	           // READ IN LOWER FRAME		
		     fread( F_lower, sizeof(float), analysis_N+2, filter.fp ) ;
    
                // POSITION TO HIGHER FRAME
		     k = ((FFT_HEADER_SIZE + 
			    ((ainchan + (analysis_chan * filtfhigh) ) 
				* (analysis_N + 2) ) ) * sizeof(float)) ; 
		     fseek( filter.fp,  k,  SEEK_SET ) ; 
                // READ IN HIGHER FRAME		
                fread( F_higher, sizeof(float) , analysis_N+2, filter.fp ) ;

                // MAKE INTERPOLATED FRAME FOR ALL BINS WHICH USE THIS PAIR
                for( kk = i,  l = j; kk <  (analysis_N + 2); kk +=2,  l++ ){
                    if(filtflow[ l ] == filtflow[ j ]){
                    // SET UP THIS BIN
                    zerodelay_dB.A[ 0 ] = fval( &zerodelay_dB, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) ) ; 
                    maxdelay_dB.A[ 0 ] = fval( &maxdelay_dB, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) ) ; 
                    delay_dB_warp.A[ 0 ] = fval( &delay_dB_warp, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) ) ; 
                    // FIRST GET THE AMP MULTIPLIER
                    binamp = curve( zerodelay_dB.A[ 0 ],  maxdelay_dB.A[ 0 ], 
				    filtbindBprop[ l ],  delay_dB_warp.A[ 0 ] ) ;  
                    binamp = filtbint[kk - 1] * dB_to_amp( binamp );	
                    channel[kk - 1] = binamp * 
			       ( F_lower[kk - 1] + (filtfprop[ l ] * (F_higher[kk - 1] - F_lower[kk - 1])) ); 
                    channel[kk] = 
			     	   F_lower[kk] + (filtfprop[ l ] * (F_higher[kk] - F_lower[kk])) ; 
                    done[ l ] = 1 ; 
                }
            }
        }

    }

    // SETUP PREVIOUS CHANNEL
    if( !frame_count )for(i = 0; i < (analysis_N + 2); i++) previous_channel[ i ] = channel[ i ] ; 


//pd(8); 
		
//******************************************************
//*************NORMALIZE THE INPUT SPECTRUM
    normalize(  channel,  (analysis_N + 2),   normamppk  ) ;

//**************COMPRESS THE INPUT SPECTRUM
   if( compflag == 1 )
	compress( channel,  (analysis_N + 2),  filtcompthreshamp, filtcompamp,  filtcompnormamp ) ; 

//*************WARP THE INPUT SPECTRUM
	warpshape.A[ 0 ] =  fval( &warpshape, funcDur, t );
    spectmagwarp( channel,  (analysis_N + 2), warpshape.A[ 0 ], 0 ) ;

//*************EQUALIZE THE INPUT SPECTRUM
    eq( channel,  (analysis_N + 2),  dBlow,  dBhi, freqlow,  freqhi,  analysis_fundamental, 1,  0, 0 ) ; 

//************* GAIN SCALE BACK
    gainscale(  channel,  (analysis_N + 2),   normamppk  ) ;



//*****************
// MODIFICATIONS LOOP
//*****************


		 for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){

//************ATTACK AND RELEASE

		    if((channel[i - 1] < previous_channel[i - 1]) ){
                   release.A[ 0 ] = fval( &release, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) );
		        smooth_setup( release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 
		        // RELEASE
			  channel[i - 1] = (releasec * previous_channel[i - 1] )
				+ ( minusreleasec * channel[i - 1] ); 
		    } else {
                   attack.A[ 0 ] = fval( &attack, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) );
                   smooth_setup( attack.A[ 0 ], &attackc, &minusattackc, IR ) ; 
		        // ATTACK
			  channel[i - 1] =  (attackc * previous_channel[i - 1] )
				+ ( minusattackc * channel[i - 1] ); 
		    }

		// FREQ SMOOTH
               fsmooth.A[ 0 ] = fval( &fsmooth, funcDur, t );
               smooth_setup( fsmooth.A[ 0 ], &fsmoothc, &minusfsmoothc, IR ) ; 
			channel[i] =  (fsmoothc * previous_channel[i] )
				+ ( minusfsmoothc * channel[i] ); 


		    previous_channel[i - 1] = channel[i - 1] ; 
		    previous_channel[i] = channel[i] ; 

//******* 

// SHIFT BY -a AND TRANSPOSE BY -P
                harmadd.A[ 0 ] =  fval( &harmadd, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) );

                ptrans.A[ 0 ] = fval( &ptrans, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) );
                pm = semitones_to_mult( ptrans.A[ 0 ] ) ;

		     temp = pm * (channel[i] + harmadd.A[ 0 ]) ;
		    
		    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		    if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
		    else channel[i] = temp ; 


               dBgain.A[ 0 ] =  fval( &dBgain, funcDur, 
					t - (function_delay_time_scaler.A[ 0 ] * binTimeDelays[ j ]) );
                   gain = dB_to_amp( dBgain.A[ 0 ] ) ; 

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

    fprintf(stderr,"\nDELAYFILTER : RESYNTHESIS COMPLETED\n");

 
    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;

   if( release.n != 1. ) fclose(release.fp ) ;
   if( attack.n != 1. ) fclose(attack.fp ) ;
   if( filter.n != 1. ) fclose(filter.fp ) ;
   if( ransmooth.n != 1. ) fclose(ransmooth.fp ) ;



    exit(EXIT_SUCCESS) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"delayfilter:  group delay  phase vocoder resynthesis\n"
	"delayfilter   [flags]  [output file ]\n"
	"	    	Pre-formatted output sound file required.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	S:	window type: 0 = hamming,  1 = rectangular  \n"
	"		    2 = Blackman,  3 = Bartlett triangular [0.]\n"
	"		    4-12 = Kaiser windows for alpha = 4-12,  respectively\n"
	"		    (representative sidelobe levels for alpha: \n"
	"		      4 = -30dB,  8 = -58 dB,  12 = -90 dB)\n"
	"	D:	analysis frames per second [200]\n"

	"	d:	output duration in seconds \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"

	"	x:	function delay time scaler for post-delay modification parameters (fun)[0.]\n"
	"	P:	pitch transposition in semitones (func) [0]\n"
	"	a:	frequency shift factor (bin frequency adder, before -P )(func)[0.] \n"
	"	A:	gain in decibels (func) [0.] \n"

	"	F:	phase vocoder analysis file \n"

	"	Q:	DATA time point (func) [0.] \n"
	"	Y:	DATA rate multiplier (func) [1.]\n"
	"		    (1. = rate of original, 2. = twice as fast, etc.)\n"
	"		    (negative = reverse,  0 = stationary) \n"

	"	B:	GROUP DELAY FILTER: group delay response file \n"
	"		    (use groupdelaymaker to make file)\n"
	"	    ** GROUP DELAY FILTER **\n"
	"	Z:	GROUP DELAY FILTER: print group delay filter response file\n"
	"		    analysis average to standard error (1 = yes, 0 = no) [0]\n"
	"	b:	GROUP DELAY FILTER: frequency  transposition in semitones \n"
	"			    of group delay response (func) [0]\n"
	"	e:	GROUP DELAY FILTER: frequency  shift of group delay response (func) [0.] \n"

	"	w:	GROUP DELAY FILTER: warp index for reshaping  response [0.] \n"
	"		    (values > 0 reduce integral of delay, < 0 increases it)\n"

	"	V:	GROUP DELAY FILTER: zero delay decibels (func) [0]\n"
	"	y:	GROUP DELAY FILTER: maximum delay decibels (func) [0]\n"
	"	z:	GROUP DELAY FILTER: delay decibels curve shape (func) [0]\n"


	"	T:	GROUP DELAY FILTER: delay time multiplier (func) [1.] \n"
	"	v:	GROUP DELAY FILTER DIFFUSION: \n"
	"			random variation switch (1 = on,  0 = off) [0]\n"
	"	q:	GROUP DELAY FILTER DIFFUSION: \n"
	"			random variation bandwidth in seconds \n"
	"				    (0 (off) -> ? ) (func) [0.] \n"
	"	r:	GROUP DELAY FILTER DIFFUSION: \n"
	"			random variation response time size in seconds \n"
	"				    (0 (off) -> ? ) (func) [0.] \n"
	"	    ****\n"
	
	"	E:	INPUT SPECTRUM compression threshold (in decibels) [0] \n"
	"	c:	INPUT SPECTRUM  decibels of compression [0] \n"

	"	W:	INPUT SPECTRUM warp index for reshaping magnitude response (func) [0.] \n"
	"		    values > 0 close down response, < 0 open it up\n"

	"	l:      OUTPUT envelope attack time  (func) [0.]\n"
	"	L:      OUTPUT envelope release time   (func) [0.]\n"
	"	f:      OUTPUT frequency change response time   (func) [0.]\n"

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

