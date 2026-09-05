#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k;
 
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int analysis_N,  analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 
int   eof = 0, obank = 0,  channelout=0;
 float P = 1.0;
  FILE *fopen();
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel, *normalization_channel, *output ;
float *previous_channel,  *F_lower,  *F_higher, *T_lower,  *T_higher,  
		 *channel_freqdev,  *previous_ranampv, 
    *previous_ranfreqv, *tempChannel ; 
float threshfac = .001,  threshfacdB=-60,  tempdB,  ranfreqboundsdiff ;

int   LoopNormalizationFlag=0 ; 
float loopSmoothTime=0.0 ; 

 
 
float	pm,  gain=1.;
double ar_dB ; 
float  temp,  temp2;  
float getthresh();
float normamp[MAXIMUM_CHANNELS],  normamppk ;
 
float   IR,  dur=0., saved_dur ;
float fundamental ;
 
float analysis_fundamental ;  
 
int ainchan ; 

// SHELF EQ
//float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 

int ranampflag=0, ranfreqflag=0  ; 

float releasec,  minusreleasec,  attackc,  minusattackc ; 
float  fsmoothc,  minusfsmoothc ; 
float  tsmoothc,  minustsmoothc ; 
float  ranampsmoothc,  minusranampsmoothc ; 

float  ranfreqsmoothc,  minusranfreqsmoothc ; 

//** ANALYSIS DATA VARIABLES
float analysisDatatnow=0., oldanalysisDatatnow=0.,   analysisDatatinc; 

int Mode__sampler_loop_0__autostop_1=0,  autostopflag=0,  wrap_0_fold_1_clip_2=0, 
	Onset_and_Release_Segment_Mode__off_0__on_1=0   ; 


//int imode=1. ; 
float dwin ; 

char tempstring[ STRING_SIZE ] ; 


// FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PEAK LOOP SMOOTH TIME
struct func peakLoopSmoothTime ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 

//  BIN AMP SOURCE dB
struct  func  sourcedB ; 

//  ATTACK
struct  func  attack ; 

//  RELEASE
struct  func  release ; 

//  FREQUENCY CHANGE RESPONSE TIME
struct  func  fsmooth ; 

// DATA
struct  func  analysisData ; 

// ****
//  DATA  RATE
struct  func  analysisDatarate ; 

// DATA TIME POINT ORIGIN
struct  func  analysisDatatorigin ; 

// DATA TIME WINDOW LOWER BOUNDARY
struct  func  analysisDatawinlow ; 

// DATA TIME WINDOW UPPER BOUNDARY
struct  func  analysisDatawinhi ; 


// DATA TIME POINT DITHER WINDOW
struct  func  ditherwin ; 

//  TIME POINT CHANGE RESPONSE TIME
struct  func  tsmooth ; 



// SPECTRUM WARPSHAPE INDEX
struct  func  warpshape ; 


//  RANDOM AMPLITUDE VARIATION  
struct  func  randomampdB ; 
struct  func  randomampvarresponse ; 

// RANDOM AMP AND FREQ VARIATION ROLLOFF
struct  func  high_randomvarfreq ; 
struct  func  low_randomvarfreq ; 
struct  func  randomvartransindex ; 


// RAMDOM FREQ VARIATION PROPORTION
struct  func  randomfreqprop ; 

// RAMDOM FREQ VARIATION RESPONSE TIME
struct  func  randomfreqresponse ; 

// RAMDOM FREQ VARIATION CURVE INDEX
struct  func  randomfreqcurve ; 




//SHELF EQ
struct  func  dBlow;
struct  func  dBhi;
struct  func  freqlow;
struct  func  freqhi ;



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

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 

//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  FREQUENCY CHANGE RESPONSE TIME
fsmooth.L = 1. ; fsmooth.n = 1. ; fsmooth.A[ 0 ] = 0. ; 

// ANALYSIS DATA 
analysisData.L = 1. ; analysisData.n = 0. ; analysisData.A[ 0 ] = 0. ; 

// *****
//  ANALYSIS DATA   RATE
analysisDatarate.L = 1. ; analysisDatarate.n = 1. ; analysisDatarate.A[ 0 ] = 1. ; 

// ANALYSIS DATA  TIME ORIGIN
analysisDatatorigin.L = 1. ; analysisDatatorigin.n = 1. ; analysisDatatorigin.A[ 0 ] = 0. ; 

// ANALYSIS DATA  TIME WINDOW LOWER BOUNDARY
analysisDatawinlow.L = 1. ; analysisDatawinlow.n = 1. ; analysisDatawinlow.A[ 0 ] = 0. ; 

// ANALYSIS DATA  TIME WINDOW UPPER BOUNDARY
analysisDatawinhi.L = 1. ; analysisDatawinhi.n = 1. ; analysisDatawinhi.A[ 0 ] = -1. ; 


// DATA TIME POINT DITHER WINDOW
ditherwin.L = 1. ; ditherwin.n = 1. ; ditherwin.A[ 0 ] = 0. ; 

//  TIME POINT CHANGE RESPONSE TIME
tsmooth.L = 1. ; tsmooth.n = 1. ; tsmooth.A[ 0 ] = 0. ; 

// SPECTRUM WARPSHAPE INDEX
warpshape.L = 1. ; warpshape.n = 1. ; warpshape.A[ 0 ] = 0. ; 

// RAMDOM AMP VARIATION RESPONSE TIME

//  RANDOM AMPLITUDE VARIATION  
randomampdB.L = 1. ; randomampdB.n = 1. ; randomampdB.A[ 0 ] = 0. ; 
randomampvarresponse.L = 1. ; randomampvarresponse.n = 1. ; randomampvarresponse.A[ 0 ] = 0. ; 

// RANDOM AMP AND FREQ VARIATION ROLLOFF
high_randomvarfreq.L = 1. ; high_randomvarfreq.n = 1. ; high_randomvarfreq.A[ 0 ] = 22050. ; 
low_randomvarfreq.L = 1. ; low_randomvarfreq.n = 1. ; low_randomvarfreq.A[ 0 ] = 0. ; 
randomvartransindex.L = 1. ; randomvartransindex.n = 1. ; randomvartransindex.A[ 0 ] = 0. ; 


// RAMDOM FREQ VARIATION PROPORTION
randomfreqprop.L = 1. ; randomfreqprop.n = 1. ; randomfreqprop.A[ 0 ] = 0. ; 

// RAMDOM FREQ VARIATION RESPONSE TIME
randomfreqresponse.L = 1. ; randomfreqresponse.n = 1. ; randomfreqresponse.A[ 0 ] = 0. ; 


// RAMDOM FREQ VARIATION CURVE INDEX
randomfreqcurve.L = 1. ; randomfreqcurve.n = 1. ; randomfreqcurve.A[ 0 ] = 0. ; 


// SHELF EQ
dBlow.L = 1. ; dBlow.n = 1. ; dBlow.A[ 0 ] = 0. ; 
dBhi.L = 1. ; dBhi.n = 1. ; dBhi.A[ 0 ] = 0. ; 
freqlow.L = 1. ; freqlow.n = 1. ; freqlow.A[ 0 ] = 200. ; 
freqhi.L = 1. ; freqhi.n = 1. ; freqhi.A[ 0 ] = 2000. ; 

strcpy( routine, "twarp" ) ; 


if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv,
    "_|=|a|A|b|B|C|d|D|e|f|F|g|G|H|i|I|j|J|k|K|l|L|m|M|n|N|o|p|P|q|Q|r|R|s|S|t|T|v|V|w|W|X|Y|z|Z|", 
     0  )) != CRACK_DONE_FLAG ) { // |h
	switch(ch) {
	    case 'M':   Nw = (int) crackfloat( arg_option, ch ); // crackfloat( arg_option, ch )
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;

	    case 'd':   dur = saved_dur = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;


           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;




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

	    case 'b':   strcpy(tempstring, arg_option);
			peakLoopSmoothTime.fp = crackstring( tempstring, &peakLoopSmoothTime );
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
			analysisData.fp = crackstring_bin_only( tempstring, 
			    &analysisData );
			break;
 

// *****
	    case 'Q':   strcpy(tempstring, arg_option);
			analysisDatatorigin.fp = crackstring( tempstring, 
			    &analysisDatatorigin );
			break;
	    case 'Y':   strcpy(tempstring, arg_option);
			analysisDatarate.fp = crackstring( tempstring, 
			    &analysisDatarate );
			break;

	    case 'g':   strcpy(tempstring, arg_option);
			analysisDatawinlow.fp = crackstring( tempstring, 
			    &analysisDatawinlow );
			break;
	    case 'G':   strcpy(tempstring, arg_option);
			analysisDatawinhi.fp = crackstring( tempstring, 
			    &analysisDatawinhi );
			break;

	    case 'T':   strcpy(tempstring, arg_option);
			ditherwin.fp = crackstring( tempstring, 
			    &ditherwin );
			break;
	
	    case 'r':	Onset_and_Release_Segment_Mode__off_0__on_1 = (int) crackfloat( arg_option, ch );
			break;
	    case 'o':	wrap_0_fold_1_clip_2 = (int) crackfloat( arg_option, ch );
			break;


	    case 'K':   strcpy(tempstring, arg_option);
			tsmooth.fp = crackstring( tempstring, 
			    &tsmooth );
			break;


	    case 'S':   Mode__sampler_loop_0__autostop_1 = (int) crackfloat( arg_option, ch ) ;
			break;




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




	    case 'B':	strcpy(tempstring, arg_option);
			randomampdB.fp = crackstring( tempstring, 
			    &randomampdB );
			break;
	    case 'I':	strcpy(tempstring, arg_option);
			high_randomvarfreq.fp = crackstring( tempstring, 
			    & high_randomvarfreq );
			break;
	    case 'J':	strcpy(tempstring, arg_option);
			low_randomvarfreq.fp = crackstring( tempstring, 
			    & low_randomvarfreq );
			break;
	    case 'N':	strcpy(tempstring, arg_option);
			randomvartransindex.fp = crackstring( tempstring, 
			    & randomvartransindex );
			break;
	    case 'e':   strcpy(tempstring, arg_option);
			randomampvarresponse.fp = crackstring( tempstring, 
			    & randomampvarresponse );
			break;

	    case 'v':   LoopNormalizationFlag = (int) crackfloat( arg_option, ch ) ; 
			break;

	    case 'j':   strcpy(tempstring, arg_option);
			randomfreqprop.fp = crackstring( tempstring, 
			    &randomfreqprop );
			break;

	    case 'k':   strcpy(tempstring, arg_option);
			randomfreqresponse.fp = crackstring( tempstring, 
			    &randomfreqresponse );
			break;


	    case 'n':   strcpy(tempstring, arg_option);
			randomfreqcurve.fp = crackstring( tempstring, 
			    &randomfreqcurve );
			break;

	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	} 
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "TIME WARP RESYNTHESIS", 69 ) ; 
prline( 69,  "-" ) ; 

    // READ IN FFT HEADER VALUES
    if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &k,  normamp, & analysisData, 1 ) == -1){
        fprintf( stderr, "CHECK YOUR ANALYSIS FILE.\t\t. . . BYE.\n\n\n" ) ; exit(EXIT_FAILURE) ; 
    } ; 



