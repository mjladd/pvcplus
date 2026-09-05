#include "globals.h"


void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,  k,  jj,  ii ;
float nyquist,  fundamental,  freqdiff ;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0 ;
float P = 1.0;
FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel, *tempchannel,   *output ;
float *previous_channel,   *F,  *channel_freqdev,  *previous_change  ; 
float threshfac = .001,  threshfacdB=-96 ;
float channelAmpSum, tempChannelAmpSum, normalizationAmp, frameNormalizationAmpLimit ; 


float	pm, gain=1. ;
float  temp,  temp1,  temp2,  temp3,  temp4  ;  
float getthresh();
int lowb,  hib ; 

float peakbinamp = 0.,  avgbinamp=0., octaves=0. ; 


float   IR, DR,   dur=0.;
int print_flag=0 ; 

int lowcutbin, hicutbin,  lowbin,   hibin ; 
float octrollmult,  peakamp, oldpeakamp, *oldpeakamps,   normamp,    normampdB, minamp,   gradient ; 

char tempstring[ STRING_SIZE ] ; 

float envattack,  envrelease,  minusattack,  minusrelease  ;  
double ar_dB ; 

float compthreshamp,  compamp, expandthreshamp, expandthreshdB,   expandamp,  normal ; 


float c_response, minusc_response ; 

// FRAME NORMALIZATION DECIBEL LIMIT
struct  func  frameNormalizationDecibelLimit ; 

//  FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 


//  COMPRESSION DECIBELS THRESHOLD
struct  func  compthresh  ; 

//  DECIBELS OF COMPRESSION
struct  func  compdB  ; 

//  EXPANSION DECIBELS THRESHOLD
struct  func  expandthresh  ; 

//  DECIBELS OF EXPANSION
struct  func  expanddB  ; 

//  WARP CURVE INDEX
struct  func  warpcurve_index ; 


//  COMPANDER RESPONSE
struct  func  compander_response ; 


//  COMPLEMENT SPECTRUM  PROPORTION
struct  func  complementprop ; 



// SLIDING COMPRESSION WINDOW SIZE
struct  func  winsize ; 


//  RELEASE
struct  func  release ; 

//  ATTACK
struct  func  attack ; 

//  LOW COMPANDING CUT
struct  func  lowcut ; 

//  HIGH COMPANDING CUT
struct  func  hicut ; 

//  OCTAVES ROLLOFF
struct  func  octavesrolloff ; 

//SHELF EQ
struct  func  dBlow;
struct  func  dBhi;
struct  func  freqlow;
struct  func  freqhi ;


//*****************INITIALIZE

// FRAME NORMALIZATION DECIBEL LIMIT
frameNormalizationDecibelLimit.L = 1. ; 
	frameNormalizationDecibelLimit.n = 1. ; frameNormalizationDecibelLimit.A[ 0 ] = 0. ; 

//  FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH MULTIPLIER
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 


//  COMPRESSION DECIBELS THRESHOLD
compthresh.L = 1. ; compthresh.n = 1. ; compthresh.A[ 0 ] = 0. ; 

//  DECIBELS OF COMPRESSION
compdB.L = 1. ; compdB.n = 1. ; compdB.A[ 0 ] = 0. ; 

//  EXPANSION DECIBELS THRESHOLD
expandthresh.L = 1. ; expandthresh.n = 1. ; expandthresh.A[ 0 ] = -96. ; 

//  DECIBELS OF EXPANSION
expanddB.L = 1. ; expanddB.n = 1. ; expanddB.A[ 0 ] = 0. ; 



//  WARP CURVE INDEX
warpcurve_index.L = 1. ; warpcurve_index.n = 1. ; warpcurve_index.A[ 0 ] = 0. ; 

//  COMPANDER RESPONSE
compander_response.L = 1. ; compander_response.n = 1. ; compander_response.A[ 0 ] = 0. ; 

//  COMPLEMENT SPECTRUM  PROPORTION
complementprop.L = 1. ; complementprop.n = 1. ; complementprop.A[ 0 ] = 0. ; 




// SLIDING COMPRESSION WINDOW SIZE
winsize.L = 1. ; winsize.n = 1. ; winsize.A[ 0 ] = 0. ; 


