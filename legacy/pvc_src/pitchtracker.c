#include "globals.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FREQUENCY_PROPORTION_THRESHOLD .99
#define BEGIN_T_FRAME_SIZE .05
#define MAX_T_FRAME_SIZE .3
#define NOTE_DB_THRESH -40
#define MAXIMUM_NUMBER_OF_FORMANTS 12
#define MINUMUM_FORMANT_DB -30

#define DEBUGFLAG 0

#define AUDIO_FILE 0
#define ANALYSIS_DATA_FILE 1

#define MAX_NUMBER_OF_STRONGEST_FORMANT_VALUES 20

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
  FILE *fopen(),  *fofd;
char ch, outformatunits[ STRING_SIZE ] ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,
    *channel_i,   *output ;
float peakamps[MAXIMUM_CHANNELS]
 ;
int numOversamps, backlogOutputFreqs=0 ; 

float averageAmp, fileAmp, fileFreq ; 
int amplitude_weighted_oversampling__factor=0 ; 
float mode_filter_window_size_in_seconds=0. ; 
 
int mode_filter_window_size_in_frames=0 ; 

 

 

 
 
int filepos ; 

int frameNow, beginFrame, endFrame ; 

float temp4, ftemp4,    binAmpsSum, oldBinAmpsSum=0.,  shortnorm ;  
float getthresh();
//int beginchan,  endchan,  
int ifileFlag ; 
 
 
 int notestate_flag=0 , goodfreq,   backlogcount, bsize,   firstnote ; 
 
 float ampnow_out[ 2048 ],  a0,  a1 ; 
 
float peakenvamp=0., ampthresh,ampgatethresh;
float   IR,  dur=0.;
char tempstring[ STRING_SIZE ], *user ; 
FILE *famp,  *ffreq ; 
FILE *famp2,  *ffreq2 ; 
char  freqname[ STRING_SIZE ],  ampname[ STRING_SIZE ] ; 
char  freqname2[ STRING_SIZE ],  ampname2[ STRING_SIZE ] ; 
float compression,  tp,  tpinc=500.,  freqdiff,  lowf,  hif ; 
float releasec,  minusreleasec,  attackc,  
    minusattackc; 
double ar_dB ;
  
float frametprop,  tempt=0.,  midC ;
int qseccount=0,  seccount=0,  printflag=0, plotflag=0 ;
int outtype=0 ; 
 

int firstThresholdHit=0 ; 

 
static int loc ; 
//*****************************
 
 
 
 
  


 
float old_freqnow=-1.,  freqnow, freqampnow;
int median_buffer_size=100; 
float *median_buffer; 

int *formants,   fsize,  *formant_freq_rank ; 
int num_formants=MAXIMUM_NUMBER_OF_FORMANTS ; 
float *formant_freq,  *formant_amp ; 

void optimal_comb(
	float F[], 
	int numberOfBins,
	float	  *freqnow,
	float *freqampnow,
	float dbThresh,
	float lowFreq,
	float highFreq,
	int optiomal_comb_OR_strongest_formant_FLAG,
	int mode_filter_window_size_in_frames

) ; 


int zero_cross_peaks( float F[], float F_i[],   int Nplus2,  
	    int formant_bin[],  int NP,  int num_formants  
	    ) ;

int print_peak_freqs( float F[], float fundamental,  int Nplus2,  int formant_bin[], int B_num_peaks ) ; 

int find_formants( 
    float F[], 
    float fundamental,  
    int Nplus2,  
    int formant_bin[], 
    int num_peaks, 
    float formant_freq[],
    float formant_amp[], 
    int formant_freq_rank[]
     ) ; 


void find_freq_new( 
    
	float	*freqnow,
	float	*freqampnow,

    	int detect_method,
    	float  lowf, 
    	float hif,
    	float formant_freq[], 
    	float  formant_amp[],
    	int formant_freq_rank[], 
    	int num_peaks 
    
)    ; 

//******************************


int begin_buffsize, 
    max_buffsize ; 

 

 

float ampnow,  fvalue,  avalue,  outval ; 
float	
	freqprop=FREQUENCY_PROPORTION_THRESHOLD, 
	tframe_size = BEGIN_T_FRAME_SIZE,  
	max_tframe_size = MAX_T_FRAME_SIZE, 	
	note_dBthresh = NOTE_DB_THRESH, 
	note_ampthresh ; 


 

float find_common_freq( 

    float freqprop, 
    float fbuff[],  
	float collectedfbuffvalues[],
    float abuff[],  
    int max_buffsize,  
    int begin_buffsize, 
    float ampthresh  
    
    ) ; 
    
    
    
float norm_value( 

    float avalue,  
    float peakenvamp, 
    float compression, 
    float ampthresh, 
    float ampgatethresh, 
    float warp   
    ) ; 

int detect_method=0 ; 

int note_onoff,  ll,  outformat=0 ; 
float mindB=-80., noteondB=-50, noteonamp,  minamp ; 

float outfreqoriginNow=440,  asmooth=0.,  minusasmooth ;

float *fbuff, *collectedfbuffvalues,  *abuff ; 

float sine_wave( float freq,  int R ) ; 


// ANALYSIS FILE STUFF
float *analysis_lower,  *analysis_higher ; 
  
int analysis_N,  analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 
float normamp[MAXIMUM_CHANNELS];
int ainchan ; 

float low, hi, range, average, median, mode, standarddeviation, sum, begin, end, middle ; 


struct func pitchTrack ; 

// DATA
struct  func  analysis ; 


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

// REFERENCE FREQUENCY 
struct func referenceFreqOrOctavePclass ;
	    
//#include "underflow.h"

//*****************INITIALIZE



referenceFreqOrOctavePclass.L = 1. ; referenceFreqOrOctavePclass.n = 0. ;
 referenceFreqOrOctavePclass.A[ 0 ] = 440. ; 

pitchTrack.L = 1. ; pitchTrack.n = 0. ; pitchTrack.A[ 0 ] = 0. ; 

// DATA
analysis.L = 1. ; analysis.n = 0. ; analysis.A[ 0 ] = 0. ; 

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



printUnixCommandLine( argc, argv ); 

if( argc < 2 )usage() ; 


while( (ch= crack( argc, argv, "a|b|B|c|C|d|D|e|E|f|F|g|G|H|j|J|L|l|m|M|N|o|O|p|P|r|R|s|S|T|W|X|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) { //


	    case 'H':   mode_filter_window_size_in_seconds = crackfloat( arg_option, ch );
			break;

	    case 'E':   amplitude_weighted_oversampling__factor = crackfloat( arg_option, ch );
			break;

	    case 'N':   N = (int) crackfloat( arg_option, ch ); // crackfloat( arg_option, ch )
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
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



	    case 'm':	detect_method = (int) crackfloat( arg_option, ch );
			break;



	    case 'O':	outformat = (int) crackfloat( arg_option, ch );
			break;

	    case 'o':	strcpy(tempstring, arg_option);
			referenceFreqOrOctavePclass.fp = crackstring( tempstring, 
			    &referenceFreqOrOctavePclass );
			break;



	    case 'j':	tframe_size = crackfloat( arg_option, ch );
			break;
	    case 'J':	max_tframe_size = crackfloat( arg_option, ch );
			break;
	    case 'd':	note_dBthresh = crackfloat( arg_option, ch );
			break;
	    case 'a':	asmooth = crackfloat( arg_option, ch );
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
	    case 'p':   printflag = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'P':   plotflag = (int) crackfloat( arg_option, ch ) ;
			break;

	}
}
 
prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "PITCH TRAJECTORY TRACKER", 69 ) ; 
prline( 69,  "-" ) ; 

user = pvc_user_tag(); 

    
minamp = dB_to_amp( mindB ) ; // pow( (double) 10.0, (double) (mindB/20.) );
noteonamp = dB_to_amp( noteondB ) ; // pow( (double) 10.0, (double) (noteondB/20.) );
note_ampthresh = dB_to_amp( note_dBthresh ) ; // pow( (double) 10.0, (double) (note_dBthresh/20.) );
note_onoff = 0 ; 

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
} ; 


strcpy( ifile, argv[ arg_index ] ) ; // INPUT SOUND FILE NAME
analysis.fp = crackstring( ifile, &analysis );

if( DEBUGFLAG ) pri( channelflag, "BEFORE outfile_setup channelflag" ) ; 
if( DEBUGFLAG ) pri( beginchan, "BEFORE outfile_setupbeginchan beginchan" ) ; 
if( DEBUGFLAG ) pri( ochan, "BEFORE outfile_setupochan ochan" ) ; 
if( DEBUGFLAG ) pri( ichan, "BEFORE outfile_setupochan ichan" ) ; 

    // READ IN FFT HEADER VALUES

rewind( analysis.fp ); 
    
if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &k,  normamp, &analysis, 0 ) == -1){
	ifileFlag = AUDIO_FILE ;   
	prline( 69,  "*" ) ; prs( ifile, "INPUT FILE" ) ;  // prt( "INPUT FILE TYPE: AUDIO" ) ; 
	prt( " SETTING UP TO USE AUDIO INPUT FILE . . . " ) ; 
	setupfiles(argc, argv) ; // AUDIO
}else{

	if( DEBUGFLAG ) prt("G1") ;
	ifileFlag = ANALYSIS_DATA_FILE ; ichan = analysis_chan ;
	prline( 69,  "*" ) ; prs( ifile, "INPUT FILE" ) ; prt( "INPUT FILE TYPE: ANALYSIS DATA" ) ;

		// FIND DURATION OF INPUT FILE

	niframes = (((analysis.n - (float) FFT_HEADER_SIZE) / (float) (analysis_N + 2))) / analysis_chan ; 
	analysis_dur = (float) (niframes) / ((float) analysis_R / (float) analysis_D ) ; 
	frames_per_sec = iframes_per_sec = (float) analysis_R /  (float) analysis_D ; 
     pri( niframes,  "NUMBER OF FRAMES IN ANALYSIS" ) ; 

	N = analysis_N; isr = R = analysis_R; D = analysis_D ; ichan = analysis_chan ; 
	idur = analysis_dur ; 

	// SET/INTERPRET BEGIN/END TIMES. 
	if( endt <= 0. ) endt = idur ;
	if( begint <= 0. ) begint = 0. ;

		// TEST BEGIN AND END TIMES.
	if( (endt <= begint)){
	        banneri() ;
	        fprintf( stderr, "\n\nBEGIN TIME = %f\nEND TIME = %f\n\n",  begint,  endt  ) ;
	        fprintf( stderr, "\n\nYOUR END TIME IS BEFORE OR EQUAL TO YOUR BEGIN TIME! BYE.\n\n" ) ;
	        exit(EXIT_FAILURE) ; 
	} ;
	
		// TEST END TIME. 
	if( endt > idur ){ 
             banneri() ;
	       fprintf( stderr, "%s%f%s%f%s", "\nYOUR END TIME OF  ", endt, "  SECONDS \nEXCEEDS THE ANALYSIS DURATION OF  ", 
		     idur, " SECONDS.\nYOUR END TIME WILL BE RESET TO THE DURATION OF THE FILE.\n\n" 
            ) ;
	      endt = idur ; 
     } ; 



	fvec( analysis_lower,  N+2  ) ;	// lower analysis array 
	fvec( analysis_higher,  N+2  ) ;	// higher analysis array 
	prt( " SETTING UP TO USE ANALYSIS DATA INPUT FILE. . . " ) ; 

	outfile_setup(argc, argv) ; // DATA
} ; 
prline( 69,  "*" ) ; 

prt(""); 


beginFrame = (int) (begint * frames_per_sec) ; 
endFrame =  (int) (endt * frames_per_sec) ;

mode_filter_window_size_in_frames = (int) (( mode_filter_window_size_in_seconds * frames_per_sec  ) + 0.5) ; 

pri( mode_filter_window_size_in_frames, "mode_filter_window_size_in_frames" ) ; 

pri( mode_filter_window_size_in_seconds, "MODE FILTER WINDOW SIZE IN SECONDS" ) ; 

if( DEBUGFLAG ) pri( beginchan, "AFTER outfile_setupbeginchan beginchan" ) ; 
if( DEBUGFLAG ) pri( ochan, "AFTER outfile_setupochan ochan" ) ; 
if( DEBUGFLAG ) pri( ichan, "AFTER outfile_setupochan ichan" ) ; 

endchan = beginchan + ochan ; 
   
// GET NAME OF OUTPUT FILE
arg_index++ ; 
if( arg_index >= argc  ){
	bannero() ;
	sprintf( ofile, "pitchtrack.out" ) ; 
	fprintf( stderr, "\n\n.....().USING DEFAULT OUTPUT FILENAME........\n\n" ) ;

	fprintf( stderr,  "\n\nOUTPUT FILE: %s\n",  ofile ) ; 
}else{
    // GET OUTPUT FILE NAME
	strcpy( ofile, argv[arg_index] ) ; 
}	 

prs( ofile, "OUTPUT FILE" ) ; 


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
// OPEN IT

srandom( time(NULL) ) ; 
sprintf( ampname, "/tmp/envelope.%s.%d", user, (int)(random()) ) ;
filesToRemove( ampname, 0 ); 
famp = fopen( ampname, "w+b" ); //AMP
if( DEBUGFLAG ) prs( ampname, "AMPLITUDE FILE NAME" ) ; 
sprintf( freqname, "/tmp/pitch.%s.%d", user, (int)(random()) ) ;
filesToRemove( freqname, 0 );
ffreq = fopen( freqname, "w+b" ); //FREQ
if( DEBUGFLAG ) prs( freqname, "FREQUENCY FILE NAME" ) ; 

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

// RECOMPUTE FOR INTERPOLATION
if(asmooth <= 0.){
	asmooth = 0. ; minusasmooth = 1. ; 
}else{
	// LOWPASS
	 asmooth = pow( (double) ar_dB,  
			(double) (IR / asmooth ) ) ; 
		minusasmooth = 1.  - asmooth ; 
}


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
prp( &lowfreq,  "LOW PITCH/FREQUENCY  BOUNDARY" ) ;  
prp( &hifreq,  "HIGH PITCH/FREQUENCY  BOUNDARY" ) ;  

prline( 1,  "*" ) ; 
prp( &release,  "ENVELOPE RELEASE TIME" ) ;  
prp( &attack,  "ENVELOPE ATTACK TIME" ) ;  
prline( 1,  "*" ) ; 
prp( &dBthreshold,  "COMPRESSION THRESHOLD (in decibels)" ) ;  
prp( &compression_dB,  "AMOUNT OF COMPRESSION (in decibels)" ) ;  
prp( &dBgate,  "GATE THRESHOLD (in decibels)" ) ;  
prline( 1,  "*" ) ; 
prf( outformat, "OUTPUT FORMAT CODE" ) ; 
prp( &referenceFreqOrOctavePclass,  "REFERENCE FREQUENCY OR PITCH" ) ; 

prp( &warp,  "WARP SHAPE INDEX" ) ;  
	    
if( outtype == 0 )
    prt("OUTPUT WILL BE ASCII............" ) ;
else  if( outtype == 1 )
    prt("OUTPUT WILL BE FLOATS............" ) ;
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
fvec( channel_i, N+2 ) ;	/* extra for formant search */

fsize = 1 + (num_formants * 2) ; 
ivec( formants, fsize ) ;	// MODEL FORMANT INDECES ARRAY

fvec( formant_freq, num_formants ) ; 
fvec( formant_amp, num_formants ) ; 
ivec( formant_freq_rank, num_formants ) ; 
 
fvec( output, Nw ) ;	/* output buffer */

fvec( median_buffer, median_buffer_size ) ;	/* median_buffer */



// OPEN INPUT  AND OUTPUT FILES
if( ifileFlag != ANALYSIS_DATA_FILE ) openfiles() ; 


//*********************************************
// LOOP FOR CHANNELS
//*********************************************

if( DEBUGFLAG ) pri( beginchan,"BEFORE CHANNEL LOOP beginchan");  
if( DEBUGFLAG ) pri( endchan,"BEFORE CHANNEL LOOP endchan"); 
if( DEBUGFLAG ) pri( ochan,"BEFORE CHANNEL LOOP ochan"); 
if( DEBUGFLAG ) pri( ichan,"BEFORE CHANNEL LOOP ichan"); 
if( DEBUGFLAG ) pri( eof, "BEFORE CHANNEL LOOP eof" ) ; 




//exit(0); 

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

	prline( 69,   "=" ) ; 
	pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

	// **** SET UP ANALYSIS CHANNEL
	if( ifileFlag == ANALYSIS_DATA_FILE ){
		 ainchan = outchan ; 

	} ; 

	//*****   REINITS
	frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; frameNow = beginFrame ;  
	tempt = 0. ; qseccount=0 ;  seccount=0 ; 
	peakamps[channow] = 0 ; backlogcount = 0 ;

	backlogOutputFreqs = 0;   

	rewind( famp ) ; 
	rewind( ffreq ) ; 

	makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
	in = -Nw ;
	if ( D )
		on = (in*I)/D ;
	else
		on = in ;
	
	if( DEBUGFLAG ) prt( "H1" ) ; 

	//*********************************************
	// LOOP FOR FRAMES
	//*********************************************

	if( DEBUGFLAG ) prf( dur, "dur" ) ; 

	while( 
     		   ( (ifileFlag == AUDIO_FILE) && !eof ) ||  ( ( ifileFlag == ANALYSIS_DATA_FILE) && (frameNow <= endFrame) ) 
	//        frameNow <= endFrame 
	){
 

		in += D ;
		on += I ;
		timenow( dur ) ;
		t = (float) frame_count * frametprop ; 
		if( DEBUGFLAG ) fprintf( stderr, "frameNow: %d\ttime: %f\n", frameNow, t ) ; 
		if( DEBUGFLAG ) prf( t, "t" ) ; 

		if( ifileFlag == AUDIO_FILE ){      
          		   // AUDIO
	 	  	eof = shiftin( input, Nw, D ) ;
	  	  	fold( input, Wanal, Nw, buffer, N, in ) ;
	  	  	rfft( buffer, N2, FORWARD ) ;
	  	  	convert( buffer, channel, N2, D, R ) ;
		} else {
			readFilterFrame(
				&analysis,
				channel,
				frameNow,
				analysis_N, 
				ainchan, 
				analysis_chan
			) ;  

		} ; 
		frameNow++ ; 

//*************************
// GET THE VALUES
//*************************
		lowfreq.A[ 0 ] = fval( &lowfreq, dur, t );
		lowf = lowfreq.A[ 0 ] <= 12 ? OPPC_to_Hz( lowfreq.A[ 0 ] ) : lowfreq.A[ 0 ] ; 
		hifreq.A[ 0 ] = fval( &hifreq, dur, t );
		hif = hifreq.A[ 0 ] <= 12 ? OPPC_to_Hz( hifreq.A[ 0 ] ) : hifreq.A[ 0 ] ; 



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

		// STUPIDITY TEST
		if( lowf < 0. ) lowf = 0. ; 
		if( hif > nyquist ) hif = nyquist ; 

		// TURN FREQ BOUNDS INTO INDECES


		i1 = 1 +  (int) ( 2. * (int) ( lowf / freqdiff ) ) ; 
		i2 = 1 +  (int) ( 2. * (int) ( hif / freqdiff ) ) ; 


		//*****************
		// MODIFICATIONS LOOP
		//*****************


    		//*************WARP THE INPUT SPECTRUM
   		//spectmagwarp( channel,  (N + 2), 20., 0 ) ;		


		// SMOOTH THE SPECTRUM TO HELP THE FORMANT SEARCH
    		//smoothspec( channel, (N2 + 1), .1, R ) ;


		// FIND FORMANT BIN VALLEYS AND PEAKS
//		num_peaks = zero_cross_peaks( channel,  channel_i,  N + 2,  
//			formants, fsize, num_formants  ) ;

		//pri( num_peaks, "num_peaks" ) ; 

		//prline( 69,   "=" ) ; 

		// USING THE VALLEY MARKERS, FIND THE FORMANT FREQUENCIES
//    		find_formants( channel,fundamental,  N + 2,  formants, num_peaks, 
//			formant_freq,  formant_amp, formant_freq_rank ) ; 


		// PRINT THE FORMANTS
		//   print_peak_freqs( channel, fundamental, N + 2,  formants, num_peaks ) ;  


		// FIND THE FREQ

		if( detect_method == 0 ){

			optimal_comb( channel, N2 + 1, &freqnow,	&freqampnow, note_dBthresh, lowf, hif, 0, mode_filter_window_size_in_frames ) ; 

		}else if( detect_method == 1 ){
	    		// STRONGEST FORMANT
//	    		find_freq_new( &freqnow, &freqampnow,
//			 	detect_method,  lowf,  hif,
//				formant_freq,  formant_amp, formant_freq_rank,  num_peaks ) ;  

			optimal_comb( channel, N2 + 1, &freqnow,	&freqampnow, note_dBthresh, lowf, hif, 1, mode_filter_window_size_in_frames ) ; 


		}else if( detect_method == 2 ){
	    		// CENTROID
	    		find_centroid( &freqnow, &freqampnow, channel, (N + 2), lowf, hif, fundamental, &old_freqnow ) ;
		}else{
	    		// BOGUS
	    		pri( detect_method,  "DETECTION METHOD" ) ;
	    		prt( " IS NOT ONE OF THE METHODS. BYE."); 
	    		exit(EXIT_FAILURE); 
		}
		if( DEBUGFLAG ) prf( freqnow,  "FREQUENCY" ) ; 

    		// USE OLD IF AMP WAS 0. 
//		if( freqnow <= 0. )  freqnow = old_freqnow ; 
 		if( freqnow == -1. )  backlogOutputFreqs++ ; else  backlogOutputFreqs = 1 ; 



		// ENVELOPE: SUM THE BIN AMPS BETWEEN THE BOUNDS
		binAmpsSum = 0 ; 

		for( i = i1; i <= i2; i += 2){
		     	binAmpsSum += channel[i - 1] ; 
		}
		

		//SMOOTH IT 
		if(binAmpsSum > oldBinAmpsSum){ //ATTACK
		    binAmpsSum = (attackc * oldBinAmpsSum) + (minusattackc * binAmpsSum) ; 
		} else { // DECAY
		    binAmpsSum = (releasec * oldBinAmpsSum) + (minusreleasec * binAmpsSum) ; 
		}
		oldBinAmpsSum = binAmpsSum ; 
		
		if( peakenvamp < binAmpsSum ) peakenvamp = binAmpsSum ; 

		// OUTPUT THE AMPLITUDE 
		if( channow == 0 ){
    		    	// WRITE FIRST CHANNEL
		    	fwrite( &binAmpsSum, sizeof(float), 1, famp );

			if( DEBUGFLAG ) prf( amp_to_dB(binAmpsSum), "AMPLITUDE in DB" ) ; 
		}else{
			loc = ftell( famp ) ;

		    	// READ IN VALUE
			fread( &fileAmp, sizeof(float) , 1, famp ) ; 

			if( !chanmethod ){ 
		     		// TAKE AVERAGE
		        	averageAmp = (    (fileAmp * (float) channow)  + averageAmp   )  / (float)  (channow + 1) ; 
			}else{
		        	// TAKE PEAK
		        	if( fileAmp > binAmpsSum ) binAmpsSum = fileAmp ; 
			}
		
			fseek( famp, loc,  SEEK_SET ) ; 

			fwrite( & binAmpsSum, sizeof(float), 1, famp );
		}		

		// ************** FREQUENCY
		// OUTPUT THE FREQUENCY 
		if( backlogOutputFreqs > 1 ) pri( backlogOutputFreqs, "backlogOutputFreqs" ) ; 

		if( channow == 0 ){
    			// WRITE FIRST CHANNEL
			for(k = 0; k < backlogOutputFreqs; k++) fwrite( &freqnow, sizeof(float), 1, ffreq );
		}else{
			loc = ftell( ffreq ) ; 

			// READ IN VALUE
			fread( &fileFreq, sizeof(float) , 1, ffreq ) ; 

			if( !chanmethod ){ 
		    		// TAKE AVERAGE
		    		freqnow = ( (fileFreq * (float) channow)  + freqnow   )  / (float)  (channow + 1) ; 
			}else{
		    		// TAKE PEAK
		    		if( fileFreq > freqnow ) freqnow = fileFreq ; 
			}

			fseek( ffreq, loc,  SEEK_SET ) ; 
			for(k = 0; k < backlogOutputFreqs; k++) fwrite( &freqnow, sizeof(float), 1, ffreq );
		}		
		// *********************

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




		if( DEBUGFLAG ) prf( frame_count, "frame_count" ); 

		frame_count++ ; 

		// FRAMES LOOP END
	}
    
	fseek(famp, 0, SEEK_END);
	i = ftell(famp);
	if( DEBUGFLAG ) pri( i, "famp FILE SIZE" ) ; 

	//if( DEBUGFLAG ) prt("CLOSING famp and ffreq"); 

	//    fclose( famp ) ; 
	//    fclose( ffreq ) ; 

} 



// NORMALIZE AMP ENVELOPE
//if( DEBUGFLAG ) prt( "OPENING famp" ) ; 
//famp = fopen( ampname, "w+" ); //AMP
eof = 0 ; 
k = 0 ; 
if( DEBUGFLAG ) prf( amp_to_dB( peakenvamp ),  "peakenvamp in dB" ) ; 
// CHANNELS LOOP END
rewind( famp ) ; 
while( eof != 1 ){
	loc = ftell( famp ) ;
	if( DEBUGFLAG ) pri( loc, "famp FILE POSITION" ) ; 
	i = fread( &fileAmp, sizeof(float), 1, famp ) ; 
	if( DEBUGFLAG )  pri( i, "i" ) ; 
	if( DEBUGFLAG ) fprintf( stderr, "\n%g", fileAmp * 100. ) ; 
	if( i > 0 ){
		fileAmp = fileAmp / peakenvamp ; 
		if( DEBUGFLAG ) fprintf( stderr, "\n%d: NEW NORMALIZED AMPLITUDE %g", k, fileAmp ) ; 
		fseek( famp, loc,  SEEK_SET ) ; 
		fwrite( &fileAmp, sizeof(float), 1, famp );
	}else{
		eof = 1 ; 
	} ; 
	k++ ; 
} ; 



if( ifileFlag == AUDIO_FILE ) fclose( analysis.fp ) ; 
fclose( famp ) ; 
fclose( ffreq ) ; 

peakenvamp = 1.0 ; 



// OPEN PITCH TRACK FILE
pitchTrack.fp = crackstring( freqname, &pitchTrack ) ; 

// FIND CHARACTERISTICS OF TRACK
findFuncStats( &pitchTrack, pitchTrack.L, &low, &hi, &range, &average, &median, &mode, &standarddeviation, &sum, &begin, 
		&end, &middle, 1, 0 ) ; // ???
prt(""); prt(""); prt("");
fclose( pitchTrack.fp ) ; 


// ADJUST FILE VALUES AND DISTRIBUTION WITH WEIGHTED OVERSAMPLING, IF CALLED FOR. 
if( amplitude_weighted_oversampling__factor >= 1. ){

	// OPEN EXISTING FILES WITH OLD FILE NAMES AND POINTERS.
	ffreq = fopen( freqname, "r+b" ); famp = fopen( ampname, "r+b" );

	// MAKE NEW NAMES FOR OVERSAMPLED FREQ AND AMP FILES.
	sprintf( ampname2, "/tmp/envelope.%s.%d", user, (int)(random()) ) ;
		filesToRemove( ampname2, 0 ); famp2 = fopen( ampname2, "w+b" ); //AMP
//prs( ampname2, "ampname2" ); 

	sprintf( freqname2, "/tmp/pitch.%s.%d", user, (int)(random()) ) ;
		filesToRemove( freqname2, 0 ); ffreq2 = fopen( freqname2, "w+b" ); //FREQ
//prs( freqname2, "freqname2" ); 

	
	prt( "** WEIGHTED OVERSAMPLING **" ); 
	while( (fread( &fileFreq, sizeof(float), 1, ffreq ) == 1) && (fread( &fileAmp, sizeof(float), 1, famp ) == 1) ){

		numOversamps = (fileAmp > 0.) ? (int)((amplitude_weighted_oversampling__factor * fileAmp) + 0.5) : 0 ; 

		for(i = 0; i < numOversamps; i++){
			fwrite( &fileFreq, sizeof(float), 1,   ffreq2 ) ; fwrite( &fileAmp, sizeof(float), 1,   famp2 ) ;		
		} ; 
	} ;  
	
}else{
	// USE FILES AS THEY ARE.
	prt( "NO WEIGHTED OVERSAMPLING" ); 

	// OPENING OLD FILES WITH NEW FILE DESCRIPTORS.
	ffreq2 = fopen( freqname, "r+b" ); famp2 = fopen( ampname, "r+b" );

} ; 
	// REWIND BOTH FILES.
	rewind( famp2 ) ; rewind( ffreq2 ) ; 



// ********************************* NOW OPEN, COMPRESS AND NORMALIZE
    
    fprintf( stderr," \nNORMALIZING, THE SIGNAL......" ) ;   
    ochan = 1 ; frame_count = 0 ; notestate_flag = 0 ; backlogcount = 0 ; firstnote = 1 ;  
    ampnow_out[ 0 ]  = 0. ; 

	// SET OLD OUTPUT FREQ VALUE AS -1., WHICH SERVES AS FLAG FOR NO VALUE. 
	old_freqnow = -1. ; 

	i = ftell(famp2);
	if( DEBUGFLAG ) pri( i, "famp2 FILE POSITION" ) ; 

    kk = 11025 ; kl = 0 ; 
    fprintf( stderr,  "\n\n" ) ; 
    shortnorm = 1. - (1./32760.) ;   
    eof = 0 ; mm = 0 ; tp = 0 ;
 
	// MAKE BEGINNING BUFFER SIZE, IF LESS THAN 6, MAKE IT 6
    begin_buffsize = (.5 + tframe_size * (float) frames_per_sec) ;
    if( begin_buffsize < 6 ) begin_buffsize = 6 ; 
	// MAKE MAX BUFFER SIZE, IF LESS THAN 6, MAKE IT 6
     max_buffsize =  (.5 + max_tframe_size * (float) frames_per_sec) ;
    if( max_buffsize < 12 ) max_buffsize = 12 ; 


	if( DEBUGFLAG ) pri( max_buffsize, "max_buffsize" ) ; 
	fvec( fbuff,  max_buffsize ) ; 
	fvec( collectedfbuffvalues, max_buffsize ) ; 
	fvec( abuff,  max_buffsize ) ; 
    

	// FILL FREQ  BUFFER
	i = fread( &fbuff[ 0 ], sizeof(float) , 1, ffreq2 ) ;
	for( j = 1 ; j < max_buffsize; j++ ) fbuff[ j ] = fbuff[ 0 ] ; 

	filepos = ftell(famp2);
	if( DEBUGFLAG ) pri( filepos, "famp2 FILE POSITION" ) ; 

	i = fread( &abuff[ 0 ], sizeof(float) , 1, famp2 ) ; 	
	for( j = 1 ; j < max_buffsize; j++ ) abuff[ j ] = abuff[ 0 ] ; 


//** BEGIN OF OUTPUT LOOP

	while ( max_buffsize > 6  ){

		//*************************************************
	    	  
		// GET SOME CONTROL VALUES
	      	
		t =  (float) frame_count * IR  ; // TIME
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
		referenceFreqOrOctavePclass.A[ 0 ] = fval( &referenceFreqOrOctavePclass, dur, t );
		outfreqoriginNow = ( referenceFreqOrOctavePclass.A[ 0 ] <= 12 ) ? 
			OPPC_to_Hz( referenceFreqOrOctavePclass.A[ 0 ] ) : 
				referenceFreqOrOctavePclass.A[ 0 ] ; 		

	    	//*************************************************


		if( DEBUGFLAG ) prf( frame_count, "frame_count" );
		if( frame_count == 0 ){
			// NORMALIZE
			for( i = 0; i < max_buffsize; i++ ){
				abuff[ i ] = norm_value( abuff[ i ], peakenvamp,  compression, 
					ampthresh, ampgatethresh,  warp.A[ 0 ]   ) ; 
				if( DEBUGFLAG ) prf( abuff[ i ],  "abuff[ i ]" ) ; 
			} ; 
		}

		//*************************************************
		// READ IN A FREQ AND AMP VALUE
		if( !eof ){
			if( fread( &fvalue, sizeof(float), 1, ffreq2 ) == 0 )  eof = 1 ; 
			filepos = ftell(famp2);
			if( DEBUGFLAG ) pri( filepos, "famp2 FILE POSITION" ) ; 
			if( fread( &avalue, sizeof(float), 1, famp2  ) == 0 )  eof = 1 ; 

			if( DEBUGFLAG ) pri( eof, "eof" ) ; 
			if( DEBUGFLAG ) prf( amp_to_dB( avalue / peakenvamp ), "BEFORE avalue in dB" ); 

			avalue = norm_value( avalue, peakenvamp,   compression, ampthresh, ampgatethresh,  warp.A[ 0 ]   ) ; 

			if( DEBUGFLAG ) prf( amp_to_dB( avalue / peakenvamp ), "AFTER avalue in dB" ); 

		} ; 

		// IF WE HAVE NOT HIT THE END, THEN....
		if( !eof ){

if( DEBUGFLAG ) prt( "NOT AT END"); 
			// SHIFT AND ADD TO BUFFERS
			// FIRST SHIFT ALL THE OLD BY ONE
			for( ll = max_buffsize - 1 ; ll > 0 ; ll-- ){ 
				fbuff[ ll ] = fbuff[ ll - 1 ] ; abuff[ ll ] = abuff[ ll - 1 ] ;
			}
			// FILL 0 LOCATION WITH NEW VALUE. 
			fbuff[ 0 ] = fvalue ; abuff[ 0 ] = avalue ;
		}else{
if( DEBUGFLAG ) prt( "AT END"); 
			// HIT THE END. DECREMENT MAX SIZE AND
			max_buffsize-- ; 
			// SHIFT VALUES TOWARDS BEGIN
			for( ll = 0 ; ll > max_buffsize ; ll++ ){ 
				fbuff[ ll ] = fbuff[ ll + 1 ] ; abuff[ ll ] = abuff[ ll + 1 ] ;
			}
		    
			// IF THE FULL (MAX) BUFFER SIZE HAS DIMINISHED BELOW BEGIN SIZE
			// MAKE THE BEGIN THE SAME AS THE MAX
			if( begin_buffsize > max_buffsize ) begin_buffsize = max_buffsize ; 
		}

 
		// MAKE THE CURRENT AMP EQUAL THE HEAD OF THE AMP BUFFER
		ampnow = abuff[0] ; 

// prf( ampnow,  "ampnow" ) ; 
       
		// ***************** FREQ AND AMP OUT *********************
	    	// ************ FREQ FINDING SECTION **********
	    	// FIND THE MOST COMMON VALUE IN THE BUFFER

		if( frame_count == 0 ) {
			// IF THE FIRST FRAME, SET OLD FREQ TO ANALYZED MEDIAN. 
			if( old_freqnow == -1. ) old_freqnow = mode ;
		}
		if( DEBUGFLAG ) prf( t,  "TIME" ) ;
		// HERE GOES... FIND THE MOST COMMON FREQ IN BUFFER
		// (freqprop IS THE THRESHOLD OF COMMONMESS PROPORTION DEFINED UP TOP
		freqnow = find_common_freq( freqprop, fbuff, collectedfbuffvalues, abuff, max_buffsize,  begin_buffsize,  note_ampthresh ) ;
		if( DEBUGFLAG ) prf( freqnow,  "HERE->                          NOTE FREQUENCY" ) ;

if( DEBUGFLAG ) pri( notestate_flag,  "notestate_flag" ) ; 
if( DEBUGFLAG ) prf( freqnow,  "NONADJUSTED FREQ" ) ; 
if( DEBUGFLAG ) prf(ampnow,  "ampnow" ) ; 
	    // AMP THRESHOLD ADJUSTMENTS
	    if(  ( ampnow < note_ampthresh ) ){

			if( firstnote ){

if( DEBUGFLAG ) prt( "// FIRST NOTE AND NO AMP. NOT A GOOD FREQ." )  ; 
				// FIRST NOTE AND NO AMP. NOT A GOOD FREQ. 
				goodfreq = 0 ; 

			}else{
				// NOT THE FIRST NOTE
		    
if( DEBUGFLAG ) prt( " BELOW AMP THRESHOLD AND NOT FIRST NOTE, USE OLD" ) ; 
				//***  BELOW AMP THRESHOLD  USE OLD
		     	freqnow = old_freqnow ;
				// NOW IT IS A GOOD FREQ
				goodfreq = 1 ; 
		    
			}
		    
		}else{
		
		
			//**** ABOVE AMP TRHESHOLD

			if( (freqnow < lowf) || (freqnow > hif) ){
		
			//*** OUT OF BOUNDS FREQ

				if( notestate_flag && goodfreq ){ 

if( DEBUGFLAG ) prt( "// NOTE IS ALREADY ON AND WE HAD A GOOD FREQ, USE OLD VALUE" ) ; 

					// NOTE IS ALREADY ON AND WE HAD A GOOD FREQ, USE OLD VALUE
					freqnow = old_freqnow ;
					// OLD VALUE HERE IS A GOOD FREQ 
			 		goodfreq = 1 ; 

				}else{
					// NOTE HAS BEEN OFF. NOT A GOOD FREQ. 
					goodfreq = 0 ; 

if( DEBUGFLAG ) prt( "// NOTE HAS BEEN OFF. NOT A GOOD FREQ." ) ; 
		     
		    		}

			}else{
if( DEBUGFLAG ) prt( "// IN BOUNDS FREQ,  A GOOD FREQ" ) ; 
				// IN BOUNDS FREQ,  A GOOD FREQ
				goodfreq = 1 ; 
			}
	    
		}

		if( DEBUGFLAG ) prf( freqnow,  "ADJUSTED FREQ" ) ;


		// SET notestate_flag FOR NEXT ROUND
		if(ampnow >= note_ampthresh) notestate_flag = 1; 
		else notestate_flag = 0 ; 


		// INCREMENT backlogcount
		backlogcount++ ; 


if( DEBUGFLAG ) fprintf( stderr,  "\ntime: (%f) ampnow = %f,  note_ampthresh = %f, old_freqnow = %f,   freqnow = %f", 
		(float) frame_count * IR, ampnow, note_ampthresh,  old_freqnow,    freqnow  ) ; 


	// ************ 


if( DEBUGFLAG ) pri( goodfreq,  "BEFORE OUTPUT ROUTINE: goodfreq" ) ; 
if( DEBUGFLAG ) pri( backlogcount,  "BEFORE OUTPUT ROUTINE:backlogcount" ) ; 



	// INSERT NEW
		ampnow_out[ backlogcount ] = ampnow ; 


	// GOOD FREQ ROUTINE
		if( goodfreq == 1  ){

			if( firstnote ){
			// FIRST NOTE, USE freqnow FOR old_freqnow AND FLIP firstnote
			old_freqnow = freqnow ; 
			firstnote = 0 ; 
		}


		if( (firstThresholdHit == 0) && (ampnow > note_ampthresh) ) firstThresholdHit = 1 ; 

	  // LOOP FOR BACKLOG
	  // SAVE backlogcount SIZE
		bsize = backlogcount ; 
	  
	  
	  
		while( backlogcount > 0 ){
if( DEBUGFLAG ) pri( backlogcount,  "IN ROUTINE LOOP:backlogcount" ) ; 

		
			// SMOOTH FREQ VALUE
			freqnow = (asmooth * old_freqnow) +  (minusasmooth * freqnow) ; 


			a0 = ampnow_out[bsize - backlogcount] ; 
			a1 = ampnow_out[bsize - backlogcount + 1] ; 
	
	 

			while( tp < 1. ){
		    		// AMP VALUE
		    		temp4 = curve( a0,  a1 ,  tp,  0.  ) ;
		    		if( temp4 >= 1. )temp4 = shortnorm ;  
		    		// FREQ
		    		ftemp4 = curve( old_freqnow,  freqnow,  tp,  0.  ) ;
	
		    		if( outformat == 0 ){
					// FREQ
					outval = ftemp4 ; 
						strcpy( outformatunits, "HERZ" ) ;  
		    
		    		}else if( outformat == 1 ){
					// O.DECIMAL
					outval = 8.0 + ( log10( ftemp4 / 261.625 ) / log10( 2.0 )) ;  
						strcpy( outformatunits, "OCTAVE DECIMALS" ) ; 
		    		}else if( outformat == 2 ){
					// SEMITONES OF DEVIATION 
			    		outval = 12. * log10(ftemp4 / outfreqoriginNow) / log10( 2.0 ) ;   
						strcpy( outformatunits, "SEMITONES OF DEVIATION" ) ; 	    
		    		}else if( outformat == 3 ){
					// SEMITONES OF DEVIATION 
			    		outval = - (12. * log10(ftemp4 / outfreqoriginNow) / log10( 2.0 )) ;   
						strcpy( outformatunits, "INVERTED SEMITONES OF DEVIATION" ) ; 	    
		    		}else if( outformat == 4 ){
					// MIDI
					outval = Hz_to_MIDI( ftemp4 ) ;   
						strcpy( outformatunits, "MIDI" ) ;
				}else if( outformat == 5 ){
					// OCTAVE.PITCHCLASS
					outval = Hz_to_OPPC( ftemp4 ) ;   
						strcpy( outformatunits, "OCTAVE.PITCHCLASS" ) ;
		    		}else {
					fprintf( stderr, "\n\nNOT AN ACCEPTED OUTPUT FORMAT!" ) ;  
		   	 	}

if( DEBUGFLAG ) fprintf( stderr, "\nnote_ampthresh: %f  ampnow: %f, outval: %f", amp_to_dB( note_ampthresh ), amp_to_dB( ampnow ), outval ) ; 

				if( outtype == 0 ){
				 		fprintf( fofd, "%f\n",  outval ) ;
		    		}else{
						fwrite( &outval, sizeof(float), 1,   fofd ) ;
					if( DEBUGFLAG ) prt( "------------WRITING TO PITCH TRACK OUTPUT FILE" ) ;		
		    		} ; 

		
		    		mm++ ; 
				if( DEBUGFLAG ) pri( mm, "mm" ) ; 
		    		tp = tp + tpinc ;
	
				// PASSIFIER PRINT
		    		if( kk < 0 ){
					fprintf( stderr, " * " ) ; kk = 11025 ; 
		    		} ; 
	
		    		kk-- ; 
			} ; 
    
			old_freqnow = freqnow ; 
			tp = tp - (float) ((int) tp ) ; 
	
	
		    	// DECREMENT backlogcount
			backlogcount-- ; 
    
	 		// END OF LOOP FOR BACKLOG OUTPUT
	 	} ; 


		// PUT ampnow IN FIRST LOCATION
		ampnow_out[ 0 ] = ampnow ; 


   		// END OF GOOD FREQ ROUTINE
      } ; 



	// INCREMENT FRAME COUNT
     frame_count++ ; 


    	// OUTPUT LOOP END
   } ; 





    
	fclose( famp2 ) ;
	fclose( ffreq2 ) ;
	// CLOSE INPUT FILES
	if(ifd)fclose(ifd);  
	fclose( fofd ); 


    
    pri( mm, "NUMBER OF VALUES IN PITCH TRAJECTORY" ) ; 

    fprintf(stderr,"\n\nPITCHTRACKER : TRACKING COMPLETED\n");

	prs( ifile, "INPUT FILE" ) ;
    	prt(""); 
    	prs( ofile, "OUTPUT FILE" ) ; 

	pitchTrack.fp =  crackstring( ofile, &pitchTrack ) ; 


	prt("PITCH TRACK STATISTICS" ) ; 