//prs( afile, "IN TWARP afile:" ) ; 
    // MAKE THE MASTER PEAK AMP
    normamppk = 0 ; 
    for(k = 0; k < analysis_chan; k++) 
	if( normamp[k] > normamppk) normamppk  = normamp[k]; 
//prf( amp_to_dB( normamppk ), "PEAK NORMALIZATION AMP IN DB" ) ; 
		
    //COPY PEAKAMPS INTO INPUT FILE PEAKAMPS
     for(k = 0; k < analysis_chan; k++)
	ipeakamp[ k ] = normamp[k] ; 
 
    //NO AUDIO INPUT FILE: SET OUTPUTFORMAT TO SHORTS
    // IF SET TO AUTOMATIC
//    if( outputformat == 0 )outputformat = 1 ;  
 

//*******
// FIND DURATION OF INPUT FILE

    niframes = (((analysisData.n - (float) FFT_HEADER_SIZE) / (float) (analysis_N + 2))) / analysis_chan ; 
    analysis_dur = (float) (niframes) / ((float) analysis_R / (float) analysis_D ) ; 
    iframes_per_sec = (float) analysis_R /  (float) analysis_D ; 
    pri( niframes,  "NUMBER OF FRAMES IN ANALYSIS" ) ; 

//*******

// DURATION
   if( dur <= 0.0 ) {
	dur = saved_dur = analysis_dur ; 
	prt( "(OUTPUT DURATION <= 0.0; RESETTING TO DURATION OF ANALYSIS DATA.)" );  
   } ; 

prf( dur, "OUTPUT DURATION" ) ; 
    outdur = dur ; 




    N = analysis_N; isr = R = analysis_R; D = analysis_D ; ichan = analysis_chan ; 
    idur = analysis_dur ; isr = R ; 

    if(channelout == 0){
	channelflag = 0 ;
        beginchan = 0 ;         
    } else{
	channelflag = 1 ; 
        beginchan = channelout -1 ; 
    } ; 

// ********** 

	// ANALYSIS DATA  WINDOW BOUNDARIES
    // -1 FLAGS 
    if( analysisDatawinlow.A[0] < 0.0 ) analysisDatawinlow.A[0] = 0.0 ; 
    if( analysisDatawinhi.A[0] < 0.0 ) analysisDatawinhi.A[0] = analysis_dur ; 

 

// SET UP OUTPUT FILE ACCORDING TO ANALYSIS DATA  RATE AND
    outfile_setup(argc, argv) ; 


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
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	

    // SET RANDOM AMPLITUDE VARIATION FLAG
    if( ( randomampdB.A[0] != 0. ) || ( randomampdB.n != 1 ) ) ranampflag = 1 ; 


    // SET RANDOM FREQ VARIATION FLAG
    if( ( randomfreqprop.A[0] != 0. ) || ( randomfreqprop.n != 1 ) ) ranfreqflag = 1 ; 



// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( (ptrans.n  != 1.) || (harmadd.n  != 1.) || 
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.)  ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
    
 		
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
prp( &fsmooth,  " FREQUENCY CHANGE RESPONSE TIME (seconds)"  ) ; 




prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (dB)" ) ; 


//pr( &?,  "~"  ) ; 

    fprintf( stderr,  "\n\n" ) ; 
    // *******


    temp2 = (float) R  / (float) I  ; 
fprintf( stderr,"\n******** TWARP  OUTPUT *******************" ) ; 
fprintf( stderr,"\nOUTPUT DURATION: %f seconds",  dur ) ; 
fprintf( stderr,"\nOUTPUT: %d frames/sec ", (int) temp2 ) ; 
fprintf( stderr,"\nOUTPUT: %d total frames", (int) (temp2 * dur) ) ;


fprintf( stderr,"\n********** ANALYSIS FILE ********************" ) ; 
prf( analysis_dur,	  "INPUT ANALYSIS DATA : DURATION" ) ; 
pri( analysis_N,  "INPUT ANALYSIS DATA : FFT SIZE" ) ; 
pri( analysis_R,  "INPUT ANALYSIS DATA : SAMPLE RATE" ) ; 
pri( analysis_D,  "INPUT ANALYSIS DATA : DECIMATION" ) ; 
pri( analysis_chan,  "INPUT ANALYSIS DATA : NUMBER OF CHANNELS" ) ;
// ***** VERIFY ANALYSIS DATA  CHANNEL
    if( channelout == 0 ){
	prt( "RESYNTHESIZING ALL CHANNELS" );
    } else{
	if( (channelout - 1) >= analysis_chan ){
	    // BAD CHANNEL
	    pri(  channelout, "\nCHANNEL" );
	    prt( " IS NOT AN ALLOWED CHANNEL.\n" ) ;
            exit(EXIT_FAILURE) ;  
	}
	pri( channelout, "RESYNTHESIZING  CHANNEL" );
	
    }

prp( &analysisDatarate,  "ANALYSIS DATA  RATE MULTIPLIER"  ) ; 
prp( &analysisDatatorigin,  "ANALYSIS DATA  TIME POINT-ORIGIN"  ) ; 

prp( &analysisDatawinlow,  "ANALYSIS DATA  TIME WINDOW LOW BOUNDARY"  ) ; 
prp( &analysisDatawinhi,  "ANALYSIS DATA  TIME WINDOW HIGH BOUNDARY"  ) ; 

if( Mode__sampler_loop_0__autostop_1 == 0){
	prt( "SET TO SAMPLER LOOP MODE" ) ;
	if( wrap_0_fold_1_clip_2 == 0 )prt( "LOOP METHOD: WRAP" ) ; 
	if( wrap_0_fold_1_clip_2 == 1 )prt( "LOOP METHOD: FOLD" ) ; 
	if( wrap_0_fold_1_clip_2 == 2 )prt( "LOOP METHOD: CLIP/LIMIT" ) ; 
	prp( &peakLoopSmoothTime, "LOOP SMOOTH TIME" ) ; 
	if( Onset_and_Release_Segment_Mode__off_0__on_1 == 1) prt( "USING ONSET/RELEASE MODE" ) ;
}else 
	prt( "SET TO AUTOSTOP MODE" ) ; 



prline( 1,  "*" ) ; 
prp( &ditherwin,  "DITHER WINDOW (SECONDS)"  ) ; 
prp( &tsmooth,  "TIME POINT CHANGE RESPONSE TIME (SECONDS)"  ) ; 


prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prp( &freqlow, "LOW SHELF FREQUENCY" ) ; 
prp( &dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prp( &freqhi, "HIGH SHELF FREQUENCY" ) ; 
prp( &dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
prp( &warpshape, "INPUT SPECTRUM WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 


prp( &randomampdB, "RANDOM AMPLITUDE VARIATION: DECIBELS " ) ; 
prp( & randomampvarresponse, "RANDOM AMPLITUDE VARIATION:  RESPONSE TIME " ) ; 

prp( &randomfreqprop, "RANDOM FREQUENCY VARIATION  PROPORTION " ) ; 
prp( &randomfreqresponse, "RANDOM FREQUENCY VARIATION  RESPONSE TIME " ) ; 
prp( &randomfreqcurve, "RANDOM FREQUENCY VARIATION  DISTRIBUTION CURVE INDEX " ) ; 

prp( & high_randomvarfreq, "RANDOM AMP AND FREQ VARIATION: HIGH ROLLOFF FREQUENCY " ) ; 
prp( & low_randomvarfreq, "RANDOM AMP AND FREQ VARIATION: LOW CUTOFF FREQUENCY" ) ; 
prp( & randomvartransindex, "RANDOM AMP AND FREQ VARIATION: HIGH-TO-LOW ROLLOFF SHAPE INDEX" ) ; 


prf( rescalev, "DECIBEL RESCALE VALUE" ) ; 


fprintf( stderr,"\n*******************************************\n\n" ) ; 

//***********
// SETUP ANALYSIS DATA  CONTROL VALUES
	         // ANALYSIS DATA  INCREMENT IN SECONDS
    analysisDatatinc =  (float) D / (float) R ; 
//***********

// MAKE THRESH AMP
    threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	

 //*****************
// SET UP SOME ANALYSIS DATA  FILE VALUES
    analysis_fundamental = nyquist / (float) (analysis_N/2) ; 
prf( analysis_fundamental, "analysis_fundamental" ) ;     

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
    fvec( normalization_channel, N+2 ) ;	/* analysis channels */



    fvec( F_lower,  analysis_N+2  ) ;	/* lower analysisData array */
    fvec( F_higher,  analysis_N+2  ) ;	/* higher analysisData array */
    fvec( T_lower,  analysis_N+2  ) ;	/* lower analysisData array */
    fvec( T_higher,  analysis_N+2  ) ;	/* higher analysisData array */

    fvec( tempChannel,  analysis_N+2  ) ; 

    fvec( channel_freqdev,  N + 2 ) ;	// channel SORT ARRAY ACCUMULATOR

    fvec( previous_ranampv, N+2 ) ;	/* channels old random amp value */
    fvec( previous_ranfreqv, N+2 ) ;	/* channels old random freq value  */



//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(channow = 0, outchan = beginchan; channow < ochan; channow++, outchan++ ){

    dur = saved_dur ; 

    // **** SET UP ANALYSIS DATA  CHANNEL
    ainchan = outchan ; 

prline( 69,   "=" ) ; 
pri( (ainchan+1), "INPUT ANALYSIS DATA CHANNEL" ) ; 

    
    
    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; autostopflag = 0 ;

//**** SEED RANDOM
    srandom(1);


//***** ZERO OLD RAN VALUES
    for( i = 0; i < (N + 2); i++ ) {
	previous_ranampv[i] = 0. ; 
	previous_ranfreqv[i] = 0. ; 
    }

    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;

//********** MAKE SOME ANNOUNCEMENTS
//    if(dBhi != 0. )fprintf(stderr," \nUSING ANALYSIS EQ ......" ) ;
    fprintf(stderr," \n") ; 




	
 //*********************************************
// LOOP FOR FRAMES
//*********************************************

   while ( (t < dur) && (autostopflag == 0) ) {
	in += D ;
	on += I ;
	timenow( dur ) ;

	// SETUP FREQDEV HISTORY ARRAY
	for( i = 1; i < (N + 2); i+= 2 ) {
	    channel_freqdev[ i - 1 ] = 0. ; channel_freqdev[ i ] = 1. ; 
	} ; 


//*************
// ANALYSIS DATA TIME ISSUES

	// CONSTRAIN TIME POINT TO WINDOW OR ANALYSIS DATA. 
	findFilterTimeAndConstrainByWindow(
	    analysisDatatinc,
	    &analysisDatatorigin, 
	    &analysisDatarate,
	    Onset_and_Release_Segment_Mode__off_0__on_1,
	    Mode__sampler_loop_0__autostop_1, 
	    &autostopflag, 
	    wrap_0_fold_1_clip_2, 
	    &analysisDatatnow, 
	    &oldanalysisDatatnow, 
	    &analysisDatawinlow, 
	    &analysisDatawinhi,
	    &dur,
	    analysis_dur,
            0,
            1.
	) ; 


	loopSmoothTime = makeLoopSmoothTime(
	    analysisDatatnow,
	    wrap_0_fold_1_clip_2,
	    Mode__sampler_loop_0__autostop_1,
	    dur,  
	    &analysisDatawinlow,    
	    &analysisDatawinhi,
	    &peakLoopSmoothTime
	) ; 


//fprintf( stderr, "time: %f\t analysisDatatnow: %f\t oldanalysisDatatnow : %f\t, diff: %f\t loopSmoothTime: %f\n", 
//	t, analysisDatatnow, oldanalysisDatatnow, (analysisDatatnow - oldanalysisDatatnow), loopSmoothTime ) ;  


//**************


//  TIME POINT DITHER WINDOW
	ditherwin.A[ 0 ] = fval( &ditherwin, dur, t );
	if( ditherwin.A[ 0 ] > 0.){
		    dwin = ditherwin.A[ 0 ] / 2. ; 
	    analysisDatatnow += randf( (-dwin),  dwin ) ; 
        }

	// GET TIME POINT CHANGE SMOOTH VALUES
	tsmooth.A[ 0 ] = fval( &tsmooth, dur, t );
	    smooth_setup( tsmooth.A[ 0 ], &tsmoothc, &minustsmoothc, IR ) ; 

	// SMOOTH THE TIME POINT
	analysisDatatnow = (tsmoothc * oldanalysisDatatnow ) + ( minustsmoothc * analysisDatatnow ); 


//******************************************************************************
// MAKE THE ANALYSIS DATA FRAME
//******************************************************


	makeInterpolatedFilterFrame ( 
	    &analysisData, 
	    F_lower, 
	    F_higher, channel,
	    iframes_per_sec, 
	    analysis_N + 2, 
	    analysisDatatnow, 
	    ainchan, 
	    analysis_chan
	) ; 

	// SAVE THE OLD TIME POINT
	oldanalysisDatatnow = analysisDatatnow ; 


    // NORMALIZE FOR LOOP
    // ************    
    normalizeLoopAmplitudes(
	LoopNormalizationFlag,
	Mode__sampler_loop_0__autostop_1,
	&analysisDatawinlow, 
	&analysisDatawinhi,
	&analysisData,
	F_lower, 
	F_higher,
	tempChannel,
	channel,
	iframes_per_sec,
	(analysis_N + 2),
	ainchan,
	analysis_chan,
	analysis_dur, 
	analysisDatatnow
    ) ; 


		
//******************************************************
//*************NORMALIZE THE INPUT SPECTRUM
//    normalize(  channel,  (analysis_N + 2),   normamppk  ) ;

//*************WARP THE INPUT SPECTRUM
    warpshape.A[ 0 ] =  fval( &warpshape, dur, t );
    spectmagwarp( channel,  (analysis_N + 2), warpshape.A[ 0 ], 0 ) ;

//*************************
// GET THE VALUES
//*************************


//  SHIFT, GAIN, AND TRANSPOSITION

    harmadd.A[ 0 ] =  fval( &harmadd, dur, t );
    dBgain.A[ 0 ] =  fval( &dBgain, dur, t );
        gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
    ptrans.A[ 0 ] = fval( &ptrans, dur, t );
        pm = semitones_to_mult( ptrans.A[ 0 ] ) ;

    //RELEASE
    release.A[ 0 ] = fval( &release, dur, t )  ;
        smooth_setup( release.A[ 0 ] + loopSmoothTime, &releasec, &minusreleasec, IR ) ; 
    attack.A[ 0 ] = fval( &attack, dur, t ) ;
        smooth_setup( attack.A[ 0 ] + loopSmoothTime, &attackc, &minusattackc, IR ) ; 

    // FRQ SMOOTH
    fsmooth.A[ 0 ] = fval( &fsmooth, dur, t ) ;
        smooth_setup( fsmooth.A[ 0 ] + loopSmoothTime, &fsmoothc, &minusfsmoothc, IR ) ; 

    // EQ VALUES
    dBlow.A[ 0 ] =  fval( &dBlow, dur, t );
    dBhi.A[ 0 ] =  fval( &dBhi, dur, t );
    freqlow.A[ 0 ] =  fval( &freqlow, dur, t );
    freqhi.A[ 0 ] =  fval( &freqhi, dur, t );



    // RANDOM AMP VARIATION
    randomampdB.A[ 0 ] = fval( &randomampdB, dur, t );
    randomampvarresponse.A[ 0 ] = fval( & randomampvarresponse, dur, t );
        smooth_setup( randomampvarresponse.A[ 0 ], 
    	    &ranampsmoothc, &minusranampsmoothc, IR ) ; 

    // RANDOM FREQ VARIATION
    randomfreqprop.A[ 0 ] = fval( &randomfreqprop, dur, t );
    randomfreqresponse.A[ 0 ] = fval( &randomfreqresponse, dur, t );
        smooth_setup( randomfreqresponse.A[ 0 ], 
    	    &ranfreqsmoothc, &minusranfreqsmoothc, IR ) ; 
    randomfreqcurve.A[ 0 ] = fval( &randomfreqcurve, dur, t );

                // RANDOM AMP AND FREQ VARIATION ROLLOFF
    high_randomvarfreq.A[ 0 ] = fval( & high_randomvarfreq, dur, t );
    low_randomvarfreq.A[ 0 ] = fval( & low_randomvarfreq, dur, t );
    ranfreqboundsdiff = high_randomvarfreq.A[ 0 ] - low_randomvarfreq.A[ 0 ] ; 
    randomvartransindex.A[ 0 ] = fval( & randomvartransindex, dur, t );


// ***********************

//*****************
// MODIFICATIONS LOOP
//*****************

    // SMOOTH THE CHANGES TO THE SPECTRUM
    smooth( channel, previous_channel, (N + 2), attackc, minusattackc, releasec, minusreleasec ) ; 

    for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){


        // FREQ SMOOTH
    	channel[i] =  (fsmoothc * previous_channel[i] )
            + ( minusfsmoothc * channel[i] ); 
        previous_channel[i] = channel[i] ; 
//        previous_channel[i - 1] = channel[i - 1] ; 

//******* 

// SHIFT BY -a AND TRANSPOSE BY -P

        temp = pm * (channel[i] + harmadd.A[ 0 ]) ;
        
        // ADD THIS MULTIPLIER TO THE STORE MULTIPLIERS
        channel_freqdev[ i ] *= pm ; 
        // ADD THIS ADDER TO THE STORE ADDERS
        channel_freqdev[ i - 1 ] += harmadd.A[ 0 ]  ; 
        
        // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
        if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
        else channel[i] = temp ; 


        // RANDOM AMPLITUDE VARIATION ******************

        if( ranampflag ){	
		    

	    if( channel[i] > high_randomvarfreq.A[ 0 ] ){
		// ABOVE: FULL SHIMMER
			    
		temp = (ranampsmoothc * previous_ranampv[i - 1] )
		     + ( minusranampsmoothc * randf( -( randomampdB.A[ 0 ] ), 
			randomampdB.A[ 0 ]  ) ); 
		channel[i - 1] *= dB_to_amp( temp ) ; 
		previous_ranampv[i - 1] = temp ; 
			    
	    }else if( channel[i] > low_randomvarfreq.A[ 0 ] ){
			    // TRANSITION RANGE: PARTIAL SHIMMER
    
		temp2 = (channel[i] - low_randomvarfreq.A[ 0 ]) / ranfreqboundsdiff ; 
		tempdB = curve( 0., randomampdB.A[ 0 ], temp2,  randomvartransindex.A[ 0 ] ) ; 
		temp = (ranampsmoothc * previous_ranampv[i - 1] )
			   + ( minusranampsmoothc * randf( tempdB,  -( tempdB ) ) ); 
		channel[i - 1] *= dB_to_amp( temp ) ; 
		previous_ranampv[i - 1] = temp ; 

	    }else{
		// NO SHIMMER
		previous_ranampv[i - 1] = 0. ; 
	    }
	}

		// RANDOM FREQ VARIATION
	if( ranfreqflag ){
	    if( channel[i] > high_randomvarfreq.A[ 0 ] ){
			    // ABOVE: FULL FREQ VARIATION
		temp = curve( 0., randomfreqprop.A[ 0 ],  randf( 0., 1. ),  randomfreqcurve.A[ 0 ] ) ; 
			    // SIGN
		temp2 = randf( -1.,  1. ) ; if( temp2 < 0. ) temp *= -1. ; 
		temp = (ranfreqsmoothc * previous_ranfreqv[i - 1] )
					+ ( minusranfreqsmoothc * temp ); 
		channel[i] *= (1. + temp) ; 
		previous_ranfreqv[i - 1] = temp ; 
			    
	    }else if( channel[i] > low_randomvarfreq.A[ 0 ] ){
			    // ROLLOFF RANGE: PARTIAL VARIATION
		temp = curve( 0., randomfreqprop.A[ 0 ],  randf( 0., 1. ),  randomfreqcurve.A[ 0 ] ) ; 
			    // SIGN
		temp2 = randf( -1.,  1. ) ; if( temp2 < 0. ) temp *= -1. ; 
                temp2 = (channel[i] - low_randomvarfreq.A[ 0 ]) / ranfreqboundsdiff ; 
                temp = curve( 0., temp, temp2,  randomvartransindex.A[ 0 ] ) ; 
		temp = (ranfreqsmoothc * previous_ranfreqv[i - 1] )
					+ ( minusranfreqsmoothc * temp ); 
		channel[i] *= (1. + temp) ; 
		previous_ranfreqv[i - 1] = temp ; 

	    }else{
			    // CUTOFF RANGE: NO VARIATION
		previous_ranfreqv[i - 1] = 0. ; 
	    }

	}

	channel[i - 1] = channel[i - 1] * gain ;  

    }

		
    //*************EQUALIZE THE OUTPUT SPECTRUM
    eq2( channel,  (N + 2),  dBlow.A[0],  dBhi.A[0], freqlow.A[0],  freqhi.A[0],  
                                    analysis_fundamental, channel_freqdev, 0 ) ; 
		

        synt = getthresh( channel, N, threshfac );


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

    fprintf(stderr,"\nTWARP : RESYNTHESIS COMPLETED\n");

    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
    if( sourcedB.n != 1. ) fclose(sourcedB.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( fsmooth.n != 1. ) fclose(fsmooth.fp ) ;
    if( analysisData.n != 1. ) fclose(analysisData.fp ) ;
    if( analysisDatarate.n != 1. ) fclose(analysisDatarate.fp ) ;
    if( analysisDatatorigin.n != 1. ) fclose(analysisDatatorigin.fp ) ;
    if( analysisDatawinlow.n != 1. ) fclose(analysisDatawinlow.fp ) ;
    if( analysisDatawinhi.n != 1. ) fclose(analysisDatawinhi.fp ) ;
    if( ditherwin.n != 1. ) fclose(ditherwin.fp ) ;
    if( tsmooth.n != 1. ) fclose(tsmooth.fp ) ;
    if( warpshape.n != 1. ) fclose(warpshape.fp ) ;
    if( randomampvarresponse.n != 1. ) fclose(randomampvarresponse.fp ) ;
    if( randomfreqprop.n != 1. ) fclose(randomfreqprop.fp ) ;
    if( randomfreqresponse.n != 1. ) fclose(randomfreqresponse.fp ) ;
    if( randomfreqcurve.n != 1. ) fclose(randomfreqcurve.fp ) ;
    if( dBlow.n != 1. ) fclose(dBlow.fp ) ;
    if( dBhi.n != 1. ) fclose(dBhi.fp ) ;
    if( freqlow.n != 1. ) fclose(freqlow.fp ) ;
    if( freqhi.n != 1. ) fclose(freqhi.fp ) ;



    exit(EXIT_SUCCESS) ;
}






void usage()
{
    fprintf(stderr, "%s",
	"twarp:  time-varying  phase vocoder resynthesis\n"
	"twarp   [flags]  [output file ]\n"
	"	    Pre-formatted output sound file required.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec
	"	d:	"OUTPUT_DURATION		// dur
	"	C:	"RESYNTHESIS_CHANNEL		// channelout

	"	P:	pitch transposition in semitones (func) [0]\n"
	"	a:	frequency shift factor (bin frequency adder, before -P )(func)[0.] \n"
	"	A:	gain in decibels (func) [0.] \n"

	"	    ANALYSIS DATA\n"
	"	F:	"PHASE_VOCODER_ANALYSIS_FILE

	"	    "DATA_TIME_HEADER
	"	Q:	"TIME_POINT_ORIGIN
	"	Y:	"RATE_MULTIPLIER

	"	T:	time point dither window size in seconds (0 (off) -> ? ) (func) [0.] \n"
	"	K:	time point change response time in seconds (func) [0.] \n"

	"	g:	"TIME_WINDOW_LOW_BOUNDARY
	"	G:	"TIME_WINDOW_HIGH_BOUNDARY

	"	S:	"TIME_WINDOW_MODE

	"	    "SAMPLER_HEADER
	"	o:	"LOOP_MODE

	"	r:	"ONSET_RELEASE_SWITCH
	"	b:	"LOOP_SMOOTH_TIME
	"	v:	"LOOP_NORMALIZATION_SWITCH

	"	    AMPLITUDE SHAPING\n"
	"	W:	"WARP_INDEX

	"	l:      "AMP_ATT_TIME
	"	L:      "AMP_RELEASE_TIME


	"	     RANDOM AMPLITUDE VARIATION (SHIMMER) \n"
	"	B:	decibels of variation (+ values) (func) [0.]\n"
	"	e:	random amplitude variation response time in seconds (func) [0.] \n"

	"	     RANDOM FREQUENCY VARIATION\n"
	"	j:	random frequency variation peak +/- change as proportion of \n"
	"		    frequency.  0 to ? (func) [0.] \n"
	"	k:	random frequency variation response time in seconds (func) [0.] \n"
	"	n:	random frequency variation distribution curve index (func) [0.] \n"
	"		    ( The index controls the function curve that maps the\n"
	"		      random value into the range. 0 = linear, values > 0\n"
	"		      are increasingly exponential, weighting the distribution\n"
	"		      increasingly toward the non-varied center. )\n"

	"	     RANDOM AMPLITUDE AND FREQUENCY VARIATION ROLLOFF\n"
	"	I:	variation rolloff frequency  (func) [22050.]\n"
	"	J:	variation cutoff frequency  (func) [0.]\n"
	"		    (rolloff should be above cutoff.)\n"
	"	N:	shape index for rolloff-to-cutoff frequencies (func) [0.]\n"
	"	 	    0 = linear rolloff,\n"
	"		    negative = gradual rolloff, positive = sudden rolloff\n"



	"	f:      frequency change response time   (func) [0.]\n"
	
	"	     "SHELF_EQ_HEADER
	"	H:	"SHELF_EQ_LOW_GAIN
	"	X:	"SHELF_EQ_HIGH_GAIN
	"	m:	"SHELF_EQ_LOW_FREQ
	"	R:	"SHELF_EQ_HIGH_FREQ


	"	t:	"RESYNTH_THRESHOLD		// threshfacdB
	"	p:	"AMP_REPORTS		// quiet 
	"	i:	"AMP_REPORTS_TIME_INTERVAL	// ampstatinc 

	"	_:	 "AUTO_PLAY
 

	"	=:	 "RESCALE_LEVEL
 


    ); // DONE
    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }