#include "globals.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

void usage() ; 
void pd( int i ) ;


int main( argc, argv )
    int argc ; char *argv[] ;
{
int i, kk, kl=0,  mm ;
float nyquist,  fundamental;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 220, I = 220, in, on;
int   eof = 0, channelout=0,  chanmethod=0,  obank=0 ;
 float P = 1.0;
  FILE *fopen(),  *fofd ;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output, 
    *previous_channel ;
float peakamps[MAXIMUM_CHANNELS]
 ;
float  temp,  temp2,  temp3, temp4,    old_temp=0.,  shortnorm ;  
float getthresh();
int exflag=0 ; 
 
float peakenvamp=0., ampthresh,ampgatethresh;
float   IR,  dur=0.;
char tempstring[ STRING_SIZE ],  *user ; 
FILE *fscratch ; 
char scratch[ STRING_SIZE ] ; 
float compression,  tp,  tpinc=500.,  freqdiff,  lowf,  hif ; 
float releasec,  minusreleasec,  attackc,  
    minusattackc; 
double ar_dB ;
int datatype=0 ;  
float frametprop,  tempt=0.,  midC ;
int qseccount=0,  seccount=0,  printflag=0, plotflag=0 ;
int outtype=0 ; 
 

int flux_weight_flag=1 ; 

float get_flux_stats(
    float flux_max_sum[], 
    float A[], 
    float old_A[], 
    int N, 
    float lowf, 
    float hif, 
    float fundamental, 
    float *old_value

 ) ;

 
float *flux_max_sum ; 

int outformat=0 ; 

float minamp ; 

// LOW FREQUENCY BOUND
struct  func  lowfreq ; 

//  HIGH FREQUENCY BOUND
struct  func  hifreq ; 
int hifreqUserSet = 0 ; // set in case 'F': tracks whether the user overrode the default (nyquist) high-frequency bound


//  BIN AMP ENVELOPE RELEASE TIME
struct  func  release ; 

//  BIN AMP ENVELOPE RELEASE  TIME
struct  func  attack ; 

//  COMPRESSION
struct  func  compression_dB ; 

//  dB COMPRESSION THRESHOLD
struct  func  dBthreshold ; 

//  dB GATE THRESHOLD
struct  func  dBgate ; 

//  WARP
struct  func  warp ; 


//#include "underflow.h"
	    
//*****************INITIALIZE
// LOW FREQUENCY BOUND
lowfreq.L = 1. ; lowfreq.n = 1. ; lowfreq.A[ 0 ] = 0. ; 

//  HIGH FREQUENCY BOUND
hifreq.L = 1. ; hifreq.n = 1. ; hifreq.A[ 0 ] = 0. ; ; // real default (nyquist) is not yet known here; set below once R is read


//  BIN AMP ENVELOPE RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  BIN AMP ENVELOPE ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 

//  DECIBELS OF COMPRESSION
compression_dB.L = 1. ; compression_dB.n = 1. ; compression_dB.A[ 0 ] = 1. ; 

//  COMPRESSION THRESHOLD IN DECIBELS
dBthreshold.L = 1. ; dBthreshold.n = 1. ; dBthreshold.A[ 0 ] = 0. ; 

//  GATE THRESHOLD IN DECIBELS
dBgate.L = 1. ; dBgate.n = 1. ; dBgate.A[ 0 ] = -96. ; 

//  WARP
warp.L = 1. ; warp.n = 1. ; warp.A[ 0 ] = 0. ; 



if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, "R|w|N|M|P|q|D|g|t|I|b|Q|e|p|G|Z|X|S|f|F|L|l|d|s|a|A|C|T|W|n|B|X|r|p|Q|z|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = (int) crackfloat( arg_option, ch );
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'X':   chanmethod = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'r':	tpinc = crackfloat( arg_option, ch );
			break;
	    case 'g':	outtype = (int) crackfloat( arg_option, ch );
			break;

	    case 'q':	outformat = (int) crackfloat( arg_option, ch );
			break;

	    case 'Q':	datatype = (int) crackfloat( arg_option, ch );
			break;
	    case 'f':   strcpy(tempstring, arg_option);
			lowfreq.fp = crackstring( tempstring, 
			    &lowfreq );
			break;
	    case 'F':   strcpy(tempstring, arg_option);
			hifreq.fp = crackstring( tempstring, 
			    &hifreq );
			hifreqUserSet = 1 ;
			break;

	    case 'G':   strcpy(tempstring, arg_option);
			compression_dB.fp = crackstring( tempstring, 
			    &compression_dB );
			break;
	    case 'T':   strcpy(tempstring, arg_option);
			dBthreshold.fp = crackstring( tempstring, 
			    &dBthreshold );
			break;
	    case 'S':   strcpy(tempstring, arg_option);
			dBgate.fp = crackstring( tempstring, 
			    &dBgate );
			break;
	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;
	    case 'W':   strcpy(tempstring, arg_option);
			warp.fp = crackstring( tempstring, 
			    &warp );
			break;

	    case 'A':   flux_weight_flag = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'p':   printflag = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'P':   plotflag = (int) crackfloat( arg_option, ch ) ;
			break;

	}
    }
 
 
 //SET SOUNDFILE OUTPUT TO INTEGERS
// outputformat = 1 ; 
 
prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "SPECTRAL FLUCTUATION TRACKER", 69 ) ; 
prline( 69,  "-" ) ; 
    
    minamp = pow( 10., (double) (-96. / 20.) ) ;
 
    if(channelout == 0){
	channelflag = -2 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }

if( (outtype == 0) ||  (outtype == 1)){
    // SET NO OUTPUT FLAG
    outputoff=1;
}else{
    fprintf( stderr,"\n\nUNKNOWN OUTPUT DATA TYPE!\n\n") ; exit(EXIT_FAILURE) ;  
}

    setupfiles(argc, argv) ; 
    endchan = beginchan + ochan ; 
/******
 
 
*******/
   
    // GET NAME OF OUTPUT FILE
    arg_index++ ; 
    if( arg_index >= argc  ){
	bannero() ;
	sprintf( ofile, "envelope.out" ) ; 
	fprintf( stderr, "\n\n.....().USING DEFAULT OUTPUT FILENAME........\n\n" ) ;

	fprintf( stderr,  "\n\nOUTPUT FILE: %s\n",  ofile ) ; 
    }else{
    // GET OUTPUT FILE NAME
	strcpy( ofile, argv[arg_index] ) ; 
    }	 
    // ASCII OR HEADERLESS FLOAT OUTPUT
    fofd = fopen( ofile,  "w" ) ;     


// **** SET UPS *****
    R = isr ; // SAMPLE RATE EQUALS INPUT FILE
    if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 200.\n\n" ) ; 
	frames_per_sec = 200 ; 
    }
    I = D = (int) ((float) R / frames_per_sec) ; 
//*****
//******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
//*********************************

 if( tpinc < frames_per_sec ){
     // CHANGE IT
     prt( ".....YOUR OUTPUT ENVELOPE SAMPLE RATE IS < THE FRAMES PER SECOND." ) ; 
     prf( frames_per_sec,  " WILL CHANGE TO THE FRAMES PER SECOND RATE " ) ;
     tpinc =  1.  ; 
 }else{
     prf( tpinc, "OUTPUT ENVELOPE SAMPLES PER SECOND" ) ; 
     tpinc = 1. / (tpinc  / frames_per_sec)  ; 
 }
if( tpinc > 1. ){
    fprintf( stderr,  "\n\nSAMPLES PER OUTPUT FRAME MUST BE >= 1. BYE.\n" ) ;
    exit(EXIT_FAILURE);
}
// MAKE /tmp ENVELOPE SCRATCH SPACE
// MAKE UNIQUE NAME
	user = getlogin(); 

    sprintf( scratch, "/tmp/%s.envelope", user ) ;
// OPEN IT
    fscratch = fopen( scratch, "w" ); 
	filesToRemove( scratch, 0 ) ;


    IR = (float) I / (float) R ; 
    
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    obank = P != 0. ;
    if( P == 0.0 ) {P = 1.0;}
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    freqdiff = (float) R / (float) N  ;
    nyquist = (float) R / 2.0;
if( !hifreqUserSet ) hifreq.A[ 0 ] = nyquist ;
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    fundamental =  ((float) R / (float) N) ; 
    frametprop = (float) D / (float) R ; 
    midC = (220.*pow(2., (3./12.))) ; 

    // COMPUTE THE DURATION
    dur = (endt - begint) ; 

//***************** PRINT VALUES
prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
prline( 1,  "*" ) ; 
pri( frames_per_sec,  "FRAMES/SECOND" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
prline( 1,  "*" ) ; 
pri( datatype, " BOUNDARY DATA TYPE" ) ; 
if( !datatype ){
    prp( &lowfreq,  "LOW FREQUENCY  BOUNDARY" ) ;  
    prp( &hifreq,  "HIGH FREQUENCY  BOUNDARY" ) ;  
}else{
    prp( &lowfreq,  "LOW OCTAVE.PITCHCLASS  BOUNDARY" ) ;  
    prp( &hifreq,  "HIGH OCTAVE.PITCHCLASS  BOUNDARY" ) ;  
}

prline( 1,  "*" ) ; 
prp( &release,  "ENVELOPE RELEASE TIME" ) ;  
prp( &attack,  "ENVELOPE ATTACK TIME" ) ;  
prline( 1,  "*" ) ; 
prp( &dBthreshold,  "COMPRESSION THRESHOLD (in decibels)" ) ;  
prp( &compression_dB,  "AMOUNT OF COMPRESSION (in decibels)" ) ;  
prp( &dBgate,  "GATE THRESHOLD (in decibels)" ) ;  
prline( 1,  "*" ) ; 

prp( &warp,  "WARP SHAPE INDEX" ) ;  
	    
if( outtype == 0 )
    prt("OUTPUT WILL BE ASCII............" ) ;
else  if( outtype == 1 )
    prt("OUTPUT WILL BE FLOATS............" ) ;
//	else if( outtype == 2 )
//    	prt("OUTPUT WILL BE NEXT SOUNDFILE............" ) ;
else
    {
    prt("NOT A RECOGNIZED OUTPUT FORMAT FLAG. BYE!\n\n" ) ; exit(EXIT_FAILURE); 
    }
if( ochan > 1 ){
    if( !chanmethod )prt(".................USING AVERAGE METHOD" ) ;
    else prt(".................USING PEAK METHOD" ) ;
}

// *******

    if (Nw == 0) Nw = N;

    if (I == 0) I = D;


// *******

    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */

   fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

   fvec( flux_max_sum, N+2 ) ;	/* previous analysis channels */



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
    tempt = 0. ; qseccount=0 ;  seccount=0 ; 
    peakamps[channow] = 0 ; 
    rewind( fscratch  ); 

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

//*************************
// GET THE VALUES
//*************************
		lowfreq.A[ 0 ] = fval( &lowfreq, dur, t );
		hifreq.A[ 0 ] = fval( &hifreq, dur, t );

		release.A[ 0 ] = fval( &release, dur, t );
		// RECOMPUTE FOR INTERPOLATION
		if(release.A[ 0 ] <= 0.){
		    releasec = 0. ; minusreleasec = 1. ; 
		}else{
		    // LOWPASS
		    releasec = 
			pow( (double) ar_dB,  
			    (double) (IR / release.A[ 0 ] ) ) ; 
		    minusreleasec = 1.  - releasec ; 
		}

		attack.A[ 0 ] = fval( &attack, dur, t );
		if(attack.A[ 0 ] <= 0.){
		    attackc = 0. ;  minusattackc = 1. ;
		}else{
		    // LOWPASS
		    attackc = 
			pow( (double) ar_dB,  
			    (double) (IR / attack.A[ 0 ] ) ) ; 
		    minusattackc = 1.  - attackc ; 
		}


//*************************


// TURN PITCH BOUNDS INTO FREQ

	    if( datatype ){
		// OCTAVE.PITCHCLASS
		if( fabs( (double) lowfreq.A[ 0 ] ) > 15. ){
		    // QUESTIONABLE
		    prf( lowfreq.A[ 0 ],  "\n\n ====> QUESTIONABLE OCTAVE.PITCHCLASS LOW BOUNDARY" ) ; 
		    prt( "QUESTIONABLE DATA.............................." ) ; 
		    exflag = 1 ;  
		} 
		if( fabs( (double) hifreq.A[ 0 ] ) > 15. ){
		    // QUESTIONABLE
		    prf( hifreq.A[ 0 ],  "\n\n ====> QUESTIONABLE OCTAVE.PITCHCLASS HIGH BOUNDARY" ) ; 
		    prt( "QUESTIONABLE DATA.............................." ) ; 
		    exflag = 1 ;  
		} 
		if( exflag ){
		    prt( "\n\nIS YOUR DATA TYPE FLAG CORRECT?\n\n.......CHECK YOU DATA.\n\n" ) ; 
		    exit(EXIT_FAILURE) ; 
		}
		// TURN INTO FREQ
		if( lowfreq.A[ 0 ] < 3.){
		    lowf = 0;
		}else{
		    lowf = (float) ((int) lowfreq.A[ 0 ] ) ; // INTEGER PART
		    lowf =  (( 12. * (lowf - 8.)) + 
			(100. * (lowfreq.A[ 0 ] - lowf)) ) / 12. ;
		    lowf = midC * pow( 2., (double) lowf ) ; 
		}

		if( hifreq.A[ 0 ] < 3.){
		    hif = nyquist;
		}else{
		    hif = (float) ((int) hifreq.A[ 0 ] ) ; // INTEGER PART
		    hif =  (( 12. * (hif - 8.)) + 
			(100. * (hifreq.A[ 0 ] - hif)) ) / 12. ;
		    hif = midC * pow( 2., (double) hif ) ; 
		}


	    }else{
		// FREQ
		if( (hifreq.A[ 0 ] < 15.) ){
		    prt("\n\n ====> QUESTIONABLE FREQUENCY  BOUNDARIES\n\n" ) ; 
		    exflag = 1 ;  
		}else{
		    lowf = lowfreq.A[ 0 ]; 
		    hif = hifreq.A[ 0 ] ; 
		}
	    }

		if( exflag ){
		    prt( "\n\nIS YOUR DATA TYPE FLAG CORRECT?\n\n.......CHECK YOU DATA.\n\n" ) ; 
		    exit(EXIT_FAILURE) ; 
		}

//*****************
// MODIFICATIONS LOOP
//*****************

		if( frame_count == 0 ) 
		    for( i = 0 ; i < (N + 2) ; i++ ) 
			previous_channel[ i ] = channel[ i ] ;

		// GET THE SPECTRAL FLUX VALUE
		temp = 
		    find_fluxoid( channel, previous_channel,  
			(N + 2), lowf, hif, fundamental, &old_temp,  flux_weight_flag ) ;  

		if( frame_count == 0 ) old_temp = temp ; 
		

		// COLLECT FLUX STATS
		get_flux_stats( flux_max_sum,  channel, previous_channel,  
			N, lowf, hif, fundamental, &old_temp ) ;

		// SAVE PREVIOUS
		for( i = 0 ; i < (N + 2) ; i++ ) 
		    previous_channel[ i ] = channel[ i ] ; 
		
		
		//SMOOTH THE FLUX VALUE
		if(temp > old_temp){ //ATTACK
		    temp = (attackc * old_temp) +
			(minusattackc * temp) ; 
		} else { // DECAY
		    temp = (releasec * old_temp) +
		    (minusreleasec * temp) ; 
		}
		
		
		old_temp = temp ; 
		
		// FIND PEAK AMP
		if( peakenvamp < temp ) peakenvamp = temp ; 

// OUTPUT THE AMPLITUDE 
	    if( channow == 0 ){
    		// WRITE FIRST CHANNEL
		fwrite( &temp, sizeof(float), 1, fscratch );
	    }else{
		// READ IN VALUE
		fread( &temp2, sizeof(float) , 1, fscratch ) ; 

		if( !chanmethod ){ 
		    // TAKE AVERAGE
		    temp = ((( (float) channow / (float) (channow + 1) )
			* temp2 ) + temp ) / (float)  (channow + 1) ; 
		}else{
		    // TAKE PEAK
		    if( temp2 > temp ) temp = temp2 ; 
		}

		fwrite( &temp, sizeof(float), 1, fscratch );
	    }		
// *** 
// *** PASSIFIER PRINT
    if(printflag != 0){
	if(!frame_count)fprintf( stderr,  "\n\nELAPSED TIME (in secs): 0 " ) ; 

	tempt = tempt + frametprop ; 
	if(tempt > .25){
	    // PRINT PASSIFIER
	    qseccount++ ; 
	    while( tempt > .25 ) tempt -= .25 ; 
	    if( qseccount == 4){
		// SECOND
		seccount++ ;  qseccount = 0 ; 
		fprintf( stderr, " %d ",  seccount ) ; 
	    }else{
		// QUARTER SECOND
		fprintf( stderr,  "*" ) ; 
	    }
	    
	}
    }
// *** 
	frame_count++ ; 

// FRAMES LOOP END
    }
    
    fclose( fscratch ) ; 
    
// CHANNELS LOOP END
} 

//********************************* NOW OPEN, COMPRESS AND NORMALIZE
    

    fprintf(stderr," \nNORMALIZING, THE SIGNAL......" ) ;   
    ochan = 1 ; 
    fscratch = fopen( scratch, "r" ); 
    rewind( fscratch  ); 
    kk = 11025 ; kl = 0 ; 
    fprintf( stderr,  "\n\n" ) ; 
    shortnorm = 1. - (1./32760.) ;   
    eof = 0 ; mm = 0 ; 
    tp = 0 ; 
    while ( !eof  ){
	if( fread( &temp, sizeof(float) , 1, fscratch ) == 0 ){
	    eof = 1;
 	}else{

	    
	
	    t =  (float) frame_count * IR  ; 
	    compression_dB.A[ 0 ] = fval( &compression_dB, dur, t );
	    if( compression_dB.A[ 0 ] < 0. )
		compression = pow( (double) 10.0, (double) (compression_dB.A[ 0 ]/20.) );
	    else
		compression = 1. ; 

	    dBthreshold.A[ 0 ] = fval( &dBthreshold, dur, t );
		ampthresh = pow( (double) 10.0, (double) (dBthreshold.A[ 0 ]/20.) );	

	    dBgate.A[ 0 ] = fval( &dBgate, dur, t );
		ampgatethresh = pow( (double) 10.0, (double) (dBgate.A[ 0 ]/20.) );	

	    if( ampthresh <= ampgatethresh ){
		// TROUBLE
		prt( "\n\nYOUR GATE THRESHOLD IS BELOW YOUR COMPRESSION THRESHOLD.\n\n.....BYE.\n\n" ) ; 
		exit(EXIT_FAILURE) ; 
	    }

	    warp.A[ 0 ] = fval( &warp, dur, t );
// NORMALIZE
	    temp = temp / peakenvamp ; 

// TEST WHETHER ABOVE THRESHOLD
	    if( temp > ampthresh ){
		temp = (ampthresh + ((temp - ampthresh) * compression)) ; 
	    }
	    if( temp > ampgatethresh ){
		temp = temp - ampgatethresh ; 
	    }else{
		temp = 0 ; 
	    }
	    temp3 = 1. /  ((ampthresh  + ((1. - ampthresh) * compression) ) - ampgatethresh) ;
	    temp = temp * temp3 *  shortnorm ; 
// WARP
	    temp = curve( 0.,  1.,  temp,  warp.A[ 0 ]  ) ; 

       
    while( tp < 1. ){
	temp4 = curve( old_temp,  temp,  tp,  0.  ) ;
	if( temp4 >= 1. )temp4 = shortnorm ;  

	// CHANGE FORMAT, IF SOUNDFILE LEAVE ALONE
	    if( outformat == 1 ){
		// TO DECIBELS
		if( temp4 > minamp )
		    temp4 = 20. * log10( (double) temp4 ) ; 
		else
		    temp4 = -96. ; 
	    }else if(  outformat == 2 ){
		// INVERTED AMPLITUDE
		temp4 = 1. - temp4 ; 
	    }else if( outformat == 3 ){
		// TO INVERTED DECIBELS
		if( temp4 > minamp )
		    temp4 = 20. * log10( (double) temp4 ) ; 
		else
		    temp4 = -96. ; 
		// INVERT IT
		temp4 = 0. - (temp4 + 96.) ; 
	    }


	if( outtype == 0 ){
	    fprintf( fofd, "%f\n",  temp4 ) ;
	}else  if( outtype == 1 ){
	    fwrite( &temp4, sizeof(float), 1,   fofd ) ;
	} ; 

	mm++ ; 
        tp = tp + tpinc ;
	    // PASSIFIER PRINT
	    if( kk < 0 ){
		fprintf( stderr, " * " ) ; kk = 11025 ; 
	    }

	    kk-- ; 
    }
    old_temp = temp ; 
    tp = tp - (float) ((int) tp ) ; 
    frame_count++ ; 


    }
				
   }

    
    fclose( fscratch ) ;
    // CLOSE  INPUT  FILES
    if(ifd)fclose(ifd);  


    // CLOSE  OUTPUT  FILES
    fclose(fofd) ; 


	filesToRemove( NULL, 1 ) ;
    
    pri( mm, "NUMBER OF VALUES IN OUTPUT ENVELOPE" ) ; 

    fprintf(stderr,"\nFLUXOID : TRACKING COMPLETED\n");

    prt(""); 
    prs( ofile, "OUTPUT FILE" ) ; 
    prt(""); prt(""); prt("");

    if( plotflag == 1 ){
	sprintf( tempstring, "showme %s", ofile ) ;  
	system( tempstring ) ; 
    } ; 

    // PRINT FLUX STATS

    if( lowfreq.n != 1. ) fclose(lowfreq.fp ) ;
    if( hifreq.n != 1. ) fclose(hifreq.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;
    if( compression_dB.n != 1. ) fclose(compression_dB.fp ) ;
    if( dBthreshold.n != 1. ) fclose(dBthreshold.fp ) ;
    if( dBgate.n != 1. ) fclose(dBgate.fp ) ;
    if( warp.n != 1. ) fclose(warp.fp ) ;
    




    exit(EXIT_SUCCESS) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"fluxoid:  spectral fluctuation tracker (noise meter)\n"
	"(Fluxoid tracks the sum of the bin frequency change (absolute value)\n"
	"   weighted by the bin's amplitude.)\n"
	"fluxoid [flags] [input file] [output fluxoid file (optional)]\n"
	"	N:	FFT length (must be a power of 2) [1024]\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	w:	window type: 0 = hamming,  1 = rectangular  \n"
	"		    2 = Blackman,  3 = Bartlett triangular [0.]\n"
	"		    4-12 = Kaiser windows for alpha = 4-12,  respectively\n"
	"		    (representative sidelobe levels for alpha: \n"
	"		      4 = -30dB,  8 = -58 dB,  12 = -90 dB)\n"
	"	D:	analysis frames per second [200]\n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds (-1. = end of file) [0.] \n"
	"	C:	analysis channel (1 -> ?) (0 = average of all) [0] \n"
	"	X:	multiple channel method: [0] \n"
	"		    0 = average,  1 = peak\n"

	"	Q:	DETECTION BAND: data type [0]\n"
	"		    0 = freq,  1 = octave.pitchclass\n"
	"	f:	DETECTION  BAND: lower freq/pitch boundary (func) [0]\n"
	"	F:	DETECTION  BAND: upper freq/pitch boundary (func) [Nyquist freq]\n"
	"	A:	Amplitude weighting: 1 = on, 0 = off (func) [1]\n"
	"	    COMPRESSOR/GATE\n"
	"	T:	compression threshold level in decibels (0 to -96) (func) [0]\n"
	"	G:	decibels of compression \n"
	"		    (0 to -96) (0 = no compression)(func) [0(off)]\n"
	"	S:	gate threshold level in decibels (0 to -96) (func) [-96 (off)]\n"
	"	l:      envelope attack time  (func) [0.]\n"
	"	L:      envelope release time   (func) [0.]\n"
	"	W:	distribution warp index (post compression) [0]\n"  
        "		    value of 0: no warp\n"
        "		    values > 0 warp the distribution downward\n"
        "		    values < 0 warp the distribution upward\n"
	"	p:	print elapsed time (1 = on,  0 = off) [0]\n"
	"	r:	output samples per second (interpolated) for non-soundfile data types [500.]\n"
	"	g:	output data type 0 = ascii,  1 = floats [0]\n"
	"	q:	output data format [0] \n"
	"		    0 = 0 to 1 amplitude scale, \n"
	"		    1 = -96 to -0 decibel scale\n"
	"		    2 = inverted amplitude scale (1. - amplitude) \n"
	"		    3 = inverted  decibel scale (0. -  (dB + 96.))\n"
	"	P:	plot output 0 = off,  1 = on [0]\n"

	);
    exit(EXIT_SUCCESS);
}



void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }


float get_flux_stats(
    float flux_max_sum[], 
    float A[], 
    float old_A[], 
    int N, 
    float lowf, 
    float hif, 
    float fundamental, 
    float *old_value

 ){
    
    static int first=1 ; 
    
    int i,  i1,  i2 ; 
    float temp2; 

    if( first ) for( i = 0; i < (N + 2) ; i += 2 ){
		    flux_max_sum[ i ] = -1 ;
		    flux_max_sum[ i + 1 ] = 0. ;
		}


    // TURN FREQ BOUNDS INTO INDECES


    i1 = 1 +  ( 2 * (int) ( lowf / fundamental ) ) ; // LOW FREQ
    i2 = 1 +  ( 2 * (int) ( hif / fundamental ) ) ; // HI FREQ
    if( i2 >= N ) i2 = N - 1 ; 
		
    if( i2 == i1 ) i2 = i1 + 2 ; 

    for( i = i1; i <= i2; i+= 2){ 

	    temp2 =  fabs( A[i]  - old_A[i] ) ;
	    flux_max_sum[ i ] += temp2 ; 
	    if( temp2 > flux_max_sum[ i - 1 ] ) flux_max_sum[ i - 1 ] = temp2 ; 
    }
	
	
    return( 1 ) ; 


}