//	if( outformat != 0 ) prs( outformatunits, "OUTPUT UNITS" ) ;  		
	findFuncStats( &pitchTrack, pitchTrack.L, &low, &hi, &range, &average, &median, &mode, &standarddeviation, &sum, &begin, 
		&end, &middle, 1, outformat ) ;  // ??? 
    	prt(""); prt(""); prt("");
	fclose( pitchTrack.fp ) ; 


	prs( ifile, "INPUT FILE" ) ;

	if( outtype == 0 ) prt("OUTPUT FORMAT: ASCII" ) ;
	else  prt("OUTPUT FORMAT: BINARY" ) ;



	prt(""); prt(""); prt("");

	fprintf(stderr,"\n\nPITCHTRACKER : SEARCH COMPLETE.\n\n");


	filesToRemove( NULL, 1 );

	if( plotflag == 1 ){
		sprintf( tempstring, "showme %s", ofile ) ;  
		system( tempstring ) ; 
    	} ; 



    exit(EXIT_SUCCESS) ;
}
void usage()
{ 
    fprintf(stderr, "%s",
	"pitchtracker:  pitch trajectory tracker\n"
	"pitchtracker [flags] [input sound file] [output file]\n"
	"	    Most input soundfile formats accepted.\n"
	"	N:	FFT length (must be a power of 2) [1024]\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	D:	analysis frames per second [200]\n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds (-1. = end of file) [0.] \n"
	"	C:	analysis channel (1 -> ?) (0 = average of all) [0] \n"
	"	X:	multiple channel method: [0] \n"
	"		    0 = average,  1 = peak\n"
	"	m:	DETECTION METHOD: 0 = optimal comb,  1 = strongest frequency,\n"
	"			2 = centroid frequency [0]\n"

	"	f:	DETECTION  BAND: lower freq/pitch boundary (func) [13]\n"
	"	F:	DETECTION  BAND: upper freq/pitch boundary (func) [Nyquist]\n"
	"		   Values less than or equal to 12 are treated as octave.pitchclass.\n"
	"	j:	DETECTION  WINDOW SIZE: BEGIN/MINIMUM (seconds) [.05]\n"
	"	J:	DETECTION  WINDOW SIZE: MAXIMUM (seconds) [.3]\n"
	"	d:	DETECTION  THRESHOLD:  (0 to -96 dB) [-40]\n"
	"	    COMPRESSOR/GATE\n"

	"	H:	mode filter window size in seconds [0]\n"
	"	E:	amplitude weighted oversampling factor [0]\n"


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
	"	O:	OUTPUT FORMAT 0 = frequencies,  1 = octave.decimal\n"
	"		    	2 = semitones of deviation above/below -o [0]\n"
	"		    	3 = negative of semitones of deviation above/below\n"
	"			4 = MIDI, 5 = octave.pitchclass [0]\n"
	"	o:	reference frequency or pitch (in octave.pitchclass)\n"
	"			 for semitones of deviation (func) [440]\n"
	"	r:	OUTPUT samples per second (interpolated) [500.]\n"
	"	g:	OUTPUT data type 0 = ascii,  1 = floats [1]\n"
	"	a:	OUTPUT lowpass filter smoothing response time [0.]\n"
	"	P:	plot output file using gnplot\n"
	);
    exit(EXIT_SUCCESS);
}

float find_freq( channel, N, lo_freq, hi_freq, fundamental  )

    float   channel[];	// BIN FREQUENCIES AND AMPS

    int	    N ;	 

    float    lo_freq,		
	    hi_freq,
	    fundamental
	     ; 		


{

    int i,  peakbin,  not_done ; 
    float maxamp,  freqnow,  asum,  fsum ; 
    int lowi, hii ; 
    
    maxamp = 0. ; 
    freqnow = 0. ;
    peakbin = -1 ; 


    lowi = 2 * (int) (lo_freq / fundamental) ;  
    hii = 2 * (int) (hi_freq / fundamental) ;  
    if( lowi <= 0 ) lowi = 2 ; 
    if( hii <= lowi ) hii = lowi + 2 ; 
    if( hii >= N ) hii = N - 2 ; 
    lowi += 1 ; 
    hii += 1 ; 
    
    
    // FIND THE STRONGEST BIN
    for( i = lowi; i < hii; i += 2){
	if( channel[i - 1] > maxamp ){
	    maxamp = channel[i - 1] ;
	    freqnow = channel[i] ;
	    peakbin = i ; 
	}
    }

    if( peakbin != -1 ){
	// MAKE A WEIGHTED AVERAGE OF BINS AROUND peakbin
	asum = channel[peakbin -1] ; 
	fsum = channel[peakbin] * channel[peakbin -1] ; 
	
	// ABOVE
	i = peakbin + 2 ; not_done = 1 ;
	while( not_done ){
	    if( ((i > 0) && (i < N)) && 
		( (channel[i - 1] / channel[peakbin] ) > .031 ) ){
		    fsum += (channel[i] * channel[i - 1] ) ;
		    asum += channel[i - 1] ;
		    i += 2 ;   
	    }else{
		not_done = 0 ; 
	    }
	}

	// BELOW
	i = peakbin - 2 ; not_done = 1 ;
	while( not_done ){
	    if( ((i > 0) && (i < N)) && 
		( (channel[i - 1] / channel[peakbin] ) > .031 ) ){
		    fsum += (channel[i] * channel[i - 1] ) ;
		    asum += channel[i - 1] ;
		    i -= 2 ;   
	    }else{
		not_done = 0 ; 
	    }
	}

	// FIND WEIGHTED SUM VALUE
	if( asum > 0. )freqnow = fsum / asum ; 

    
    }    
    return( freqnow ) ;     


}







float find_common_freq( 

	float freqprop, 
	float fbuff[],  
	float collectedfbuffvalues[],  
	float abuff[], 
	int max_buffsize,  
	int begin_buffsize, 
    float ampthresh 
    
){
	float a1,  a2,  v1,  v2,  notefreq,  tfreqprop,  ampweightedfreqsum,  ampsum,  temp1 ; 
	int minNumValsForAverage,  n,  i, beginbufferindexpoint,  begin_buffsizesave,  zeroflag, numberOfCommonValuesFound ; 
	float low, hi, range, average, median, mode, standarddeviation, sum, begin, end, middle ; 
 
   
	begin_buffsizesave = begin_buffsize ; 
	notefreq = -1. ;
	tfreqprop = freqprop ;  
	zeroflag = 0 ; 
	//pd(500);

	//prf( ampthresh,  "ampthresh" ) ; 
	// TEST AMP BUFFER FOR ZERO AMP
	numberOfCommonValuesFound = 0 ; 
	for( i = 0; i < max_buffsize; i++ ) if( abuff[i] > 0. ) numberOfCommonValuesFound++ ; 
	if( numberOfCommonValuesFound < begin_buffsize ){
	    	// LEAVE
		//prt( "ALL ZERO AMP" ) ; 
	    	return( notefreq ) ; 
	}

	//* MAKE FREQUENCY*************
	// notefreq: THE COMMON FREQ, SET TO -1 INITIALLY AS FLAG FOR NONE FOUND
	// beginbufferindexpoint: THE BEGINNING INDEX POINT IN THE FREQ AND AMP BUFFERS
	// begin_buffsize: THE BEGINNING BUFFER SIZE FROM WHICH TO FIND COMMON FREQ IN


	while( notefreq == -1. ){
		// HAVEN'T FOUND A FREQ YET
		// SET THE INDEX TO THE BEGINNING OF THE BUFFER
		beginbufferindexpoint = 0 ;
			
		// IF STILL NO FREQUENCY AND BEGIN BUFF HAS NOT SHRUNK TO LESS THAN 6 
		while( (notefreq == -1.) && (begin_buffsize > 5) ){
			
			//pd(501);

			// * TEST FOR AMPLITUDE;  SUM AMP
			ampsum = 0. ; 
			for( i = beginbufferindexpoint; i < (beginbufferindexpoint + begin_buffsize) ; i++ ) ampsum += abuff[i] ; 
			//pd(502);

			if( ampsum != 0. ){
				// THERE IS AMPLITUDE
				// SET THE MINUMUM NUMBER OF VALUES FROM WHICH AN AVERAGE WILL 
				// BE MADE TO EQUAL A 1/4 OF THE BEGINNING BUFFER SIZE. 
				// SET zeroflag TO 1 TO SIGNAL AMP
				zeroflag = 1 ; 
				minNumValsForAverage = (float) begin_buffsize * .25 ; 
				
				// numberOfCommonValuesFound IS THE COUNT ON THE VALUES FOUND COMMON ENOUGH TO USE
				// ampweightedfreqsum AND ampsum ARE THE SUM OF THE VALUES FOUND
				numberOfCommonValuesFound = 0 ; ampweightedfreqsum = 0. ; ampsum = 0. ; 


				for( i = beginbufferindexpoint; i < (beginbufferindexpoint + (begin_buffsize / 2)) ; i++ ){
				    // i BEGINS AT beginbufferindexpoint AND MOVES HALFWAY THROUGH THE BEGINNING
				    // BUFFER SIZE
				
				    // MAKE A PROPORTION OUT OF VALUES LYING SYMMETRICALLY IN
				    // THIS SHIFTING BEGIN BUFF RANGE 
				    // IF THE END IS ZERO PREVENT THE DIVIDE (VALUE WILL NOT BE USED)
				    v1 = fbuff[i] ; v2 = fbuff[beginbufferindexpoint + begin_buffsize - i] ; 
				    a1 = abuff[i] ; a2 = abuff[beginbufferindexpoint + begin_buffsize - i] ;
					//prf( a1,  "a1" ) ; 
					//prf( a2,  "a2" ) ;
 				    
					//SKIP IF EITHER ARE BELOW THE AMP THRESHOLD
				    	if( (a1 > ampthresh) && (a2 > ampthresh) ){
						//prt( "INSIDE LOOKING" ) ; 				    
						if(v2 == 0.){
					    		temp1 = fabs((double) (v1 / .001 )) ;
						}else{				
					    		temp1 = fabs((double) (v1 / v2)) ;
						}

						//INVERT THE VALUE IF IT IS NOT A PROPORTION 
						if( temp1 > 1.) temp1 = 1./temp1 ; 

						//IF THE PROPORTION  IS ABOVE THE THRESHOLD,  THEN USE THE VALUES
						if( temp1 > tfreqprop){
					    		// USE THEM

					    		ampweightedfreqsum = ampweightedfreqsum +	(a1 * v1) + (a2  * v2 );
					    		ampsum += a1 + a2 ;
//					    		// ADD TO THE COUNT
//					    		numberOfCommonValuesFound += 2 ; 

							collectedfbuffvalues[numberOfCommonValuesFound] = v1 ; 
							collectedfbuffvalues[numberOfCommonValuesFound + 1] = v2 ; 
							numberOfCommonValuesFound += 2 ; 							

						}
				    
					}
				}
				//pd(504);
				// IF WE HAVE ENOUGH VALUES, THEN TAKE AN AVERAGE
				// NON -1 VALUE WILL TRIGGER RETURN
				if( numberOfCommonValuesFound >= minNumValsForAverage ){
					//prt( "ENOUGH VALUES TAKING AVERAGE...." ) ; 
				    // THIS WILL WORK: TAKE THE AVERAGE
					notefreq = ampweightedfreqsum / ampsum ;				 

				}
			}
				
			// IF WE DID NOT GET A FREQUENCY CHANGE AND DO IT AGAIN
			if( notefreq == -1. ){
				// THIS DOES NOT WORK
				// IF THE BUFFER BEGIN PT IS STILL AT THE BEGINNING (beginbufferindexpoint = 0 )
				// INCREASE THE BEGINNING BUFFER SIZE TOWARDS END OF FULL BUFFER
				// OTHERWISE SHRINK IT AWAY FROM BEGINNING, (THE END ON THE END)
				if( beginbufferindexpoint == 0 ){
					// MAKE THE BUFFERSIZE LARGER
				    	begin_buffsize++ ;
				    
				    	// IF THE BUFFER HAS REACHED FULL SIZE
				    	if(  begin_buffsize > max_buffsize){
						// TOO BIG: START SHRINKING TOWARDS END
						beginbufferindexpoint = 1 ; 
						// IF THE EXPANSION THROUGH THE FULL BUFFER DID NOT ONCE
						// PRODUCE A NON 0 AMP, THEN RETURN WITH -1 FREQ
						if( zeroflag == 0 ){
							prt( "HERE! OH MY GOD!!!!" ) ; 

					    		return( notefreq ) ; 
						}
						begin_buffsize = max_buffsize - beginbufferindexpoint ; 
				    }
				}else{
					// SHRINK MORE TOWARDS END
					beginbufferindexpoint++ ; begin_buffsize = max_buffsize - beginbufferindexpoint ; 
				}
			}
			//pd(505);

		}
			  
		// THIS HAS NOT WORKED. MAKE THE THRESHOLD MORE TOLERANT
		tfreqprop *= tfreqprop ; 
		//prf( tfreqprop,  "tfreqprop" ) ; 
		// IF THE PROPORTION HAS FALLEN TOO LOW, USE THE MEDIAN AND LEAVE.
		if( tfreqprop < .75 ){
			// USE MEDIAN
			if( ampsum > 0. ) {

				//prt( "USING MEDIAN VALUE FROM THRESHOLD_MEETING VALUES" ) ; 
				n = 0 ; 
				for( i = 0; i < max_buffsize ; i++ ){
					if( abuff[i] >= ampthresh ) { collectedfbuffvalues[n] = fbuff[i] ;  n++ ; }
				} ; 
				// IF BUFFER HAS VALUES, THEN SORT AND FIND MEDIAN.
				if(n > 0){
					findArrayStats( collectedfbuffvalues, &n, 
						&low, &hi, &range, &average, &median, &mode, &standarddeviation, 
							&sum, &begin, &end, &middle, 0 ) ;
					notefreq = median ;  
				}else{
					notefreq = -1 ; 
				} ; 

			}else notefreq = -1 ;  


			return( notefreq ) ; 
		}

		// RESET THE BUFFER TO THE BEGINNING SIZE
		begin_buffsize = begin_buffsizesave ; 
	} // (END OF WHILE LOOP)

	//* IT'S MADE*************

    	return( notefreq ) ; 
	//*************************************************

}



float sine_wave( float freq,  int R ){
    
    static float vlow, v,   vhi, fracdex,  sintab[ 2048 ], ftabdex=0.,  incr ; 
    static int tabsize=2048,  first=1,  ilowdex,  ihidex ; 
    int kt ; 
    
    if( first == 1 ){
	// MAKE TABLE
	first = 0 ; 
    //prt( "INITIIALIZING SINE TABLE" ) ; 
	for( kt = 0; kt < tabsize; kt++ ){
	    sintab[ kt ] = sin((double) (TWOPI * ( (float) kt / (float) tabsize ) ) ) ; 
	    //prf( sintab[ kt ],  "sintab" ) ; 
	}
    //prt( "TABLE DONE." ) ; 
    }
    
    // GET SINE VALUE
    // MAKE TABLE INCREMENT OUT OF FREQ
    incr = freq *  (float) tabsize / (float) R ; 
    ilowdex = (int) ftabdex ; 
    ihidex = ilowdex + 1; 
    if( ihidex == tabsize ) ihidex = 0 ; 
    //pri( ihidex, "ihidex" ) ;    
    fracdex = ftabdex - (float) ilowdex ; 

    vlow = sintab[ ilowdex ] ; 
    vhi =  sintab[ ihidex ]  ; 
    v = vlow + fracdex * (vhi - vlow) ; 
    
    ftabdex = ftabdex + incr ; 
    while( ftabdex >= (float) tabsize ) ftabdex = ftabdex - (float) tabsize ; 
    return( v ) ; 
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }


float norm_value( 
    float avalue,  
    float peakenvamp, 
    float compression, 
    float ampthresh, 
    float ampgatethresh, 
    float warp   
    ){ 

    float temp3 ;
    static float shortnorm ; 
    
    shortnorm = 1. - (1./32760.) ;   
    
// NORMALIZE 
	    avalue = avalue / peakenvamp ; 

//prf( avalue, "avalue in norm_value" ) ; 

// AMP COMPRESSION
// TEST WHETHER ABOVE THRESHOLD
	    if( avalue > ampthresh ){
		avalue = (ampthresh + ((avalue - ampthresh) * compression)) ; 
	    }
// GATE
	    if( avalue > ampgatethresh ){
		// ABOVE GATE LOWER BY GATE AMP AMOUNT
		avalue = avalue - ampgatethresh ; 
	    }else{
		// BELOW GATE THRESHOLD, GATE IT
		avalue = 0 ; 
	    }
	    
	    // USING THE COMPRESSION AND GATE STUFF, NORMALIZE IT BACK TO THE ORIGINAL RANGE
	    temp3 = 1. /  ((ampthresh  + ((1. - ampthresh) * compression) ) - ampgatethresh) ;
	    avalue = avalue * temp3 *  shortnorm ; 

// WARP THE RESULTING SIGNAL
	    avalue = curve( 0.,  1.,  avalue,  warp  ) ;


 
    return( avalue ) ; 
 
 }
 




int zero_cross_peaks( float F[], float F_i[],   int Nplus2,  
	    int formant_bin[],  int NP,  int num_formants  
	    ){
    
    // TAKE SUCCESSIVE DIFFERENCES TO PLACE PEAKS AT ZERO CROSSINGS
    int i, j,  sign,  p,  breakout,  doneflag, peak_count,  PV ;
    int addflag ; 
    float peak,  minamp,  
	min_formant_dB, dB_adder;  	    
    
     
    
    doneflag = 0 ; 
    min_formant_dB = -3 ; 
     dB_adder = 10 ; 
     addflag = 0 ; 
     
    // COPY F INTO F_i, FINDING PEAK
    // ZER0 OUT DC 
    peak = -999999. ; 
    for( i = 2 ; i < Nplus2 ; i += 2 ){
	    // CHECK FOR PEAK
	   if( F[ i ] > peak ) peak = F[ i ] ;  
	   // MAKE ARRAY OF DIFFERENCES IN WHICH PEAKS AND VALLEYS ARE ZERO, AND
	   // POSITVE VALUES ARE ASCENT TO PEAK, AND NEGATIVE ARE DESCENT TO VALLEY.  
	   F_i[ i - 2 ] = F[ i ] -  F[ i - 2 ] ; // CHANGE FROM VALUE
	   F_i[ i - 1 ] = F[ i - 1 ]  ; //TRANSFER FREQ
    }

    // "peak" IS THE AMPLITUDE OF THE PEAK
    
    F_i[ Nplus2 - 2 ] = 0 ; F_i[ Nplus2 - 1 ] = F[ Nplus2 - 1 ] ;

    while( doneflag == 0 ){
	
	// CREATE THE MINIMUM FORMANT AMP (FROM dB) RELATIVE TO THE PEAK
	minamp = peak * dB_to_amp( min_formant_dB ) ; 

	sign = 1 ; p = 0 ; breakout = 0 ;

      // ANALYZE FOR ZERO CROSSINGS

      // SAVE FIRST  VALLEY BIN AS DC BIN (0)
      formant_bin[ p ] = 0 ;  PV = 1 ; 

    i = 2; j = 1; peak_count = 0 ; 
     
    while( (i < Nplus2) ){
	    if( (sign >= 0.) ){
		// WAS ASCENDING
		if(  F_i[ i ] >= 0. ){
		    // STILL ASCENDING
		}else{
		    // NOW DESCENDING: ARE WE ABOVE THE AMP THRESHOLD, THEN

		    if(  F[ i ] > minamp ){ 

			// CHANGE SIGN
			// HIT PEAK: SAVE BIN LOCATION AND INCREMENT COUNT

 
			sign = -1 ;
			peak_count++ ; 
			if( p >= (NP - 1) ) {
			    breakout = 1 ; 
			} 
			else{
			    p++ ;  
			    formant_bin[ p ] = j ; PV = 0 ; 
			}
		    }  
		}
	    }else{
		// WAS DESCENDING
		if(  F_i[ i ] < 0. ){
		    // STILL DESCENDING
		}else{
		    // NOW ASCENDING: CHANGE SIGN
		    // HIT VALLEY: SAVE BIN LOCATION AND INCREMENT COUNT
		    sign = 1 ;
		    if( p >= (NP - 1)  ) {
			breakout = 1 ; 
		    } 
		    else{
			p++ ;  formant_bin[ p ] = j ; PV = 1 ; 
		    } 
		    
		}
		
	    }
	i += 2 ; j++ ; 
      }




	peak_count = p / 2 ; 

     if( (peak_count == num_formants) 
	    || (dB_adder < 1. )
	    || (min_formant_dB < MINUMUM_FORMANT_DB )  ){

	 // EITHER WE HAVE MATCHED OR CONVERGED TO DEATH: EXIT

	    if((p - ( ( p / 2 ) * 2 )) == 0 ){
		// OK,REPLACE THE  TOP  VALLEY WITH THE TOP BIN
		formant_bin[ p ] = (Nplus2 / 2) - 1 ;
	    }else{
		//INCREMENT TO THE TOP VALLEYLESS PEAK AND ADD TOP BIN AS TOP VALLEY
		p++ ; formant_bin[ p ] = (Nplus2 / 2) - 1 ; 
	    }

	    return( peak_count ) ; 

     }else if( peak_count < num_formants  ){
	    // TOO FEW PEAKS:  LOWER dB THRESHOLD AND SEARCH AGAIN
	    if( addflag == 1 ) dB_adder = 2. * dB_adder / 3. ;
	    min_formant_dB -= dB_adder ; 
	    addflag = -1 ; 
	    doneflag = 0 ; 

      }else{
	// TOO MANY PEAKS: RAISE dB THRESHOLD AND SEARCH AGAIN
	    if( addflag == -1 ) dB_adder = 2. * dB_adder / 3. ;
	    min_formant_dB += dB_adder ; 
	    addflag = 1 ; 
	    doneflag = 0 ; 

      }


    }	

	return( peak_count ) ; 

		
  }
  
  
    
int print_peak_freqs( float F[], float fundamental,  int Nplus2,  int formant_bin[], int num_peaks ){
    
    // PRINT THE PEAK FREQUENCIES
	int i; 
	float oldfreq = 0,  diff, dB,    diffsum ; 

    diffsum = 0. ; 
/*
 * formant_bin CONTAINS THE NUMBER OF THE BINS (AMP/FREQ PAIRS) WHICH REPRESENT
 * THE VALLEYS AND PEAKS. THEY ARE ARRANGED: VALLEY PEAK VALLEY PEAK... VALLEY (extra on top)
 * 
 */

	fprintf( stderr, "\n*** PEAK FREQUENCIES ****" ) ; 
		oldfreq = F[(2 * formant_bin[ (0 * 2) + 1 ]) + 1] ; 
	for( i = 0; i <	num_peaks; i++){
	    diff = F[(2 * formant_bin[ (i * 2) + 1 ]) + 1] - oldfreq ; 
	    dB = (20. * log10( (double)  F[ 2 * formant_bin[ (i * 2) + 1 ] ] )) ; 

	    diffsum += diff ; 
	    fprintf( stderr, "\n %d) VALLEY BIN: %d,  VALLEY:%6.2f, PEAK BIN: %d,  PEAK:%6.2f, PEAKDB: %6.2f,  DIFF: %f  ",  
		i+1, formant_bin[ (i * 2)  ], F[(2 * formant_bin[ (i * 2)  ]) + 1] , 
		 formant_bin[ (i * 2) + 1 ],  F[(2 * formant_bin[ (i * 2) + 1 ]) + 1], 
		dB, diff ) ; 

	    oldfreq = F[(2 * formant_bin[ (i * 2) + 1 ]) + 1] ; 
	}

    fprintf( stderr, "\ndiff freq: %f",  (diffsum/(float) num_peaks) ) ; 

    return(1) ; 

}

int find_formants( 
    float F[], 
    float fundamental,  
    int Nplus2,  
    int formant_bin[], 
    int num_peaks, 
    float formant_freq[], 
    float formant_amp[], 
    int formant_freq_rank[]
     ){
    
    // COMPUTE THE FORMANT CENTER FREQUENCIES USING THE BINS BETWEEN THE VALLEYS.
    //AVERAGE THEIR FREQS WEIGHTED BY THE SQUARE OF THEIR AMPS. ???


    // SORT THEM VIA formant_freq_rank 

/*
 * formant_bin CONTAINS THE NUMBER OF THE BINS (AMP/FREQ PAIRS) WHICH REPRESENT
 * THE VALLEYS AND PEAKS. THEY ARE ARRANGED: VALLEY PEAK VALLEY PEAK... VALLEY (extra on top)
 * 
 */

	int i, k,  again ;
	float diffsum,  oldfreq ;  

		for( i = 0; i <	num_peaks; i++){
		
	    formant_freq[ i ] = F[(2 * formant_bin[ (i * 2) + 1 ]) + 1] ; 
	    formant_amp[ i ] = F[(2 * formant_bin[ (i * 2) + 1 ])] ; 

	}

	for( i = 0; i <	num_peaks; i++)
		formant_freq_rank[ i ] = i ; 

	// SORT
	again = 1 ; 
	
	while( again ){
	    // SORT AGAIN
	    again = 0 ; 
	    for( i = 1; i <	num_peaks; i++){
		// COMPARE
		if( formant_amp[ formant_freq_rank[ i ] ] > 
			formant_amp[formant_freq_rank[ i - 1 ]] ){
		    // SWITCH RANKING
		    k = formant_freq_rank[i - 1] ; 
		    formant_freq_rank[i - 1] = formant_freq_rank[i] ;
		    formant_freq_rank[i] = k ;
		    // DO IT AGAIN
		    again = 1 ; 
		} 
	    }
	    
	}

/*
prline( 69,   "=" ) ; 
	for( i = 0; i <	num_peaks; i++){
	    k = formant_freq_rank[i] ; 
	    fprintf( stderr,  "\n%d: FREQ: %f,  DB: %f",  
			i,  formant_freq[k],  amp_to_dB( formant_amp[k] ) ); 
	}
*/

	diffsum = 0. ; oldfreq = formant_freq[ 0 ]  ; 
	for( i = 1; i <	(num_peaks - 3); i++){
	    diffsum += (formant_freq[ i ] - oldfreq ) ; 
	    oldfreq = formant_freq[ i ] ; 
	}    
	 

    return(1) ; 
}


void optimal_comb(
	float F[], 
	int numberOfBins,
	float	*freqnow,
	float	*freqampnow,
	float dbThresh,
	float lowFreq,
	float highFreq,
	int optiomal_comb_OR_strongest_formant_FLAG,
	int mode_filter_window_size_in_frames
)
{
	static float ampThresh, *previous_F, *F_copy ; 

	static float *FramesBufferOfStrongestFormantsAsMIDI,
		*FramesBufferOfStrongestFormants, 
		*freqOutputBuffer, 
		*ampOutputBuffer ;

	static int first=1, frames=0, numVals=MAX_NUMBER_OF_STRONGEST_FORMANT_VALUES, numberInBuffers=0, zeroBins ; 
	float alternateFreq=-1., alternateAmp=0. ; 

	int bin, i, k, ampIndex, freqIndex, isUnique, breakLoop, sortedFlag, insertFlag, modeFoundFlag ; 
	float peakAmp, peakAdjustedAmpThresh, temp ;
	int  fundBin, fundAmpIndex, fundFreqIndex; 
	float thisFundamental, thisFundamentalSum;
	float strongestFundamentals[MAX_NUMBER_OF_STRONGEST_FORMANT_VALUES], strongestFundamentalSums[MAX_NUMBER_OF_STRONGEST_FORMANT_VALUES] ; 
	int numberOfContributingPartials[MAX_NUMBER_OF_STRONGEST_FORMANT_VALUES];  
 
	  
	 

	float mode, thisModeFreqDiff, absModeFreqDiff, thisOutputFreq, thisOutputAmp ; 

	int previouslyInPartialBandRegion   ; 
	float floatPartial, partialFraction, thisPartialPeakAmp   ; 


	ampThresh = dB_to_amp( dbThresh ) ; 

	*freqnow = -1. ; *freqampnow = 0. ; zeroBins = 0 ;   

	// CREATE BUFFERS
	if( first == 1 ){

		fvec( FramesBufferOfStrongestFormantsAsMIDI, mode_filter_window_size_in_frames ) ; 
		fvec( FramesBufferOfStrongestFormants, mode_filter_window_size_in_frames ) ;
		fvec( freqOutputBuffer, mode_filter_window_size_in_frames ) ;
		fvec( ampOutputBuffer, mode_filter_window_size_in_frames ) ; 

		for(i = 0; i < mode_filter_window_size_in_frames; i++)FramesBufferOfStrongestFormants[i] = 0. ; 

		fvec( previous_F, numberOfBins * 2 ) ; 
		fvec( F_copy, numberOfBins * 2 ) ; 
		first = 0 ; 


	} ; 

	// COPY F INTO COPY SPACE
	for(bin = 0, ampIndex = 0, freqIndex = 1; bin < numberOfBins; bin++, ampIndex += 2, freqIndex += 2){
		F_copy[ampIndex] = F[ampIndex] ; F_copy[freqIndex] = F[freqIndex] ; 
	}; 

	
/*
	// GIVE 0 AMP TO ANY BIN WITHOUT FREQ CONSTANCY.
	for(bin = 0, ampIndex = 0, freqIndex = 1; bin < numberOfBins; bin++, ampIndex += 2, freqIndex += 2){
		if( fabs( Hz_to_MIDI(F_copy[freqIndex]) - Hz_to_MIDI(previous_F[freqIndex]) ) > 0.02 ) {
			F_copy[ampIndex] = 0. ; zeroBins++ ; 
		} ; 
	}; 

	pri( zeroBins, "ZERO BINS" ) ; 
*/

	// ZERO DATA
	for(i = 0; i < numVals; i++){
		strongestFundamentals[i] = 0. ; strongestFundamentalSums[i] = 0. ; numberOfContributingPartials[i] = 0 ;  
	} ; 
	
//prt("c0" ) ; 

	// FIND PEAK AMP IN FRAME
	peakAmp = 0. ;
	for(bin = 0, ampIndex = 0; bin < numberOfBins; bin++, ampIndex += 2) if( F_copy[ampIndex] > peakAmp ) peakAmp = F_copy[ampIndex] ; 
	peakAdjustedAmpThresh = ampThresh * peakAmp ; 
	
//prt("c1" ) ; 

	// FOR ALL BINS . . . 
	for(fundBin = 0, fundAmpIndex = 0, fundFreqIndex = 1; fundBin < numberOfBins ; fundBin++, fundAmpIndex += 2, fundFreqIndex += 2 ){

		// EXAMINE EACH AS A POSSIBLE FUNDAMENTAL . . .
		thisFundamental = F_copy[fundFreqIndex] ; thisFundamentalSum = F_copy[fundAmpIndex] ;
 
		// IF IT IS BETWEEN THE FREQUENCY BOUNDARIES AND ABOVE THE AMP THRESHOLD . . . 
		if( (thisFundamental >= lowFreq) && (thisFundamental <= highFreq) &&  (thisFundamentalSum >= peakAdjustedAmpThresh) ){

			// TEST THIS FUNDAMENTAL AGAINST ALL IN LIST FOR UNIQUE FREQUENCY AND OR GREATER AMP
			// AGAINST COMPARABLE FREQUENCY; IF GREATER, THEN SUBSTITUTE AND SORT LIST.
			isUnique = 1 ; i = 0; breakLoop = 0 ; 
			while( (i < numVals) && (breakLoop == 0) ){
				if( strongestFundamentals[i] > 0. ){
					temp = fabs(Hz_to_MIDI( thisFundamental ) - Hz_to_MIDI( strongestFundamentals[i] ))  ;
					if( temp <= 0.25 ){
						// NOT A UNIQUE FREQ; TEST FOR GREATER AMP
						isUnique = 0 ; // NOT UNIQUE
						if(thisFundamentalSum > strongestFundamentalSums[i]){
							// GREATER AMP: SUBSTITUTE
							strongestFundamentals[i] = thisFundamental ; 
							strongestFundamentalSums[i] = thisFundamentalSum ; 
							// AND SORT AGAINST AMPLITUDE
							sortedFlag = 0 ; 
							while(sortedFlag == 0){
								sortedFlag = 1 ; 
								for(k = 1; k < numVals; k++){
									if(strongestFundamentalSums[k] > strongestFundamentalSums[k - 1]){
										temp = strongestFundamentalSums[k - 1] ; 
										strongestFundamentalSums[k - 1] = strongestFundamentalSums[k] ; 
										strongestFundamentalSums[k] = temp ; 

										temp = strongestFundamentals[k - 1] ; 
										strongestFundamentals[k - 1] = strongestFundamentals[k] ; 
										strongestFundamentals[k] = temp ; 

										sortedFlag = 0 ; 
									}; 
								} ;  
							} ; 
						} ; 
						breakLoop = 1 ; 
					} ; 
				} ;
				i++ ;  
			} ; 


			// PLACE THIS UNIQUE FUNDAMENTAL INTO RANKED LIST
			if( isUnique == 1 ){		
				insertFlag = 0 ; k = 0 ; 
				while( (insertFlag == 0) && (k < numVals) ){
					if( thisFundamentalSum > strongestFundamentalSums[k] ){ 
						insertFlag = 1 ; 
						for(i = (numVals - 1); i > k; i--) {
							strongestFundamentalSums[i] = strongestFundamentalSums[i - 1] ; 
							strongestFundamentals[i] = strongestFundamentals[i - 1] ; 
						} ; 
						strongestFundamentalSums[k] = thisFundamentalSum ; 
						strongestFundamentals[k] = thisFundamental ; 
					} ;
					k++ ;  
				} ; 			
			} ; 	
		} ; 

//prt("c10" ) ; 

	} ;   // END OF LOOP FOR LOOKING AT BINS 

 	// IF OPTIMAL COMB METHOD, ADD AMPS OF PARTIALS ABOVE
	if( optiomal_comb_OR_strongest_formant_FLAG == 0 ){

		k = 0; // INDEX FOR DETECTED STRONG FORMANTS
		// FOR ANY NON-ZERO FUNDAMENTALS . . .
		while( strongestFundamentals[k] > 0 ){

//prt("c110" ) ; 
			previouslyInPartialBandRegion = 0 ;  
			// LOOK AT ALL BINS . . . 
			for(bin = 0; bin < numberOfBins; bin++ ){
				floatPartial = F_copy[ (bin * 2) + 1] / strongestFundamentals[k] ; 
				if( (floatPartial > 1.5) && (floatPartial < 7.5)   ){
//prt("c111" ) ; 
					// IN RANGE FOR POSSIBLE PARTIALS
					partialFraction = fabs( floatPartial - (float)(int)(floatPartial + 0.5) ) ; 
					if( partialFraction < 0.05 ){
//prt("c112" ) ; 
						// NOW IN A PARTIAL REGION (WHETHER PREVIOUSLY SO OR NOT.)
						if( previouslyInPartialBandRegion == 0 ){
//prt("c113" ) ; 
							// WERE NOT PREVIOUSLY IN PARTIAL BAND; SET FLAG SO THAT WE WILL BE NEXT TIME. 
							previouslyInPartialBandRegion = 1 ;
							// SAVE AMP AS PEAK FOR THIS BAND
							thisPartialPeakAmp = F_copy[bin * 2];  
						}else{
							// WERE IN PARTIAL BAND BEFORE
							if( F_copy[bin * 2] > thisPartialPeakAmp ) thisPartialPeakAmp = F_copy[bin * 2] ; 
						} ; 
					}else{
						// NOT IN A PARTIAL REGION (WHETHER PREVIOUSLY OR NOT).
						if( previouslyInPartialBandRegion == 1 ){
//prt("c114" ) ; 
							// WERE PREVIOUSLY IN A REGION ; SET FLAG SO THAT WE WILL NOT BE IN A REGION NEXT TIME. 
							previouslyInPartialBandRegion = 0 ; 
							// ADD AMP TO SUM
							strongestFundamentalSums[k] += thisPartialPeakAmp ; thisPartialPeakAmp = 0. ; 
						}else{
							// NOT PREVIOUSLY IN REGION; DO NOTHING
						} ; 
					} ; 
//prt("c115" ) ; 
				} ; 
			}
			k++ ; 
		};
//prt("c12" ) ; 

		// AND RE-SORT AGAINST AMPLITUDE
		sortedFlag = 0 ; 
		while(sortedFlag == 0){
			sortedFlag = 1 ; 
			for(k = 1; k < numVals; k++){
				if(strongestFundamentalSums[k] > strongestFundamentalSums[k - 1]){
					temp = strongestFundamentalSums[k - 1] ; 
					strongestFundamentalSums[k - 1] = strongestFundamentalSums[k] ; 
					strongestFundamentalSums[k] = temp ; 

					temp = strongestFundamentals[k - 1] ; 
					strongestFundamentals[k - 1] = strongestFundamentals[k] ; 
					strongestFundamentals[k] = temp ; 

					sortedFlag = 0 ; 
				}; 
			} ;  
		} ; 
	} ; 
//prt("c2" ) ; 


	// IF WE FOUND ONE . . .
	if( strongestFundamentalSums[0] != 0. ){
		// FOUND A FUNDAMENTAL

		// IF OPTIMAL COMB METHOD
		if( optiomal_comb_OR_strongest_formant_FLAG == 0 ){
			// TEST FOR FUNDAMENTALS BENEATH
			alternateFreq = strongestFundamentals[0] ; alternateAmp = strongestFundamentalSums[i] ;  
			for(i = 1; i < numVals; i++){
				if( strongestFundamentals[i] < strongestFundamentals[0] ){
					temp = strongestFundamentals[0] / strongestFundamentals[i] ;
					temp = fabs( temp - (float) (int) (temp + 0.5) ) ;  
					if(temp < .05){
						if( strongestFundamentals[i] < alternateFreq ){
							alternateFreq = strongestFundamentals[i] ; 
							alternateAmp = strongestFundamentalSums[i] ; 
						} ; 
					} ; 
				} ; 
			} ; 
		} ; 

		// SHIFT BUFFER VALUES RIGHT
		for(i = mode_filter_window_size_in_frames - 1; i > 0; i--) 
				FramesBufferOfStrongestFormants[i] = FramesBufferOfStrongestFormants[i - 1] ;  

		if( optiomal_comb_OR_strongest_formant_FLAG == 0 ){
			// INSERT ALTERNATE FREQ  INTO 0 POSITION (CURRENT FRAME); 
			FramesBufferOfStrongestFormants[0] = alternateFreq ; 
		}else{
			// INSERT STRONGEST FUNDAMENTAL INTO 0 POSITION (CURRENT FRAME)
			FramesBufferOfStrongestFormants[0] = strongestFundamentals[0] ;
		} ; 

		//	INCREMENT NUMBER IN BUFFER AND CLIP IF NECESSARY.
		 numberInBuffers++ ;		
		if( numberInBuffers > mode_filter_window_size_in_frames ) numberInBuffers = mode_filter_window_size_in_frames ;  

//pri( numberInBuffers, "numberInBuffers" ) ; 
//	prt( "" ) ; 
//	for(i = 0; i < numberInBuffers; i++)prf( FramesBufferOfStrongestFormants[i], "FramesBufferOfStrongestFormants" ) ; 
//	prt( "" ) ; 


		// DETECTION OF A MODE IN SERIES OF FRAMES, FIVE MINIMUM.
		if( numberInBuffers > 5 ){
//prt("finding mode" ) ; 
			// ENOUGH IN BUFFER FOR MODE DETECTION

			// CONVERT FREQS TO MIDI TO EQUALIZE MODE MERGER FACTOR ACROSS RANGE.
			for(i = 0; i < numberInBuffers; i++) FramesBufferOfStrongestFormantsAsMIDI[i] = Hz_to_MIDI( FramesBufferOfStrongestFormants[i] ); 
			// FIND MODE AND CONVERT IT TO HERTZ.
			mode = findMode( FramesBufferOfStrongestFormantsAsMIDI, numberInBuffers, 0.25, &modeFoundFlag ) ; mode = MIDI_to_Hz( mode ); 
			// USE DETECTED FORMANT THAT IS CLOSEST TO MODE;  THAT IS, IF IT EXISTS.
			// FIND FORMANT THAT IS CLOSEST TO MODE. 
			absModeFreqDiff = fabs( strongestFundamentals[0] - mode ) ;
			thisOutputFreq =  strongestFundamentals[0] ; thisOutputAmp = strongestFundamentalSums[0] ;
			for(i = 1; i < MAX_NUMBER_OF_STRONGEST_FORMANT_VALUES; i++){
				thisModeFreqDiff = fabs( strongestFundamentals[i] - mode ) ; 
				if(	thisModeFreqDiff < absModeFreqDiff ){
					absModeFreqDiff = thisModeFreqDiff ; thisOutputFreq = strongestFundamentals[i] ; 
					thisOutputAmp = strongestFundamentalSums[i] ; 
				} ; 
			} ; 
			if( fabs( Hz_to_MIDI( thisOutputFreq ) - Hz_to_MIDI( mode ) ) <= 1. ){
				// ACCEPTABLE FORMANT; USE
				*freqnow = thisOutputFreq ; *freqampnow = thisOutputAmp ; 
			}else{
				// UNACCEPTABLE; GENERATE ACCEPTABLE FREQ FROM BUFFER OF PREVIOUS OUTPUTS
				*freqnow = freqOutputBuffer[0] + (freqOutputBuffer[0] - freqOutputBuffer[1]) ;
				*freqampnow =  ampOutputBuffer[0] + (ampOutputBuffer[0] - ampOutputBuffer[1]) ;			
			}  ; 
//prf( *freqnow, "freqnow in optimal comb" ) ; 
		}else{
			// NOT ENOUGH; OUTPUT STRONGEST FORMANT
			*freqnow = strongestFundamentals[0] ; *freqampnow =  strongestFundamentalSums[0] ; 			
		} ; 



	}else{
		// NO ACCEPTABLE FORMANT FOUND; OUTPUT LAST ONE OUTPUT
		*freqnow = freqOutputBuffer[0] ; *freqampnow =  ampOutputBuffer[0] ; 			

	} ; 
//prt("c3" ) ; 


	// SHIFT AND INSERT INTO OUTPUT BUFFERS
	for(i = mode_filter_window_size_in_frames - 1; i > 0; i--){
		freqOutputBuffer[i] = freqOutputBuffer[i - 1] ; ampOutputBuffer[i] = ampOutputBuffer[i - 1] ; 
	} ; 
	freqOutputBuffer[0] = *freqnow ;ampOutputBuffer[0] = *freqampnow ; 


	// TRANSFER F INTO previous_F

	for(bin = 0, i = 0, k = 1; bin < numberOfBins; bin++, i += 2, k += 2){
		previous_F[i] = F_copy[i] ; previous_F[k] = F_copy[k] ; 
	}; 

	frames++ ; 

/*
	prt( "********" ) ; 
	for(i = 0; i < numVals; i++) fprintf( stderr, "\n(%d): %f  %f dB", 
			i, strongestFundamentals[i], amp_to_dB(strongestFundamentalSums[i]) ) ; 
	prf( *freqnow, "*freqnow" ) ; 
*/ 	

} ;  



