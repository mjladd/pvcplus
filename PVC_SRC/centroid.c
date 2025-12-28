#include "globals.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,  k, kk, kl=0,  i1,  i2,  mm ;
float nyquist,  fundamental;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 220, I = 220, in, on;
int   eof = 0, channelout=0,  chanmethod=0,  obank=0 ;
 float P = 1.0;
  FILE *fopen(), *fp,  *fofd ;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float	gain=1., peakamps[MAXIMUM_CHANNELS]
 ;
float  temp,  temp2,  temp3, temp4,    old_temp=0. ;  
float getthresh();
int exflag=0 ; 
 
float peakbinamp = 0.,  avgbinamp=0.,  peakamp, peakenvamp=0., ampthresh,ampgatethresh,    limit ;
float   IR,  dur=0.;
char tempstring[ STRING_SIZE ] ; 
FILE *fscratch ; 
char scratch[ STRING_SIZE ] ; 
float OR=44100.,  tp,  tpinc=500.,  freqdiff,  lowf,  hif,  difff ; 
float releasec,  minusreleasec,  attackc,  
    minusattackc,  freqchangec,  minusfreqchangec ; 
double ar_dB ;
int datatype=0 ;  
float frametprop,  tempt=0.,  midC,  log_of_2 ;
int qseccount=0,  seccount=0,  printflag=0, plotflag=0 ;
int outtype=0 ; 
float snfloats[ 1000 ] ; 
float refoctave,  refpitch=8.0 ; 



int outformat=0 ; 

float minamp ; 

// LOW FREQUENCY BOUND
struct  func  lowfreq ; 

//  HIGH FREQUENCY BOUND
struct  func  hifreq ; 


//  BIN AMP ENVELOPE RELEASE TIME
struct  func  release ; 

//  BIN AMP ENVELOPE RELEASE  TIME
struct  func  attack ; 

//  SPECTRUM WARPSHAPE
struct  func  swarpshape ; 


//  WARP
struct  func  warp ; 

//#include "underflow.h"
	    
//*****************INITIALIZE
// LOW FREQUENCY BOUND
lowfreq.L = 1. ; lowfreq.n = 1. ; lowfreq.A[ 0 ] = 0. ; 

//  HIGH FREQUENCY BOUND
hifreq.L = 1. ; hifreq.n = 1. ; hifreq.A[ 0 ] = nyquist ; ; 


//  BIN AMP ENVELOPE RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  BIN AMP ENVELOPE ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 

//  WARP
warp.L = 1. ; warp.n = 1. ; warp.A[ 0 ] = 0. ; 

//  SPECTRUM WARPSHAPE
swarpshape.L = 1. ; swarpshape.n = 1. ; swarpshape.A[ 0 ] = 0. ; 


strcpy( routine, "centroid" ) ; 


if( argc < 2 )usage() ; 

// P -> ? Q -> ? 


    while( (ch= crack( argc, argv, "R|N|M|P|q|w|S|D|g|t|I|b|Q|e|H|p|G|Z|X|S|f|F|L|l|d|s|a|A|C|T|W|n|B|X|r|p|Q|z|h", 0  )) != CRACK_DONE_FLAG ) {
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
	    case 'H':   strcpy(tempstring, arg_option);
			swarpshape.fp = crackstring( tempstring, 
			    &swarpshape );
			break;

	    case 'r':	tpinc = crackfloat( arg_option, ch );
			break;
	    case 'g':	outtype = (int) crackfloat( arg_option, ch );
			break;

	    case 'q':	outformat = (int) crackfloat( arg_option, ch );
			break;

	    case 'Q':	datatype = (int) crackfloat( arg_option, ch );
			break;
	    case 'G':	refpitch = crackfloat( arg_option, ch );
			break;
	    case 'f':   strcpy(tempstring, arg_option);
			lowfreq.fp = crackstring( tempstring, 
			    &lowfreq );
			break;
	    case 'F':   strcpy(tempstring, arg_option);
			hifreq.fp = crackstring( tempstring, 
			    &hifreq );
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
	    case 'p':   printflag = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'P':   plotflag = (int) crackfloat( arg_option, ch ) ;
			break;

	}
    }
 
 
 //SET SOUNDFILE OUTPUT TO INTEGERS
 //outputformat = 1 ; 
 
prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "CENTROID ENVELOPE TRACKER", 69 ) ; 
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
} else{
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
	sprintf( ofile, "centroid.out" ) ; 
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
    	sprintf( scratch, "/tmp/envelope.%d", (int)(random()) ) ;
	filesToRemove( scratch, 0 ) ;
// OPEN IT
    fscratch = fopen( scratch, "w" ); 

    IR = (float) I / (float) R ; 
    
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    obank = P != 0. ;
    if( P == 0.0 ) {P = 1.0;}
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    freqdiff = (float) R / (float) N  ;
    nyquist = (float) R / 2.0;
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    fundamental =  (float) R / (float) N  ; 
    frametprop = (float) D / (float) R ; 
    midC = (220.*pow(2., (3./12.))) ; 

    temp = (float)((int) refpitch ) ; 
    refoctave = temp + ((refpitch - temp) / .12) ; 
        
    log_of_2 = log10f( 2.0 ) ; 

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

prp( &warp,  "WARP SHAPE INDEX" ) ;  
prp( &swarpshape,  "SPECTRUM WARP SHAPE INDEX" ) ;  
	    
if( outtype == 0 )
    prt("OUTPUT WILL BE ASCII............" ) ;
else  if( outtype == 1 )
    prt("OUTPUT WILL BE FLOATS............" ) ;
else
    {
    prt("NOT A RECOGNIZED OUTPUT TYPE FLAG. BYE!\n\n" ) ; exit(EXIT_FAILURE); 
    }
if( ochan > 1 ){
    if( !chanmethod )prt(".................USING AVERAGE METHOD" ) ;
    else prt(".................USING PEAK METHOD" ) ;
}


if( outformat == 0 )
    prt("OUTPUT WILL BE IN FREQUENCY" ) ;
else if( outformat == 1 )
    prt("OUTPUT WILL BE IN OCTAVE UNITS" ) ;
else if( outformat == 2 )
    prt("OUTPUT WILL BE IN OCTAVE.PITCHCLASS" ) ;
else if( outformat == 3 ){
    prt("OUTPUT WILL BE IN SEMITONES OF DEVIATION" ) ;
    prf( refpitch,  "REFERENCE PITCH" ) ; 
}else if( outformat == 4 ){
    prt("OUTPUT WILL BE IN INVERTED SEMITONES OF DEVIATION" ) ;
    prf( refpitch,  "REFERENCE PITCH" ) ; 
}else
    {
    prt("NOT A RECOGNIZED OUTPUT FORMAT FLAG. BYE!\n\n" ) ; exit(EXIT_FAILURE); 
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


		swarpshape.A[ 0 ] = fval( &swarpshape, dur, t );

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


	    difff = hif - lowf ; 

		if( exflag ){
		    prt( "\n\nIS YOUR DATA TYPE FLAG CORRECT?\n\n.......CHECK YOU DATA.\n\n" ) ; 
		    exit(EXIT_FAILURE) ; 
		}


		

//*****************
// MODIFICATIONS LOOP
//*****************


//*************WARP THE INPUT SPECTRUM
	    //spectmagwarp( channel,  (N + 2), swarpshape.A[ 0 ], 0 ) ;		


// MAKE THE CENTROID



//fprintf( stderr, "\nN = %d, lowf = %f, hif = %f, fundamental = %f", N,  lowf, hif, fundamental ) ; 
  
    find_centroid( &temp, &temp4, channel, (N + 2), lowf, hif, fundamental, &old_temp ) ;  


//prf( temp,  "CENTROID FREQUENCY" ); 


		
		
		//SMOOTH IT 
		if(temp > old_temp){ //ATTACK
		    temp = (attackc * old_temp) +
			(minusattackc * temp) ; 
		} else { // DECAY
		    temp = (releasec * old_temp) +
		    (minusreleasec * temp) ; 
		}
		old_temp = temp ; 
		
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
    

    ochan = 1 ; 
    fscratch = fopen( scratch, "r" ); 
    rewind( fscratch  ); 
    kk = 11025 ; kl = 0 ; 
    fprintf( stderr,  "\n\n" ) ; 
    eof = 0 ; mm = 0 ; 
    tp = 0 ; 
    while ( !eof  ){
	if( fread( &temp, sizeof(float) , 1, fscratch ) == 0 ){
	    eof = 1;
 	}else{

	    

	
	    t =  (float) frame_count * IR  ; 

	    warp.A[ 0 ] = fval( &warp, dur, t );

// WARP
	    
	    temp = curve( lowf,  hif,  ((temp - lowf) / difff ),  warp.A[ 0 ]  ) ; 

       
    while( tp < 1. ){
	temp4 = curve( old_temp,  temp,  tp,  0.  ) ;

	// CHANGE FORMAT, IF SOUNDFILE LEAVE ALONE
	    if( outformat == 1 ){
		// TO OCTAVE
		temp4 = 8.0 + (log10f(temp4 / midC) / log_of_2) ; 
		
	    }else if( outformat == 2 ){
		// TO OCTAVE.PITCHCLASS
		temp4 = 8.0 + (.12 * (log10f(temp4 / midC) / log_of_2)) ; 
		
	    }else if(  outformat == 3 ){
		// SEMITONES OF DEVIATION
		// MAKE OCTAVE
		temp4 = 8.0 + ( (log10f(temp4 / midC) / log_of_2)) ; 
		// FIND OCTAVE DIFF MAGNIFIED BY SEMITONES
		temp4 = 12. * (temp4 - refoctave) ; 

	    }else if(  outformat == 4 ){
		// SEMITONES OF DEVIATION
		// MAKE OCTAVE
		temp4 = 8.0 + ( (log10f(temp4 / midC) / log_of_2)) ; 
		// FIND OCTAVE DIFF MAGNIFIED BY SEMITONES
		temp4 = ( -12. * (temp4 - refoctave)) ; 

	    }


	if( outtype == 0 ){
	    fprintf( fofd, "%f\n",  temp4 ) ;
	}else if( outtype == 1 )
	{
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
	filesToRemove( NULL, 1 ) ; 


    // CLOSE  INPUT  FILES
    fclose(ifd) ;  


    // CLOSE  OUTPUT  FILES
	fclose(fofd) ; 
   
    if( lowfreq.n != 1. ) fclose(lowfreq.fp ) ;
    if( hifreq.n != 1. ) fclose(hifreq.fp ) ;
    if( release.n != 1. ) fclose(release.fp ) ;
    if( attack.n != 1. ) fclose(attack.fp ) ;
    if( warp.n != 1. ) fclose(warp.fp ) ;
    if( swarpshape.n != 1. ) fclose(swarpshape.fp ) ;
    
    
    pri( mm, "NUMBER OF VALUES IN OUTPUT CENTROID ENVELOPE" ) ; 

    fprintf(stderr,"\n\tCENTROID : TRACKING COMPLETED\n");

    prt(""); 
    prs( ofile, "OUTPUT FILE" ) ; 
    prt(""); prt(""); prt("");

    if( plotflag == 1 ){
	sprintf( tempstring, "showme %s", ofile ) ;  
	system( tempstring ) ; 
    } ; 

    exit(EXIT_SUCCESS) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"centroid:  brightness envelope tracker\n"
	"centroid [flags] [input file] [output centroid file (optional)]\n"
	"                    (16-bit shorts)     (default: centroid.out)\n"
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
	"	F:	DETECTION  BAND: upper freq/pitch boundary (func) [Nyquist]\n"
	"	H:	warp index for reshaping magnitude response (func) [0.] \n"
	    "		    Values > 0 expand the dynamic range, \n"
	    "		    values < 0 compress the dynamic range. \n"
	"	l:      envelope ascent time  (func) [0.]\n"
	"	L:      envelope descent time   (func) [0.]\n"
	"	W:	centroid distribution warp index  [0]\n"  
	    "		    value of 0: no warp\n"
	    "		    values > 0 warp the  distribution downward\n"
	    "		    values < 0 warp the distribution upward\n"
	"	p:	print elapsed time (1 = on,  0 = off) [0]\n"
	"	r:	output samples per second (interpolated) [500.]\n"
	"	g:	output data type 0 = ascii,  1 = floats [0]\n"
	"	q:	output data format [0] \n"
	    "		    0 = frequency units, \n"
	    "		    1 = as pitch in octave units, \n"
	    "		    3 = octave.pitchclass code\n"
	    "		    4 = semitones of deviation from reference \n"
	    "		    4 = inverted semitones of deviation from reference \n"
	"	G:	reference pitch in octave.pitchclass for -q2\n"
	    "		    semitones of deviation [0]\n"
	"	P:	plot output, 1 = yes, 0 = no [0]\n"
	    );
	exit(EXIT_SUCCESS);
    }



void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }



