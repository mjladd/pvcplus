#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j;
float pm; 
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  channelout=0 ;
float P = 1.0;

float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float *previous_channel,  *F, *amp_change, 
	  *previous_amp_change ; 
float threshfac = .001,  threshfacdB=-96 ;
float releasec,  minusreleasec,  attackc,  minusattackc ; 
double ar_dB ; 
float	gain=1. ;
FILE *fopen();


float noise_thresh_limit_dB=0. ;  
 


 
int print_flag=0 ; 
char ch;
// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 

float  temp,  temp2;  
float getthresh();
float   IR,  dur=0.;

float dBnumerator ; 
float fundamental, factor,   noisefiltergain;  

float temp_begint,  temp_endt ; 
float A_begint=0.,  A_endt=0. ; 
int A_method, A_print=22050 ; 
char  tempResponseFile[ STRING_SIZE ], scratchString[ STRING_SIZE ], responsePlotFile[ STRING_SIZE ] ; 

float thisPeakAmp ; 

float sum_noise_amp,  frame_sum; 

char tempstring[ STRING_SIZE ] ; 

// COMB FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH SEMITONE TRANSPOSE
struct  func  ptrans ; 


//  SOURCE DECIBELS
struct  func  noisethreshadjustdB ; 

//  ATTACK
struct  func  attack ; 

//  RELEASE
struct  func  release ; 


// NOISE EXPANSION INDEX
struct  func  expandex ; 

// FILTER
struct  func  filter ; 


//*****************INITIALIZE
// COMB FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH SEMITONE TRANSPOSE
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 


//  SOURCE DECIBELS
noisethreshadjustdB.L = 1. ; noisethreshadjustdB.n = 1. ; noisethreshadjustdB.A[ 0 ] = 0. ; 

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 

//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 


// NOISE EXPANSION INDEX
expandex.L = 1. ; expandex.n = 1. ; expandex.A[ 0 ] = 3. ; 

// FILTER
filter.L = 1. ; filter.n = 0. ; filter.A[ 0 ] = 0. ; 


if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, "y|f|R|N|M|w|P|D|t|x|_|=|n|I|p|i|B|F|b|G|E|e|C|i|T|Q|Z|l|L|V|S|H|m|X|W|c|d|s|a|A|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = (int) crackfloat(arg_option, ch);
			break;
	    case 'M':   Nw = (int) crackfloat(arg_option, ch);
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

	    case 'b':   begint = temp_begint = crackfloat(arg_option, ch) ;
			break;
	    case 'e':   endt = temp_endt = crackfloat(arg_option, ch) ;
			break;

	    case 'C':   channelout = (int) crackfloat(arg_option, ch) ;
			break;



           case '_':	autoplayreps = (int) crackfloat(arg_option, ch) ; break;

           case '=':	rescalev = crackfloat(arg_option, ch) ; break;


	    case 'H':   dBlow = crackfloat(arg_option, ch) ;
			break;
	    case 'X':   dBhi = crackfloat(arg_option, ch) ;
			break;
	    case 'm':   freqlow = crackfloat(arg_option, ch) ;
			break;
	    case 'R':   freqhi = crackfloat(arg_option, ch) ;
			break;

            case 'p':	quiet = (int) crackfloat(arg_option, ch) ; break;
            case 'i':	ampstatinc = crackfloat(arg_option, ch) ; break;
            case 'Z':	print_flag = (int) crackfloat(arg_option, ch) ; break;


            case 'B':	A_begint = crackfloat(arg_option, ch) ; break;
            case 'E':	A_endt = crackfloat(arg_option, ch) ; break;
            case 'F':	A_method = (int) crackfloat(arg_option, ch) ; break;
            case 'y':	A_print = (int) crackfloat(arg_option, ch) ; break;



	    case 'S':   strcpy(tempstring, arg_option);
			noisethreshadjustdB.fp = crackstring( tempstring, 
			    &noisethreshadjustdB );
			break;

	    case 'x':   strcpy(tempstring, arg_option);
			expandex.fp = crackstring( tempstring, 
			    &expandex );
			break;



            case 'Q':	noise_thresh_limit_dB = crackfloat(arg_option, ch) ; break;


	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;


	    case 't':   threshfacdB = crackfloat(arg_option, ch);
			break;
	}
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "FREQRESPONSE", 69 ) ; 
prline( 69,  "-" ) ; 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    } ; 



// **** MAKE AND SAVE TO FILE NOISE RESPONSES FOR EACH CHANNEL
getInputFileDataToSetOutputChannels(argc, argv); 

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ )
{
      // MAKE AN AVERAGE OR PEAK FREQRESPONSE  USING SYSTEM CALL TO freqresponse.

      sprintf( tempResponseFile, "/tmp/%s.noiseResponseFile.chan%d", getlogin(), outchan ) ; 




      filesToRemove( tempResponseFile, 0 ); 


      sprintf( responsePlotFile, "/tmp/%s.noiseResponsePlotFile.chan%d", getlogin(), outchan ) ; 

      fprintf( stderr, "\nAnalyzed noise response plot file written to: %s\n", responsePlotFile ) ; 

      snprintf( scratchString, sizeof(scratchString),
         "freqresponse  -a%s -B1 -M0  -D%f -w%i -N%i   -b%f -e%f  -c%i -P%i -C%i %s %s",
	responsePlotFile, frames_per_sec, window_type, N,
	A_begint, A_endt, A_method, A_print, (outchan + 1), ifile, tempResponseFile
      ) ;


      // PRINT COMMAND

      prt( "** SYSTEM COMMAND: " ) ; 

      fprintf( stderr, "\n%s\n", scratchString ) ; 


         // RUN COMMAND
      system( scratchString ) ; 

} ; 


// END NOISE RESPONSE MAKE.
 
prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "NOISEFILTER", 69 ) ; 
prline( 69,  "-" ) ; 



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
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    IR = (float) I / (float) R ; 
    fundamental = (float) R / (float) N ; 
    factor = (float) R / ((float) D * TWOPI) ; 
    dBnumerator = (float) pow( (double) 10.0, (double) -3.9 ) ; 
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    
    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 


// ******** FILTER FILE SETUPS
    if( A_endt <= 0. ) A_endt = idur ; 

// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( 
	(ptrans.n  != 1.) || (harmadd.n  != 1.) || 
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.)  ) {
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
prline( 1,  "*" ) ; 
//*****************

prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 

prf( frames_per_sec,  "FRAMES/SECOND" ) ; 
prf( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
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
prp( &noisethreshadjustdB,  "NOISE THRESHOLD ADJUST (in dB)"  ) ; 
prline( 1,  "*" ) ; 
prp( &attack,  "ENVELOPE ATTACK TIME (in seconds)"  ) ; 
prp( &release,  "ENVELOPE RELEASE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 


prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
// *******


//***************** SET UP ARRAYS


    
    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */
    fvec( amp_change, N2+1 ) ;	/* previous amplitude change for smoothing */
    fvec( previous_amp_change, N2+1 ) ;	/* previous amplitude change for smoothing */
    fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

// MAKE THRESH AMP
    threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	

// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

// ALLOCATE FILTER SPACE
fvec( F,  N+2  ) ;	/* filter array */


    if(dBlow != dBhi) prt("......USING SHELF EQ..............." ) ; 

//*********************************************
// LOOP FOR CHANNELS
//*********************************************



for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

thisPeakAmp = 0. ; 
prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 


   // OPEN RESPONSE FILE
   sprintf( tempResponseFile, "/tmp/%s.noiseResponseFile.chan%d", getlogin(), outchan ) ; 
//   sprintf( tempResponseFile, "/Users/koonce/%s.noiseResponseFile.chan%d", getlogin(), outchan ) ; 



   filter.fp = crackstring( tempResponseFile, &filter );

   //************ FILL ARRAY FROM FILE
    fillfunc( &filter, F, (N + 2)  ) ;     




// FIND THE AVERAGE AMPLITUDE
    sum_noise_amp = 0. ; 
    for( i = 0; i < (N + 2);  i += 2 ){
	sum_noise_amp += F[i] ; 
    }

    if(print_flag)tprintspec( F, (N + 2), fundamental,  print_flag) ;

    fprintf( stderr, "\n(OUT) noise_thresh_limit_dB = %f",  noise_thresh_limit_dB ) ; 

// SET TO ZERO THE BINS WHICH ARE MORE THAN THE THRESHOLD
    threshold_limit( F, (N + 2),  noise_thresh_limit_dB ) ;  


//*********** PRINT TO TERMINAL IF DESIRED ******
    if(print_flag)tprintspec( F, (N + 2), fundamental,  print_flag) ;
    
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

	// SETUP PREVIOUS AMP CHANGE FOR SMOOTHING
	    if( !frame_count )for(i = 0; i < (N2 + 1); i++)
		    previous_amp_change[ i ] = 1. ; 



//*************************
// GET THE VALUES
//*************************



//  SHIFT, GAIN, AND TRANSPOSITION

		harmadd.A[ 0 ] =  fval( &harmadd, dur, t );
		dBgain.A[ 0 ] =  fval( &dBgain, dur, t );
		ptrans.A[ 0 ] = fval( &ptrans, dur, t );
		    gain = dB_to_amp(dBgain.A[ 0 ] );	
		    pm = semitones_to_mult( ptrans.A[ 0 ] ) ;


		noisethreshadjustdB.A[ 0 ] = fval( &noisethreshadjustdB, dur, t );
		    noisefiltergain = dB_to_amp(noisethreshadjustdB.A[ 0 ] );

		expandex.A[ 0 ] = fval( &expandex, dur, t );




		attack.A[ 0 ] = fval( &attack, dur, t );
		smooth_setup( attack.A[ 0 ], &attackc, &minusattackc, IR ) ; 
		release.A[ 0 ] = fval( &release, dur, t );
		smooth_setup( release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 



//*****************
// MODIFICATIONS LOOP
//*****************

    // FIND THE SUM OF THE FRAME'S AMPLITUDES
    frame_sum = 0. ; 
    for( i = 0; i < (N + 2); i+= 2 ) frame_sum += channel[ i ] ; 

/*
    if( (frame_sum > sum_noise_amp) )
	trackampgain =  sum_noise_amp / frame_sum ; 
    else
	trackampgain = 1. ; 
*/




          // NOISE FILTERING
              // MAKE AMP CHANGE MULTIPLIER FOR ALL BINS
          for( i = 0,  j = 0; i < (N + 2); i+= 2,  j++ ){
              // MAKE THRESHOLD LEVEL AS MODIFIED BY THRESHOLD GAIN
              temp2 = F[i] * noisefiltergain ; 
		    
              // IF THRESHOLD IS > 0., THEN
             if( temp2 > 0.  ){
                    // IF AMP IS BELOW THRESHOLD, THEN
                    if( channel[i] < temp2 ){

                    // MAKE AMP MULTIPLIER TO EXPAND OUT THE NOISE
			    // FIRST, MAKE FORM OF AMP NORMALIZED TO THRESHOLD
			    temp = channel[i] / temp2 ;
			     
			    // USE NORMALIZED AMP TO MAKE AMP MULTIPLIER (0-1)
			    amp_change[j] =  curve(0., 1., temp,  expandex.A[ 0 ]  );

			}else{
			    // AMP ABOVE THRESHOLD,  NO GAIN MULTIPLER
			     amp_change[j] = 1. ; 
			}
			// NOW SMOOTH THIS MULTIPLIER AGAINST PREVIOUS
		

		    }else{
			// THRESHOLD BELOW 0., NO GAIN MULTIPLER
			amp_change[j] = 1. ; 
		    }
		}
		
		// SMOOTH AMP CHANGE MULTIPLIERS
		smooth_amp_change( 
		    amp_change, previous_amp_change, (N2 + 1), 
			attackc, minusattackc, releasec, minusreleasec ) ; 
		
		// NOW APPLY AMP CHANGES TO SPECTRUM
		for( i = 0,  j = 0; i < (N + 2); i+= 2,  j++ ) channel[ i ] *= amp_change[ j ] ; 


			

//*************EQ 
    eq( channel,  (N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1,  0, 0 ) ; 


    // SMOOTH THE CHANGES TO THE SPECTRUM
//    smooth_zero( channel, previous_channel, (N + 2), attackc, minusattackc, releasec, minusreleasec, 
//			F, noisefiltergain ) ; 




	for( i = 1; i < (N + 2); i+= 2 ){

	    // SHIFT BY -a AND TRANSPOSE BY -P

	    temp = pm * (channel[i] + harmadd.A[ 0 ]) ;
		    
	    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
	    if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
	    else channel[i] = temp ; 
	    
	    channel[i - 1] = channel[i - 1] * gain ;  

	}
		

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

    filesToRemove( NULL, 1 ); 



    fprintf(stderr,"\n\nNOISEFILTER : RESYNTHESIS COMPLETED\n");
 
   fclose( filter.fp ) ; 

    if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
    if( ptrans.n != 1. ) fclose(ptrans.fp ) ;


   if( release.n != 1. ) fclose(release.fp ) ;
   if( attack.n != 1. ) fclose(attack.fp ) ;
   if( noisethreshadjustdB.n != 1. ) fclose(noisethreshadjustdB.fp ) ;

   if( expandex.n != 1. ) fclose(expandex.fp ) ;



    exit(EXIT_SUCCESS) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"noisefilter:  phase vocoder spectrum remover \n"
	"noisefilter   [flags] [input file] [output file (optional)]\n"
	"	    (values in brackets denote defaults)\n"
	"	N:	"FFT_LENGTH 		// N
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec
	"	I:	"TIME_FACTOR		// tfactor

	"	P:	"PITCH_TRANS		// ptrans
	"	a:	"FREQ_SHIFT		// harmadd
	"	A:	"DB_GAIN			// dBgain

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout

	"	B:	NOISE RESPONSE ANALYSIS: begin time in seconds  [0.] \n"
	"	E:	NOISE RESPONSE ANALYSIS: end time in seconds (0. = end of file) [0.] \n"
	"	F:	NOISE RESPONSE ANALYSIS: method: 0 = average, 1 = peak  [0.] \n"
	"	Z:	NOISE ANALYSIS FREQUENCY RESPONSE PRINTOUT:\n"
	"		     high cutoff frequency in Hz (0 = off) [0]\n"



	"	S:	NOISE THRESHOLD ADJUST: gain in dB (func) [0.] \n"
	"		    Values > 0. increase the amount of noise reduction\n"
	"		    Values < 0. reduce the amount of noise reduction\n"
	"	x:	NOISE EXPANSION INDEX 0 - ? [3.] \n"
	"		    This value controls the shape of the expansion curve.\n"
	"		    The expansion is increased with greater values.\n"
	"	Q:	NOISE RESPONSE ANALYSIS: bypass threshold in dB [1.]\n"
	"		    ( Bins in the normalized noise response whose amp is greater \n"
	"			than the threshold are replaced with 0. letting \n"
	"			those frequencies pass when filtered. Set to 1. to \n"
	"			pass all frequencies through noise filter. )\n"

	"	l:      envelope attack time  (func) [0.]\n"
	"	L:      envelope release time   (func) [0.]\n"

	"	      SHELF EQ: (post-noise filtering,  pre-transposition/shift)\n"
	"	H:	"SHELF_EQ_LOW_GAIN		// dBlow
	"	X:	"SHELF_EQ_HIGH_GAIN		// dBhi
	"	m:	"SHELF_EQ_LOW_FREQ		// freqlow
	"	R:	"SHELF_EQ_HIGH_FREQ		// freqhi

	"	t:	"RESYNTH_THRESHOLD		// threshfacdB
	"	p:	"AMP_REPORTS		// quiet 
	"	i:	"AMP_REPORTS_TIME_INTERVAL	// ampstatinc 

	"	y:	ANALYSIS PRINTOUT high cutoff frequency [22050] \n"



	"	_:	 "AUTO_PLAY		// autoplayreps

	"	=:	 "RESCALE_LEVEL		// rescalev

	);
    exit(EXIT_SUCCESS);
}



void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }