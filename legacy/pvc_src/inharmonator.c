#include "globals.h"
#include <math.h>

#define NUM_PARAMETERS 5

#define PARTIAL_NUMBER PP[ j ]
#define PARTIAL_SHIFT_DATA PP[j + 1]
#define PARTIAL_DECIBELS PP[j + 2]
#define PARTIAL_TIME_DELAY PP[j + 3]
#define PARTIAL_FEEDBACK_DECAY_TIME PP[j + 4]

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k, n,   i1,  i2,  ipartial,  numberOfTargetPartials, bin ;
float nyquist;
double atof();
 
float tv0, tv1, tv2, tv3 ; 

int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag=0,   channelout=0;

float SOURCE_maxDelayT=0., HARMONY_maxDelayT=0., HARMONY_maxDecayT=0. ;
int SOURCE_maxNumOfDelayFrames, HARMONY_maxNumOfDelayFrames, 
	SOURCE_frameNowChannelDelayIndex, HARMONY_frameNowChannelDelayIndex, 
		SOURCE_thisFrameDelay, HARMONY_thisFrameDelay ; 
float data_partial_number_scaler, data_partial_number_shifter, data_decibel_scaler, 
		data_time_delay_scaler, data_time_delay_shifter, 
	data_time_decay_scaler, data_time_decay_shifter ;

float *delayTimes, *timeRateScalers, *decayTimes, *dBLevels, *partialPitchShifts, *partialNumbers ; 
 
float TARGETS_randelayTsmoothc, TARGETS_minusrandelayTsmoothc ; 
float NON_TARGETS_randelayTsmoothc, NON_TARGETS_minusrandelayTsmoothc ; 
 
float funcMin, funcMax, funcAvg ; 


float P = 1.0;
FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *bufferSum, *channel, *source_channel_out,
	  *SOURCE_channel_delay, *HARMONY_channel_delay, *output ;
float *previous_source_channel_out,  *F,  *FT,  *harmony,  *harmony_delayed_inputs, *harmony_delay_now, 
	*buffer_harmony_delayed_inputs,  *previous_harmony, *timeDelay, *previous_timeDelay,
	*timeDelayInFrames, *decayTime, *decayTimeInFrames ;
int  *partialDataLine, dataLine ;  
float threshfac = .001,  threshfacdB=-96 ;
float	random_seed, gain=1. ;
float  temp,  temp2,  temp3 ;  
//int beginchan,  endchan ; 
int sourceflag=1, PartialBandWindowType__rectangle_0__Hann_1__Welch_2=2 ; 
float getthresh();
float  FundamentalFreqNow  ;
	float diff,  ireleasec, ireleasem,  
		    iattackc,    iattackm,  
		    ifreqsmoothc, ifreqsmoothm,  
		    sreleasec,  sreleasem,  
		    sattackc,  sattackm ; 

float   IR,  dur=0.,  pms,  pmnt,   pmt,  sgain ;
int TARGETS_randomizationFlag=0, NON_TARGETS_randomizationFlag=0 ; 
float dBhi=0,  dBlow=0, middB=0,   midfreq=1000,   eqshape=0.  ; 
int method_flag=0 ; 
double ar_dB ; 

FILE *data ; 
char datafile[ STRING_SIZE ] = "", new_datafile[ STRING_SIZE ] ; 

float *PP,    fundamental,  target_gain,  non_target_gain, midC,  part ; 
int method=0 ; 

char tempstring[ STRING_SIZE ] ; 

struct func TARGETS_timedelayresponsetime ; float *TARGETS_timedelayresponsetime_VALUES ;   // OK
float *TARGETS_randelayTsmoothc_VALUES, *TARGETS_minusrandelayTsmoothc_VALUES ; 



struct func NON_TARGETS_timedelayresponsetime ;  // OK

// PARTIAL BANDWIDTH
struct func partial_bandwidth ; float *partial_bandwidth_VALUES ;  // OK

// MASTER GAIN
struct  func  dBgain ; float *dBgain_VALUES ;  // NOT RESPONSIVE TO DIFFERENT DELAYS

// NON TARGET  FREQUENCY SHIFT ADDER
struct  func  NON_target_harmadd ;  // OK

// NON TARGET  PITCH TRANSPOSITION IN SEMITONES
struct  func  NON_target_ptrans ; // OK  

// NON TARGET DECIBELS
struct  func  NON_target_dB ; // OK 

// NON TARGET DELAY TIME
struct  func  NON_target_delayT ; // OK  

struct  func  NON_target_decayT ; // OK


// FUNDAMENTAL FREQUENCY OR OCTAVE.PITCHCLASS
struct  func  Fund_Freq_or_OctavePointPitchClass ;  // OK

// TARGET DECIBELS
struct  func  target_dB ; float *target_dB_VALUES, *target_gain_TONE_VALUES ; // OK 

// TARGET  FREQUENCY SHIFT ADDER
struct  func  target_harmadd ; float *target_harmadd_VALUES ;  // OK

// TARGET  PITCH TRANSPOSITION IN SEMITONES
struct  func  target_ptrans ; float *target_ptrans_VALUES, *pmt_TONE_VALUES ;  // OK

// AMP INTERPOLATION CONTROL
struct func target_AmpInterpControl ; float *target_AmpInterpControl_VALUES ;  // OK



// FREQ SHIFT CONTROL
struct  func  target_FreqInterpControl ; float *target_FreqInterpControl_VALUES ; // OK 

// TIME INTERPOLATION CONTROL
struct func target_TimeInterpControl ; // OK  


//  INHARMONATOR RELEASE
struct  func  INHARM_release ; // NOT RESPONSIVE TO DIFFERENT DELAYS

//  INHARMONATOR ATTACK
struct  func  INHARM_attack; // NOT RESPONSIVE TO DIFFERENT DELAYS

//  FREQ MOVING AVERAGE COEFFICIENT
struct  func  INHARM_freqsmooth ; // NOT RESPONSIVE TO DIFFERENT DELAYS  


// SPECTRUM WARPSHAPE INDEX
struct  func  warpshape ; // NOT RESPONSIVE TO DIFFERENT DELAYS  


//*************** SOURCE 

// SOURCE  FREQUENCY SHIFT ADDER
struct  func  SOURCE_harmadd ; // OK

// SOURCE  PITCH MULTIPLIER
struct  func  SOURCE_ptrans ; // OK

// SOURCE DECIBELS
struct  func  SOURCE_dB ; // OK

//  SOURCE RELEASE
struct  func  SOURCE_release ; // OK 

//  SOURCE ATTACK
struct  func  SOURCE_attack ; // OK

// SOURCE TIME DELAY
struct func SOURCE_delayT ;// OK

//********

//*****************INITIALIZE

TARGETS_timedelayresponsetime.L = 1. ; TARGETS_timedelayresponsetime.n = 1. ; TARGETS_timedelayresponsetime.A[ 0 ] = 0. ; 
NON_TARGETS_timedelayresponsetime.L = 1. ; NON_TARGETS_timedelayresponsetime.n = 1. ; 
	NON_TARGETS_timedelayresponsetime.A[ 0 ] = 0. ; 

// PARTIAL BANDWIDTH
partial_bandwidth.L = 1. ; partial_bandwidth.n = 1. ; partial_bandwidth.A[ 0 ] = 1. ; 


// NON TARGET  FREQUENCY SHIFT ADDER
NON_target_harmadd.L = 1. ; NON_target_harmadd.n = 1. ; NON_target_harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// NON TARGET  PITCH TRANSPOSITION IN SEMITONES
NON_target_ptrans.L = 1. ; NON_target_ptrans.n = 1. ; NON_target_ptrans.A[ 0 ] = 0. ; 

// AMP INTERPOLATION CONTROL
target_AmpInterpControl.L = 1. ; target_AmpInterpControl.n = 1. ; target_AmpInterpControl.A[ 0 ] = 1. ; 

// FREQ INTERPOLATION CONTROL
target_FreqInterpControl.L = 1. ; target_FreqInterpControl.n = 1. ; target_FreqInterpControl.A[ 0 ] = 1. ; 

// TIME INTERPOLATION CONTROL
target_TimeInterpControl.L = 1. ; target_TimeInterpControl.n = 1. ; target_TimeInterpControl.A[ 0 ] = 1. ; 

// FUNDAMENTAL FREQUENCY OR OCTAVE.PITCHCLASS
Fund_Freq_or_OctavePointPitchClass.L = 1. ; Fund_Freq_or_OctavePointPitchClass.n = 1. ; 
	Fund_Freq_or_OctavePointPitchClass.A[ 0 ] = 60. ; 


