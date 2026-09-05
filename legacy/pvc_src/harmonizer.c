#include "globals.h"
#define NUM_PARAMETERS 8

#define FREQ_MULTIPLIER_OR_ADDER PP[j + 0]
#define LOW_FREQ_BOUNDARY PP[j + 1]
#define HIGH_FREQ_BOUNDARY PP[j + 2]
#define CENTER_FREQ PP[j + 3]
#define HARMONIZER_PEAK_DB_LEVEL PP[j + 4]
#define HARMONIZER_STOPBAND_DB_LEVEL PP[j + 5]
#define HARMONIZER_Q_INDEX PP[j + 6]
#define HARMONIZER_DELAY_TIME PP[j + 7]

// #define HARMONIZER_DECAY_TIME PP[j + 8]

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k, l,  i1,  i2, ipartial,  bug=0, exflag=0, ampIndex, freqIndex, bin, 
     mm,  n,  NC, NCmult2, NCdiv2,   numberOfTargetBands,   *harmonizer_channel_bins  ;
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag=0,   channelout=0;
float P = 1.0;
FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, 
    *buffer, *channel, *channel_source_out, *HARMONIZER_channel_delay, HARMONIZER_maxDelayT=0.,  
	*HARMONIZER_DATA_delayT, *delayT_NOW, HARMONIZER_maxDecayT=0., 
	*SOURCE_channel_delay, SOURCE_maxDelayT=0., SOURCE_maxDecayT=0.,
		*HARMONIZER_DATA_decayT, *decayT_NOW, *output ;
int *delayT_NOW_inFrames, *decayT_NOW_inFrames, *bandDataLine  ; 
float *previous_channel,  *F,  *FT,  *SUM,  *harmony, *harmony_delayed_inputs, *harmony_delay_now,  
	*bufferSum, *buffer_harmony_delayed_inputs, *ampfreq, *HARMONIZER_DATA_amp, *HARMONIZER_DATA_freq, pmt,  
		*pmt_VALUES, pms  ; 
float threshfac = .001,  threshfacdB=-96. ;
float	random_seed, gain=1. ;
float lowfreq,  hifreq, centerfreq, dur ; 
float  temp,  temp1, temp2,  temp3,  temp4,  temp5,  temp6 ;  
float getthresh();
float funcMin, funcMax, funcAvg ; 
float   IR ;
int HARMONIZER_maxNumOfDelayFrames, HARMONIZER_frameNowChannelDelayIndex, 
	SOURCE_maxNumOfDelayFrames, SOURCE_frameNowChannelDelayIndex, 
		this_HARMONIZER_FrameDelay, this_SOURCE_FrameDelay ; 

float *delayTimes, *timeRateScalers ; 
int band ; 

float lowFreqChannel, highFreqChannel, centerFreqChannel, channelNow ; 

int sourceflag=1 ; 
float freqMultiplierOrAdder ; 
FILE *data ; 
char datafile[ STRING_SIZE ] = "",  new_datafile[ STRING_SIZE ] ; 
float *PP,  fundamental,  target_gain, *target_gain_VALUES,  SOURCE_gain,  midC ; 
int shift_format=0,   nd=0 ; 
char tempstring[ STRING_SIZE ] ; 

float data_shift_factor_scaler, data_shift_factor_shifter, data_frequency_scaler, data_frequency_shifter, 
		data_peak_dB_scaler, data_stopband_dB_scaler, data_stopband_dB_shifter, 
	    data_time_delay_scaler, data_time_delay_shifter, data_Q_index_shifter,
	data_decay_time_scaler, data_decay_time_shifter ; 

// AMP INTERPOLATION CONTROL
struct func target_AmpInterpControl ; float *target_AmpInterpControl_VALUES ; // OK

// FREQ INTERPOLATION CONTROL
struct func target_FreqInterpControl ; float *target_FreqInterpControl_VALUES ; // OK

// TIME INTERPOLATION CONTROL
struct func target_TimeInterpControl ; float *target_TimeInterpControl_VALUES ; // OK


// GAIN
struct  func  dBgain ; // OK

// SOURCE  FREQUENCY SHIFT ADDER
struct  func  SOURCE_harmadd ; // OK

// SOURCE  PITCH TRANSPOSITION IN SEMITONES
struct  func  SOURCE_ptrans ; // OK

// SOURCE DECIBELS
struct  func  SOURCE_dB ; // OK

// SOURCE DELAY TIME
struct func SOURCE_delayT ; // OK

// TARGET DECIBELS
struct  func  target_dB ; float *target_dB_VALUES ; // OK

// TARGET  FREQUENCY SHIFT ADDER
struct  func  target_harmadd ; float *target_harmadd_VALUES ; // OK

// TARGET  PITCH TRANSPOSITION IN SEMITONES
struct  func  target_ptrans ; float *target_ptrans_VALUES ; // OK



// TARGET SPECTRUM WARPSHAPE INDEX
struct  func  target_warpshape ; // NOT RESPONSIVE TO DIFFERENT DELAYS



// *****************INITIALIZE




// AMP INTERPOLATION CONTROL
target_AmpInterpControl.L = 1. ; target_AmpInterpControl.n = 1. ; target_AmpInterpControl.A[ 0 ] = 1. ; 

// FREQ INTERPOLATION CONTROL
target_FreqInterpControl.L = 1. ; target_FreqInterpControl.n = 1. ; target_FreqInterpControl.A[ 0 ] = 1. ; 

// TIME INTERPOLATION CONTROL
target_TimeInterpControl.L = 1. ; target_TimeInterpControl.n = 1. ; target_TimeInterpControl.A[ 0 ] = 1. ; 


// SOURCE  FREQUENCY SHIFT ADDER
SOURCE_harmadd.L = 1. ; SOURCE_harmadd.n = 1. ; SOURCE_harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// SOURCE  PITCH TRANSPOSITION IN SEMITONES
SOURCE_ptrans.L = 1. ; SOURCE_ptrans.n = 1. ; SOURCE_ptrans.A[ 0 ] = 1. ; 

// TARGET DECIBELS
target_dB.L = 1. ; target_dB.n = 1. ; target_dB.A[ 0 ] = 0. ; 

// SOURCE DECIBELS
SOURCE_dB.L = 1. ; SOURCE_dB.n = 1. ; SOURCE_dB.A[ 0 ] = 0. ; 


// SOURCE DELAY TIME
SOURCE_delayT.L = 1. ; SOURCE_delayT.n = 1. ; SOURCE_delayT.A[ 0 ] = 0. ; 




// TARGET  PITCH TRANSPOSITION IN SEMITONES
target_ptrans.L = 1. ; target_ptrans.n = 1. ; target_ptrans.A[ 0 ] = 1. ; 

// TARGET  FREQUENCY SHIFT ADDER
target_harmadd.L = 1. ; target_harmadd.n = 1. ; target_harmadd.A[ 0 ] = 0. ; 

// TARGET SPECTRUM WARPSHAPE INDEX
target_warpshape.L = 1. ; target_warpshape.n = 1. ; target_warpshape.A[ 0 ] = 0. ; 



if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, 
	"_|=|@|a|A|b|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|W|x|X|y|Y|z|Z|", 
    0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {





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

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;

	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;

	    case 'W':   strcpy(tempstring, arg_option);
			target_warpshape.fp = crackstring( tempstring, 
			    &target_warpshape );
			break;




// SOURCE
	    case 'a':   strcpy(tempstring, arg_option);
			SOURCE_harmadd.fp = crackstring( tempstring, 
			    &SOURCE_harmadd );
			break;
	    case 'P':   strcpy(tempstring, arg_option);
			SOURCE_ptrans.fp = crackstring( tempstring, &SOURCE_ptrans ); 
			break;
	    case 'G':   strcpy(tempstring, arg_option);
			SOURCE_dB.fp = crackstring( tempstring, 
			    &SOURCE_dB );
			break;

	    case 'g':   strcpy(tempstring, arg_option);
			SOURCE_delayT.fp = crackstring( tempstring, 
			    & SOURCE_delayT );
			break;

// TARGETS
	    case 'Y':   data_shift_factor_scaler = crackfloat( arg_option, ch ) ;
			break;
	    case '@':   data_shift_factor_shifter = crackfloat( arg_option, ch ) ;
			break;



	    case 'U':   data_frequency_scaler = crackfloat( arg_option, ch ) ;
			break;

	    case 'y':   data_frequency_shifter = crackfloat( arg_option, ch ) ;
			break;




	    case 'o':   data_peak_dB_scaler = crackfloat( arg_option, ch ) ;
			break;

	    case 'O':   data_stopband_dB_scaler = crackfloat( arg_option, ch ) ;
			break;

	    case 'Q':   data_stopband_dB_shifter = crackfloat( arg_option, ch ) ;
			break;



	    case 'j':   data_time_delay_scaler = crackfloat( arg_option, ch ) ;
			break;
	    case 'n':   data_time_delay_shifter = crackfloat( arg_option, ch ) ;
			break;

	    case 'r':   data_Q_index_shifter = crackfloat( arg_option, ch ) ;
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


	    case 'J':   strcpy(tempstring, arg_option);
			target_AmpInterpControl.fp = crackstring( tempstring, 
			    &target_AmpInterpControl );
			break;
	    case 'K':   strcpy(tempstring, arg_option);
			target_FreqInterpControl.fp = crackstring( tempstring, 
			    &target_FreqInterpControl );
			break;
	    case 'Z':   strcpy(tempstring, arg_option);
			target_TimeInterpControl.fp = crackstring( tempstring, 
			    & target_TimeInterpControl );
			break;



           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;



	    case 'z':   shift_format = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'F':   strcpy(datafile, arg_option);
			break;

	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	}
    }
 

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "HARMONIZER", 69 ) ; 
prline( 69,  "-" ) ; 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }

   // DO THIS IN ORDER TO GET SAMPLE RATE AND, FROM IT, FUNDAMENTAL, AND THEN RINGTIME BEFORE SETUP OF FILES. 
   getInputFileDataToSetOutputChannels(argc, argv); 
    R = isr ; // SAMPLE RATE EQUALS INPUT FILE
    nyquist = R/2.0;
    fundamental = (float) R / (float) N ; 

// ********

// ***************** 

if( strcmp( datafile, "") == 0  ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A DATA FILE. BYE.\n" ) ;
    exit(EXIT_FAILURE);
}

// ****** TURN SOURCE ON OR OFF
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
// ***************** 

// **************************GET DATA	
// READ IN LEVELS


// MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
    cut_data_lines( datafile,  new_datafile,  NUM_PARAMETERS ) ; 


// OPEN FILE
  	if( (data = fopen( new_datafile, "r")) == NULL ){
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  datafile ) ; 
	    exit(EXIT_FAILURE); 
	}





// COUNT VALUES IN FILE
	k = 0 ; 
	while( fscanf( data,  " %f ",  &temp ) != EOF ) k++ ; 			
	rewind( data ) ;    
// ALLOCATE SPACE FOR LEVELS
    fvec( PP, k ) ;	// LEVELS
// COMPUTE NUMBER OF LEVELS
    numberOfTargetBands = k / NUM_PARAMETERS ;

prline( 69,  "-" ) ;
prbanner( "HARMONIZER: DATAFILE VALUES -- AS SPECIFIED", 69 ) ; 
prline( (NUM_PARAMETERS * 13),  "-" ) ;
prline( (NUM_PARAMETERS * 13),  "*" ) ;
    if( shift_format == 0 ){
	fprintf( stderr,  "\nFrequency   |Low Boundary|Hi Boundary |Center      |Peak        |Boundary    |Q-Index     |Delay Time  |" ) ; 
	fprintf( stderr,  "\nMultiplier  |Frequency or Octave.Pitchclass        |Decibels    |Decibels    |            |            |" ) ; 
	fprintf( stderr,  "\n            |Low = -1: 0Hz, High = -1: Nyquist Freq|            |            |            |            |" ) ; 
    }else if( shift_format == 1 ){
	fprintf( stderr,  "\nFrequency   |Low Boundary|Hi Boundary |Center      |Peak        |Boundary    |Q-Index     |Delay Time  |" ) ; 
	fprintf( stderr,  "\nAdder       |Frequency or Octave.Pitchclass        |Decibels    |Decibels    |            |            |" ) ; 
	fprintf( stderr,  "\n            |Low = -1: 0Hz, High = -1: Nyquist Freq|            |            |            |            |" ) ; 
    }else if( shift_format == 2 ){
	fprintf( stderr,  "\nSemitones   |Low Boundary|Hi Boundary |Center      |Peak        |Boundary    |Q-Index     |Delay Time  |" ) ; 
	fprintf( stderr,  "\nof Trans-   |Frequency or Octave.Pitchclass        |Decibels    |Decibels    |            |            |" ) ; 
	fprintf( stderr,  "\nposition    |Low = -1: 0Hz, High = -1: Nyquist Freq|            |            |            |            |" ) ; 
    }else{
	fprintf( stderr,  "\nILLEGAL SHIFT METHOD FORMAT. BYE.\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
    }
prline( (NUM_PARAMETERS * 13),  "*" ) ;
    


// READ IN VALUES AND PRINT AS INPUT
k = 0 ; 
for( i = 0; i < numberOfTargetBands ; i++ ){
    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < NUM_PARAMETERS ; j++ ){
	fscanf( data,  " %f ",  &PP[ k ] ) ; 
	fprintf( stderr,  "%-12.3f|", PP[ k ]  ) ;
	k++ ; 			
    }
} 

prline( (NUM_PARAMETERS * 13),  "*" ) ;
    fprintf( stderr,  "\n\n" ) ; 


// ************* CONVERTED, SCALED AND SHIFTED.

prline( 69,  "-" ) ;
prbanner( "HARMONIZER: DATAFILE VALUES -- CONVERTED, SCALED AND SHIFTED", 69 ) ; 
prline( (NUM_PARAMETERS * 13),  "-" ) ;
prline( (NUM_PARAMETERS * 13),  "*" ) ;
    if( shift_format == 0 ){
	fprintf( stderr,  "\nFrequency   |Low Boundary|Hi Boundary |Center      |Peak        |Boundary    |Q-Index     |Delay Time  |" ) ; 
	fprintf( stderr,  "\nMultiplier  |Frequency                             |Decibels    |Decibels    |            |            |" ) ; 
	fprintf( stderr,  "\n            |Low = -1: 0Hz, High = -1: Nyquist Freq|            |            |            |            |" ) ; 
    }else if( shift_format == 1 ){
	fprintf( stderr,  "\nFrequency   |Low Boundary|Hi Boundary |Center      |Peak        |Boundary    |Q-Index     |Delay Time  |" ) ; 
	fprintf( stderr,  "\nAdder       |Frequency                             |Decibels    |Decibels    |            |            |" ) ; 
	fprintf( stderr,  "\n            |Low = -1: 0Hz, High = -1: Nyquist Freq|            |            |            |            |" ) ; 
    }else if( shift_format == 2 ){
	fprintf( stderr,  "\nSemitones   |Low Boundary|Hi Boundary |Center      |Peak        |Boundary    |Q-Index     |Delay Time  |" ) ; 
	fprintf( stderr,  "\nof Trans-   |Frequency                             |Decibels    |Decibels    |            |            |" ) ; 
	fprintf( stderr,  "\nposition    |Low = -1: 0Hz, High = -1: Nyquist Freq|            |            |            |            |" ) ; 
    }else{
	fprintf( stderr,  "\nILLEGAL SHIFT METHOD FORMAT. BYE.\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
    }
prline( (NUM_PARAMETERS * 13),  "*" ) ;
    


// READ IN VALUES
k = 0 ; 
for( i = 0; i < numberOfTargetBands ; i++ ){
    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < NUM_PARAMETERS ; j++ ){
	// MODIFY	
	if( j == 0 ) PP[ k ] = (PP[ k ] * data_shift_factor_scaler) + data_shift_factor_shifter ; 
	if( (j == 1) || (j == 2) || (j == 3) ){
	    if( (PP[ k ] < 15) && (PP[ k ] > 0) ) PP[ k ] = OPPC_to_Hz( PP[ k ] ) ;  
	    if( (PP[ k ] < 0) && (j == 1) ) PP[ k ] = 0. ; 
	    if( (PP[ k ] < 0) && (j == 2) ) PP[ k ] = nyquist ; 
	    PP[ k ] = (data_frequency_scaler * PP[ k ]) + data_frequency_shifter ; 
	} ; 
	if( j == 4 ) PP[ k ] = data_peak_dB_scaler * PP[ k ] ; 
	if( j == 5 ) PP[ k ] = (data_stopband_dB_scaler * PP[ k ]) + data_stopband_dB_shifter ; 
	if( j == 6 ) PP[ k ] = PP[ k ] + data_Q_index_shifter ; 
	if( j == 7 ) PP[ k ] = (data_time_delay_scaler * PP[ k ]) + data_time_delay_shifter ; 
//	if( j == 8 ) PP[ k ] = (data_decay_time_scaler * PP[ k ]) + data_decay_time_shifter ; 
	fprintf( stderr,  "%-12.3f|", PP[ k ]  ) ;
	k++ ; 			
    }
} 

prline( (NUM_PARAMETERS * 13),  "*" ) ;
fprintf( stderr,  "\n\n" ) ; 

    


fvec( target_AmpInterpControl_VALUES, numberOfTargetBands ) ; 
fvec( target_FreqInterpControl_VALUES, numberOfTargetBands ) ; 
fvec( target_TimeInterpControl_VALUES, numberOfTargetBands ) ; 
fvec( target_harmadd_VALUES, numberOfTargetBands ) ; 
fvec( target_dB_VALUES, numberOfTargetBands ) ; 
fvec( target_gain_VALUES, numberOfTargetBands ) ; 
fvec( target_ptrans_VALUES, numberOfTargetBands ) ; 
fvec( pmt_VALUES, numberOfTargetBands ) ; 

fvec( delayTimes, numberOfTargetBands ) ; 
fvec( timeRateScalers,  numberOfTargetBands ) ; 

for( i = 0; i < numberOfTargetBands ; i++ ){
   delayTimes[i] = PP[ (i * NUM_PARAMETERS) + 7 ] ; 
   timeRateScalers[i] = 1. ; 
} ; 


// ************************************
// SET UP THE ARRAY POINTING AT THE BINS AND THEIR AMPLITUDES
    // FIRST TIME LOOP TO ACCESS ARRAY SIZE NEEDED
  for(l = 0 ; l < 2 ; l++ ){
    mm = 0 ; 
    // LOOP FOR  LEVELS

    for( j = 0, band = 0; j < (numberOfTargetBands * NUM_PARAMETERS) ; j += NUM_PARAMETERS, band++ ){

	// SHIFT VALUE
	if( shift_format == 0){
 	    // MOVE FREQUENCY MULTIPLIER INTO TEMP
	    freqMultiplierOrAdder = FREQ_MULTIPLIER_OR_ADDER ;
	}else if( shift_format == 1){
	    // MOVE FREQ ADDER INTO TEMP
	    freqMultiplierOrAdder = FREQ_MULTIPLIER_OR_ADDER ;
	    
	}else{
	    // MAKE FREQ MULTIPLIER FROM SEMITONES OF TRANSPOSITION
	    // AND PUT IN TEMP
	    freqMultiplierOrAdder = semitones_to_mult( FREQ_MULTIPLIER_OR_ADDER ) ;		
	}
	    
	// FIND THE LOW, HIGH, AND CENTER FREQUENCIES
	     
	if( (LOW_FREQ_BOUNDARY < 15) && (LOW_FREQ_BOUNDARY > 0.) ) lowfreq = OPPC_to_Hz( LOW_FREQ_BOUNDARY ) ; 
	else {
	    if( LOW_FREQ_BOUNDARY < 0.) lowfreq = 0; else lowfreq = LOW_FREQ_BOUNDARY ;
	} ; 

	if( (HIGH_FREQ_BOUNDARY < 15) && (HIGH_FREQ_BOUNDARY > 0.) ) hifreq = OPPC_to_Hz( HIGH_FREQ_BOUNDARY ) ; 
	else {
	    if( HIGH_FREQ_BOUNDARY < 0.) hifreq = nyquist;  else hifreq = HIGH_FREQ_BOUNDARY ; 
	} ; 

	if( (CENTER_FREQ < 15) && (CENTER_FREQ > 0.) ) centerfreq = OPPC_to_Hz( LOW_FREQ_BOUNDARY ) ; 
	else {
	    if( CENTER_FREQ < 0.) centerfreq = 0.;  else centerfreq = CENTER_FREQ ; 
	} ; 

// ***

	if( (centerfreq < lowfreq) || (centerfreq > hifreq) ){
		prt( " ****** ORDER PROBLEMS IN YOUR FREQUENCY DATA *******" ) ; 
		fprintf( stderr, "\n\nHARMONIZER INPUT BAND %d:\t%f %f %f\n", 
			j, LOW_FREQ_BOUNDARY, HIGH_FREQ_BOUNDARY, CENTER_FREQ ) ;  
		prf( hifreq, "HIGH FREQUENCY BOUNDARY"); 
		prf( centerfreq, "CENTER FREQUENCY"); 
		prf( lowfreq, "LOW FREQUENCY BOUNDARY"); 
		prt( "\n\n" ) ; 
		exit(EXIT_FAILURE) ; 
	} ; 

	// LOOP FOR WINDOW BAND
	lowFreqChannel =  floor( .5 + (lowfreq / fundamental) ) ; 
	highFreqChannel =  floor( .5 + (hifreq / fundamental) ) ; 
	centerFreqChannel = floor( .5 + (centerfreq / fundamental) ) ;  

	i1 = (2 * lowFreqChannel) + 1 ; 
	i2 = (2 * highFreqChannel) + 1 ;


	// LOOP FOR THE BINS
	for(i = i1, channelNow = lowFreqChannel; i <= i2; i += 2, channelNow += 1.0 ){

	    // IN THE NEXT (ipartial-i1)*2 SPACES IN THE FILTER ARRAY 
	    // PLACE THE APPROPRIATE AMP VALUE FOR THIS LEVEL
	    
	    // IF IN RANGE.....
	    if((i > 0) && (i < N) ){

	        // THE BIN INDEX
	        if( l == 1) harmonizer_channel_bins[ mm ] = (int) channelNow ; 
	        // THE AMPFREQ TRANSFORMATION VALUES
	        // AMP
	        if( l == 1) {
		    if( channelNow <= centerFreqChannel ) 
		        temp = 1.0 - ( (channelNow - lowFreqChannel) / ((centerFreqChannel - lowFreqChannel)) ) ; 
		    else
		        temp = ( (channelNow - centerFreqChannel) / ((highFreqChannel  - centerFreqChannel)) ) ; 

		    temp = curve( 0., 1., temp, -1. * HARMONIZER_Q_INDEX ) ; 

		    temp = HARMONIZER_PEAK_DB_LEVEL + (temp * HARMONIZER_STOPBAND_DB_LEVEL ) ; 

		    HARMONIZER_DATA_amp[ mm ] = (float)  dB_to_amp( temp ) ; 
	        } ; 
	        // FREQ
	        if( l == 1) HARMONIZER_DATA_freq[ mm ] = freqMultiplierOrAdder ; 

	        if( l == 1 ) {
		    HARMONIZER_DATA_delayT[ mm ] = HARMONIZER_DELAY_TIME ; 
		    if( HARMONIZER_DATA_delayT[ mm ] < 0.) HARMONIZER_DATA_delayT[ mm ] = 0. ;  
		    if( HARMONIZER_DATA_delayT[ mm ] > HARMONIZER_maxDelayT ) 
				HARMONIZER_maxDelayT = HARMONIZER_DATA_delayT[ mm ] ;  
	        } ; 

              if( l == 1 ){
                  bandDataLine[ mm ] = band ; 
              } ; 


//	        if( l == 1 ) {
//		    HARMONIZER_DATA_decayT[ mm ] = HARMONIZER_DECAY_TIME ; 
//		    if( HARMONIZER_DATA_decayT[ mm ] > HARMONIZER_maxDecayT ) 
//				HARMONIZER_maxDecayT = HARMONIZER_DATA_decayT[ mm ] ;  
//	        } ; 

		mm++ ; 


	    }


	}

      
// END LEVELS LOOP
     }

//fprintf( stderr,  "\nmm = %d",  mm ) ; 

    NC = mm ; NCmult2 = NC * 2 ;   
//    fprintf( stderr, "\nNC: %d,  NCmult2: %d", NC,   NCmult2 ) ;    


    
        // MAKE  ARRAYS
    if(l == 0){
//	fvec( ampfreq, NCmult2 ) ;

	fvec( HARMONIZER_DATA_amp, NC ) ;
	fvec( HARMONIZER_DATA_freq, NC ) ;
	ivec( harmonizer_channel_bins, NC ) ;
	fvec( HARMONIZER_DATA_delayT, NC ) ;     
//	fvec( HARMONIZER_DATA_decayT, NC ) ;     
	fvec( delayT_NOW, NC ) ;     
	ivec( delayT_NOW_inFrames, NC ) ;     
//	fvec( decayT_NOW, NC ) ;     
//	ivec( decayT_NOW_inFrames, NC ) ;     

     ivec( bandDataLine, NC ) ; 

    } ; 
}


ringTime = HARMONIZER_maxDelayT	; //  + HARMONIZER_maxDecayT ; 

if( sourceflag != 0 ){
    findFuncMinMaxAvg( &SOURCE_delayT, &funcMin, &SOURCE_maxDelayT, &funcAvg ); 
    if( SOURCE_maxDelayT > ringTime ) ringTime = SOURCE_maxDelayT ; 

//   prf( SOURCE_maxDelayT, "SOURCE_maxDelayT" ) ; 
}; 

//prf( ringTime, "ringTime" ) ; 

	// CHANNEL DELAYS
HARMONIZER_maxNumOfDelayFrames = 1 + (int)((HARMONIZER_maxDelayT * frames_per_sec) + 0.5) ; 
//pri( HARMONIZER_maxNumOfDelayFrames, "HARMONIZER_maxNumOfDelayFrames" ) ; 
fvec( HARMONIZER_channel_delay, HARMONIZER_maxNumOfDelayFrames * (NCmult2 + 2) ) ; 

if( sourceflag != 0 ){
   SOURCE_maxNumOfDelayFrames = 1 + (int)((SOURCE_maxDelayT * frames_per_sec) + 0.5) ; 
    fvec( SOURCE_channel_delay, SOURCE_maxNumOfDelayFrames * (N + 2) ) ; 
} ; 
    
	// MAKE  ARRAY
	fvec( harmony, (NCmult2 + 2) ) ;
	fvec( harmony_delayed_inputs, NCmult2 + 2 ) ; 
	fvec( harmony_delay_now, NCmult2 + 2 ) ; 

	fvec( buffer_harmony_delayed_inputs, NCmult2 ) ; 
	fvec( bufferSum, NCmult2 ) ;		// FFT buffer


    for( bin = 0 ; bin < NC ; bin++ ){
	    if(bug)fprintf( stderr,  "\n harmonizer_channel_bins: %d,  amp: %f,  freq: %f ",  
		harmonizer_channel_bins[ bin ],  HARMONIZER_DATA_amp[ bin ], HARMONIZER_DATA_freq[ bin ]  ) ;  
   }





// ********
 
// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
    setupfiles(argc, argv) ; 

    endchan = beginchan + ochan ; 
    
// **** SET UPS *****
//    R = isr ; // SAMPLE RATE EQUALS INPUT FILE
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
//    nyquist = R/2.0;
    IR = (float) I / (float) R ; 

    obank = P != 0. ;
    if( P == 0.0 ) {P = 1.0;}

    N2 = N>>1 ; // N / 2
    Nw2 = Nw>>1 ; // NW / 2
    midC = (220.*pow(2., (3./12.))) ; 
//    fundamental = (float) R / (float) N ; 

    // COMPUTE THE DURATION
    dur = (endt - begint) * (float) I / (float) D ; 





