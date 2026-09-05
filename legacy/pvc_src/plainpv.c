#include "globals.h"

void usage() ;
void pd( int i ) ;

int main( int argc, char *argv[] )
{
int i,j, k, l,  m, nnn=0,    numbins,  offset ;
float nyquist,  fundamental ;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 220, I = 220, in, on;
int   eof = 0, obank = 0,  channelout=0,  write_ascii=0 ;
float P = 1.0;
FILE *fopen(),  *write_ascii_d,  *tdata ;
char ch,  tempstring[ STRING_SIZE ],  write_ascii_filename[ STRING_SIZE ]="./ascii.out", 
    scratch[128],  scratch2[ 128 ],  *user ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, 
    *buffer, *channel, *output ;
float threshfac = .001,  threshfacdB=-96.;
float  *binfreq,  dur ;
float  gain;
float  *previous_channel, *channel_freqdev;
float  temp, temp1,  temp2,  pm,  IR  ;  
float getthresh();
float lowfreq=-1,  hifreq=-1; 
int showme=0, phaseLockFlag=1 ; 

// SHELF EQ



float releasec,  minusreleasec,  attackc,  minusattackc ; 
double ar_dB ; 
float factor,  ampfactor ; 
int lowbin=0,  highbin=-1,  numberframes=0,  n=0 ; 
float lowbinfreq=0,  highbinfreq=-1 ; 
int filttype=0 ; 

// FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 

//  RELEASE
struct  func  release ; 

//  ATTACK
struct  func  attack ; 


// SPECTRUM WARPSHAPE INDEX
struct  func  warpshape ; 


//SHELF EQ
struct  func  dBlow;
struct  func  dBhi;
struct  func  freqlow;
struct  func  freqhi ;



//*****************INITIALIZE
// FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH MULTIPLIER
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 

//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 


// SPECTRUM WARPSHAPE INDEX
warpshape.L = 1. ; warpshape.n = 1. ; warpshape.A[ 0 ] = 0. ; 

// SHELF EQ
dBlow.L = 1. ; dBlow.n = 1. ; dBlow.A[ 0 ] = 0. ; 
dBhi.L = 1. ; dBhi.n = 1. ; dBhi.A[ 0 ] = 0. ; 
freqlow.L = 1. ; freqlow.n = 1. ; freqlow.A[ 0 ] = 200. ; 
freqhi.L = 1. ; freqhi.n = 1. ; freqhi.A[ 0 ] = 2000. ; 


if( argc < 2 )usage() ; 


    while( (ch = crack( argc, argv,
    "=|_|a|A|b|B|c|C|d|D|e|f|F|h|H|i|I|l|L|m|M|n|N|p|P|q|Q|R|s|S|t|T|u|U|v|w|W|x|X|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = (int) crackfloat( arg_option, ch );
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'I':   tfactor = crackfloat( arg_option, ch );
			break;
	    case 'P':   strcpy(tempstring, arg_option);
			ptrans.fp = crackstring( tempstring, &ptrans); 
			break;
	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	    case 'a':   strcpy(tempstring, arg_option);
			harmadd.fp = crackstring( tempstring, &harmadd );
			break;

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

	    case 'W':   strcpy(tempstring, arg_option);
			warpshape.fp = crackstring( tempstring, 
			    &warpshape );
			break;




	    case 'A':   strcpy(tempstring, arg_option);
			dBgain.fp = crackstring( tempstring, &dBgain );
			break;

	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;
	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;

            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;

           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;


	    case 'T':   filttype = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'f':   lowfreq = crackfloat( arg_option, ch );
			break;
	    case 'F':   hifreq = crackfloat( arg_option, ch );
			break;

	    case 'S':   showme = (int) crackfloat( arg_option, ch );
			break;
	    case 'u':   lowbinfreq = crackfloat( arg_option, ch ) ;
			break;
	    case 'U':   highbinfreq = crackfloat( arg_option, ch ) ;
			break;
	    case 'n':   numberframes = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'c':   write_ascii = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'd':   strcpy(write_ascii_filename, arg_option);
			break;

    } 
}



prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "PLAINPV", 69 ) ; 
prline( 69,  "-" ) ; 

    if(channelout == 0){ // ALL CHANNELS
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
 
// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
    setupfiles(argc, argv) ; 



    endchan = beginchan + ochan ; 

    // GET NAME OF USER
 	user = getlogin(); 
  
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
    if( (phaseLockFlag == 1) ||
	(ptrans.n  != 1.) || (harmadd.n  != 1.)  ||
	    (ptrans.A[0] != 0.) || (harmadd.A[0] != 0.) ) {
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }
    

// WINDOW FILTER SETUP

    if( (highbinfreq < 0.) || ( highbinfreq > nyquist ) )
	highbin = N / 2 ; 
     else
	 highbin = (int) (.5 + (highbinfreq /  fundamental)) ; 

    if( lowbinfreq < 0. )
	lowbin = 0 ; 
    else
	lowbin = (int) (.5 + (lowbinfreq /  fundamental)) ; 


	
    if(lowfreq < 0)lowfreq = 0 ; 
    if(hifreq < 0)hifreq = nyquist ; 
    if(hifreq > nyquist)hifreq = nyquist ; 

// CHECK FOR STUPIDTY OR JUST CARELESSNESS
    if( (hifreq >= nyquist) && (lowfreq <= 0) && (filttype == 1)){
	prt( "\n\n=====> YOUR BAND REJECT SETTING IS REJECTING EVERYTHING FROM 0Hz TO THE NYQUIST. BYE.\n" ) ; 
	exit(EXIT_FAILURE) ; 
    }else if(hifreq < lowfreq){
	prt( "\n\n=====> REVERSE YOUR  HI-LOW FILTER FREQUENCIES. BYE.\n" ) ; 
	exit(EXIT_FAILURE) ; 
    }else{
	
    }


//**** WRITE ASCII FILE, FILE SETUP
if( write_ascii != 0 ){
    // WE WILL BE WRITING A FILE

    // OPEN FILE
    // TEST FOR FILE NAME
    if(  (strlen( write_ascii_filename ) == 0) ){
	// NO FILE SPECIFIED
	fprintf( stderr, "\n\n\tPLEASE SUPPLY AN OUTPUT FILE NAME FOR THE TERMINAL DISPLAY ASCII WRITE.\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
    }
	
	sprintf( scratch, "/tmp/%s.ascii.out", user ) ;
	tdata  = fopen( scratch, "w") ;

}


    
//***************** PRINT VALUES

prf( dur, "OUTPUT FILE: DURATION" ) ; 

prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
pri( window_type, "WINDOW TYPE INDEX" ) ; 

//pri( R,  "SAMPLE RATE" ) ; 

pri( frames_per_sec,  "FRAMES/SECOND" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 

prbanner( "RESYNTHESIS PARAMETERS",  69 ) ; 

prf( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
prline( 1,  "*" ) ; 
pri( I,  "      INTERPOLATION SAMPLES (samples between resynthesis frames)" ) ; 
prline( 1,  "*" ) ; 
prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (in dB)" ) ; 
prline( 1,  "*" ) ; 
prp( &dBgain,  "GAIN (in dB)"  ) ; 
prp( &ptrans,  "PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &harmadd,  "FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
prp( &attack,  "AMPLITUDE ENVELOPE ATTACK TIME (in seconds)"  ) ; 
prp( &release,  "AMPLITUDE ENVELOPE RELEASE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prp( &warpshape, "SPECTRUM WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 
if( filttype == 0 )prt( "BRICKWALL FILTER SET TO BANDPASS" ); 
else prt( "BRICKWALL FILTER SET TO BAND REJECT" ) ; 
prf( lowfreq,  "FREQUENCY WINDOW: LOW BOUNDARY" ) ; 
prf( hifreq,  "FREQUENCY WINDOW: HIGH BOUNDARY" ) ; 
prline( 1,  "*" ) ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prp( &freqlow, "LOW SHELF FREQUENCY" ) ; 
prp( &dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prp( &freqhi, "HIGH SHELF FREQUENCY" ) ; 
prp( &dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 

prf( rescalev, "DECIBEL RESCALE VALUE" ) ; 


    // *******

// SET UP ARRAYS

    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */
    fvec( previous_channel, N+2 ) ;	/* previous analysis channels */


    fvec( binfreq, N2 ) ;	/* bin frequencies */

    fvec( channel_freqdev,  N + 2 ) ;	// channel SORT ARRAY ACCUMULATOR

    
    // SET UP BIN FREQUENCIES
    for( i = 0; i < N2;  i++ ) binfreq[i] = (float) i * fundamental ; 

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
	phaselock( channel, N2 ) ; 


	// SETUP PREVIOUS CHANNEL
	if( !frame_count )for(i = 0; i < N; i++)
		    previous_channel[ i ] = channel[ i ] ; 
	// SETUP FREQDEV HISTORY ARRAY
	for( i = 1; i < N; i+= 2 ) {
	    channel_freqdev[ i - 1 ] = 0. ; channel_freqdev[ i ] = 1. ; 
	} ; 


    //*************************
    // GET THE VALUES
    //*************************


            harmadd.A[ 0 ] = fval( &harmadd, dur, t );
            dBgain.A[ 0 ] = fval( &dBgain, dur, t );
            gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
            ptrans.A[ 0 ] = fval( &ptrans, dur, t ) ;
            pm = semitones_to_mult( ptrans.A[ 0 ] ) ;

            attack.A[ 0 ] = fval( &attack, dur, t );
            smooth_setup( attack.A[ 0 ], &attackc, &minusattackc, IR ) ; 
            release.A[ 0 ] = fval( &release, dur, t );
            smooth_setup( release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 

            warpshape.A[ 0 ] =  fval( &warpshape, dur, t );

            dBlow.A[ 0 ] =  fval( &dBlow, dur, t );
            dBhi.A[ 0 ] =  fval( &dBhi, dur, t );
            freqlow.A[ 0 ] =  fval( &freqlow, dur, t );
            freqhi.A[ 0 ] =  fval( &freqhi, dur, t );

            //*************************
		
             // SMOOTH THE CHANGES TO THE SPECTRUM
            smooth( channel, previous_channel, (N + 2), attackc, minusattackc, releasec, minusreleasec ) ; 


            // CHANGE THE AMPLITUDES 

            for( i = 1, j = 0; i < N; i+= 2, j++ ){
		     
                // ADD THIS MULTIPLIER TO THE STORE MULTIPLIERS
		channel_freqdev[ i ] *= pm ; 
		// ADD THIS ADDER TO THE STORE ADDERS
		channel_freqdev[ i - 1 ] += harmadd.A[ 0 ]  ; 
		    
		    
		// NEUTOR OUT OF BOUNDS FREQ BINS
		temp = pm  * (harmadd.A[ 0 ]  + channel[i]) ;
		    
                if(filttype == 0)
                {
                    if((temp > 0.) 
			&& (temp < nyquist) 
				&& (binfreq[j] >= lowfreq) 
					&& (binfreq[j] <= hifreq) ) 
					    channel[i] = temp ;
		else 
                    channel[i - 1] = 0. ;  
		}
                else
                {
		    if(
                         (temp > 0.) 
			   && (temp < nyquist) 
				&& ((binfreq[j] < lowfreq) 
					|| (binfreq[j] > hifreq)) 
                    ) 
                       channel[i] = temp ;
		    else 
                        channel[i - 1] = 0. ;  
                }

            }

            // ADD GAIN
            for( i = 1; i < N; i+= 2 ) channel[i - 1] = channel[i - 1] * gain ;

            //*************WARP THE INPUT SPECTRUM
            spectmagwarp( channel,  (N + 2), warpshape.A[ 0 ], 0 ) ;		

            //*************EQUALIZE THE OUTPUT SPECTRUM
            eq2( channel,  (N + 2),  dBlow.A[0],  dBhi.A[0], freqlow.A[0],  freqhi.A[0],  
			fundamental, channel_freqdev, 0 ) ; 
		
		
if( showme && (n < numberframes) ){
    
    fprintf( stderr, "\n****:FRAME %d\n",  n ) ; 


    i = 0 ; 

    for(j = 1 + (lowbin * 2),  k = lowbin; j <  2 + (highbin * 2); j +=2,  k++ ){


 if( i == 0 ){

    fprintf( stderr, "\n------" ) ; 

    if( (showme == 1) || (showme == 3) )
	fprintf( stderr, 
	    "----------------------------------------" ) ;   

    if( (showme == 2) || (showme == 3) )
	fprintf( stderr,"------------------------" ) ; 

    fprintf( stderr, "\nBIN |" ) ; 

    if( (showme == 1) || (showme == 3) )
	fprintf( stderr, 
	    " BIN FREQ | FREQ     |PHASE DIFF.(rad.)|" ) ;   
         
    if( (showme == 2) || (showme == 3) )
	fprintf( stderr," AMPLITUDE | in DB     |" ) ; 

    fprintf( stderr, "|" ) ; 



    fprintf( stderr, "\n------" ) ; 

    if( (showme == 1) || (showme == 3) )
	fprintf( stderr, 
	    "----------------------------------------" ) ;   

    if( (showme == 2) || (showme == 3) )
	fprintf( stderr,"------------------------" ) ; 

  }

    i++ ; 
    if( i == 20 ) i = 0 ; 



	fprintf( stderr, "\n%-4d|",  k ) ; 
 
	if( (showme == 1) || (showme == 3) )
	    fprintf( stderr, " %-9.1f| %-9.1f| %-16.4f|", 
		fundamental * (float) k,  channel[j],  
		    (channel[j] -  (fundamental * (float) k)) / factor ) ; 


	if( (showme == 2) || (showme == 3) ){

	    fprintf( stderr, " %-10.7f| %-10.2f|", 
		ampfactor * channel[j - 1], 
		(float) (20. * log10( (double) (ampfactor * channel[j - 1] )))  );
	}

	fprintf( stderr, "|" ) ; 



    }




    n++ ; 
}

//**************
  
  
    
if( (write_ascii != 0) && (nnn < numberframes) ){
    
    numbins = 0 ; 
    for(j = 1 + (lowbin * 2),  k = lowbin; j <  2 + (highbin * 2); j +=2,  k++ ){

	numbins++ ; 

	if( write_ascii == 1 ){
	    // FREQ


	    temp = (float) k ; 
	    fwrite( &temp, sizeof(float) , 1, tdata ) ; 
	    fwrite( &channel[j], sizeof(float) , 1, tdata ) ; 


	}else if( (write_ascii == 2) || (write_ascii == 3) ){
	    // DB

	    temp = (float) k ; 
	    fwrite( &temp, sizeof(float) , 1, tdata ) ; 
	    temp = (float) (20. * log10( (double) (ampfactor * channel[j - 1] ))) ;
	    fwrite( &temp, sizeof(float) , 1, tdata ) ; 

	}else{
	    // NO WRITE
	}
    }

    nnn++ ; 
}



//**************



        synt = getthresh( channel, N, threshfac );
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


// SORT WRITE ASCII FILE
    if( write_ascii != 0 ){


prline( 69,  "=" ) ; 
prt( "GRAPH FILE PARAMETERS:" ) ; 
prf( highbinfreq,  "HIGH GRAPH/DISPLAY BIN FREQUENCY " ) ; 
prf( lowbinfreq,  "LOW GRAPH/DISPLAY BIN FREQUENCY " ) ; 
pri( highbin,  "HIGH BIN" ) ; 
pri( lowbin,  "LOW BIN" ) ; 
pri( numbins,  "NUMBER OF BINS" ) ; 
pri( nnn,  "NUMBER OF FRAMES" ) ; 
prline( 69,  "=" ) ; 

prt( "******** SORTING GRAPH FILE VALUES.............." ) ; 
prt( "\n...............DONE." ) ; 
	fclose( tdata ) ; 
	// SORT IT
	tdata  = fopen( scratch, "r") ;
	// MAKE OUT FILE
	write_ascii_d  = fopen( write_ascii_filename, "w") ;
	
	// LOOP FOR GRAPHS
	for( i = 0, l = lowbin;  i < numbins; i++,  l++ ){

	    if( i > 0 ) 
		fseek( tdata,  2 * i * sizeof(float),  SEEK_SET ) ; 

	    offset = (float) nnn * ((float) (numbins - i) / (float)  numbins ) ;  
	    
	    // LOOP FOR POINTS OF A GRAPH
	    for( n = 0; n < nnn; n++ ){
		// GO THROUGH THE FRAMES
		// READ IN A PAIR
		fread( &temp1, sizeof(float) , 1, tdata ) ; 
		fread( &temp2, sizeof(float) , 1, tdata ) ; 


		if(write_ascii == 3 ){
		    // DB WATERFALL PLOT
		    m = n + offset ;
		    if( temp2 < -150.) temp2 = -150. ; 
		    temp2 = temp2 +  96. + ((float) i * fundamental ) ;   
		}else
		    m = n ; 

		 
		// WRITE IT OUT
		fprintf( write_ascii_d,  "\n%f %f", (float) m,  temp2 ) ; 

		// SKIP TO THE NEXT ONE
		fseek( tdata,  2 * (numbins - 1) * sizeof(float),  SEEK_CUR ) ; 

	    }

	    // BLANK LINE TO BREAK GNUPLOT GRAPH
	    fprintf( write_ascii_d,  "\n" ) ; 

	    
	}


 
    }




    // CLOSE  INPUT FILE
    if(ifd)fclose(ifd);  


   if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
   if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
   if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
   if( release.n != 1. ) fclose(release.fp ) ;
   if( attack.n != 1. ) fclose(attack.fp ) ;
   if( warpshape.n != 1. ) fclose(warpshape.fp ) ;
    if( dBlow.n != 1. ) fclose(dBlow.fp ) ;
    if( dBhi.n != 1. ) fclose(dBhi.fp ) ;
    if( freqlow.n != 1. ) fclose(freqlow.fp ) ;
    if( freqhi.n != 1. ) fclose(freqhi.fp ) ;

    if( write_ascii != 0 ){
	fprintf( write_ascii_d,  "\n" ) ; 
	fclose( write_ascii_d ) ; 
	fclose( tdata ) ; 
	snprintf( scratch2, sizeof(scratch2),  "rm %s",  scratch ) ;
	system( scratch2 ) ;


    }

     
    fprintf(stderr,"\nPLAINPV: RESYNTHESIS COMPLETED\n");
    exit(EXIT_SUCCESS) ;
}

void usage()
{
    fprintf(stderr, "%s",
	"plainpv:  basic phase vocoder  \n"
	"plainpv   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	N:	"FFT_LENGTH 		// N
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec
	"	I:	"TIME_FACTOR		// tfactor

	"	P:	"PITCH_TRANS		// ptrans
	"	a:	"FREQ_SHIFT		// harmadd

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout

	"	     "SHELF_EQ_HEADER
	"	H:	"SHELF_EQ_LOW_GAIN		// dBlow
	"	X:	"SHELF_EQ_HIGH_GAIN		// dBhi
	"	m:	"SHELF_EQ_LOW_FREQ		// freqlow
	"	R:	"SHELF_EQ_HIGH_FREQ		// freqhi


	"	W:	"WARP_INDEX		// warpshape


	"	A:	"DB_GAIN			// dBgain
	"	l:	"AMP_ATT_TIME		// attack
	"	L:	"AMP_RELEASE_TIME		// release
	"	T:	brickwall filter type: 0 = bandpass, not 0 = band reject [0]\n"
	"	f:	frequency window: low boundary  \n"
	"		    (before -P and -a) (in Hz) [0.] \n"
	"	F:	frequency window: high boundary \n"
	"		    (before -P and -a)(in Hz) [Nyquist frequency] \n"

	"	t:	"RESYNTH_THRESHOLD		// threshfacdB
	"	p:	"AMP_REPORTS		// quiet 
	"	i:	"AMP_REPORTS_TIME_INTERVAL	// ampstatinc 

	"	_:	 "AUTO_PLAY		// autoplayreps

	"	=:	 "RESCALE_LEVEL		// rescalev

	"		TERMINAL DISPLAY AND GRAPH FILE OUTPUT\n"
	"	n:	    number of frames  [0]\n"
	"	u:	    low bin frequency  [-1]\n"
	"	U:	    high bin frequency  \n"
	"		    (-1 = nyquist) [Nyquist frequency]\n"
	"	S:	TERMINAL DISPLAY: display option  [0]\n"
	"		  (0 = off,  1 = phase data,  2 = amp data, 3 = both)\n"
	"	c:	GRAPH FILE: WRITE ascii to FILE\n"
	"		    0 = off,  1 = freq,  2 = decibels [0]\n"
	"		    3 = decibels - waterfall plot\n"
	"		    (When on,  this flag writes ascii point pairs\n"
	"		     (with time frame on x axis) for plotting \n"
	"		      with gnuplot.)\n"
	"	d:    	TERMINAL DISPLAY FILE NAME for -c [./ascii.out]\n" 	    

	);
    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