// TARGET DECIBELS
target_dB.L = 1. ; target_dB.n = 1. ; target_dB.A[ 0 ] = 0. ; 

// NON TARGET DECIBELS
NON_target_dB.L = 1. ; NON_target_dB.n = 1. ; NON_target_dB.A[ 0 ] = 0. ; 

// NON TARGET DELAY TIME
NON_target_delayT.L = 1. ; NON_target_delayT.n = 1. ; NON_target_delayT.A[ 0 ] = 0. ; 


NON_target_decayT.L = 1. ; NON_target_decayT.n = 1. ; NON_target_decayT.A[ 0 ] = 0. ; 

// TARGET  PITCH TRANSPOSITION IN SEMITONES
target_ptrans.L = 1. ; target_ptrans.n = 1. ; target_ptrans.A[ 0 ] = 0. ; 

// TARGET  FREQUENCY SHIFT ADDER
target_harmadd.L = 1. ; target_harmadd.n = 1. ; target_harmadd.A[ 0 ] = 0. ; 

//  INHARMONATOR RELEASE
INHARM_release.L = 1. ; INHARM_release.n = 1. ; INHARM_release.A[ 0 ] = 0. ; 

//  INHARMONATOR ATTACK
INHARM_attack.L = 1. ; INHARM_attack.n = 1. ; INHARM_attack.A[ 0 ] = 0. ; 

//  FREQ MOVING AVERAGE COEFFICIENT
INHARM_freqsmooth.L = 1. ; INHARM_freqsmooth.n = 1. ; INHARM_freqsmooth.A[ 0 ] = 0. ; 




// SOURCE  FREQUENCY SHIFT ADDER	OK
SOURCE_harmadd.L = 1. ; SOURCE_harmadd.n = 1. ; SOURCE_harmadd.A[ 0 ] = 0. ; 

// SOURCE  PITCH MULTIPLIER OK
SOURCE_ptrans.L = 1. ; SOURCE_ptrans.n = 1. ; SOURCE_ptrans.A[ 0 ] = 0. ; 

// SOURCE DECIBELS OK
SOURCE_dB.L = 1. ; SOURCE_dB.n = 1. ; SOURCE_dB.A[ 0 ] = 0. ; 

//   SOURCE RELEASE OK
SOURCE_release.L = 1. ; SOURCE_release.n = 1. ; SOURCE_release.A[ 0 ] = 0. ; 

//   SOURCE ATTACK OK
SOURCE_attack.L = 1. ; SOURCE_attack.n = 1. ; SOURCE_attack.A[ 0 ] = 0. ; 

// SOURCE TIME DELAY OK
SOURCE_delayT.L = 1. ; SOURCE_delayT.n = 1. ; SOURCE_delayT.A[ 0 ] = 0. ; 



// SPECTRUM WARPSHAPE INDEX
warpshape.L = 1. ; warpshape.n = 1. ; warpshape.A[ 0 ] = 0. ; 

if( argc < 2 )usage() ; 

    while( (ch= crack( argc, argv, 
	"_|=|a|A|b|B|c|C|d|D|e|E|f|F|g|G|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|W|y|Y|x|X|z|Z|",
	0  )) != CRACK_DONE_FLAG ) {
	switch(ch) { 


	    case 'n':   strcpy(tempstring, arg_option);
			NON_TARGETS_timedelayresponsetime.fp = crackstring( tempstring, 
			    &NON_TARGETS_timedelayresponsetime );
			break;

	    case 'K':	NON_TARGETS_randomizationFlag = (int) crackfloat( arg_option, ch );
			break;


	    case 'c':   strcpy(tempstring, arg_option);
			TARGETS_timedelayresponsetime.fp = crackstring( tempstring, 
			    &TARGETS_timedelayresponsetime );
			break;


	    case 'H':	TARGETS_randomizationFlag = (int) crackfloat( arg_option, ch );
			break;

	    case 'N':	N = (int) crackfloat( arg_option, ch );
			break;
	    case 'M':	Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'I':   tfactor = crackfloat( arg_option, ch );
			break;
	    case 'A':   strcpy(tempstring, arg_option);
			dBgain.fp = crackstring( tempstring, 
			    &dBgain );
			break;

           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;


	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;


	    case 'W':   strcpy(tempstring, arg_option);
			warpshape.fp = crackstring( tempstring, 
			    &warpshape );
			break;



// NON TARGETS
	    case 'a':   strcpy(tempstring, arg_option);
			NON_target_harmadd.fp = crackstring( tempstring, 
			    &NON_target_harmadd );
			break;
	    case 'P':   strcpy(tempstring, arg_option);
			NON_target_ptrans.fp = crackstring( tempstring, &NON_target_ptrans ); 
			break;
	    case 'G':   strcpy(tempstring, arg_option);
			NON_target_dB.fp = crackstring( tempstring, 
			    &NON_target_dB );
			break;
	    case 'j':   strcpy(tempstring, arg_option);
			NON_target_delayT.fp = crackstring( tempstring, 
			    &NON_target_delayT );
			break;
	    case 'E':   strcpy(tempstring, arg_option);
			NON_target_decayT.fp = crackstring( tempstring, 
			    &NON_target_decayT );
			break;




// TARGETS

	    case 'x':   strcpy(tempstring, arg_option);
			partial_bandwidth.fp = crackstring( tempstring, 
			    &partial_bandwidth );
			break;

	    case 'q':   strcpy(tempstring, arg_option);
			target_harmadd.fp = crackstring( tempstring, 
			    &target_harmadd );
			break;
	    case 'X':   strcpy(tempstring, arg_option);
			target_ptrans.fp = crackstring( tempstring, 
			    &target_ptrans );
			break;
	    case 'm':   strcpy(tempstring, arg_option);
			target_dB.fp = crackstring( tempstring, 
			    &target_dB );
			break;

	    case 'U':   strcpy(tempstring, arg_option);
			target_AmpInterpControl.fp = crackstring( tempstring, 
			    &target_AmpInterpControl );
			break;
	    case 'S':   strcpy(tempstring, arg_option);
			target_FreqInterpControl.fp = crackstring( tempstring, 
			    &target_FreqInterpControl );
			break;
	    case 'T':   strcpy(tempstring, arg_option);
			target_TimeInterpControl.fp = crackstring( tempstring, 
			    & target_TimeInterpControl );
			break;


	    case 'Z':   method = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'f':   strcpy(tempstring, arg_option);
			Fund_Freq_or_OctavePointPitchClass.fp = crackstring( tempstring, 
			    &Fund_Freq_or_OctavePointPitchClass );
			break;

	    case 'F':   strcpy(datafile, arg_option);
			break;

	    case 'v':   strcpy(tempstring, arg_option);
			INHARM_attack.fp = crackstring( tempstring, 
			    &INHARM_attack );
			break;
	    case 'V':   strcpy(tempstring, arg_option);
			INHARM_release.fp = crackstring( tempstring, 
			    &INHARM_release );
			break;
	    case 'Y':   strcpy(tempstring, arg_option);
			INHARM_freqsmooth.fp = crackstring( tempstring, 
			    &INHARM_freqsmooth );
			break;

// SOURCE
	    case 'Q':   strcpy(tempstring, arg_option);
			SOURCE_harmadd.fp = crackstring( tempstring, 
			    &SOURCE_harmadd );
			break;
	    case 'u':   strcpy(tempstring, arg_option);
			SOURCE_ptrans.fp = crackstring( tempstring, &SOURCE_ptrans ); 
			break;
	    case 'r':   strcpy(tempstring, arg_option);
			SOURCE_dB.fp = crackstring( tempstring, 
			    &SOURCE_dB );
			break;

	    case 'l':   strcpy(tempstring, arg_option);
			SOURCE_attack.fp = crackstring( tempstring, 
			    &SOURCE_attack );
			break;
	    case 'L':   strcpy(tempstring, arg_option);
			SOURCE_release.fp = crackstring( tempstring, 
			    &SOURCE_release );
			break;
	    case 'J':   strcpy(tempstring, arg_option);
			SOURCE_delayT.fp = crackstring( tempstring, 
			    & SOURCE_delayT );
			break;




// END SOURCE

			// DATA MODIFIERS 
	    case 'z':   data_partial_number_scaler = crackfloat( arg_option, ch ) ;
			break;
	    case 'R':   data_partial_number_shifter = crackfloat( arg_option, ch ) ;
			break;
	    case 'y':   data_decibel_scaler = crackfloat( arg_option, ch ) ;
			break;
	    case 'o':   data_time_delay_scaler = crackfloat( arg_option, ch ) ;
			break;
	    case 'O':   data_time_delay_shifter = crackfloat( arg_option, ch ) ;
			break;

	    case 'g':   data_time_decay_scaler = crackfloat( arg_option, ch ) ;
			break;
	    case 'k':   data_time_decay_shifter = crackfloat( arg_option, ch ) ;
			break;




	    case 'B':   PartialBandWindowType__rectangle_0__Hann_1__Welch_2 = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 't':	threshfacdB = crackfloat( arg_option, ch );
			break;
	}
    }
 
    TWOPI = 8.*atan(1.) ;

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "INHARMONATOR", 69 ) ; 
prline( 69,  "-" ) ; 

// MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
    cut_data_lines( datafile,  new_datafile,  NUM_PARAMETERS ) ; 


//**************************GET DATA	
// READ IN PARTIAL AMPS AND SHIFTS
// OPEN FILE
	if( (data = fopen( new_datafile, "r")) == NULL ){
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  new_datafile ) ; 
	    exit(EXIT_FAILURE); 
	}
// COUNT VALUES IN FILE
	k = 0 ; 
	while( fscanf( data,  " %f ",  &temp ) != EOF ) k++ ; 			
	numberOfTargetPartials = k / NUM_PARAMETERS ;  
	rewind( data ) ;    
// ALLOCATE SPACE FOR PARTIALS
    fvec( PP, k ) ;	/* PARTIALS */
// READ IN VALUES
//    while( fscanf( data,  " %f ",  &PP[ k ] ) != 0 ) k++ ; 			


prline( 69,  "-" ) ;
prbanner( "INHARMONATOR: DATAFILE VALUES -- AS SPECIFIED", 69 ) ; 
prp( &Fund_Freq_or_OctavePointPitchClass,  "INHARMONATOR: FUNDAMENTAL FREQUENCY (IN HZ OR OCTAVE.PITCHCLASS)" ) ; 
prline( 69,  "-" ) ;

if(method == 0){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Multiplier  |Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Shift       |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Factor      |            |            |            |" ) ; 

}else if(method == 1){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Frequency   |Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Shift       |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Point       |            |            |            |" ) ; 

}else if(method == 2){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Octave.Pitch|Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Class       |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Shift Point |            |            |            |" ) ; 

}else if(method == 3){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Partial     |Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Number      |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Shift Point |            |            |            |" ) ; 

}else{
    fprintf( stderr, "\n\n%d IS NOT AN ACCEPTED DATA FORM\n\n",  method ) ; 
    exit(EXIT_FAILURE) ; 
}
prline( 13 * NUM_PARAMETERS,  "." ) ;

// READ IN VALUES
k = 0 ; 
for( i = 0; i < numberOfTargetPartials ; i++ ){
    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < NUM_PARAMETERS ; j++ ){
	fscanf( data,  " %f ",  &PP[ k ] ) ; fprintf( stderr,  "%-12.3f|", PP[ k ]  ) ;
	k++ ; 			
    }
} 
prline( 13 * NUM_PARAMETERS,  "*" ) ;
fprintf( stderr,  "\n\n" ) ; 

// ******
// ************* CONVERTED, SCALED AND SHIFTED.

prline( 69,  "-" ) ;
prbanner( "INHARMONATOR: DATAFILE VALUES -- CONVERTED, SCALED AND SHIFTED", 69 ) ; 
prp( &Fund_Freq_or_OctavePointPitchClass,  "INHARMONATOR: FUNDAMENTAL FREQUENCY (IN HZ OR OCTAVE.PITCHCLASS)" ) ; 
prline( 69,  "-" ) ;

if(method == 0){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Multiplier  |Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Shift       |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Factor      |            |            |            |" ) ; 

}else if(method == 1){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Frequency   |Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Shift       |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Point       |            |            |            |" ) ; 

}else if(method == 2){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Octave.Pitch|Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Class       |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Shift Point |            |            |            |" ) ; 

}else if(method == 3){
prline( 13 * NUM_PARAMETERS,  "*" ) ;
    fprintf( stderr,  "\nPartial     |Partial     |Decibels    |Delay Time  |Decay Time  |" ) ; 
    fprintf( stderr,  "\nNumber      |Number      |            |            |            |" ) ; 
    fprintf( stderr,  "\n            |Shift Point |            |            |            |" ) ; 

}else{
    fprintf( stderr, "\n\n%d IS NOT AN ACCEPTED DATA FORM\n\n",  method ) ; 
    exit(EXIT_FAILURE) ; 
}
prline( 13 * NUM_PARAMETERS,  "." ) ;

// READ IN VALUES
k = 0 ; 
for( i = 0; i < numberOfTargetPartials ; i++ ){
    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < NUM_PARAMETERS ; j++ ){

	    if( j == 0 ){
		PP[ k ] = (   ( data_partial_number_scaler * (PP[ k ] - 1.) ) + 1.   ) + data_partial_number_shifter; 
		if( PP[ k ] < 1. ) PP[ k ] = 1. ;  
	    } ; 
	    if( j == 2 ) PP[ k ] *= data_decibel_scaler ; 
	    if( j == 3 ){
		PP[ k ] = (PP[ k ] * data_time_delay_scaler) + data_time_delay_shifter ;
	    		// FIND MAX TIME DELAY
		if( PP[ k ] > HARMONY_maxDelayT ) HARMONY_maxDelayT = PP[ k ] ;  
	    } ; 
	    if( j == 4 ){
		PP[ k ] = (PP[ k ] * data_time_decay_scaler) + data_time_decay_shifter ;
		// FIND MAX DECAY TIME
		if( PP[ k ] > HARMONY_maxDecayT ) HARMONY_maxDecayT = PP[ k ] ; 
	    } ; 

	    fprintf( stderr,  "%-12.3f|", PP[ k ]  ) ;
	     k++ ; 			
    }
} 

fvec( delayTimes, numberOfTargetPartials ) ; 
fvec( decayTimes, numberOfTargetPartials ) ;
fvec( dBLevels, numberOfTargetPartials ) ;
fvec( partialPitchShifts, numberOfTargetPartials ) ;
fvec( partialNumbers, numberOfTargetPartials ) ; 
fvec( timeRateScalers, numberOfTargetPartials ) ; 


for( i = 0; i < numberOfTargetPartials ; i++ ){
    partialNumbers[i] = PP[ (i * NUM_PARAMETERS) + 0] ; 
    partialPitchShifts[i] = PP[ (i * NUM_PARAMETERS) + 1] ; 
    dBLevels[i] = PP[ (i * NUM_PARAMETERS) + 2] ; 
    delayTimes[i] = PP[ (i * NUM_PARAMETERS) + 3] ; 
    decayTimes[i] = PP[ (i * NUM_PARAMETERS) + 4] ;
    timeRateScalers[i] = 1. ;  
} ; 

prline( 13 * NUM_PARAMETERS,  "*" ) ;
fprintf( stderr,  "\n\n" ) ; 


fvec( target_ptrans_VALUES, numberOfTargetPartials ) ; 
fvec( pmt_TONE_VALUES, numberOfTargetPartials ) ; 
fvec( target_harmadd_VALUES, numberOfTargetPartials ); 
fvec( target_FreqInterpControl_VALUES, numberOfTargetPartials ) ; 
fvec( target_AmpInterpControl_VALUES, numberOfTargetPartials ) ; 
fvec( target_dB_VALUES, numberOfTargetPartials ) ; 
fvec( target_gain_TONE_VALUES, numberOfTargetPartials ); 
fvec( TARGETS_timedelayresponsetime_VALUES, numberOfTargetPartials ) ; 
fvec( TARGETS_randelayTsmoothc_VALUES, numberOfTargetPartials ) ; 
fvec( TARGETS_minusrandelayTsmoothc_VALUES, numberOfTargetPartials ) ; 
fvec( partial_bandwidth_VALUES, numberOfTargetPartials ) ; 

// ****
// HARMONY
HARMONY_maxDelayT += HARMONY_maxDecayT ; 
findFuncMinMaxAvg( &NON_target_delayT, &funcMin, &funcMax, &funcAvg ); 
findFuncMinMaxAvg( &NON_target_decayT, &funcMin, &temp, &funcAvg ); 
funcMax += temp ; 
if( funcMax > HARMONY_maxDelayT ) HARMONY_maxDelayT = funcMax ; 


// SOURCE
findFuncMinMaxAvg( &SOURCE_delayT, &funcMin, &funcMax, &funcAvg ); 
if( funcMax > SOURCE_maxDelayT ) SOURCE_maxDelayT = funcMax ; 

// RINGTIME
ringTime = (SOURCE_maxDelayT > HARMONY_maxDelayT) ? SOURCE_maxDelayT :
		 HARMONY_maxDelayT ; 

prf( HARMONY_maxDelayT, "HARMONY_maxDelayT" ) ; 
prf( SOURCE_maxDelayT, "SOURCE_maxDelayT" ) ; 

// MAKE CHANNEL DELAY MEMORY
	// CHANNEL DELAYS
	HARMONY_maxNumOfDelayFrames = 
		1 + (int)((HARMONY_maxDelayT * frames_per_sec) + 0.5) ; 
	fvec( HARMONY_channel_delay, HARMONY_maxNumOfDelayFrames * (N + 2) ) ; 

	SOURCE_maxNumOfDelayFrames = 1 + (int)((SOURCE_maxDelayT * frames_per_sec) + 0.5) ; 
	fvec( SOURCE_channel_delay, SOURCE_maxNumOfDelayFrames * (N + 2) ) ; 




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

    IR = (float) I / (float) R ; 
    midC = (220.*pow(2., (3./12.))) ; 
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
    nyquist = R/2.0;
    obank = P != 0. ;
    if( P == 0.0 ) {P = 1.0;}
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    fundamental = (float) R / (float) N  ; 

    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 

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
prf( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
pri( I,  "      INTERPOLATION SAMPLES (samples between resynthesis frames)" ) ; 
prline( 1,  "*" ) ; 
prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (in dB)" ) ; 
prline( 1,  "*" ) ; 

prp( &dBgain,  "MASTER GAIN (in dB)" ) ;  
prline( 1,  "*" ) ; 

prp( &NON_target_harmadd,   "NON TARGETS: FREQUENCY SHIFT (in Hz)" ) ; 
prp( &NON_target_ptrans,  "NON TARGETS: PITCH TRANSPOSITION (in semitones)" ) ; 
prp( &NON_target_dB,  "NON TARGETS: GAIN (in dB)" ) ; 
prp( &NON_target_delayT, "NON TARGETS: DELAY TIME in seconds" ) ; 
prp( &NON_target_decayT, "NON TARGETS: FEEDBACK DECAY TIME in seconds" ) ; 
prp( &NON_TARGETS_timedelayresponsetime, "NON TARGETS: TIME DELAY RESPONSE TIME (in seconds)" ) ; 
if( NON_TARGETS_randomizationFlag == 1 )prt( "NON TARGETS DELAY TIMES RANDOMIZED" ) ;  

prline( 1,  "*" ) ; 




prp( &Fund_Freq_or_OctavePointPitchClass,  "INHARMONATOR: FUNDAMENTAL FREQUENCY OR OCTAVE.PITCHCLASS" ) ; 
prp( &INHARM_attack,  "INHARMONATOR: ENVELOPE ATTACK TIME(in secs)" ) ;
prp( &INHARM_release,  "INHARMONATOR: ENVELOPE RELEASE TIME(in secs)" ) ; 
prp( &INHARM_freqsmooth,  "INHARMONATOR: FREQUENCY CHANGE RESPONSE TIME(in secs)" ) ; 
prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 

prp( &partial_bandwidth, "TARGETS: PARTIAL BANDWIDTH" ) ; 
if( PartialBandWindowType__rectangle_0__Hann_1__Welch_2 == 0 ) prt( "PARTIAL BAND WINDOW TYPE SET TO RECTANGULAR" ) ; 
if( PartialBandWindowType__rectangle_0__Hann_1__Welch_2 == 0 ) prt( "PARTIAL BAND WINDOW TYPE SET TO HANN" ) ; 
if( PartialBandWindowType__rectangle_0__Hann_1__Welch_2 == 0 ) prt( "PARTIAL BAND WINDOW TYPE SET TO WELCH" ) ; 

prp( &target_dB,  "TARGETS: GAIN (in dB)" ) ; 
prp( &target_harmadd,  "TARGETS: FREQUENCY SHIFT (in Hz)" ) ; 
prp( &target_ptrans,  "TARGETS: PITCH TRANSPOSITION (in semitones)" ) ; 
prp( &target_AmpInterpControl,  "TARGETS: (0-1) AMPLITUDE INTERPOLATION CONTROL" ) ; 
prp( &target_FreqInterpControl,  "TARGETS: (0-1) FREQUENCY INTERPOLATION CONTROL" ) ; 
prp( &target_TimeInterpControl,  "TARGETS: (0-1) TIME INTERPOLATION CONTROL" ) ; 
prp( &TARGETS_timedelayresponsetime, "TARGETS: TIME DELAY RESPONSE TIME" ) ; 
if( TARGETS_randomizationFlag == 1 )prt( "TARGETS: RANDOMIZED DELAY TIMES" ) ; 


prline( 1,  "*" ) ; 

prp( &SOURCE_harmadd,  "SOURCE: FREQUENCY SHIFT (in Hz)" ) ; 
prp( &SOURCE_ptrans,  "SOURCE: PITCH TRANSPOSITION (in semitones)" ) ; 
prp( &SOURCE_dB,  "SOURCE: GAIN (in dB)" ) ; 
prp( &SOURCE_release,  "SOURCE: ENVELOPE RELEASE TIME(in secs)" ) ; 
prp( &SOURCE_attack,  "SOURCE: ENVELOPE ATTACK TIME(in secs)" ) ; 
prp( &SOURCE_delayT, "SOURCE: TIME DELAY (in seconds)" ) ; 
prline( 1,  "*" ) ; 
prp( &warpshape, "TARGETS SPECTRUM WARPSHAPE INDEX" ) ; 
 
prf( data_partial_number_scaler, "INPUT DATA: PARTIAL NUMBER SCALER" ) ; 
prf( data_partial_number_shifter, "INPUT DATA: PARTIAL NUMBER SHIFTER" ) ; 
prf( data_decibel_scaler, "INPUT DATA: DB SCALER" ) ; 
prf( data_time_delay_scaler, "INPUT DATA: TIME DELAY SCALER" ) ; 
prf( data_time_delay_shifter, "INPUT DATA: TIME DELAY SHIFTER" ) ; 
prf( data_time_decay_scaler, "INPUT DATA: TIME DECAY SCALER" ) ; 
prf( data_time_decay_shifter, "INPUT DATA: TIME DECAY SHIFTER" ) ; 


if( strcmp( datafile, "") == 0  ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A DATA FILE. BYE.\n" ) ;
    exit(EXIT_FAILURE);
}

if( (SOURCE_dB.A[0] > threshfacdB) || (SOURCE_dB.n != 1.)){
    // INCLUDE SOURCE
    sourceflag = 1 ;
    fprintf( stderr, "\n SOURCE SOUND INCLUDED......." ) ; 
    fprintf( stderr, "\n\t(SOURCE DECIBEL LEVEL BASED ON FUNCTION, \n\tOR IS > RESYNTHESIS THRESHOLD OF %f dB)", 
	threshfacdB ) ; 
}else{
    // EXCLUDE SOURCE
    sourceflag = 0 ; 
    fprintf( stderr, "\n ====> * SOURCE SOUND EXCLUDED * <=======" ) ; 
    fprintf( stderr, "\n\t(SOURCE DECIBEL LEVEL  < RESYNTHESIS THRESHOLD OF %f dB)", 
	threshfacdB ) ; 
}
    

    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( bufferSum, N ) ;		/* FFT buffer */
    fvec( buffer_harmony_delayed_inputs, N ) ; 
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */
    fvec( previous_source_channel_out, N+2 ) ;	/* previous analysis channels */

    fvec( source_channel_out, N+2 ) ; 

// MAKE INHARMONATOR  ARRAY SPACE
    fvec( previous_harmony, N+2 ) ;	/* previous analysis channels */
    fvec( harmony, (N+2) ) ;	/* inharmonator channels */
    fvec( harmony_delayed_inputs, (N+2) ) ; 
    fvec( harmony_delay_now, N+2 ) ; 



// MAKE SHIFT ARRAY
    fvec( F, N+2 ) ;	/* SHIFT ARRAY */

    fvec( timeDelay, N2 + 1 ) ; 
    fvec( previous_timeDelay, N2 + 1 ) ; 
    fvec( decayTime, N2 + 1 ) ; 
    fvec( timeDelayInFrames, N2 + 1 ) ; 
    fvec( decayTimeInFrames, N2 + 1 ) ; 
    ivec( partialDataLine, N2 + 1 ) ; 


// MAKE THRESH AMP
	threshfac = dB_to_amp( threshfacdB ) ; 

// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 



//************************************
//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; 

    for( i = 0; i < (HARMONY_maxNumOfDelayFrames * (N + 2)); i++ ) HARMONY_channel_delay[i] = 0.  ; 
    for( i = 0; i < (SOURCE_maxNumOfDelayFrames * (N + 2)); i++ ) SOURCE_channel_delay[i] = 0.  ; 


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

	// TRANSFER channel INTO CIRCULAR DELAY LINE FOR TARGETS/NON-TARGETS
	HARMONY_frameNowChannelDelayIndex = frame_count ; 
	while( HARMONY_frameNowChannelDelayIndex >=
				 HARMONY_maxNumOfDelayFrames ) 
		HARMONY_frameNowChannelDelayIndex -=
				  HARMONY_maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2) ; i++ ) 
		HARMONY_channel_delay[ (HARMONY_frameNowChannelDelayIndex * (N + 2)) + i] =
			 channel[i] ; 


	// TRANSFER channel INTO CIRCULAR DELAY LINE FOR SOURCE
	SOURCE_frameNowChannelDelayIndex = frame_count ; 
	while( SOURCE_frameNowChannelDelayIndex >= SOURCE_maxNumOfDelayFrames ) 
		SOURCE_frameNowChannelDelayIndex -=  SOURCE_maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2) ; i++ ) 
		SOURCE_channel_delay[ (SOURCE_frameNowChannelDelayIndex * (N + 2)) + i] = channel[i] ; 


	// TRANSFER DELAYED SOURCE CHANNELS INTO channel_out
	SOURCE_delayT.A[ 0 ] = fval( &SOURCE_delayT, dur, t );
	SOURCE_thisFrameDelay = SOURCE_frameNowChannelDelayIndex - 
			(int)((SOURCE_delayT.A[0] * frames_per_sec) + 0.5) ; 
	while( SOURCE_thisFrameDelay < 0) SOURCE_thisFrameDelay += SOURCE_maxNumOfDelayFrames ; 
	for( i = 0; i < (N + 2) ; i++ ) source_channel_out[i] = 
			SOURCE_channel_delay[ (SOURCE_thisFrameDelay * (N + 2)) + i] ; 



	// SETUP PREVIOUS SOURCE CHANNEL OUT
//	    if( !frame_count )for(i = 0; i < (N + 2); i++)
//		    previous_source_channel_out[ i ] = source_channel_out[ i ] ; 
// NOTE: HANDLED IN CALL TO smooth



//*************************
// GET THE VALUES
//*************************

//  SHIFT, GAIN, AND MULTIPLIER

// ALL
	dBgain.A[ 0 ] =  fval( &dBgain, dur, t ); gain = dB_to_amp( dBgain.A[ 0 ] ) ; 


//***** INHARMONATOR


	INHARM_freqsmooth.A[ 0 ] = fval( &INHARM_freqsmooth, dur, t );
	    smooth_setup( INHARM_freqsmooth.A[ 0 ], &ifreqsmoothc, &ifreqsmoothm, IR ) ; 


	INHARM_release.A[ 0 ] = fval( &INHARM_release, dur, t );
	    smooth_setup( INHARM_release.A[ 0 ], &ireleasec, &ireleasem, IR ) ; 


	INHARM_attack.A[ 0 ] = fval( &INHARM_attack, dur, t );
	    smooth_setup( INHARM_attack.A[ 0 ], &iattackc, &iattackm, IR ) ; 

	warpshape.A[ 0 ] =  fval( &warpshape, dur, t );



// TARGETS
	Fund_Freq_or_OctavePointPitchClass.A[ 0 ] =  fval( &Fund_Freq_or_OctavePointPitchClass, dur, t );
	if( Fund_Freq_or_OctavePointPitchClass.A[ 0 ] <= 12. ){
			// OCTAVE.PITCHCLASS
	    FundamentalFreqNow = OPPC_to_Hz( Fund_Freq_or_OctavePointPitchClass.A[ 0 ] );  
	}else{
			// HZ
	    FundamentalFreqNow = Fund_Freq_or_OctavePointPitchClass.A[ 0 ] ; 
	} ; 

      getGlobalFunctionValues(
            TARGETS_timedelayresponsetime_VALUES, numberOfTargetPartials, dur, delayTimes, timeRateScalers, &TARGETS_timedelayresponsetime 
      ) ; 
      for( i = 0; i < numberOfTargetPartials; i++ )
		    smooth_setup( TARGETS_timedelayresponsetime_VALUES[ i ], 
			    &TARGETS_randelayTsmoothc_VALUES[ i ], &TARGETS_minusrandelayTsmoothc_VALUES[ i ], IR ) ; 


//	partial_bandwidth.A[ 0 ] =  fval( &partial_bandwidth, dur, t );
      getGlobalFunctionValues(
            partial_bandwidth_VALUES, numberOfTargetPartials, dur, delayTimes, timeRateScalers, &partial_bandwidth 
      ) ; 


      getGlobalFunctionValues(
            target_harmadd_VALUES, numberOfTargetPartials, dur, delayTimes, timeRateScalers, &target_harmadd 
      ) ; 

      getGlobalFunctionValues(
            target_ptrans_VALUES, numberOfTargetPartials, dur, delayTimes, timeRateScalers, &target_ptrans 
      ) ; 
      for( i = 0; i < numberOfTargetPartials; i++ ) pmt_TONE_VALUES[i] = semitones_to_mult( target_ptrans_VALUES[ i ] ) ;




      getGlobalFunctionValues(
            target_dB_VALUES, numberOfTargetPartials, dur, delayTimes, timeRateScalers, &target_dB 
      ) ; 
      for( i = 0; i < numberOfTargetPartials; i++ ) target_gain_TONE_VALUES[i] = dB_to_amp( target_dB_VALUES[ i ] ) ;

      getGlobalFunctionValues(
            target_AmpInterpControl_VALUES, numberOfTargetPartials, dur, delayTimes, timeRateScalers, &target_AmpInterpControl 
      ) ; 


      getGlobalFunctionValues(
            target_FreqInterpControl_VALUES, numberOfTargetPartials, dur, delayTimes, timeRateScalers, &target_FreqInterpControl 
      ) ; 

	target_TimeInterpControl.A[ 0 ] =  fval( &target_TimeInterpControl, dur, t );

// NONTARGETS
	NON_target_delayT.A[ 0 ] =  fval( &NON_target_delayT, dur, t );

	NON_TARGETS_timedelayresponsetime.A[ 0 ] = fval( &NON_TARGETS_timedelayresponsetime, dur, t - NON_target_delayT.A[ 0 ] );
		    smooth_setup( NON_TARGETS_timedelayresponsetime.A[ 0 ], 
			    &NON_TARGETS_randelayTsmoothc, &NON_TARGETS_minusrandelayTsmoothc, IR ) ; 


	NON_target_harmadd.A[ 0 ] =  fval( &NON_target_harmadd, dur, t - NON_target_delayT.A[ 0 ] );
	NON_target_ptrans.A[ 0 ] = fval( &NON_target_ptrans, dur, t - NON_target_delayT.A[ 0 ] );
		    pmnt = semitones_to_mult( NON_target_ptrans.A[ 0 ] ) ;
	NON_target_dB.A[ 0 ] =  fval( &NON_target_dB, dur, t - NON_target_delayT.A[ 0 ] );
		    non_target_gain = dB_to_amp( NON_target_dB.A[ 0 ] ) ; 
	NON_target_decayT.A[ 0 ] =  fval( &NON_target_decayT, dur, t - NON_target_delayT.A[ 0 ] );




//***** SOURCE
    if(sourceflag){
	    SOURCE_harmadd.A[ 0 ] =  fval( &SOURCE_harmadd, dur, t - SOURCE_delayT.A[ 0 ] );
	    SOURCE_ptrans.A[ 0 ] = fval( &SOURCE_ptrans, dur, t - SOURCE_delayT.A[ 0 ] );
			pms = semitones_to_mult( SOURCE_ptrans.A[ 0 ] ) ;
	    SOURCE_dB.A[ 0 ] =  fval( &SOURCE_dB, dur, t - SOURCE_delayT.A[ 0 ] );
			sgain = dB_to_amp( SOURCE_dB.A[ 0 ] ) ; 

	    SOURCE_release.A[ 0 ] = fval( &SOURCE_release, dur, t - SOURCE_delayT.A[ 0 ] );
			smooth_setup( SOURCE_release.A[ 0 ], &sreleasec, &sreleasem, IR ) ; 

	    SOURCE_attack.A[ 0 ] = fval( &SOURCE_attack, dur, t - SOURCE_delayT.A[ 0 ] );
			smooth_setup( SOURCE_attack.A[ 0 ], &sattackc, &sattackm, IR ) ; 


    }

//*************************

//** GET NEW VALUES IF  FIRST, OR IF FUNDAMENTAL IS CHANGING, OR IF TIME DELAYS ARE RANDOMIZED, ALL OF WHICH
// CHANGE THE TARGET BINS.
    if( 	(frame_count == 0) || 
		(Fund_Freq_or_OctavePointPitchClass.n > 1.) || 
		(TARGETS_randomizationFlag == 1) ||
		(NON_TARGETS_randomizationFlag == 1) ||
		(NON_target_delayT.n > 1.) ||
		(NON_target_decayT.n > 1.) ||
		(partial_bandwidth.n > 1.)
		
    ){

	    //************************* SET UP FILTER ARRAY
	    // INITIALIZE SPACE

        for(i = 1, bin = 0; i < (N + 2) ; i += 2, bin++ ){
	        F[ i ] = 1. ; F[ i - 1 ] = -1. ; 
	        timeDelay[ bin ] = -1. ; // NON_target_delayT.A[ 0 ] ;
	        decayTime[ bin ] = 0. ; // NON_target_decayT.A[ 0 ] ;
	   } ; 




	    // STEP THROUGH ALL  PARTIALS
        for( j = 0, dataLine = 0; j < k; j += NUM_PARAMETERS, dataLine++ ){

	       // FREQ OF PARTIAL
	       part = FundamentalFreqNow * PARTIAL_NUMBER ; 
	       // INDEX OF PARTIAL
	       ipartial = 1 + (2 * ( rint((double)((PARTIAL_NUMBER * FundamentalFreqNow) / fundamental)) )) ; 
	       // INDEX OF 1/2 PARTIAL BELOW
	       i1 = 1 + (2 * ( rint((double)(((PARTIAL_NUMBER - (partial_bandwidth_VALUES[ dataLine ] * 0.5)) 
		    	* FundamentalFreqNow) / fundamental)) )) ; 
	       // INDEX OF 1/2 PARTIAL ABOVE
	       i2 = 1 + (2 * ( rint((double)(((PARTIAL_NUMBER + (partial_bandwidth_VALUES[ dataLine ] * 0.5)) 
		    * FundamentalFreqNow) / fundamental)) ))  ;  
	       // CHANGE RANGE OF FILTER 1/2WAY ABOVE AND BELOW PARTIAL POINT.

	       //**** SETUP THE BIN OF THE PARTIAL
	       i = ipartial ; bin = (i - 1) / 2 ; 
	       if( ( i >= 0 ) && (i < N ) ){

	            // TIME DELAY
                if( TARGETS_randomizationFlag == 1 ){ 
			    timeDelay[ bin ] = PARTIAL_TIME_DELAY * randf( 0., 1. ) ; 
			    if( frame_count == 0 ) previous_timeDelay[ bin ] = timeDelay[ bin ] ; 
			    // SMOOTH
			    timeDelay[ bin ] = (TARGETS_randelayTsmoothc_VALUES[ dataLine ] * previous_timeDelay[ bin ] )
					+ ( TARGETS_minusrandelayTsmoothc_VALUES[ dataLine ] * timeDelay[ bin ] );
		     }else{ 
			    timeDelay[ bin ] = PARTIAL_TIME_DELAY ; 
		     } ; 
	          decayTime[ bin ] = PARTIAL_FEEDBACK_DECAY_TIME ; 
                partialDataLine[ bin ] = dataLine ; 

	    	     if( method == 2 ){

		        //**** OCTAVE.PITCHCLASS
		        F[ i ] = OPPC_to_Hz( PARTIAL_SHIFT_DATA ) / part ; 

	           } else if(method == 1) {

		        //**** FREQ SHIFT POINT
		        F[ i ] = PARTIAL_SHIFT_DATA / part ; 

	           } else if(method == 0) {

		        //**** MULTIPLER 
		        F[ i ] = PARTIAL_SHIFT_DATA ;

	           }else if(method == 3) {
		    
		        //**** PARTIAL SHIFT POINT
		        F[ i ] =  (PARTIAL_SHIFT_DATA * FundamentalFreqNow ) / part ;    

	           }else {
		         fprintf( stderr, "\n\n%d IS NOT AN ACCEPTED DATA FORM\n\n",  method ) ; 
		         exit(EXIT_FAILURE) ; 
	           }
		
	           // DB TO AMP FOR PARTIAL BINS
	           F[ i - 1 ] = dB_to_amp( PARTIAL_DECIBELS );
	

            } 

	      //**** SETUP THE BINS BELOW THE PARTIAL
	      for(i = i1,  n = 2, bin = (i1 - 1) / 2;  i < ipartial; i += 2,  n += 2, bin++ ){

	            // TIME DELAY
		    if( TARGETS_randomizationFlag == 1 ){ 
			timeDelay[ bin ] = PARTIAL_TIME_DELAY * randf( 0., 1. ) ; 
			if( frame_count == 0 ) previous_timeDelay[ bin ] = timeDelay[ bin ] ; 
			// SMOOTH
			timeDelay[ bin ] = (TARGETS_randelayTsmoothc_VALUES[ dataLine ] * previous_timeDelay[ bin ] )
					+ ( TARGETS_minusrandelayTsmoothc_VALUES[ dataLine ] * timeDelay[ bin ] );
		    }else{ 
			timeDelay[ bin ] = PARTIAL_TIME_DELAY ; 
		    } ; 
	         decayTime[ bin ] = PARTIAL_FEEDBACK_DECAY_TIME ; 
              partialDataLine[ bin ] = dataLine ;
 
	         if( ( i >= 0 ) && (i < N ) ){

		    if( method == 2 ){
	    	         //**** OCTAVE.PITCHCLASS
	    	         F[ i ] = OPPC_to_Hz( PARTIAL_SHIFT_DATA ) / part ; 
	          } else if(method == 1) {
	              //**** FREQ SHIFT POINT
	              F[ i ] = PARTIAL_SHIFT_DATA / part ; 
	          } else if(method == 0) {
	             //**** MULTIPLER 
	             F[ i ] = PARTIAL_SHIFT_DATA ;
	          }else if(method == 3) {
	             //**** PARTIAL SHIFT POINT
	             F[ i ] =  (PARTIAL_SHIFT_DATA * FundamentalFreqNow ) / part ;    
	          }else {
	              fprintf( stderr, "\n\n%d IS NOT AN ACCEPTED DATA FORM\n\n",  method ) ; 
	              exit(EXIT_FAILURE) ; 
	          }
		
	          // DB TO AMP FOR PARTIAL BINS
	          F[ i - 1 ] = dB_to_amp( PARTIAL_DECIBELS );

	          // WINDOW
	          if( PartialBandWindowType__rectangle_0__Hann_1__Welch_2 == 1 )
	             F[ i - 1 ] = halfHannWindow( 1. - ((float) n / (float) (ipartial - i1)) ); 
	          else if (PartialBandWindowType__rectangle_0__Hann_1__Welch_2 == 2)
	              F[i - 1] = halfWelchWindow( 1. - ((float) n / (float) (ipartial - i1)) );  
       	     } 
          }    
    //**** SETUP THE BINS ABOVE THE PARTIAL
          for(i = i2,  n = 2, bin = (i2 - 1) / 2;  i > ipartial; i -= 2,  n += 2, bin-- ){

	            // TIME DELAY
              if( TARGETS_randomizationFlag == 1 ){ 
                  timeDelay[ bin ] = PARTIAL_TIME_DELAY * randf( 0., 1. ) ; 
			 if( frame_count == 0 ) previous_timeDelay[ bin ] = timeDelay[ bin ] ; 
			 // SMOOTH
			 timeDelay[ bin ] = (TARGETS_randelayTsmoothc_VALUES[ dataLine ] * previous_timeDelay[ bin ] )
					+ ( TARGETS_minusrandelayTsmoothc_VALUES[ dataLine ] * timeDelay[ bin ] );
              }else{ 
			  timeDelay[ bin ] = PARTIAL_TIME_DELAY ; 
		   } ; 
	        decayTime[ bin ] = PARTIAL_FEEDBACK_DECAY_TIME ; 
              partialDataLine[ bin ] = dataLine ;

              if( ( i >= 0 ) && (i < N ) ){

	            if( method == 2 ){
		          //**** OCTAVE.PITCHCLASS 
		          F[ i ] = OPPC_to_Hz( PARTIAL_SHIFT_DATA ) / part ; 
	            } else if(method == 1) {
		         //**** FREQ SHIFT POINT
		         F[ i ] = PARTIAL_SHIFT_DATA / part ; 
	            } else if(method == 0) {
		          //**** MULTIPLER 
		          F[ i ] = PARTIAL_SHIFT_DATA ;
	            }else if(method == 3) {
		          //**** PARTIAL SHIFT POINT
		          F[ i ] =  (PARTIAL_SHIFT_DATA * FundamentalFreqNow ) / part ;    
	            }else {
		          fprintf( stderr, "\n\n%d IS NOT AN ACCEPTED DATA FORM\n\n",  method ) ; 
		          exit(EXIT_FAILURE) ; 
	            }
		
		       // DB TO AMP FOR PARTIAL BINS
	            F[ i - 1 ] = dB_to_amp( PARTIAL_DECIBELS );

	            // WINDOW
	            if( PartialBandWindowType__rectangle_0__Hann_1__Welch_2 == 1 )
		            F[ i - 1 ] = halfHannWindow(   1. - ( (float) n / (float) (i2 - ipartial) )      ); 
	            else if (PartialBandWindowType__rectangle_0__Hann_1__Welch_2 == 2)
		            F[i - 1] = halfWelchWindow( 1. - ( (float) n / (float) (i2 - ipartial) )   );  
	        } 
        }
    }

}

    // NON TARGETS
    for(bin = 0; bin < N2 + 1; bin++ ){
	if( timeDelay[ bin ] == -1. ){
	    // TIME DELAY
	    if( NON_TARGETS_randomizationFlag == 1 ){ 
		timeDelay[ bin ] = NON_target_delayT.A[ 0 ] * randf( 0., 1. ) ; 
		if( frame_count == 0 ) previous_timeDelay[ bin ] = timeDelay[ bin ] ; 
		// SMOOTH
		timeDelay[ bin ] = (NON_TARGETS_randelayTsmoothc * previous_timeDelay[ bin ] )
					+ ( NON_TARGETS_minusrandelayTsmoothc * timeDelay[ bin ] );
	    }else{ 
		timeDelay[ bin ] = NON_target_delayT.A[ 0 ] ; 
	    } ; 
	    decayTime[ bin ] = NON_target_decayT.A[ 0 ] ; 
	} ;     

    }; 


    for(bin = 0; bin < N2 + 1; bin++ ){
	timeDelayInFrames[ bin ] = floor( (timeDelay[ bin ] * frames_per_sec) + 0.5  ) ; 
	decayTimeInFrames[ bin ] = floor( (decayTime[ bin ] * frames_per_sec) + 0.5  ) ; 
	previous_timeDelay[ bin ] = timeDelay[ bin ] ; // SAVE ALL PREVIOUS
    } ; 


//*************************END FILTER ARRAY SETUP

 



//************DEBUG
/*
	fprintf( stderr,"\n***********************");  
		 for( i = 1; i < 100; i = i + 2){
		 	fprintf( stderr,  
	    "\nF[ %d ]: %f, F[ %d  ]: %f, channel[%d]: %f",
		i, F[ i ], (i-1),  F[ i - 1 ] , i,   channel[ i ] ) ; 
		}

*/



//*****************
// MODIFICATIONS LOOPS
//*****************
//pd(0); 


 // ***** APPLY MASTER GAIN TO SOURCE
    for( i = 0; i < (N + 2); i = i + 2) source_channel_out[i] = source_channel_out[i] * gain ; 


// INHARMONATOR


// ************* INHARMONATOR CHANGES *******************
		    
//pd(1); 

    for( i = 1, bin = 0; i < (N + 2); i = i + 2, bin++ ){

	// TRANSFER FROM DELAY ARRAY
	HARMONY_thisFrameDelay = HARMONY_frameNowChannelDelayIndex - 
		(int)(( target_TimeInterpControl.A[ 0 ] * timeDelay[ bin ] * frames_per_sec ) + 0.5) ; 

	while( HARMONY_thisFrameDelay < 0) HARMONY_thisFrameDelay += HARMONY_maxNumOfDelayFrames ; 
	harmony[i] = harmony_delayed_inputs[i] = 
		HARMONY_channel_delay[ (HARMONY_thisFrameDelay * (N + 2)) + i] ;
	harmony[i - 1] = harmony_delayed_inputs[i - 1] = 
		HARMONY_channel_delay[ (HARMONY_thisFrameDelay * (N + 2)) + i - 1 ] ;  

	if( F[i - 1] != -1. ){
	    // **** TARGETS *****
         
	    // FREQ
	    temp = pmt_TONE_VALUES[ partialDataLine[ bin ] ] *  ( (harmony[i] *  F[ i ]) + target_harmadd_VALUES[ partialDataLine[ bin ] ] ) ;
	    // INTERPOLATE
 	    harmony[i] = harmony[i] + (target_FreqInterpControl_VALUES[ partialDataLine[ bin ] ] * (temp - harmony[i])); 
	    // AMP
	    harmony[i - 1] *= target_gain_TONE_VALUES[ partialDataLine[ bin ] ] ; 
	    if( harmony[i - 1] >= dB_to_amp( -600. ) ){ 
	        temp3 = amp_to_dB( harmony[i - 1] ) ; 
	        tv3 = harmony[i - 1] * F[i - 1]  ; 
		if( tv3 >= dB_to_amp( -600. ) ){
		    tv0 = amp_to_dB( tv3 ) ; 
		    tv1 = temp3 + (target_AmpInterpControl_VALUES[ partialDataLine[ bin ] ] * (tv0 - temp3)) ; 
	       	    harmony[i - 1] = dB_to_amp(  tv1 ) ; 
		} ; 
	    } ; 
	}else{
	    // **** NON TARGETS *****
	    // FREQ
	    harmony[i] = pmnt * (harmony[i] + NON_target_harmadd.A[ 0 ]) ;
	    // AMP
	    harmony[i - 1] = harmony[i - 1] * non_target_gain ; 			
	}  
		    

//************ SMOOTH FREQ CHANGES
//if( t > 14. )fprintf( stderr, "ifreqsmoothc: %f\t ifreqsmoothm: %f\n", ifreqsmoothc, ifreqsmoothm ) ; 
//	harmony[i] = (ifreqsmoothc * previous_harmony[i] ) + ( ifreqsmoothm * harmony[i] );
//	previous_harmony[i] = harmony[i] ; 
		    
 
 //******* 
    }

//if( t > 14.72) pd(3); 
//*************WARP THE TARGETS SPECTRUM
    spectmagwarp_inharm( harmony, F,  (N + 2), warpshape.A[ 0 ] ) ;


//************ATTACK AND RELEASE BOTH TARGETS AND NON
    // SMOOTH THE CHANGES TO THE SPECTRUM
    smooth( harmony, previous_harmony, (N + 2), iattackc, iattackm, ireleasec, ireleasem ) ; 


// *************END OF INHARMONATOR CHANGES *******************

// FEED BACK DELAY INPUTS INTO HARMONY_channel_delay
// FIRST ATTENUATE THE INPUTS BEFORE FEEDBACK
	// ATTENUATE DELAYED FILTER INPUTS AND ADD INTO DELAY ARRAY
	for(i = 0, bin = 0; i < (N + 2); i += 2, bin++){ 
	    if( decayTimeInFrames[ bin ] <= 0. ){
		// ZERO DECAY TIME
		harmony_delayed_inputs[ i ] = 0. ; 
	    }else{
		// NON ZERO DECAY TIME
		if(  timeDelayInFrames[ bin ] <= 0. ){
		    // ZERO DELAY TIME
		    harmony_delayed_inputs[ i ] =  0. ; 
		}else{
		    harmony_delayed_inputs[ i ] *= 
			dB_to_amp(-60. / ( decayTimeInFrames[ bin ] / timeDelayInFrames[ bin ] )) ; 
		} ; 	
	    } ; 
	} ; 


	unconvert2( harmony_delayed_inputs, buffer_harmony_delayed_inputs, N2, I, R ) ; 

	for(i = 0 ; i < (N + 2); i++) harmony_delay_now[i] = 
		HARMONY_channel_delay[ (HARMONY_frameNowChannelDelayIndex * (N + 2)) + i] ; 

	unconvert3( harmony_delay_now, bufferSum, N2, I, R ) ;

	for(i = 0; i < N; i++ ) bufferSum[i] = bufferSum[i] + buffer_harmony_delayed_inputs[i] ; 	    
	convert3( bufferSum, harmony_delay_now, N2, D, R ) ;	 

	for( i = 0; i < (N + 2) ; i++ ) 
		HARMONY_channel_delay[ (HARMONY_frameNowChannelDelayIndex * (N + 2)) + i] = 
			harmony_delay_now[i] ; 
	
 

//*************** SOURCE CHANGES ***********
// SOURCE
		 if(sourceflag){

		    // SMOOTH THE CHANGES TO THE SPECTRUM
		    smooth( source_channel_out, previous_source_channel_out, 
					(N + 2), sattackc, sattackm, sreleasec, sreleasem ) ; 

		   for( i = 1; i < (N + 2); i = i + 2){
		    //** SOURCE GAIN
		    source_channel_out[i - 1] = source_channel_out[i - 1] * sgain ; 

		    //** SOURCE ADD/TRANSPOSE 
		    temp = pms *  (SOURCE_harmadd.A[ 0 ] + source_channel_out[i] ) ;   

		    // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		    if((temp <= 0.) || (temp >= nyquist)) source_channel_out[i - 1] = 0. ; 
		    else  source_channel_out[i] = temp ;
 
 //*************** END  SOURCE CHANGES ***********


		  }
		}


		// ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		for(i = 1; i < (N + 2); i += 2 ){
			if(harmony[i] < 0.){ 
			    harmony[i - 1] = 0. ; harmony[i] = 0. ;   
			} ; 
			if(harmony[i] >= nyquist) { 
			    harmony[i-1] = 0. ; harmony[i] = nyquist ;
			} ; 
		} ; 

	    cutDC( harmony, N+2, 5. ) ; 

		
	    if(sourceflag){
		synt = getthresh( source_channel_out, N, threshfac );
		temp = getthresh( harmony, N, threshfac );
		if(temp > synt) synt = temp ; 
	    }else{
		temp = getthresh( harmony, N, threshfac );
	    }


	    if(sourceflag){
		// DO BOTH
		noscbank2(source_channel_out, N2, R, Nw, I, P,  output,  harmony, N2 );
	    }else{
		// ONLY DO INHARMONATOR
		noscbank(harmony, N2, R, Nw, I, P, output);
	    }

	    shiftout( output, Nw, I, on+Nw-I, 0 ) ;

	    frame_count++ ; 
    // FRAMES LOOP END

    }


    // FLUSH OUT AND CLOSE OUTPUT FILE
    shiftout( output, Nw, I, 1, 1 ) ;

    
// CHANNELS LOOP END
} 

    // CLOSE  INPUT FILE
    if(ifd)fclose(ifd);  











    fprintf(stderr,"\n\nINHARMONATOR : RESYNTHESIS COMPLETED\n");

 
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;

    if( NON_target_harmadd.n != 1. ) fclose(NON_target_harmadd.fp ) ;
    if( NON_target_ptrans.n != 1. ) fclose(NON_target_ptrans.fp ) ;
    if( NON_target_dB.n != 1. ) fclose(NON_target_dB.fp ) ;

    if( target_harmadd.n != 1. ) fclose(NON_target_harmadd.fp ) ;
    if( target_ptrans.n != 1. ) fclose(target_ptrans.fp ) ;
    if( target_dB.n != 1. ) fclose(target_dB.fp ) ;
    if( target_FreqInterpControl.n != 1. ) fclose(target_FreqInterpControl.fp ) ;
    if( Fund_Freq_or_OctavePointPitchClass.n != 1. ) fclose(Fund_Freq_or_OctavePointPitchClass.fp ) ;

  if( INHARM_release.n != 1. ) fclose(INHARM_release.fp ) ;
  if( INHARM_attack.n != 1. ) fclose(INHARM_attack.fp ) ;
  if( INHARM_freqsmooth.n != 1. ) fclose(INHARM_freqsmooth.fp ) ;
  if( SOURCE_harmadd.n != 1. ) fclose(SOURCE_harmadd.fp ) ;
  if( SOURCE_ptrans.n != 1. ) fclose(SOURCE_ptrans.fp ) ;
  if( SOURCE_dB.n != 1. ) fclose(SOURCE_dB.fp ) ;
  if( SOURCE_release.n != 1. ) fclose(SOURCE_release.fp ) ;
  if( SOURCE_attack.n != 1. ) fclose(SOURCE_attack.fp ) ;
  if( warpshape.n != 1. ) fclose(warpshape.fp ) ;


    
    fclose( data ) ; 
 


    exit(EXIT_SUCCESS) ;
}





void usage()
{
    fprintf(stderr, "%s",
	"inharmonator:  harmonic partials remapper\n"
	"inharmonator   [flags] [input file] [output file]\n"
	"	    	Most formats accepted. Output format copied from input file.\n"
	"	    	(Values in brackets denote defaults.)\n"
	"	N:	"FFT_LENGTH 		// N
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec
	"	I:	"TIME_FACTOR

	"	A:	master gain in decibels (func) [0.] \n"

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout

	"	    SOURCE PARAMETERS:\n"
	"	Q:	"FREQ_SHIFT

	"	u:	"PITCH_TRANS		// SOURCE_ptrans


	"	J: SOURCE delay time (func) [0]\n"


	"	r:	gain in decibels (func) [0.] \n"

	"	l:	"AMP_ATT_TIME		// SOURCE_attack
	"	L:	"AMP_RELEASE_TIME		// SOURCE_release


	"	    INHARMONATOR PARAMETERS:\n"
	"	f:	fundamental frequency of remapping scheme  (func) [60.] \n\n"
	"	v:      envelope attack time  (func) [0.]\n"
	"	V:      envelope release time   (func) [0.]\n"
	"	Y:      frequency change response time  (func) [0.]\n\n"
	"	a:	NON TARGETS: frequency shift factor \n"
	"		    (bin frequency adder, before -P )(func)[0.] \n"
	"	P:	NON TARGETS: pitch transposition in semitones (func)[0]\n"
	"	G:	NON TARGETS: gain control in decibels (func) [0.] \n\n"

	"	E: NON TARGETS decay time (func) [0]\n"
	"	K: NON TARGETS time delay randomization  0 = off 0, 1 = on [0]\n"
	"	j: NON TARGETS delay time (func) [0]\n"
	"	n: NON TARGETS time delay response time in seconds (func) [0]\n"


	"	q:	TARGETS: frequency shift factor \n"
	"		    (bin frequency adder, before -P )(func)[0.] \n"
	"	X:	TARGETS: pitch transposition in semitones (func)[0]\n"
	"	m:	TARGETS: gain control in decibels (func) [0.] \n"
	"	S:	TARGETS: proportion of data file frequency shift (0-1)(func) [1.] \n"
	"		    (0 = no shift,  1 = full shift)\n"
	"	W:	TARGETS: spectrum warp index for reshaping magnitude response (func) [0.] \n"
	"		    Values > 0 expand the dynamic range, \n"
	"		    values < 0 compress the dynamic range. \n"

	"	H: TARGETS time delay randomization  0 = off 0, 1 = on [0]\n"
	"	c: TARGETS time delay response time in seconds (func) [0]\n"
	"	g: TARGETS decay time scaler (func) [0]\n"
	"	k: TARGETS decay time shifter (func) [0]\n"	"	x: TARGETS partial bandwidth proportion (func) [0]\n"


	"	B:	dB level at boundaries of partial band [-96.]\n"
	"		    (relative to partial dB)\n"
	"		    (boundaries lie 1/2way between partials) \n\n"
	"	F:	(ascii) DATA FILE of unordered, selected partial triples:\n"
	"		     Each consisting of: \n"
	"			partial number, shift(see below), decibels, and delay time\n"
	"	Z:	SHIFT DATA FORMAT: [0]\n"
	"		     0 = transposition multiplier\n"
	"		     1 = frequency shift point\n"
	"		     2 = octave.pitch_class shift point\n"
	"		     3 = partial number of fundamental shift point\n\n"

	"	     INPUT DATA MACRO MODIFIERS:\n"
	"	z:	data partial number scaler, scaled relative to 1 and above [1]\n"
	"		  i.e. partial 1 remains constant. [1, 2, 3] becomes [1, 3, 5] with scaler of 2\n"
	"	R:	data partial number shifter [0]\n"
	"	y:	data decibels scaler [1]\n"
	"	o:	data delay time scaler [1]\n"
	"	O:	data delay time shifter [0]\n"

	"	U:	TARGETS: amplitude interpolation control (0-1): 0 = original, 1 = modified (func) [1] \n"
	"	S:	TARGETS: frequency interpolation control (0-1): 0 = original, 1 = modified (func) [1] \n"
	"	T:	TARGETS: time interpolation control (0-1): 0 = original, 1 = modified (func) [1] \n"


	"	_:	 "AUTO_PLAY

	"	=:	 "RESCALE_LEVEL

	"	t:	"RESYNTH_THRESHOLD		// threshfacdB
	"	p:	"AMP_REPORTS		// quiet 
	"	i:	"AMP_REPORTS_TIME_INTERVAL	// ampstatinc 

	); // DONE
    exit(EXIT_SUCCESS);
}





void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

