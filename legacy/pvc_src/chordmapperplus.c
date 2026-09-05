#include "globals.h"

// THINGS TO ADD:

// DIFFERENT FILTERS: HP, LP, BR, COMB
// PRIME/NON-PRIME PARTIALS SELECT

#define NUMBER_OF_WRITE_NUMBERS 30

#define NUM_PARAMETERS 23

#define SOURCE_POINT PP[j+0]
#define VALUE_SOURCE_POINT_CASE_0 case 0:
#define PARAMETER_NAME_0 "Source Point" 

#define STRUCTURE_TRANSPOSE_POINT_CASE_1 case 1:


#define LOW_PARTIAL PP[j+2]
#define VALUE_LOW_PARTIAL_CASE_2 case 2:
#define PARAMETER_NAME_2 "Low Partial" 

#define PARTIAL_SPACING PP[j+3]
#define VALUE_PARTIAL_SPACING_CASE_3 case 3:
#define PARAMETER_NAME_3 "Partial Spacing" 

#define NUMBER_OF_PARTIALS PP[j+4]
#define VALUE_NUMBER_OF_PARTIALS_CASE_4 case 4:
#define PARAMETER_NAME_4 "Number of Partials" 

#define BANDWIDTH PP[j+5]
#define VALUE_BANDWIDTH_CASE_5 case 5:
#define PARAMETER_NAME_5 "Bandwidth" 

#define STRUCTURE_PARTIAL_SHIFT_CASE_6 case 6:

#define STRUCTURE_SPECTRAL_STRETCH_COMPRESS_CASE_7 case 7:

#define STRUCTURE_TONE_DECIBELS_CASE_8 case 8:


#define STRUCTURE_STASIS_MEDIAN_FOR_HARMONY_IN_DB_CASE_9 case 9:

#define STRUCTURE_NOISE_DB_CASE_10 case 10:

#define STRUCTURE_STASIS_MEDIAN_FOR_NOISE_IN_DB_CASE_11 case 11:

#define NOISE_SWITCH PP[j+12]
#define VALUE_NOISE_SWITCH_CASE_12 case 12:
#define PARAMETER_NAME_12 "Noise Switch" 

#define STRUCTURE_TRANSPOSE_POINT_NOISE_CASE_13 case 13:

#define STRUCTURE_FORCE_FACTOR_CASE_14 case 14:

#define MASTER_TRANSPOSITION_SWITCH PP[j+15]
#define VALUE_MASTER_TRANSPOSITION_SWITCH_CASE_15 case 15:
#define PARAMETER_NAME_15 "Master Transposition Switch" 

#define SYNTHETIC_VIBRATO_SWITCH PP[j+16]
#define VALUE_SYNTHETIC_VIBRATO_SWITCH_CASE_16 case 16:
#define PARAMETER_NAME_16 "Synthetic Vibrato Switch" 

#define STRUCTURE_BANDPASS_CF_CASE_17 case 17:

#define STRUCTURE_BANDPASS_ROLLOFF_IN_DB_PER_OCTAVE_CASE_18 case 18:

#define TONE_FILTER_SWITCH PP[j+19]
#define VALUE_TONE_FILTER_SWITCH_CASE_19 case 19:
#define PARAMETER_NAME_19 "Tone Filter Switch" 

#define DELAY_TIME_SWITCH_SCALER PP[j+20]
#define VALUE_DELAY_TIME_SWITCH_SCALER_CASE_20 case 20:
#define PARAMETER_NAME_20 "Delay Time Switch Scaler" 

#define STRUCTURE_DELAY_TIME_CASE_21 case 21:

#define TONE_CHANNEL_OUTPUT_NUMBER PP[j+22]
#define VALUE_TONE_CHANNEL_OUTPUT_NUMBER_CASE_22 case 22:
#define PARAMETER_NAME_22 "Tone Channel Output Number" 



#define NUMBER_OF_STATIC_FREQRESPONSE_AVERAGES 80

#define AMP_REDUCTION dB_to_amp( -7 )

void EXIT_failure() ; 

void findJumpPointGainScales(
	float lowMinimaReleaseVibratoPoint,
	float highMinimaReleaseVibratoPoint,
	float releaseVibratoPeriodJumpPoint,
    struct func *filter,
    float F_lower[], 
    float F_higher[],
    float tempChannel[],
    float iframes_per_sec,
    int analysis_Nplus2,
    int ainchan,
    int analysis_chan,
	float *lowMinimaReleaseVibratoPointGainScale_inDecibels,	
	float *highMinimaReleaseVibratoPointGainScale_inDecibels	

) ;


float normalizeLoopAmplitudesForChordmapperplus(
    int LoopNormalizationFlag,
    int Mode__sampler_loop_0__autostop_1, 
    struct func *filtwinlow, 
    struct func *filtwinhi,
    struct func *filter,
    float F_lower[], 
    float F_higher[],
    float tempChannel[],
    float channel[],
    float iframes_per_sec,
    int analysis_Nplus2,
    int ainchan,
    int analysis_chan,
    float analysis_dur, 
    float filttnow
) ;



float sumAndPrintDB(
    float array[],
    int size, 
    int startIndex,
    int incr,
    int printFlag
) ; 

float find_fundamental_frequency(
	struct func *analysis,
	struct func *pitchTrackFile, 
	int channelout,
	int N, 
	int D
) ; 


void makeAmplitudeEnvelopeFromSumOfAmps(
    struct func *analysis, 
    int analysis_Nplus2, 
    int ainchan, 
    int analysis_chan,
    float analysis_dur,
    float analysis_lower[], 
    float analysis_higher[],
    int normalizeFlag, 
    int iframes_per_sec,
    float ampEnvelope[],
    int niframes,
    float *peakSum 
) ; 


float smooth_frequency_change( 
    float A[], 
    float old_A[], 
    int Nplus2, 
    float att,
    float matt, 
    float rel, 
    float mrel
     
) ;


void makeStaticFreqResponseAveragesFromDataFile ( 
    struct func *analysis, 
    int analysis_Nplus2, 
    float analysis_fundamental,
    int ainchan, 
    int analysis_chan,
    float analysis_dur,
    float analysis_lower[], 
    float analysis_higher[],
    float static_freqresponse_averages[],
    int numOfStaticFreqresponseAverages,  

    float harmony_force_curve_indeces[],
    int partial_Band_Begin[], 
    int numberOfBands,
    int indexInChannelForHarmony[],

    float *static_freqresponse_peak_SUM_in_dB, 

    int iframes_per_sec,
    float midTimePoint,
    float timeWindowSize,
    int UseHammingWindowFlag,
    float HammingWindow[],
    int HammingWindowSize,

    float thisFreqresponseFrame[],
    float ampWeightSums[],
    float averageBandFreq[],

    int harmonyBands_IndexOfStrongestBinInBand[], 
    float harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand[], 
    int NC



) ; 

void getVibratoValuesAndIncrement(
    float vibValNow_TONE_VALUES[],
    int numTones,
    float synthetic_Vibrato_Rate_TONE_VALUES[],
    float synthetic_Vibrato_Randomization_Prop_TONE_VALUES[],
    int incrementFlag,

    float timeWarp[],
    float oldTimeWarp[],
    float ranAmpScale[],
    float oldRanAmpScale[],
    float rateMod[], 
    float oldRateMod[], 
    float vibPhaseNow[],
    float vibratoPeriodTable[],
    float vibratoPeriodTableSize
) ; 


void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{

float *peakBinAmpAverageForToneBands ; 
int bottomBinIndex, topBinIndex, *noiseBandLowerBinIndex, *noiseBandUpperBinIndex, flag, numberOfNoiseBands ; 

float lastGainScale_inDecibels=0., lastGainScaleFilterTimePoint, releaseVibratoPeriodJumpPoint ; 

int	numberOfSuppressedChannels ; 
float analysis_PeakAmpSave, analysis_PeakDecibelsSave ; 

float realMedianIndex, lowerProp, upperProp ; 
int lowerMedianIndex, upperMedianIndex ; 
int numOfStaticFreqresponseAverages=NUMBER_OF_STATIC_FREQRESPONSE_AVERAGES ; 

float auto_adjust_stopband_dB=-20 ; 

float Fundamental_Frequency_for_Vibrato_Periods_Detection=0. ; 

 

float channel_change_increase_SmoothCoef, minus_channel_change_increase_SmoothCoef ; 
float channel_change_decrease_SmoothCoef, minus_channel_change_decrease_SmoothCoef ; 


float lowMinimaReleaseVibratoPointGainScale_inDecibels,	highMinimaReleaseVibratoPointGainScale_inDecibels ;


float lowFreq, hiFreq ; 

int funcDerivationSize, direction, numberOfMaxima, numberOfMinima, vibratoDetection=0, numberOfMinimaMaximaTimepointDifferences ; 
float *funcDerivation, *minimaTpts, *maximaTpts, *minimaMaximaTimepointDifferences, 
	*minimaMaximaTimepointDifferencesSorted, medianVibratoPeriodLength ; 
int *maximaTptSegmentFlags, *minimaTptSegmentFlags ; 

int numberOfMaximaSAVE, numberOfMinimaSAVE ; 

float minimaClosestToEndOfTone ; 

int releaseSettingsForVibratoHaveBeenSetFlag=0, releaseVibratoPeriodJumpFlag=0 ; 
float 	lowMinimaReleaseVibratoPoint, highMinimaReleaseVibratoPoint,
	crossfadeRamp, releaseVibratoPeriodGainscaleNow ;

float *sourcePointFreqSave, *tunedSourcePointFreqSave ;

int freqMatchFoundFlag ;

float filtwinlow_amountToChange, filtwinhi_amountToChange ; 

int startDataIndex, endDataIndex, groupNumber=(-1) ; 

int i,j,k,  l,  i1,  i2, ipartial, ipartial_save, i1_save, i2_save, tone_now, *partial_Band_Begin,  *trans_switch, doubleBin, 
     mm, numberOfBands,  n,  NC, NCmult2, numTones,  
	*indexInChannelForHarmony, *indexInChannelForNoise, *tone, *bandIndexforHarmonyBins, *synthetic_VIBRATO_SWITCH ;

float maxDelayT=0., ringTimeCountDown=0., *functionDelayT_Switchscaler ; 
int *noise_switch, *noise_bank_tone_number, number_of_noise_banks=0, noiseBank, toneNumber, 
	*tone_Filter_Switch, *tone_Filter_Type, *noise_Filter_Switch, *tone_Channel_Output_Number, *partial_Band_Channel_Output_Number,
		*noise_Bank_Channel_Output_Number, maximum_Channel_Output_Number=0  ; 
float *tonesums,   *oldbandampsum, 
	this_Stasis_Median_harmony, this_Stasis_Median_harmony_for_Static_Harmony_Index, this_Stasis_Median_noise, 
		*oldtunemult  ;

float Vibrato_Period_Duration_Deviation_Threshold_As_Proportion=.05, deviationNow ; 
int stopSearchFlag ; 

int numberOfPeriodsInLongestSegment, indexOfBeginOfLongestSegment, inSegFlag, index, count ; 

int boundariesResetExitCode, boundariesResetFlag ; 

int Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1=0 ; 

 

float peakAmp, peakdB, resetTriggerPoint ; 

float thisForce, forceMedianDifference, 
	*rateCorrelatedMedianRaiser_TONE_VALUES, *rateCorrelatedForceSuppressor_TONE_VALUES, *filtrate_TONE_VALUES,
		*noise_band_decibel_limit_TONE_VALUES, *noise_band_decibel_limit_rolloff_TONE_VALUES,
	*pitchChangeExpansionDecibels_TONE_VALUES, * frequency_change_suppression_threshold_TONE_VALUES, 
	*frequency_change_suppression_threshold_increase_response_time_in_seconds_TONE_VALUES ; 



int rateCorrelatedForceSuppressionSwitch=0, Rate_Correlated_Randomization_Switch=0 ; 

 

float *channel_average ; 

float avg ; int length ; 

float low, hi, range, average, median, mode, standarddeviation, sum, begin, end, middle ; 

float *channel_change_average, *channel_change, *previous_channel_change ; 

float sumOfNonSuppressedStaticAmps, sumOfStaticAmps ; 

float analyzedFundamental=-1. ; 

float vibratoPeriodTableSize=1024. ; 
float *timeWarp, *oldTimeWarp, *ranAmpScale, *oldRanAmpScale, *rateMod, *oldRateMod,  *vibPhaseNow, *vibratoPeriodTable ; 

 
float *noiseBandDecibelLimiterThreshold, *noiseBandChannelPositionIndex ; 

float *HammingWindow ; 
int HammingWindowSize=1024 ; 

float tuneNullPhasePropPoint, tuneNullPhaseCurveIndex,  tuneBandPropPoint, tuneBandCurveIndex, thisTuneFactorSave ; 

int SourceFreqAnalysisAdjustmentSwitch=0; 

 

float *thisFreqresponseFrame, *ampWeightSums, *averageBandFreq ; 

float vibratoDepthInDB=10., vibDepthForceProp ; 

float 		*master_gain_in_dB_TONE_VALUES, *master_gain_TONE_VALUES, master_gain_no_delay_for_source,
		*tones_macro_pitch_controller_in_semitones_TONE_VALUES, *pmt_TONE_VALUES,
			*tones_master_gain_controller_in_dB_TONE_VALUES, *tones_master_gain_controller_TONE_VALUES,
		*tones_master_freq_shift_controller_TONE_VALUES, 

		*Rate_Correlated_Noise_Control_in_dB_TONE_VALUES, 
		*Rate_Correlated_Tone_Control_in_dB_TONE_VALUES,

	*synthetic_Vibrato_Rate_TONE_VALUES, *synthetic_Vibrato_Randomization_Prop_TONE_VALUES, *vibValNow_TONE_VALUES
  ; 

float	Rate_Correlated_Noise_Amp_Randomization_Proportion, Rate_Correlated_Noise_Freq_Randomization_Proportion ;

 


float *tonesChannelAmpSums, *channelAmpSum_delay_buffer, thisChannelAmpSum ; 

float partial_formant_inclusion_threshold_in_dB=-100. ; 
float thisNoiseBankGain ; 

float nyquist;
 
  
int frameNowChannelDelayIndex, ampIndex, freqIndex, bin ; 

 
int thisFrameDelay ;  


float *static_freqresponse_averages ; 

float static_freqresponse_peak_SUM_in_dB ; 
 
float cf ; 
float *channelsSumBefore ; 
float thisAvgFreq, thisProp, thisTuneFactor, thisForceFactor, thisAmpForceFactor ; 
 
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int ainchan ; 
//int loopmode=0,  loopmodenow=0 ;
  
int   eof = 0, obank = 0,  channelout=0, 
     part_count,  numpasspartials, 
    rejected_part_count,  current_rejected_part ;
float thisPartialbw, thisPartialNumber,  P = 1.0;
 
int band ; 

float ampSquared ; 

int *tempChannelNoiseBinFlags, NumNoiseBins, NumNoiseBinsMult2 ; 

float *noise, *previous_noise, *static_noise, thisNoiseBankForceControl;  
int noiseFlag=0; 

int *transposeShiftMethod_PITCH, *transposeShiftMethod_NOISE, this_transposeShiftMethod_PITCH, this_transposeShiftMethod_NOISE ; 
float unshiftedpartialfreq,  this_sourceptfundfreq, *sourceptfundfreq; 

float *channelsSumAfter, *rescaleProp, tProp, env; 

float *vibratoPeriodDurations, averageVibratoPeriodDuration, vibratoPeriodDurationNow ; 


FILE *fopen();




char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *previous_channel, *output ;
float *previous_harmony,   *harmony, *channel_delay, *static_harmony, *previous_static_harmony, 
	*harmony_force_curve_indeces, 
	 *sourcefreq, *filttnow_delay_buffer, *tones_delayed_filttnow, 
	*coswindow,    pms  ; 

int * harmonyBands_IndexOfStrongestBinInBand ;  
float *harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand ; 


float threshfac = .001,  threshfacdB=-96 ;
float	newfreq ;
float  temp, temp1,  temp2,  temp6,  temp7;  
float getthresh();
float   IR,  dur=0., funcDur, saved_dur ;
 

float *analysis_lower,  *analysis_higher, *previous_change, *tempChannel ; 

 

float ditx;   

int maxNumOfDelayFrames ; 

double ar_dB ; 
float loopSmoothCoef, minusLoopSmoothCoef ; 
float asum,  fsum ;
int bcount,  b1 ; 
int saved_mm ; 

float analysis_fundamental ;  
int analysis_N,  analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 

float normamp[MAXIMUM_CHANNELS],  normamppk ;

float oldpeakamp; 

 
 

 

double log2 ; 

float rolloff_amp ; 

int sourceflag=0 ; 
 

float halfcoscurve( float v ),  fullcoscurve( float v ) ; 
float coscurve_dB( float v ) ; 

FILE *data ; 
char datafile[ STRING_SIZE ] = "EMPTY",  new_datafile[ STRING_SIZE ], *user; 
float *PP,    fundamental,  SOURCE_gain,  midC; 
 
float bw ; 
char tempstring[ STRING_SIZE ], tempPitchTrackName[ STRING_SIZE ], tempstring2[ STRING_SIZE ] ; 

int Mode__sampler_loop_0__autostop_1=0,  autostopflag=0,  wrap_0_fold_1_clip_2=0, 
	Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2=0   ; 

int Use_Specified_Time_Boundaries_0__Detected_Vibrato_Periods_1=0 ; 

int   LoopNormalizationFlag=0 ; 
float loopSmoothTime=0.0 ; 

int auto_adjust_cf_and_bw=0 ; 

 
 

// ** FILTER VARIABLES
float filttnow=0., oldfilttnow=0.,   filttinc; 
 
 

float *delayTimes, *timeRateScalers ; 


// ****** TONE PARAMETER FUNCTION STRUCTURES HERE
struct func *transposePoint_PITCH ; float *transposePoint_PITCH_values ; 
struct func *partialShift ;
struct func *spectralStretchCompress ;
struct func *toneDB ;
struct func *stasisMedianForHarmonyInDB ;
struct func *noiseDB ; 
struct func *stasisMedianForNoiseInDB ;
struct func *transposePoint_NOISE ; float *transposePoint_NOISE_values ; 
struct func *forceFactor ; 
struct func *bandpassCF ;
struct func *bandpassRolloffInDbPerOctave ;
struct func *delayTime ;
// ************************************


struct func Vibrato_Period_Durations__Mechanical_0__Natural_1 ; 

struct func pitchTrackFile ; 

struct func pitchTrack ; 

// SYNTHETIC VIBRATO RANDOMIZATION PROPORTION
struct func synthetic_Vibrato_Randomization_Prop ; 

// SYNTHETIC VIBRATO RATE
struct func synthetic_Vibrato_Rate ; 


// RATE CORRELATED TONE CONTROL
struct func Rate_Correlated_Tone_Control_in_dB ; 

// RATE CORRELATED NOISE CONTROL
struct func Rate_Correlated_Noise_Control_in_dB ; 

// PEAK LOOP SMOOTH TIME
struct func peakLoopSmoothTime ; 


// DATA
struct  func  analysis ; 

// ****
//  DATA  RATE
struct  func  filtrate ; 

// DATA TIME POINT ORIGIN
struct  func  filttorigin ; 

// DATA TIME WINDOW LOWER BOUNDARY
struct  func  filtwinlow ; 

// DATA TIME WINDOW UPPER BOUNDARY
struct  func  filtwinhi ; 




// GAIN
struct  func  master_gain_in_dB ; 

// SOURCE  FREQUENCY SHIFT ADDER
struct  func  SOURCE_harmadd ; 

// SOURCE  PITCH TRANSPOSITION IN SEMITONES
struct  func  SOURCE_ptrans ; 

// SOURCE DECIBELS
struct  func  SOURCE_dB ; 


// MASTER TONES CONTROLLER: DECIBELS
struct  func  tones_master_gain_controller_in_dB ; 

// TONES MASTER FREQ SHIFT CONTROL
struct  func  tones_master_freq_shift_controller ; 

// TONES MACRO PITCH TRANSPOSITION IN SEMITONES
struct  func  tones_macro_pitch_controller_in_semitones ; 


// NOISE BAND DECIBEL LIMITER
struct func noise_band_decibel_limit ; 

// NOISE BAND DECIBEL LIMITER ROLLOFF
struct func noise_band_decibel_limit_rolloff ; 


// PITCH CHANGE EXPANSION DECIBELS
struct func pitchChangeExpansionDecibels ; 

// FREQUENCY CHANGE SUPPRESSION THRESHOLD
struct func frequency_change_suppression_threshold ; 

// FREQUENCY CHANGE SUPPRESSION THRESHOLD INCREASE RESPONSE TIME
struct func frequency_change_suppression_threshold_increase_response_time_in_seconds ; 

// *****************INITIALIZE



Vibrato_Period_Durations__Mechanical_0__Natural_1.L = 1. ; 
	Vibrato_Period_Durations__Mechanical_0__Natural_1.n = 1. ; Vibrato_Period_Durations__Mechanical_0__Natural_1.A[ 0 ] = 1. ; 

// PITCH TRACK FILE
pitchTrackFile.L = 1. ; pitchTrackFile.n = 1. ; pitchTrackFile.A[ 0 ] = 0. ; 

pitchTrack.L = 1. ; pitchTrack.n = 1. ; pitchTrack.A[ 0 ] = 0. ; 

// RATE CORRELATED NOISE CONTROL
Rate_Correlated_Noise_Control_in_dB.L = 1. ; Rate_Correlated_Noise_Control_in_dB.n = 1. ; Rate_Correlated_Noise_Control_in_dB.A[ 0 ] = 0. ; 

// RATE CORRELATED TONE CONTROL
Rate_Correlated_Tone_Control_in_dB.L = 1. ; Rate_Correlated_Tone_Control_in_dB.n = 1. ; Rate_Correlated_Tone_Control_in_dB.A[ 0 ] = 0. ; 



// SYNTHETIC VIBRATO RANDOMIZATION PROPORTION
synthetic_Vibrato_Randomization_Prop.L = 1. ; synthetic_Vibrato_Randomization_Prop.n = 1. ; synthetic_Vibrato_Randomization_Prop.A[ 0 ] = 0. ; 

// SYNTHETIC VIBRATO RATE
synthetic_Vibrato_Rate.L = 1. ; synthetic_Vibrato_Rate.n = 1. ; synthetic_Vibrato_Rate.A[ 0 ] = 6. ; 





// PEAK LOOP SMOOTH TIME
peakLoopSmoothTime.L = 1. ; peakLoopSmoothTime.n = 1. ; peakLoopSmoothTime.A[ 0 ] = 0.2 ; 

// FILTER
analysis.L = 1. ; analysis.n = 0. ; analysis.A[ 0 ] = 0. ; 

// *****
//  FILTER  RATE
filtrate.L = 1. ; filtrate.n = 1. ; filtrate.A[ 0 ] = 1. ; 

// FILTER TIME ORIGIN
filttorigin.L = 1. ; filttorigin.n = 1. ; filttorigin.A[ 0 ] = 0. ; 

// FILTER TIME WINDOW LOWER BOUNDARY
filtwinlow.L = 1. ; filtwinlow.n = 1. ; filtwinlow.A[ 0 ] = 0. ; 

// FILTER TIME WINDOW UPPER BOUNDARY
filtwinhi.L = 1. ; filtwinhi.n = 1. ; filtwinhi.A[ 0 ] = -1. ; 



// SOURCE  FREQUENCY SHIFT ADDER
SOURCE_harmadd.L = 1. ; SOURCE_harmadd.n = 1. ; SOURCE_harmadd.A[ 0 ] = 0. ; 

// GAIN
master_gain_in_dB.L = 1. ;  master_gain_in_dB.n = 1. ; master_gain_in_dB.A[ 0 ] = 0. ; 

// SOURCE  PITCH TRANSPOSITION IN SEMITONES
SOURCE_ptrans.L = 1. ; SOURCE_ptrans.n = 1. ; SOURCE_ptrans.A[ 0 ] = 1. ; 

// MASTER TONES CONTROLLER: DECIBELS
tones_master_gain_controller_in_dB.L = 1. ; tones_master_gain_controller_in_dB.n = 1. ; 
		tones_master_gain_controller_in_dB.A[ 0 ] = 0. ; 

// SOURCE DECIBELS
SOURCE_dB.L = 1. ; SOURCE_dB.n = 1. ; SOURCE_dB.A[ 0 ] = 0. ; 

// CONTROLLER:  PITCH TRANSPOSITION IN SEMITONES
tones_macro_pitch_controller_in_semitones.L = 1. ; tones_macro_pitch_controller_in_semitones.n = 1. ;
	 tones_macro_pitch_controller_in_semitones.A[ 0 ] = 1. ; 

// CONTROLLER:  FREQUENCY SHIFT ADDER
tones_master_freq_shift_controller.L = 1. ; tones_master_freq_shift_controller.n = 1. ; tones_master_freq_shift_controller.A[ 0 ] = 0. ; 


// NOISE BAND DECIBEL LIMITER
noise_band_decibel_limit.L = 1. ; noise_band_decibel_limit.n = 1. ; noise_band_decibel_limit.A[ 0 ] = -0. ; 

// NOISE BAND DECIBEL LIMITER ROLLOFF
noise_band_decibel_limit_rolloff.L = 1. ; noise_band_decibel_limit_rolloff.n = 1. ; noise_band_decibel_limit_rolloff.A[ 0 ] = -0. ; 




// PITCH CHANGE EXPANSION DECIBELS
pitchChangeExpansionDecibels.L = 1. ; pitchChangeExpansionDecibels.n = 1. ; pitchChangeExpansionDecibels.A[ 0 ] = -50. ; 

// PITCH CHANGE EXPANSION THRESHOLD
frequency_change_suppression_threshold.L = 1. ; frequency_change_suppression_threshold.n = 1. ; frequency_change_suppression_threshold.A[ 0 ] = 0.1 ; 

// FREQUENCY CHANGE SUPPRESSION THRESHOLD INCREASE RESPONSE TIME
frequency_change_suppression_threshold_increase_response_time_in_seconds.L = 1. ; 
frequency_change_suppression_threshold_increase_response_time_in_seconds.n = 1. ; 
frequency_change_suppression_threshold_increase_response_time_in_seconds.A[ 0 ] = 0.1 ; 




if( argc < 2 )usage() ; 

	while( (ch= crack( argc, argv,  
		"~|:|@|_|=|/|a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|W|x|X|y|Y|z|Z|", 0  )) 
				!= CRACK_DONE_FLAG ) { // !
	// 

	// // 
	switch(ch) {

	    case 'h':   	strcpy(tempstring, arg_option);
			frequency_change_suppression_threshold_increase_response_time_in_seconds.fp = crackstring( tempstring, 
			    &frequency_change_suppression_threshold_increase_response_time_in_seconds );
			break;


	    case 'S':   	strcpy(tempstring, arg_option);
			pitchChangeExpansionDecibels.fp = crackstring( tempstring, 
			    &pitchChangeExpansionDecibels );
			break;
	    case 'c':   	strcpy(tempstring, arg_option);
			frequency_change_suppression_threshold.fp = crackstring( tempstring, 
			    & frequency_change_suppression_threshold );
			break;


	    case 'L':   	strcpy(tempstring, arg_option);
			noise_band_decibel_limit.fp = crackstring( tempstring, 
			    & noise_band_decibel_limit );
			break;

	    case 'b':   	strcpy(tempstring, arg_option);
			noise_band_decibel_limit_rolloff.fp = crackstring( tempstring, 
			    & noise_band_decibel_limit_rolloff );
			break;

	    case 'u':	Fundamental_Frequency_for_Vibrato_Periods_Detection = crackfloat( arg_option, ch ) ;
			break;


	    case 'l':	partial_formant_inclusion_threshold_in_dB = crackfloat( arg_option, ch ) ;
			break;

	    case 'o':	auto_adjust_stopband_dB = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'n':	auto_adjust_cf_and_bw = (int) crackfloat( arg_option, ch ) ;
			break;



	    case 'V':	Vibrato_Period_Duration_Deviation_Threshold_As_Proportion = crackfloat( arg_option, ch ) ;
			break;


	    case 'H':	Rate_Correlated_Randomization_Switch = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'O':	SourceFreqAnalysisAdjustmentSwitch = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'T':   	strcpy(tempstring, arg_option);
			Rate_Correlated_Tone_Control_in_dB.fp = crackstring( tempstring, 
			    &Rate_Correlated_Tone_Control_in_dB );
			break;


	    case 'E':   	strcpy(tempstring, arg_option);
			Rate_Correlated_Noise_Control_in_dB.fp = crackstring( tempstring, 
			    &Rate_Correlated_Noise_Control_in_dB );
			break;




	    case 'U':   	strcpy(tempstring, arg_option);
			Vibrato_Period_Durations__Mechanical_0__Natural_1.fp = crackstring( tempstring, 
			    &Vibrato_Period_Durations__Mechanical_0__Natural_1 );
			break;

	    case 'B':	rateCorrelatedForceSuppressionSwitch = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'M':	Nw = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'I':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'D':   dur = saved_dur = funcDur = crackfloat( arg_option, ch ) ;
			break;
	    case 'A':   strcpy(tempstring, arg_option);
			master_gain_in_dB.fp = crackstring( tempstring, 
			    &master_gain_in_dB );
			break;

	    case 'C':  channelout = (int) crackfloat( arg_option, ch ) ;  // channelout = atoi(arg_option) ;
			break;

            case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; 
			break;
            case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; 
			break;

           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':	rescalev = crackfloat( arg_option, ch ) ; break;



// SOURCE
          case 's':   sourceflag = (int) crackfloat( arg_option, ch ) ; break;

	    case 'a':   	strcpy(tempstring, arg_option);
			SOURCE_harmadd.fp = crackstring( tempstring, 
			    &SOURCE_harmadd );
			break;
	    case 'P':   	strcpy(tempstring, arg_option);
			SOURCE_ptrans.fp = crackstring( tempstring, &SOURCE_ptrans ); 
			break;
	    case 'G':   	strcpy(tempstring, arg_option);
			SOURCE_dB.fp = crackstring( tempstring, 
			    &SOURCE_dB );
			break;



// NOISE/REMAINDER

// CONTROLLERS

	    case 'q':   strcpy(tempstring, arg_option);
			tones_master_freq_shift_controller.fp = crackstring( tempstring, 
			    &tones_master_freq_shift_controller );
			break;
	    case 'X':   strcpy(tempstring, arg_option);
			tones_macro_pitch_controller_in_semitones.fp = crackstring( tempstring, 
			    &tones_macro_pitch_controller_in_semitones );
			break;
	    case 'm':   strcpy(tempstring, arg_option);
			tones_master_gain_controller_in_dB.fp = crackstring( tempstring, 
			    &tones_master_gain_controller_in_dB );
			break;



// AMPLITUDE RESPONSE


// SYNTHETIC VIBRATO
	    case 'r':   strcpy(tempstring, arg_option);
			synthetic_Vibrato_Rate.fp = crackstring( tempstring, 
			    &synthetic_Vibrato_Rate );
			break;

	    case 'v':   strcpy(tempstring, arg_option);
			synthetic_Vibrato_Randomization_Prop.fp = crackstring( tempstring, 
			    &synthetic_Vibrato_Randomization_Prop );
			break;





// ** NEW BEGIN

	    case 'f':   strcpy(tempstring, arg_option);
			analysis.fp = crackstring_bin_only( tempstring, 
			    &analysis );
			break;
 

	    case 'x':   strcpy(tempstring, arg_option);
			filttorigin.fp = crackstring( tempstring, 
			    &filttorigin );
			break;
	    case 'Y':   strcpy(tempstring, arg_option);
			filtrate.fp = crackstring( tempstring, 
			    &filtrate );
			break;

	    case 'g':   strcpy(tempstring, arg_option);
			filtwinlow.fp = crackstring( tempstring, 
			    &filtwinlow );
			break;
	    case 'k':   strcpy(tempstring, arg_option);
			filtwinhi.fp = crackstring( tempstring, 
			    &filtwinhi );
			break;

	    case 'R': Use_Specified_Time_Boundaries_0__Detected_Vibrato_Periods_1 = (int) crackfloat( arg_option, ch );
			break;



	    case 'Q':	wrap_0_fold_1_clip_2 = (int) crackfloat( arg_option, ch );
			break;

	    case '@':	Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 = (int) crackfloat( arg_option, ch );
			break;

	    case '/':   strcpy(tempstring, arg_option);
			peakLoopSmoothTime.fp = crackstring( tempstring, &peakLoopSmoothTime );
			break;


	    case 'z':   Mode__sampler_loop_0__autostop_1 = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'e':   LoopNormalizationFlag = (int) crackfloat( arg_option, ch ) ; 
			break;




// ** NEW END

	    case 'F':   strcpy(datafile, arg_option);
			break;

	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	}
    } ; 


/*
for( i = 0; i < NUMBER_OF_WRITE_NUMBERS; i++ ){
	sprintf( tempstring, "/tmp/d%d", i ) ; 
	adata[i] = fopen( tempstring, "w") ; 

} ; 
*/

// GET NAME OF USER
user = getlogin(); 

// DEBUG FLAG
debugFlag = 0 ; 

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "CHORDMAPPERPLUS", 69 ) ; 
prline( 69,  "-" ) ; 


// BEGIN ********* NEW


    log2 = 1. / log10( 2. ) ; 
     
    // READ IN FFT HEADER VALUES
    if( 
	readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &k,  normamp, &analysis, 1 ) == -1
    ){
        fprintf( stderr, "CHECK YOUR ANALYSIS FILE.\t\t. . . BYE.\n\n\n" ) ; EXIT_failure() ; 
    } ; 


	Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1 = Use_Specified_Time_Boundaries_0__Detected_Vibrato_Periods_1 ; 

    // MAKE THE MASTER PEAK AMP
    normamppk = 0 ; 
    for(k = 0; k < analysis_chan; k++) 
	if( normamp[k] > normamppk) normamppk  = normamp[k]; 
		
    //COPY PEAKAMPS INTO INPUT FILE PEAKAMPS
     for(k = 0; k < analysis_chan; k++)
	ipeakamp[ k ] = normamp[k] ; 
 
 
 
// *******
// FIND DURATION OF INPUT FILE

    niframes = (((analysis.n - (float) FFT_HEADER_SIZE) / (float) (analysis_N + 2))) / analysis_chan ; 
    analysis_dur = (float) (niframes) / ((float) analysis_R / (float) analysis_D ) ; 
    iframes_per_sec = (float) analysis_R /  (float) analysis_D ; 
    pri( niframes,  "NUMBER OF FRAMES IN ANALYSIS" ) ; 

// *******
	// SET OUTPUT DURATION TO THE DURATION OF THE ANALYSIS FILE.
	if( dur <= 0. ){
		 dur =  saved_dur = funcDur = analysis_dur ; 
		prt( "OUTPUT DURATION SET TO DURATION OF ANALYSIS FILE" ) ; 
		prf( dur, "OUTPUT DURATION" ) ; 
	} ; 
// *******

    N = analysis_N; isr = R = analysis_R; D = analysis_D ; ichan = analysis_chan ; 
    idur = analysis_dur ; isr = R ; 


// END ********* NEW

    // -1 FLAGS 
    if( filtwinlow.A[0] < 0.0 ) { filtwinlow.A[0] = 0.0 ;  } ; 
    if( filtwinhi.A[0] < 0.0 ) { filtwinhi.A[0] = analysis_dur ;  } ; 

    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }

 
// *** NEW BEGIN

// DURATION
prf( dur, "OUTPUT DURATION" ) ; 
    outdur = dur ; 
    if( outdur <= 0. ){
	// BOGUS DURATION
	prt( "\n\n=============> BOGUS INPUT DURATION <=================\n\n................BYE." ) ; 
	prf( outdur,  "DURATION" ) ; EXIT_failure() ; 
	 
    }




// **************************GET DATA	
// READ IN CHORD TONES, SHIFTPOINT, NUMBER_OF_PARTIALS, BW, AND DB


 

// **********

// MAKE NEW DATA FILE WITH COMMENTED, MUTED, OR NON-SOLO-ED LINES REMOVED.

    cut_data_lines( datafile,  new_datafile,  NUM_PARAMETERS ) ; 


// OPEN FILE
    if( (data = fopen( new_datafile, "r")) == NULL ){
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  datafile ) ; 
	    EXIT_failure( EXIT_FAILURE ); 
    }

 
// COUNT ENTRIES IN FILE
    k = 0 ; 

    while( fscanf( data,  "%s",  tempstring ) != EOF ) k++ ; 	
    rewind( data ) ;    

    // ABORT IF NOT ENOUGH DATA
    if( k < (NUM_PARAMETERS - 1) ){
        prt( "\n\n==========> INSUFFICIENT TONES DATA FILE <==========\n\n" ) ; 
        EXIT_failure() ; 
    } ; 

    // ALLOCATE SPACE FOR CHORD TONES
    fvec( PP, k ) ;	// PARTIALS

    // COMPUTE NUMBER OF TONES
    numTones = k / NUM_PARAMETERS ;

//    pri( numTones, "numTones" ) ; 
    for( toneNumber = 0; toneNumber < numTones; toneNumber++ ){
        fprintf( stderr, "\nTONE: %d\n", toneNumber ) ; 
        for( i = 0; i < NUM_PARAMETERS; i++ ){
            fscanf( data,  "%s ",  tempstring ) ; fprintf( stderr, "%s ", tempstring ) ;

	} ; 
	fprintf( stderr, "\n" ) ; 
  } ; 
    rewind( data ) ;    


    // MAKE ARRAYS
    // PARAMETER 1
    transposePoint_PITCH = (struct func *)  calloc( numTones, sizeof(struct func) );
        fvec( transposePoint_PITCH_values, numTones  ) ; 
    // PARAMETER 6
    partialShift = (struct func *)  calloc( numTones, sizeof(struct func) );

    // PARAMETER 7
    spectralStretchCompress = (struct func *)  calloc( numTones, sizeof(struct func) );

    // PARAMETER 8
    toneDB = (struct func *)  calloc( numTones, sizeof(struct func) );
    // PARAMETER 9
    stasisMedianForHarmonyInDB = (struct func *)  calloc( numTones, sizeof(struct func) );
    // PARAMETER 10
    noiseDB = (struct func *)  calloc( numTones, sizeof(struct func) );
    // PARAMETER 11
    stasisMedianForNoiseInDB = (struct func *)  calloc( numTones, sizeof(struct func) );
    // PARAMETER 13
    transposePoint_NOISE = (struct func *)  calloc( numTones, sizeof(struct func) );
        fvec( transposePoint_NOISE_values, numTones ) ; 
    // PARAMETER 14
    forceFactor = (struct func *)  calloc( numTones, sizeof(struct func) );
    // PARAMETER 17
    bandpassCF = (struct func *)  calloc( numTones, sizeof(struct func) );
    // PARAMETER 18
    bandpassRolloffInDbPerOctave = (struct func *)  calloc( numTones, sizeof(struct func) );
    // PARAMETER 21
    delayTime = (struct func *)  calloc( numTones, sizeof(struct func) );
    fvec( delayTimes, numTones ) ; 
    fvec( timeRateScalers, numTones ) ; 

    fprintf( stderr, "\n\n CONVERTING DATA AND FILE NAMES FROM DATA FILE . . . \n\n" ) ; 

    for( toneNumber = 0; toneNumber < numTones; toneNumber++ ){
        fprintf( stderr, "\nTONE: %d\n", toneNumber ) ; 
        for( i = 0; i < NUM_PARAMETERS; i++ ){
            fscanf( data,  "%s ",  tempstring ) ; 
            fprintf( stderr, "%s ", tempstring ) ;  

pd( 1000 ) ; 


            switch (i){ 
                STRUCTURE_TRANSPOSE_POINT_CASE_1 
                    transposePoint_PITCH[toneNumber].L = 1. ; transposePoint_PITCH[toneNumber].n = 1. ; 
                        transposePoint_PITCH[toneNumber].A[ 0 ] = 1. ; 
                    transposePoint_PITCH[toneNumber].fp = crackstring( tempstring, &transposePoint_PITCH[toneNumber] ) ; 
                    break ; 
                STRUCTURE_PARTIAL_SHIFT_CASE_6 
                    partialShift[toneNumber].L = 1. ; partialShift[toneNumber].n = 1. ; partialShift[toneNumber].A[ 0 ] = 0. ; 
                    partialShift[toneNumber].fp = crackstring( tempstring, &partialShift[toneNumber] ) ; 
                    break ; 
                STRUCTURE_SPECTRAL_STRETCH_COMPRESS_CASE_7 
                    spectralStretchCompress[toneNumber].L = 1. ; spectralStretchCompress[toneNumber].n = 1. ; spectralStretchCompress[toneNumber].A[ 0 ] = 0. ; 
                    spectralStretchCompress[toneNumber].fp = crackstring( tempstring, & spectralStretchCompress[toneNumber] ) ; 
                    break ; 
               STRUCTURE_TONE_DECIBELS_CASE_8 
                    toneDB[toneNumber].L = 1. ; toneDB[toneNumber].n = 1. ; toneDB[toneNumber].A[ 0 ] = 0. ; 
                    toneDB[toneNumber].fp = crackstring( tempstring, &toneDB[toneNumber] ) ; 
                    break ; 
                STRUCTURE_STASIS_MEDIAN_FOR_HARMONY_IN_DB_CASE_9 
                    stasisMedianForHarmonyInDB[toneNumber].L = 1. ; stasisMedianForHarmonyInDB[toneNumber].n = 1. ;
                        stasisMedianForHarmonyInDB[toneNumber].A[ 0 ] = 0. ; 
                    stasisMedianForHarmonyInDB[toneNumber].fp = crackstring( tempstring, & stasisMedianForHarmonyInDB[toneNumber] ) ; 
                    break ; 
                STRUCTURE_NOISE_DB_CASE_10 
                    noiseDB[toneNumber].L = 1. ; noiseDB[toneNumber].n = 1. ; noiseDB[toneNumber].A[ 0 ] = 0. ; 
                    noiseDB[toneNumber].fp = crackstring( tempstring, &noiseDB[toneNumber] ) ; 
                    break ; 
                STRUCTURE_STASIS_MEDIAN_FOR_NOISE_IN_DB_CASE_11 
                    stasisMedianForNoiseInDB[toneNumber].L = 1. ; stasisMedianForNoiseInDB[toneNumber].n = 1. ; 
                        stasisMedianForNoiseInDB[toneNumber].A[ 0 ] = 0. ; 
                    stasisMedianForNoiseInDB[toneNumber].fp = crackstring( tempstring, & stasisMedianForNoiseInDB[toneNumber] ) ; 
                    break ; 
                STRUCTURE_TRANSPOSE_POINT_NOISE_CASE_13 
                    transposePoint_NOISE[toneNumber].L = 1. ; transposePoint_NOISE[toneNumber].n = 1. ; 
                        transposePoint_NOISE[toneNumber].A[ 0 ] = 1. ; 
                    transposePoint_NOISE[toneNumber].fp = crackstring( tempstring, & transposePoint_NOISE[toneNumber] ) ; 
                    break ; 
                STRUCTURE_FORCE_FACTOR_CASE_14 
                    forceFactor[toneNumber].L = 1. ; forceFactor[toneNumber].n = 1. ; forceFactor[toneNumber].A[ 0 ] = 1. ; 
                    forceFactor[toneNumber].fp = crackstring( tempstring, &forceFactor[toneNumber] ) ; 
                    break ; 
                STRUCTURE_BANDPASS_CF_CASE_17 
                    bandpassCF[toneNumber].L = 1. ; bandpassCF[toneNumber].n = 1. ; bandpassCF[toneNumber].A[ 0 ] = 1. ; 
                    bandpassCF[toneNumber].fp = crackstring( tempstring, &bandpassCF[toneNumber] ) ; 
                    break ; 
                STRUCTURE_BANDPASS_ROLLOFF_IN_DB_PER_OCTAVE_CASE_18 
                    bandpassRolloffInDbPerOctave[toneNumber].L = 1. ; bandpassRolloffInDbPerOctave[toneNumber].n = 1. ;
                        bandpassRolloffInDbPerOctave[toneNumber].A[ 0 ] = 0. ; 
                    bandpassRolloffInDbPerOctave[toneNumber].fp = crackstring( tempstring, &bandpassRolloffInDbPerOctave[toneNumber] ) ; 
                    break ; 
                STRUCTURE_DELAY_TIME_CASE_21 
                    delayTime[toneNumber].L = 1. ; delayTime[toneNumber].n = 1. ; delayTime[toneNumber].A[ 0 ] = 0. ; 
                    delayTime[toneNumber].fp = crackstring( tempstring, &delayTime[toneNumber] ) ;
                    break ; 



                VALUE_SOURCE_POINT_CASE_0 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_0, toneNumber ) ; EXIT_failure() ; 
                   } ; 
                   break ; 

                VALUE_LOW_PARTIAL_CASE_2 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_2, toneNumber ) ; EXIT_failure() ; 
                   } ; 
                VALUE_PARTIAL_SPACING_CASE_3 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_3, toneNumber ) ; EXIT_failure() ; 
                   } ; 
                VALUE_NUMBER_OF_PARTIALS_CASE_4 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_4, toneNumber ) ; EXIT_failure() ; 
                   } ; 
                VALUE_BANDWIDTH_CASE_5 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_5, toneNumber ) ; EXIT_failure() ; 
                   } ; 
              
                VALUE_NOISE_SWITCH_CASE_12 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_12, toneNumber ) ; EXIT_failure() ; 
                   } ; 

                VALUE_MASTER_TRANSPOSITION_SWITCH_CASE_15 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_15, toneNumber ) ; EXIT_failure() ; 
                   } ; 
                VALUE_SYNTHETIC_VIBRATO_SWITCH_CASE_16 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_16, toneNumber ) ; EXIT_failure() ; 
                   } ; 

                VALUE_TONE_FILTER_SWITCH_CASE_19 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_19, toneNumber ) ; EXIT_failure() ; 
                   } ; 
                VALUE_DELAY_TIME_SWITCH_SCALER_CASE_20 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_20, toneNumber ) ; EXIT_failure() ; 
                   } ; 
                VALUE_TONE_CHANNEL_OUTPUT_NUMBER_CASE_22 
                    if( (k = sscanf( tempstring,  "%f",  &PP[ (NUM_PARAMETERS * toneNumber) + i] )) == 0 ){
                        fprintf( stderr, 
                            "\n\nERROR: BAD INPUT DATA ----> %s <---- IN PARAMETER FIELD %d (%s) OF TONE %d \n\n", 
                                  tempstring, i, PARAMETER_NAME_22, toneNumber ) ; EXIT_failure() ; 
                   } ; 


            } ; 

        } ; 
        fprintf( stderr, "\n" ) ; 

    } ; 

pd( 1001 ) ; 


    // **** FIND THE VIBRATO PITCH PERIOD MAXIMA AND MINIMAS. ???
    if( Use_Specified_Time_Boundaries_0__Detected_Vibrato_Periods_1 == 1 ){
	  	prt( "\n******************** VIBRATO ANALYSIS ***************************\n"); 

		if( Fundamental_Frequency_for_Vibrato_Periods_Detection == 0. ){
			prt( "\n\nERROR ==> Vibrato Periods Fundamental Detection Frequency must be specified.\n\n. . . BYE.\n\n" ) ; 
			exit( EXIT_FAILURE ) ; 
		} ; 

		prt( "USING ORIGINAL SOUND FILE AND pitchtracker TO FIND VIBRATO PERIODS."  ) ;  
		prs( analysis.fname, "ANALYSIS FILE" ) ; 
		prs( afile, "ORIGINAL SOUND FILE" ) ; 
		sprintf( tempPitchTrackName, "/tmp/pitchtrack.%s.%d", user, (int)(random()) ) ;
		filesToRemove( tempPitchTrackName, 0 ) ;			
		// 1. DONE
		if( Fundamental_Frequency_for_Vibrato_Periods_Detection <= 12. )
			temp = OPPC_to_Hz( Fundamental_Frequency_for_Vibrato_Periods_Detection ) ; 
		else
			temp = Fundamental_Frequency_for_Vibrato_Periods_Detection ; 
		lowFreq = temp * (5. / 6. ) ;  
		hiFreq = temp * (6. / 5. ) ;

 		// PITCHTRACKER FOR VIBRATO ANALYSIS
		sprintf( tempstring2,  
		"pitchtracker -N%d -M0 -D%d -b%f -e%f -C%d -f%f -F%f -L.0 -l.0 -W0 -G-0 -T-0 -S-80 -r1000 -g1 -m2 -j.05 -J.2 -d-90 -O0 -o220 -a0. -X0 -p1 -P0  -E0 -H0 %s %s", 
				N, 200, (float) begint, (float) endt, channelout, lowFreq, hiFreq, afile, tempPitchTrackName ) ; 
		system( tempstring2 ) ; 


		pitchTrackFile.fp = crackstring( tempPitchTrackName, &pitchTrackFile );
	
        
        funcDerivationSize = (int)(analysis_dur * iframes_per_sec) ; fvec( funcDerivation, funcDerivationSize ) ; 
	    // ZERO FILL
        for( i = 0; i < funcDerivationSize; i++ ) funcDerivation[ i ] = 0. ; 
	    // MAKE ARRAY OF DERIVATIVE
        for( t = 0., i = 0;  (t < analysis_dur) || (i < funcDerivationSize) ; t += (1.0 / iframes_per_sec), i++ ){
            pitchTrackFile.A[ 0 ] = fval( &pitchTrackFile, analysis_dur, t ) ;
            funcDerivation[ i ] = (i == 0) ? 0. : (pitchTrackFile.A[ 0 ] - temp)  ;
            temp = pitchTrackFile.A[ 0 ] ; 
        } ; 

        direction = -1 ; numberOfMinima = 0 ; numberOfMaxima = 0 ; 

        for( i = 0, t = 0.; i < funcDerivationSize; i++, t += (1.0 / iframes_per_sec) ){
            if( funcDerivation[ i ] > 0. ){
                if( direction == -1 ){
                    // MINIMA POINT
                    direction = 1 ; numberOfMinima++ ; 
               } ; 
            }else if( funcDerivation[ i ] < 0.){
                if( direction == 1 ){
                   // MAXIMA POINT
                    direction = -1 ; numberOfMaxima++ ; 
                } ; 
            } ; 

        } ;   


        if( (numberOfMaxima >=  1) && (numberOfMinima >=  3) ){
            vibratoDetection = 1 ; 
	      // SAVE MAXIMA AND MINIMA
            fvec( maximaTpts, numberOfMaxima ) ; fvec( minimaTpts, numberOfMinima ) ;
            ivec( maximaTptSegmentFlags, numberOfMaxima - 1 ) ; ivec( minimaTptSegmentFlags, numberOfMinima - 1 ) ;

            direction = -1 ; numberOfMinima = 0 ; numberOfMaxima = 0 ; 
            for( i = 0, t = 0.; i < funcDerivationSize; i++, t += (1.0 / iframes_per_sec) ){
                if( funcDerivation[ i ] > 0. ){
                    if( direction == -1 ){
                        // MINIMA POINT
                        direction = 1 ; minimaTpts[ numberOfMinima ] = t ; numberOfMinima++ ; 
                    } ; 
                }else if( funcDerivation[ i ] < 0.){
                    if( direction == 1 ){
                        // MAXIMA POINT
                        direction = -1 ; maximaTpts[ numberOfMaxima ] = t ; numberOfMaxima++ ; 
                    } ; 
                } ; 

            } ;   


        }else{
            vibratoDetection = 0 ; 
        } ; 

		fclose( pitchTrackFile.fp ) ; 

    } ; 

pd( 1002 ) ; 


pd( 10025 ) ; 

    	if( vibratoDetection == 1 ){
pd( 1003 ) ; 

		// MAKE AND FILL AN ARRAY OF THE TIMEPOINT DIFFERENCES FOR BOTH MINIMA AND MAXIMA POINTS
		// IN ORDER TO FIND THE MEDIAN DIFFERENCE. 

		numberOfMinimaMaximaTimepointDifferences = numberOfMinima + numberOfMaxima - 2 ;  
		fvec( minimaMaximaTimepointDifferences, numberOfMinimaMaximaTimepointDifferences ) ; 
        	fvec( minimaMaximaTimepointDifferencesSorted, numberOfMinimaMaximaTimepointDifferences ) ; 



        	for( i = 0; i < numberOfMinima - 1; i++ ) 
			minimaMaximaTimepointDifferences[ i ] = minimaTpts[i + 1] - minimaTpts[i] ; 
        	for( i = 0; i < numberOfMaxima - 1; i++ ) 
			minimaMaximaTimepointDifferences[ i + numberOfMinima - 1 ] = maximaTpts[ i ] - maximaTpts[i - 1] ; 

    			// SORT 
	   	for( i = 0; i < numberOfMinimaMaximaTimepointDifferences; i++ ) 
			minimaMaximaTimepointDifferencesSorted[i] = minimaMaximaTimepointDifferences[i] ;
        	j = 0 ;
        	while( j != 1 ){
          		j = 1 ; 
            	for(i = 1; i < (numberOfMinimaMaximaTimepointDifferences); i++ ){
                	if( minimaMaximaTimepointDifferencesSorted[i - 1] > minimaMaximaTimepointDifferencesSorted[i] ){
                    		temp = minimaMaximaTimepointDifferencesSorted[i] ; 
                    		minimaMaximaTimepointDifferencesSorted[i] = minimaMaximaTimepointDifferencesSorted[i - 1] ;
                    		minimaMaximaTimepointDifferencesSorted[i - 1] = temp ; 
                    		j = 0 ; 
                	} ; 
            	} ; 
        	} ; 
		// END SORT

// HERE
		deviationNow = Vibrato_Period_Duration_Deviation_Threshold_As_Proportion ; 
		stopSearchFlag = 0 ; 

		numberOfMaximaSAVE = numberOfMaxima ; numberOfMinimaSAVE = numberOfMinima ; 

		while( stopSearchFlag != 1 ){

        		// SAVE MEDIAN PERIOD LENGTH
        		medianVibratoPeriodLength =  minimaMaximaTimepointDifferencesSorted[ (int)( 0.5 + 
				(float) numberOfMinimaMaximaTimepointDifferences / 2.) ] ;    
	  		fprintf( stderr, "\n\nAVERAGE VIBRATO PERIOD DURATION: %f\n", medianVibratoPeriodLength ) ; 
	  		fprintf( stderr, "THRESHOLD PROPORTION: %f ( DURATION RANGE: %f <--> %f )\n", 
				deviationNow, 
					medianVibratoPeriodLength * (1. - deviationNow), 
						medianVibratoPeriodLength * (1. + deviationNow) ) ; 

        		// SETUP FLAGS
        		for(i = 0; i < numberOfMinima - 1; i++ ){
            		temp = medianVibratoPeriodLength  / (minimaTpts[i + 1] - minimaTpts[ i ]) ; 
            		minimaTptSegmentFlags[ i ] = 
					( (temp > (1. + deviationNow)) || 
						(temp < (1. - deviationNow)) ) ? 0 : 1 ;  
        		} ; 

 
       		for(i = 0; i < numberOfMaxima - 1; i++ ){
            		temp = medianVibratoPeriodLength  / (maximaTpts[i + 1] - maximaTpts[ i ]) ; 
            		maximaTptSegmentFlags[ i ] = 
					( (temp > (1. + deviationNow)) || 
					(temp < (1. - deviationNow)) ) ? 0 : 1 ;  
        		} ; 



        		// FIND THE LONGEST SEGMENT OF ACCEPTABLE PERIODS AND SETUP AS THE TIMEPOINTS
        		indexOfBeginOfLongestSegment = 0 ; numberOfPeriodsInLongestSegment = 0 ; 
        		index = 0; count = 0 ; 
        		inSegFlag = 0 ; 
        		for(i = 0; i < numberOfMinima - 1; i++ ){
            		if( minimaTptSegmentFlags[ i ] == 1 ){
                		if( inSegFlag == 0 ){
                   			inSegFlag = 1 ; index = i ; count = 1 ;  
                		}else{ 
                    			// COUNT
                    			count++ ; 
                		} ; 
                		if( i == (numberOfMinima - 2) ){ // LAST
                    			if( count > numberOfPeriodsInLongestSegment ){
                        			indexOfBeginOfLongestSegment = index ; numberOfPeriodsInLongestSegment = count ;  
                    			} ;             
                		} ; 
            		}else{
                		// 0: STOP COUNTING
                		if( inSegFlag == 1 ){
                    			inSegFlag = 0 ; 
                    			if( count > numberOfPeriodsInLongestSegment ){
                        			indexOfBeginOfLongestSegment = index ; numberOfPeriodsInLongestSegment = count ;  
                    			} ;             
                		}
            		} ; 
        		} ; 


//	  		fprintf( stderr, "", ?? ) ; 
			//	******* PRINT ***********
	  		fprintf( stderr, "\nVIBRATO PERIODS:\n" ) ;
	  		fprintf( stderr, "MINIMA TIMEPOINT, DURATION\n" ) ; 
	   		k = 0 ;  
	  		for(i = 0; i < numberOfMinima - 1; i++ ){ 
				fprintf( stderr, "%d) %f \t%f", i, minimaTpts[i], minimaTpts[i + 1] - minimaTpts[i] ) ; 		
				if( minimaTptSegmentFlags[ i ] == 1 ) {
					fprintf( stderr, "<---- " ) ; 
					if( (i >= indexOfBeginOfLongestSegment) && 
						(i <= (indexOfBeginOfLongestSegment + numberOfPeriodsInLongestSegment - 1) )   
						){
							if( k == 0 ) {
								fprintf( stderr,  "SELECTED VIBRATO REGION" ) ; k = 1 ; 
							}else{
								fprintf( stderr, "***********************" ) ;  
							} ;
						} ;
				}
				fprintf( stderr, "\n" ) ;  
	  		}; 


			// TEST FOR RELEASE FROM SEARCH
			if( ((numberOfPeriodsInLongestSegment + 1) >= 3) || (deviationNow > 0.75) ){
				stopSearchFlag = 1 ; 
				prt( "ENDING SEARCH.\n" ) ; 
			}else{
				deviationNow = ((1.0 + deviationNow) * 
					(1.0 + Vibrato_Period_Duration_Deviation_Threshold_As_Proportion)) - 1.0 ; 
				numberOfMaxima = numberOfMaximaSAVE ; numberOfMinima  = numberOfMinimaSAVE ; 
				prt( "REVISING SEARCH  . . . \n" ) ; 
				prf( deviationNow, "NEW EXPANDED THRESHOLD PROPORTION" ) ; 

			} ; 


		}

pd( 1004 ) ; 

		// SAVE MINIMA TIMEPOINT CLOSEST TO END. 
		minimaClosestToEndOfTone =  minimaTpts[ numberOfMinima - 1 ] ; 
prf( minimaClosestToEndOfTone, "minimaClosestToEndOfTone" ) ; 


		// REORGANIZE MINIMA TIMEPOINTS
        	numberOfMinima = numberOfPeriodsInLongestSegment + 1 ; 
        	for( i = 0; i < numberOfMinima; i++ ) {
        		minimaTpts[ i ] = minimaTpts[ i + indexOfBeginOfLongestSegment ] ; 
        	} ; 

	   	// **
        	// FIND THE LONGEST SEGMENT OF ACCEPTABLE PERIODS AND SETUP AS THE TIMEPOINTS
        	indexOfBeginOfLongestSegment = 0 ; numberOfPeriodsInLongestSegment = 0 ; 
        	index = 0; count = 0 ; 
        	inSegFlag = 0 ; 
        	for(i = 0; i < numberOfMaxima - 1; i++ ){
          		if( maximaTptSegmentFlags[ i ] == 1 ){
                	if( inSegFlag == 0 ){
                   		inSegFlag = 1 ; index = i ; count = 1 ;  
                	}else{ 
                    		// COUNT
                    		count++ ; 
                	} ; 
                	if( i == (numberOfMaxima - 2) ){ // LAST
                    		if( count > numberOfPeriodsInLongestSegment ){
                        		indexOfBeginOfLongestSegment = index ; numberOfPeriodsInLongestSegment = count ;  
                    		} ;             
                	} ; 
            	}else{
                	// 0: STOP COUNTING
                	if( inSegFlag == 1 ){
                    		inSegFlag = 0 ; 
                    		if( count > numberOfPeriodsInLongestSegment ){
                        		indexOfBeginOfLongestSegment = index ; numberOfPeriodsInLongestSegment = count ;  
                    		} ;             
                	}
            	} ; 
        	} ; 


        	// REORGANIZE MAXIMA TIMEPOINTS
        	numberOfMaxima = numberOfPeriodsInLongestSegment + 1 ; 
        	for( i = 0; i < numberOfMaxima; i++ ) maximaTpts[ i ] = maximaTpts[ i + indexOfBeginOfLongestSegment ] ; 

		
		// TURN OFF 
        	if( numberOfMinima < 3 ){
		 	vibratoDetection = 0 ; 
            prt( "WARNING: INSUFFICIENT NUMBER OF VIBRATO PERIODS FOUND." ) ; 
        	}else{


        	} ; 

        	// FIND THE VIBRATO PERIOD DURATIONS AND THE AVERAGE VIBRATO PERIOD DURATION.
        	fvec( vibratoPeriodDurations, numberOfMinima - 1 ) ; averageVibratoPeriodDuration = 0. ; 
        	for(i = 0 ; i < numberOfMinima - 1; i++ ) {
            	vibratoPeriodDurations[i] = minimaTpts[i + 1] - minimaTpts[i] ; 
            	averageVibratoPeriodDuration += vibratoPeriodDurations[i] ; 
        	} ; 
       	averageVibratoPeriodDuration /= (float) numberOfMinima ; 


    	} ; // END OF "if( vibratoDetection == 1 )"

pd( 1005 ) ; 


    	// ABORT IF ATTEMPTING TO USE VIBRATO PERIODS AS RATE UNIT WITHOUT SUFFICIENT NUMBER OF PERIODS.
    	if( (Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1 == 1) && (vibratoDetection == 0) ){
        	prt( "CRITICAL FAILURE: INSUFFICIENT NUMBER OF VIBRATO PERIODS FOUND. SYNTHESIS ABORTED." ) ; 
	  	prt( "INCREASE VIBRATO PERIOD DETECTION THRESHOLD OR TURN OFF VIBRATO PERIOD DETECTION AND SET APPROPRIATE TIME BOUNDARIES.\n\n" ) ;  
        	prt( ". . . . . . . BYE.\n\n") ;   
        	EXIT_failure() ; 
    	}; 

pd( 1006 ) ; 

	    // **


	    // REDEFINE LOOP BEGIN AND END
	if( (Mode__sampler_loop_0__autostop_1 == 0) 
//			&& (Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) 
				&& (vibratoDetection == 1) )
	{

	    // FILTER TIME WINDOW LOWER BOUNDARY
	    filtwinlow.L = 1. ; filtwinlow.n = 1. ; filtwinlow.A[ 0 ] = minimaTpts[ 0 ] ; 

	    // FILTER TIME WINDOW UPPER BOUNDARY
	    filtwinhi.L = 1. ; filtwinhi.n = 1. ; filtwinhi.A[ 0 ] = minimaTpts[ numberOfMinima - 1 ] ;   

	} ;   

	// ** VIBRATO DETECTION END?


	fvec( sourcePointFreqSave, numTones ) ; 
	fvec( tunedSourcePointFreqSave, numTones ) ; 

	for( toneNumber = 0, j = 0; toneNumber < numTones ; toneNumber++, j += NUM_PARAMETERS ){
        if( PP[ j ] <= 0. ){
 		// ZERO OR NEGATIVE INTEGER SOURCE POINTS FOR USE AS PARTIAL MULTIPLES OF ANALYZED FUNDAMENTAL.
            PP[ j ] = (float)(int) PP[ j ] ; 	PP[ j ] = analyzedFundamental * fabs( PP[ j ] ); 
        } ; 
        if( SourceFreqAnalysisAdjustmentSwitch == 1 ){

               if( PP[ j ] < 13. ) temp = OPPC_to_Hz( PP[ j ] ) ; 
               else temp = PP[ j ] ; 
			
			sourcePointFreqSave[ toneNumber ] = temp ; 
			 
			// COMPARE temp AGAINST ALL PREVIOUS TUNED SOURCE POINTS
			freqMatchFoundFlag = 0 ; 
			if( toneNumber > 0 ){
				for( n = 0; n <= (toneNumber - 1); n++ ){
					if( temp == sourcePointFreqSave[ n ] ){
						freqMatchFoundFlag = 1 ; 
						PP[ j ] = tunedSourcePointFreqSave[ n ] ; 
						fprintf( stderr, "\nTONE %d SOURCE POINT MATCHES TONE %d -- USING TONE %d TUNING ANALYSIS . . . ",
							toneNumber, n, n ) ;
						break ;    
					} ; 
				} ;  
			} ; 

			if( freqMatchFoundFlag == 0 ) {
					//
                	prt( "USING ORIGINAL SOUND FILE AND pitchtracker TO TUNE SOURCE POINT USING STRONGEST FREQUENCY"  ) ;  
					prt("IN STRONGEST FORMANT METHOD." ) ; 
                	prs( analysis.fname, "ANALYSIS FILE" ) ; 
    					prs( afile, "ORIGINAL SOUND FILE" ) ; 
                	sprintf( tempPitchTrackName, "/tmp/pitchtrack.%s.%d", user, (int)(random()) ) ;
					filesToRemove( tempPitchTrackName, 0 ) ;			
				if( analyzedFundamental == -1.){ 
					lowFreq = temp * (4. / 5.) ; hiFreq = temp * (5. / 4.) ; 
				}else{
					lowFreq = temp - (analyzedFundamental * .333) ; hiFreq = temp + (analyzedFundamental * .333) ; 
				} ;
			
				// 1. DONE // TUNING OF SOURCE POINT
                	sprintf( tempstring2,  
					"pitchtracker -N%d -M0 -D%d -b%f -e%f -C%d -f%f -F%f -L.0 -l.0 -W0 -G-0 -T-0 -S-80 -r1000 -g1 -m1 -j.05 -J.2 -d-90 -O0 -o220 -a0. -X0 -p1 -P0  -E10 -H0.5 %s %s", 
					N, 200, (float) begint, (float) endt, channelout, lowFreq, hiFreq, afile, tempPitchTrackName ) ; 
                	system( tempstring2 ) ; 
				
				pitchTrack.fp =  crackstring( tempPitchTrackName, &pitchTrack ) ; 
	
				//prt("PITCH TRACK STATISTICS IN HERZ:" ) ; 

				findFuncStats( &pitchTrack, pitchTrack.L, &low, &hi, &range, &average, &median, &mode, 
					&standarddeviation, &sum, &begin, &end, &middle, 1, 1 ) ; 
				prt(""); prt(""); prt("");
				fclose( pitchTrack.fp ) ; 

                	fprintf( stderr, "\nTONE %d SOURCE POINT FREQUENCY OF %f ADJUSTED TO %f\tOCTAVE.PITCHCLASS: %f (%4.2f)", 
					toneNumber, temp, median, Hz_to_OPPC( median ), (0.01 * floor(100. * Hz_to_OPPC(median)))   ) ; 

                	if( PP[ j ] < 13. ) PP[ j ] = Hz_to_OPPC( median ) ; 
                	else PP[ j ] = median ; 
			
				tunedSourcePointFreqSave[ toneNumber ] = median ; 

			} ;

            } ; 
    } ; 





    for( toneNumber = 0, j = 0; toneNumber < numTones ; toneNumber++, j += NUM_PARAMETERS ){
        funcStats( &delayTime[toneNumber], &low, &hi, &avg, &length, &median ) ; 
        hi *= DELAY_TIME_SWITCH_SCALER ; 
        if( hi > maxDelayT ) maxDelayT = hi ; 
	// prf( maxDelayT, "maxDelayT" ) ; 
    } ; 

//prf( maxDelayT, "maxDelayT" ) ; 
    ringTime = maxDelayT ; 


    // MAKE THE SUM BINS
    fvec( tonesums, numTones ) ;

    prline( 105,  "-" ) ;
    prbanner( "CHORDMAPPERPLUS: DATAFILE VALUES", 105 ) ; 
    prline( 105,  "-" ) ;

 

    fprintf( stderr,  "\n*********************************************************************************************************" ) ; 
    fprintf( stderr,  "\n|Tone|Source    |Transp. Map Point    |Source Low|Partials  |Number    |Partial BW|Partials             |" ) ; 
    fprintf( stderr,  "\n|    |Hz or     |Hz or Oct.Pclass     |Partial   |Spacing   |of        |as Prop.  |Shift Factor         |" ) ; 
    fprintf( stderr,  "\n|    |Oct.Pclass|Base     |Peak       |Number    |Proportion|Partials  |of Fund.  |Base     |Peak       |" ) ;
	

    fprintf( stderr,  "\n........................................................................................................|" ) ; 
 
for( toneNumber = 0; toneNumber < numTones ; toneNumber ++ ){
    // PRINT TONE NUMBER
    fprintf( stderr,  "\n" ) ; fprintf( stderr,  "|%-4d|",  toneNumber + 1 ) ; 

    startDataIndex = 0 ; 
    endDataIndex = startDataIndex + 6 ; 
    for( k = startDataIndex; k <= endDataIndex; k++ ) {
        if( k == 1 ){
            funcStats( & transposePoint_PITCH[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
            if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
        } else if( k == 6 ){
            funcStats( & partialShift[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
            if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
        } else {
            fprintf( stderr,  "%-10.3f|", PP[ (toneNumber * NUM_PARAMETERS) + k ]  ) ;
        } ; 

    } ; 


} ; 



    fprintf( stderr,  "\n*********************************************************************************************************" ) ; 
fprintf( stderr,  "\n\n" ) ; 
    


// 9-16

fprintf( stderr,  "\n********************************************************************************************************************" ) ; 
fprintf( stderr,  "\n|Tone|Spectral             |Tone/Pitch           |Tone                 |Noise                |Noise                |" ) ;  
fprintf( stderr,  "\n|    |Stretch/Compress     |Decibel Level        |Stasis Median (in dB)|Decibel Level        |Stasis Median (in dB)|" ) ;
fprintf( stderr,  "\n|    |Base      |Peak      |Base      |Peak      |Base      |Peak      |Base      |Peak      |Base      |Peak      |" ) ;
fprintf( stderr,  "\n...................................................................................................................|" ) ; 


for( toneNumber = 0; toneNumber < numTones ; toneNumber ++ ){
    // PRINT TONE NUMBER
    fprintf( stderr,  "\n" ) ; fprintf( stderr,  "|%-4d|",  toneNumber + 1 ) ; 

    startDataIndex = 7 ; 
    endDataIndex = startDataIndex + 4 ; 
    for( k = startDataIndex; k <= endDataIndex; k++ ) {
       if( k == 7 ){
            funcStats( & spectralStretchCompress[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
             if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
       } ; 
        if( k == 8 ){
            funcStats( &toneDB[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
             if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
       } ; 
        if( k == 9 ){
            funcStats( & stasisMedianForHarmonyInDB[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
             if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
       } ; 
        if( k == 10 ){
            funcStats( &noiseDB[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
             if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
       } ; 
        if( k == 11 ){
            funcStats( & stasisMedianForNoiseInDB[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
             if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
       } ; 


    } ; 


} ; 


fprintf( stderr,  "\n********************************************************************************************************************" ) ; 
fprintf( stderr,  "\n\n" ) ; 


    
// 17-24


fprintf( stderr,  "\n**********************************************************************************************" ) ; 
fprintf( stderr,  "\n|Tone|Noise     |Noise Trans. Point   |Force (0 to 1)       |Master    |Synthetic            |" ) ; 
fprintf( stderr,  "\n|    |Switch    |Hz or Oct.Pclass     |                     |Transpose |Vibrato              |" ) ; 
fprintf( stderr,  "\n|    |(0 or 1)  |Base      |Peak      |Base      |Peak      |Switch    |Switch 0/1           |" ) ; 
fprintf( stderr,  "\n.............................................................................................|" ) ; 

for( toneNumber = 0; toneNumber < numTones ; toneNumber ++ ){
    // PRINT TONE NUMBER
    fprintf( stderr,  "\n" ) ; fprintf( stderr,  "|%-4d|",  toneNumber + 1 ) ; 

    startDataIndex = 12 ; 
    endDataIndex = startDataIndex + 4 ; 
    for( k = startDataIndex; k <= endDataIndex; k++ ) {
        if( k == 14 ){
            funcStats( & forceFactor[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
            if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
        }else if( k == 13 ){
            funcStats( & transposePoint_NOISE[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
            if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
        }else{
            if( k == 16)fprintf( stderr,  "%-10.3f           |", PP[ (toneNumber * NUM_PARAMETERS) + k ]  ) ;
            else fprintf( stderr,  "%-10.3f|", PP[ (toneNumber * NUM_PARAMETERS) + k ]  ) ;
        } ; 

    } ; 


} ; 


fprintf( stderr,  "\n**********************************************************************************************" ) ; 
fprintf( stderr,  "\n\n" ) ; 
    
// 24-29


fprintf( stderr,  "\n*********************************************************************************************************" ) ; 
fprintf( stderr,  "\n|Tone|Bandpass             |Bandpass             |Filter    |Function  |Delay Time in Seconds|Output    |" ) ; 
fprintf( stderr,  "\n|    |Center  Frequency    |Rolloff in dB        |Bus Router|Delay Time|                     |Channel   |" ) ; 
fprintf( stderr,  "\n|    |Base      |Peak      |Base      |Peak      |(0,1,2,3) |Scaler 0-1|Base      |Peak      |(0=all)   |" ) ; 
fprintf( stderr,  "\n........................................................................................................|" ) ; 

for( toneNumber = 0; toneNumber < numTones ; toneNumber ++ ){
    // PRINT TONE NUMBER
    fprintf( stderr,  "\n" ) ; fprintf( stderr,  "|%-4d|",  toneNumber + 1 ) ; 

    startDataIndex = 17 ; 
    endDataIndex = startDataIndex + 5 ; 
    for( k = startDataIndex; k <= endDataIndex; k++ ) {
        if( k == 18 ){
            funcStats( & bandpassRolloffInDbPerOctave[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
            if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
        }else if( k == 21 ){
            funcStats( & delayTime[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
            if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
        }else if( k == 17 ){
            funcStats( & bandpassCF[toneNumber], &low, &hi, &avg, &length, &median ) ; 
            fprintf( stderr,  "%-8.3f ", low  ) ; 
            if( hi != low ) fprintf( stderr,  "  %-8.3f  |", hi  ) ; else fprintf( stderr,  "      *     |" ) ;  
        }else{
            fprintf( stderr,  "%-10.3f|", PP[ (toneNumber * NUM_PARAMETERS) + k ]  ) ;
        } ; 

    } ; 


} ; 

fprintf( stderr,  "\n**********************************************************************************************************" ) ; 


//	ADD HERE ????
// 	FIND MAX OUTPUT CHANNEL NUMBER FROM PP
for( toneNumber = 0; toneNumber < numTones ; toneNumber++ ){
	if( PP[ (toneNumber * NUM_PARAMETERS) + 22 ] > maximum_Channel_Output_Number ) 
		maximum_Channel_Output_Number = PP[ (toneNumber * NUM_PARAMETERS) + 22 ] ; 
} ; 
pri( maximum_Channel_Output_Number, "maximum_Channel_Output_Number" ) ; 

// DETERMINE NUMBER OF OUTPUT CHANNELS NEEDED
if ( channelout == 0 )
	ochan = maximum_Channel_Output_Number > analysis_chan ? maximum_Channel_Output_Number : analysis_chan ;
else
	ochan =  maximum_Channel_Output_Number > 1 ? maximum_Channel_Output_Number : 1 ;

// SET FLAG TO SIGNAL OUTPUT CHANNELS NUMBER AS PRESET
channelflag = -1 ; 

// SET UP OUTPUT FILE ACCORDING TO ANALYSIS RATE AND
    outfile_setup(argc, argv) ; 



// **** SET UPS *****
    if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 200.\n\n" ) ; 
	frames_per_sec = 200 ; 
    }
    I = D = (int) ((float) R / frames_per_sec) ; 

// ******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
    if( Nw < I ){
	// INCREASE WINDOW SIZE TO ACCOMODATE INTERPOLATION
	Nw = 2 ; while( Nw <= I )Nw *= 2 ;
	prt( "\n----> INCREASING WINDOW SIZE TO ACCOMMODATE TIME RESYNTHESIS INTERPOLATION. <---" ) ;
	pri( Nw,  "NEW WINDOW SIZE" ) ; 
    }
// *********************************

// **** NEW END
 
// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
// OLD    setupfiles(argc, argv) ; 



    endchan = beginchan + ochan ; // ????

 	// MAKE SPACE FOR RANGE OF TONE CONTROL VALUES

    fvec( master_gain_in_dB_TONE_VALUES, numTones ) ; 
    fvec( master_gain_TONE_VALUES, numTones ) ; 
    fvec( tones_macro_pitch_controller_in_semitones_TONE_VALUES, numTones ) ; fvec( pmt_TONE_VALUES, numTones ) ; 

    fvec( Rate_Correlated_Tone_Control_in_dB_TONE_VALUES, numTones ) ; 
    fvec( Rate_Correlated_Noise_Control_in_dB_TONE_VALUES, numTones ) ; 




    fvec( tones_master_gain_controller_in_dB_TONE_VALUES, numTones ) ; fvec( tones_master_gain_controller_TONE_VALUES, numTones ) ; 
    fvec( tones_master_freq_shift_controller_TONE_VALUES, numTones ) ; 


    fvec( filtrate_TONE_VALUES, numTones ) ; 
    fvec( rateCorrelatedForceSuppressor_TONE_VALUES, numTones ) ; 
    fvec( rateCorrelatedMedianRaiser_TONE_VALUES, numTones ) ; 

    fvec( noise_band_decibel_limit_TONE_VALUES, numTones ) ; 
    fvec( noise_band_decibel_limit_rolloff_TONE_VALUES, numTones ) ; 


    fvec( pitchChangeExpansionDecibels_TONE_VALUES, numTones ) ; 
    fvec( frequency_change_suppression_threshold_TONE_VALUES, numTones ) ; 
    fvec( frequency_change_suppression_threshold_increase_response_time_in_seconds_TONE_VALUES, numTones ) ; 



    fvec( synthetic_Vibrato_Rate_TONE_VALUES, numTones ) ;
    fvec( synthetic_Vibrato_Randomization_Prop_TONE_VALUES, numTones ) ;
    fvec( vibValNow_TONE_VALUES, numTones ) ;     

    fvec( tonesChannelAmpSums, numTones ) ; 

    fvec( channelsSumBefore, numTones ) ; 
    fvec( channelsSumAfter, numTones ) ; 
    fvec( rescaleProp, numTones ) ; 

    fvec( tones_delayed_filttnow, numTones ) ; 

    fvec( timeWarp, numTones ) ;  
    fvec( oldTimeWarp, numTones ) ; 
    fvec( ranAmpScale, numTones ) ; 
    fvec( oldRanAmpScale, numTones ) ; 
    fvec( rateMod, numTones ) ;  
    fvec( oldRateMod, numTones ) ;  
    fvec( vibPhaseNow, numTones ) ; 





    fvec( vibratoPeriodTable, (int) vibratoPeriodTableSize + 1 ) ; 




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
    
    IR = (float) I / (float) R ; 
    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
//prf( nyquist, "nyquist" ) ; 
	// OSC BANK
	P = 1. ; obank = 1 ;  
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    midC = (220.*pow(2., (3./12.))) ; 
    fundamental = (float) R / (float) N ; 
    ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	


    // COMPUTE THE DURATION
// OLD    dur = (endt - begint) * (float) I / (float) D ; 

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
// OLD pri( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
pri( I,  "      INTERPOLATION SAMPLES (samples between resynthesis frames)" ) ; 
prline( 1,  "*" ) ; 
prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (in dB)" ) ; 
prline( 1,  "*" ) ; 
prp( &master_gain_in_dB,  "MASTER GAIN (in dB)"  ) ; 
prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 
prt( "********* SOURCE **********" ) ; 
if( sourceflag == 1){ 
	prt( "SOURCE INCLUDED" ) ; 
	prp( &SOURCE_dB,	"SOURCE: GAIN (in dB)"  ) ; 
	prp( &SOURCE_ptrans,  "SOURCE: PITCH TRANSPOSITION (in semitones)"  ) ; 
	prp( &SOURCE_harmadd,  "SOURCE: FREQUENCY SHIFT (in Hz)"  ) ; 
} else
	prt( "SOURCE EXCLUDED" ) ; 


prline( 1,  "*" ) ; 
prt( "******** TONES: **************" ) ; 
prp( &tones_master_gain_controller_in_dB, "TONES: MACRO GAIN (in dB)"  ) ; 
prp( &tones_macro_pitch_controller_in_semitones,  "TONES: MACRO PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &tones_master_freq_shift_controller,  "TONES: MACRO FREQUENCY SHIFT (in Hz)"  ) ; 

prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 

prt("****** SYNTHETIC VIBRATO *********" ) ; 
prp( &synthetic_Vibrato_Rate, "SYNTHETIC VIBRATO: RATE" ) ; 
prp( &synthetic_Vibrato_Randomization_Prop, "SYNTHETIC VIBRATO: 0-1 RANDOMIZATION PROPORTION" ) ; 

prline( 1,  "*" ) ;

prt("****** NATURAL VIBRATO ***********" ) ; 
if( Use_Specified_Time_Boundaries_0__Detected_Vibrato_Periods_1 == 0 ) prt( "AUTOMATIC VIBRATO DETECTION/RANDOMIZATION IS OFF" ) ; 
else prt( "AUTOMATIC VIBRATO DETECTION/RANDOMIZATION IS ON" ) ; 

// NEW **** BEGIN

// FILTER WINDOW BOUNDARIES
    if( filtwinhi.A[ 0 ] < 0.) filtwinhi.A[ 0 ] = analysis_dur ;

    temp2 = (float) R  / (float) I  ; 
fprintf( stderr,"\nOUTPUT DURATION: %f seconds",  dur ) ; 
fprintf( stderr,"\nOUTPUT: %d frames/sec ", (int) temp2 ) ; 
fprintf( stderr,"\nOUTPUT: %d total frames", (int) (temp2 * dur) ) ;


fprintf( stderr,"\n********** FILTER FILE ********************" ) ; 
prf( analysis_dur,	  "INPUT ANALYSIS: DURATION" ) ; 
pri( analysis_N,  "INPUT ANALYSIS: FFT SIZE" ) ; 
pri( analysis_R,  "INPUT ANALYSIS: SAMPLE RATE" ) ; 
pri( analysis_D,  "INPUT ANALYSIS: DECIMATION" ) ; 
pri( analysis_chan,  "INPUT ANALYSIS: NUMBER OF CHANNELS" ) ;
// ***** VERIFY ANALYSIS CHANNEL
    if( channelout == 0 ){
	prt( "RESYNTHESIZING ALL CHANNELS" );
    } else{
	if( (channelout - 1) >= analysis_chan ){
	    // BAD CHANNEL
	    pri(  channelout, "\nCHANNEL" );
	    prt( " IS NOT AN ALLOWED CHANNEL. WILL CHANGE TO 1.\n" ) ; 
	}
	pri( channelout, "RESYNTHESIZING  CHANNEL" );
	
    }

if( Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1 == 0 ) prt( "DATA RATE UNIT: SECONDS" ) ; 
else prt( "DATA RATE UNIT: VIBRATO PERIODS" ) ; 
prp( &filtrate,  "INPUT ANALYSIS: RATE MULTIPLIER"  ) ; 

prp( &Vibrato_Period_Durations__Mechanical_0__Natural_1, "VIBRATO PERIOD DURATIONS MECHANICAL/NATURAL FACTOR" ) ; 

prp( &filttorigin,  "INPUT ANALYSIS: TIME POINT-ORIGIN"  ) ; 
prp( &filtwinlow,  "INPUT ANALYSIS: TIME WINDOW LOWER BOUNDARY"  ) ; 
prp( &filtwinhi,  "INPUT ANALYSIS: TIME WINDOW UPPER BOUNDARY"  ) ; 


if( Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) prt( "ONSET AND RELEASE MODE: ON" ) ;
if( Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 0) prt( "ONSET AND RELEASE MODE: OFF" ) ; 
if( Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 2) prt( "ONSET ONLY MODE: ON" ) ;
// ADD ONSET ONLY

if( wrap_0_fold_1_clip_2 == 0 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: WRAP" ) ; 
if( wrap_0_fold_1_clip_2 == 1 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: FOLD" ) ; 
if( wrap_0_fold_1_clip_2 == 2 )prt( "TIME WINDOW OUT-OF-BOUNDS MODE: CLIP/LIMIT" ) ; 
if( Mode__sampler_loop_0__autostop_1 == 1 )prt( "AUTOSTOP ON" ) ; 
if( Mode__sampler_loop_0__autostop_1 == 0 )prt( "AUTOSTOP OFF" ) ; 

prp( &peakLoopSmoothTime, "PEAK LOOP SMOOTHING TIME"); 


prp( &Rate_Correlated_Tone_Control_in_dB, "RATE CORRELATED TONE LEVEL CONTROL in dB" ) ; 
prp( &Rate_Correlated_Noise_Control_in_dB, "RATE CORRELATED NOISE LEVEL CONTROL in dB" ) ; 





// NEW *** END


prline( 1,  "*" ) ; 

// ***************** 


// NEW *** BEGIN

// ***********
// SETUP FILTER CONTROL VALUES
	         // FILTER INCREMENT IN SECONDS
    filttinc =  (float) D / (float) R ; 
// ***********

 // *****************
// SET UP SOME FILTER FILE VALUES
    analysis_fundamental = nyquist / (float) (analysis_N/2) ; 
    
// *****************

// NEW *** END
    
// ****** MAKE ARRAYS
    fvec( Wanal, Nw ) ;		// analysis window
    fvec( Wsyn, Nw ) ;		// synthesis window 
    fvec( input, Nw ) ;		// input buffer 
    fvec( Hwin, Nw ) ;		// plain Hamming window 
    fvec( winput, Nw ) ;	// windowed input buffer 
    fvec( buffer, N ) ;		// FFT buffer 
    fvec( channel, (N+2) ) ;	// analysis channels 
    fvec( previous_channel, (N+2) ) ;	// analysis channels 
    fvec( output, Nw ) ;	// output buffer 
    fvec( previous_change, N+2 ) ;	// previous amp multiplier 

    fvec( channel_change_average, (N+2) ) ; 
    fvec( channel_change, (N+2) ) ; 
    fvec( previous_channel_change, (N+2) ) ; 


    fvec( analysis_lower,  analysis_N+2  ) ;	// lower analysis array 
    fvec( analysis_higher,  analysis_N+2  ) ;	// higher analysis array 

    fvec( tempChannel,  analysis_N+2  ) ; 

    fvec( thisFreqresponseFrame, analysis_N + 2 ) ; 
    fvec( static_freqresponse_averages, (analysis_N + 2) * numOfStaticFreqresponseAverages ) ; 
    fvec( ampWeightSums, numOfStaticFreqresponseAverages * (analysis_N + 2) ) ; 


    fvec( HammingWindow, HammingWindowSize ) ; 


	// ******** MAKE FREQ RESPONSE SUM OF ALL FRAMES FOR USE IN SETTING PARTIAL BANDWIDTHS. 

	fvec( channel_average, analysis_N+2 ) ; 
	filttnow = 0. ; k = 0 ; ainchan = 0 ; 
	while(  filttnow < analysis_dur ){

		for(channow = 0, ainchan = beginchan; channow < ochan; channow++, ainchan++ ){

       		makeInterpolatedFilterFrame ( 
				&analysis, 
				analysis_lower, 
				analysis_higher, 
				channel,
				iframes_per_sec, 
				(analysis_N + 2), 
				filttnow, 
				ainchan, 
				analysis_chan
        		) ; 
			for(i = 0 ; i < (analysis_N + 2); i++) 
				channel_average[i] = (k == 0) ?  channel[i] : channel_average[i] + channel[i] ;  	

			k++ ;  
		} ; 
		filttnow += (1. / (float) iframes_per_sec) ; 

	} ; 

	for(i = 0 ; i < (analysis_N + 2); i++) {
		channel_average[i] /= (float) k ; 
	} ; 
	peakAmp = 0. ; 
	for(i = 0 ; i < (analysis_N + 2); i += 2) if( channel_average[i] > peakAmp ) peakAmp  = channel_average[i] ;  	


	analysis_PeakAmpSave = peakAmp ; 
	analysis_PeakDecibelsSave = amp_to_dB( analysis_PeakAmpSave ) ;
//	fprintf( stderr, "analysis_PeakDecibelsSave: %f dB", analysis_PeakDecibelsSave ) ; 
	
//	prf( peakAmp, "channel_average  peakAmp" ) ; 

	// NORMALIZE
	if( peakAmp > 0. ) 
		for(i = 0 ; i < (analysis_N + 2) ; i += 2){
			 channel_average[i] /= peakAmp ; // prf( channel_average[i], "channel_average[i]" ) ; 
		} ; 

/*
	    // SECRET PLOT OF channel_average
    testOut = fopen( "/tmp/channel_average", "w+" ) ; 
    for(i = 0; i < (analysis_N + 2); i += 2){
	temp = amp_to_dB( channel_average[i] ) ; 
	if( temp < -96 ) temp = -96 ; 
	fwrite( &temp, sizeof(float), 1, testOut ) ; 
    }; 
	fclose( testOut ) ; 
*/
 
// ***************** 

// MAKE THRESH AMP
    threshfac = dB_to_amp( threshfacdB );	





// ************************************
// SET UP THE ARRAY POINTING AT THE BINS AND THEIR AMPLITUDES
    // FIRST TIME LOOP TO ACCESS ARRAY SIZE NEEDED
for(l = 0 ; l < 2 ; l++ ){
	mm = 0 ; numberOfBands = 0 ; 

	// LOOP FOR  TONES BELOW
	for( j = 0, tone_now = 0 ; j < (numTones*NUM_PARAMETERS) ; j += NUM_PARAMETERS,  tone_now++ ){

		// SAVE THE mm INDEX FOR ADJUSTING THE ROLLOFF DB LATER
		saved_mm = mm ; 

		// *******************
		// *** SOURCE POINT **
		// *******************

		// *** --> 0 <--- FIND THE SOURCE TONE FREQUENCY AND MAKE THE BW MULTIPLIER


		// ********** OCTAVE.POINT.PITCHCLASS
		if( (SOURCE_POINT < 13.) && (SOURCE_POINT > 0.) ){
 
			// TRANSLATE OCTAVE.PT.PITCHCLASS INTO FREQUENCY AND REPLACE IN ARRAY
			this_sourceptfundfreq = OPPC_to_Hz( SOURCE_POINT ); 

			// ********** FREQUENCY METHOD
		} else {
           	// MOVE FREQUENCY INTO TEMP
           	this_sourceptfundfreq = SOURCE_POINT ;
        	}
 

		if( l == 1 ) {
			sourceptfundfreq[ tone_now ] = this_sourceptfundfreq ; 
        	} ; 



		// MAKE BW ADDER
        	if( BANDWIDTH > 0. ){
            		bw = this_sourceptfundfreq * BANDWIDTH * .5 ;
        	}else{
            		prf( BANDWIDTH,  "BANDWIDTH" ) ; prt( "BANDWIDTH MUST BE > 0.,  BYE\n\n" ) ; EXIT_failure() ; 
        	}


        	// TEST ON LOWEST PARTIAL
        	if( LOW_PARTIAL < 1.  ){
            	prf( LOW_PARTIAL,  "LOWEST PARTIAL" ) ; prt( "LOWEST PARTIAL MUST BE >= 1,  BYE\n\n" ) ; EXIT_failure() ; 
        	}	    

        	// FIND THE ALLOWED NUMBER OF PASSMODE PARTIALS
        	// FACTOR IN SPACING AND POSITION OF LOWEST PARTIAL

        	numpasspartials = (int)((nyquist - (this_sourceptfundfreq * LOW_PARTIAL)) / 
			(this_sourceptfundfreq * fabs( PARTIAL_SPACING ))) ; 
        	//pri( numpasspartials,  "(A)NUMBER OF PASS PARTIALS FOR THIS TONE numpasspartials" ) ; 

        	//prf( NUMBER_OF_PARTIALS,  "(A) CODE: NUMBER OF PARTIALS FOR THIS TONE NUMBER_OF_PARTIALS" ) ; 

        	// IF NUMBER OF PARTIALS IS 1 OR GREATER,USE THE NUMBER SPECIFIED
        	if( ((int) NUMBER_OF_PARTIALS >= 1.)  ) numpasspartials = NUMBER_OF_PARTIALS ; 

        	// pri( numpasspartials,  "(B) NUMBER OF PARTIALS FOR THIS TONE numpasspartials" ) ; 

        	// BRANCH ON SPACING
        	if( PARTIAL_SPACING >= 1. ){
	    
            	// ************  PASSMODE SPACING == 1 or more
            	// ALREADY DONE


        	}else{

            	// ************ REJECTMODE: SPACING -2 OR LESS
            	// FIND THE TOTAL NUMBER OF PARTIALS FOR THE FUNDAMENTAL,
            	//  THEN SUBTRACT THE  NUMBER IN THE PASSMODE
            	// TOTAL PARTIALS
            	temp7 = (float) ((int)((nyquist - this_sourceptfundfreq) / this_sourceptfundfreq)) ; // nyquist

            	//pri( temp7,  "(C) TOTAL NUMBER OF PARTIALS FOR THIS TONE" ) ; 
            	numpasspartials = temp7 - numpasspartials ; 
	    	    
//            	pri( numpasspartials,  "(C) NUMBER OF PARTIALS FOR THIS TONE numpasspartials" ) ; 


            	if( numpasspartials <= 0 ){
                		// ILLEGAL NUMBER OF PASS PARTIALS (SPACING MUST BE BETWEEN -2 and 1)
                		prf( LOW_PARTIAL,  "LOWEST PARTIAL" ) ; 
                		prf( NUMBER_OF_PARTIALS,  "NUMBER OF PARTIALS" ) ; 
                		prf( PARTIAL_SPACING,  "PARTIAL SPACING" ) ; 
                		prt( "THERE ARE NO PARTIALS IN REJECT MODE,\n\tCHECK YOUR SPACING, NUMBER OF PARTIALS,  AND FIRST PARTIAL,  BYE.\n\n\n" ) ; 
                		EXIT_failure() ; 
            	}

        	} ; 

        	// *******************
        	// *** TRANSPOSE POINT
        	// *******************

        	// FIND THE SHIFT METHOD FOR EACH TONE: PITCH
        	funcStats( &transposePoint_PITCH[ tone_now ], &low, &hi, &avg, &length, &median ) ;
        	if( ((low < 13.) && (hi > 13.)) || ((hi < 13.) && (low > 13.)) ){
           	fprintf( stderr, "\n\nTRANSPOSE POINT FUNCTION CROSSES UNIT RANGE FROM FREQUENCY TO OCTAVE.PITCHCLASS.\n" ) ;  
                	EXIT_failure() ; 
        	}else{
           	this_transposeShiftMethod_PITCH = low < 13. ? 0 : 1 ;    

        	} ; 
        	if( l == 1 ) transposeShiftMethod_PITCH[ tone_now ] = this_transposeShiftMethod_PITCH ; 	


        	// FIND THE SHIFT METHOD FOR EACH TONE: NOISE
        	funcStats( &transposePoint_NOISE[ tone_now ], &low, &hi, &avg, &length, &median ) ;
        	if( ((low < 13.) && (hi > 13.)) || ((hi < 13.) && (low > 13.)) ){
           	fprintf( stderr, "\n\nTRANSPOSE POINT FUNCTION CROSSES UNIT RANGE FROM FREQUENCY TO OCTAVE.PITCHCLASS.\n" ) ;  
               	EXIT_failure() ; 
        	}else{
           	this_transposeShiftMethod_NOISE = low < 13. ? 0 : 1 ;    

        	} ; 
        	if( l == 1 ) transposeShiftMethod_NOISE[ tone_now ] = this_transposeShiftMethod_NOISE ; 	




		// MAKE SHIFT FACTOR
        	// this_sourceptfundfreq IS THE FUNDAMENTAL, temp2(freqshiftpoint_or_adder_P) IS THE FREQUENCY TO SHIFT THE FUNDAMENTAL
        	// AND SELECTED PARTIALS TO,  EXCEPT IN METHOD 1 WHERE IT IS THE AMOUNT TO ADD
        	// THAT IS THE NUMBER OF PARTIALS TO SHIFT * THE FUNDAMENTAL FREQ



        	// LOOP FOR PARTIALS
        	part_count = 1 ;
        	rejected_part_count = 0 ; 
        	current_rejected_part = (int) LOW_PARTIAL ; // FIRST PARTIAL 


        	for( k = 1; k <= numpasspartials; k++ ){

            	// SAVE THE ADDRESS OF THE BOTTOM OF THIS BAND
            	if( l == 1 )partial_Band_Begin[ numberOfBands ] = mm ; 

            	// MAKE (UNSHIFTED) PARTIAL FREQUENCY
            	if( PARTIAL_SPACING >= 1. ){
                		// SELECTED PARTIALS "PASSMODE": MAKE (UNSHIFTED) PARTIAL FREQUENCY
                		unshiftedpartialfreq = (this_sourceptfundfreq * LOW_PARTIAL) + (this_sourceptfundfreq * PARTIAL_SPACING * (float) (k - 1)) ; 

            	}else{
                		if( part_count == current_rejected_part ){
                    		part_count++ ; // SKIP THIS PARTIAL
                    		rejected_part_count++ ; // COUNT THE REJECTED PARTIAL

                    		//INCREMENT TO NEXT REJECT PARTIAL IF THERE ARE MORE TO REJECT
                    		if( ( NUMBER_OF_PARTIALS == 0. ) || (rejected_part_count < NUMBER_OF_PARTIALS ) )
                        		current_rejected_part += (int) fabs( PARTIAL_SPACING ) ; // INCREMENT
                		}

                		unshiftedpartialfreq = this_sourceptfundfreq * (float) part_count ; 
		
		
                		// INCREMENT TO NEXT PARTIAL
                		part_count++ ; 

            	}	    


            	// INDEX OF THE FREQ BIN BELOW THE PARTIAL
			thisPartialNumber = unshiftedpartialfreq / this_sourceptfundfreq ;
			thisPartialbw = bw *  ( 1.0 + (( (float) thisPartialNumber - 1.0) * 0.25)) ;
			if( thisPartialbw > (this_sourceptfundfreq * 0.5) ) thisPartialbw = this_sourceptfundfreq * 0.5 ;  

//fprintf( stderr, "\n UNSHIFTED PARTIAL FREQUENCY: %f  PARTIAL: %f  PARTIAL HALF BW IN HZ: %f ", 
//				unshiftedpartialfreq, thisPartialNumber, thisPartialbw ) ; 

            	i1 = 1 + (2 * ( rint((double)(unshiftedpartialfreq - (thisPartialbw) ) / fundamental)) ) ; 
            	// INDEX OF THE FREQ BIN ABOVE THE PARTIAL
            	i2 = 1 + (2 * ( rint((double)(unshiftedpartialfreq + (thisPartialbw)) / fundamental)) )  ;  

//fprintf( stderr, "\n analysis_N: %d  i2: %d ", analysis_N, i2 ) ; 

            	// INDEX OF THE FREQ BIN AT THE PARTIAL
            	ipartial = 1 + (2 * ( rint((double)(unshiftedpartialfreq) / fundamental)) )  ;  
			ipartial_save = ipartial ; i1_save = i1 ; i2_save = i2 ; 



			if( auto_adjust_cf_and_bw == 1 ){

				// FIND THE STRONGEST BIN AMP IN AVERAGE BETWEEN i1 and i2. 
				// MAKE IT THE NEW ipartial. FIND ITS AMP IN DB. 
				// THEN START ipartial and WORK OUT ON BOTH SIDES UNTIL AN AMP LESS THAN THE STOPBAND IS FOUND. 
				// CHANGE i1 and i2 ACCORDINGLY.
			
				peakAmp = channel_average[i1 - 1] ; ipartial = i1 ;  
				for(i = i1 + 2; i <= i2; i += 2) if( channel_average[i - 1] > peakAmp ){
					peakAmp = channel_average[i - 1] ; ipartial = i ; 
				} ;  


				peakdB = amp_to_dB( peakAmp ) ; 
				for(i = ipartial; i > i1; i -= 2)
					if( ( amp_to_dB( channel_average[i - 1] ) - peakdB) < auto_adjust_stopband_dB ) { i1 = i ; break ; } ; 
				for(i = ipartial; i < i2; i += 2)
					if( ( amp_to_dB( channel_average[i - 1] ) - peakdB) < auto_adjust_stopband_dB ) { i2 = i ; break ; } ; 

				// fprintf( stderr, "\ni1_save: %d i1: %d ipartial_save: %d ipartial: %d i2_save: %d i2: %d", 
				//			i1_save, i1, ipartial_save, ipartial, i2_save, i2 ) ; 					 
			} ; 


			if( (auto_adjust_cf_and_bw == 0) || (peakdB > partial_formant_inclusion_threshold_in_dB)  ){
    
            		// ******** NEW STUFF BEGIN
            		// LOOP FOR THE ipartial-i1 BINS (BELOW THE PARTIAL) >>>>>>>> BELOW <<<<<<<<<<<<<<<
            		for(i = i1; i < ipartial; i += 2){

                			// IN THE NEXT (ipartial-i1)*2 SPACES IN THE FILTER ARRAY 
                			// PLACE THE APPROPRIATE AMP VALUE FOR THIS TONE
	    
                			// IF IN RANGE.....
                			if((i > 0) && (i < N) ){
                    			// AMP
                    			temp6 = (float) (ipartial - i) / (float) (ipartial - i1) ; // 0-1 function index
                   			temp6 = halfcoscurve( temp6 ) ; // COSINE CURVE AMP 


                    			// THE BIN INDEX
                				if( l == 1) indexInChannelForHarmony[ mm ] = i - 1 ; // SAVE ADDRESS OF AMP LOCATION FOR BIN
	         				if( l == 1) {
		       				tone[ mm ] = tone_now ;  // THE TONE
                     				bandIndexforHarmonyBins[ mm ] = numberOfBands ; 
	         				} ; 


                				// SAVE THE UNSHIFTED PARTIAL FREQUENCY FOR TUNING OF THE PARTIAL BINS
                				if( l == 1) sourcefreq[mm] = unshiftedpartialfreq ; 

                				// THE AMPFREQ MULTIPLIERS - 111111111111111111111111111111111111111

                				if( l == 1){
		    
                    				// MAKE FINAL AMP
                    				coswindow[ mm ] = temp6 ; 

		  					// *******

	    					}

	    					mm++ ;  // 0

            			}
        			}


        			// SETUP PARTIAL	>>>>>>>>>>>>> AT PARTIAL <<<<<<<<<<<<<<<<<<<<<<
        			// IF IN RANGE.....
        			if((ipartial > 0) && (ipartial < N) ){

            			// THE BIN INDEX
            			if( l == 1) indexInChannelForHarmony[ mm ] = ipartial - 1 ; // SAVE ADDRESS OF AMP LOCATION FOR BIN
	     				if( l == 1) {
		   				tone[ mm ] = tone_now ;  // THE TONE
                 			bandIndexforHarmonyBins[ mm ] = numberOfBands ; 
	     				} ; 


            			// SAVE THE UNSHIFTED PARTIAL FREQUENCY FOR TUNING OF THE PARTIAL BINS
            			if( l == 1) sourcefreq[mm] = unshiftedpartialfreq ; 

            			// THE AMPFREQ MULTIPLIERS - 2222222222222222222222222
            			if( l == 1){
                				// MAKE FINAL AMP
                				coswindow[ mm ] = 1. ; 

		  				// *******
            			}



            			mm++ ; // 1 

        			}




        			// LOOP FOR THE ipartial-i2 BINS   >>>>>>>>>>>>>>>>>> ABOVE <<<<<<<<<<<<<<<<
        			for(i = i2; i > ipartial; i -= 2){
            			// IN THE NEXT (ipartial-i2)*2 SPACES IN THE FILTER ARRAY 
            			// PLACE THE APPROPRIATE AMP VALUE FOR THIS TONE
	    
        				// IF IN RANGE.....
        				if((i > 0) && (i < N) ){
						// AMP
						temp6 = (float) (i - ipartial) / (float) (i2 - ipartial) ; // 0-1 function index
						temp6 = halfcoscurve( temp6 ) ; // COSINE CURVE AMP 


						// THE BIN INDEX
						if( l == 1) indexInChannelForHarmony[ mm ] = i - 1 ; // SAVE ADDRESS OF AMP LOCATION FOR BIN
						if( l == 1) {
							tone[ mm ] = tone_now ;  // THE TONE
            					bandIndexforHarmonyBins[ mm ] = numberOfBands ; 
						} ; 
						// SAVE THE UNSHIFTED PARTIAL FREQUENCY FOR TUNING OF THE PARTIAL BINS
						if( l == 1) sourcefreq[mm] = unshiftedpartialfreq ; 

						// THE AMPFREQ MULTIPLIERS - 333333333333333333333333
						if( l == 1){
		    					// MAKE FINAL AMP
	    						coswindow[ mm ] = temp6 ; 

		  					// *******
            				}

            				mm++ ;  // 2 

					}
        			}

				// SAVE THE TONE NUMBER FOR THIS BAND
				if( l == 1 ) partial_Band_Channel_Output_Number[ numberOfBands ] = TONE_CHANNEL_OUTPUT_NUMBER ;  

        			// INCREMENT THE NUMBER OF BANDS
        			numberOfBands++ ; 

			}


        		// END PARTIALS LOOP
    		}


    		// IF ROLLOFF DB IS POSITIVE, THEN SHIFT THE VALUES FOR THIS TONE DOWN
    		// BY THE DIFFERENCE BETWEEN THE TOP AND BOTTOM
    		// (THIS WILL GIVE THE TOP PARTIAL OF THIS TONE THE PEAK DB 
    		// WITH A ROLLOFF DB DOWN FROM THE TOP)
    


    		if( l == 1 )synthetic_VIBRATO_SWITCH[ tone_now ] = SYNTHETIC_VIBRATO_SWITCH ; 

    		if( l == 1 ) functionDelayT_Switchscaler[ tone_now ] = DELAY_TIME_SWITCH_SCALER ; 

    		if( l == 1 ) {
			tone_Filter_Switch[ tone_now ] = 
				((int)(0.1 * TONE_FILTER_SWITCH) == 1) || ((int)(0.1 * TONE_FILTER_SWITCH) == 2) ? 1 : 0 ; 
			noise_Filter_Switch[ tone_now ] = 
				((int)(0.1 * TONE_FILTER_SWITCH) == 1) || ((int)(0.1 * TONE_FILTER_SWITCH) == 3) ? 1 : 0 ; 
//			pri( tone_now, "tone_now" ) ; 
//			pri( tone_Filter_Switch[ tone_now ], "tone_Filter_Switch[ tone_now ]" ) ; 
    		} ; 

		if( l == 1 ){
			tone_Filter_Type[ tone_now ] = (int)( 10. * ((TONE_FILTER_SWITCH * .1) - floor(TONE_FILTER_SWITCH * .1)))  ;
			if( (tone_Filter_Type[ tone_now ] < 0) || (tone_Filter_Type[ tone_now ] > 3) ){
				fprintf( stderr, "\n\nERROR:\n\t%f IS NOT AN ACCEPTED FILTER SWITCH. BYE\n", TONE_FILTER_SWITCH ) ;
				break ; 

			
			} ; 
//			pri( tone_now, "tone_now" ) ; 
//			pri( tone_Filter_Type[ tone_now ], "tone_Filter_Type[ tone_now ]" ) ; 
		} ; 

		if( l == 1 ) {
			tone_Channel_Output_Number[ tone_now ] = TONE_CHANNEL_OUTPUT_NUMBER ; 
				// HANDLED EARLIER
//			if( TONE_CHANNEL_OUTPUT_NUMBER > maximum_Channel_Output_Number ) 
//					maximum_Channel_Output_Number = TONE_CHANNEL_OUTPUT_NUMBER ; 
		} ; 

		// SAVE THE TRANSPOSITION SWITCH FOR THIS TONE DATA SET
    		if( l == 1 ){
			if( MASTER_TRANSPOSITION_SWITCH == 1. ) trans_switch[ tone_now ] = 1 ; 
			else  trans_switch[ tone_now ] = 0 ; 
    		} ; 

    		// NOISE
    		if( l == 1 ){
			noise_switch[ tone_now ] = NOISE_SWITCH ; // O OR 1

			if( noise_switch[ tone_now ] == 1 ) {
	    			// ADD NOISE BANK FOR THIS TONE
	    			noise_bank_tone_number[ number_of_noise_banks ] = tone_now ; 
                		noise_Bank_Channel_Output_Number[ number_of_noise_banks ] = TONE_CHANNEL_OUTPUT_NUMBER ; 
			} ; 
    		} ; 
    		if( NOISE_SWITCH == 1 ) number_of_noise_banks++ ;

    	}	// END LOOP FOR TONES


    
	// MAKE  ARRAYS

	if(l == 0){
		ivec( partial_Band_Begin, numberOfBands + 1 ) ; // FOR EACH PARTIAL BAND, THIS IS THE LOWEST NUMBERED BIN IN HARMONY WHERE THE PARTIAL BAND BEGINS.
		fvec( oldbandampsum, numberOfBands ) ;
          	fvec( averageBandFreq, numberOfBands ) ; 
		ivec( partial_Band_Channel_Output_Number, numberOfBands ) ; 

          	ivec( harmonyBands_IndexOfStrongestBinInBand, numberOfBands ) ; 

	    	fvec( sourcefreq, mm ) ;
	    	fvec( coswindow, mm ) ;
	    	ivec( tone, mm ) ;	// tone IS THE TONE LINE TO WHICH THIS BIN BELONGS
	    	ivec( indexInChannelForHarmony, mm ) ;
          	fvec( harmony_force_curve_indeces, mm ) ; 
          	ivec( bandIndexforHarmonyBins, mm ) ; // THE NUMBERED BAND THAT THE BIN IS PART OF.


		fvec( sourceptfundfreq, numTones ) ; 
		ivec( transposeShiftMethod_PITCH, numTones ) ; 
		ivec( transposeShiftMethod_NOISE, numTones ) ; 

		fvec( oldtunemult, numTones ) ;

		ivec( synthetic_VIBRATO_SWITCH, numTones ) ;
	    	fvec( functionDelayT_Switchscaler, numTones ) ;

	    	ivec( tone_Filter_Switch, numTones ) ;
	    	ivec( noise_Filter_Switch, numTones ) ;
		ivec( tone_Filter_Type, numTones ) ; 

		ivec( tone_Channel_Output_Number, numTones ) ; 

	    	ivec( trans_switch, numTones ) ;
	    	ivec( noise_switch, numTones ) ; // TONE LINE SWITCH FOR USE OR NOT OF NOISE
	    	ivec( noise_bank_tone_number, number_of_noise_banks ) ;  // TONE NUMBER FOR EACH OF THE NOISE BANKS. 
		ivec( noise_Bank_Channel_Output_Number, number_of_noise_banks ) ; 
	
           number_of_noise_banks = 0 ; // RESET FOR SECOND PASS RECOUNT


	} ; 


} ; 




for( toneNumber = 0; toneNumber < numTones; toneNumber++ ){
	funcStats( &bandpassRolloffInDbPerOctave[ toneNumber ], &low, &hi, &avg, &length, &median ) ; 
	if( avg == 0. ){
		tone_Filter_Switch[ toneNumber ] = 0 ; noise_Filter_Switch[ toneNumber ] = 0 ;
	} ; 
} ; 


// **********

// pri( number_of_noise_banks, "number_of_noise_banks" ) ; 

// TURN ON noiseFlag IF ONE OR MORE BANKS OF NOISE.
noiseFlag = number_of_noise_banks == 0 ?  0 : 1 ; 


    
NC = mm ; NCmult2 = NC * 2 ;   

for( bin = 0; bin < NC ; bin++ ){
//	fprintf( stderr, "tone[ %d ]: %d\n", bin, tone[ bin ] ) ;  
//	fprintf( stderr, "bandIndexforHarmonyBins[ %d ]: %d\n", bin, bandIndexforHarmonyBins[ bin ] ) ;  
//	fprintf( stderr, "harmony_force_curve_indeces[ %d ]: %f\n", bin, harmony_force_curve_indeces[ bin ] ) ;  
} ; 



partial_Band_Begin[ numberOfBands ] = NC ; // PUT THE NONEXISTENT BIN ADDRESS IN THE TOPMOST partial_Band_Begin IN ORDER TO 
								// IDENTIFY THE END OF THE TOP BAND. 


	// CHANNEL DELAYS
maxNumOfDelayFrames = 1 + (int)((maxDelayT * frames_per_sec) + 0.5) ; 
fvec( channel_delay, maxNumOfDelayFrames * (N+2) ) ; 

 


// MAKE CIRCULAR DELAY BUFFER FOR DATA TIME.
fvec( filttnow_delay_buffer, maxNumOfDelayFrames ) ; 


// MAKE CIRCULAR DELAY BUFFER FOR channel AMP SUMS.
fvec( channelAmpSum_delay_buffer, maxNumOfDelayFrames ) ; 

 


// ***
// ********** NOISE/RESIDUE
// FIND THE INDICES OF THE RESIDUE/UNUSED BINS
ivec( tempChannelNoiseBinFlags, N2 + 1 ) ; // AN ARRAY OF FLAGS FOR THE BINS IN CHANNEL TO IDENTIFY NOISE BINS

for( i = 0; i < (N2 + 1) ; i++ ) tempChannelNoiseBinFlags[i] = 1 ; // INITIALIZE ALL TO ON
// TURN OFF BIN IF BIN IS USED IN Harmony TONES. 


//	TURN OFF TONE CHANNELS IN NOISE FLAGS ARRAY
for( mm = 0; mm < NC; mm++ ) 
		tempChannelNoiseBinFlags[ indexInChannelForHarmony[ mm ] / 2 ] = 0 ;   

mm = 0 ; // COUNTER FOR BINS STILL ON. 

// COUNT THE BINS STILL ON; THEY ARE THE NOISE/REMAINDER. 
for( i = 0; i < (N2+1) ; i++ ) if( tempChannelNoiseBinFlags[ i ] == 1 ) mm++ ; 

 


NumNoiseBins = mm ; // SAVE NOISE 
NumNoiseBinsMult2 = NumNoiseBins * 2 ; 
mm = 0 ; // INDEX FOR NEXT OPEN BIN IN indexInChannelForNoise.
// MAKE ARRAY FOR INDICES IN channel FOR NOISE BIN AMP LOCATION. 

ivec( indexInChannelForNoise, NumNoiseBins ) ; // 

// LOOP FOR tempChannelNoiseBinFlags; 
// i IS FOR tempChannelNoiseBinFlags, j IS FOR channel AMP INDEX.

for( i = 0, j = 0; i < (N2 + 1) ; i++, j += 2 ) {

	// IF FLAG IS ON.....
	if( tempChannelNoiseBinFlags[ i ] == 1 ){
		// SAVE THE INDEX FOR THE NOISE BIN'S AMP LOCATION IN channel. 
	    	indexInChannelForNoise[ mm ] = j ; // AMP
	     	mm++ ; 
	}; 
}; 

// *************** ???

for(j = 0; j < 2; j++ ){
	numberOfNoiseBands = 0 ; 
	bottomBinIndex = 0 ; 
	for( i = 1; i <= NumNoiseBins - 1; i++ ){
		if( ( (indexInChannelForNoise[i] - indexInChannelForNoise[i - 1]) > 2) ||
				(i == (NumNoiseBins - 1)) ){
			if( j == 1 ){
				// TOP INDEX OF BAND
				noiseBandLowerBinIndex[ numberOfNoiseBands  ] = bottomBinIndex ; 
				topBinIndex = (i == (NumNoiseBins - 1)) ? i : i - 1 ; 
				noiseBandUpperBinIndex[ numberOfNoiseBands  ] = topBinIndex ; 

				// MAKE NEW BOTTOM FROM TOP
				bottomBinIndex = topBinIndex + 1 ; 
			} ; 
			numberOfNoiseBands = numberOfNoiseBands + 1 ; 
		} ; 
	} ; 

	if( j == 0 ) {
//		fprintf( stderr, "\nNUMBER OF NOISE BANDS: %d", numberOfNoiseBands ) ; 
		ivec( noiseBandLowerBinIndex, numberOfNoiseBands ) ; 
		ivec( noiseBandUpperBinIndex, numberOfNoiseBands ) ; 
	} ; 	
}; 

//fprintf( stderr, "\nNOISE BAND INDICES" ) ; 
//for(i = 0; i < numberOfNoiseBands; i++ )
//	fprintf( stderr, "\n %d: %d %d ", i, noiseBandLowerBinIndex[i], noiseBandUpperBinIndex[i] ) ; 

// FIND TONE BANDS
fvec( peakBinAmpAverageForToneBands, (analysis_N / 2) + 1 ) ; 
// TRANSFER AVERAGES 
for( i = 0, k = 0; i < ((analysis_N / 2) + 1); i++, k += 2) peakBinAmpAverageForToneBands[i] = channel_average[ k ] ; 

for( i = 0; i < numberOfNoiseBands; i++){
	for( j = noiseBandLowerBinIndex[ i ]; j <= noiseBandUpperBinIndex[ i ]; j++ ) 
			peakBinAmpAverageForToneBands[  indexInChannelForNoise[ j ] / 2  ] = -1. ;  
} ;

flag = 0 ; 
for( i = 0; i < ((analysis_N / 2) + 1); i++ ){
	if( peakBinAmpAverageForToneBands[ i ] == -1. ){
		// IN NOISE BAND
		if( flag == 1 ){
			// JUST LEFT TONE BAND
			// PUT PEAK VALUE IN ALL TONE BAND RANGE SLOTS.
			topBinIndex = i - 1 ; 
			for( j = bottomBinIndex ; j <= topBinIndex ; j++ ) peakBinAmpAverageForToneBands[ j ] = peakAmp ; 
		} ;
		flag = 0 ; // SET FOR NOISE
	}else{
		// IN TONE BAND
		if( flag == 0 ){
			// JUST ENTERED TONE
			bottomBinIndex = i ;
			peakAmp =  peakBinAmpAverageForToneBands[ i ] ;
		}else{
			// ALREADY IN TONE
			if( peakBinAmpAverageForToneBands[ i ] > peakAmp ) peakAmp = peakBinAmpAverageForToneBands[ i ] ; 
		} ; 
		flag = 1 ; // SET FOR TONE
		
	} ; 

	if( i == ( (analysis_N / 2) - 1) ){
			topBinIndex = i ; 
			for( j = bottomBinIndex ; j <= topBinIndex ; j++ ) peakBinAmpAverageForToneBands[ j ] = peakAmp ;
	} ; 

} ;


fvec( noiseBandDecibelLimiterThreshold, NumNoiseBins ) ; // 
fvec( noiseBandChannelPositionIndex, NumNoiseBins ) ; // 


for( k = 0 ; k < numberOfNoiseBands; k++ ){
//	fprintf( stderr, "\n" ) ; 

	bottomBinIndex = (indexInChannelForNoise[  noiseBandLowerBinIndex[ k ]  ] / 2) - 1 ; 
	topBinIndex = (indexInChannelForNoise[  noiseBandUpperBinIndex[ k ]  ] / 2) + 1 ; 
	if( bottomBinIndex < 0 ) bottomBinIndex = topBinIndex ; 	
	if( topBinIndex > (( analysis_N / 2) - 1) ) topBinIndex = bottomBinIndex ; 

	temp1 = amp_to_dB( peakBinAmpAverageForToneBands[ bottomBinIndex ] ) + analysis_PeakDecibelsSave ; 
	temp2 = amp_to_dB( peakBinAmpAverageForToneBands[ topBinIndex ] ) + analysis_PeakDecibelsSave ; 



	for( l = noiseBandLowerBinIndex[ k ], i = 0 ; l <= noiseBandUpperBinIndex[ k ]; l++, i++ ) {

		temp = (noiseBandUpperBinIndex[ k ] - noiseBandLowerBinIndex[ k ]) == 0 ?
			0.0 : ( (float) i / (float) (noiseBandUpperBinIndex[ k ] - noiseBandLowerBinIndex[ k ]) ) ;

		noiseBandDecibelLimiterThreshold[ l ] = temp1 + (temp * (temp2 - temp1)) ; 

		noiseBandChannelPositionIndex[ l ] = (1.0 - fabs( (temp * 2.0) - 1.0 )) * 
			(0.5 * (float)(noiseBandUpperBinIndex[ k ] - noiseBandLowerBinIndex[ k ])) ;

//		fprintf( stderr, "\n%d: %f dB, index: %f", i, noiseBandDecibelLimiterThreshold[ l ], noiseBandChannelPositionIndex[ l ] ) ;

	} ; 

}; 

/*
for( i = 0; i < NumNoiseBins; i++){
	fprintf( stderr, "\n%d: %f dB, index: %f", i, noiseBandDecibelLimiterThreshold[ i ], noiseBandChannelPositionIndex[ i ] ) ; 
}; 
*/


// ***************


// **

    
	// MAKE  ARRAY
fvec( harmony, (NCmult2+2) ) ; fvec( previous_harmony, (NCmult2+2) ) ;  
fvec( static_harmony, (NCmult2+2) ) ;
fvec( previous_static_harmony, (NCmult2+2) ) ;

fvec( harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand, numberOfBands * numOfStaticFreqresponseAverages ) ;




// SETUP FREQS IN static_harmony
//    for(i = 0, j = 0; i < NC; i++, j += 2 ){ 
//	static_harmony[ j + 1 ] = sourcefreq[ i ] ;
//    } ; 


if( noiseFlag == 1 ){
	fvec( noise, NumNoiseBinsMult2 * number_of_noise_banks ) ; 
	fvec( previous_noise, NumNoiseBinsMult2 * number_of_noise_banks ) ; 
	fvec( static_noise, NumNoiseBinsMult2 * number_of_noise_banks ) ; 
	// **** SETUP noise
} ; 

	
// ************************************
// *********************************************
// LOOP FOR CHANNELS
// *********************************************

for(channow = 0, outchan = beginchan; channow < ochan; channow++, outchan++ ){

pd( 0 ) ; 
	dur = funcDur = saved_dur ;

	boundariesResetExitCode = 0 ;  boundariesResetFlag = 0 ; filtwinlow_amountToChange = 0.;  filtwinhi_amountToChange = 0. ; 
	releaseSettingsForVibratoHaveBeenSetFlag = 0 ; releaseVibratoPeriodJumpFlag = 0 ;  

	// **** SET UP ANALYSIS CHANNEL
	ainchan = outchan ; 

	// ADD HERE TO MODIFY ainchan ????
	if( channelout == 0 )
		while( ainchan >= analysis_chan ) ainchan = ainchan - analysis_chan ;
	else
		ainchan = channelout - 1 ; 





	// SETUP FILTER CONTROL VALUES
	filttorigin.A[ 0 ] = fval( &filttorigin, dur, 0. );
	filttnow = filttorigin.A[ 0 ] ; // ACCUMULATED FILTER TIME POINT IN SECONDS
	oldfilttnow = filttnow ; //INITIALIZE OLD FILTER TIME POINT IN SECONDS

	prline( 69,   "=" ) ; 
	pri( (channow+1), "OUTPUT CHANNEL" ) ; 
	pri( (ainchan+1), "INPUT ANALYSIS CHANNEL" ) ; 
	
	// *****   REINITS
	frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; autostopflag = 0, ringTimeCountDown=0. ; 
	ditx = 1. ; 

	for( i = 0; i < (maxNumOfDelayFrames * (N + 2)); i++ ) channel_delay[i] = 0.  ; 
       
	// ZERO OUT DELAY BUFFER FOR DATA TIME
	for(i = 0; i < maxNumOfDelayFrames ; i++ ) filttnow_delay_buffer[ i ] = 0. ;  

	// ZERO OUT DELAY BUFFER FOR CHANNEL AMP SUM
	for(i = 0; i < maxNumOfDelayFrames ; i++ ) channelAmpSum_delay_buffer[ i ] = 0. ;  



	// **** SEED RANDOM
	srandom(1);


	makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
	in = -Nw ;
	if ( D )
		on = (in*I)/D ;
	else
		on = in ;

pd( 1 ) ; 

    // ZERO OLD BAND AMPS
    for( i = 0; i < numberOfBands; i++) oldbandampsum[i] = 0. ; 



	// STATIC FRAME

	makeStaticFreqResponseAveragesFromDataFile( 
		&analysis, 
		analysis_N + 2, 
          analysis_fundamental,
		ainchan, 
		analysis_chan,
		analysis_dur,
		analysis_lower, 
		analysis_higher,
		static_freqresponse_averages,
		numOfStaticFreqresponseAverages,
          harmony_force_curve_indeces,
          partial_Band_Begin, 
		numberOfBands,
           indexInChannelForHarmony,

		&static_freqresponse_peak_SUM_in_dB, 

		iframes_per_sec,
		(analysis_dur * 0.5),
		analysis_dur,
		0,
      	HammingWindow,
      	HammingWindowSize,

      	thisFreqresponseFrame,
      	ampWeightSums,
      	averageBandFreq,

      	harmonyBands_IndexOfStrongestBinInBand, 
      	harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand, 
      	NC
    

    ) ; 

/*
for( bin = 0; bin < NC ; bin++ ){
	fprintf( stderr, "harmony_force_curve_indeces[ %d ]: %f\n", bin, harmony_force_curve_indeces[ bin ] ) ;  
} ; 
*/

/*
	fscratch = fopen( "/tmp/static_freqresponse_averages_AMPS",  "w" ) ;     
//	for( i = 0; i < ((numOfStaticFreqresponseAverages - 1) * (analysis_N + 2)); i += 2 )
	for( i = 0; i < (1 * (analysis_N + 2)); i += 2 ){ 
		temp = amp_to_dB(static_freqresponse_averages[i]);
		if( temp < -200. ) temp = -200. ;  
		fwrite( &temp, sizeof(float), 1, fscratch );
	} ; 
	fclose( fscratch );  
*/

pd( 2 ) ; 


// *********************************************
// LOOP FOR FRAMES
// *********************************************

//prf( ringTimeCountDown, "ringTimeCountDown" ) ; 
//prf( ringTime, "ringTime" ) ; 
	while ( ((t < dur) && (autostopflag == 0)) || // FOR NON-LOOPS
		(ringTimeCountDown < ringTime) || // FOR ADDITION OF DELAYED COMPONENTS
			// FOR ONSET AND RELEASE 
		((Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) && (filttnow < analysis_dur ) && (filttnow > 0. )) ||
			( (Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 2) && (t < dur)) // FOR ONSET ONLY
	){

		if( t >= dur ) ringTimeCountDown += ( 1.0 / frames_per_sec ) ; 


		in += D ;
		on += I ;
		timenow( dur ) ;

//pri( frame_count, "frame_count" ) ; 

pd( 3 ) ; 
		// INIT OLD PEAK
		if( !frame_count ) { 
			oldpeakamp = 0. ; 
			// **** SEED RANDOM
            	srandom(1);


	        	// REDEFINE LOOP BEGIN AND END
            	if( (Mode__sampler_loop_0__autostop_1 == 0) && 
//				(Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) && 
					(vibratoDetection == 1) ){

	        		// FILTER TIME WINDOW LOWER BOUNDARY
	        		filtwinlow.L = 1. ; filtwinlow.n = 1. ; filtwinlow.A[ 0 ] = minimaTpts[ 0 ] ; 

	        		// FILTER TIME WINDOW UPPER BOUNDARY
	        		filtwinhi.L = 1. ; filtwinhi.n = 1. ; filtwinhi.A[ 0 ] = minimaTpts[ numberOfMinima - 1 ] ; 

//				prf( minimaTpts[ numberOfMinima - 1 ], "minimaTpts[ numberOfMinima - 1 ]" ) ; 

//				prf( filtwinlow.A[ 0 ], "filtwinlow.A[ 0 ]" ) ; 
//				prf( filtwinhi.A[ 0 ], "filtwinhi.A[ 0 ]" ) ; 

            	} ;   
        	} ; 

		// ZERO channel_change_average
		if( frame_count == 0 ) {
			for( i = 0; i < (N + 2) ; i++ ) channel_change_average[ i ] = 0.0 ;
		} ; 


pd( 4 ) ; 

	   	// ZERO  channelsSumBefore
        	for( toneNumber = 0; toneNumber < numTones ; toneNumber++ ) channelsSumBefore[ toneNumber ] = 0. ;

        	// ZERO  tonesChannelAmpSums
        	for( toneNumber = 0; toneNumber < numTones ; toneNumber++ ) tonesChannelAmpSums[ toneNumber ] = 0. ;


        	//  FOR EACH TONE, GET DELAY TIME *NOW* SCALED BY SWITCH/SCALER.
        	for( toneNumber = 0; toneNumber < numTones ; toneNumber++ ){
            	delayTime[toneNumber].A[ 0 ] =  fval( &delayTime[toneNumber], funcDur, t ) * functionDelayT_Switchscaler[ toneNumber ] ; 
            	delayTimes[ toneNumber] = delayTime[toneNumber].A[ 0 ] ; 
//fprintf( stderr, "DELAY TIMES: %d\t%f\n", toneNumber, delayTimes[ toneNumber] ) ; 

        	}; 

        	// MAKE ARRAY OF NEUTRAL TIME RATE SCALERS FOR SCALING THE SCALER FUNCTIONS.
        	for( toneNumber = 0; toneNumber < numTones ; toneNumber++ ) timeRateScalers[ toneNumber ] = 1. ; 

        	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, stasisMedianForHarmonyInDB ) ; // 9
        	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, forceFactor ) ; // 14 
        	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, stasisMedianForNoiseInDB ) ; // 11


//		for( toneNumber = 0; toneNumber < numTones ; toneNumber++ ) 
//		fprintf( stderr, "stasisMedianForHarmonyInDB[ toneNumber ].A[0]: %f\n", stasisMedianForHarmonyInDB[ toneNumber ].A[0] )   ;

//		prt( "a81" ) ; 
 

		// STATIC WAS HERE

        	// TRIGGER CHANGE TO TIME BOUNDARIES.
        	if( (boundariesResetExitCode == 1) && (boundariesResetFlag != -1) ) boundariesResetFlag = 1 ; 
        	if( boundariesResetExitCode == -1 ) boundariesResetFlag = -1 ; 

        	resetTriggerPoint = curve( 0.5, 1.0, t/funcDur, 2.) ; 


//		prt( "41" ); 

pd( 5 ) ; 

	  	if( (boundariesResetFlag == 1) && (vibratoDetection == 1) &&
	  		(		( (filttnow - filtwinlow.A[ 0 ]) / (filtwinhi.A[ 0 ] - filtwinlow.A[ 0 ]) ) > resetTriggerPoint	) 
	  	){
           	boundariesResetFlag =  0 ;  
			// WITHIN MINIMA LIMITS
            	i = 0; 

			while( (filttnow > minimaTpts[ i ]) && (i <= (numberOfMinima - 2)) ) i++ ; 

            	k = i ; i = i - 1 ;   

            	// RESET BOUNDARIES
                j = (int)curve( 0, i, t/funcDur, 10. ); 
//            	i = (i == j) ? j : randi( j, i ) ; 
            	i = (i == j) ? j : (int)(0.5 + randf( (float) j, (float) i )) ; 
            	filtwinlow_amountToChange = minimaTpts[ i ] - filtwinlow.A[ 0 ] ;   

//            	k = (k == (numberOfMinima - 1)) ? k : randi(k, numberOfMinima - 1 ) ; 
           	k = (k == (numberOfMinima - 1)) ? k : (int)( 0.5 + randf( (float) k, (float)( numberOfMinima - 1) )) ; 
             	filtwinhi_amountToChange = minimaTpts[ k ] - filtwinhi.A[ 0 ] ;  

			fprintf( stderr, "\n\tNEW BOUNDARIES: %f <-> %f  ", filtwinlow.A[ 0 ], filtwinhi.A[ 0 ] ) ; 
			for( j = 0; j < i; j++) fprintf( stderr, "   " ) ; 
			fprintf( stderr, "*" ) ;
			for( j = 0; j < (k - i); j++) fprintf( stderr, "-~-" ) ; 
			fprintf( stderr, "*" ) ;

 	   	} ; 



		if( filtwinlow_amountToChange != 0. ){
			temp = (1.5 * fabs(filttnow - oldfilttnow)) < fabs( filtwinlow_amountToChange ) ? 
					copysign( (1.5 * fabs(filttnow - oldfilttnow)), filtwinlow_amountToChange ) :
						filtwinlow_amountToChange ; 
			filtwinlow.A[ 0 ] += temp ; filtwinlow_amountToChange -= temp ; 
        	} ; 

        	if( filtwinhi_amountToChange != 0. ){
			temp = (1.5 * fabs(filttnow - oldfilttnow)) < fabs( filtwinhi_amountToChange ) ? 
					copysign( (1.5 * fabs(filttnow - oldfilttnow)), filtwinhi_amountToChange ) :
						filtwinhi_amountToChange ; 
			filtwinhi.A[ 0 ] += temp ; filtwinhi_amountToChange -= temp ; 
        	} ; 

pd( 6 ) ; 

//		prt( "42" ); 


        	// *************
        	// FILTER TIME ISSUES
        	Vibrato_Period_Durations__Mechanical_0__Natural_1.A[ 0 ] =  
			fval( & Vibrato_Period_Durations__Mechanical_0__Natural_1, funcDur, t );
pd( 61 ) ; 

        	vibratoPeriodDurationNow = averageVibratoPeriodDuration ; 
pd( 62 ) ; 
        	if( 
			(Use_Specified_Time_Boundaries_0__Detected_Vibrato_Periods_1 == 1) &&
			(Vibrato_Period_Durations__Mechanical_0__Natural_1.A[ 0 ] < 1.) 
		){
           	for(i = 0 ; i < numberOfMinima - 1; i++ ){
                		if( (filttnow >= minimaTpts[i]) && (filttnow < minimaTpts[i + 1]) ){ 
                			vibratoPeriodDurationNow = 
						(Vibrato_Period_Durations__Mechanical_0__Natural_1.A[ 0 ] * averageVibratoPeriodDuration) + 
							( (1. - Vibrato_Period_Durations__Mechanical_0__Natural_1.A[ 0 ]) * vibratoPeriodDurations[i] ) ; 
//					prf( t, "time" ) ; 
//					pri( i, "vib periods" ) ; 
//					prf( vibratoPeriodDurationNow, "vibratoPeriodDurationNow" ) ; 
                		} ; 
            	} ; 
		} ; 
pd( 63 ) ; 

//		pri( frame_count, "frame_count" ) ; 




        // CONSTRAIN TIME POINT TO WINDOW OR ANALYSIS. 
        	boundariesResetExitCode  = findFilterTimeAndConstrainByWindow(
	    		filttinc,
			&filttorigin, 
			&filtrate,
			Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2,
			Mode__sampler_loop_0__autostop_1, 
			&autostopflag, 
			wrap_0_fold_1_clip_2, 
			&filttnow, 
			&oldfilttnow, 
			&filtwinlow, 
			&filtwinhi,
			&dur,
			analysis_dur,
          	Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1,
          	vibratoPeriodDurationNow
        	) ; 


		// SET UP RELEASE SETTINGS IF USING VIBRATO PERIODS
		if( (boundariesResetExitCode == -1) && // IN RELEASE MODE 
			(Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1 == 1) &&
				(releaseSettingsForVibratoHaveBeenSetFlag == 0) // HAVE NOT YET ASSESSED RELEASE ISSUES. 
		){
			// WHAT ARE THE CLOSEST VIBRATO MINIMA POINTS?
			for(i = 0 ; i < numberOfMinima - 1; i++ ){
				if( (filttnow > minimaTpts[i]) && (filttnow < minimaTpts[i + 1]) ){
					lowMinimaReleaseVibratoPoint = minimaTpts[i] ; 
					highMinimaReleaseVibratoPoint = minimaTpts[i + 1] ;
				} ; 
			} ; 			
			// SET JUMP POINT
			releaseVibratoPeriodJumpPoint = minimaTpts[ numberOfMinima - 1 ] ; 
			prt( "SETTING VIBRATO RELEASE . . ." ) ; 
			releaseSettingsForVibratoHaveBeenSetFlag = 1 ; 
//			prf( lowMinimaReleaseVibratoPoint, "LOW VIBRATO MINIMA JUMP BOUNDARY" ) ; 
//			prf( highMinimaReleaseVibratoPoint, "HIGH VIBRATO MINIMA JUMP BOUNDARY" ) ;
			 
			// FIND JUMP POINT GAINSCALE LEVELS FOR BOTH LOW AND HIGH MINIMA
			findJumpPointGainScales(
				lowMinimaReleaseVibratoPoint,
				highMinimaReleaseVibratoPoint,
				releaseVibratoPeriodJumpPoint,
				&analysis,
				analysis_lower, 
				analysis_higher,
    				tempChannel,
    				iframes_per_sec,
    				analysis_N + 2,
    				ainchan,
    				analysis_chan,
				&lowMinimaReleaseVibratoPointGainScale_inDecibels,	// <-------
				&highMinimaReleaseVibratoPointGainScale_inDecibels	// <-------
			) ;
		} ;

		if( (releaseSettingsForVibratoHaveBeenSetFlag == 1) && (releaseVibratoPeriodJumpFlag == 0) ){

			// RELEASE SET BUT NO JUMP YET; TEST FOR JUMP TO RELEASE
			if( (filttnow <= lowMinimaReleaseVibratoPoint) || (filttnow >= highMinimaReleaseVibratoPoint) ){

				// TIME TO JUMP
				filttnow = releaseVibratoPeriodJumpPoint ; //  minimaClosestToEndOfTone ; 
				releaseVibratoPeriodJumpFlag = 1 ; 
				prf( t, "JUMPING AT TIME" ); 
				if( filttnow <= lowMinimaReleaseVibratoPoint )
					prf( lowMinimaReleaseVibratoPoint, "FROM VIBRATO MINIMA POINT OF" ) ; 
				else
					prf( highMinimaReleaseVibratoPoint, "FROM VIBRATO MINIMA POINT OF" ) ; 

				prf( releaseVibratoPeriodJumpPoint, "TO RELEASE STAGE POINT OF" ) ;  

			} else {

				// NO JUMP; MAKE PRE-JUMP RELEASE CROSSFADED GAINSCALE
				if( (filttnow - oldfilttnow) > 0. ){
					// USE HIGH
					crossfadeRamp = (filttnow - lastGainScaleFilterTimePoint)  / 
							(highMinimaReleaseVibratoPoint - lastGainScaleFilterTimePoint) ; 
					releaseVibratoPeriodGainscaleNow = dB_to_amp(
						(lastGainScale_inDecibels + 
							(crossfadeRamp * 
								(highMinimaReleaseVibratoPointGainScale_inDecibels - lastGainScale_inDecibels)))
					) ;
				}else{
					// USE LOW
					crossfadeRamp = (filttnow - lastGainScaleFilterTimePoint)  / 
							(lowMinimaReleaseVibratoPoint - lastGainScaleFilterTimePoint) ; 
					releaseVibratoPeriodGainscaleNow = dB_to_amp(
						(lastGainScale_inDecibels + 
							(crossfadeRamp * 
								(lowMinimaReleaseVibratoPointGainScale_inDecibels - lastGainScale_inDecibels)))
					) ;
				} ; 
//				prf( amp_to_dB( releaseVibratoPeriodGainscaleNow ), "releaseVibratoPeriodGainscaleNow in DB" ) ; 
	
			} ; 

		} ; 


// prf( filttnow, "filttnow" ) ; 
//pri( autostopflag, "autostopflag" ) ; 

pd( 7 ) ; 


//		fwrite( & filttnow, sizeof(float), 1, adata[14] ) ; 




//        	for(i = 0; i < 20; i++)fprintf( stderr, " " ) ; 
//        	for(i = 0; i < (int)(filttnow * 20.); i++ )fprintf( stderr, "*" ) ;
//        	fprintf( stderr, "\n" ) ;



        	getGlobalFunctionValues(
			filtrate_TONE_VALUES,  
			numTones, funcDur, delayTimes, timeRateScalers, 
			&filtrate 
        	) ; 
        	// TRANSLATE filtrate_TONE_VALUES  FROM PERIODS/SECOND TO SECONDS/SECOND IF SET TO VIBRATO PERIOD
        	// UNITS.
        	if( Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1 == 1 ){
            	for( i = 0; i < numTones; i++ ){
				filtrate_TONE_VALUES[i] *= averageVibratoPeriodDuration ;
//				prf( filtrate_TONE_VALUES[i], "filtrate_TONE_VALUES[i]" ) ;  
			} ; 
        	} ; 

pd( 8 ) ; 

		// MAKE FORCE SUPPRESSORS FROM RATE; INCREASE SUPPRESSION 
		// AS RATE APPROACHES 0.; NO SUPPRESSION AT 1. OR MORE. 
		// MAKE MEDIAN STASIS MEDIAN RAISER FOR BOTH
        	for( i = 0; i < numTones; i++ ){
			temp = fabs( filtrate_TONE_VALUES[ i ] ) > 1. ? 1 :  fabs( filtrate_TONE_VALUES[ i ] ) ;  
           	rateCorrelatedForceSuppressor_TONE_VALUES[ i ] = curve( 0., 1., temp, -7. ) ; 
           	rateCorrelatedMedianRaiser_TONE_VALUES[ i ] = 1. - curve( 0., 1., temp, 12. ) ; 
        	 } ; 

        	// RATE CORRELATED NOISE CONTROL
        	getGlobalFunctionValues(
			Rate_Correlated_Noise_Control_in_dB_TONE_VALUES,  // GLOBAL TONES RATE CORRELATED NOISE CONTROL
			numTones, funcDur, delayTimes, timeRateScalers, 
			&Rate_Correlated_Noise_Control_in_dB 
        	) ; 
        	// RATE CORRELATED TONE CONTROL
        	getGlobalFunctionValues(
			Rate_Correlated_Tone_Control_in_dB_TONE_VALUES,  // GLOBAL TONES RATE CORRELATED TONE CONTROL
			numTones, funcDur, delayTimes, timeRateScalers, 
			&Rate_Correlated_Tone_Control_in_dB 
        	) ; 
        	for( i = 0; i < numTones; i++ ){
			temp = fabs( filtrate_TONE_VALUES[ i ] ) > 1. ? 0 :  1. - fabs( filtrate_TONE_VALUES[ i ] ) ;
			temp = curve( 0., 1., temp, 0 ); 
		 	Rate_Correlated_Noise_Control_in_dB_TONE_VALUES[ i ] *= temp ;   
		 	Rate_Correlated_Tone_Control_in_dB_TONE_VALUES[ i ] *= temp ;   
		} ; 



		if( releaseSettingsForVibratoHaveBeenSetFlag == 1 ){
			// IN VIBRATO RELEASE STAGE -- MAKE loopSmoothTime BASED ON PRE OR POST JUMP TIMES.
			peakLoopSmoothTime.A[ 0 ] =  fval( &peakLoopSmoothTime, dur, t );
			if( releaseVibratoPeriodJumpFlag == 1 ){
				// POST JUMP
				loopSmoothTime = peakLoopSmoothTime.A[ 0 ] + (releaseVibratoPeriodJumpPoint - filttnow) ;
			} else {
				// PRE-JUMP
				if( (filttnow - oldfilttnow) > 0. ){
					// USE HIGH
					loopSmoothTime = peakLoopSmoothTime.A[ 0 ] + (filttnow  - highMinimaReleaseVibratoPoint) ; 
				}else{
					// USE LOW
					loopSmoothTime = peakLoopSmoothTime.A[ 0 ] + (lowMinimaReleaseVibratoPoint - filttnow) ; 
				} ; 
			} ; 
			if( loopSmoothTime < 0. ) loopSmoothTime = 0. ; 
//			if( loopSmoothTime > 0. )prf( loopSmoothTime, "loopSmoothTime for JumpPoint" ); 

		} else { 
       		loopSmoothTime = makeLoopSmoothTime(
	    			filttnow,
	    			wrap_0_fold_1_clip_2,
	    			Mode__sampler_loop_0__autostop_1,
	    			dur,  
	    			&filtwinlow,    
	    			&filtwinhi,
	    			&peakLoopSmoothTime
        		) ; 
		} ; 

//fprintf( stderr, 
//	"\ntime: %f, Mode__sampler_loop_0__autostop_1: %d, filttnow: %f, dur: %f, loopSmoothTime: %f  ", 
//		t, Mode__sampler_loop_0__autostop_1, filttnow, dur,  loopSmoothTime ) ; 

//prp( &peakLoopSmoothTime, "peakLoopSmoothTime" ) ; 
// *************


//		prt( "a82" ) ; 
pd( 9 ) ; 
 

// ******************************************************************************
// MAKE THE FILTER FRAME
// ******************************************************



        	makeInterpolatedFilterFrame ( 
			&analysis, 
			analysis_lower, 
			analysis_higher, 
			channel,
			iframes_per_sec, 
			(analysis_N + 2), 
			filttnow, 
			ainchan, 
			analysis_chan
        	) ; 

//		fwrite( &temp5, sizeof(float), 1, adata[12] ) ; 

//if( filttnow >= 2.0 ) {
//	temp = sumAndPrintDB( channel, (N+2), 0, 2, 0 ) ; 
//	prf( temp, "A: SUM OF channel AMPS IN DB FIRST MAKING" ) ; 
//} ; 




        // SAVE THE OLD TIME POINT
//        oldfilttnow = filttnow ; 

//		prt( "a83" ) ; 
 

		// CROSSFADE GAIN channel IF THIS IS IN THE PRE-JUMP VIBRATO RELEASE STAGE. 
		if( (releaseSettingsForVibratoHaveBeenSetFlag == 1) && (releaseVibratoPeriodJumpFlag == 0) ){
			for( i = 0; i < (analysis_N + 2); i += 2) channel[i] *= releaseVibratoPeriodGainscaleNow ; 
		} ; 		




        // NORMALIZE FOR LOOP
        // ************    
		if( releaseSettingsForVibratoHaveBeenSetFlag != 1 ){
        		lastGainScale_inDecibels = normalizeLoopAmplitudesForChordmapperplus(
				LoopNormalizationFlag,
				Mode__sampler_loop_0__autostop_1,
				&filtwinlow, 
				&filtwinhi,
				&analysis,
				analysis_lower, 
				analysis_higher,
				tempChannel,
				channel,
				iframes_per_sec,
				analysis_N + 2,
				ainchan,
				analysis_chan,
				analysis_dur, 
				filttnow
        		) ; 
			lastGainScaleFilterTimePoint = filttnow ;
		} ; 



pd( 10 ) ; 
		if( frame_count == 0 ) for( i = 0; i < (N + 2); i++) previous_channel[ i ] = channel[ i ] ; 

 
/*
		weighted_channel_change_frame_average = 0 ; 
		frameWeightsSum = 0 ; 
		peak = 0 ; 
		for( i = 0, j = 1; i < (N + 2) ; i += 2, j += 2 ){
			weighted_channel_change_frame_average += ((channel[ j ] - previous_channel[ j ]) * channel[ i ]) ; 
			frameWeightsSum += channel[ i ] ; 
			if( channel[ i ] > peak ) peak = channel[ i ] ; 
		} ; 
		weighted_channel_change_frame_average /= frameWeightsSum ; 
*/
		for( i = 0, j = 1, l = 0.; i < (N + 2) ; i += 2, j += 2, l += 1.0 ){			
			channel_change[ i ] = fabs( amp_to_dB( channel[ i ] ) - amp_to_dB( previous_channel[ i ] ) ) ;
			channel_change[ j ] = .01 * frames_per_sec * fabs( channel[ j ] - previous_channel[ j ] ) / analysis_fundamental  ;


		} ; 	// ???	
		
		if( frame_count == 0 ) {
			for( i = 0; i < (N + 2) ; i++ ) previous_channel_change[ i ] = channel_change[ i ] ;
		} ; 


//		prt( "a830" ) ; 

//		temp5 = sumAndPrintDB( channel, analysis_N, 0, 2, 0 ) ; 
//		fwrite( &temp5, sizeof(float), 1, adata[13] ) ; 



pd( 11 ) ; 


        	// PITCH CHANGE EXPANSION DECIBELS
        	getGlobalFunctionValues(
			pitchChangeExpansionDecibels_TONE_VALUES, 
			numTones, funcDur, delayTimes, timeRateScalers, 
			&pitchChangeExpansionDecibels 
        	) ; 

        	// PITCH CHANGE THRESHOLD
        	getGlobalFunctionValues(
			frequency_change_suppression_threshold_TONE_VALUES, 
			numTones, funcDur, delayTimes, timeRateScalers, 
			&frequency_change_suppression_threshold 
        	) ; 

        	// FREQUENCY CHANGE SUPPRESSION THRESHOLD INCREASE RESPONSE TIME
        	getGlobalFunctionValues(
			frequency_change_suppression_threshold_increase_response_time_in_seconds_TONE_VALUES, 
			numTones, funcDur, delayTimes, timeRateScalers, 
			&frequency_change_suppression_threshold_increase_response_time_in_seconds 
        	) ; 




		// SMOOTH channel_change
        	smooth_setup( frequency_change_suppression_threshold_increase_response_time_in_seconds.A[ 0 ], 
						&channel_change_increase_SmoothCoef, &minus_channel_change_increase_SmoothCoef, IR ) ; 

        	smooth_setup( 0.0, &channel_change_decrease_SmoothCoef, & minus_channel_change_decrease_SmoothCoef, IR ) ; 

		// 	AMPS AND FIRST SETUP
        	smooth_frequency_change( channel_change, previous_channel_change, N + 2, 
			channel_change_increase_SmoothCoef, minus_channel_change_increase_SmoothCoef, 
							channel_change_decrease_SmoothCoef, minus_channel_change_decrease_SmoothCoef ) ; 

		for( i = 0, j = 1; i < (N + 2) ; i += 2, j += 2 ){
		    	channel_change_average[ i ] += channel_change[ i ] ; 
		    	channel_change_average[ j ] += channel_change[ j ] ; 
		} ; 	// ???	


pd( 12 ) ; 




// ???	NOISE BAND CHANNEL SUPPRESSION VIA FREQ CHANGE
        	for( i = 0; i < NumNoiseBins ; i++ ){

			// FREQ 

			if( channel_change[ indexInChannelForNoise[ i ] + 1 ] < frequency_change_suppression_threshold.A[ 0 ] ){
				temp = channel_change[ indexInChannelForNoise[ i ] + 1 ] > frequency_change_suppression_threshold.A[ 0 ] ? 0. :
					1.0 - (channel_change[ indexInChannelForNoise[ i ] + 1 ] / frequency_change_suppression_threshold.A[ 0 ] ) ; 
				channel[ indexInChannelForNoise[ i ] ] *=  dB_to_amp( pitchChangeExpansionDecibels.A[ 0 ] * temp ) ; 
			} ; 

		} ; 










		//  SMOOTH HERE??
        	smooth_setup( loopSmoothTime, &loopSmoothCoef, &minusLoopSmoothCoef, IR ) ; 
		// 	AMPS AND FIRST SETUP

        	smooth( channel, previous_channel, (N + 2), loopSmoothCoef, minusLoopSmoothCoef, loopSmoothCoef, minusLoopSmoothCoef ) ; 


		// FREQS
        	smoothfreqs( channel, previous_channel, (N + 2), loopSmoothCoef, minusLoopSmoothCoef ) ; 




pd( 130 ) ; 



        	// NOISE BAND DECIBEL LIMIT CONTROL
        	getGlobalFunctionValues(
			noise_band_decibel_limit_TONE_VALUES, 
			numTones, funcDur, delayTimes, timeRateScalers, 
			& noise_band_decibel_limit 
        	) ; 
pd( 131 ) ; 

        	// NOISE BAND DECIBEL LIMIT ROLLOFF CONTROL
        	getGlobalFunctionValues(
			noise_band_decibel_limit_rolloff_TONE_VALUES, 
			numTones, funcDur, delayTimes, timeRateScalers, 
			& noise_band_decibel_limit_rolloff 
        	) ; 

pd( 132 ) ; 





		numberOfSuppressedChannels = 0 ;

        	for( i = 0; i < NumNoiseBins ; i++ ){
pd( 1330 )	;
			temp = dB_to_amp( 
				noiseBandDecibelLimiterThreshold[ i ]  + noise_band_decibel_limit.A[ 0 ] +
						( noise_band_decibel_limit_rolloff.A[ 0 ] * noiseBandChannelPositionIndex[ i ] ) 
			) ;
pd( 1331 ) ; // 

//fprintf( stderr, "\ni: %d	indexInChannelForNoise[ i ]: %d	N: %d", i, indexInChannelForNoise[ i ], N ) ; 

			if( channel[ indexInChannelForNoise[ i ] ] >= temp ){ 
					channel[ indexInChannelForNoise[ i ] ] = temp ; 
					numberOfSuppressedChannels += 1 ;  
			} ; 
			

        	} ; 
//		if( numberOfSuppressedChannels > 0 ) 
//			fprintf( stderr, "\nNUMBER OF SUPPRESSED NOISE BANK CHANNELS:  %d", numberOfSuppressedChannels ) ; 

// ???

pd( 14 ) ; 

		// MODIFY NOISE BINS IN CORRELATION WITH RATE
		// RATE CORRELATED NOISE/RESIDUE RANDOMIZATION
       	if( Rate_Correlated_Randomization_Switch == 1 ){
            	      temp = (Data_Time_Rate_Units__Seconds_0__Vibrato_periods_1 == 1) ? filtrate.A[ 0 ] * vibratoPeriodDurationNow : 
						filtrate.A[ 0 ] ;  
			temp = fabs( temp ) > 1. ? 0 :  1. - fabs( temp ) ; // WHEN TIME >= 1, temp = 0 ; WHEN TIME == 0, temp = 1 ;   




        		for( i = 0; i < NumNoiseBins ; i++ ){

				Rate_Correlated_Noise_Amp_Randomization_Proportion = 
						dB_to_amp( curve( 0., -6, temp, 1. ) * randf(0., 1.) ) ;
				Rate_Correlated_Noise_Freq_Randomization_Proportion = 
						curve( 0., analysis_fundamental * 2., temp, 1. ) * randf(-1., 1.) ;


	      		channel[ indexInChannelForNoise[ i ] ] *= Rate_Correlated_Noise_Amp_Randomization_Proportion ;
	      		channel[ indexInChannelForNoise[ i ] + 1 ] += Rate_Correlated_Noise_Freq_Randomization_Proportion ;
        		} ; 
		} ; 


//		prt( "a831" ) ; 



        	// INSERT channel (ALL BINS) INTO channel_delay CIRCULAR DELAY LINE FOR ALL BINS @@@
        	frameNowChannelDelayIndex = frame_count ; 
        	while( frameNowChannelDelayIndex >= maxNumOfDelayFrames ) frameNowChannelDelayIndex -=  maxNumOfDelayFrames ; 
        	for( ampIndex = 0, freqIndex = 1; ampIndex < (N+2) ; ampIndex += 2, freqIndex += 2 ){
			channel_delay[ (frameNowChannelDelayIndex * (N+2)) + ampIndex ] = channel[ ampIndex ] ;
			channel_delay[ (frameNowChannelDelayIndex * (N+2)) + freqIndex ] = channel[ freqIndex ] ;
        	} ; 



//		prt( "a832" ) ; 


pd( 15 ) ; 

        	// INSERT filttnow INTO DATA TIME DELAY BUFFER
        	filttnow_delay_buffer[ frameNowChannelDelayIndex ] = filttnow ; 

        	// FIND channel AMP SUM
		thisChannelAmpSum = 0. ; 
        	for( i = 0; i < (N + 2); i += 2) thisChannelAmpSum += channel[ i ] ; 

        	// INSERT thisChannelAmpSum INTO channelAmpSum_delay_buffer 
        	channelAmpSum_delay_buffer[ frameNowChannelDelayIndex ] = thisChannelAmpSum ; 


        	// FIND DELAYED DATA TIME AND CHANNEL AMP SUM FOR EACH TONE 1
        	for( toneNumber = 0; toneNumber < numTones; toneNumber++ ){ 
            	thisFrameDelay = (int) floor((delayTimes[ toneNumber ] * frames_per_sec) + 0.5) ;
            	thisFrameDelay = (frameNowChannelDelayIndex - thisFrameDelay) ; 
            	while( thisFrameDelay < 0) thisFrameDelay += maxNumOfDelayFrames ;
 
            	tones_delayed_filttnow[ toneNumber ] = filttnow_delay_buffer[ thisFrameDelay ] ; 
            	tonesChannelAmpSums[ toneNumber ] = channelAmpSum_delay_buffer[ thisFrameDelay ] ; // @@@
	  	} ; 

		// INSERT STATIC STUFF
        	// ******** STATIC RESPONSE

        	// PREPARE STATIC SINES/HARMONY FREQ RESPONSE FOR THIS CHANNEL BY MAKING AN INTERPOLATED VALUE 
        	// FOR EACH BIN (AND ALL TONES) BASED ON THE MEDIAN STASIS INDEX AND THE PEAK AND AVERAGE VALUES.

 //		prt( "a833" ) ; 
       

        	sumOfNonSuppressedStaticAmps = 0. ; sumOfStaticAmps = 0. ; 
 
		// FOR EVERY BIN @@@
        	for(bin = 0, doubleBin = 0; bin < NC; bin++, doubleBin += 2 ){ 
            	toneNumber = tone[bin] ; 
            	band = bandIndexforHarmonyBins[bin] ;
 

			// IF THIS IS FIRST FRAME, OR WE HAVE A FUNCTION FOR THE MEDIAN, ARE IN ONSET OR RELEASE STAGE, OR
			// CORRELATING FORCE WITH RATE 
 
//prf( forceFactor[ toneNumber].A[0], "forceFactor[ toneNumber].A[0]" ) ;                
            	// CORRELATING FORCE AND RATE: MAKE this_Stasis_Median_harmony. ###
            	if( rateCorrelatedForceSuppressionSwitch == 1 ){
 		     		thisForce = forceFactor[ toneNumber].A[0] > 1. ? 1. : forceFactor[ toneNumber].A[0] ; // FORCE
                  	forceMedianDifference = 1.0 * ((stasisMedianForHarmonyInDB[ toneNumber ].A[0] * (1. - thisForce)) - 
				     stasisMedianForHarmonyInDB[ toneNumber ].A[0]) ; // 
 
//fprintf( stderr, "thisForce: %f, forceMedianDifference: %f, stasisMedianForHarmonyInDB[ toneNumber ].A[0]: %f, 
//rateCorrelatedMedianRaiser_TONE_VALUES[ toneNumber ]: %f", 
//thisForce, forceMedianDifference, stasisMedianForHarmonyInDB[ toneNumber ].A[0], 
//rateCorrelatedMedianRaiser_TONE_VALUES[ toneNumber ] ); 

                 	this_Stasis_Median_harmony = stasisMedianForHarmonyInDB[ toneNumber ].A[0] + 
				     ((rateCorrelatedMedianRaiser_TONE_VALUES[ toneNumber ]) * forceMedianDifference) ;  
//	prf( this_Stasis_Median_harmony, "0 this_Stasis_Median_harmony" );  
           	}else{
		         	this_Stasis_Median_harmony = stasisMedianForHarmonyInDB[ toneNumber ].A[0] ; 
//	prf( this_Stasis_Median_harmony, "1 this_Stasis_Median_harmony" );  
            	} ; 

pd( 16 ) ; 

            	// IF IN ONSET/RELEASE MODE, THEN MODIFY this_Stasis_Median_harmony IN ONSET OR RELEASE STAGE.
            	if( (Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) ||
					(Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 2) ){ // EXCLUDE RELEASE FOR ONSET ONLY
 
               	if( tones_delayed_filttnow[ toneNumber ] < filtwinlow.A[ 0 ] ){ 
		    			// ONSET
                	 	tProp = tones_delayed_filttnow[ toneNumber ] / filtwinlow.A[ 0 ] ; // 0-1
                	     	env =  curve( 0.0, 1.0, tProp, -2 ) ; // 0-1
                      temp = amp_to_dB( tonesChannelAmpSums[ toneNumber ] ) - static_freqresponse_peak_SUM_in_dB ; 
                      if( this_Stasis_Median_harmony > temp ) 
                      		this_Stasis_Median_harmony = ((1. - env) * temp) + (env * this_Stasis_Median_harmony) ; 
				} ; 

                	if( (tones_delayed_filttnow[ toneNumber ] > filtwinhi.A[ 0 ]) &&
								(Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) ){ 
                		// RELEASE HERE for this_Stasis_Median_harmony
                 		tProp = (tones_delayed_filttnow[ toneNumber ] - filtwinhi.A[ 0 ]) / 
						(analysis_dur - filtwinhi.A[ 0 ]) ; // 0-1
                		env =  curve( 1.0, 0.0, tProp, 2 ) ; // 
                 		temp = amp_to_dB( tonesChannelAmpSums[ toneNumber ] ) - static_freqresponse_peak_SUM_in_dB ; 
                		if( this_Stasis_Median_harmony > temp ) 
                			this_Stasis_Median_harmony = ((1. - env) * temp) + (env * this_Stasis_Median_harmony) ; 

				}; 
			} ; 

pd( 170 ) ; 

//		fwrite( &this_Stasis_Median_harmony, sizeof(float), 1, adata[0] ) ; 

// 		MAKE this_Stasis_Median_harmony_for_Static_Harmony_Index FROM this_Stasis_Median_harmony.

//		prt( "a834" ) ; 

pd( 171 ) ;
		this_Stasis_Median_harmony_for_Static_Harmony_Index = -1. *  this_Stasis_Median_harmony ; 


           // MAKE realMedianIndex FROM this_Stasis_Median_harmony_for_Static_Harmony_Index.
pd( 172 ) ;
		if( this_Stasis_Median_harmony_for_Static_Harmony_Index < 0. ) realMedianIndex = 0. ; 
           else if( this_Stasis_Median_harmony_for_Static_Harmony_Index > (float)(numOfStaticFreqresponseAverages - 2) )
			realMedianIndex = (float)(numOfStaticFreqresponseAverages - 2) ; 
		else
			realMedianIndex = this_Stasis_Median_harmony_for_Static_Harmony_Index ;

pd( 173 ) ;
//		prt( "a835" ) ; 


//		if( (boundariesResetFlag == -1) && (bin == 0.) ) 
//			fprintf( stderr, "time: %f\t realMedianIndex: %f\n", t, realMedianIndex ) ; 

//		fwrite( &realMedianIndex, sizeof(float), 1, adata[1] ) ; 

//		prt( "a8350" ) ; 

//		prf( realMedianIndex, "realMedianIndex" ) ; 
pd( 174 ) ;
			
		upperProp = realMedianIndex - floor( realMedianIndex )  ; // FRACTION OF REAL
		lowerProp = 1.0 - upperProp ; 
//prf( lowerProp, "lowerProp" ) ; 
//prf( upperProp, "upperProp" ) ; 
		lowerMedianIndex =  ((int) realMedianIndex) * numberOfBands ; 
		upperMedianIndex = lowerMedianIndex + numberOfBands ; 
pd( 175 ) ;
//	fprintf( stderr, "\nupperProp: %f, lowerProp: %f, lowerMedianIndex: %d, upperMedianIndex: %d, band: %d, doubleBin: %d", 
//				upperProp, lowerProp, lowerMedianIndex, upperMedianIndex, band, doubleBin ) ;  

			// AMP
//		prt( "a8351" ) ; 

pd( 176 ) ;
 
		if( bin == harmonyBands_IndexOfStrongestBinInBand[ band ] ){
pd( 177 ) ;
//		prt( "a83510" ) ; 
//pri( doubleBin, "doubleBin" ) ; 
//pri( lowerMedianIndex, "lowerMedianIndex" ) ;
//pri( upperMedianIndex, "upperMedianIndex" ) ; 
//pri( band, "band" ) ; 
//pri( numberOfBands, "numberOfBands" ); 
//pri( numOfStaticFreqresponseAverages, "numOfStaticFreqresponseAverages" ); 
//pri( (numberOfBands * numOfStaticFreqresponseAverages), 
//	"Size of harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand (numberOfBands * numOfStaticFreqresponseAverages)" ) ; 
//pri( lowerMedianIndex + band, "lowerMedianIndex + band" ) ; 
//pri( upperMedianIndex + band, "upperMedianIndex + band" ) ; 

//pri( (NCmult2+2), "static_harmony size"); 


		     // STRONGEST BIN IN BAND

pd( 1770 ) ;
			temp = harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand[ lowerMedianIndex + band ] ; 
pd( 1771 ) ;
			temp1 = harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand[ upperMedianIndex + band ] ;
pd( 1772 ) ;
		    static_harmony[ doubleBin ] = (lowerProp * temp) + (upperProp * temp1) ; 
pd( 178 ) ;
		} else {
//		prt( "a83511" ) ; 
pd( 179 ) ;

		     // OTHER SUPPRESSED BINS
		    static_harmony[ doubleBin ] = 0. ; 
pd( 1790 ) ;
		} ; 
pd( 18 ) ; 

//		prt( "a8352" ) ; 

//		fprintf( stderr, "time: %f\t static_harmony[ doubleBin ] dB: %f\n", t, amp_to_dB( static_harmony[ doubleBin ] ) ) ;                 

//		prt( "a836" ) ; 

		lowerMedianIndex =  ((int) realMedianIndex) * (analysis_N + 2) ; 
		upperMedianIndex = lowerMedianIndex + (analysis_N + 2) ; 
		sumOfNonSuppressedStaticAmps += (  
		    (lowerProp * static_freqresponse_averages[ lowerMedianIndex + indexInChannelForHarmony[ bin ] ]) + 
		    (upperProp * static_freqresponse_averages[ upperMedianIndex + indexInChannelForHarmony[ bin ] ]) 
		) ; 

		sumOfStaticAmps += static_harmony[ doubleBin ] ; 
//		prt( "a8353" ) ; 


			// FREQ
		static_harmony[ doubleBin + 1 ] = sourcefreq[ bin ] ; 


	} ; 

//		prt( "a8354" ) ; 

//	fwrite( & static_harmony[ 0 ], sizeof(float), 1, adata[5] ) ; 
//	fwrite( & static_harmony[ 2 ], sizeof(float), 1, adata[6] ) ; 




	// RESCALE STATIC AMPS AGAINST NON-SUPPRESSED VALUES TO MATCH LEVEL
	if( sumOfStaticAmps > 0. ){
		temp = sumOfNonSuppressedStaticAmps / sumOfStaticAmps ; 
           for(bin = 0, doubleBin = 0; bin < NC; bin++, doubleBin += 2 ) {
           	static_harmony[ doubleBin ] *= temp ;  
           } ; 
	} ; 

//	fwrite( & static_harmony[0], sizeof(float), 1, adata[7] ) ; 
//	fwrite( & static_harmony[2], sizeof(float), 1, adata[8] ) ; 

//		prt( "a837" ) ; 

	if( noiseFlag == 1 ){
	    	    
		for( noiseBank = 0; noiseBank < number_of_noise_banks; noiseBank++ ){

			toneNumber = noise_bank_tone_number[ noiseBank ] ;

               
			if( rateCorrelatedForceSuppressionSwitch == 1 ){ // ###
				thisForce = forceFactor[ toneNumber].A[0] > 1. ? 1. : forceFactor[ toneNumber].A[0] ; 
			     forceMedianDifference = 1.0 * ((stasisMedianForNoiseInDB[ toneNumber ].A[0] * (1. - thisForce)) - 
				     stasisMedianForNoiseInDB[ toneNumber ].A[0]) ; 
			     this_Stasis_Median_noise = stasisMedianForNoiseInDB[ toneNumber ].A[0] + 
				     ((rateCorrelatedMedianRaiser_TONE_VALUES[ toneNumber ]) * forceMedianDifference) ;  
			}else{
                	this_Stasis_Median_noise = stasisMedianForNoiseInDB[ toneNumber ].A[0] ; 
              } ; 


              // IF IN ONSET/RELEASE MODE, THEN MODIFY NOISE STASIS MEDIAN IF IN ONSET OR RELEASE STAGE.
              if( (Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) ||
					(Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 2) ){ // EXCLUDE RELEASE FOR ONSET ONLY
                	if( tones_delayed_filttnow[ toneNumber ] < filtwinlow.A[ 0 ] ){ 

		         		// ONSET
                		tProp = tones_delayed_filttnow[ toneNumber ] / filtwinlow.A[ 0 ] ; // 0-1
                		env =  curve( 0.0, 1.0, tProp, 0 ) ; // 1-0
                  		temp = amp_to_dB( tonesChannelAmpSums[ toneNumber ] ) - static_freqresponse_peak_SUM_in_dB ; 
                  		if( this_Stasis_Median_noise > temp ) 
                  			this_Stasis_Median_noise = ((1. - env) * temp) + (env * this_Stasis_Median_noise) ; 

                  }else if( 
					(tones_delayed_filttnow[ toneNumber ] > filtwinhi.A[ 0 ]) &&
						(Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) ){ // ADD EXCLUSION HERE FOR ONSET ONLY

                  		// RELEASE HERE for this_Stasis_Median_noise
                       tProp = (tones_delayed_filttnow[ toneNumber ] - filtwinhi.A[ 0 ]) / 
							(analysis_dur - filtwinhi.A[ 0 ]) ; // 0-1
                       env =  curve( 1.0, 0.0, tProp, -0 ) ; // 
                       temp = amp_to_dB( tonesChannelAmpSums[ toneNumber ] ) - static_freqresponse_peak_SUM_in_dB ; 
                       if( this_Stasis_Median_noise > temp ) 
                       	this_Stasis_Median_noise = ((1. - env) * temp) + (env * this_Stasis_Median_noise) ; 

                 	} ; 
        		} ; 



           	this_Stasis_Median_noise = -1. * this_Stasis_Median_noise ; 

             	if( this_Stasis_Median_noise < 0. ) realMedianIndex = 0. ; 
              	else if( this_Stasis_Median_noise > (float)(numOfStaticFreqresponseAverages - 1) )
		     		realMedianIndex = (float)(numOfStaticFreqresponseAverages - 1) ; 
              	else
		    		realMedianIndex = this_Stasis_Median_noise ;

              	upperProp = realMedianIndex - floor( realMedianIndex )  ; 
             	lowerProp = 1.0 - upperProp ; 
              	lowerMedianIndex =  ((int) realMedianIndex) * (analysis_N + 2) ; 
              	upperMedianIndex = lowerMedianIndex + (analysis_N + 2) ; 

                	for(i = 0, j = noiseBank * NumNoiseBinsMult2; i < NumNoiseBins; i++, j += 2 ){
				// AMP
                   	static_noise[ j ] = 
                            (lowerProp * static_freqresponse_averages[ lowerMedianIndex + indexInChannelForNoise[ i ] ]) + 
                            (upperProp * static_freqresponse_averages[ upperMedianIndex + indexInChannelForNoise[ i ] ]) ; 
			 	// FREQ
                    	static_noise[ j + 1 ] = 
                            (lowerProp * static_freqresponse_averages[ lowerMedianIndex + indexInChannelForNoise[ i ] + 1 ]) + 
                            (upperProp * static_freqresponse_averages[ upperMedianIndex + indexInChannelForNoise[ i ] + 1 ]) ; 


             	} ; 

  		} ;  
	} ;

	// *** END STATIC HARMONY AND NOISE


//	prt( "a85" ) ; 

	// *************

 	// VIBRATO
  	getGlobalFunctionValues(
		synthetic_Vibrato_Rate_TONE_VALUES,  
		numTones, funcDur, delayTimes, timeRateScalers, 
		&synthetic_Vibrato_Rate 
  	) ; 

//	prt( "a850" ) ; 
 

	getGlobalFunctionValues(
		synthetic_Vibrato_Randomization_Prop_TONE_VALUES,  
		numTones, funcDur, delayTimes, timeRateScalers, 
		&synthetic_Vibrato_Randomization_Prop
    	) ; 

//	prt( "a851" ) ; 
 


	getVibratoValuesAndIncrement( 
		vibValNow_TONE_VALUES, 
		numTones, 
		synthetic_Vibrato_Rate_TONE_VALUES, 
		synthetic_Vibrato_Randomization_Prop_TONE_VALUES, 
		1, 
           timeWarp,
           oldTimeWarp,
           ranAmpScale,
           oldRanAmpScale,
           rateMod, 
           oldRateMod, 
           vibPhaseNow,
           vibratoPeriodTable,
           vibratoPeriodTableSize
  	) ; 


//	prt( "a852" ) ; 
 

	// FADE IN OR OUT, RESPECTIVELY,  FROM OR TO A VALUE OF 1.0, THE VIBRATO DURING THE 
	// ONSET OR RELEASE SEGMENTS FOR THE TIME IN EACH TONE; AS A MULTIPLIER AGAINST FORCE, THIS
	// WILL ALLOW THE PREVIOUSLY DESIGNED ONSET AND RELEASE STAGES TO ENVELOPE IN OR OUT
	// WITHOUT A TINY BUMP.

// I THINK THIS SHOULD ONLY BE NEEDED IF THE SYNTHETIC VIBRATO IS IN USE SINCE vibValNow_TONE_VALUES IS
// USED ONLY FORWARD CODE BASED ON IT. CHECK THIS OUT. (NO HARM OTHERWISE, I THINK.)

  	if( (Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) ||
					(Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 2) ){ // EXCLUDE RELEASE FOR ONSET ONLY
    		for(toneNumber = 0; toneNumber < numTones; toneNumber++ ){
		// ONSET
			if( tones_delayed_filttnow[ toneNumber ] < filtwinlow.A[ 0 ] ){ 
				tProp = tones_delayed_filttnow[ toneNumber ] / filtwinlow.A[ 0 ] ; // 0-1
                	env =  curve( 0.0, 1.0, tProp, -2 ) ; // 0-1
				vibValNow_TONE_VALUES[ toneNumber ] = (1.0 - env) + (env * vibValNow_TONE_VALUES[ toneNumber ]) ; 
              } ; 
			// RELEASE HERE for vibValNow_TONE_VALUES[ toneNumber ]
              if( (tones_delayed_filttnow[ toneNumber ] > filtwinhi.A[ 0 ]) &&
								(Onset_and_Release_Segment_Mode__Off_0__On_1__Onset_Only_2 == 1) ){ 
               	tProp = (tones_delayed_filttnow[ toneNumber ] - filtwinhi.A[ 0 ]) / 
						(analysis_dur - filtwinhi.A[ 0 ]) ; // 0-1
                   env =  curve( 1.0, 0.0, tProp, 2 ) ; // 
	         		vibValNow_TONE_VALUES[ toneNumber ] = (1.0 - env) + (env * vibValNow_TONE_VALUES[ toneNumber ]) ; 

            	} ; 
       	} ; 

  	} ; 

	

 


   	// *************************
    	// GET THE VALUES
     	// *************************


    	//  SHIFT, GAIN, AND MULTIPLIER

    	getGlobalFunctionValues(
		master_gain_in_dB_TONE_VALUES,  
		numTones, funcDur, delayTimes, timeRateScalers, 
		&master_gain_in_dB 
   	) ; 
    	for( i = 0; i < numTones; i++ ) master_gain_TONE_VALUES[ i ] = dB_to_amp( master_gain_in_dB_TONE_VALUES[ i ] ) ; 

    	master_gain_in_dB.A[ 0 ] =  fval( &master_gain_in_dB, funcDur, t );
    	master_gain_no_delay_for_source = dB_to_amp( master_gain_in_dB.A[ 0 ] ) ; 

    	// CONTROLLERS
    	getGlobalFunctionValues(
		tones_master_freq_shift_controller_TONE_VALUES,  // GLOBAL TONES FREQ SHIFT
		numTones, funcDur, delayTimes, timeRateScalers, 
		&tones_master_freq_shift_controller 
   	) ; 

     	getGlobalFunctionValues(
		tones_macro_pitch_controller_in_semitones_TONE_VALUES,  // GLOBAL TONES PITCH TRANSPOSE
		numTones, funcDur, delayTimes, timeRateScalers, 
		&tones_macro_pitch_controller_in_semitones 
     	) ; 
   	for( i = 0; i < numTones; i++ ) pmt_TONE_VALUES[i] = 
     		semitones_to_mult( tones_macro_pitch_controller_in_semitones_TONE_VALUES[ i ] ) ;

  	// MASTER GAIN FOR TONES
	getGlobalFunctionValues(
		tones_master_gain_controller_in_dB_TONE_VALUES,  
		numTones, funcDur, delayTimes, timeRateScalers, 
		&tones_master_gain_controller_in_dB
   	) ; 



//	for( i = 0; i < numTones; i++ )
//		fprintf( stderr, "noise_band_decibel_limit_TONE_VALUES[ %d ]: %f", i, noise_band_decibel_limit_TONE_VALUES[i] ) ; 





	for( i = 0; i < numTones; i++) tones_master_gain_controller_TONE_VALUES[ i ] = 
		dB_to_amp( tones_master_gain_controller_in_dB_TONE_VALUES[ i ] ) ; 





	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, transposePoint_PITCH ) ; // 1
	for( i = 0; i < numTones; i++) transposePoint_PITCH_values[i] = 
		transposeShiftMethod_PITCH[i] == 0 ? 
                   OPPC_to_Hz( transposePoint_PITCH[i].A[0] ) : transposePoint_PITCH[i].A[0] ; 
//	for( i = 0; i < numTones; i++) prf( transposePoint_PITCH[ i ].A[ 0 ], "transposePoint_PITCH[ i ].A[ 0 ]"); 

	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, transposePoint_NOISE ) ; // 13 
	for( i = 0; i < numTones; i++) transposePoint_NOISE_values[i] = 
		transposeShiftMethod_NOISE[i] == 0 ? 
                   OPPC_to_Hz( transposePoint_NOISE[i].A[0] ) : transposePoint_NOISE[i].A[0] ; 
//	for( i = 0; i < numTones; i++) prf( transposePoint_NOISE[ i ].A[ 0 ], "transposePoint_NOISE[ i ].A[ 0 ]"); 


	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, partialShift ) ; // 6 
//	for( i = 0; i < numTones; i++) prf( partialShift[ i ].A[ 0 ], "partialShift[ i ].A[ 0 ]"); 
        	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, spectralStretchCompress ) ; // 7
//	for( i = 0; i < numTones; i++) prf( spectralStretchCompress[ i ].A[ 0 ], "spectralStretchCompress[ i ].A[ 0 ]"); 

	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, toneDB ) ; // 8
//	for( i = 0; i < numTones; i++) prf( toneDB[ i ].A[ 0 ], "toneDB[ i ].A[ 0 ]"); 
//   	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, stasisMedianForHarmonyInDB ) ; // 9
//	for( i = 0; i < numTones; i++) prf( stasisMedianForHarmonyInDB[ i ].A[ 0 ], "stasisMedianForHarmonyInDB[ i ].A[ 0 ]"); 
     	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, noiseDB ) ; // 10 
//	for( i = 0; i < numTones; i++) prf( noiseDB[ i ].A[ 0 ], "noiseDB[ i ].A[ 0 ]"); 
//   	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, stasisMedianForNoiseInDB ) ; // 11 ???
//	for( i = 0; i < numTones; i++) prf( stasisMedianForNoiseInDB[ i ].A[ 0 ], "stasisMedianForNoiseInDB[ i ].A[ 0 ]"); 
//   		setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, forceFactor ) ; // 14 ???
//	for( i = 0; i < numTones; i++) prf( forceFactor[ i ].A[ 0 ], "forceFactor[ i ].A[ 0 ]"); 
     	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, bandpassCF ) ; // 17
//	for( i = 0; i < numTones; i++) prf( bandpassCF[ i ].A[ 0 ], "bandpassCF[ i ].A[ 0 ]"); 
     	setupToneBankOrPartialFunctionValues( numTones, funcDur, delayTimes, timeRateScalers, bandpassRolloffInDbPerOctave ) ; // 18
//	for( i = 0; i < numTones; i++) prf( bandpassRolloffInDbPerOctave[ i ].A[ 0 ], "bandpassRolloffInDbPerOctave[ i ].A[ 0 ]"); 



// *******************

//		prt( "a86" ) ; 
 


//	fwrite( &tones_master_gain_controller_TONE_VALUES[0], sizeof(float), 1, adata[5] ) ; 
//	fwrite( & master_gain_TONE_VALUES[0], sizeof(float), 1, adata[6] ) ; 
//	fwrite( & Rate_Correlated_Tone_Control_in_dB_TONE_VALUES[0], sizeof(float), 1, adata[7] ) ; 
//	fwrite( & master_gain_TONE_VALUES[0], sizeof(float), 1, adata[8] ) ; 




	// ****** TRANSFER DELAYED channel BANKS INTO MULTI-BANK NOISE/REMAINDER ARRAY FROM APPROPRIATE DELAY POINT

	if( noiseFlag == 1 ){


//		prt( "a860" ) ; 
 

		 

		for( noiseBank = 0; noiseBank < number_of_noise_banks; noiseBank++ ){
        		toneNumber = noise_bank_tone_number[ noiseBank ] ;

	           // FIND TONE NOISE BANK DELAY TIME, THEN FRAME DELAY.  2
	           thisFrameDelay = (int) floor((delayTime[ toneNumber ].A[0] * frames_per_sec) + 0.5) ;
	           thisFrameDelay = (frameNowChannelDelayIndex - thisFrameDelay) ; 
                	while( thisFrameDelay < 0) thisFrameDelay += maxNumOfDelayFrames ;



	           for(i = 0, j = noiseBank * NumNoiseBinsMult2; i < NumNoiseBins; i++, j += 2 ){
		     		// AMP
		         	noise[ j ] = channel_delay[ ((N+2) * thisFrameDelay) + indexInChannelForNoise[ i ] ] ;  

		        	// FREQ
		        	noise[ j + 1 ] = channel_delay[ ((N+2) * thisFrameDelay) + indexInChannelForNoise[ i ] + 1 ] ;  
		        	// ADD noise TO channelsSumBefore
		        	channelsSumBefore[ toneNumber ] += noise[ j ] ; //
	    		} ; 
   		} ; 



     		// PROCESS


		for( noiseBank = 0; noiseBank < number_of_noise_banks; noiseBank++ ){

           	// THIS TONE'S NOISE BANK
        		toneNumber = noise_bank_tone_number[ noiseBank ] ;

                	thisNoiseBankForceControl = forceFactor[ toneNumber ].A[0] > 1. ? 1. :  forceFactor[ toneNumber ].A[0] ; 
			// 
			if( rateCorrelatedForceSuppressionSwitch == 1 )
				thisNoiseBankForceControl *= rateCorrelatedForceSuppressor_TONE_VALUES[ toneNumber ] ; 

                if(synthetic_VIBRATO_SWITCH[ toneNumber ] == 1){
                		// ADD VIBRATO
                    	this_Stasis_Median_noise = stasisMedianForNoiseInDB[ toneNumber ].A[0] ; 
 
                   	vibDepthForceProp =  ( vibratoDepthInDB / (0. - this_Stasis_Median_noise) ) ; 
                    	vibDepthForceProp = vibDepthForceProp > 1. ? 1.  : vibDepthForceProp ; 
                    	thisNoiseBankForceControl = 
						(vibDepthForceProp  * vibValNow_TONE_VALUES[ toneNumber ] * thisNoiseBankForceControl) + 
                      		((1. - vibDepthForceProp) * thisNoiseBankForceControl);  
                } ; 

                thisNoiseBankGain = dB_to_amp( noiseDB[ toneNumber ].A[0] + Rate_Correlated_Noise_Control_in_dB_TONE_VALUES[ toneNumber ] )  ; 

		    

//		    Force_Correlated_Noise_Amp_Randomization_Proportion = dB_to_amp( curve( -10., 0., thisNoiseBankForceControl, -0. ) * randf(-1., 1.) ) ;
//		    Force_Correlated_Noise_Freq_Randomization_Proportion = curve( analysis_fundamental * 2., 0., thisNoiseBankForceControl, -0. ) * randf(-1., 1.) ;

                for(i = 0, j = noiseBank * NumNoiseBinsMult2, k = 0; i < NumNoiseBinsMult2 ; i += 2, j += 2, k++ ){
				temp = dB_to_amp( noiseBandDecibelLimiterThreshold[ k ] + 
								noise_band_decibel_limit_TONE_VALUES[ toneNumber ] +
									noise_band_decibel_limit_rolloff_TONE_VALUES[ toneNumber ]
				) ; 
                    
		     		// AMP  HERE @@@ NOISE SUPPRESSION
		          	if( static_noise[ j ] > temp ){
					 noise[ j ] = temp + (thisNoiseBankForceControl * (noise[ j ] - temp ) ) ;
				} else {
					noise[ j ] = static_noise[ j ] + (thisNoiseBankForceControl * ( noise[ j ] - static_noise[ j ] ) ) ; 
				} ; 


// **
				// RANDOMIZE AMP AND FREQ AS FORCE DECREASES
//	      			noise[ j ] *= Force_Correlated_Noise_Amp_Randomization_Proportion ;
//	      			noise[ j + 1 ] += Force_Correlated_Noise_Freq_Randomization_Proportion ;

// **

                    	noise[ j ] = noise[ j ] < 0.0 ? 0.0 : noise[ j ] * thisNoiseBankGain ; 


                     	// FREQ
 
	              	// TRANPOSE THE NOISE
                    	if( transposePoint_NOISE[ toneNumber ].A[0] == 0. )
	         	     		noise[ j + 1 ] *= (transposePoint_PITCH_values[ toneNumber ] / sourceptfundfreq[ toneNumber ] ) ; // USE HARMONY TRANSPOSE
                    	else
	         	     		noise[ j + 1 ] *= (transposePoint_NOISE_values[ toneNumber ] / sourceptfundfreq[ toneNumber ] ) ; // USE NOISE TRANSPOSE

                    	if( trans_switch[ toneNumber ] == 1 ){
                      	noise[ j + 1 ] = 
                           		(noise[ j + 1 ] + tones_master_freq_shift_controller_TONE_VALUES[ toneNumber ]) * 
							pmt_TONE_VALUES[ toneNumber ] ; 
                    	} ; 
                    	if( noise[ j + 1 ] < 0. ){ noise[ j + 1 ] = 0. ;  noise[ j ] = 0.0 ; } ; 

                	} ; 

		} ; 


	} ; // ( noiseFlag == 1 )


//	prt( "a861" ) ; 
 


	// SOURCE
   	if(sourceflag){
		SOURCE_harmadd.A[ 0 ] =  fval( &SOURCE_harmadd, funcDur, t );
		SOURCE_ptrans.A[ 0 ] = fval( &SOURCE_ptrans, funcDur, t );
	    	pms = semitones_to_mult( SOURCE_ptrans.A[ 0 ] ) ;
		SOURCE_dB.A[ 0 ] =  fval( &SOURCE_dB, funcDur, t );
		SOURCE_gain = dB_to_amp( SOURCE_dB.A[ 0 ] ) ; 
     }


	// **********************************

	// MAKE TRANSFER TO HARMONY ARRAY
	// !!!! indexInChannelForHarmony[ mm ] = THE ADDRESS OF THE AMP VALUE IN channel 
	//	FOR THE mm BIN  (A BIN BEING AN AMP/FREQ PAIR).


//	fwrite( &delayTime[ 0 ].A[0], sizeof(float), 1, adata[12] ) ;



	for( mm = 0 ; mm < NC ; mm++ ){

     		// FIND BIN DELAY TIME, THEN BIN FRAME DELAY. 
		thisFrameDelay = (int) floor((delayTime[ tone[mm] ].A[0] * frames_per_sec) + 0.5) ;
		thisFrameDelay = frameNowChannelDelayIndex - thisFrameDelay ; 
		while( thisFrameDelay < 0) thisFrameDelay += maxNumOfDelayFrames ;


		// AMP
		harmony[ (mm * 2) ] = channel_delay[ ((N+2) * thisFrameDelay) + (indexInChannelForHarmony[ mm ]) ] ; 	    
		// FREQ
		harmony[ (mm * 2) + 1 ] = channel_delay[ ((N+2) * thisFrameDelay) + (indexInChannelForHarmony[ mm ] + 1) ] ; 


		// SUM AMP INTO channelsSumBefore
	 	channelsSumBefore[ tone[mm] ] += harmony[ (mm * 2) ] ; 

	} ; 




//	prt( "a87" ) ; 
 

//	sumAndPrintDB( harmony, NCmult2, 0, 2, 1 ) ; 

//	temp5 = sumAndPrintDB( harmony, NCmult2, 0, 2, 0 ) ; 
//	fwrite( &temp5, sizeof(float), 1, adata[11] ) ; 


// **********************************
/*
	    // TUNE THE PARTIAL BANDS IN harmony
	mm = 0 ; band = 0 ; 
	b1 = 0 ; bcount = 0 ; asum = 0. ; fsum = 0. ;   
	while( mm < NC ){
	    
 
		toneNumber = tone[mm] ; 

		// SUM FREQS WEIGHTED BY FREQS
		ampSquared = harmony[ (mm * 2) ] * harmony[ (mm * 2) ] ; // SQUARE THE AMP
		fsum += (harmony[ (mm * 2) + 1 ] * ampSquared) ; // SUM THE FREQ WEIGHTED BY SQUARED AMP
		 
		// SUM AMPS
		asum += ampSquared ; 

		bcount++ ; // INCREMENT COUNT ON BINS FOR THIS PARTIAL

		if( mm == (NC - 1) ) temp = 0. ; else temp = sourcefreq[mm + 1] ; 


		// IF THE NEXT bin sourcefreq IS DIFFERENT, THEN TUNE THE GROUP OF BINS
		if( temp != sourcefreq[mm] ){	// THIS COULD BE A PROBLEM! ???  ADD CHECK FOR DIFFERENT TONE NUMBER IN CASE FREQS ARE THE SAME. 

 	    
           	// THIS IS THE TOP, FIND STATS AND TUNE BINS (SKIP IF FLAG IS 0)

			// IF NON ZERO AMPLITUDE, THEN
 			if( (asum > 0.) ){

                		// FIND AVERAGE FREQ
				thisAvgFreq = fsum / asum ; 

				// CREATE PROPORTION OF THE UNSHIFTED-PARTIAL-FREQUENCY AND THE COMPUTED AVERAGE FREQUENCY
				// IN ESSENCE, THE PROPORTIONAL DEVIANCE OF THE CURRENT MEASURED (AMP-WEIGHTED) AVERAGE FREQ 
				// FROM THE IDEAL INTEGER PARTIAL FREQ.
				//

		    		thisTuneFactor = forceFactor[ toneNumber ].A[0] ; 
			 
				if( rateCorrelatedForceSuppressionSwitch == 1 )
					thisTuneFactor *= rateCorrelatedForceSuppressor_TONE_VALUES[ toneNumber ] ; 


				if( synthetic_VIBRATO_SWITCH[ toneNumber ] == 1 ) thisTuneFactor *= vibValNow_TONE_VALUES[ toneNumber ]  ; // OK
                 	thisTuneFactorSave = thisTuneFactor ; 

				// TUNE BAND
		     		tuneBandPropPoint = 0.33 ; tuneBandCurveIndex = 1 ; // .33 and 3
                  	if( thisTuneFactor <= 1. ){
                     		if( (thisTuneFactor <= 1.) && (thisTuneFactor >= tuneBandPropPoint) ){
						// tuneBandPropPoint to 1.
                         		temp = curve(1., 0., (thisTuneFactor - tuneBandPropPoint) * (1./(1. - tuneBandPropPoint)), tuneBandCurveIndex)  ; 
                         		thisProp = ( temp * ((averageBandFreq[band] / thisAvgFreq) - 1.)  ) + 1. ; 
                    		}else{
                         		temp = thisTuneFactor < tuneBandPropPoint ? 1. : 0. ; 
                         		thisProp = ( temp * ((averageBandFreq[band] / thisAvgFreq) - 1.)  ) + 1. ; 
                    		} ; 

		        		// GO THROUGH ALL THE BINS FOR THIS PARTIAL AND TUNE
		        		for( i = b1 ; i < (b1 + bcount); i++ ) harmony[(i * 2) + 1] *= thisProp ;  

                  	}else{
		        		for( i = b1 ; i < (b1 + bcount); i++ ) harmony[(i * 2) + 1] *= 
						powf( (thisAvgFreq / averageBandFreq[band]),  (thisTuneFactor - 1.) ) ; 
                  	} ; 

                      // TRY sourcefreq[b1] for averageBandFreq[band]



				// NULL PHASE
		    		thisTuneFactor = thisTuneFactorSave ; 
		    		if( thisTuneFactor < 1. ){
		        		tuneNullPhasePropPoint = 0.5 ; tuneNullPhaseCurveIndex = 2 ; // .5 and 2

			    		// FORCE RANGE FROM tuneNullPhasePropPoint to 1.0 
		        		if( (thisTuneFactor < 1.) && (thisTuneFactor > tuneNullPhasePropPoint) ){
			      		thisProp = 
							curve( 0.0, 1.0, (1. / (1. - tuneNullPhasePropPoint)) * (thisTuneFactor - tuneNullPhasePropPoint),
							 tuneNullPhaseCurveIndex ) ;
		        		} else{
			       		thisProp = thisTuneFactor >= 1. ? 1. : 0.0 ; 
		        		} ; 

		        		// BRING BIN AS GROUP INTO TUNE TO THE AVERAGE FREQ (I.E. ALL THE SAME) BY THE 
					// tuneNullPhasePropPoint thisTuneFactor LEVEL. (NULL PHASE)
		        		// FOR ALL FORCE LESS THAN 1.0, TUNE. 
		        		for( i = b1 ; i < (b1 + bcount); i++ ){
			      		harmony[(i * 2) + 1] = sourcefreq[b1] + (  thisProp * ( harmony[(i * 2) + 1] - sourcefreq[b1] )  ); 
		        		} ; 

                	} ; 

			} ; 
		
			b1 = b1 + bcount ; // SET LOW BIN PAIR TO THE NEXT BOTTOM
           	band++ ; 
			bcount = 0 ; asum = 0. ; fsum = 0. ; 
		}

		mm++ ; // INCREMENT MASTER BIN COUNT
	}	    

*/ 
// END OF OLD WAY

// START OF NEW WAY ********************************************************
	    // TUNE THE PARTIAL BANDS IN harmony
	mm = 0 ; band = 0 ; 
	b1 = 0 ; bcount = 0 ;    
	// FOR EACH BAND . . . 
	for( band = 0; band < numberOfBands; band++ ){
		// FIND AMP-WEIGHTED FREQ AVERAGE
		asum = 0. ; fsum = 0. ;
		for( mm = partial_Band_Begin[ band ]; mm < partial_Band_Begin[band + 1]; mm++ ){

			// SUM FREQS WEIGHTED BY FREQS
			ampSquared = harmony[ (mm * 2) ] * harmony[ (mm * 2) ] ; // SQUARE THE AMP
			fsum += (harmony[ (mm * 2) + 1 ] * ampSquared) ; // SUM THE FREQ WEIGHTED BY SQUARED AMP
		 
			// SUM AMPS
			asum += ampSquared ; 
		} ;

		// IF NON ZERO AMPLITUDE, THEN
 		if( (asum > 0.) ){

			toneNumber = tone[ partial_Band_Begin[ band ] ] ; 

         		// FIND AVERAGE FREQ
			thisAvgFreq = fsum / asum ; 

			// CREATE PROPORTION OF THE UNSHIFTED-PARTIAL-FREQUENCY AND THE COMPUTED AVERAGE FREQUENCY
			// IN ESSENCE, THE PROPORTIONAL DEVIANCE OF THE CURRENT MEASURED (AMP-WEIGHTED) AVERAGE FREQ 
			// FROM THE IDEAL INTEGER PARTIAL FREQ.
			//
		    	thisTuneFactor = forceFactor[ toneNumber ].A[0] ; 
			 
			if( rateCorrelatedForceSuppressionSwitch == 1 )
				thisTuneFactor *= rateCorrelatedForceSuppressor_TONE_VALUES[ toneNumber ] ; 

			if( synthetic_VIBRATO_SWITCH[ toneNumber ] == 1 ) thisTuneFactor *= vibValNow_TONE_VALUES[ toneNumber ]  ; // OK
 
             thisTuneFactorSave = thisTuneFactor ; 

			// TUNE BAND
		    	tuneBandPropPoint = 0.33 ; tuneBandCurveIndex = 1 ; // .33 and 3
              if( thisTuneFactor <= 1. ){
              	if( (thisTuneFactor <= 1.) && (thisTuneFactor >= tuneBandPropPoint) ){
					// tuneBandPropPoint to 1.
                       temp = curve(1., 0., (thisTuneFactor - tuneBandPropPoint) * 
						(1./(1. - tuneBandPropPoint)), tuneBandCurveIndex)  ; 
                       thisProp = ( temp * ((averageBandFreq[ band ] / thisAvgFreq) - 1.)  ) + 1. ; 
                    }else{
                       temp = thisTuneFactor < tuneBandPropPoint ? 1. : 0. ; 
                       thisProp = ( temp * ((averageBandFreq[ band ] / thisAvgFreq) - 1.)  ) + 1. ; 
                    } ; 

		        	// GO THROUGH ALL THE BINS FOR THIS PARTIAL AND TUNE
		        	for( i = partial_Band_Begin[ band ]; i < partial_Band_Begin[band + 1]; i++ ) harmony[(i * 2) + 1] *= thisProp ;  

              }else{
		     	for( i = partial_Band_Begin[ band ]; i < partial_Band_Begin[band + 1]; i++ ) harmony[(i * 2) + 1] *= 
					powf( (thisAvgFreq / averageBandFreq[ band ]),  (thisTuneFactor - 1.) ) ; 
              } ; 


			// NULL PHASE
		    	thisTuneFactor = thisTuneFactorSave ; 
		    	if( thisTuneFactor < 1. ){
		     	tuneNullPhasePropPoint = 0.5 ; tuneNullPhaseCurveIndex = 2 ; // .5 and 2

				// FORCE RANGE FROM tuneNullPhasePropPoint to 1.0 
		    		if( (thisTuneFactor < 1.) && (thisTuneFactor > tuneNullPhasePropPoint) ){
			      	thisProp = curve( 0.0, 1.0, (1. / (1. - tuneNullPhasePropPoint)) * 
						(thisTuneFactor - tuneNullPhasePropPoint),  tuneNullPhaseCurveIndex ) ;
		        	}else{
			     	thisProp = thisTuneFactor >= 1. ? 1. : 0.0 ; 
		        	} ; 

		        	// BRING BIN AS GROUP INTO TUNE TO THE AVERAGE FREQ (I.E. ALL THE SAME) BY THE 
				// tuneNullPhasePropPoint thisTuneFactor LEVEL. (NULL PHASE)
		        	// FOR ALL FORCE LESS THAN 1.0, TUNE. 
		        	for( i = partial_Band_Begin[ band ]; i < partial_Band_Begin[band + 1]; i++ ){
			     	harmony[(i * 2) + 1] = sourcefreq[ i ] + (  thisProp * ( harmony[(i * 2) + 1] - sourcefreq[ i ] )  ); 
		        	} ; 

             } ; 

//			if( band == 2 ) 
//				for( i = partial_Band_Begin[ band ]; i < partial_Band_Begin[band + 1]; i++ ) 
//						prf( harmony[(i * 2) + 1], "harmony[(i * 2) + 1]" ) ; 
		} ; 


	}	    

// END OF NEW WAY ********************************************************




	// OK sumAndPrintDB( harmony, NCmult2, 0, 2 ) ; 


	// **********************************


	// ################ MODIFY AMP AND FREQ ################ 

	for( bin = 0, doubleBin = 0 ; bin < NC ; bin++, doubleBin += 2 ){

		toneNumber = tone[ bin ] ; 
		// AMP		    

         thisForceFactor = forceFactor[ toneNumber ].A[0] > 1. ? 1. : forceFactor[ toneNumber ].A[0] ; 
		if( rateCorrelatedForceSuppressionSwitch == 1 )
			thisForceFactor *= rateCorrelatedForceSuppressor_TONE_VALUES[ toneNumber ] ; 

		// AMP DEVIATION SUPPRESSION/AMPLIFICATION

//		if( boundariesResetFlag == -1) fprintf( stderr, "\n time: %f\t thisForceFactor: %f\n", t, thisForceFactor ) ; 

//		fwrite( &thisForceFactor, sizeof(float), 1, adata[2] ) ; 



		// REMAP BASED UPON CURVE INDICES.
		thisAmpForceFactor = curve( 0., 1., thisForceFactor, harmony_force_curve_indeces[ bin ] ) ; 

//		fwrite( & thisAmpForceFactor, sizeof(float), 1, adata[3] ) ; 

		if( synthetic_VIBRATO_SWITCH[ toneNumber ] == 1 ){
			// ADD VIBRATO
                this_Stasis_Median_harmony = stasisMedianForHarmonyInDB[ toneNumber ].A[0] =
                		fval( &stasisMedianForHarmonyInDB[toneNumber], dur, t ) ; 

// prf( this_Stasis_Median_harmony, "5 this_Stasis_Median_harmony" );  


			vibDepthForceProp =  ( vibratoDepthInDB / (0. - this_Stasis_Median_harmony) ) ; 
			vibDepthForceProp = vibDepthForceProp > 1. ? 1.  : vibDepthForceProp ; 
			thisAmpForceFactor = (vibDepthForceProp  * vibValNow_TONE_VALUES[ toneNumber ] * thisAmpForceFactor) + 
                                      ((1. - vibDepthForceProp) * thisAmpForceFactor);  

//			if( boundariesResetFlag == -1) fprintf( stderr, "\n time: %f\t INSIDE SYNTHETIC VIBRATO thisAmpForceFactor: %f\n",
//					 t, thisAmpForceFactor ) ; 

           }; 


//		fwrite( & thisAmpForceFactor, sizeof(float), 1, adata[4] ) ; 


		// harmony AMP CONTROLLED BY FORCE SHAPED BY AMP CURVES FOR THIS BIN. 
		harmony[ doubleBin ] = static_harmony[ doubleBin ] + 
		    (thisAmpForceFactor * (harmony[ doubleBin ] - static_harmony[ doubleBin ])) ; 



		harmony[ doubleBin ] *=
			dB_to_amp( toneDB[ toneNumber ].A[0] + Rate_Correlated_Tone_Control_in_dB_TONE_VALUES[ toneNumber ] ) * 
			coswindow[ bin ] * tones_master_gain_controller_TONE_VALUES[ toneNumber ] * 
				master_gain_TONE_VALUES[ toneNumber ] ;  

		// FREQ TRANSPOSITION

			// PARTIAL SHIFT
	      newfreq = harmony[ doubleBin + 1 ]  + ( sourceptfundfreq[ toneNumber ] * (partialShift[ toneNumber ].A[0]) ) ;  


			// SPECTRAL STRETCH
		newfreq = (newfreq + 
				(sourceptfundfreq[ toneNumber ] * 
				   spectralStretchCompress[ toneNumber ].A[0] * ( (newfreq / sourceptfundfreq[ toneNumber ]) - 1. ) ) ) ; 

			// TRANSPOSITION
		newfreq *= (transposePoint_PITCH_values[ toneNumber ] / sourceptfundfreq[ toneNumber ])  ;  


		if( trans_switch[ toneNumber ] == 1 )
			    harmony[ doubleBin + 1 ] = 
				(newfreq +  tones_master_freq_shift_controller_TONE_VALUES[ toneNumber ] ) 
					* pmt_TONE_VALUES[ toneNumber ] ; // ADD IN TONES GLOBAL SHIFT AND TRANSPOSE
		else
			harmony[ doubleBin + 1 ] = newfreq ; // DON'T ADD IN. 


	} ; 

//	fwrite( & harmony[0], sizeof(float), 1, adata[9] ) ; 
//	fwrite( & harmony[2], sizeof(float), 1, adata[10] ) ; 


//	sumAndPrintDB( harmony, NCmult2, 0, 2 ) ; 


	// END TRACK ENVELOPE *************************************************************

	// FILTER THE PARTIAL BANDS
	mm = 0 ; 
	b1 = 0 ; bcount = 0 ;  
     	while( mm < NC ){

		toneNumber = tone[mm] ; 
	    
	    	// TURN ON filterflag IF NON-ZERO ROLLOFF DB BASE OR RANGE.
//	    	if( (bandpass_DB_rolloff_Base[ toneNumber ] != 0.) || (bandpass_DB_rolloff_Range[ toneNumber ] != 0.) )
//			filterflag = 1 ; 



 	    	bcount++ ; // INCREMENT COUNT ON BINS FOR THIS PARTIAL

	    	if( mm == (NC - 1) ) temp = 0. ; else temp = sourcefreq[mm + 1] ; 

	    	if( temp != sourcefreq[mm] ){
			// THIS IS THE TOP OF THIS BAND, FILTER ALL TONE BINS IN BAND IF TONE FILTER SWITCH IS ON

			if( tone_Filter_Switch[ toneNumber ] != 0 ){

				// FIND AMP ROLLOFF FROM DB ROLLOFF
		    		rolloff_amp = dB_to_amp( bandpassRolloffInDbPerOctave[ toneNumber ].A[0] ) ; 

		    		// FIND CF
                		cf = bandpassCF[ toneNumber ].A[0] ; 
		    
		    		// GO THROUGH ALL THE BINS AND FILTER THEM
		    		if( rolloff_amp != 1.0 ){
		      		for( i = b1 ; i < (b1 + bcount); i++ ){

                      		// FILTER ONLY IF BIN FREQ IS GREATER THAN ZERO
                           		if( harmony[(i * 2) + 1] > 0. ){
			    
                           			// FIND PROPORTION
							if( harmony[(i * 2) + 1] > cf ){
								// FREQUENCIES ABOVE cf
								if( tone_Filter_Type[ toneNumber ] != 2 ){ // NOT A HIGHPASS
				    					temp2 = harmony[(i * 2) + 1] / cf  ; 
									// FIND THE AMP
									temp2 = log10( temp2 ) * log2 ;
									harmony[(i * 2)] = harmony[(i * 2)] * powf( rolloff_amp,  temp2 ) ; 
								} ; 
							}else{
								// FREQUENCIES EQUAL TO OR BELOW CF
								if( tone_Filter_Type[ toneNumber ] != 3 ){ // NOT A LOWPASS
				    					temp2 = cf / harmony[(i * 2) + 1]  ; 
									// FIND THE AMP
									temp2 = log10( temp2 ) * log2 ;
									harmony[(i * 2)] = harmony[(i * 2)] * powf( rolloff_amp,  temp2 ) ; 
								} ;
    							} ;
						}	
		        		}
				}

			}
		
			b1 = b1 + bcount ; // SET LOW BIN PAIR TO THE NEXT BOTTOM
			bcount = 0 ; 
		}

		mm++ ; // INCREMENT MASTER BIN COUNT
	}

	// FILTER NOISE
     for( noiseBank = 0; noiseBank < number_of_noise_banks; noiseBank++ ){

     		// THIS NOISE BANK'S TONE NUMBER
           toneNumber = noise_bank_tone_number[ noiseBank ] ;

           if( noise_Filter_Switch[ toneNumber ] != 0 ){

           	// FIND AMP ROLLOFF FROM DB ROLLOFF
                rolloff_amp = dB_to_amp( bandpassRolloffInDbPerOctave[ toneNumber ].A[0] ) ; 


                if( rolloff_amp != 1.0 ){
	              	// FIND CF
                		cf = bandpassCF[ toneNumber ].A[0] ; 
	         
                   	for( i = 0, j = noiseBank * NumNoiseBinsMult2; i < NumNoiseBinsMult2 ; i += 2, j += 2 ){ 

                      	// GO THROUGH ALL THE BINS AND FILTER THEM

                           	// SKIP IF BIN FREQ IS ZERO OR LESS
                           	if( noise[j + 1] > 0. ){
			    
                           		// FIND PROPORTION
		           		if( noise[j + 1] > cf ){
							// FREQUENCIES ABOVE
							if( tone_Filter_Type[ toneNumber ] != 2 ){ // NOT A HIGHPASS
								temp2 = noise[j + 1] / cf  ; 
		           				// FIND THE AMP
		           				temp2 = log10( temp2 ) * log2 ;
		           				noise[ j ] *= powf( rolloff_amp,  temp2 ) ; 
							} ; 
		           		}else{
							// FREQUENCIES BELOW
							if( tone_Filter_Type[ toneNumber ] != 3 ){ // NOT A LOWPASS
								 temp2 = cf / noise[j + 1]  ; 
		           				// FIND THE AMP
		           				temp2 = log10( temp2 ) * log2 ;
		           				noise[ j ] *= powf( rolloff_amp,  temp2 ) ; 
							} ; 
						} ; 

                            } ; 	

				} ; 

			} ;    

		} ; 

	} ; 
            

	// END BANDPASS

    	// *************EQUALIZE THE OUTPUT SPECTRUM FOR NOISE AND/OR TONES


    	// REMOVE DC
    	cutDC( harmony, NCmult2 + 2, 20. ) ;     
    	if( noiseFlag == 1 ) {
	if( frame_count == 0 ) prt( "CUTTING DC IN NOISE" ) ; 
	cutDC( noise, NumNoiseBinsMult2 * number_of_noise_banks, 20. ) ; 



	//THIS IS THE END OF THE FRAMES SYNTHESIS LOOP, I THINK. 


} ; 

// *****************
// MODIFICATIONS 
// *****************


// *********************************************************



		    
// **************** SOURCE
if(sourceflag){
	for( i = 1; i < (N + 2); i = i + 2){
		// MODIFY SOURCE

		// **FREQ
		temp = pms * (channel[i] + SOURCE_harmadd.A[ 0 ]) ;
		// **AMP
		channel[i - 1] = channel[i - 1] * SOURCE_gain * master_gain_no_delay_for_source ; 
		// ZERO BINS OUT OF 0-Nyquist FREQUENCY RANGE
		if((temp <= 0.) || (temp >= nyquist)) channel[i - 1] = 0. ; 
		else channel[i] = temp ; 
	}
}

// *************   
// *************   
// harmony OUT OF BOUNDS?
for( i = 1; i < (NCmult2 + 2); i += 2 ){
	if((harmony[ i ] <= 0.) || (harmony[ i ] >= nyquist)) harmony[i - 1] = 0. ; 

}

//if( frame_count == 0 ) pri( channow, "channow" ) ; 

// ZERO AMPS FOR PARTIAL BANDS NOT INCLUDED IN THIS OUTPUT CHANNEL. 
// FOR EACH BAND . . . 
for( band = 0; band < numberOfBands; band++ ){
	if( ( (partial_Band_Channel_Output_Number[ band ] - 1) != channow) && (partial_Band_Channel_Output_Number[ band ] != 0) ) {
		for( mm = partial_Band_Begin[ band ]; mm < partial_Band_Begin[band + 1]; mm++ ) harmony[ (mm * 2) ] = 0. ; 
	} ;
} ;

// ZERO AMPS FOR TONE NOISE BANKS NOT INCLUDED IN THIS OUTPUT CHANNEL. ????
if( noiseFlag == 1 ){

//	if( frame_count == 0 ) prt( "IN NOISE CHANNEL ZERO SECTION" ) ;  
//	if( frame_count == 0 ) pri( number_of_noise_banks, "number_of_noise_banks" ) ; 

	for( noiseBank = 0; noiseBank < number_of_noise_banks; noiseBank++ ){

//		if( frame_count == 0 ) fprintf( stderr, "\nNOISE BANK: %d, noise_Bank_Channel_Output_Number[ noiseBank ]: %d", 
//			noiseBank, noise_Bank_Channel_Output_Number[ noiseBank ] ) ;

		if( ( (noise_Bank_Channel_Output_Number[ noiseBank ] - 1) != channow) && (noise_Bank_Channel_Output_Number[ noiseBank ] != 0) ) {

//			if( frame_count == 0 ) fprintf( stderr, "\nZEROING NOISE BANK: %d", noiseBank ) ;

			for( i = 0, j = noiseBank * NumNoiseBinsMult2; i < NumNoiseBinsMult2 ; i += 2, j += 2 ) noise[ j ] = 0. ; 
		} ; 
	} ;
} ;

if(sourceflag){
	synt = getthresh( channel, N + 2, threshfac );
	temp = getthresh( harmony, NCmult2, threshfac );
	if(temp > synt) synt = temp ; 
}else{
	synt = getthresh( harmony, NCmult2, threshfac );
}


//noscbank( static_harmony, NC, R, Nw, I, P, output );
//noscbank( noise,  NumNoiseBins * number_of_noise_banks, R, Nw, I, P, output );
//noscbank2( static_harmony, NC, R, Nw, I, P, output, noise,  NumNoiseBins * number_of_noise_banks );




	    // OSCILLATOR RESYNTHESIS
if( (sourceflag == 0) && (noiseFlag == 0) ){ // 0 0 


 
	// TONES ALONE (NO SOURCE OR NOISE)
	if( frame_count == 0 )prt("SYNTHESIZING OF TONES ALONE--NO SOURCE OR NOISE."); 
			noscbank( harmony, NC, R, Nw, I, P, output ) ; 

}else if( (sourceflag == 1) && (noiseFlag == 0) ){ // 1 0
 
			// SOURCE AND TONES (NO NOISE)
 			if( frame_count == 0 )prt("SYNTHESIZING SOURCE WITH TONES--NO NOISE."); 
			noscbank2( channel, N2, R, Nw, I, P, output, harmony,  NC );

}else if ( (sourceflag == 0) && (noiseFlag == 1) ){ // 0 1

			// TONES AND NOISE (NO SOURCE)
 			if( frame_count == 0 )prt("SYNTHESIZING TONES AND NOISE--NO SOURCE"); 
			noscbank2( harmony, NC, R, Nw, I, P, output, noise,  NumNoiseBins * number_of_noise_banks );

}else{						// 1 1 

			// SOURCE, TONES, AND NOISE.
 			if( frame_count == 0 )prt("SYNTHESIZING SOURCE, TONES, AND NOISE."); 
			noscbank3( channel, N2, R, Nw, I, P, output, harmony, NC, noise,  NumNoiseBins * number_of_noise_banks );

} ; 

	    
shiftout( output, Nw, I, on+Nw-I, 0 ) ;


frame_count++ ; 
// FRAMES LOOP END


//fprintf( stderr, "\nt: %f dur: %f autostopflag: %d ringTimeCountDown: %f ringTime: %f", 	
//	t, dur, autostopflag, ringTimeCountDown, ringTime ) ;  

}
 
/*
// MAKE AVERAGE CHANGE FOR AMP AND FREQ, THEN PRINT
average_amp_change_in_dB = 0 ; average_freq_change = 0 ; 
fprintf( stderr, "\n\nAVERAGE CHANGE:" ) ; 
fprintf( stderr, "\nCHANNEL\tCENTER FREQUENCY\tAMPLITUDE(dB)\tFREQUENCY (AS PROPORTION OF ANALYSIS BANDWIDTH)" ) ; 

for( i = (N - 2), j = (N - 1), k = (N / 2) ; i >= 0; i -= 2, j -= 2, k-- ) {
	channel_change_average[ i ] /= (float) frame_count ; average_amp_change_in_dB += channel_change_average[ i ] ;
	channel_change_average[ j ] /= (float) frame_count ; average_freq_change += channel_change_average[ j ] ;

	fprintf( stderr, "\n%d\t%f\t%f\t%f", 
		k, ((float) k * analysis_fundamental),  channel_change_average[ i ], channel_change_average[ j ] ) ;

	for( l = 0; l < (int)( 20.0 * channel_change_average[ j ] ) ; l++ )fprintf( stderr, "*" ) ; 

} ;
fprintf( stderr, "\n\nAVERAGE AMPLITUDE CHANGE (dB): %f,\tAVERAGE FREQUENCY CHANGE (AS PROPORTION OF ANALYSIS BANDWIDTH): %f", 
		average_amp_change_in_dB / (float) (N / 2), average_freq_change / (float) (N / 2)
) ; 

*/


// FLUSH OUT AND CLOSE OUTPUT FILE
shiftout( output, Nw, I, 1, 1 ) ;
 


    
// CHANNELS LOOP END
} 

 
// CLOSE  INPUT FILE
//    fclose(ifd) ;  


//for( i = 0; i < NUMBER_OF_WRITE_NUMBERS; i++ ){
//	fclose( adata[i] ) ; 
//} ; 



fprintf(stderr,"\n\nCHORDMAPPERPLUS : RESYNTHESIS COMPLETED\n");




if( SOURCE_harmadd.n != 1. ) fclose(SOURCE_harmadd.fp ) ;
if( master_gain_in_dB.n != 1. ) fclose(master_gain_in_dB.fp ) ;
if( SOURCE_ptrans.n != 1. ) fclose(SOURCE_ptrans.fp ) ;
if( tones_master_gain_controller_in_dB.n != 1. ) fclose(tones_master_gain_controller_in_dB.fp ) ;
if( SOURCE_dB.n != 1. ) fclose(SOURCE_dB.fp ) ;
if( tones_macro_pitch_controller_in_semitones.n != 1. ) fclose(tones_macro_pitch_controller_in_semitones.fp ) ;
if( tones_master_freq_shift_controller.n != 1. ) fclose(tones_master_freq_shift_controller.fp ) ;
if( analysis.n != 1. ) fclose(analysis.fp ) ;
if( filtrate.n != 1. ) fclose(filtrate.fp ) ;
if( filttorigin.n != 1. ) fclose(filttorigin.fp ) ;
if( filtwinlow.n != 1. ) fclose(filtwinlow.fp ) ;
if( filtwinhi.n != 1. ) fclose(filtwinhi.fp ) ;

 

  
fclose( data ) ; 




filesToRemove( NULL, 1 ) ;
    
//    sprintf( tempstring,  "rm %s", new_datafile ) ; 
//    system( tempstring ) ; 




//	EXIT_failure(); 	 

    exit(EXIT_SUCCESS) ;
}

void EXIT_failure(){

	filesToRemove( NULL, 1 ) ;
	exit( EXIT_FAILURE ) ; 

}; 

void pd( int i ){ if( debugFlag == 1) fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

float sumAndPrintDB(
    float array[],
    int size, 
    int startIndex,
    int incr,
    int printFlag
){
    int i ; 
    static float sum, dBsum ; 

    sum = 0 ; 
    for( i = startIndex; i < size; i += incr ) sum += array[ i ] ;

	dBsum = amp_to_dB( sum ) ; 
    if( printFlag == 1) fprintf( stderr, "time: %f\t%f\n", t, dBsum ) ; 

	return( dBsum ) ; 
} ; 


float halfcoscurve( float v ){
    
    
    static float table[ 1024 ] ; 
    static int first=1 ; 
    int i ; 
    float x ; 
    
    // COSINE CURVE OF PI LENGTH FITTED TO TRAVERSE FROM 1 to 0. 
    // SETUP TABLE FIRST TIME
    
    if( first ){
	
	for( i = 0 ; i < 1024; i++ ) {
	    x = ((float) i / 1024.) * (TWOPI * .5) ; 
	    table[ i ] = .5 * ((float) cos( (double) x ) + 1.) ; 
	}
	first = 0 ; 
    }
    
    i = (int) ((v * 1023.) + .5) ; 
    return( table[ i ] ) ; 
    
    
}


float coscurve_dB( float v ){
    
    
    static float table[ 1024 ] ; 
    static int first=1 ; 
    int i ; 
    float x ; 
    
    // SETUP TABLE FIRST TIME
    
    if( first ){
	
	for( i = 0 ; i < 1024; i++ ) {
	    x = ((float) i / 1024.) * (TWOPI * .5) ; 
	    table[ i ] = .5 * ((float) cos( (double) x ) + 1.) ; 
	    if( table[ i ] > 0. )
		table[ i ] = 20. * log10( table[ i ] ) ;
	    else
		table[ i ] = -200. ;  
	}
	first = 0 ; 
    }
    
    i = (int) ((v * 1023.) + .5) ; 
    return( table[ i ] ) ; 
    
    
}

float fullcoscurve( float v ){
    
    
    static float table[ 4096 ] ; 
    static int first=1 ; 
    int i ; 
    float x ; 
    
    // COSINE CURVE OF TWOPI LENGTH INVERTED AND FITTED TO TRAVERSE FROM 0 to 1 to 0. 
    // SETUP TABLE FIRST TIME
    
    if( first ){
	
	for( i = 0 ; i < 4096; i++ ) {
	    x = ((float) i / 4096.) * (TWOPI) ; 
	    table[ i ] = -.5 * ((float) ( -1. * cos( (double) x )) + 1.) ; 
	}
	first = 0 ; 
    }
    
    i = (int) ((v * 4096.) + .5) ; 
    if( i >= 4096 ) i = 0 ; 
    return( table[ i ] ) ; 
    
    
}

    
 
void makeAmplitudeEnvelopeFromSumOfAmps(
    struct func *analysis, 
    int analysis_Nplus2, 
    int ainchan, 
    int analysis_chan,
    float analysis_dur,
    float analysis_lower[], 
    float analysis_higher[],
    int normalizeFlag, 
    int iframes_per_sec,
    float ampEnvelope[],
    int niframes,
    float *peakSum 
){
    int frame, i ; 
    float thisTime ;     
    float *thisFreqresponseFrame, thisSum ; 

    *peakSum=-99999999.0 ; 

    fvec( thisFreqresponseFrame, analysis_Nplus2 ) ; 


    for( frame = 0; frame < niframes; frame++ ){
        thisTime = (float) frame / (float) iframes_per_sec ; 

	// GET FRAME
        makeInterpolatedFilterFrame ( 
	    analysis, 
	    analysis_lower, 
	    analysis_higher, 
	    thisFreqresponseFrame,
	    iframes_per_sec, 
	    analysis_Nplus2, 
	    thisTime, 
	    ainchan, 
	    analysis_chan
        ) ; 
	// MAKE SUM 
        thisSum = 0. ; 
        for( i = 0; i < analysis_Nplus2; i += 2 ) thisSum += thisFreqresponseFrame[ i ] ; 

        // PUT IN ENVELOPE
        ampEnvelope[ frame ] = thisSum ;

       // SAVE PEAK FOR NORMALIZATION
        if( thisSum > *peakSum ) *peakSum = thisSum ;  

    } ; 

	// NORMALIZE
    if( normalizeFlag == 1 ){
        prt( "NORMALIZING ENVELOPE AMPLITUDE" ) ;  	
         for( frame = 0; frame < niframes; frame++ )ampEnvelope[ frame ] /= *peakSum ; 
         *peakSum = 1.0 ; 
    } else {

        prf( amp_to_dB( *peakSum ), "PEAK ENVELOPE AMP IN DB" ) ;  
    } ; 


} ;      


     
void makeStaticFreqResponseAveragesFromDataFile ( 
    struct func *analysis, 
    int analysis_Nplus2, 
    float analysis_fundamental,
    int ainchan, 
    int analysis_chan,
    float analysis_dur,
    float analysis_lower[], 
    float analysis_higher[],
    float static_freqresponse_averages[], 
    int numOfStaticFreqresponseAverages, 

    float harmony_force_curve_indeces[],
    int partial_Band_Begin[], 
    int numberOfBands,
    int indexInChannelForHarmony[],

    float *static_freqresponse_peak_SUM_in_dB, 

    int iframes_per_sec,
    float midTimePoint,
    float timeWindowSize,
    int UseHammingWindowFlag,
    float HammingWindow[],
    int HammingWindowSize,

    float thisFreqresponseFrame[],
    float ampWeightSums[],
    float averageBandFreq[],

    int harmonyBands_IndexOfStrongestBinInBand[], 
    float harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand[], 
    int NC

){

    FILE *fopen();
    float thisAmpWeightSum, averageFreqForThisBand ; 

    float valueSum, windowAmpSum ; 
    float thisSum, strongestSum, peakAmp, weightedFreqSum, weightsSum, thisFreq ; 
    int band, strongestBin, bin, dBlevelFreqresponse ; 

    int iFlag, amp, freq, start, end ; 
    float mult, temp; 
    int dBdiffsum ; 
    float sumOfAmps, dBsum,  peakdBsum=(-9999999.) ;  
    int i, j, k, l, n, static_freqresponse_averages_base_index, 
	static_freqresponse_index ;
    float beginT, endT, timeIncr=(1./25.), thisTime, halfTimeWindowSize ; 
 

    float thisAmpWeight ;      
    int totalFrames ; 
    int frame ;   
    int thisHammingWindowIndex ;   
    float realIndexIncr ;
    float realHammingWindowStartIndex ;
    float thisHammingWindowValue ;
    float sumOfHammingWindowValues ;
    
    int static_freqresponse_averages_sum_count[ NUMBER_OF_STATIC_FREQRESPONSE_AVERAGES ] ;  
    

    prt("MAKING DB LEVEL FREQUENCY RESPONSES............." ) ; 

    pri( ainchan + 1, "THIS CHANNEL" ) ; 

pd( 20 ); 

 
	// MAKE HAMMING WINDOW 
     for ( i = 0 ; i < HammingWindowSize ; i++ )
             HammingWindow[i] = 0.54 - 0.46*cos( TWOPI*i/(HammingWindowSize - 1) ) ;

pd( 21 ); 


    // *********** ADJUST/SET WINDOW TIME BOUNDARIES
    halfTimeWindowSize = 0.5 * timeWindowSize ; 
    if( halfTimeWindowSize > midTimePoint ){
	// SHRINK TIME WINDOW RELATIVE TO BEGIN.
	halfTimeWindowSize = midTimePoint ; 
    }else if( halfTimeWindowSize > (analysis_dur - midTimePoint)){
	// SHRINK TIME WINDOW RELATIVE TO END.
	halfTimeWindowSize = analysis_dur - midTimePoint ; 
    } ; 
pd( 22 ); 


    beginT = midTimePoint - halfTimeWindowSize ;  endT = midTimePoint + halfTimeWindowSize ;  

    // INITIALIZE 
    for( dBlevelFreqresponse = 0; dBlevelFreqresponse < numOfStaticFreqresponseAverages * analysis_Nplus2 ; dBlevelFreqresponse ++ ) {
	ampWeightSums[dBlevelFreqresponse] = 0 ; 
	static_freqresponse_averages[ dBlevelFreqresponse ] = 0. ;
    } ; 
    for( dBlevelFreqresponse = 0; dBlevelFreqresponse < numOfStaticFreqresponseAverages; dBlevelFreqresponse ++ ) 
		static_freqresponse_averages_sum_count[dBlevelFreqresponse] = 0 ; 
     

pd( 23 ); 


    thisTime = beginT ;  


    // ***********  FIND HAMMING WINDOW INCREMENT AND START POINT
    totalFrames = (int) ((endT - beginT) / timeIncr) ;  
    realIndexIncr = (float) HammingWindowSize / (float) totalFrames ; 
    realHammingWindowStartIndex = 0.5 * realIndexIncr ; 
    sumOfHammingWindowValues = 0. ; 

pd( 24 ); 


    // FIND PEAK DB SUM FOR ALL FRAMES FOR GIVEN TIME INCREMENT
    for( frame = 0; frame < totalFrames; frame++ ){

	thisTime = beginT + ((float) frame * timeIncr) ;

	makeInterpolatedFilterFrame ( 
	    analysis, 
	    analysis_lower, 
	    analysis_higher, 
	    thisFreqresponseFrame,
	    iframes_per_sec, 
	    analysis_Nplus2, 
	    thisTime, 
	    ainchan, 
	    analysis_chan
	) ; 

pd( 25 ); 



	// FIND SUM OF AMPS
	sumOfAmps = 0. ; 
	for( i = 0; i < analysis_Nplus2; i += 2 ){
	   sumOfAmps +=  (thisFreqresponseFrame[ i ] * AMP_REDUCTION ) ; // EXPERIMENT: AMP REDUCTION 
	} ; 	

	dBsum = (int) (0.5 + amp_to_dB( sumOfAmps )); 
	if( dBsum > peakdBsum) peakdBsum = dBsum ; 

    } ; 

pd( 26 ); 

 

    *static_freqresponse_peak_SUM_in_dB = peakdBsum ; 


    // *********** 
    //  SUM EACH FRAME INTO THE APPROPRIATE AVERAGE FREQ RESPONSE FOR 
    // ITS DB SUM. KEEP TRACK OF THE NUMBER OF FRAMES ADDED. 
    for( frame = 0; frame < totalFrames; frame++ ){

	thisTime = beginT + ((float) frame * timeIncr) ;

	makeInterpolatedFilterFrame ( 
	    analysis, 
	    analysis_lower, 
	    analysis_higher, 
	    thisFreqresponseFrame,
	    iframes_per_sec, 
	    analysis_Nplus2, 
	    thisTime, 
	    ainchan, 
	    analysis_chan
	) ; 

pd( 27 ); 

	// FIND SUM OF AMPS
	sumOfAmps = 0. ; 
	for( i = 0; i < analysis_Nplus2; i += 2 ){
	   sumOfAmps +=  ( thisFreqresponseFrame[ i ]  * AMP_REDUCTION ) ; // EXPERIMENT 
	} ; 	

	dBsum = (int) (0.5 + amp_to_dB( sumOfAmps )) ; 

//fprintf( stderr, "\n dBsum: %f", dBsum ) ; 

	dBdiffsum = (int)(peakdBsum - dBsum + 0.5) ;
	if( dBdiffsum < 0 ) static_freqresponse_index = 0 ; 
	else if( dBdiffsum > (numOfStaticFreqresponseAverages - 1) ) static_freqresponse_index = numOfStaticFreqresponseAverages - 1 ; 
	else static_freqresponse_index = dBdiffsum ; 
	

//fprintf( stderr, "\n static_freqresponse_index: %d", static_freqresponse_index ) ; 


	static_freqresponse_averages_base_index = static_freqresponse_index * analysis_Nplus2  ; 

pd( 28 ); 


	// MAKE HAMMING WINDOW VALUE
	if( UseHammingWindowFlag == 1 ){
	    thisHammingWindowIndex = (int)( realHammingWindowStartIndex + ((float) frame * realIndexIncr) ) ; 
	    thisHammingWindowValue = HammingWindow[ thisHammingWindowIndex ] ; 
	}else{
	    thisHammingWindowValue = 1.0 ; 
	} ; 
pd( 280 ); 

	sumOfHammingWindowValues += thisHammingWindowValue ; 
pd( 281 ); 

	// FREQS
	for( i = 1; i < analysis_Nplus2; i += 2 ){
	    thisAmpWeight = thisFreqresponseFrame[ i - 1 ] * thisHammingWindowValue * AMP_REDUCTION ; // AMP WEIGHTED BY HAMMING EXPERIMENT
               // SUM WEIGHTED FREQ 
	    static_freqresponse_averages[ static_freqresponse_averages_base_index  + i ] += 
			(thisFreqresponseFrame[ i ] * thisAmpWeight) ; 
               // SUM AMP WEIGHT
	    ampWeightSums[ static_freqresponse_averages_base_index  + i - 1 ] += thisAmpWeight ; 
	} ; 
pd( 282 ); 

	// AMPS
	for( i = 0; i < analysis_Nplus2; i += 2 ){ 
	    static_freqresponse_averages[ static_freqresponse_averages_base_index + i ] += 
			(thisFreqresponseFrame[ i ] * thisHammingWindowValue  * AMP_REDUCTION) ; // AVG EXPERIMENT
	} ; 

pd( 283 ); 

//fprintf( stderr, "\n static_freqresponse_index: %d", static_freqresponse_index ) ; 

	static_freqresponse_averages_sum_count[static_freqresponse_index]++ ; 
	
    } ; 

pd( 29 ); 

    // ***********  TURN SUMS INTO AVERAGES
    for(dBlevelFreqresponse = 0, j = 0; dBlevelFreqresponse < numOfStaticFreqresponseAverages; dBlevelFreqresponse ++, j += analysis_Nplus2 ){
		// j IS BASE INDEX

	if( static_freqresponse_averages_sum_count[dBlevelFreqresponse] > 0 ){
	    // AT LEAST ONE
	    for(k = 0, l = 1; k < analysis_Nplus2; k += 2, l += 2){

		// AMPS
		static_freqresponse_averages[j + k] /= (float) static_freqresponse_averages_sum_count[ dBlevelFreqresponse ] ; 
		// FREQS
		if( ampWeightSums[j + k] > 0. )
		    static_freqresponse_averages[j + l] /= ampWeightSums[j + k] ; 
		else
		    static_freqresponse_averages[j + l] = analysis_fundamental * (float)(dBlevelFreqresponse / 2) ; 
	    };      
	}; 
    }; 

 
/*
fscratch = fopen( "/tmp/static_freqresponse_averages_BEFORE",  "w" ) ;     
// ???
for( i = 0; i < (numOfStaticFreqresponseAverages * analysis_Nplus2); i += 2 ){ 
	temp = amp_to_dB( static_freqresponse_averages[i] );
	if( temp < -200. ) temp = -200. ;  
	fwrite( &temp, sizeof(float), 1, fscratch );
} ; 
fclose( fscratch );  
*/

    // ***********  INTERPOLATE BOTH AMPS AND FREQS TO CREATE MISSING AVERAGES
    iFlag = 0 ; 
    for(dBlevelFreqresponse = 0; dBlevelFreqresponse < numOfStaticFreqresponseAverages; dBlevelFreqresponse ++ ){// j IS BASE INDEX
	if( static_freqresponse_averages_sum_count[dBlevelFreqresponse] == 0 ){
	    if( iFlag == 0 ){ start = dBlevelFreqresponse - 1 ; iFlag = 1 ; }; 	    
	} else{
	    if( iFlag == 1 ){ 
		end = dBlevelFreqresponse ; iFlag = 0 ; 

		for( k = start + 1, l = 1 ; k < end; k++, l++  ){
		    mult = (float)(l) / (float)(end - start) ; 

	    	    for(amp = 0, freq = 1; amp < analysis_Nplus2; amp += 2, freq += 2){
			// AMPS
			static_freqresponse_averages[ (k * analysis_Nplus2) + amp] = 
			    ((1.0 - mult) * static_freqresponse_averages[(start * analysis_Nplus2) + amp]) + 
				(mult * static_freqresponse_averages[(end * analysis_Nplus2) + amp]) ; 
			// FREQS
			static_freqresponse_averages[ (k * analysis_Nplus2) + freq] = 
			    ((1.0 - mult) * static_freqresponse_averages[(start * analysis_Nplus2) + freq]) + 
				(mult * static_freqresponse_averages[(end * analysis_Nplus2) + freq]) ; 
		    } ; 
		} ; 
	    };      
	} ; 
    } ; 

pd( 30 ); 

    // ***********  SMOOTH DIFFERENCES BY WINDOW AVERAGING WITH A HAMMING WINDOW ACROSS A DB RANGE.
    for(dBlevelFreqresponse = 0; dBlevelFreqresponse < (numOfStaticFreqresponseAverages - 1); dBlevelFreqresponse ++ ){// j IS BASE INDEX
       for(l = 0; l < analysis_Nplus2; l++){
            valueSum = 0. ; windowAmpSum = 0. ; 
            for(k = (dBlevelFreqresponse - 5), n = 0; k <= (dBlevelFreqresponse + 5); k++, n++ ){
                if( (k >= 0) && (k <= (numOfStaticFreqresponseAverages - 1)) ){
                    temp = HammingWindow[ (int)( ((float) n / 11. ) * (float)(HammingWindowSize - 1) ) ] ; 
                    valueSum += (temp * static_freqresponse_averages[ (k * analysis_Nplus2) + l ] ); 
                    windowAmpSum += temp ;                     
                } ; 
            } ; 
            static_freqresponse_averages[ (dBlevelFreqresponse * analysis_Nplus2) + l ] = valueSum / windowAmpSum ; 
        } ; 	

    } ;     

pd( 31 ); 

    //  ***********  FIND THE PEAK AMPLITUDE BIN FOR EACH BAND ACROSS ALL FRAMES.
    //  ***********  USE THIS BIN AS THE REPOSITORY OF THE PEAK AMPLITUDE AND AMP-WEIGHTED FREQUENCY
    //   ***********  FOR ALL FRAMES. SET ALL OTHER BINS IN A BAND TO ZERO AMPLITUDE. SET THE THRESHOLD 
    //  ***********  CURVES IN ACCORD WITH THEIR PEAK OR ZERO AMPLITUDE.  
    // FOR EACH BAND
    for(band = 0; band < numberOfBands; band++ ){
        // FIND THE STRONGEST BIN IN BAND ACROSS ALL DECIBEL SPECTRA
        // FOR EACH BIN IN BAND...
        strongestSum = -9999999. ;
        for( bin = partial_Band_Begin[ band ]; bin < partial_Band_Begin[band + 1] ; bin++ ){
            // MAKE SUM FOR BIN
            thisSum = 0. ;  
            for(dBlevelFreqresponse = 0; dBlevelFreqresponse < (numOfStaticFreqresponseAverages - 1); dBlevelFreqresponse++ ){ 
                thisSum += 
                    static_freqresponse_averages[ (dBlevelFreqresponse * analysis_Nplus2) + indexInChannelForHarmony[ bin ] ] ; 
            } ; 
            // TEST SUM AGAINST LAST SUM
            if( thisSum > strongestSum ){ strongestBin = bin ; strongestSum = thisSum ; } ; 
        } ; 

        // SAVE STRONGEST BIN IN THIS BAND
        harmonyBands_IndexOfStrongestBinInBand[ band ] = strongestBin ; 

        // FOR EACH DECIBEL SPECTRUM
        averageFreqForThisBand = 0. ; thisAmpWeightSum = 0. ; 
        for(dBlevelFreqresponse = 0; dBlevelFreqresponse < (numOfStaticFreqresponseAverages - 1); dBlevelFreqresponse ++ ){
            // FOR THE BINS OF THIS BAND, FIND THE PEAK AMPLITUDE AND CORRESPONDING FREQUENCY.
            peakAmp = -99999999. ; weightedFreqSum = 0. ; weightsSum = 0. ;  
            for( bin = partial_Band_Begin[ band ]; bin < partial_Band_Begin[band + 1] ; bin ++ ){
                temp = static_freqresponse_averages[ (dBlevelFreqresponse * analysis_Nplus2) + indexInChannelForHarmony[ bin ] ] ; 
                if( temp > peakAmp ){
                    peakAmp = temp ;
                    thisFreq =  static_freqresponse_averages[ (dBlevelFreqresponse * analysis_Nplus2) + indexInChannelForHarmony[ bin ] + 1] ; 
                } ; 
            } ; 
            // ADD TO MAKING OF AVERAGE FREQ
            averageFreqForThisBand +=  (thisFreq * peakAmp) ; thisAmpWeightSum += peakAmp ; 
            // ZERO AMPLITUDE OF ALL BINS EXCEPT FOR THE STRONGEST BIN; IN IT PUT thisFreq AND peakAmp. 
            for( bin = partial_Band_Begin[ band ]; bin < partial_Band_Begin[band + 1] ; bin ++ ){
                if( bin == harmonyBands_IndexOfStrongestBinInBand[ band ] ){

                    // SAVE THE FORCE CURVE FOR THIS PRINCIPAL BIN.
                    if( dBlevelFreqresponse == 0 )harmony_force_curve_indeces[ bin ] = 1. ; // 1

                    // SAVE PEAK AMP AND FREQ FOR PEAK AMP FOR THE STRONGEST BIN IN THIS BAND FOR THIS STATIC FREQRESPONSE.
                    harmonyBandsForStaticFreqResponses_PeakAmpOfStrongestBinInBand[ (dBlevelFreqresponse * numberOfBands) + band ] = peakAmp ;  

               }else{
                    // SAVE THE FORCE CURVE FOR THIS SUPPRESSED BIN.
                    if( dBlevelFreqresponse == 0 )harmony_force_curve_indeces[ bin ] = 5. ; // 5
                } ; 
            } ; 
        } ; 
        // MAKE BAND AMP-WEIGHTED AVERAGE FREQ
        averageBandFreq[ band ] = averageFreqForThisBand / thisAmpWeightSum ; 
         
    } ; 
pd( 32 ); 


} ;  




void getVibratoValuesAndIncrement(
    float vibValNow_TONE_VALUES[],
    int numTones,
    float synthetic_Vibrato_Rate_TONE_VALUES[],
    float synthetic_Vibrato_Randomization_Prop_TONE_VALUES[],
    int incrementFlag,

    float timeWarp[],
    float oldTimeWarp[],
    float ranAmpScale[],
    float oldRanAmpScale[],
    float rateMod[], 
    float oldRateMod[], 
    float vibPhaseNow[],
    float vibratoPeriodTable[],
    float vibratoPeriodTableSize

)
{
    int i, toneIndex ; 
    float x, rateNow, vibPhaseWithTimeMod, coef=0.66 ; 

    static int first=1 ;

    // MAKE TABLE
    if( first == 1 ){

	for( i = 0; i < ((int) vibratoPeriodTableSize + 1); i++ ){
	    x = ((float) i / (1. + vibratoPeriodTableSize) ) ; 
	    x = curve( 0., (TWOPI), x, -2. );
	    vibratoPeriodTable[ i ] = 1.0 - (.5 * ((float) cos( (double) x ) + 1.)) ; 
	    vibratoPeriodTable[ i ] = curve( 0., 1., vibratoPeriodTable[ i ], -2 ); 
	    if( (vibratoPeriodTable[ i ] > 1.0) || (vibratoPeriodTable[ i ] < 0.) )vibratoPeriodTable[ i ] = 1. ; 

	} ; 

	first = 0 ; 
    } ; 


    if( frame_count == 0 ){
prt( "RESETTING VIBRATO VALUES" ) ; 
	for( toneIndex = 0; toneIndex < numTones; toneIndex++ ){
		timeWarp[ toneIndex ] = 0. ; 
		oldTimeWarp[ toneIndex ] = 0. ; 
		ranAmpScale[ toneIndex ] = 1. ;  
		oldRanAmpScale[ toneIndex ] = 1. ;  
		rateMod[ toneIndex ] = 1. ;  
		oldRateMod[ toneIndex ] = 1. ;  
	} ; 
    } ; 

    for( toneIndex = 0; toneIndex < numTones; toneIndex++ ){

        if( frame_count == 0 ){
	vibPhaseNow[ toneIndex ] = 0. ; 
	srand( 0 ) ; 
        } ; 

        vibPhaseWithTimeMod = curve( 0., 1., vibPhaseNow[ toneIndex ], timeWarp[ toneIndex ] ) ;
        vibValNow_TONE_VALUES[ toneIndex ] = ranAmpScale[ toneIndex ]  * 
		vibratoPeriodTable[ (int)( 0.5 + (vibPhaseWithTimeMod * vibratoPeriodTableSize) ) ] ; 

        if( vibValNow_TONE_VALUES[ toneIndex ] > 1.0 ) vibValNow_TONE_VALUES[ toneIndex ] = 1.0 ; 
        if( vibValNow_TONE_VALUES[ toneIndex ] < 0.0 ) vibValNow_TONE_VALUES[ toneIndex ] = 0.0 ; 

        if( incrementFlag == 1 ) {
	rateNow = synthetic_Vibrato_Rate_TONE_VALUES[ toneIndex ] ; 
	rateNow *= rateMod[ toneIndex ] ;  
	vibPhaseNow[ toneIndex ] = vibPhaseNow[ toneIndex ] + (rateNow / frames_per_sec ) ; 

	if( vibPhaseNow[ toneIndex ] > 1.0 ) {
	    vibPhaseNow[ toneIndex ] = vibPhaseNow[ toneIndex ]  - floor( (double) vibPhaseNow[ toneIndex ] ) ; 

	    oldTimeWarp[ toneIndex ] = timeWarp[ toneIndex ] ; 
	    timeWarp[ toneIndex ] = (4. * synthetic_Vibrato_Randomization_Prop_TONE_VALUES[ toneIndex ]) * ((float)(rand() % 1000) / 1000.0 ) ;
	    timeWarp[ toneIndex ] = (coef * timeWarp[ toneIndex ]) + ((1. - coef) * oldTimeWarp[ toneIndex ]) ; 

	    oldRanAmpScale[ toneIndex ] = ranAmpScale[ toneIndex ] ; 
	    ranAmpScale[ toneIndex ] = 
		1.0 - curve(0., (0.5 * synthetic_Vibrato_Randomization_Prop_TONE_VALUES[ toneIndex ]), (float)(rand() % 1000) / 1000.0, 2. );  
	    ranAmpScale[ toneIndex ] = (coef * ranAmpScale[ toneIndex ]) + ((1. - coef) * oldRanAmpScale[ toneIndex ]) ; 

	    oldRateMod[ toneIndex ] = rateMod[ toneIndex ] ; 
	    rateMod[ toneIndex ] = curve( 1. - (.2 * synthetic_Vibrato_Randomization_Prop_TONE_VALUES[ toneIndex ]), 
		1. + (.2 * synthetic_Vibrato_Randomization_Prop_TONE_VALUES[ toneIndex ]),  (float)(rand() % 1000) / 1000., 0. );
	    rateMod[ toneIndex ] = (coef * rateMod[ toneIndex ]) + ((1. - coef) * oldRateMod[ toneIndex ]) ; 
	} ; 
        } ;     
    } ; 

}; 

	 
void usage()
{
    fprintf(stderr, "%s",
	"chordmapperplus:  time-varying, mutiple harmonic tone mapper \n"
	"chordmapperplus   [flags]  [output file ]\n"
	"	    Pre-formatted output sound file required.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	I:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec

	"	D:	"OUTPUT_DURATION		// dur



	"	C:	"RESYNTHESIS_CHANNEL		// channelout
	"	A:	"DB_GAIN				// master_gain_in_dB
 	"	f:	"PHASE_VOCODER_ANALYSIS_FILE		// analysis

	"	    "DATA_TIME_HEADER
	"	x:	"TIME_POINT_ORIGIN
	"	Y:	"RATE_MULTIPLIER

	"	g:	"TIME_WINDOW_LOW_BOUNDARY
	"	k:	"TIME_WINDOW_HIGH_BOUNDARY

	"	S:	"TIME_WINDOW_MODE





	"	    "SAMPLER_HEADER

	"	z:	playback mode: 0 = sampler, 1 = autostop [0] \n"

	"	Q:	"LOOP_MODE

	"	@:	"ONSET_RELEASE_SWITCH
	"	/:	"LOOP_SMOOTH_TIME
	"	e:	"LOOP_NORMALIZATION_SWITCH
 
	"	    SYNTHETIC VIBRATO\n"
	"	r:	synthetic vibrato rate (func) [0.] \n"
	"	v:	synthetic vibrato randomization control 0-1 (func) [0.]\n"

	"	    NATURAL VIBRATO\n"
	"	R:	automatic vibrato period detection and time boundary selection\n"
	"		randomization, 0 = off, 1 = on [0]\n"
	"	V:	vibrato period duration detection variance threshold as proportion [.05]\n"

	"	    SOURCE\n"
	"	s:	source switch:  0 = omit source, 1 = include source[0]\n"
	"	a:	source frequency shift factor \n"
	"		    (bin frequency adder, before -P )(func)[0.] \n"
	"	P:	source pitch transposition in semitones (func) [0.]\n"
	"	G:	source gain control in decibels (func) [0.] \n"
	"	O:	source frequency analysis adjustment switch 0 = off, 1 = on [0.] \n"
	"	h:	frequency stasis suppression threshold rise response time in seconds (func)[0.] \n"



	"	    TONES\n"
	"	       Master Controls:\n"
	"	q:	tones master frequency shift (func) [0.] \n"
	"	X:	tones master pitch transposition in semitones (func) [0.]\n"
	"	m:	tones master gain in decibels (func) [0.] \n"
	" 	       Parameter Field Interpolation Controls:\n"
	"	c:	tones amplitude interpolation control,  0-1 (func) [0]\n"
	"	B:	Rate-correlated force suppression/stasis compensation switch 0 = off, 1 = on [0]\n"
	"			Switched on, playback rate multipliers less than 1 attenuate\n"
	"			force level while adjusting median stasis levels, in both tone and noise,\n"
	"			to compensate for the change in amplitude. A rate multiplier of\n"
	"			0 (stationary time) neutralizes all force, resulting in a static, stasis\n"
	"			median sound, adjusted in level to correspond to the original force level.\n"
	"	F:	ASCII file of tone data (unordered groups of 20 parameters):\n"
	"		     Each consisting of: \n"
	"			(1) source point (see -Q source point data format below)\n"
	"			(2, 3) shift factor or point: base, peak (see -Z shift data format below)\n"
	"			(4) lowest partial number\n"
	"			(5) partial spacing as proportion of fundamental frequency\n"
	"			(6) number of partials (0 = all partials below Nyquist frequency)\n"
	"				positive values: pass mode\n"
	"				    (include the selected partials)\n"
	"				negative values: reject mode\n"
	"				    (include all partials except the selected ones) \n"
	"			(7) partial shift factor as proportion of fundamental frequency\n"
	"			(8) bandwidth as proportion of fundamental frequency\n"
	"			(9, 10) decibels: base, peak \n"
	"			(11, 12) force: base, peak\n"
	"			(13, 14) stasis median position: base, peak\n"
	"			(15, 16) bandpass center frequency: base, peak\n"
	"			(17, 18) bandpass rolloff in dB/octave: base, peak\n"
	"			(15) transposition switch\n"
	"			    1 = use master tones transpose function,  0 = bypass transpose function\n"
	"			(16) synthetic vibrato switch: 0 = off, 1 = on\n"
	"	      Rate Correlated Controls:\n"
	"	B:	rate correlated force suppression switch: 0 = off, 1 = on [0]\n"
	"	T:	rate correlated tone level control in dB (func)[0]\n"
	"	E:	rate correlated noise level control in dB (func)[0]\n"
	"	H:	rate correlated randomization switch: 0 = off, 1 = on [0]\n"

	"	L:	noise band limit level in dB (func)[0]\n"
	"	b:	noise band limit level rolloff in dB per channel (func)[0]\n"

	"	u:	Fundamental frequency for vibrato periods detection. [0]\n"
	"	U:	vibrato period duration variation: [1]\n"
	"		    0 = mechanical synchronization\n"
	"		    1 = natural variation\n"

	"	n: 	auto-adjust partial positions and bandwidths 0 = off, 1 = on [0]\n"
	"	o:	auto-adjust decibel stopband level [-20.]\n"
	"	l:	auto-adjust partial inclusion threshold in dB [-100.]\n"


	"	t:	"RESYNTH_THRESHOLD		// threshfacdB
	"	p:	"AMP_REPORTS		// quiet 
	"	i:	"AMP_REPORTS_TIME_INTERVAL	// ampstatinc 

	"	_:	 "AUTO_PLAY		// autoplayreps

	"	=:	 "RESCALE_LEVEL		// rescalev


	); // DONE
    exit(EXIT_SUCCESS);
}

float find_fundamental_frequency(
	struct func *analysis,
	struct func *pitchTrackFile, 
	int channelout,
	int N, 
	int D
)
{
	char tempPitchTrackName[ STRING_SIZE ], tempstring2[ STRING_SIZE ], *user ; 
	
	float analyzedFundamental ;
	FILE *fopen(); 

	float low, hi, range, average, median, mode, standarddeviation, sum, begin, end, middle ; 


	struct func pitchTrack ; 

	pitchTrack.L = 1. ; pitchTrack.n = 1. ; pitchTrack.A[ 0 ] = 0. ; 



   // FIND FUNDAMENTAL FOR USE WITH UNSPECIFIED SOURCE POINTS


	user = getlogin() ; 

// FROM HERE ------------------

//
    prt( "USING pitchtracker AND ORIGINAL SOUND FILE TO FIND FUNDAMENTAL FREQUENCY OF TONE\nIN ANALYSIS FILE FOR USE WITH UNSPECIFIED SOURCE POINTS . . . . . . . . . . "  ) ;  
    prs( analysis->fname, "ANALYSIS FILE" ) ; 
    prs( afile, "ORIGINAL SOUND FILE" ) ; 
    srandom( time(NULL) ) ; 
    sprintf( tempPitchTrackName, "/tmp/pitchtrack.%s.%d", user, (int)(random()) ) ;

	filesToRemove( tempPitchTrackName, 0 ) ;

	// 2. DONE // NOT BEING USED ANYMORE! FINDING FUNDAMENTAL FOR UNSPECIFIED SOURCE POINTS
    sprintf( tempstring2,  
	"pitchtracker -N%d -M0 -D%d -b%f -e%f -C%d -f%f -F%f -L.0 -l.0 -W0 -G-0 -T-0 -S-80 -r200 -g1 -m0 -j.05 -J.1 -d-25 -O0 -o500 -a0. -X0 -p1 -P0 -E10 -H0.5 %s %s", 
		N, 200, (float) begint, (float) endt, channelout, 20., 5000., afile, tempPitchTrackName ) ; 

    system( tempstring2 ) ; 

	pitchTrack.fp =  crackstring( tempPitchTrackName, &pitchTrack ) ; 

	//prt("PITCH TRACK STATISTICS IN HERZ:" ) ; 

	findFuncStats( &pitchTrack, pitchTrack.L, &low, &hi, &range, &average, &median, &mode, 
			&standarddeviation, &sum, &begin, &end, &middle, 1, 1 ) ; 
	prt(""); prt(""); prt("");
	fclose( pitchTrack.fp ) ; 
    analyzedFundamental = median ; 

    fprintf( stderr, 
		"\nINITIAL ANALYZED FUNDAMENTAL FREQUENCY: %f\t(octave.pclass: %f)", 
			analyzedFundamental, Hz_to_OPPC( analyzedFundamental ) ) ; 


	return( analyzedFundamental ) ; 


} ; 


float smooth_frequency_change( 
    float A[], 
    float old_A[], 
    int Nplus2, 
    float att,
    float matt, 
    float rel, 
    float mrel
     
    )

{    

    int i ; 

   if( frame_count == 0 ){
	for(i = 0; i < Nplus2; i++ ) old_A[i] = A[i] ; 
//	prt( "FIRST FRAME: FILLING PREVIOUS ARRAY." ) ; 
//	for( i = 0 ; i < 100; i++ ) 
//		prf( old_A[i], "old_A[i]" ) ; 
    } ; 

	//*********ATTACK/RELEASE:

  if( (rel != 0.) || (att != 0.) ){

	for( i = 1; i < Nplus2; i+= 2 ){

		if((A[i] < old_A[i]) ){
		    // RELEASE
		    A[i] =  
			(rel * old_A[i] ) + ( mrel * A[i] ); 
		} else {
		    // ATTACK
		    A[i] =  
			(att * old_A[i] ) + ( matt * A[i] ); 

		}
		
	}
  }
  
  for( i = 1; i < Nplus2; i += 2 ) old_A[i] = A[i] ;

  return( 1 ) ; 

}


// *****


float normalizeLoopAmplitudesForChordmapperplus(
    int LoopNormalizationFlag,
    int Mode__sampler_loop_0__autostop_1, 
    struct func *filtwinlow, 
    struct func *filtwinhi,
    struct func *filter,
    float F_lower[], 
    float F_higher[],
    float tempChannel[],
    float channel[],
    float iframes_per_sec,
    int analysis_Nplus2,
    int ainchan,
    int analysis_chan,
    float analysis_dur, 
    float filttnow
)
{
    static int firstLowBoundaryChannelAmp ; 
    static int firstHighBoundaryChannelAmpSum ;
    static int findNewPeakBoundaryAmpSum ; 
    static float lowBoundaryAmpSum ; 
    static float highBoundaryAmpSum ; 
    static float peakBoundaryAmpSum ; 
    static float channelAmpSum ;
    static float prop ;   
    static float lowGainScaleInDB ; 
    static float highGainScaleInDB ;    
    static float gainScale=1.0 ;
    static int i ;
    static float last_filtwinlow=-1. ;
    static float last_filtwinhi=-1. ;


    if( t == 0. ){
	firstLowBoundaryChannelAmp = 1 ; 
	firstHighBoundaryChannelAmpSum = 1 ; 
	findNewPeakBoundaryAmpSum = 1 ;
  }; 

	if( (Mode__sampler_loop_0__autostop_1 == 0) && (LoopNormalizationFlag == 1) ){
		// NORMALIZATION ON
		// FIND BOUNDARY AMP SUMS
		if( (firstLowBoundaryChannelAmp == 1) || (filtwinlow->A[ 0 ] != last_filtwinlow)){
//fprintf( stderr, "RESETTING IN LOOP NORMALIZATION AT %f\n", t ) ; 
	    		// GET FRAME FOR LOW BOUNDARY
	    		makeInterpolatedFilterFrame( filter, F_lower, F_higher, tempChannel,
				iframes_per_sec, analysis_Nplus2, filtwinlow->A[ 0 ], ainchan, analysis_chan
	    			) ; 
	    		// MAKE LOW BOUNDARY AMP SUM.
	    		lowBoundaryAmpSum = 0.0 ; 
	    		for(i = 0; i < analysis_Nplus2; i += 2) lowBoundaryAmpSum += tempChannel[i] ; 
	    		if( filtwinlow->n == 1 )  firstLowBoundaryChannelAmp = 0 ; 
	    		findNewPeakBoundaryAmpSum = 1 ; 
		}; 
		if( (firstHighBoundaryChannelAmpSum == 1) || (filtwinhi->A[ 0 ] != last_filtwinhi) ){
//fprintf( stderr, "RESETTING IN LOOP NORMALIZATION AT %f\n", t ) ; 
			// GET FRAME FOR HIGH BOUNDARY
	    		makeInterpolatedFilterFrame( filter, F_lower, F_higher, tempChannel,
				iframes_per_sec, analysis_Nplus2, filtwinhi->A[ 0 ], ainchan, analysis_chan
	    			) ; 
	    		// MAKE HIGH BOUNDARY AMP SUM.
	    		highBoundaryAmpSum = 0.0 ; 
	    		for(i = 0; i < analysis_Nplus2; i += 2) highBoundaryAmpSum += tempChannel[i] ; 
	    		if( filtwinhi->n == 1 ) firstHighBoundaryChannelAmpSum = 0 ; 
	    		findNewPeakBoundaryAmpSum = 1 ; 
		}; 
		if( findNewPeakBoundaryAmpSum == 1){
	    		if( lowBoundaryAmpSum > highBoundaryAmpSum ) peakBoundaryAmpSum = lowBoundaryAmpSum ; 
	    		else peakBoundaryAmpSum = highBoundaryAmpSum ; 
	    		findNewPeakBoundaryAmpSum = 0 ; 
		}; 


		// FIND AMP SUM
		channelAmpSum = 0.0 ; 
		for( i = 0; i < analysis_Nplus2; i += 2) channelAmpSum += channel[i]; 

		// NORMALIZE
		if( channelAmpSum > 0.0 ){
			if( filttnow <= filtwinlow->A[ 0 ] ){
	     		// ONSET SEGMENT
	        		prop = filttnow / filtwinlow->A[ 0 ] ; 
	        		lowGainScaleInDB = 0.0 ; 
				highGainScaleInDB = amp_to_dB( peakBoundaryAmpSum / lowBoundaryAmpSum )  ; 
	        		gainScale = 
					dB_to_amp( lowGainScaleInDB + (prop * (highGainScaleInDB - lowGainScaleInDB)) ) ; 
	    		}else if( filttnow >= filtwinhi->A[ 0 ] ){
	        		// RELEASE SEGMENT
	        		prop = ((filttnow - filtwinhi->A[ 0 ]) / (analysis_dur - filtwinhi->A[ 0 ])) ; 
	        		lowGainScaleInDB = amp_to_dB( peakBoundaryAmpSum / highBoundaryAmpSum ) ; 
				highGainScaleInDB = 0.0 ; 
	        		gainScale = 
					dB_to_amp( lowGainScaleInDB + (prop * (highGainScaleInDB - lowGainScaleInDB)) ) ; 
	    		}else{
	        		// LOOP
	        		gainScale = peakBoundaryAmpSum / channelAmpSum ; 
	    		}; 
	
	    		for( i = 0; i < analysis_Nplus2; i += 2) channel[i] *= gainScale ; 
		}; 
    };     

	last_filtwinlow = filtwinlow->A[ 0 ] ; last_filtwinhi = filtwinhi->A[ 0 ] ; 

	return( amp_to_dB( gainScale ) ); 
} ; 


void findJumpPointGainScales(
	float lowMinimaReleaseVibratoPoint,
	float highMinimaReleaseVibratoPoint,
	float releaseVibratoPeriodJumpPoint,
    struct func *filter,
    float F_lower[], 
    float F_higher[],
    float tempChannel[],
    float iframes_per_sec,
    int analysis_Nplus2,
    int ainchan,
    int analysis_chan,
	float *lowMinimaReleaseVibratoPointGainScale_inDecibels,	
	float *highMinimaReleaseVibratoPointGainScale_inDecibels	

){
	float lowMinimaReleaseVibratoPointAmpSum ; 
	float highMinimaReleaseVibratoPointAmpSum ;
	float jumpPointAmpSum ;
	int i ; 

	// MAKE LOW FRAME
	makeInterpolatedFilterFrame( filter, F_lower, F_higher, tempChannel,
		iframes_per_sec, analysis_Nplus2, lowMinimaReleaseVibratoPoint, ainchan, analysis_chan
	) ; 
	// MAKE LOW BOUNDARY AMP SUM.
	lowMinimaReleaseVibratoPointAmpSum = 0.0 ; 
	for(i = 0; i < analysis_Nplus2; i += 2) lowMinimaReleaseVibratoPointAmpSum += tempChannel[i] ; 

	// MAKE HIGH FRAME
	makeInterpolatedFilterFrame( filter, F_lower, F_higher, tempChannel,
		iframes_per_sec, analysis_Nplus2, highMinimaReleaseVibratoPoint, ainchan, analysis_chan
	) ; 
	// MAKE HIGH BOUNDARY AMP SUM.
	highMinimaReleaseVibratoPointAmpSum = 0.0 ; 
	for(i = 0; i < analysis_Nplus2; i += 2) highMinimaReleaseVibratoPointAmpSum += tempChannel[i] ; 

	// MAKE JUMP POINT FRAME
	makeInterpolatedFilterFrame( filter, F_lower, F_higher, tempChannel,
		iframes_per_sec, analysis_Nplus2, releaseVibratoPeriodJumpPoint, ainchan, analysis_chan
	) ; 
	// MAKE JUMP POINT AMP SUM.
	jumpPointAmpSum = 0.0 ; 
	for(i = 0; i < analysis_Nplus2; i += 2) jumpPointAmpSum += tempChannel[i] ; 

	*lowMinimaReleaseVibratoPointGainScale_inDecibels = 
		amp_to_dB( jumpPointAmpSum / lowMinimaReleaseVibratoPointAmpSum ) ;	
	*highMinimaReleaseVibratoPointGainScale_inDecibels = 
		amp_to_dB( jumpPointAmpSum / highMinimaReleaseVibratoPointAmpSum ) ;	
	


} ;