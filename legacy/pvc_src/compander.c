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
int   eof = 0, obank = 0,  channelout=0 ;
float P = 1.0;
FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float *previous_channel,   *F,  *channel_freqdev,  *previous_change  ; 
float threshfac = .001,  threshfacdB=-96 ;

float channelAmpSum, tempChannelAmpSum, previousTempChannelAmpSum, 
	normalizationAmp, frameNormalizationAmpLimit ; 
float normenv, minusnormenv ; 

float	pm, gain=1. ;
float  temp,  temp1,  temp2,  temp3 ;  
float getthresh();

float peakbinamp = 0.,  avgbinamp=0., smoothingBW=0. ; 

float   IR, DR,   dur=0.;
int print_flag=0 ; 

int lowcutbin, hicutbin,  lowbin,   hibin ; 
float octrollmult ; 

char tempstring[ STRING_SIZE ] ; 

float envattack,  envrelease,  minusattack,  minusrelease  ;  
double ar_dB ; 

float compthreshamp,  compthreshampInverse, compamp, expandthreshamp,   expandamp,  normal ; 

float ampchange,  newamp,  normamp ; 

/*
// FRAME NORMALIZATION DECIBEL LIMIT
struct  func  frameNormalizationDecibelLimit ; 

// FRAME NORMALIZATION ENVELOPE RESPONSE TIME
struct func normEnvResponseTime ; 
*/

//  FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 

// FREQUENCY RESPONSE PEAKS
struct  func  peaks ; 

//  COMPRESSION DECIBELS THRESHOLD
struct  func  compthresh  ; 

//  DECIBELS OF COMPRESSION
struct  func  compdB  ; 

//  EXPANSION DECIBELS THRESHOLD
struct  func  expandthresh  ; 

//  DECIBELS OF EXPANSION
struct  func  expanddB  ; 


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
/*
// FRAME NORMALIZATION DECIBEL LIMIT
frameNormalizationDecibelLimit.L = 1. ; 
	frameNormalizationDecibelLimit.n = 1. ; frameNormalizationDecibelLimit.A[ 0 ] = 0. ; 


// FRAME NORMALIZATION ENVELOPE RESPONSE TIME
normEnvResponseTime.L = 1. ; normEnvResponseTime.n = 1. ; normEnvResponseTime.A[ 0 ] = 0. ; 
*/

//  FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH MULTIPLIER
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 

// FILTER
peaks.L = 1. ; peaks.n = 0. ; peaks.A[ 0 ] = 0. ; 

//  COMPRESSION DECIBELS THRESHOLD
compthresh.L = 1. ; compthresh.n = 1. ; compthresh.A[ 0 ] = 0. ; 

//  DECIBELS OF COMPRESSION
compdB.L = 1. ; compdB.n = 1. ; compdB.A[ 0 ] = 0. ; 

//  EXPANSION DECIBELS THRESHOLD
expandthresh.L = 1. ; expandthresh.n = 1. ; expandthresh.A[ 0 ] = -96. ; 

//  DECIBELS OF EXPANSION
expanddB.L = 1. ; expanddB.n = 1. ; expanddB.A[ 0 ] = 0. ; 

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

strcpy( routine, "compander" ) ; 



if( argc < 2 )usage() ; 


//CASE -> USAGE//h -> ? s -> ? 

    while( (ch= crack( argc, argv, "|R|w|N|M|P|D|c|d|f|o|O|Q|_|=|q|W|t|S|I|b|e|Z|p|i|H|m|d|X|s|F|a|A|C|c|L|l|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = (int) (int) crackfloat(arg_option, ch);
			break;
	    case 'M':   Nw = (int) (int) crackfloat(arg_option, ch);
			break;
	    case 'w':   window_type = (int) crackfloat(arg_option, ch) ;
			break;
	    case 'D':   frames_per_sec = crackfloat(arg_option, ch);
			break;
	    case 'I':   tfactor = crackfloat(arg_option, ch);
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
	    case 't':	threshfacdB = crackfloat(arg_option, ch);
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


            case 'p':	quiet = (int) crackfloat(arg_option, ch) ; break;
            case 'i':	ampstatinc = crackfloat(arg_option, ch) ; break;
            case 'Z':	print_flag = (int) crackfloat(arg_option, ch) ; break;

	    case 'b':   begint = crackfloat(arg_option, ch) ;
			break;
	    case 'e':   endt = crackfloat(arg_option, ch) ;
			break;

	    case 'C':   channelout = (int) crackfloat(arg_option, ch) ;
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


  
  
  
           case '_':	autoplayreps = (int) crackfloat(arg_option, ch) ; break;

           case '=':	rescalev = crackfloat(arg_option, ch) ; break;



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



	    case 'F':   strcpy(tempstring, arg_option);
			peaks.fp = crackstring( tempstring, 
			    &peaks );
			break;
	    case 'S':	smoothingBW = crackfloat(arg_option, ch);
			break;

	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
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
    

if( peaks.n < 1. ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A PEAKS FILE. BYE.\n" ) ;
    exit(EXIT_FAILURE);
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


prt( "..........COMPANDING FREQUENCY BAND...........*" ) ; 
prp( &lowcut,  "LOW CUTOFF FREQUENCY" ) ; 
prp( &hicut,  "HIGH CUTOFF FREQUENCY" ) ; 
prp( &octavesrolloff,  "OCTAVES ROLLOFF" ) ; 

//prp( &frameNormalizationDecibelLimit, "FRAME NORMALIZATION DECIBEL LIMIT" ) ; 
//prp( &normEnvResponseTime, "NORMALIZATION ENVELOPE RESPONSE TIME" ) ; 

prp( &attack,  "ENVELOPE ATTACK TIME (in seconds)"  ) ; 
prp( &release,  "ENVELOPE RELEASE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prp( &freqlow, "LOW SHELF FREQUENCY" ) ; 
prp( &dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prp( &freqhi, "HIGH SHELF FREQUENCY" ) ; 
prp( &dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
prf( smoothingBW, "RANGE OF PEAKS SMOOTHING (in frequency or octaves)" ) ; 

   // *******

// SET UP ARRAYS

    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

    fvec( channel_freqdev,  N + 2 ) ;	// channel SORT ARRAY ACCUMULATOR

    fvec( previous_change, N+2 ) ;	/* previous amp multiplier */


// MAKE THRESH AMP
    threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	


    fvec( output, Nw ) ;	/* output buffer */

// ALLOCATE FILTER SPACE
    fvec( F,  N+2  ) ;	/* peak response array */

//************FIRST TIME: FILL ARRAY FROM FILE
    fillfunc( &peaks, F, (N + 2)  ) ;     

//*********** SMOOTH THE SPECTRUM
    smoothspec( F, (N2 + 1), smoothingBW, R ) ; 



//*********** PRINT TO TERMINAL IF DESIRED ******
    if(print_flag)tprintspec( F, (N + 2), fundamental, print_flag) ;


// FIND THE PEAK AMP AND CREATE THRESHOLDS NORMALIZATION VALUE
normal = -99999999 ; 
for(i=1; i < (N + 2); i += 2 ){
    if( F[i - 1] > normal ) normal = F[i - 1] ; 
}

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
		    compthreshampInverse = 0.25 / compthreshamp ; 

		compdB.A[ 0 ] = fval( &compdB, dur, t );
		    compamp = dB_to_amp( compdB.A[ 0 ] ) ; 
		    normal = .25 / (compthreshamp + (compamp * (1. - compthreshamp))) ; 


	    // EXPANSION
		expandthresh.A[ 0 ] =  fval( &expandthresh, dur, t );
		    expandthreshamp = dB_to_amp( expandthresh.A[ 0 ] ) ; 
		expanddB.A[ 0 ] = fval( &expanddB, dur, t );
		    expandamp = 1. / dB_to_amp( expanddB.A[ 0 ] ) ; 

	    // FREQ BAND
		lowcut.A[ 0 ] =  fval( &lowcut, dur, t );
		hicut.A[ 0 ] =  fval( &hicut, dur, t );
		octavesrolloff.A[ 0 ] =  fval( &octavesrolloff, dur, t );
		if( octavesrolloff.A[ 0 ] < 0. ){
		    prf( octavesrolloff.A[ 0 ],  "ILLEGAL OCTAVES ROLLOFF" ) ; 
		    prt( "MUST BE >= 0. \n\n\n" ) ; exit(EXIT_FAILURE) ; 
		}
		octrollmult = powf( 2., octavesrolloff.A[ 0 ] ) ; 

		temp1 = lowcut.A[ 0 ] ; // LOW CUT FREQ
		if (temp1 < 0. ) temp1 = 0. ; 
		temp2 = hicut.A[ 0 ] ; // HI CUT FREQ
		if ((temp2 < 0.) || (temp2 > nyquist) ) temp2 = nyquist ; 

		lowcutbin =  ((int) (temp1 / freqdiff ) * 2) + 1 ; 
		hicutbin =  ((int) (temp2 / freqdiff ) * 2) + 1 ; 
		lowbin =  ((int) ((temp1 * ( 1. / octrollmult )) / freqdiff) * 2) + 1 ; 		
		hibin =  (((int) (temp2 * octrollmult ) / freqdiff) * 2) + 1 ; 		
		if( hibin > N ) hibin = N ; 

/*
	    // FRAME NORMALIZATION DECIBEL LIMIT
		frameNormalizationDecibelLimit.A[ 0 ] = 
			fval( & frameNormalizationDecibelLimit, dur, t );
		frameNormalizationAmpLimit = dB_to_amp( frameNormalizationDecibelLimit.A[ 0 ] ); 

		// NORMALIZATION ENVELOPE RESPONSE TIME
		normEnvResponseTime.A[ 0 ] = fval( & normEnvResponseTime, dur, t );
		    smooth_setup( normEnvResponseTime.A[ 0 ], &normenv, &minusnormenv, IR ) ; 
*/


//*************************

//*****************
// MODIFICATIONS LOOP
//*****************
	// FIND CHANNEL AMP SUM
//	channelAmpSum = 0.0 ; 
//	for( i = 0; i < (N + 2); i+= 2 ){
//		channelAmpSum += channel[i] ; 
//	} ; 

	for( i = lowbin, j = 0; i < hibin; i+= 2, j++ ){

		    // MODIFY  BIN AMPLITUDES 
		if((channel[i - 1] > 0.) && (F[i - 1] > 0.)){

			// CREATE NORMALIZED FORM OF THIS BIN's AMP TO THE PEAK IN F[]
			normamp = (channel[i - 1] / F[i - 1]) ; 
			
			//CREATE THE ampchange MULTIPLIER
		  if( normamp > 0. ){
			// NON-ZERO AMP
			
		    if( normamp > compthreshamp){
			// COMPRESS
			if(compamp < 1.){
				// ATTENUATING COMPRESSION AMP, OK
			    // MAKE THE AMP MULTIPLIER
			    ampchange =  (compthreshamp +  
				((normamp - compthreshamp) * compamp)) 
				/ normamp  ; 



			}else{
			    // AMPLIFYING COMPRESSION AMP, BAD! 
			    ampchange = 1. ; 
			}
		    }else if( normamp < expandthreshamp){
		    
			// EXPAND
			if( expandamp > 1.){
				// PROPER EXPANSION AMP
				
			    ampchange = (expandthreshamp -  
				((expandthreshamp - normamp)  * expandamp )) 
				/ normamp  ; 
			    if( ampchange < 0.) ampchange = 0. ;


			}else{
			    // IMPROPER EXPANSION AMP, NO CHANGE
			    ampchange = 1. ;
			} 
		    }else{
			// NO COMPRESSION OR EXPANSION
			ampchange = 1. ; 
		    }
		  }else{
			// ZERO AMP
			ampchange = 1. ;
		  }
		  

		// SMOOTH TRANSITION INTO ampchange MULTIPLIER
		if( ampchange > previous_change[i - 1] ){

		    // ATTACK
		    ampchange = (minusattack * ampchange) + 
			    (envattack * previous_change[i - 1]) ; 

		}else{

		    // DECAY
		    ampchange = (minusattack * ampchange) + 
			    (envattack * previous_change[i - 1]) ; 
			   
		}



		// SAVE SMOOTHED AMP CHANGE AS PREVIOUS CHANGE
		previous_change[i - 1] = ampchange ; 

		// MAKE MODIFIED AMP
		newamp = channel[i - 1] * ampchange  ; 


		  
		    if( i < lowcutbin ){
			// LOW ROLLOFF
			channel[i - 1] = channel[i - 1] +
			     ((newamp - channel[i - 1]) *
			     ((float) (i - lowbin) / (float) (lowcutbin - lowbin))) ; 
		    }else if( i > hicutbin ){
			// HI ROLLOFF
			channel[i - 1] = channel[i - 1] +
			     ((newamp - channel[i - 1]) *
			     ((float) (i - hibin) / (float) (hicutbin - hibin))) ; 
		    }else{
			// MIDDLE
			channel[i - 1] = newamp ; 
			 
		    }


		}
	}

	// FRAME NORMALIZE IF DESIRED. 
// ******	
	//  NORMALIZE AND REPLACE INTO CHANNEL.
		// FIND NORMALIZATION FACTOR.
/*
	tempChannelAmpSum = 0. ; 	
	for( i = 0; i < (N + 2); i+= 2 ){
		tempChannelAmpSum += channel[i] ;
	} ; 
	if( frame_count == 0 ) previousTempChannelAmpSum = tempChannelAmpSum ; 
	tempChannelAmpSum = (minusnormenv * tempChannelAmpSum) + (normenv * previousTempChannelAmpSum) ;  
		// NORMALIZE AND TRANSFER
	if( (tempChannelAmpSum > 0.0) && (frameNormalizationAmpLimit != 1.0 ) ){
		normalizationAmp = channelAmpSum / tempChannelAmpSum ;
		if( normalizationAmp > frameNormalizationAmpLimit )
			normalizationAmp = frameNormalizationAmpLimit ; 
		for( i = 0; i < (N + 2); i+= 2 )
			channel[i] = channel[i] * normalizationAmp ; 	
	} ; 
*/

// *******

	// SMOOTH THE CHANGES TO THE SPECTRUM
	//smooth( channel, previous_channel, (N + 2), envattack, minusattack, envrelease, minusrelease ) ; 

		
	for( i = 1; i < (N + 2); i+= 2 ){


		// ADD THIS MULTIPLIER TO THE STORE MULTIPLIERS
		    channel_freqdev[ i ] *= pm ; 
		// ADD THIS ADDER TO THE STORE ADDERS
		    channel_freqdev[ i - 1 ] += harmadd.A[ 0 ]  ; 

		    temp = pm * (channel[i] + harmadd.A[ 0 ]) ;
		    
		    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		    if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
			else channel[i] = temp ; 
	    
		    channel[i - 1] = channel[i - 1] * gain  ; // * compthreshampInverse ;  

	}
		


//*************EQUALIZE THE OUTPUT SPECTRUM
		dBlow.A[ 0 ] =  fval( &dBlow, dur, t );
		dBhi.A[ 0 ] =  fval( &dBhi, dur, t );
		freqlow.A[ 0 ] =  fval( &freqlow, dur, t );
		freqhi.A[ 0 ] =  fval( &freqhi, dur, t );

    //*************EQUALIZE THE OUTPUT SPECTRUM
    eq2( channel,  (N + 2),  dBlow.A[0],  dBhi.A[0], freqlow.A[0],  freqhi.A[0],  fundamental, channel_freqdev, 0 ) ; 


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
    fclose(ifd) ;  



    fprintf(stderr,"\nCOMPANDER : RESYNTHESIS COMPLETED\n");




    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
    if( peaks.n != 1. ) fclose(peaks.fp ) ;
    if( compthresh.n != 1. ) fclose(compthresh.fp ) ;
    if( compdB.n != 1. ) fclose(compdB.fp ) ;
    if( expandthresh.n != 1. ) fclose(expandthresh.fp ) ;
    if( expanddB.n != 1. ) fclose(expanddB.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;
    if( lowcut.n != 1. ) fclose(lowcut.fp ) ;
    if( hicut.n != 1. ) fclose(hicut.fp ) ;
    if( octavesrolloff.n != 1. ) fclose(octavesrolloff.fp ) ;
    if( dBlow.n != 1. ) fclose(dBlow.fp ) ;
    if( dBhi.n != 1. ) fclose(dBhi.fp ) ;
    if( freqlow.n != 1. ) fclose(freqlow.fp ) ;
    if( freqhi.n != 1. ) fclose(freqhi.fp ) ;

    exit(EXIT_SUCCESS) ;
}

void usage()
{
    fprintf(stderr, "%s",
	"compander:  amplitude compressor/expander\n"
	"compander   [flags] [input file] [output file]\n"
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
	"	P:	pitch transposition in semitones (func) [0]\n"
	"	a:	frequency shift factor (bin frequency adder, before -P )(func)[0.] \n"
	"	A:	gain in decibels (func) [0.] \n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds (0. = end of file) [0.] \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"

	"	F:	PEAKS FREQUENCY RESPONSE FILE \n"
	"	S:	PEAKS RESPONSE: smoothing bandwidth (in octaves or frequency) [0]\n"
	"		    (The value for each peak is  replaced with the average\n"
	"		     value within the band, centered around the peak. Positive values\n"
	"			are interpreted as frequency bandwidths and negative values as\n"
	"			    octave bandwidths, once made positive.\n"
	"	Z:	(SMOOTHED) PEAKS FREQ RESPONSE PRINTOUT: high cutoff frequency in Hz [0]\n"
	"		    (0 = off)\n"

	"	o:	COMPRESSION: threshold in dB (0 to -96)(func) [-0.]\n"
	"	O:	decibels of compression  (0 to -96)(func) [-0.(off)]\n"
	"	q:	EXPANSION: threshold in dB (0 to -96)(func) [-96.]\n"
	"	Q:	decibels of expansion (0 to -96) (func) [-0.(off)]\n"

	"		COMPANDING FREQUENCY BAND:\n"
	"	c:	low cutoff frequency (func) [0]\n"
	"	d:	high cutoff frequency (func) [Nyquist]\n"
	"	f:	octaves of rolloff from freq band (func) [0.]\n"
	"	l:	COMPANDER envelope attack time  (func) [0.]\n"
	"	L:	COMPANDER envelope release time  (func) [0.]\n"



	"	_:	 AUTO OUTPUT SOUND FILE PLAY:\n"
	"		    0  = off \n"
	"		    -1 = interactive: Prompt for each play.\n"
	"		    -2 = interactive: Play once, then prompt for more.\n"
        "                   1 or greater = Auto-repeat for specified repetitions. [0]\n" 

	"	=:	 PEAK RESCALE LEVEL 0 to -96 dB \n"
	"		    1 = Rescale to level of input file.\n" 
	"		    2 = Bypass rescaling. [ 1 ]\n" 

	"	t:	oscillator resynthesis threshold in decibels [ -96 ]\n"
	"	     SHELF EQ: (post transpose/shift) \n"
	"	H:	SHELF EQ: Low shelf gain in dB (func) [0.] \n"
	"	X:	SHELF EQ: High shelf gain in dB (func) [0.] \n"
	"	m:	SHELF EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	R:	SHELF EQ: High shelf frequency in Hz (func) [2000.] \n"

	"	p:	amplitude reports print mode: 0 = off, 1 = on [0]\n" 
	"	i:	time interval between amplitude reports [.25]\n" 



	);
    exit(EXIT_SUCCESS);
}



void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