void find_freq_new( 
    
	float	*freqnow,
	float	*freqampnow,

    	int detect_method,
    	float  lowf, 
    	float hif,
    	float formant_freq[], 
    	float  formant_amp[],
    	int formant_freq_rank[], 
    	int num_peaks 
    
){

    float temp, tempAmp ; 
    int i; 
    
    *freqnow = -1. ; *freqampnow = 0. ; 

    	if( detect_method == 0 ){    


		// USE OPTIMAL COMB METHOD ON FORMANTS
	
		 
		static float *sums ; 
		int i, j, k, numberOfOverPartials=5, opartial, overPartial, freqAmpIndex ; 
		float temp; 
	
	    	fvec( sums, num_peaks * numberOfOverPartials ) ; 
	
		for( i = 0; i < num_peaks; i++ ){
	    		// MAKE SUM FOR EACH FORMANT
	    
			for( opartial = 0; opartial < numberOfOverPartials ; opartial++ ){
	    			sums[ (i * numberOfOverPartials) + opartial ] = 0. ; 
				for( k = i; k < num_peaks ; k++){
					temp = formant_freq[ k ] / (formant_freq[ i ] / (float)(opartial + 1)) ; 
					temp = (  2. * ( temp - (float) ((int)temp) )  ) - 1.  ; // -1 to 1
//					if( temp < 0. ) temp *= -1. ;
					temp = temp * temp ;  
					sums[ (i * numberOfOverPartials) + opartial ] += temp ; 

	    			}
			}
		}    

/*			// PRINT
		for( i = 0; i < num_peaks ; i++){
			for(opartial = 0; opartial < numberOfOverPartials; opartial++)
	    			fprintf( stderr, "\n%d: FREQ: %f, AS PARTIAL: %d, SUM: %f",  
					i, formant_freq[ i ], opartial + 1,   sums[ i ] ) ; 
		}
*/
 
		// FIND OUT WHICH ONE IS THE STRONGEST
		overPartial = 1 ; freqAmpIndex = 0 ; 
		for( i = 1; i < num_peaks ; i++){
			for(opartial = 0; opartial < numberOfOverPartials; opartial++){
				j = (i * numberOfOverPartials) + opartial ; 
	    			if( sums[ j ] > sums[k] ){
					overPartial = opartial + 1 ; freqAmpIndex = i ;   
				} ; 
			} 
		}

    
		*freqnow = formant_freq[ freqAmpIndex ] / (float) overPartial ;
		*freqampnow = formant_amp[ freqAmpIndex ] ;  

    	}else{
		// STRONGEST FREQ
		
		for( i = 0; i < num_peaks ; i++){
	    
	    		temp = formant_freq[ formant_freq_rank[ i ] ] ;
			tempAmp = formant_amp[ formant_freq_rank[ i ] ] ;  

	    		if( (temp >= lowf) && (temp <= hif ) ){
				*freqnow = temp ; 
	    			*freqampnow = tempAmp ; 
			} ; 

	    		if( *freqnow != -1. ) break ; 

		}
	

	}

} 

