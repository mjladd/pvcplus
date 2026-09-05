#include "globals.h"


void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j;
float nyquist,  fundamental ;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 220, I = 220, in, on;
int   eof = 0, obank = 0,  channelout=0 ;
float P = 1.0;
FILE *fopen();
char ch,  tempstring[ STRING_SIZE ] ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, 
    *buffer, *channel, *tempchannel, *output ;
float threshfac = .001,  threshfacdB=-96.;
float  *binfreq,  dur ;
float  gain;
float  *previous_channel1, *wouldbephasepoint,  *channel_freqdev, *previous_gain_mult;
float	*avg_change ; 
float  temp,  pm,  IR  ;  
float getthresh();
 

float channelAmpSum, tempChannelAmpSum, normalizationAmp, frameNormalizationAmpLimit ; 
float freqChangeBWnormalizer ; 

float avgresponsec, minusavgresponsec ; 

// SHELF EQ

float f_spect_t,  freqc; 
int spect_type=0 ; 

float releasec,  minusreleasec ; 
double ar_dB ; 
float factor,  ampfactor ; 

// FRAME NORMALIZATION DECIBEL LIMIT
struct  func  frameNormalizationDecibelLimit ; 


//  COMPLEMENT SPECTRUM  PROPORTION
struct  func  complementprop ; 

// FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 

//  RELEASE
struct  func  release ; 



// SPECTRUM WARPSHAPE INDEX
struct  func  warpshape ; 


//SHELF EQ
struct  func  dBlow;
struct  func  dBhi;
struct  func  freqlow;
struct  func  freqhi ;

//  FREQ  EXTRACTION THRESHOLD IN PER CENT CHANGE
struct  func  f_spect_thresh ; 

//  FREQ  CHANGE THRESHOLD RESPONSE TIME
struct  func  avgresponse ; 

//#include "underflow.h"

//*****************INITIALIZE

// FRAME NORMALIZATION DECIBEL LIMIT
frameNormalizationDecibelLimit.L = 1. ; 
	frameNormalizationDecibelLimit.n = 1. ; frameNormalizationDecibelLimit.A[ 0 ] = 0. ; 

//  COMPLEMENT SPECTRUM  PROPORTION
complementprop.L = 1. ; complementprop.n = 1. ; complementprop.A[ 0 ] = 0. ; 


// FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH MULTIPLIER
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 

//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 



// SPECTRUM WARPSHAPE INDEX
warpshape.L = 1. ; warpshape.n = 1. ; warpshape.A[ 0 ] = 0. ; 

//  FREQ  EXTRACTION THRESHOLD IN PER CENT CHANGE
f_spect_thresh.L = 1. ; f_spect_thresh.n = 1. ; f_spect_thresh.A[ 0 ] = 0. ; 

//  FREQ  CHANGE THRESHOLD RESPONSE TIME 
avgresponse.L = 1. ; avgresponse.n = 1. ; avgresponse.A[ 0 ] = 0. ; 




// SHELF EQ
dBlow.L = 1. ; dBlow.n = 1. ; dBlow.A[ 0 ] = 0. ; 
dBhi.L = 1. ; dBhi.n = 1. ; dBhi.A[ 0 ] = 0. ; 
freqlow.L = 1. ; freqlow.n = 1. ; freqlow.A[ 0 ] = 200. ; 
freqhi.L = 1. ; freqhi.n = 1. ; freqhi.A[ 0 ] = 2000. ; 

strcpy( routine, "spectralextractor" ) ; 

if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv,
    "E|c|H|m|X|R|N|M|P|D|t|I|s|a|A|p|q|T|Q|_|=|W|g|X|v|p|i|S|w|T|C|n|x|u|U|d|B|f|F|L|l|b|e|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = atoi(arg_option);
			break;
	    case 'M':   Nw = atoi(arg_option);
			break;
	    case 'w':   window_type = atoi(arg_option) ;
			break;
	    case 'D':   frames_per_sec = atof(arg_option);
			break;
	    case 'I':   tfactor = atof(arg_option);
			break;
	    case 'P':   strcpy(tempstring, arg_option);
			ptrans.fp = crackstring( tempstring, &ptrans); 
			break;
	    case 't':   threshfacdB = atof(arg_option);
			break;
	    case 'a':   strcpy(tempstring, arg_option);
			harmadd.fp = crackstring( tempstring, &harmadd );
			break;

	    case 'b':   begint = atof(arg_option) ;
			break;
	    case 'e':   endt = atof(arg_option) ;
			break;

	    case 'C':   channelout = atoi(arg_option) ;
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

	    case 'W':   strcpy(tempstring, arg_option);
			warpshape.fp = crackstring( tempstring, 
			    &warpshape );
			break;


	    case 'Q':   strcpy(tempstring, arg_option);
			f_spect_thresh.fp = crackstring( tempstring, &f_spect_thresh );
			break;


	    case 'g':   strcpy(tempstring, arg_option);
			avgresponse.fp = crackstring( tempstring, &avgresponse );
			break;

	    case 'c':   strcpy(tempstring, arg_option);
			complementprop.fp = crackstring( tempstring, 
			    &complementprop );
			break;

	    case 'E':   strcpy(tempstring, arg_option);
			frameNormalizationDecibelLimit.fp = crackstring( tempstring, 
			    & frameNormalizationDecibelLimit );
			break;



	    case 'q':   spect_type = atoi(arg_option) ;
			break;


	    case 'A':   strcpy(tempstring, arg_option);
			dBgain.fp = crackstring( tempstring, &dBgain );
			break;

	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;

            case 'p':	quiet = atoi(arg_option) ; break;
            case 'i':	ampstatinc = atof(arg_option) ; break;

           case '_':	autoplayreps = atoi(arg_option) ; break;

           case '=':	rescalev = atof(arg_option) ; break;


	} }



prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "SPECTRALEXTRACTOR", 69 ) ; 
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

    nyquist = R/2.0;
    fundamental =  ((float) R / (float) N) ; 
    PI = 4.*atan(1.) ;
    TWOPI = 8.* (float) atan(1.) ;
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    ampfactor = 1. / pow( (double) 10.0, (double) (-100./20.) );	
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
// HERE
    IR = (float) I / (float) R ;
    factor = (float) R / ((float) D * TWOPI);

// HERE
    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 

// DETERMINE OVERLAP/ADD OR OSCIL BANK RESYNTHESIS
    if( 
	(ptrans.n  != 1.) || (harmadd.n  != 1.)  ||
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.) ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
    

    freqChangeBWnormalizer = 10.0 / (0.5 * fundamental) ; 
    
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
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 

prbanner( "RESYNTHESIS PARAMETERS",  69 ) ; 

pri( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
prline( 1,  "*" ) ; 
pri( I,  "      INTERPOLATION SAMPLES (samples between resynthesis frames)" ) ; 
prline( 1,  "*" ) ; 
prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (in dB)" ) ; 
prline( 1,  "*" ) ; 
prp( &dBgain,  "GAIN (in dB)"  ) ; 
prp( &ptrans,  "PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &harmadd,  "FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
prp( &release,  "ENVELOPE RELEASE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prp( &warpshape, "SPECTRUM WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prp( &freqlow, "LOW SHELF FREQUENCY" ) ; 
prp( &dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prp( &freqhi, "HIGH SHELF FREQUENCY" ) ; 
prp( &dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
prp( &f_spect_thresh, "THRESHOLD REPRESENTING THE MAX(PERIODIC) /MIN(APERIODIC) \n\tFREQUENCY CHANGE ALLOWED EVERY 5 MILISECONDS" ) ; 
prp( &avgresponse, "RESPONSE TIME OF FREQUENCY-CHANGE THRESHOLD ACCUMULATOR TO CHANGE" ) ; 
if( spect_type == 0 )prt( "EXTRACTING PERIODIC SIGNAL (BELOW THRESHOLD)" ) ; 
    else prt( "EXTRACTING NOISE (ABOVE THRESHOLD)" ) ; 
prp( &complementprop,  "COMPLEMENT AMPLITUDE SPECTRUM PROPORTION" ) ; 
prp( & frameNormalizationDecibelLimit, "FRAME NORMALIZATION DECIBEL LIMIT" ) ; 

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
    fvec( output, Nw ) ;	/* output buffer */
    fvec( previous_channel1, N+2 ) ;	/* previous analysis channels */
    fvec( previous_gain_mult, N2 ) ;	/* precious gain multiplier */
    fvec( avg_change,  N+2 ) ;	/* average amp/freq change */



    fvec( binfreq, N2 + 1 ) ;	/* bin frequencies */
    fvec( wouldbephasepoint, N2 + 1 ) ;	/* no change projected  phase point */

    fvec( channel_freqdev,  N + 2 ) ;	// channel SORT ARRAY ACCUMULATOR

    
    // SET UP BIN FREQUENCIES
    for( i = 0; i < (N2 + 1);  i++ ) binfreq[i] = (float) i * fundamental ; 

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

// HERE

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
		    previous_channel1[ i ] = channel[ i ] ; 




	// SETUP PREVIOUS GAIN MULTIPLIER
	    if( !frame_count )for(i = 0; i < (N2 + 1); i++)
		    previous_gain_mult[ i ] = 1. ; 

    
	// SETUP PREVIOUS AVERAGE CHANGE
	    if( !frame_count )for(i = 0; i < (N + 2); i++)
		    avg_change[ i ] = 0. ; 


	// SETUP FREQDEV HISTORY ARRAY
	    	for( i = 1; i < (N + 2); i+= 2 ) {
			channel_freqdev[ i - 1 ] = 0. ; 
			channel_freqdev[ i ] = 1. ; 
		}


//*************************
// GET THE VALUES
//*************************


		harmadd.A[ 0 ] = fval( &harmadd, dur, t );
		dBgain.A[ 0 ] = fval( &dBgain, dur, t );
		    gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
		ptrans.A[ 0 ] = fval( &ptrans, dur, t ) ;
		pm = semitones_to_mult( ptrans.A[ 0 ] ) ;

		release.A[ 0 ] = fval( &release, dur, t );
		smooth_setup( release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 

		warpshape.A[ 0 ] =  fval( &warpshape, dur, t );

		dBlow.A[ 0 ] =  fval( &dBlow, dur, t );
		dBhi.A[ 0 ] =  fval( &dBhi, dur, t );
		freqlow.A[ 0 ] =  fval( &freqlow, dur, t );
		freqhi.A[ 0 ] =  fval( &freqhi, dur, t );


		f_spect_thresh.A[ 0 ] = fval( &f_spect_thresh, dur, t );
		f_spect_t =  200. * (f_spect_thresh.A[ 0 ]) /  ((float) frames_per_sec) ; 

		avgresponse.A[ 0 ] = fval( &avgresponse, dur, t );
		smooth_setup( avgresponse.A[ 0 ], &avgresponsec, &minusavgresponsec, IR ) ; 


	    // COMPLEMENT SPECTRUM PROPORTION
		complementprop.A[ 0 ] = fval( &complementprop, dur, t );

	    // FRAME NORMALIZATION DECIBEL LIMIT
		frameNormalizationDecibelLimit.A[ 0 ] = 
			fval( & frameNormalizationDecibelLimit, dur, t );
		frameNormalizationAmpLimit = dB_to_amp( frameNormalizationDecibelLimit.A[ 0 ] ); 
/*
if( !frame_count ){
prf( minusavgresponsec,  "minusavgresponsec" ) ;  
prf( avgresponsec,  "avgresponsec" ) ; 
pri( spect_type,  "spect_type" ) ;     
prf( f_spect_t,  "f_spect_t" ) ; 
}
*/

//*************************
		

//********



// CHANGE THE AMPLITUDES 
		
		channelAmpSum = 0. ; 	
		 for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){
			channelAmpSum += channel[i] ; 
			tempchannel[i] = channel[i] ;
			tempchannel[i - 1] = channel[i - 1] ;

		    // CREATE AVERAGE CHANGE IN FREQ

		    freqc =  fabs( (double) (tempchannel[i] - previous_channel1[i]) ) *
				freqChangeBWnormalizer ; 

		    // MAKE AVERAGE 
		    avg_change[i] = 
			( minusavgresponsec * freqc ) +
			     (  avgresponsec * avg_change[i] ) ; 


//fprintf( stderr,"\n%d ) (%d Hz) freqc: %f ,  avg_change: %f", j,  j * (int) fundamental, freqc,  avg_change[i] ) ;


		    if(spect_type == 0 ){
			// PERIODIC
			if( (avg_change[i] > f_spect_t)  ){
			    // TO OFF
			    temp =  releasec * previous_gain_mult[j] ;
			    
			}else{
			    // TO ON
			    temp =  (releasec * previous_gain_mult[j] ) + minusreleasec  ;
			    
			}

		    }else{
			// NOISE
			if( (avg_change[i] < f_spect_t)  ){
			    // TO OFF
			    temp =  releasec * previous_gain_mult[j] ;
			    
			}else{
			    // TO ON
			    temp =  (releasec * previous_gain_mult[j] ) + minusreleasec ;
			    
			}
	
		    }

		    tempchannel[i - 1] *= temp ; 
		    previous_gain_mult[j] = temp ; 

		    previous_channel1[i - 1] = tempchannel[i - 1] ; 
		    previous_channel1[i] = tempchannel[i] ; 

		}

//********

	// MAKE SOURCE/COMPLEMENT MIX.
	for( i = 0; i < (N + 2); i+= 2 ){
		tempchannel[i] = tempchannel[i] + 
			(complementprop.A[ 0 ] * (channel[i] - (2. * tempchannel[i]))) ; 
	} ; 

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



// *********


// CHANGE THE AMPLITUDES 

    for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){
		     
		// ADD THIS MULTIPLIER TO THE STORE MULTIPLIERS
		    channel_freqdev[ i ] *= pm ; 
		// ADD THIS ADDER TO THE STORE ADDERS
		    channel_freqdev[ i - 1 ] += harmadd.A[ 0 ]  ; 
		    
		    
		    // NEUTOR OUT OF BOUNDS FREQ BINS
		    temp = pm  * (harmadd.A[ 0 ]  + channel[i]) ;
		    
    }

    // ADD GAIN
   for( i = 1; i < (N + 2); i+= 2 ) channel[i - 1] = channel[i - 1] * gain ;

    //*************WARP THE INPUT SPECTRUM
    spectmagwarp( channel,  (N + 2), warpshape.A[ 0 ], 0 ) ;		

    //*************EQUALIZE THE OUTPUT SPECTRUM
    eq2( channel,  (N + 2),  dBlow.A[0],  dBhi.A[0], freqlow.A[0],  freqhi.A[0],  fundamental, channel_freqdev, 0 ) ; 
		
		
        synt = getthresh( channel, (N + 2), threshfac );

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


   if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
   if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
   if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
   if( release.n != 1. ) fclose(release.fp ) ;
   if( warpshape.n != 1. ) fclose(warpshape.fp ) ;




     
    fprintf(stderr,"\nSPECTRAL EXTRACTOR: RESYNTHESIS COMPLETED\n");
    exit(EXIT_SUCCESS) ;
}

void usage()
{
    fprintf(stderr, "%s",
	"spectralextractor:  periodic or noise spectrum extractor  \n"
	"spectralextractor   [flags] [input file] [output file]\n"
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
	"	a:	frequency shift factor \n"
	"		    (bin frequency adder, before -P )(func) [0.] \n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds ( 0. = end of file) [0.] \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"

	"	q:	SPECTRAL TYPE: 0 = periodic,  1 = noise [0]\n"
	"	Q:	FREQUENCY CHANGE THRESHOLD: threshold representing the \n"
	"		    max(periodic) or min(noise) frequency change  allowed in\n"
	"		    5 miliseconds, in Hz (0-?) (func) [0.]\n"
	"	g:	RESPONSE TIME of frequency change threshold to change,in seconds (func) [0]\n"

	"	     SHELF EQ:(post transpose/shift)\n"
	"	H:	SHELF EQ: Low shelf gain in dB (func) [0.] \n"
	"	X:	SHELF EQ: High shelf gain in dB (func) [0.] \n"
	"	m:	SHELF EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	R:	SHELF EQ: High shelf frequency in Hz (func) [2000.] \n"

	"	W:	warp index for reshaping magnitude response (func) [0.] \n"
	"		    Values > 0 expand the dynamic range, \n"
	"		    values < 0 compress the dynamic range. \n"

	"	c:	COMPLEMENT AMPLITUDE SPECTRUM proportion (0-1) (func) [0.]\n"
	"		    (Value of 1 produces the difference of the old and new bin amp.\n"
	"		      0 = no inversion. The sum of the old and new equals the original.)\n"

	"	E:	Frame Normalization Decibel Limit: (0-96) \n"
	"		Scale output frame amps to match or approach input frame amps\n"
	"		using the (sum of input amps)/(sum of output amps) limited to\n" 
	"		the Decibel limit. 0 dB prevents normalization. [0]\n"   


	"	A:	gain in decibels (func) [0.] \n"
	"	L:      amplitude change response time in seconds  (func) [0.]\n"

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


	"	t:	oscillator resynthesis threshold in decibels [ -96 ]\n"

	);
    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