// ***************** PRINT VALUES
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
prp( &dBgain,  "MASTER GAIN (in dB)"  ) ; 
prline( 1,  "*" ) ; 
prp( &target_dB,	"TARGETS: GAIN (in dB)"  ) ; 
prp( &target_ptrans,  "TARGETS: PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &target_harmadd,  "TARGETS: FREQUENCY SHIFT (in Hz)"  ) ; 
prp( & target_warpshape, "TARGETS SPECTRUM WARPSHAPE INDEX" ) ; 
prp( &target_AmpInterpControl, "AMPLITUDE INTERPOLATION CONTROL" ) ; 
prp( &target_FreqInterpControl, "FREQUENCY INTERPOLATION CONTROL" ) ; 
prp( &target_TimeInterpControl, "TIME INTERPOLATION CONTROL" ) ; 
prline( 1,  "*" ) ; 
if( sourceflag != 0 ){
    prp( &SOURCE_dB,	"SOURCE: GAIN (in dB)"  ) ; 
    prp( &SOURCE_ptrans,  "SOURCE: PITCH TRANSPOSITION (in semitones)"  ) ; 
    prp( &SOURCE_harmadd,  "SOURCE: FREQUENCY SHIFT (in Hz)"  ) ; 
    prp( &SOURCE_delayT, "SOURCE: DELAY TIME" ) ; 
} ; 



prf( data_shift_factor_scaler, "DATA: SHIFT FACTOR SCALER" ) ; 
prf( data_shift_factor_shifter, "DATA: SHIFT FACTOR SHIFTER" ) ; 
prf( data_frequency_scaler, "DATA: FREQUENCY SCALER" ) ; 
prf( data_frequency_shifter, "DATA: FREQUENCY SHIFTER" ) ; 
prf( data_peak_dB_scaler, "DATA: PEAK DB SCALER" ) ; 
prf( data_stopband_dB_scaler, "DATA: STOPBAND DB SCALER" ) ; 
prf( data_stopband_dB_shifter, "DATA: STOPBAND DB SHIFTER" ) ; 
prf( data_time_delay_scaler, "DATA: TIME DELAY SCALER" ) ; 
prf( data_time_delay_shifter, "DATA: TIME DELAY SHIFTER" ) ; 
prf( data_Q_index_shifter, "DATA: Q INDEX SHIFTER" ) ; 

prf( rescalev, "DECIBEL RESCALE VALUE" ) ; 

prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 


    

// ****** MAKE ARRAYS
    fvec( Wanal, Nw ) ;		// analysis window
    fvec( Wsyn, Nw ) ;		// synthesis window 
    fvec( input, Nw ) ;		// input buffer 
    fvec( Hwin, Nw ) ;		// plain Hamming window 
    fvec( winput, Nw ) ;	// windowed input buffer 
    fvec( buffer, N ) ;		// FFT buffer 
    fvec( channel, (N+2) ) ;	// analysis channels 
    if( sourceflag == 1) fvec( channel_source_out, (N + 2) ) ; 
//pri( Nw, "BEFORE fvec o output: Nw" ) ; 
    fvec( output, Nw ) ;	// output buffer 
// ***************** 


// MAKE THRESH AMP
    threshfac = dB_to_amp( threshfacdB );	


// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 




// ************************************
// *********************************************
// LOOP FOR CHANNELS
// *********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

    // *****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; 
//pd(0); 
    for( i = 0; i < (HARMONIZER_maxNumOfDelayFrames * (NCmult2 + 2)); i++ ) HARMONIZER_channel_delay[i] = 0.  ; 

    if( sourceflag != 0 ) 
	for( i = 0; i < (SOURCE_maxNumOfDelayFrames * (N + 2)); i++ ) SOURCE_channel_delay[i] = 0.  ; 

//pd(1); 

   makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;
//pd(2); 
 	
// *********************************************
// LOOP FOR FRAMES
// *********************************************

    while ( !eof ) {
	in += D ;
	on += I ;
	timenow( dur ) ;
//pd(3); 
	eof = shiftin( input, Nw, D ) ;
	fold( input, Wanal, Nw, buffer, N, in ) ;
	rfft( buffer, N2, FORWARD ) ;
	convert( buffer, channel, N2, D, R ) ;

//pd(4); 
	// TRANSFER channel INTO HARMONIZER CIRCULAR DELAY LINE FOR ALL BINS
	HARMONIZER_frameNowChannelDelayIndex = frame_count ; 
	while( HARMONIZER_frameNowChannelDelayIndex >= HARMONIZER_maxNumOfDelayFrames ) 
			HARMONIZER_frameNowChannelDelayIndex -=  HARMONIZER_maxNumOfDelayFrames ; 
//pd(5); 	
	for( bin = 0, ampIndex = 0, freqIndex = 1; bin < NC ; bin++, ampIndex += 2, freqIndex += 2 ){
//pd(51);
	    HARMONIZER_channel_delay[ (HARMONIZER_frameNowChannelDelayIndex * (NCmult2 + 2)) + ampIndex ] = 
			channel[ 2 * harmonizer_channel_bins[bin] ] ; 
//pd(52);
	    HARMONIZER_channel_delay[ (HARMONIZER_frameNowChannelDelayIndex * (NCmult2 + 2)) + freqIndex ] = 
			channel[ (2 * harmonizer_channel_bins[bin]) + 1 ] ; 

	} ; 
	// TRANSFER channel INTO SOURCE CIRCULAR DELAY LINE
	if( sourceflag != 0 ){
	    SOURCE_frameNowChannelDelayIndex = frame_count ; 
	    while( SOURCE_frameNowChannelDelayIndex >= SOURCE_maxNumOfDelayFrames ) 
	    		SOURCE_frameNowChannelDelayIndex -=  SOURCE_maxNumOfDelayFrames ; 
	
	    for( i = 0; i < (N + 2) ; i++ ) 
	        SOURCE_channel_delay[ (SOURCE_frameNowChannelDelayIndex * (N + 2)) + i] = channel[i] ; 
	} ; 

// *************************
// GET THE VALUES
// *************************

//  SHIFT, GAIN, AND MULTIPLIER

	dBgain.A[ 0 ] =  fval( &dBgain, dur, t );
	    gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
// TARGETS
      getGlobalFunctionValues(
            target_harmadd_VALUES, numberOfTargetBands, dur, delayTimes, timeRateScalers, &target_harmadd 
      ) ; 


	target_ptrans.A[ 0 ] = fval( &target_ptrans, dur, t );

      getGlobalFunctionValues(
            target_ptrans_VALUES, numberOfTargetBands, dur, delayTimes, timeRateScalers, &target_ptrans 
      ) ; 
      for( i = 0; i < numberOfTargetBands; i++ ) 
		pmt_VALUES[ i ] = semitones_to_mult( target_ptrans_VALUES[ i ] ) ;

      getGlobalFunctionValues(
            target_dB_VALUES, numberOfTargetBands, dur, delayTimes, timeRateScalers, &target_dB 
      ) ; 
      for( i = 0; i < numberOfTargetBands; i++ ) 
		target_gain_VALUES[ i ] = dB_to_amp( target_dB_VALUES[ i ] ) ; 

	target_warpshape.A[ 0 ] =  fval( &target_warpshape, dur, t );


      getGlobalFunctionValues(
            target_FreqInterpControl_VALUES, numberOfTargetBands, dur, delayTimes, timeRateScalers, &target_FreqInterpControl 
      ) ; 

      getGlobalFunctionValues(
            target_TimeInterpControl_VALUES, numberOfTargetBands, dur, delayTimes, timeRateScalers, &target_TimeInterpControl 
      ) ; 

//pd(6); 

// **************** SOURCE

	if(sourceflag){

	    SOURCE_delayT.A[ 0 ] =  fval( &SOURCE_delayT, dur, t );
	    SOURCE_harmadd.A[ 0 ] =  fval( &SOURCE_harmadd, dur, t - SOURCE_delayT.A[ 0 ] );
	    SOURCE_ptrans.A[ 0 ] = fval( &SOURCE_ptrans, dur, t - SOURCE_delayT.A[ 0 ] );
		pms = semitones_to_mult( SOURCE_ptrans.A[ 0 ] ) ;
	    SOURCE_dB.A[ 0 ] =  fval( &SOURCE_dB, dur, t - SOURCE_delayT.A[ 0 ] );
		SOURCE_gain = dB_to_amp( SOURCE_dB.A[ 0 ] ) ; 


	    // TRANSFER DELAY INTO CHANNEL_OUT

	    this_SOURCE_FrameDelay = 
		SOURCE_frameNowChannelDelayIndex - (int)((SOURCE_delayT.A[ 0 ] * frames_per_sec) + 0.5) ; 
	    while( this_SOURCE_FrameDelay < 0) this_SOURCE_FrameDelay += SOURCE_maxNumOfDelayFrames ; 


	    for( i = 0; i < (N + 2); i++ )channel_source_out[ i ] = 
			SOURCE_channel_delay[ (this_SOURCE_FrameDelay * (N + 2)) + i ] ; 

	    for( i = 1; i < (N + 2); i = i + 2){
	        // MODIFY SOURCE
		
	        // **FREQ
	        temp = pms * (channel_source_out[i] + SOURCE_harmadd.A[ 0 ]) ;
	        // **AMP
	        channel_source_out[i - 1] = channel_source_out[i - 1] * SOURCE_gain * gain ; 

	        // ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
	        if((temp <= 0.) || (temp >= nyquist)) channel_source_out[i - 1] = 0. ; 
	        else channel_source_out[i] = temp ; 
	    }
	} ; 


// ***********************************
//pd(7); 

// MAKE TRANSFER TO HARMONY ARRAY
	for( bin = 0, ampIndex = 0, freqIndex = 1 ; bin < NC ; bin++, ampIndex += 2, freqIndex += 2 ){

	    // FIND DELAY INDEX ADDER
	    delayT_NOW[ bin ] = HARMONIZER_DATA_delayT[ bin ] * target_TimeInterpControl_VALUES[ bandDataLine[ bin ] ] ; 
	    delayT_NOW_inFrames[ bin ] = (int)(( (delayT_NOW[ bin ]) * frames_per_sec ) + 0.5 ) ; 
	    this_HARMONIZER_FrameDelay =  HARMONIZER_frameNowChannelDelayIndex - delayT_NOW_inFrames[ bin ] ; 
	    while( this_HARMONIZER_FrameDelay < 0) this_HARMONIZER_FrameDelay += HARMONIZER_maxNumOfDelayFrames ; 


//	    // DECAY TIME
//	    decayT_NOW[ bin ] = HARMONIZER_DATA_decayT[ bin ] ; 
//	    decayT_NOW_inFrames[ bin ] = (int)(( (decayT_NOW[ bin ]) * frames_per_sec ) + 0.5 ) ; ; 

	    // AMP
	    harmony[ ampIndex ] = harmony_delayed_inputs[ ampIndex ] = 
		HARMONIZER_channel_delay[ (this_HARMONIZER_FrameDelay * (NCmult2 + 2) ) + ampIndex ]  
					* gain * target_gain_VALUES[ bandDataLine[ bin ] ] ; 
	    if( harmony[ ampIndex ] > dB_to_amp( -600. ) ){
	    	temp2 = amp_to_dB( harmony[ ampIndex ] ) ; 
		temp5 = harmony[ ampIndex ] * HARMONIZER_DATA_amp[ bin ] ; 
		if( temp5 > dB_to_amp( -600. ) ){
	 	    temp3 = amp_to_dB( temp5 ) ; 
		    temp4 = temp2 + (target_AmpInterpControl_VALUES[ bandDataLine[ bin ] ] * (temp3 - temp2)) ; 	
	    	    harmony[ ampIndex ] = dB_to_amp( temp4 ) ;
		} ; 
	    } ; 	    

	    // FREQ
	    harmony[ freqIndex ] = harmony_delayed_inputs[ freqIndex ] = 
				HARMONIZER_channel_delay[ (this_HARMONIZER_FrameDelay * (NCmult2 + 2)) + freqIndex ] ; 
	    if( shift_format == 1 ){
		// FREQ ADDER
		harmony[ freqIndex ] = ( (harmony[ freqIndex ]  + 
			    (target_FreqInterpControl_VALUES[ bandDataLine[ bin ] ] * HARMONIZER_DATA_freq[ bin ]) )
				+  target_harmadd_VALUES[ bandDataLine[ bin ] ] ) * pmt_VALUES[ bandDataLine[ bin ] ] ;

	    }else{
		    // FREQ MULTIPLIER
		temp = 1. + (target_FreqInterpControl.A[ 0 ] * (HARMONIZER_DATA_freq[ bin ] - 1.)); 
		harmony[ freqIndex ] = 
			( (harmony[ freqIndex ]  * temp) +  target_harmadd_VALUES[ bandDataLine[ bin ] ] ) * pmt_VALUES[ bandDataLine[ bin ] ] ;
	    }
	} ; 

	// END HARMONY




		    // harmony OUT OF BOUNDS?
	for( i = 1; i < (NCmult2 + 2); i += 2 ){
 		    if((harmony[ i ] <= 0.) || (harmony[ i ] >= nyquist)) harmony[i - 1] = 0. ; 
	} ; 


// ******
    // *************WARP THE TARGETS SPECTRUM
	spectmagwarp( harmony,  (NCmult2 + 2), target_warpshape.A[ 0 ], 0 ) ;		

//pd(8); 

/*
	// FEED BACK DELAY INPUTS INTO HARMONIZER_channel_delay
	// FIRST ATTENUATE THE INPUTS BEFORE FEEDBACK
	// ATTENUATE DELAYED FILTER INPUTS AND ADD INTO DELAY ARRAY


	for(ampIndex = 0, bin = 0; bin < NC; ampIndex += 2, bin++){ 
	    if( decayT_NOW_inFrames[ bin ] <= 0. ){
		// ZERO DECAY TIME
		harmony_delayed_inputs[ ampIndex ] = 0. ; // ZERO AMP
	    }else{
		// NON ZERO DECAY TIME
		if(  delayT_NOW_inFrames[ bin ] <= 0. ){
		    // ZERO DELAY TIME
		    harmony_delayed_inputs[ ampIndex ] =  0. ; // ZERO AMP
		}else{
		    // ATTENUATE FEEDBACK AMP
		    harmony_delayed_inputs[ ampIndex ] *= 
			dB_to_amp(-60. / ( decayT_NOW_inFrames[ bin ] / delayT_NOW_inFrames[ bin ] )) ; 
		} ; 	
	    } ; 
	} ; 


	leanunconvert2_disarray( harmony_delayed_inputs, buffer_harmony_delayed_inputs, NC, I, R, harmonizer_channel_bins, fundamental ) ; 
	for(i = 0 ; i < (NCmult2 + 2); i++) harmony_delay_now[i] = 
		HARMONIZER_channel_delay[ (HARMONIZER_frameNowChannelDelayIndex * (NCmult2 + 2)) + i] ; 
	leanunconvert3_disarray( harmony_delay_now, bufferSum, NC, I, R, harmonizer_channel_bins, fundamental ) ;
	for(i = 0; i < NCmult2; i++ ) bufferSum[i] = bufferSum[i] + buffer_harmony_delayed_inputs[i] ; 	    
	leanconvert3_disarray( bufferSum, harmony_delay_now, NC, D, R, harmonizer_channel_bins, fundamental ) ;	 
	for( i = 0; i < (NCmult2 + 2) ; i++ ) 
		HARMONIZER_channel_delay[ (HARMONIZER_frameNowChannelDelayIndex * (NCmult2 + 2)) + i] = 
			harmony_delay_now[i] ; 
*/

	if(sourceflag){
	    synt = getthresh( channel_source_out, N, threshfac );
	    temp = getthresh( harmony, NCmult2, threshfac );
	    if(temp > synt) synt = temp ; 
	}else{
	    synt = getthresh( harmony, NCmult2, threshfac );
	} ; 

//pd(9); 
	if(sourceflag){
//pd(10); 		// DO BOTH
		if( !frame_count )prt( "RESYNTHESIZING BOTH SOURCE AND HARMONIZED PORTIONS....." ) ; 
		noscbank2( channel_source_out, N2, R, Nw, I, P, output, harmony,  NC );
	}else{
	    // ONLY HARMONIZER
		if( !frame_count )prt( "RESYNTHESIZING  HARMONIZED PORTIONS ONLY....." ) ; 
		noscbank(harmony, NC, R, Nw, I, P, output);
	}
	    // NOW OUTPUT
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






    fprintf(stderr,"\n\nHARMONIZER : RESYNTHESIS COMPLETED\n");







 
    if( dBgain.n != 1. ) fclose(dBgain.fp ) ;

    if( SOURCE_harmadd.n != 1. ) fclose(SOURCE_harmadd.fp ) ;
    if( SOURCE_ptrans.n != 1. ) fclose(SOURCE_ptrans.fp ) ;
    if( SOURCE_dB.n != 1. ) fclose(SOURCE_dB.fp ) ;

    if( target_harmadd.n != 1. ) fclose(target_harmadd.fp ) ;
    if( target_ptrans.n != 1. ) fclose(target_ptrans.fp ) ;
    if( target_dB.n != 1. ) fclose(target_dB.fp ) ;
    if( target_warpshape.n != 1. ) fclose(target_warpshape.fp ) ;

  
    fclose( data ) ; 
 


    exit(EXIT_SUCCESS) ;
}


void usage()
{
    fprintf(stderr, "%s",
	"harmonizer:  multiple level, phase vocoder harmonizer \n"
	"		    with selectable frequency-band window \n"
	"harmonizer   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	N:	"FFT_LENGTH 		// N
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec
	"	I:	"TIME_FACTOR		// tfactor

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout


	"	A:	"DB_GAIN			// dBgain


	"	    SOURCE\n"
	"	a:	source frequency shift factor \n"
	"		    (bin frequency adder, before -P )(func)[0.] \n"
	"	P:	source pitch transposition in semitones (func) [0]\n"
	"	G:	source gain control in decibels (func) [0.] \n"
	"	g:	source delay time in seconds (func)[0.]\n"

	"	    HARMONIZER\n"
	"	q:	harmonizer frequency shift factor \n"
	"		    (bin frequency adder, before -X )(func)[0.] \n"
	"	X:	harmonizer pitch transposition in semitones (func) [0]\n"
	"	m:	harmonizer gain control in decibels (func) [0.] \n"
	"	W:	harmonizer warp index for reshaping magnitude response (func) [0.] \n"
	"		    Values > 0 expand the dynamic range, \n"
	"		    values < 0 compress the dynamic range. \n"


	"	    DATA FILE\n"
	"	F:	ascii data file name:\n"
	"		  data line parameter fields: \n"
	"		    (1) shift factor (see -z for format) \n"
	"		    (2) window: low boundary (in Hz or octave.pitchclass) \n"
	"		    (3) window: high boundary   \n"
	"		    (4) decibels \n"
	"		    (5) peak decibels \n"
	"		    (6) stopband/boundary decibels \n"
	"		    (7) Q-index (0: linear, positive: sharper/thinner, negative: smoother/wider ) \n"
	"		    (8) time delay in seconds \n"
	"	z:	data file shift method format [0]\n"
	"		     0 = frequency multipler\n"
	"		     1 = frequency adder\n"
	"		     2 = semitones of transposition\n"

	"	     INPUT DATA MACRO MODIFIERS:\n"
	"	U:	data frequency scaler [1] \n"
	"	y:	data frequency shifter [0] \n"
	"	o:	data peak dB_scaler [1] \n"
	"	O:	data stopband dB scaler [1] \n"
	"	Q:	data stopband dB shifter [0] \n"
	"	r:	data Q index shifter [0] \n"
	"	j:	data time delay scaler [1] \n"
	"	n:	data time delay shifter [0] \n"
	"	@:	shift factor shifter [0] \n"
	"	Y:	shift factor scaler [1] \n"


	"	J:	harmonizer amplitude interpolation control (0-1): 0 = original, 1 = modified (func) [1] \n"
	"	K:	harmonizer frequency interpolation control (0-1): 0 = original, 1 = modified (func) [1] \n"
	"	Z:	harmonizer time interpolation control (0-1): 0 = original, 1 = modified (func) [1] \n"

	"	t:	"RESYNTH_THRESHOLD		// threshfacdB
	"	p:	"AMP_REPORTS		// quiet 
	"	i:	"AMP_REPORTS_TIME_INTERVAL	// ampstatinc 

	"	_:	 "AUTO_PLAY		// autoplayreps

	"	=:	 "RESCALE_LEVEL		// rescalev

	); // DONE
    exit(EXIT_SUCCESS);
}



void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