//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  BIN AMP CHANGE ATTACK RESPONSE
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 

//  LOW COMPANDING CUT
lowcut.L = 1. ; lowcut.n = 1. ; lowcut.A[ 0 ] = 0. ; 

//  HIGH COMPANDING CUT
hicut.L = 1. ; hicut.n = 1. ; hicut.A[ 0 ] = -1. ; 

//  OCTAVES ROLLOFF
octavesrolloff.L = 1. ; octavesrolloff.n = 1. ; octavesrolloff.A[ 0 ] = 0. ; 

// SHELF EQ
dBlow.L = 1. ; dBlow.n = 1. ; dBlow.A[ 0 ] = 0. ; 
dBhi.L = 1. ; dBhi.n = 1. ; dBhi.A[ 0 ] = 0. ; 
freqlow.L = 1. ; freqlow.n = 1. ; freqlow.A[ 0 ] = 200. ; 
freqhi.L = 1. ; freqhi.n = 1. ; freqhi.A[ 0 ] = 2000. ; 


strcpy( routine, "spectwarper" ) ; 


if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, "n|R|w|N|M|r|P|g|D|W|c|S|d|f|o|O|Q|_|=|q|W|t|S|I|b|e|Z|p|i|H|m|d|X|s|F|a|A|C|c|L|l|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = (int) (int) crackfloat( arg_option, ch ); //  crackfloat( arg_option, ch )
			break;
	    case 'M':   Nw = (int) (int) crackfloat( arg_option, ch );
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'I':   tfactor = crackfloat( arg_option, ch );
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
	    case 't':	threshfacdB = crackfloat( arg_option, ch );
			break;


	    case 'c':   strcpy(tempstring, arg_option);
			lowcut.fp = crackstring( tempstring, 
			    &lowcut );
			break;

	    case 'd':   strcpy(tempstring, arg_option);
			hicut.fp = crackstring( tempstring, 
			    &hicut );
			break;
	    case 'f':   strcpy(tempstring, arg_option);
			octavesrolloff.fp = crackstring( tempstring, 
			    &octavesrolloff );
			break;

	    case 'n':   strcpy(tempstring, arg_option);
			frameNormalizationDecibelLimit.fp = crackstring( tempstring, 
			    &frameNormalizationDecibelLimit );
			break;

	


            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;
            case 'Z':	print_flag = (int) crackfloat( arg_option, ch ) ; break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
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

  
  
  
  
           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;



	    case 'o':   strcpy(tempstring, arg_option);
			compthresh.fp = crackstring( tempstring, 
			    &compthresh );
			break;
	    case 'O':   strcpy(tempstring, arg_option);
			compdB.fp = crackstring( tempstring, 
			    &compdB );
			break;

	    case 'q':   strcpy(tempstring, arg_option);
			expandthresh.fp = crackstring( tempstring, 
			    &expandthresh );
			break;
	    case 'Q':   strcpy(tempstring, arg_option);
			expanddB.fp = crackstring( tempstring, 
			    &expanddB );
			break;

	    case 'W':   strcpy(tempstring, arg_option);
			warpcurve_index.fp = crackstring( tempstring, 
			    &warpcurve_index );
			break;

	    case 'r':   strcpy(tempstring, arg_option);
			compander_response.fp = crackstring( tempstring, 
			    &compander_response );
			break;


	    case 'g':   strcpy(tempstring, arg_option);
			complementprop.fp = crackstring( tempstring, 
			    &complementprop );
			break;



	    case 'S':   strcpy(tempstring, arg_option);
			winsize.fp = crackstring( tempstring, 
			    &winsize );
			break;


	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;
	    case 's':	sflag = 1;
			break;
	}
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "COMPANDER",  69 ) ; 
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
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = ((float) R) / 2.0 ;
    fundamental =  ((float) R / (float) N) ; 
    freqdiff = (float) R / (float) N ;
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
// REVERB TIME STUFF
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );
    minamp = dB_to_amp( -96. ) ; 	
    DR = (float) D / (float) R ; 
    IR = (float) I / (float) R ;

    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 

// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( 
	(ptrans.n  != 1.) || (harmadd.n  != 1.) ||
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.) ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
    


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
prp( &dBgain,  "GAIN (in dB)"  ) ; 
prp( &ptrans,  "PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &harmadd,  "FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 

prp( &compthresh,  "COMPRESSION THRESHOLD (in dB)"  ) ; 
prp( &compdB,  "DECIBELS OF COMPRESSION"  ) ; 
prp( &expandthresh,  "EXPANSION THRESHOLD (in dB)"  ) ; 
prp( &expanddB,  "DECIBELS OF EXPANSION"  ) ; 
prp( &warpcurve_index,  "WARP CURVE INDEX (0-?)" ) ; 
prp( &compander_response,  "COMPANDER RESPONSE TIME (in secs)" ) ; 

prt( "..........COMPANDING FREQUENCY BAND...........*" ) ; 
prp( &lowcut,  "LOW CUTOFF FREQUENCY" ) ; 
prp( &hicut,  "HIGH CUTOFF FREQUENCY" ) ; 
prp( &octavesrolloff,  "OCTAVES ROLLOFF" ) ; 

prp( &winsize,  "SLIDING COMPRESSION WINDOW SIZE (in Hz)" ) ; 
prp( &complementprop,  "COMPLEMENT AMPLITUDE SPECTRUM PROPORTION" ) ; 
prp( & frameNormalizationDecibelLimit, "FRAME NORMALIZATION DECIBEL LIMIT" ) ; 

prp( &attack,  "COMPANDING PEAK-FOLLOWING, ATTACK RESPONSE TIME (in seconds)"  ) ; 
prp( &release, "COMPANDING PEAK-FOLLOWING, RELEASE RESPONSE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prp( &freqlow, "LOW SHELF FREQUENCY" ) ; 
prp( &dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prp( &freqhi, "HIGH SHELF FREQUENCY" ) ; 
prp( &dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 

   // *******

// SET UP ARRAYS

    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( tempchannel, N+2 ) ;	/* temporary holder of channel data (for inversion) */

    fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

    fvec( channel_freqdev,  N + 2 ) ;	// channel SORT ARRAY ACCUMULATOR

    fvec( previous_change, N+2 ) ;	/* previous amp multiplier */

    fvec( oldpeakamps, N+2 ) ;	/* old peak amps */

// MAKE THRESH AMP
    threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	


    fvec( output, Nw ) ;	/* output buffer */


// MAKE THRESH AMP
    threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	
    

// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 


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

		// SETUP PREVIOUS CHANNEL
		if( !frame_count )for(i = 0; i < (N + 2); i++)
		    previous_channel[ i ] = channel[ i ] ; 

		// SETUP PREVIOUS CHANGE
		if( !frame_count )for(i = 0; i < (N + 2); i++)
		    previous_change[ i ] = 1. ; 

		// INIT OLD PEAK
		if( !frame_count ) oldpeakamp = 0. ; 

		// SETUP OLD PEAK AMPS
		if( !frame_count )for(i = 0; i < (N + 2); i++)
		    oldpeakamps[ i ] = 0. ; 



		// SETUP FREQDEV HISTORY ARRAY
	    	for( i = 1; i < (N + 2); i+= 2 ) {
			channel_freqdev[ i - 1 ] = 0. ; 
			channel_freqdev[ i ] = 1. ; 
		}


//*************************
// GET THE VALUES
//*************************


//  SHIFT, GAIN, AND TRANSPOSITION

		harmadd.A[ 0 ] =  fval( &harmadd, dur, t );
		dBgain.A[ 0 ] = fval( &dBgain, dur, t );
		    gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
		ptrans.A[ 0 ] = fval( &ptrans, dur, t ) ;
		pm = semitones_to_mult( ptrans.A[ 0 ] ) ;

		release.A[ 0 ] = fval( &release, dur, t );
		    smooth_setup( release.A[ 0 ], &envrelease, &minusrelease, IR ) ; 
		attack.A[ 0 ] = fval( &attack, dur, t );
		    smooth_setup( attack.A[ 0 ], &envattack, &minusattack, IR ) ; 

	    // COMPRESSION
		compthresh.A[ 0 ] =  fval( &compthresh, dur, t );
		    compthreshamp = dB_to_amp( compthresh.A[ 0 ] ) ; 
		compdB.A[ 0 ] = fval( &compdB, dur, t );
		    compamp = dB_to_amp( compdB.A[ 0 ] ) ; 
		    normal = .25 / (compthreshamp + (compamp * (1. - compthreshamp))) ; 


	    // EXPANSION
		expandthreshdB = expandthresh.A[ 0 ] =  fval( &expandthresh, dur, t );
		    if( expandthreshdB < -95. ) expandthreshdB = -95. ; 
		    expandthreshamp = dB_to_amp( expandthreshdB ) ; 
		expanddB.A[ 0 ] = fval( &expanddB, dur, t );
		    expandamp = 1. / dB_to_amp( expanddB.A[ 0 ] ) ; 

	    // WARP CURVE INDEX
		warpcurve_index.A[ 0 ] = fval( &warpcurve_index, dur, t );

	    // COMPANDER RESPONSE TIME
		compander_response.A[ 0 ] = fval( &compander_response, dur, t );
		    smooth_setup( compander_response.A[ 0 ], 
			    &c_response, &minusc_response, IR ) ; 


	    // COMPLEMENT SPECTRUM PROPORTION
		complementprop.A[ 0 ] = fval( &complementprop, dur, t );

	    // FRAME NORMALIZATION DECIBEL LIMIT
		frameNormalizationDecibelLimit.A[ 0 ] = 
			fval( & frameNormalizationDecibelLimit, dur, t );
		frameNormalizationAmpLimit = dB_to_amp( frameNormalizationDecibelLimit.A[ 0 ] ); 




	    // FREQ BAND
		lowcut.A[ 0 ] =  fval( &lowcut, dur, t );
		hicut.A[ 0 ] =  fval( &hicut, dur, t );
		octavesrolloff.A[ 0 ] =  fval( &octavesrolloff, dur, t );
		if( octavesrolloff.A[ 0 ] < 0. ){
		    prf( octavesrolloff.A[ 0 ],  "ILLEGAL OCTAVES ROLLOFF" ) ; 
		    prt( "MUST BE >= 0. \n\n\n" ) ; exit(EXIT_FAILURE) ; 
		}
		octrollmult = powf( 2., octavesrolloff.A[ 0 ] ) ; 

		temp1 = lowcut.A[ 0 ] ; 
		if (temp1 < 0. ) temp1 = 0. ; 
		temp2 = hicut.A[ 0 ] ; 
		if ((temp2 < 0.) || (temp2 > nyquist) ) temp2 = nyquist ; 


		lowcutbin =  ((int) (temp1 / freqdiff ) * 2) + 1 ; 
		hicutbin =  ((int) (temp2 / freqdiff ) * 2) + 1 ; 
		lowbin =  ((int) ((temp1 * ( 1. / octrollmult )) / freqdiff) * 2) + 1 ; 		
		hibin =  (((int) (temp2 * octrollmult ) / freqdiff) * 2) + 1 ; 		
		if( hibin > N ) hibin = N ; 
		if( lowbin < 0 ) lowbin = 1 ; 


	    // BANDWIDTH FOR PEAK
		winsize.A[ 0 ] = fval( &winsize, dur, t );


//*************************

//*****************
// MODIFICATIONS LOOP
//*****************


	// IF WINDOW IS OFF, THEN FIND THE PEAK AMP FOR FRAME COMPRESSION
	if( winsize.A[ 0 ] <= 0. ){
	    peakamp = -99999999. ; 

	    for( i = lowbin; i < hibin; i += 2){
		if( channel[i - 1] > peakamp ) peakamp = channel[i - 1] ; 
	    }
	    peakamp = 
		smooth_one_value( peakamp, oldpeakamp,  
		    envattack, minusattack, envrelease, minusrelease ) ; 
	    oldpeakamp = peakamp ; 



	}

	// SET UP TEMP OF CHANNEL
	  for( i = 0; i < (N + 2); i++ ) tempchannel[ i ] = channel[ i ] ; 
	  

	
	// 
	  for( i = lowbin; i < hibin; i+= 2 ){

	    if(winsize.A[ 0 ] > 0.){

 		// FIND THE PEAK AMP FOR THIS BIN'S WINDOW
		if( winsize.A[ 0 ] <= 8 ){		
			temp1 = (float) ((i - 1) / 2) * fundamental ; // FREQ OF THIS BIN
			temp2 =  powf( 2., winsize.A[ 0 ] * .5 ) ; // MULTIPLIER
			temp3 = temp1 * temp2 ; // UPPER FREQ 
			temp4 = temp1 / temp2 ; // LOWER FREQ 
			hib = (2 * (int) (( temp3 / fundamental  ) + .5 )) ; // UPPER BIN
			lowb = (2 * (int) (( temp4 / fundamental  ) + .5 )) ; // LOWER BIN

			if( lowb < 1 ) lowb = 0 ; 
			if( hib < N ) hib = N - 1 ; 
		} else {
			temp1 = (float) ((i - 1) / 2) * fundamental ; // FREQ OF THIS BIN
			temp2 =  winsize.A[ 0 ] * .5 ; // MULTIPLIER
			temp3 = temp1 + temp2 ; // UPPER FREQ 
			temp4 = temp1 - temp2 ; // LOWER FREQ 
			hib = (2 * (int) (( temp3 / fundamental  ) + .5 )) ; // UPPER BIN
			lowb = (2 * (int) (( temp4 / fundamental  ) + .5 )) ; // LOWER BIN

			if( lowb < 1 ) lowb = 0 ; 
			if( hib < N ) hib = N - 1 ;
	        } ; 




		
		peakamp = -99999999. ; 
		for( ii = lowb; ii < hib; ii+= 2 ){
		    if( channel[ii] > peakamp ) peakamp = channel[ii] ; 
		}
		peakamp = 
		    smooth_one_value( peakamp, oldpeakamps[i],  
			envattack, minusattack, envrelease, minusrelease ) ; 
		oldpeakamps[i] = peakamp ; 

		


	    }




	     if( peakamp > 0. ){

		    // MODIFY  BIN AMPLITUDES 
		    // SKIP OUT IF 0
		if( channel[i - 1] > 0. ){
			
		    // MAKE NORMALIZED AMP
		    normamp = channel[i - 1] / peakamp ;
		    // MAKE DB FORM
		    normampdB =  (float) 20. * log10( (double) normamp ) ;
		     
		 if( normampdB >= -96. ){
		    if( normampdB > compthresh.A[ 0 ] ){
			// COMPRESS
			if(compdB.A[ 0 ] < 0. ){
			    // MAKE GRADIENT PROPORTION
			    gradient = 
				( normampdB - compthresh.A[ 0 ] ) / fabs( compthresh.A[ 0 ] ) ; 
			    // SHAPE GRADIENT VALUE
			     gradient =
				curve( 0.,  1., gradient, warpcurve_index.A[ 0 ] ) ;  

			    // MAKE  AMP MULTIPLIER
			    temp2 = dB_to_amp( compdB.A[ 0 ] * gradient )  ; 

			    // SMOOTH TRANSITION INTO IT
			    temp2 = 
				(minusc_response * temp2) + 
				    (c_response * previous_change[i - 1]) ; 

			    // SAVE OLD CHANGE
			    previous_change[i - 1] = temp2 ; 

			    // MAKE  THE MODIFIED AMP 
			    temp2 = channel[i - 1] * temp2  ; 


			}else{

			    // MAKE  AMP MULTIPLIER
			    temp2 = 1. ; 

			    // SMOOTH TRANSITION INTO IT
			    temp2 = 
				(minusc_response * temp2) + 
				    (c_response * previous_change[i - 1]) ; 

			    // SAVE OLD CHANGE
			    previous_change[i - 1] = temp2 ; 

			    // MAKE  THE MODIFIED AMP 
			    temp2 = channel[i - 1] * temp2  ; 
			
			}
		    }else if( normampdB < expandthreshdB ){
		    
			// EXPAND
			if( expanddB.A[ 0 ] < 0. ){
			    // MAKE GRADIENT PROPORTION
			    gradient = 
				( normampdB - expandthreshdB ) / ( -96. - expandthreshdB ) ; 
			    // SHAPE GRADIENT VALUE
			     gradient =
				curve( 0.,  1., gradient, warpcurve_index.A[ 0 ] ) ;  

			    // MAKE  AMP MULTIPLIER
			    temp2 = dB_to_amp( expanddB.A[ 0 ] * gradient )  ; 

			    // SMOOTH TRANSITION INTO IT
			    temp2 = 
				(minusc_response * temp2) + 
				    (c_response * previous_change[i - 1]) ; 

			    // SAVE OLD CHANGE
			    previous_change[i - 1] = temp2 ; 

			    // MAKE MODIFIED AMP
			    temp2 = channel[i - 1] * temp2  ; 
			    if( temp2 < 0.) temp2 = 0. ; 


			}else{
			    // MAKE  AMP MULTIPLIER
			    temp2 = 1. ; 

			    // SMOOTH TRANSITION INTO IT
			    temp2 = 
				(minusc_response * temp2) + 
				    (c_response * previous_change[i - 1]) ; 

			    // SAVE OLD CHANGE
			    previous_change[i - 1] = temp2 ; 

			    // MAKE  THE MODIFIED AMP 
			    temp2 = channel[i - 1] * temp2  ; 

			}

		    }else{
			    // MAKE  AMP MULTIPLIER
			    temp2 = 1. ; 

			    // SMOOTH TRANSITION INTO IT
			    temp2 = 
				(minusc_response * temp2) + 
				    (c_response * previous_change[i - 1]) ; 

			    // SAVE OLD CHANGE
			    previous_change[i - 1] = temp2 ; 

			    // MAKE  THE MODIFIED AMP 
			    temp2 = channel[i - 1] * temp2  ; 
			
		    }
		

// SELECTED BAND		    
		    if( i < lowcutbin ){
			// LOW ROLLOFF
			tempchannel[i - 1] = channel[i - 1] +
			     ((temp2 - channel[i - 1]) *
			     ((float) (i - lowbin) / (float) (lowcutbin - lowbin))) ; 
		    }else if( i > hicutbin ){
			// HI ROLLOFF
			tempchannel[i - 1] = channel[i - 1] +
			     ((temp2 - channel[i - 1]) *
			     ((float) (i - hibin) / (float) (hicutbin - hibin))) ; 
		    }else{
			// MIDDLE
			tempchannel[i - 1] = temp2 ; 
		    }


		  }

		}

	   }

	} ; 


	// MAKE SOURCE/COMPLEMENT MIX.
	for( i = 0; i < (N + 2); i+= 2 )
	    tempchannel[i] = tempchannel[i] + 
		(complementprop.A[ 0 ] * (channel[i] - (2. * tempchannel[i]))) ; 


	//  NORMALIZE AND REPLACE INTO CHANNEL.
		// FIND NORMALIZATION FACTOR.
	channelAmpSum = 0. ; tempChannelAmpSum = 0. ; 	
	for( i = 0; i < (N + 2); i+= 2 ){
		channelAmpSum += channel[i] ; 
		tempChannelAmpSum += tempchannel[i] ;
	} ; 
		// NORMALIZE AND TRANSFER
	if( (tempChannelAmpSum > 0.0) && (frameNormalizationAmpLimit != 1.0 ) ){
		normalizationAmp = channelAmpSum / tempChannelAmpSum ;
		if( normalizationAmp > frameNormalizationAmpLimit )
			normalizationAmp = frameNormalizationAmpLimit ; 
		for( i = 0; i < (N + 2); i+= 2 )
			channel[i] = tempchannel[i] * normalizationAmp ; 	
	}else{
		for( i = 0; i < (N + 2); i+= 2 ) channel[i] = tempchannel[i] ; 	
	} ; 

	for( i = 1; i < (N + 2); i+= 2 ){


		// ADD THIS MULTIPLIER TO THE STORE MULTIPLIERS
		    channel_freqdev[ i ] *= pm ; 
		// ADD THIS ADDER TO THE STORE ADDERS
		    channel_freqdev[ i - 1 ] += harmadd.A[ 0 ]  ; 

		    temp = pm * (channel[i] + harmadd.A[ 0 ]) ;
		    
		    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		    if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
			else channel[i] = temp ; 
	    
		    channel[i - 1] = channel[i - 1] * gain ;  

	}
		


//*************EQUALIZE THE OUTPUT SPECTRUM
		dBlow.A[ 0 ] =  fval( &dBlow, dur, t );
		dBhi.A[ 0 ] =  fval( &dBhi, dur, t );
		freqlow.A[ 0 ] =  fval( &freqlow, dur, t );
		freqhi.A[ 0 ] =  fval( &freqhi, dur, t );

    //*************EQUALIZE THE OUTPUT SPECTRUM
    eq2( channel,  (N + 2),  dBlow.A[0],  dBhi.A[0], freqlow.A[0],  freqhi.A[0],  fundamental, channel_freqdev, 0 ) ; 


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

    // CLOSE  INPUT FILE
    if(ifd)fclose(ifd);  



    fprintf(stderr,"\nSPECTWARPER : RESYNTHESIS COMPLETED\n");

 
    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;


   if( compthresh.n != 1. ) fclose(compthresh.fp ) ;
    if( compdB.n != 1. ) fclose(compdB.fp ) ;
    if( expandthresh.n != 1. ) fclose(expandthresh.fp ) ;
    if( expanddB.n != 1. ) fclose(expanddB.fp ) ;

    exit(EXIT_SUCCESS) ;
}

void usage()
{
    fprintf(stderr, "%s",
	"spectwarper:  spectral amplitude compressor/expander\n"
	"spectwarper   [flags] [input file] [output file (optional)]\n"
	"	    (values in brackets denote defaults)\n"
	"	N:	"FFT_LENGTH 		// N
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec
	"	I:	"TIME_FACTOR		// tfactor

	"	P:	"PITCH_TRANS		// ptrans
	"	a:	"FREQ_SHIFT		// harmadd
	"	A:	gain in decibels (func) [0.] \n"

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout

	"	o:	COMPRESSION: threshold in dB (0 to -96)(func) [-0.]\n"
	"	O:	decibels of compression  (0 to -96)(func) [-0.(off)]\n"
	"	q:	EXPANSION: threshold in dB (0 to -96)(func) [-96.]\n"
	"	Q:	decibels of expansion (0 to -96) (func) [-0.(off)]\n"
	"	W:	WARP CURVE INDEX: (0 to +?) (func) [0.]\n"
	"		    0 = linear,  + values make smoother transition\n"
	"	l:	COMPANDER peak following response time: attack (0 to +?) (func) [0.]\n"
	"	L:	COMPANDER peak following response time: release (0 to +?) (func) [0.]\n"
	"	r:	COMPANDER RESPONSE TIME: in seconds (0 to +?) (func) [0.]\n"
	"		    Positive values smooth the shift into the regions of \n"
	"		    compression and expansion, negative values sharpen it.\n"
	"		COMPANDING FREQUENCY BAND:\n"
	"		    ( Companding is applied only to the band relative to its peak.)\n"
	"	c:	low cutoff frequency (func) [0]\n"
	"	d:	high cutoff frequency (func) [Nyquist]\n"
	"	f:	octaves of rolloff (func) [0.]\n"

	"	S:	COMPRESSION WINDOW SIZE in octaves 0. = off (func) [0.]\n"
	"		    (This is the octave bandwidth over which the peak value\n"
	"		      is obtained from which compression is applied. \n"
	"		       0. uses one peak for all.)\n"
	"	g:	COMPLEMENT AMPLITUDE SPECTRUM proportion (0-1) (func) [0.]\n"
	"		    (Value of 1 produces the difference of the old and new bin amp.\n"
	"		      0 = no inversion. The sum of the old and new equals the original.)\n"

	"	    "FRAME_NORMALIZATION
	"	n:	"FRAME_NORMALIZATION_DB_LIMIT


	"	_:	 "AUTO_PLAY		// autoplayreps

	"	=:	 "RESCALE_LEVEL		// rescalev

	"	     "SHELF_EQ_HEADER
	"	H:	"SHELF_EQ_LOW_GAIN		// dBlow
	"	X:	"SHELF_EQ_HIGH_GAIN		// dBhi
	"	m:	"SHELF_EQ_LOW_FREQ		// freqlow
	"	R:	"SHELF_EQ_HIGH_FREQ		// freqhi


	"	t:	"RESYNTH_THRESHOLD		// threshfacdB
	"	p:	"AMP_REPORTS		// quiet 
	"	i:	"AMP_REPORTS_TIME_INTERVAL	// ampstatinc 

	"	Z:	print flag [0]\n"



	);
    exit(EXIT_SUCCESS);
}


void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
