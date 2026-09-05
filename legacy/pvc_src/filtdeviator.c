#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k, jj,   i1,  i2 ;
float i1p,  i2p,  pm,  fm,  fs,  fdbm, fdpm, fdmdiff,  fd,  fsdiff,  fdevminus ; 
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  channelout=0 ;
float P = 1.0;
int SOURCE_maxNumOfDelayFrames, FILTER_maxNumOfDelayFrames ; 
float SOURCE_maxDelayT=0., FILTER_maxDelayT=0. ; 
float peakDelayTime, baseDelayTime,  maxDecayTime, feedbackDecayTimeRange ; 
float interpDecayTFilterAmp ; 
float funcMin, funcMax, funcAvg, maxDelayT=0., timeRange, timeDelayFeedbackAmp, *thisFilterDelayT,
	*previous_thisFilterDelayT, 
	*thisFilterDelayT_inFrames, *thisFilterDecayT, *thisFilterDecayT_inFrames,  
	interpTdelayFilterAmp, this_freqdevmode, thisdBval, this_randelayTval ; 
int maxNumOfDelayFrames, SOURCE_frameNowChannelDelayIndex, FILTER_frameNowChannelDelayIndex, 
	thisFilterFrameDelay, thisSourceFrameDelay, sourceflag=0 ; 
int randomDelayTflag=0 ; 
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *bufferSum, 
			*buffer_filter, *buffer_filter_delayed_inputs, *channel, 
	*channel_filter, *channel_filter_delayed_inputs, 
		*SOURCE_channel_delay, *FILTER_channel_delay, *FILTER_channel_delay_now, *output ;
float *previous_channel_filter,  *F,  *Ffreq, *FtimeDelay, *FampWarp,  *FfreqWarp, *FtimeDelayWarp,
	* FdecayTimeWarp, 
	 *previous_ranfreqv,  *previous_ranampv ; 
float threshfac = .001,  threshfacdB=-96 ;
float releasec,  minusreleasec,  attackc,  minusattackc ; 
double ar_dB ; 
float	gain=1. ;
int bandrejecton=0 ; 
FILE *fopen();
FILE *fp;
float randelayTsmoothc, minusrandelayTsmoothc ; 


float peakdBnow,  basedBnow,  ampdiffnow,  dBdiffnow,  fdevdiff ; 

float channel_outAmpSum, tempChannelAmpSum, filterChannelAmpSum,
	normgain, frameNormalizationAmpLimit, normalizationAmp, Normalize_to__Input_Sound_0__Filter_1=0 ; 

int print_flag=0 ; 
int pitchflag=0; 
char ch;
// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 

float this_ranampval, this_interpFilterAmp, this_interpFilterFreq, this_smoothedrandomval, this_smoothedfileval,
	fullydeviatedfreq ; 
float  temp,  temp2,  temp3,  temp4 ;  
float getthresh();
float   IR,  dur=0.;

int ranfreqdevswitch=0 ; 
int ranampdevswitch=0 ; 
int ranabovebelowswitch=0 ; 

float  ranfreqsmoothc,  minusranfreqsmoothc ; 

float  ranampsmoothc,  minusranampsmoothc ; 

float fundamental, factor,   sourceamp,  filtamp ;  

char tempstring[ STRING_SIZE ] ; 

// FUNCTION DELAY TIME SCALER
struct func Function_Delay_Time_Scaler ; 


// WARPS
struct  func  ampwarpshape ; 
struct  func  freqwarpshape  ;
struct  func  timedelaywarpshape ; 
struct func decaytimewarpshape ; 

struct func FILTER_OUTPUT_randelayTdevresponse ; 

// SOURCE
struct  func  SOURCE_dB ; // OK
struct  func  SOURCE_fshift ; // OK 
struct  func  SOURCE_delayT ; // OK
struct  func  SOURCE_ptrans ; // OK


// COMB FREQUENCY SHIFT ADDER
struct  func  FILTER_OUTPUT_harmadd ; 

// GAIN
struct  func  FILTER_OUTPUT_dBgain ; 

// PITCH SEMITONE TRANSPOSE
struct  func  FILTER_OUTPUT_ptrans ; 

// FILTER FREQ RESPONSE
struct  func  FILTER_freq_response ; 

// FILTER SEMITONE TRANSPOSE
struct  func  FILTER_FREQ_RESPONSE_ftrans ; 

// FILTER TRANSPOSE SHIFTER
struct  func  FILTER_FREQ_RESPONSE_fshift ; 

//  FILTER SOURCE DECIBELS FLOOR
struct  func  FILTER_dB_source_floor ; 

//  FILTER OUTPUT RELEASE
struct  func  FILTER_OUTPUT_release ; 

//  FILTER OUTUT ATTACK
struct  func  FILTER_OUTPUT_attack ; 

// **** FREQ DEVIATION: BASE
struct  func  FILTER_OUTPUT_fdevbase ; 

// **** FREQ DEVIATION: PEAK
struct  func  FILTER_OUTPUT_fdevpeak ; 

// **** FREQ DEVIATION SHIFT: BASE
struct  func  FILTER_OUTPUT_fshiftbase ; 

// **** FREQ DEVIATION SHIFT: PEAK
struct  func  FILTER_OUTPUT_fshiftpeak ; 

// **** RANDOM FREQ DEVIATION RESPONSE TIME
struct  func  FILTER_OUTPUT_ranfreqdevresponse ; 

// **** RANDOM AMP DEVIATION RESPONSE TIME
struct  func  FILTER_OUTPUT_ranampdevresponse ; 

// **** FREQ DEVIATION: MASTER DEVIATION CONTROL
struct  func  FILTER_OUTPUT_fdevcontrol ; 

// **** TIME DELAY: MASTER DEVIATION CONTROL
struct  func  FILTER_OUTPUT_timeDelayDevControl ; 

// TIME DELAY BASE
struct func FILTER_OUTPUT_timeDelayBase ; 

// TIME DELAY PEAK
struct func FILTER_OUTPUT_timeDelayPeak ; 


// **** FREQ DEVIATION MODE
struct  func  FILTER_OUTPUT_freqdevmode ; 


struct  func  FILTER_OUTPUT_peak_decay_time_in_seconds ; 
struct  func  FILTER_OUTPUT_base_decay_time_in_seconds ; 
struct func FILTER_OUTPUT_decayTimeDevControl ; 

// FRAME NORMALIZATION DECIBEL LIMIT
struct  func  FILTER_OUTPUT_frameNormalizationDecibelLimit ; 



// *****************INITIALIZE

// FUNCTION DELAY TIME SCALER
Function_Delay_Time_Scaler.L = 1. ; Function_Delay_Time_Scaler.n = 1. ; Function_Delay_Time_Scaler.A[ 0 ] = 1. ; 

FILTER_OUTPUT_randelayTdevresponse.L = 1. ; FILTER_OUTPUT_randelayTdevresponse.n = 1. ;
	FILTER_OUTPUT_randelayTdevresponse.A[ 0 ] = 0. ;

// WARPS
ampwarpshape.L = 1. ; ampwarpshape.n = 1. ; ampwarpshape.A[ 0 ] = 0. ;
freqwarpshape.L = 1. ; freqwarpshape.n = 1. ; freqwarpshape.A[ 0 ] = 0. ;
timedelaywarpshape.L = 1. ; timedelaywarpshape.n = 1. ; timedelaywarpshape.A[ 0 ] = 0. ;
decaytimewarpshape.L = 1. ; decaytimewarpshape.n = 1. ; decaytimewarpshape.A[ 0 ] = 0. ;



// SOURCE
SOURCE_dB.L = 1. ; SOURCE_dB.n = 1. ; SOURCE_dB.A[ 0 ] = 0. ;
SOURCE_fshift.L = 1. ; SOURCE_fshift.n = 1. ; SOURCE_fshift.A[ 0 ] = 0. ;
SOURCE_delayT.L = 1. ; SOURCE_delayT.n = 1. ; SOURCE_delayT.A[ 0 ] = 0. ;
SOURCE_ptrans.L = 1. ; 
	SOURCE_ptrans.n = 1. ; 
		SOURCE_ptrans.A[ 0 ] = 0. ;

 

// FILTER OUTPUT
// FREQ SHIFT
FILTER_OUTPUT_harmadd.L = 1. ; FILTER_OUTPUT_harmadd.n = 1. ; FILTER_OUTPUT_harmadd.A[ 0 ] = 0. ; 

// GAIN
FILTER_OUTPUT_dBgain.L = 1. ;  FILTER_OUTPUT_dBgain.n = 1. ; FILTER_OUTPUT_dBgain.A[ 0 ] = 0. ; 

// PITCH SEMITONE TRANSPOSE
FILTER_OUTPUT_ptrans.L = 1. ; FILTER_OUTPUT_ptrans.n = 1. ; FILTER_OUTPUT_ptrans.A[ 0 ] = 0. ; 

// FILTER FREQ RESPONSE
FILTER_freq_response.L = 1. ; FILTER_freq_response.n = 0. ; FILTER_freq_response.A[ 0 ] = 0. ; 
FILTER_FREQ_RESPONSE_ftrans.L = 1. ; FILTER_FREQ_RESPONSE_ftrans.n = 1. ; FILTER_FREQ_RESPONSE_ftrans.A[ 0 ] = 0. ; 
FILTER_FREQ_RESPONSE_fshift.L = 1. ; FILTER_FREQ_RESPONSE_fshift.n = 1. ; FILTER_FREQ_RESPONSE_fshift.A[ 0 ] = 0. ; 

//  FILTER SOURCE DECIBELS FLOOR
FILTER_dB_source_floor.L = 1. ; FILTER_dB_source_floor.n = 1. ; FILTER_dB_source_floor.A[ 0 ] = -96. ; 

FILTER_OUTPUT_release.L = 1. ; FILTER_OUTPUT_release.n = 1. ; FILTER_OUTPUT_release.A[ 0 ] = 0. ; 
FILTER_OUTPUT_attack.L = 1. ; FILTER_OUTPUT_attack.n = 1. ; FILTER_OUTPUT_attack.A[ 0 ] = 0. ; 

// **** FREQ DEVIATION: BASE
FILTER_OUTPUT_fdevbase.L = 1. ; FILTER_OUTPUT_fdevbase.n = 1. ; FILTER_OUTPUT_fdevbase.A[ 0 ] = 0. ; 

// **** FREQ DEVIATION: PEAK
FILTER_OUTPUT_fdevpeak.L = 1. ; FILTER_OUTPUT_fdevpeak.n = 1. ; FILTER_OUTPUT_fdevpeak.A[ 0 ] = 0. ; 


// **** FREQ DEVIATION SHIFT: BASE
FILTER_OUTPUT_fshiftbase.L = 1. ; FILTER_OUTPUT_fshiftbase.n = 1. ; FILTER_OUTPUT_fshiftbase.A[ 0 ] = 0. ; 

// **** FREQ DEVIATION SHIFT: PEAK
FILTER_OUTPUT_fshiftpeak.L = 1. ; FILTER_OUTPUT_fshiftpeak.n = 1. ; FILTER_OUTPUT_fshiftpeak.A[ 0 ] = 0. ; 


// **** FREQ DEVIATION: MASTER DEVIATION CONTROL
FILTER_OUTPUT_fdevcontrol.L = 1. ; FILTER_OUTPUT_fdevcontrol.n = 1. ; FILTER_OUTPUT_fdevcontrol.A[ 0 ] = 1. ; 

// **** TIME DELAY: MASTER DEVIATION CONTROL
FILTER_OUTPUT_timeDelayDevControl.L = 1. ; FILTER_OUTPUT_timeDelayDevControl.n = 1. ; FILTER_OUTPUT_timeDelayDevControl.A[ 0 ] = 1. ; 


// TIME DELAY BASE
FILTER_OUTPUT_timeDelayBase.L = 1. ; FILTER_OUTPUT_timeDelayBase.n = 1. ; FILTER_OUTPUT_timeDelayBase.A[ 0 ] = 0. ; 

// TIME DELAY PEAK
FILTER_OUTPUT_timeDelayPeak.L = 1. ; FILTER_OUTPUT_timeDelayPeak.n = 1. ; FILTER_OUTPUT_timeDelayPeak.A[ 0 ] = 0. ; 


// **** RANDOM FREQ DEVIATION RESPONSE TIME
FILTER_OUTPUT_ranfreqdevresponse.L = 1. ; FILTER_OUTPUT_ranfreqdevresponse.n = 1. ; FILTER_OUTPUT_ranfreqdevresponse.A[ 0 ] = 0. ; 

// **** RANDOM AMP DEVIATION RESPONSE TIME
FILTER_OUTPUT_ranampdevresponse.L = 1. ; FILTER_OUTPUT_ranampdevresponse.n = 1. ; FILTER_OUTPUT_ranampdevresponse.A[ 0 ] = 0. ; 

// **** FREQ DEVIATION MODE
FILTER_OUTPUT_freqdevmode.L = 1. ; FILTER_OUTPUT_freqdevmode.n = 1. ; FILTER_OUTPUT_freqdevmode.A[ 0 ] = 0. ; 


FILTER_OUTPUT_base_decay_time_in_seconds.L = 1. ; FILTER_OUTPUT_base_decay_time_in_seconds.n = 1. ;
	 FILTER_OUTPUT_base_decay_time_in_seconds.A[ 0 ] = 0. ; 
FILTER_OUTPUT_peak_decay_time_in_seconds.L = 1. ; FILTER_OUTPUT_peak_decay_time_in_seconds.n = 1. ;
	 FILTER_OUTPUT_peak_decay_time_in_seconds.A[ 0 ] = 0. ; 
FILTER_OUTPUT_decayTimeDevControl.L = 1. ; FILTER_OUTPUT_decayTimeDevControl.n = 1. ;
	 FILTER_OUTPUT_decayTimeDevControl.A[ 0 ] = 1. ; 



// FRAME NORMALIZATION DECIBEL LIMIT
FILTER_OUTPUT_frameNormalizationDecibelLimit.L = 1. ; 
	FILTER_OUTPUT_frameNormalizationDecibelLimit.n = 1. ; FILTER_OUTPUT_frameNormalizationDecibelLimit.A[ 0 ] = 0. ; 


if( argc < 2 )usage() ; 

//CASE -> USAGE//   

    while( (ch= crack( argc, argv, 
	"~|/|:|_|=|@|a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|W|x|X|y|Y|z|Z|", 0  )) != CRACK_DONE_FLAG ) {  //    
	switch(ch) {

	    case '/':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_peak_decay_time_in_seconds.fp = 
				crackstring( tempstring, &FILTER_OUTPUT_peak_decay_time_in_seconds ); 
			break;
	    case 'Y':   randomDelayTflag = (int) crackfloat( arg_option, ch ) ; 
			break;
	    case 'h':   strcpy(tempstring, arg_option);
			SOURCE_dB.fp = crackstring( tempstring, &SOURCE_dB ); 
			break;
	    case 'q':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_timeDelayDevControl.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_timeDelayDevControl );
			break;
	    case 'r':   strcpy(tempstring, arg_option);
			SOURCE_fshift.fp = 
				crackstring( tempstring, &SOURCE_fshift ); 
			break;
	    case 's':   strcpy(tempstring, arg_option);
			SOURCE_delayT.fp = crackstring( tempstring, &SOURCE_delayT ); 
			break;
	    case 'y':   strcpy(tempstring, arg_option);
			SOURCE_ptrans.fp = 
			   crackstring( tempstring, &SOURCE_ptrans ); 
			break;
	    case 'z':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_randelayTdevresponse.fp = 
				crackstring( tempstring, &FILTER_OUTPUT_randelayTdevresponse ); 
			break;
	    case '~':   strcpy(tempstring, arg_option);
			decaytimewarpshape.fp = crackstring( tempstring, 
			    &decaytimewarpshape );
			break;

// 

	    case '@':   strcpy(tempstring, arg_option);
			Function_Delay_Time_Scaler.fp = crackstring( tempstring, 
			    &Function_Delay_Time_Scaler );
			break;





	    case ':':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_base_decay_time_in_seconds.fp = 
				crackstring( tempstring, &FILTER_OUTPUT_base_decay_time_in_seconds ); 
			break;

	    case 'B':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_decayTimeDevControl.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_decayTimeDevControl );
			break;











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

           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;


	    case 'P':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_ptrans.fp = crackstring( tempstring, &FILTER_OUTPUT_ptrans ); 
			break;
	    case 'a':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_harmadd.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_harmadd );
			break;
	    case 'A':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_dBgain.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_dBgain );
			break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'H':   dBlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'X':   dBhi = crackfloat( arg_option, ch ) ;
			break;
	    case 'm':   freqlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'R':   freqhi = crackfloat( arg_option, ch ) ;
			break;

	    case 'k':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_fdevbase.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_fdevbase );
			break;
	    case 'K':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_fdevpeak.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_fdevpeak );
			break;

	    case 'u':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_fshiftbase.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_fshiftbase );
			break;
	    case 'U':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_fshiftpeak.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_fshiftpeak );
			break;



	    case 'j':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_timeDelayBase.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_timeDelayBase );
			break;
	    case 'J':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_timeDelayPeak.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_timeDelayPeak );
			break;

	    case 'O':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_fdevcontrol.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_fdevcontrol );
			break;


            case 'p':	quiet = (int) crackfloat( arg_option, ch )      ; break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;
            case 'Z':	print_flag = (int) crackfloat( arg_option, ch ) ; break;



	    case 'F':   strcpy(tempstring, arg_option);
			FILTER_freq_response.fp = crackstring( tempstring, 
			    &FILTER_freq_response );
			break;

            case 'G':	bandrejecton = (int) crackfloat( arg_option, ch ) ; break;


	    case 'T':   strcpy(tempstring, arg_option);
			FILTER_FREQ_RESPONSE_ftrans.fp = crackstring( tempstring, 
			    &FILTER_FREQ_RESPONSE_ftrans );
			break;
	    case 'V':   strcpy(tempstring, arg_option);
			FILTER_FREQ_RESPONSE_fshift.fp = crackstring( tempstring, 
			    &FILTER_FREQ_RESPONSE_fshift );
			break;
	    case 'S':   strcpy(tempstring, arg_option);
			FILTER_dB_source_floor.fp = crackstring( tempstring, 
			    &FILTER_dB_source_floor );
			break;

	    case 'L':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_release.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_release );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_attack.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_attack );
			break;


	   case 'g':	strcpy(afile, arg_option); break;


	    case 'W':   strcpy(tempstring, arg_option);
			ampwarpshape.fp = crackstring( tempstring, 
			    &ampwarpshape );
			break;
	    case 'v':   strcpy(tempstring, arg_option);
			freqwarpshape.fp = crackstring( tempstring, 
			    &freqwarpshape );
			break;
	    case 'o':   strcpy(tempstring, arg_option);
			timedelaywarpshape.fp = crackstring( tempstring, 
			    &timedelaywarpshape );
			break;



	    case 'E':	strcpy(tempstring, arg_option);
			FILTER_OUTPUT_freqdevmode.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_freqdevmode );
			break;
	    case 'Q':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_ranfreqdevresponse.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_ranfreqdevresponse );
			break;

	    case 'c':	ranampdevswitch = (int) crackfloat( arg_option, ch ) ;
	    case 'd':    strcpy(tempstring, arg_option);
			FILTER_OUTPUT_ranampdevresponse.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_ranampdevresponse );
			break;
	    case 'f':	ranabovebelowswitch = (int) crackfloat( arg_option, ch ) ;
			break ; 

	    case 'n':   strcpy(tempstring, arg_option);
			FILTER_OUTPUT_frameNormalizationDecibelLimit.fp = crackstring( tempstring, 
			    &FILTER_OUTPUT_frameNormalizationDecibelLimit );
			break;

	    case 'x':   Normalize_to__Input_Sound_0__Filter_1 = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	}
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "FILDEVIATOR", 69 ) ; 
prline( 69,  "-" ) ; 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
 

    sourceflag = ( (SOURCE_dB.A[0] > -96.0) || ( SOURCE_dB.n != 1.) ) ? 1 : 0 ;  


   // FIND MAX DELAY TIME
    findFuncMinMaxAvg( &FILTER_OUTPUT_timeDelayBase, &funcMin, &funcMax, &funcAvg ); 
    if( funcMax > FILTER_maxDelayT ) FILTER_maxDelayT = funcMax ; 
    findFuncMinMaxAvg( &FILTER_OUTPUT_timeDelayPeak, &funcMin, &funcMax, &funcAvg ); 
    if( funcMax > FILTER_maxDelayT ) FILTER_maxDelayT = funcMax ; 
  
   // COMPUTE FEEDBACK RING TIME SPACE
    findFuncMinMaxAvg( &FILTER_OUTPUT_peak_decay_time_in_seconds, &funcMin, &maxDecayTime, &funcAvg );  
    findFuncMinMaxAvg( &FILTER_OUTPUT_base_decay_time_in_seconds, &funcMin, &funcMax, &funcAvg );  
    if (funcMax > maxDecayTime) maxDecayTime = funcMax ; 

    ringTime = FILTER_maxDelayT = FILTER_maxDelayT + maxDecayTime ; 

    if( sourceflag == 1 ) {
	findFuncMinMaxAvg( &SOURCE_delayT, &funcMin, &SOURCE_maxDelayT, &funcAvg ) ; 
	ringTime = (SOURCE_maxDelayT > ringTime) ? SOURCE_maxDelayT : ringTime ; 
    } ; 


prf( ringTime, "ringTime" ) ; 

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

// ******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
    if( Nw < I ){
	// INCREASE WINDOW SIZE TO ACCOMODATE INTERPOLATION
	Nw = 2 ; while( Nw <= I )Nw *= 2 ;
	prt( "\n----> INCREASING WINDOW SIZE TO ACCOMODATE TIME RESYNTHESIS INTERPOLATION. <---" ) ;
	pri( Nw,  "NEW WINDOW SIZE" ) ; 
    }
// *********************************
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    IR = (float) I / (float) R ; 
    fundamental = (float) R / (float) N ; 
    factor = (float) R / ((float) D * TWOPI) ; 

    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    
    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 


if( FILTER_freq_response.n < 1. ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A FILTER FUNCTION. BYE.\n" ) ;
    exit(EXIT_FAILURE);
}

// SET UP FLAG FOR OSCIL BANK OR OVERLAP/ADD
    if( 
	(FILTER_OUTPUT_ptrans.n  != 1.) || (FILTER_OUTPUT_ptrans.A[0] != 0.) || 
	(FILTER_OUTPUT_harmadd.n  != 1.) || (FILTER_OUTPUT_harmadd.A[0] != 0.) ||
	(FILTER_OUTPUT_fdevbase.n  != 1.) || (FILTER_OUTPUT_fdevbase.A[0] != 0.) || 
	(FILTER_OUTPUT_fdevpeak.n  != 1.) || (FILTER_OUTPUT_fdevpeak.A[0] != 0.) || 
	(FILTER_OUTPUT_fshiftbase.n  != 1.) || (FILTER_OUTPUT_fshiftbase.A[0] != 0.) || 
	(FILTER_OUTPUT_fshiftpeak.n  != 1.) || (FILTER_OUTPUT_fshiftpeak.A[0] != 0.) ||
	(SOURCE_fshift.n  != 1.) || (SOURCE_fshift.A[0] != 0.) ||
	(SOURCE_ptrans.n  != 1.) || (SOURCE_ptrans.A[0] != 0.)
    ){
	// OSC BANK
	P = 1. ; obank = 1 ;  
    }else{
	// OVERLAPP-ADD
	P = 0. ; obank = 0 ;  
    }






   // FREQ DEVIATION MODE SETUP
    if( FILTER_OUTPUT_freqdevmode.n != 1. ){
	// FILE MODE
	ranfreqdevswitch = 2 ; 
    }else if( FILTER_OUTPUT_freqdevmode.A[ 0 ] == 0. ){
	// RESPONSE MODE
	ranfreqdevswitch = 0 ; 
    }else if( FILTER_OUTPUT_freqdevmode.A[ 0 ] == 1. ){ 
	// RANDOM MODE
	ranfreqdevswitch = 1 ; 
    }else{
	// ILLEGAL MODE
	prt( "\n\n\tILLEGAL FREQUENCY DEVIATION MODE. BYE!\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
    } 
    
// ***************** PRINT VALUES
prf( dur, "OUTPUT FILE: DURATION" ) ; 

prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 

prf( frames_per_sec,  "FRAMES/SECOND" ) ; 
pri( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
pri( I,  "      INTERPOLATION SAMPLES (samples between resynthesis frames)" ) ; 
prline( 1,  "*" ) ; 
prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (in dB)" ) ; 
prline( 1,  "*" ) ; 

prp( &SOURCE_dB, "SOURCE GAIN (in dB)" ) ; 
prp( &SOURCE_fshift, "SOURCE FREQUENCY SHIFT (in Hz)" ) ; 
prp( &SOURCE_delayT, "SOURCE DELAY TIME (in seconds)" ) ; 
prp( &SOURCE_ptrans, "SOURCE TRANSPOSITION (in semitones)" ) ; 


prp( &FILTER_OUTPUT_dBgain,  "FILTER OUTPUT: GAIN (in dB)"  ) ; 
prp( &FILTER_OUTPUT_ptrans,  "FILTER OUTPUT: PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &FILTER_OUTPUT_harmadd,  "FILTER OUTPUT: FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
if( bandrejecton ){
    prt( "FILTER TYPE: ............REJECT RESPONSE" ) ;
}else{
    prt( "FILTER TYPE: ............PASS RESPONSE" ) ;
}
prp( &FILTER_FREQ_RESPONSE_ftrans,  "FILTER FREQ RESPONSE: PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &FILTER_FREQ_RESPONSE_fshift,  "FILTER FREQ RESPONSE: FREQUENCY SHIFT (in Hz)"  ) ; 
prp( &FILTER_dB_source_floor,  "FILTER: SOURCE SIGNAL FLOOR (in dB)"  ) ; 
prline( 1,  "*" ) ; 
prp( &FILTER_OUTPUT_attack,  "FILTER OUTPUT: AMPLITUDE ATTACK TIME (in seconds)"  ) ; 
prp( &FILTER_OUTPUT_release,  "FILTER OUTPUT: AMPLITUDE RELEASE TIME (in seconds)"  ) ; 
prline( 1,  "*" ) ; 
prp( &ampwarpshape, "INPUT SPECTRUM AMPLITUDE DOMAIN WARPSHAPE INDEX" ) ; 
prline( 1,  "*" ) ; 


prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 
// *******
if( ranampdevswitch ){
    prt( " RANDOM AMP DEVIATION MODE" ) ; 
    prp( &FILTER_OUTPUT_ranampdevresponse,  "FILTER OUTPUT: RANDOM AMP DEVIATION FUNCTION RESPONSE TIME" ) ; 
}

if( ranfreqdevswitch == 0 ){
    prt( " FREQ DEVIATION: RESPONSE DRIVEN MODE" ) ; 
}else if( ranfreqdevswitch == 1 ){
    prt( " FREQ DEVIATION: RANDOM  MODE" ) ; 
    prp( &FILTER_OUTPUT_ranfreqdevresponse,  "FILTER OUTPUT: FREQUENCY DEVIATION FUNCTION RESPONSE TIME" ) ; 
}else if( ranfreqdevswitch == 2 ){ 
    prt( " FREQ DEVIATION: FUNCTION FILE MODE" ) ; 
    prp( &FILTER_OUTPUT_ranfreqdevresponse,  "FREQUENCY DEVIATION FUNCTION RESPONSE TIME" ) ; 
}

prp( &FILTER_OUTPUT_fdevbase,  "FILTER OUTPUT: BASE FREQUENCY DEVIATION (in semitones)"  ) ; 
prp( &FILTER_OUTPUT_fdevpeak,  "FILTER OUTPUT: PEAK FREQUENCY DEVIATION (in semitones)"  ) ; 
prline( 1,  "*" ) ; 
prp( &FILTER_OUTPUT_fshiftbase,  "FILTER OUTPUT: BASE FREQUENCY DEVIATION SHIFT (in Hz)"  ) ; 
prp( &FILTER_OUTPUT_fshiftpeak,  "FILTER OUTPUT: PEAK FREQUENCY DEVIATION SHIFT (in Hz)"  ) ; 

prp( &freqwarpshape, "INPUT SPECTRUM FREQUENCY DOMAIN WARPSHAPE INDEX" ) ; 
prp( &FILTER_OUTPUT_fdevcontrol,  "FILTER OUTPUT: MASTER FREQUENCY DEVIATION CONTROL (0-1)"  ) ; 

prline( 1,  "*" ) ; 

prp( &timedelaywarpshape, "INPUT SPECTRUM TIME DELAY WARPSHAPE INDEX" ) ; 
prp( &FILTER_OUTPUT_timeDelayBase, "BASE TIME DELAY (in seconds)" ) ; 
prp( &FILTER_OUTPUT_timeDelayPeak, "PEAK TIME DELAY (in seconds)" ) ; 
prp( &FILTER_OUTPUT_timeDelayDevControl,  "MASTER TIME DELAY CONTROL (0-1)"  ) ; 

prp( &decaytimewarpshape, "INPUT SPECTRUM DECAY TIME WARPSHAPE INDEX" ) ; 
prp( &FILTER_OUTPUT_peak_decay_time_in_seconds, "FILTER OUTPUT: PEAK DECAY TIME (in seconds)" ) ;  
prp( &FILTER_OUTPUT_base_decay_time_in_seconds, "FILTER OUTPUT: BASE DECAY TIME (in seconds)" ) ; 
prp( &FILTER_OUTPUT_decayTimeDevControl,  "FILTER OUTPUT: MASTER DECAY TIME CONTROL (0-1)"  ) ; 
if( randomDelayTflag == 1 ) prt( "FILTER OUTPUT: RANDOM TIME DELAY DEVIATION IS ON" ) ; 
else prt( "FILTER OUTPUT: RANDOM TIME DELAY DEVIATION IS OFF" ) ;

prp( &FILTER_OUTPUT_randelayTdevresponse , 
	"FILTER_OUTPUT: RANDOM TIME DELAY DEVIATION RESPONSE TIME (in seconds)" ) ; 

prp( &Function_Delay_Time_Scaler, "FUNCTION DELAY TIME SCALER" ); 

prline( 1,  "*" ) ; 

prp( &FILTER_OUTPUT_frameNormalizationDecibelLimit, "FRAME NORMALIZATION DECIBEL LIMIT" ) ; 
if( Normalize_to__Input_Sound_0__Filter_1 == 0 )prt( "NORMALIZING TO INPUT SOUND" ) ; 
else prt( "NORMALIZING TO FILTER FRAME" ) ; 


prf( rescalev, "DECIBEL RESCALE VALUE" ) ; 





// ***************** SET UP ARRAYS


    
    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( bufferSum, N ) ;		/* FFT buffer */
    fvec( buffer_filter, N ) ; 
    fvec( buffer_filter_delayed_inputs, N ) ; 
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( channel_filter, N+2 ) ; 
    fvec( channel_filter_delayed_inputs, N+2 ) ; 
    fvec( FILTER_channel_delay_now, N+2 ) ; 
    fvec( output, Nw ) ;	/* output buffer */
    fvec( previous_channel_filter, N+2 ) ;	/* previous analysis channels */

    fvec( previous_ranampv, N+2 ) ;	/* channels old random amp value  */
    fvec( previous_ranfreqv, N+2 ) ;	/* channels old random freq value  */

    fvec( thisFilterDelayT, N2 + 1 ) ; 
    fvec( previous_thisFilterDelayT, N2 + 1 ) ; 
    fvec( thisFilterDelayT_inFrames, N2 + 1 ) ; 
    fvec( thisFilterDecayT, N2 + 1 ) ; 
    fvec( thisFilterDecayT_inFrames, N2 + 1 ) ; 

    // CHANNEL DELAYS
    SOURCE_maxNumOfDelayFrames = 1 + (int)((SOURCE_maxDelayT * frames_per_sec) + 0.5) ; 
    fvec( SOURCE_channel_delay, SOURCE_maxNumOfDelayFrames * (N + 2) ) ; 
    FILTER_maxNumOfDelayFrames = 1 + (int)((FILTER_maxDelayT * frames_per_sec) + 0.5) ; 
    fvec( FILTER_channel_delay, FILTER_maxNumOfDelayFrames * (N + 2) ) ; 

// MAKE THRESH AMP
    threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	

// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

// ALLOCATE FILTER SPACE FOR BOTH AMP AND FREQ FORMS OF RESPONSE
fvec( F,  N+2  ) ;	/* filter array */
//fvec( Ffreq,  N+2  ) ;	/* filter array */
//fvec( FtimeDelay, N+2 ) ; 
fvec( FampWarp,  N+2  ) ;	/* filter array */
fvec( FfreqWarp,  N+2  ) ;	/* filter array */
fvec( FtimeDelayWarp, N+2 ) ; 
fvec( FdecayTimeWarp, N+2 ) ; 

//************FIRST TIME: FILL ARRAY FROM FILE
    fillfunc( &FILTER_freq_response, F, (N + 2)  ) ;     

// MAKE FILTER AMP SUM
   filterChannelAmpSum = 0. ; 	
   for( i = 0; i < (N + 2); i+= 2 ) filterChannelAmpSum += F[i] ; // !!!


// ********** COPY F INTO Ffreq
//    for(i = 0; i < (N+2); i++)Ffreq[i] = F[i] ; 

// ********** COPY F INTO FtimeDelay
//    for(i = 0; i < (N+2); i++) FtimeDelay[i] = F[i] ; 

/*
    for(i = 0; i < (N+2); i++) {
	FampWarp[i] = FfreqWarp = FtimeDelayWarp[i] = FdecayTimeWarp[i] =  F[i] ; 
    } ; 

    
// *************EQ AND NORMALIZE
    if(dBlow !=  dBhi)prt("...............EQUALIZING AND NORMALIZING INPUT FREQUENCY RESPONSE....." ) ; 
    else prt("...............NORMALIZING INPUT FREQUENCY RESPONSE....." ) ;
    eq( F,  (N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1,  0, 1 ) ; 

// *************** IF BAND REJECT, THEN INVERT THE RESPONSE
   if( bandrejecton ){
	prt( "INVERTING RESPONSE......." ) ;
	invertresponse( F,  (N + 2),  0  ) ; // !!!
    }

// *************WARP: AMPLITUDE
    if(ampwarpshape != 0) prt("..............WARPING  RESPONSE: AMPLITUDE DOMAIN....." ) ; 
    spectmagwarp( F,  (N + 2), ampwarpshape, 1 ) ; // !!!

// *************WARP
    if(freqwarpshape != 0) prt("..............WARPING  RESPONSE: FREQUENCY DOMAIN....." ) ; 
    spectmagwarp( Ffreq,  (N + 2), freqwarpshape, 1 ) ; // !!!

// *************WARP
    if(timedelaywarpshape != 0) prt("..............WARPING  RESPONSE: TIME DELAY DOMAIN....." ) ; 
    spectmagwarp( FtimeDelay,  (N + 2), timedelaywarpshape, 1 ) ; // !!!




// *********** PRINT TO TERMINAL IF DESIRED ******
    if(print_flag){
	prbanner( "FILTER RESPONSE",  69 ) ; 
	tprintspec( F, (N + 2), fundamental,  print_flag) ;

 	if(freqwarpshape != 0) {
	    prbanner( "FREQUENCY DEVIATION RESPONSE",  69 ) ; 
	    tprintspec( Ffreq, (N + 2), fundamental,  print_flag) ;
	}

 	if(timedelaywarpshape != 0) {
	    prbanner( "TIME DELAY DEVIATION RESPONSE",  69 ) ; 
	    tprintspec( FtimeDelay, (N + 2), fundamental,  print_flag) ;
	}

   }

*/

// ************ CONVERT FREQUENCIES TO PHASE DIFFERENCES
    for(i = 1,  j = 0; i < (N + 2); i += 2,  j++) // !!!
	F[ i ] = (F[ i ] - ((float) j * fundamental)) / factor ; 


// *********************************************
// LOOP FOR CHANNELS
// *********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

    // *****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; 

    for( i = 0; i < (SOURCE_maxNumOfDelayFrames * (N + 2)); i++ ) SOURCE_channel_delay[i] = 0.  ; 
    for( i = 0; i < (FILTER_maxNumOfDelayFrames * (N + 2)); i++ ) FILTER_channel_delay[i] = 0.  ; 



// **** SEED RANDOM
    srandom(1);


// ***** ZERO OLD RAN VALUES
    for( i = 0; i < (N + 2); i++ ) { // !!!
	previous_ranfreqv[i] = 0. ; previous_ranampv[i] = 0. ;  
    }




    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;
	
// *********************************************
// LOOP FOR FRAMES
// *********************************************

    while ( !eof ) {
	in += D ;
	on += I ;
	timenow( dur ) ;

	eof = shiftin( input, Nw, D ) ;
	fold( input, Wanal, Nw, buffer, N, in ) ;
	rfft( buffer, N2, FORWARD ) ;
	convert( buffer, channel, N2, D, R ) ;	 

	// TRANSFER NEW channel INTO CIRCULAR DELAY LINES FOR SOURCE AND FILTER 
	SOURCE_frameNowChannelDelayIndex = frame_count ; 
	while( SOURCE_frameNowChannelDelayIndex >= SOURCE_maxNumOfDelayFrames ) 
		SOURCE_frameNowChannelDelayIndex -=  SOURCE_maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2) ; i++ ) 
		SOURCE_channel_delay[ (SOURCE_frameNowChannelDelayIndex * (N + 2)) + i] = channel[i] ; 

	FILTER_frameNowChannelDelayIndex = frame_count ; 
	while( FILTER_frameNowChannelDelayIndex >= FILTER_maxNumOfDelayFrames ) 
		FILTER_frameNowChannelDelayIndex -=  FILTER_maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2) ; i++ ) 
		FILTER_channel_delay[ (FILTER_frameNowChannelDelayIndex * (N + 2)) + i] = channel[i] ; 


// *************************
// GET THE VALUES
// *************************


//  SHIFT, GAIN, AND TRANSPOSITION

		

		FILTER_OUTPUT_attack.A[ 0 ] = fval( &FILTER_OUTPUT_attack, dur, t );
		    smooth_setup( FILTER_OUTPUT_attack.A[ 0 ], &attackc, &minusattackc, IR ) ; 
		FILTER_OUTPUT_release.A[ 0 ] = fval( &FILTER_OUTPUT_release, dur, t );
		    smooth_setup( FILTER_OUTPUT_release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 


		// **** FREQ DEVIATION: SETUP 

		// MAKE BASE, PEAK AND RANGE FOR TIME DELAY.
		FILTER_OUTPUT_timeDelayBase.A[ 0 ] = fval( &FILTER_OUTPUT_timeDelayBase, dur, t );
		FILTER_OUTPUT_timeDelayPeak.A[ 0 ] = fval( &FILTER_OUTPUT_timeDelayPeak, dur, t );
		    timeRange = FILTER_OUTPUT_timeDelayPeak.A[ 0 ] - FILTER_OUTPUT_timeDelayBase.A[ 0 ] ; 	
		FILTER_OUTPUT_timeDelayDevControl.A[ 0 ] = fval( &FILTER_OUTPUT_timeDelayDevControl, dur, t );

		FILTER_OUTPUT_randelayTdevresponse.A[ 0 ] = fval( &FILTER_OUTPUT_randelayTdevresponse, dur, t );
		    smooth_setup( FILTER_OUTPUT_randelayTdevresponse.A[ 0 ], 
			    &randelayTsmoothc, &minusrandelayTsmoothc, IR ) ; 



	    	// FRAME NORMALIZATION DECIBEL LIMIT
		FILTER_OUTPUT_frameNormalizationDecibelLimit.A[ 0 ] = 
			fval( &FILTER_OUTPUT_frameNormalizationDecibelLimit, dur, t );
		frameNormalizationAmpLimit = dB_to_amp( FILTER_OUTPUT_frameNormalizationDecibelLimit.A[ 0 ] ); 

		
	    // *****		

    // ********** FIND THE SUM OF THE AMPS FOR NORMALIZATION
    channel_outAmpSum = 0 ; 



    for(i = 0; i < (N+2); i++) {
	FampWarp[i] = FfreqWarp[i] = FtimeDelayWarp[i] = FdecayTimeWarp[i] = F[i] ; 
    } ; 

    
// *************EQ AND NORMALIZE
//    if(dBlow !=  dBhi)prt("...............EQUALIZING AND NORMALIZING INPUT FREQUENCY RESPONSE....." ) ; 
//    else prt("...............NORMALIZING INPUT FREQUENCY RESPONSE....." ) ;
    eq( FampWarp,  (N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1,  0, 1 ) ; // !!!

// *************** IF BAND REJECT, THEN INVERT THE RESPONSE
   if( bandrejecton ){
//	prt( "INVERTING RESPONSE......." ) ;
	invertresponse( FampWarp,  (N + 2),  0  ) ; // !!!
    }

// *************WARP: AMPLITUDE
    ampwarpshape.A[ 0 ] = fval( &ampwarpshape, dur, t );
    spectmagwarp( FampWarp,  (N + 2), ampwarpshape.A[ 0 ], 1 ) ; // !!!

// *************WARP: FREQ
    freqwarpshape.A[ 0 ] = fval( &freqwarpshape, dur, t );
    spectmagwarp( FfreqWarp,  (N + 2), freqwarpshape.A[ 0 ], 1 ) ;

// *************WARP: TIME DELAY
    timedelaywarpshape.A[ 0 ] = fval( &timedelaywarpshape, dur, t );
    spectmagwarp( FtimeDelayWarp,  (N + 2), timedelaywarpshape.A[ 0 ], 1 ) ;

// *************WARP: DECAY TIME
    decaytimewarpshape.A[ 0 ] = fval( &decaytimewarpshape, dur, t );
    spectmagwarp( FdecayTimeWarp,  (N + 2), decaytimewarpshape.A[ 0 ], 1 ) ;




// *****************
// MODIFICATIONS LOOP
// *****************


     Function_Delay_Time_Scaler.A[ 0 ] =  fval( & Function_Delay_Time_Scaler, dur, t );
     // fs
	FILTER_OUTPUT_harmadd.A[ 0 ] =  fval( &FILTER_OUTPUT_harmadd, dur, t  );
	fs = FILTER_FREQ_RESPONSE_fshift.A[ 0 ] = fval( &FILTER_FREQ_RESPONSE_fshift, dur, t  );

      // fm
	FILTER_FREQ_RESPONSE_ftrans.A[ 0 ] = fval( &FILTER_FREQ_RESPONSE_ftrans, dur, t  );
     fm = semitones_to_mult( FILTER_FREQ_RESPONSE_ftrans.A[ 0 ]  ) ;


	for( i = 1, j = 0; i < (N + 2); i += 2, j++ ){ // !!!

          // HERE NEED fm, fs
	    // FIND THE INDECES WHICH RESULT AFTER SHIFT AND TRANSPOSE OF 
	    // THE FILTER
		    // APPLY FILTER TRANSPOSE
		    // SHIFT
		    temp = (i - 1) / 2 ; 
		    temp = temp -  ( fs / fundamental ) ; 
		    // TRANSPOSE
		    temp =  temp / fm   ; 
		    i2p = temp - (float)((int) temp ) ; 
		    i1p = 1. - i2p ; 
		    i1 = 2 * (int) temp ; 
		    i2 = i1 + 2 ; 
		    
		    if( i1 < 0 ){ //UNDER THE ARRAY
			i1 = 0; i2 = i1 + 2 ; i1p = 1. ; i2p = 0. ; 
		    }
		    if( i2 > N ){ //OVER THE ARRAY
			i2 = N - 2 ; i1 = i2 - 2 ; i2p = 1. ; i1p = 0. ; 
		    }

		// PLACE DELAYED CHANNEL VALUES INTO channel_filter 
		// FOR THIS FtimeDelayWarp RESPONSE AND BOUNDARIES.

		// FIRST MAKE INTERPOLATED TIME DELAY
		    interpTdelayFilterAmp = ( FtimeDelayWarp[ i1 ] * i1p ) + (FtimeDelayWarp[ i2 ] * i2p ) ; 

		        // FIND TIME DELAY
		    if( randomDelayTflag == 1 ){
			// RANDOM MODE
		        thisFilterDelayT[ j ] = FILTER_OUTPUT_timeDelayDevControl.A[ 0 ] * 
			    (FILTER_OUTPUT_timeDelayBase.A[ 0 ] + 
				(randf( 0,  1. ) * interpTdelayFilterAmp * timeRange ) ) ; 
			if( frame_count == 0 ) previous_thisFilterDelayT[ j ] = thisFilterDelayT[ j ] ; 
			// SMOOTH
			thisFilterDelayT[ j ] = (randelayTsmoothc * previous_thisFilterDelayT[ j ] )
					+ ( minusrandelayTsmoothc * thisFilterDelayT[ j ] );
			// SAVE AS PREVIOUS
		        previous_thisFilterDelayT[ j ] = thisFilterDelayT[ j ] ; 

		    }else{
			// RESPONSE ONLY MODE
		        thisFilterDelayT[ j ] = FILTER_OUTPUT_timeDelayDevControl.A[ 0 ] * 
			    (FILTER_OUTPUT_timeDelayBase.A[ 0 ] + 
				(interpTdelayFilterAmp * timeRange ) ) ; 

		    } ; 

		    thisFilterDelayT_inFrames[ j ] = floor((thisFilterDelayT[ j ] * frames_per_sec) + 0.5) ; 
		    thisFilterFrameDelay = FILTER_frameNowChannelDelayIndex - thisFilterDelayT_inFrames[ j ] ; 
		    while( thisFilterFrameDelay < 0) thisFilterFrameDelay += FILTER_maxNumOfDelayFrames ; 
		    channel_filter[i] = channel_filter_delayed_inputs[i] =
				FILTER_channel_delay[ (thisFilterFrameDelay * (N + 2)) + i] ; 
		    channel_filter[i - 1] = channel_filter_delayed_inputs[i - 1] =
				FILTER_channel_delay[ (thisFilterFrameDelay * (N + 2)) + i - 1] ; 
		    


		    // SETUP PREVIOUS CHANNEL IF FIRST FRAME
		    if( !frame_count ) {
				previous_channel_filter[ i ] = channel[ i ] ; // !!!
				previous_channel_filter[i - 1] = channel[i - 1] ; // !!!
		    } ; 

		    // ADD TO NORMALIZATION SUM
		    channel_outAmpSum += channel_filter[i - 1] ; 

		// DECAY TIME
		// FIRST MAKE INTERPOLATED TIME DELAY
		    interpDecayTFilterAmp = ( FdecayTimeWarp[ i1 ] * i1p ) + (FdecayTimeWarp[ i2 ] * i2p ) ; 

		    // FIND DECAY TIME

		    // RESPONSE ONLY MODE
		    FILTER_OUTPUT_decayTimeDevControl.A[ 0 ] = 
                   fval( &FILTER_OUTPUT_decayTimeDevControl,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
               FILTER_OUTPUT_base_decay_time_in_seconds.A[ 0 ] = 
			    fval( &FILTER_OUTPUT_base_decay_time_in_seconds,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
               FILTER_OUTPUT_peak_decay_time_in_seconds.A[ 0 ] = 
			     fval( &FILTER_OUTPUT_peak_decay_time_in_seconds,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
               feedbackDecayTimeRange = FILTER_OUTPUT_peak_decay_time_in_seconds.A[ 0 ] - 
			     FILTER_OUTPUT_base_decay_time_in_seconds.A[ 0 ] ; 


		    thisFilterDecayT[ j ] = FILTER_OUTPUT_decayTimeDevControl.A[ 0 ] * 
			    (FILTER_OUTPUT_base_decay_time_in_seconds.A[ 0 ] + (interpTdelayFilterAmp * feedbackDecayTimeRange ) ) ; 
		    thisFilterDecayT_inFrames[ j ] = floor( (thisFilterDecayT[ j ] * frames_per_sec) + 0.5  ) ; 

	           // MAKE NEW AMPLITUDE FOR THIS BIN
		    // FIRST MAKE INTERPOLATED FILTER AMP FOR THIS BIN/CHANNEL
		    this_interpFilterAmp = ( FampWarp[ i1 ] * i1p ) + (FampWarp[ i2 ] * i2p ) ; 

               FILTER_dB_source_floor.A[ 0 ] = fval( &FILTER_dB_source_floor,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
               sourceamp = dB_to_amp( FILTER_dB_source_floor.A[ 0 ] );  filtamp = 1. - sourceamp ;  

               // *** RANDOM AMP DEVIATION SETUP
               FILTER_OUTPUT_ranampdevresponse.A[ 0 ] = fval( &FILTER_OUTPUT_ranampdevresponse,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
		    smooth_setup( FILTER_OUTPUT_ranampdevresponse.A[ 0 ], &ranampsmoothc, &minusranampsmoothc, IR ) ; 
		    
		    this_ranampval = (ranampsmoothc * previous_ranampv[i - 1] ) + ( minusranampsmoothc * randf( 0,  1. ) ); 
		    previous_ranampv[i - 1] = this_ranampval ; 


		    // MAKE NEW AMP
              if( ranampdevswitch ){
		        // RANDOM AMP DEVIATION MODE
		        // MAKE SMOOTHED RANDOM VALUE

 
		        if( ranabovebelowswitch ){
		            // RANDOMIZE ABOVE RESPONSE

		            channel_filter[i - 1] = channel_filter[i - 1] * 
			          (   filtamp * 
				      (this_interpFilterAmp + (this_ranampval * (1. - this_interpFilterAmp)))  
				         + sourceamp 
			       )  ; 

			       thisdBval =  amp_to_dB( (this_interpFilterAmp * filtamp) + sourceamp ) ; 
			       channel_filter[i - 1] = channel_filter[i - 1] *	
			    	 dB_to_amp( (this_ranampval * -1. * thisdBval) + thisdBval ) ; 
		    
		         }else{
		            // RANDOMIZE BELOW RESPONSE
			
			      channel_filter[i - 1] = channel_filter[i - 1] * 
				    dB_to_amp( (this_interpFilterAmp * 
					( amp_to_dB( (filtamp * this_interpFilterAmp) + sourceamp ) - 
					 	FILTER_dB_source_floor.A[ 0 ] ) 
					* this_ranampval) + 
					FILTER_dB_source_floor.A[ 0 ] ) ; 


		        }
		
		}else{
		    // NON RANDOM MODE
		    channel_filter[i - 1] = channel_filter[i - 1] * 
			((filtamp * this_interpFilterAmp) + sourceamp );  
		}


	      // *** MAKE NEW DEVIATED FREQUENCY FOR THIS BIN
		// FIRST MAKE INTERPOLATED FILTER AMP
		this_interpFilterFreq = ( FfreqWarp[ i1 ] * i1p ) + (FfreqWarp[ i2 ] * i2p ) ; 

		  // MAKE NEW DEVIATED FREQUENCY
           FILTER_OUTPUT_freqdevmode.A[ 0 ] = fval( &FILTER_OUTPUT_freqdevmode,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );

		    // MAKE FULLY DEVIATED FREQ
           FILTER_OUTPUT_fdevbase.A[ 0 ] = fval( &FILTER_OUTPUT_fdevbase,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
           FILTER_OUTPUT_fdevpeak.A[ 0 ] = fval( &FILTER_OUTPUT_fdevpeak,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
           fdmdiff = FILTER_OUTPUT_fdevpeak.A[ 0 ] - FILTER_OUTPUT_fdevbase.A[ 0 ] ; 		    

           FILTER_OUTPUT_fshiftbase.A[ 0 ] = fval( &FILTER_OUTPUT_fshiftbase,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
           FILTER_OUTPUT_fshiftpeak.A[ 0 ] = fval( &FILTER_OUTPUT_fshiftpeak,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
           fsdiff = FILTER_OUTPUT_fshiftpeak.A[ 0 ] - FILTER_OUTPUT_fshiftbase.A[ 0 ] ; 		    

           fullydeviatedfreq = 
			    semitones_to_mult(FILTER_OUTPUT_fdevbase.A[ 0 ] + (FILTER_OUTPUT_freqdevmode.A[ 0 ] * fdmdiff))
			    * 
			(channel_filter[i] + (FILTER_OUTPUT_fshiftbase.A[ 0 ] + (FILTER_OUTPUT_freqdevmode.A[ 0 ] * fsdiff)) ) ; 

		FILTER_OUTPUT_fdevcontrol.A[ 0 ] = fval( &FILTER_OUTPUT_fdevcontrol,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
		fdevminus = 1. - FILTER_OUTPUT_fdevcontrol.A[ 0 ] ; 


		if( ranfreqdevswitch == 2 ){
		    // FILE MODE

		    channel_filter[i] = channel_filter[i] + 
			(this_interpFilterFreq * FILTER_OUTPUT_fdevcontrol.A[ 0 ] * (fullydeviatedfreq - channel_filter[i])) ; 
		    
		}else if( ranfreqdevswitch == 1 ){
		    // RANDOM DEVIATION MODE

		    FILTER_OUTPUT_ranfreqdevresponse.A[ 0 ] = fval( &FILTER_OUTPUT_ranfreqdevresponse, dur, 
                         t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
		        smooth_setup( FILTER_OUTPUT_ranfreqdevresponse.A[ 0 ], 
			    &ranfreqsmoothc, &minusranfreqsmoothc, IR ) ; 

		    // MAKE SMOOTHED RANDOM VALUE
		    this_smoothedrandomval = (ranfreqsmoothc * previous_ranfreqv[i - 1] )
					+ ( minusranfreqsmoothc * randf( 0,  1. ) ); 
		    previous_ranfreqv[i - 1] = this_smoothedrandomval ; 

		    // MAKE FULLY DEVIATED FREQ
		    fullydeviatedfreq = semitones_to_mult(FILTER_OUTPUT_fdevbase.A[ 0 ] + 
				(this_smoothedrandomval * fdmdiff)) * 
			(channel_filter[i] + (FILTER_OUTPUT_fshiftbase.A[ 0 ] + (this_smoothedrandomval * fsdiff)) ) ; 

		    channel_filter[i] = channel_filter[i] + 
			(this_interpFilterFreq * FILTER_OUTPUT_fdevcontrol.A[ 0 ] * (fullydeviatedfreq - channel_filter[i])) ; 
		    
		}else if( ranfreqdevswitch == 0 ){
		    // RESPONSE MODE
		    channel_filter[i] = 
			( fdevminus + (FILTER_OUTPUT_fdevcontrol.A[ 0 ] 
			    * semitones_to_mult(FILTER_OUTPUT_fdevbase.A[ 0 ] + (this_interpFilterFreq * fdmdiff)))  )
			    * 
			(channel_filter[i] + 
			    (FILTER_OUTPUT_fdevcontrol.A[ 0 ] * 
				(FILTER_OUTPUT_fshiftbase.A[ 0 ] + (this_interpFilterFreq * fsdiff)))
			 ) ; 
		}


	}

	// ****
// ******
    
	//  NORMALIZE AND REPLACE INTO CHANNEL.
		// FIND NORMALIZATION FACTOR.
	tempChannelAmpSum = 0. ; 	
	for( i = 0; i < (N + 2); i+= 2 ){ // !!!
		tempChannelAmpSum += channel_filter[i] ;
	} ; 
		// NORMALIZE AND TRANSFER
	if( (tempChannelAmpSum > 0.0) && (frameNormalizationAmpLimit != 1.0 ) ){
		if( Normalize_to__Input_Sound_0__Filter_1 == 0 ){
			normalizationAmp = channel_outAmpSum / tempChannelAmpSum ;
		}else {
			normalizationAmp = filterChannelAmpSum / tempChannelAmpSum ;
		} ; 

		if( normalizationAmp > frameNormalizationAmpLimit )
			normalizationAmp = frameNormalizationAmpLimit ; 
		for( i = 0; i < (N + 2); i+= 2 ) // !!!
			channel_filter[i] = channel_filter[i] * normalizationAmp ; 	
	} ; 




// ********
	
    // SMOOTH THE CHANGES TO THE SPECTRUM // !!!
    smooth( channel_filter, previous_channel_filter, (N + 2), attackc, minusattackc, releasec, minusreleasec ) ; 
	 
	for( i = 1, j = 0; i < (N + 2); i += 2, j++ ){ // !!!

	    // SHIFT BY -a AND TRANSPOSE BY -P
          // HERE NEED pm 

           // pm
		FILTER_OUTPUT_harmadd.A[ 0 ] =  fval( &FILTER_OUTPUT_harmadd,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
		FILTER_OUTPUT_ptrans.A[ 0 ] = fval( &FILTER_OUTPUT_ptrans,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
		    pm = semitones_to_mult( FILTER_OUTPUT_ptrans.A[ 0 ] ) ;

	    temp = pm * (channel_filter[i] + FILTER_OUTPUT_harmadd.A[ 0 ]) ;
		    
	    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
	    if((temp <= 0.) || (temp >= nyquist)) channel_filter[i - 1] = 0. ; 
	    else channel_filter[i] = temp ; 
	    

         FILTER_OUTPUT_dBgain.A[ 0 ] =  fval( &FILTER_OUTPUT_dBgain,  
                  dur, t - (Function_Delay_Time_Scaler.A[ 0 ] * thisFilterDelayT[ j ] ) );
		    gain = dB_to_amp(FILTER_OUTPUT_dBgain.A[ 0 ] );	

	    channel_filter[i - 1] = channel_filter[i - 1] * gain ;  

	}




	// ATTENUATE DELAYED FILTER INPUTS AND ADD INTO DELAY ARRAY
	for(i = 0, j = 0; i < (N + 2); i += 2, j++){ 
	    if( thisFilterDecayT_inFrames[ j ] <= 0. ){
		// ZERO DECAY TIME
		channel_filter_delayed_inputs[i] = 0. ; 
	    }else{
		// NON ZERO DECAY TIME
		if(  thisFilterDelayT_inFrames[ j ] <= 0. ){
		    // ZERO DELAY TIME
		    channel_filter_delayed_inputs[i] =  0. ; 
		}else{
		    channel_filter_delayed_inputs[i] *= 
			dB_to_amp(-60. / ( thisFilterDecayT_inFrames[ j ] / thisFilterDelayT_inFrames[ j ] )) ; 
		} ; 	
	    } ; 
	} ; 

	unconvert2( channel_filter_delayed_inputs, buffer_filter_delayed_inputs, N2, I, R ) ; 
	for(i = 0 ; i < (N + 2); i++) FILTER_channel_delay_now[i] = 
		FILTER_channel_delay[ (FILTER_frameNowChannelDelayIndex * (N + 2)) + i] ; 
	unconvert3( FILTER_channel_delay_now, bufferSum, N2, I, R ) ;
		// !!!
	for(i = 0; i < N; i++ ) bufferSum[i] = bufferSum[i] + buffer_filter_delayed_inputs[i] ; 	    
	convert3( bufferSum, FILTER_channel_delay_now, N2, D, R ) ;	 
	for( i = 0; i < (N + 2) ; i++ ) 
		FILTER_channel_delay[ (FILTER_frameNowChannelDelayIndex * (N + 2)) + i] = 
			FILTER_channel_delay_now[i] ; 
	



    // SOURCE
    if( sourceflag == 1){

	SOURCE_delayT.A[ 0 ] = fval( &SOURCE_delayT, dur, t );
 	thisSourceFrameDelay = SOURCE_frameNowChannelDelayIndex - 
			(int)((SOURCE_delayT.A[ 0 ] * frames_per_sec) + 0.5) ; 
	while( thisSourceFrameDelay < 0) thisSourceFrameDelay += SOURCE_maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2); i++ ) channel[ i ] = SOURCE_channel_delay[ (thisSourceFrameDelay * (N + 2)) + i] ; 

	SOURCE_dB.A[ 0 ] =  fval( &SOURCE_dB, dur, t - SOURCE_delayT.A[ 0 ] );
	SOURCE_ptrans.A[ 0 ] =  fval( &SOURCE_ptrans, dur, t - SOURCE_delayT.A[ 0 ] );
	    pm = semitones_to_mult( SOURCE_ptrans.A[ 0 ] ) ; 
		
	SOURCE_fshift.A[ 0 ] =  fval( &SOURCE_fshift, dur, t - SOURCE_delayT.A[ 0 ] );

	for(i = 0; i < (N + 2); i += 2 ){ // !!!
	    channel[i] *= dB_to_amp( SOURCE_dB.A[ 0 ] ) ; 
	    channel[i + 1] = (channel[i + 1] * pm) + SOURCE_fshift.A[ 0 ] ;
	    if( channel[i + 1] < 0. ){
		channel[i + 1] = 0;  channel[i] = 0. ; 
	    } ;   
	} ; 
    }; 

    if(sourceflag == 1){
	synt = getthresh( channel, N, threshfac );
	temp = getthresh( channel_filter, N, threshfac );
	if(temp > synt) synt = temp ; 
    }else{
	temp = getthresh( channel_filter, N, threshfac );
    }

 
    if(sourceflag == 1){ 
	// WITH BOTH SOURCE AND FILTER OUTPUT
        if ( obank ) { 
if( !frame_count )prt( "HERE: OSCIL BANK  FOR BOTH" ) ; 
	    // PITCH CHANGE IN ONE OR THE OTHER: OSCILLATOR BANK FOR BOTH
	    noscbank2(channel_filter, N2, R, Nw, I, P,  output,  channel, N2 );
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;
	} else {
if( !frame_count )prt( "HERE: OVERLAPP-ADD FOR BOTH" ) ; 
	    // OVERLAP-ADD
	    unconvert1( channel_filter, buffer_filter, N2, I, R ) ;
	    unconvert( channel, buffer, N2, I, R ) ;
	    // COMBINE BUFFERS // !!!
	    for(i = 0; i < N; i++ ) buffer[i] += buffer_filter[i] ;    
	    rfft( buffer, N2, INVERSE ) ;
	    overlapadd( buffer, N, Wsyn, output, Nw, on ) ;
    	    shiftout( output, Nw, I, on, 0 ) ;
	
	}
    }else {
	// FILTER OUTPUT ONLY
	if ( obank ) {
if( !frame_count )prt( "HERE: OSCIL BANK FOR FILTER ONLY" ) ; 
	    // OSCIL BANK
	    noscbank(channel_filter, N2, R, Nw, I, P, output);
	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;
	} else {
if( !frame_count )prt( "HERE: OVERLAPP-ADD  FOR FILTER ONLY" ) ; 
	    // OVERLAP-ADD
	    unconvert1( channel_filter, buffer_filter, N2, I, R ) ;
	    rfft( buffer_filter, N2, INVERSE ) ;
	    overlapadd( buffer_filter, N, Wsyn, output, Nw, on ) ;
	    shiftout( output, Nw, I, on, 0 ) ;
	
	}
    } ; 

	frame_count++ ; 

    // FRAMES LOOP END

    }
    // FLUSH OUT AND CLOSE OUTPUT FILE
    shiftout( output, Nw, I, 1, 1 ) ;

    
// CHANNELS LOOP END
} 

    // CLOSE  INPUT FILE
    if(ifd)fclose(ifd);  



    fprintf(stderr,"\n\nFILDEVIATOR : RESYNTHESIS COMPLETED\n");

    if( FILTER_OUTPUT_harmadd.n != 1. ) fclose(FILTER_OUTPUT_harmadd.fp ) ;
    if( FILTER_OUTPUT_dBgain.n != 1. ) fclose(FILTER_OUTPUT_dBgain.fp ) ;
    if( FILTER_OUTPUT_ptrans.n != 1. ) fclose(FILTER_OUTPUT_ptrans.fp ) ;
    if( FILTER_freq_response.n != 1. ) fclose(FILTER_freq_response.fp ) ;
    if( FILTER_FREQ_RESPONSE_ftrans.n != 1. ) fclose(FILTER_FREQ_RESPONSE_ftrans.fp ) ;
    if( FILTER_FREQ_RESPONSE_fshift.n != 1. ) fclose(FILTER_FREQ_RESPONSE_fshift.fp ) ;
    if( FILTER_dB_source_floor.n != 1. ) fclose(FILTER_dB_source_floor.fp ) ;
    if( FILTER_OUTPUT_release.n != 1. ) fclose(FILTER_OUTPUT_release.fp ) ;
    if( FILTER_OUTPUT_attack.n != 1. ) fclose(FILTER_OUTPUT_attack.fp ) ;
    if( FILTER_OUTPUT_fdevbase.n != 1. ) fclose(FILTER_OUTPUT_fdevbase.fp ) ;
    if( FILTER_OUTPUT_fdevpeak.n != 1. ) fclose(FILTER_OUTPUT_fdevpeak.fp ) ;
    if( FILTER_OUTPUT_fshiftbase.n != 1. ) fclose(FILTER_OUTPUT_fshiftbase.fp ) ;
    if( FILTER_OUTPUT_fshiftpeak.n != 1. ) fclose(FILTER_OUTPUT_fshiftpeak.fp ) ;
    if( FILTER_OUTPUT_fdevcontrol.n != 1. ) fclose(FILTER_OUTPUT_fdevcontrol.fp ) ;


    exit(EXIT_SUCCESS) ;
}
void usage()
{


    fprintf(stderr, "%s",
	"filtdeviator:  fixed-spectrum, phase vocoder filter\n"
	"	    with response-correlated frequency deviation\n"
	"filtdeviator   [flags] [input file] [output file (optional)]\n"
	"	    (values in brackets denote defaults)\n"
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
	"	A:	gain in decibels (func)[0.] \n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds (0. = end of file) [0.] \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"


	"	h:	SOURCE gain in dB (func) [0.] \n"
	"	r:	SOURCE frequency shift in Hz (func) [0.] \n"
	"	s:	SOURCE delay time in seconds (func) [0.] \n"
	"	y:	SOURCE pitch transposition in semitones (func) [0.] \n"



	"	F:	FILTER: frequency response file \n"
	"	G:	FILTER: filtering method: \n"
	"		 0 = pass, 1 = reject (inverted response) [0] \n"

	"	Z:	FILTER FREQUENCY RESPONSE PRINTOUT: high cutoff freq in Hz [0]\n"
	"		    (0 = off)\n"
	"	T:	FILTER: frequency response transposition in semitones (func) [0]\n"
	"	V:	FILTER: frequency response shifter (func) [0.] \n"
	"	S:	FILTER: source decibels floor \n"
	"		    (reduces filtered signal in proportion)(func) [0.] \n"
	"	     RESPONSE WARPING:\n"
	"	W:	AMPLITUDE RESPONSE WARP:  index for reshaping frequency response [0.] \n"
	"		    values > 0 expand the dynamic range, \n"
	"		    values < 0 compress the dynamic range \n"

	"	    TIME DELAY:\n"
	"	J:	time delay peak (func) [0.]\n"
	"	j:	time delay base (func) [0.]\n"
	"	o:   	time delay warp: index for reshaping time delay response [0]\n"
	"	@:	delay time scaler (func) [0]\n"

	"	     FILTER FREQUENCY RESPONSE -- SHELF EQ: (pre-processing)\n"
	"	H:	FILTER SHELF EQ: Low shelf gain in dB [0.] \n"
	"	X:	FILTER SHELF EQ: High shelf gain in dB [0.] \n"
	"	m:	FILTER SHELF EQ: Low shelf frequency in Hz [200.] \n"
	"	R:	FILTER SHELF EQ: High shelf frequency in Hz [2000.] \n"

	"	l:      amplitude attack time  (func) [0.]\n"
	"	L:      amplitude release time   (func) [0.]\n"

	"	c:	RANDOM AMPLITUDE DEVIATION off = 0,  on = 1 [0]\n"
	"	d:	RANDOM AMPLITUDE DEVIATION FUNCTION: response time in seconds (func) [0.]\n"
	"	f:	RANDOM AMPLITUDE DEVIATION randomize mode \n"
	"		    in passband = 0,  in stopband = 1 [0]\n"

	"	E:	RANDOM FREQUENCY DEVIATION MODE:\n"
	"		    0 = response mode,  1 = random mode,  \"file name\" = file mode [0]\n"
	"	B:	FILTER OUTPUT: MASTER DECAY TIME CONTROL: (func) [0.]\n"

	"	q:	FILTER OUTPUT: master time delay control in seconds (func) [0.]\n"
	"	Y:	FILTER OUTPUT: random time delay deviation  0 = off, 1 = on [0]\n"
	"	z:	FILTER OUTPUT: random time delay deviation response time in seconds (func) [0.]\n"
	"	~:	FILTER FREQUENCY RESPONSE decay time warpshape index (func) [0.]\n"
	"	/:	FILTER OUTPUT: peak decay time in seconds (func) [0.]\n"


	"	Q:	FREQUENCY DEVIATION FUNCTION: response time in seconds (func) [0.]\n"
	"		    (random and file modes only)\n"
	"	k:      FREQUENCY DEVIATION: base deviation in semitones (func) [0.]\n"
	"	K:      FREQUENCY DEVIATION: peak deviation in semitones (func) [0.]\n"

	"	u:      FREQUENCY DEVIATION: base frequency shift (func) [0.]\n"
	"	U:      FREQUENCY DEVIATION: peak frequency shift (func) [0.]\n"
	"	v:	FREQUENCY RESPONSE WARP:  index for reshaping frequency response [0.] \n"
	"		    in frequency processing (frequency deviation domain)\n"
	"		    values > 0 close down response, < 0 open it up\n"
	"	O:	FREQUENCY DEVIATION: master deviation control (0 (off) - 1) (func) [1.] \n"

	"		FRAME NORMALIZATION: \n"
	"	n:	Frame Normalization Decibel Limit: (0-?) \n"
	"		Scale output frame amps to match or approach input frame amps\n"
	"		using the (sum of input amps)/(sum of output amps) limited to\n" 
	"		the Decibel limit. 0 dB prevents normalization. [0]\n"   

	"	x:	Frame Normalization Reference: \n"
	"		0 = Changing Input Sound, 1 = Constant Filter [0]\n"


	"	p:	amplitude reports print mode: 0 = off, 1 = on [0]\n" 
	"	i:	time interval between amplitude reports [.25]\n" 

	"	g:	Optional Analysis Sound File Source (for playback comparison only) [none]\n"

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
