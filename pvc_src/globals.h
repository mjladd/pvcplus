#include <stdio.h>
#include <sndfile.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "stdbool.h"

#include <time.h>
#include "../pvc_lib/pv.h"


#define flog10 log10
#define log10f log10
#define powf pow

// DEFINES FOR PRINT OF USAGE FLAGS
#define FFT_LENGTH "FFT length (must be a power of 2) [1024]\n"

#define WINDOW_SIZE "window size in samples (must be a power of 2) [2*FFT]\n"\
"\t\t    (0 will automatically set window to 2*FFT size or larger)\n"

//#define WINDOW_SIZE2 "    (0 will automatically set window to 2*FFT size or larger)\n"

#define WINDOW_TYPE "window type: 0 = hamming,  1 = rectangular  \n"\
"\t\t    2 = Blackman,  3 = Bartlett triangular [0.]\n"\
"\t\t    4-12 = Kaiser windows for alpha = 4-12,  respectively\n"\
"\t\t    (representative sidelobe levels for alpha: \n"\
"\t\t     4 = -30dB,  8 = -58 dB,  12 = -90 dB)\n"\
"\t\t    13 = Blackman-Harris, 14 = Nuttal, 15 = Blackman-Nuttal\n"\
"\t\t    16 = Flat top\n"

#define ANALYSIS_FRAMES_PER_SEC "analysis frames per second [200]\n"

#define OUTPUT_DURATION "output duration in seconds \n"

#define TRANSPOSITION_SHIFT "TRANSPOSITION/SHIFT application FLAG [0]\n"\
"\t\t    Apply -P and -a to:\n"\
"\t\t     source only -- prefilter (0),\n"\
"\t\t     or to  source and filter -- postfilter (1)\n"

#define  FREQUENCY_RESPONSE_PRINTOUT "FREQUENCY RESPONSE PRINTOUT: high cutoff frequency in Hz [0]\n"\
"\t\t     (0 = off)\n"

#define FILTER_FREQ_RESPONSE_TRANSP "FILTER: frequency response transposition in semitones (func) [0]\n"
#define FILTER_FREQ_RESPONSE_SHIFT "FILTER: frequency response shifter (func) [0.] \n"
#define FILTER_DB_FLOOR "FILTER: source decibels floor \n"\
"\t\t     (reduces filtered signal in proportion)(func) [0.] \n"

#define TIME_FACTOR "time expansion/contraction factor  [1.] \n"\
"\t\t  (duration = duration * factor, 1. = original time) \n"


#define BEGIN_TIME "begin time in seconds  [0.] \n"
#define END_TIME "end time in seconds ( 0. = end of file) [0.] \n"
#define RESYNTHESIS_CHANNEL "resynthesis channel (1 -> ?) (0 = all) [0] \n"

#define SHELF_EQ_HEADER "SHELF EQ:(post transpose/shift)\n"
#define SHELF_EQ_LOW_GAIN "shelf EQ: Low shelf gain in dB (func) [0.] \n"
#define SHELF_EQ_HIGH_GAIN "shelf EQ: High shelf gain in dB (func) [0.] \n"
#define SHELF_EQ_LOW_FREQ "shelf EQ: Low shelf frequency in Hz (func) [200.] \n"
#define SHELF_EQ_HIGH_FREQ "shelf EQ: High shelf frequency in Hz (func) [2000.] \n"

#define FILTER_SHELF_EQ_HEADER "FILTER FREQUENCY RESPONSE -- SHELF EQ:(post transpose/shift)\n"
#define FILTER_SHELF_EQ_LOW_GAIN "filter shelf EQ: Low shelf gain in dB (func) [0.] \n"
#define FILTER_SHELF_EQ_HIGH_GAIN "filter shelf EQ: High shelf gain in dB (func) [0.] \n"
#define FILTER_SHELF_EQ_LOW_FREQ "filter shelf EQ: Low shelf frequency in Hz (func) [200.] \n"
#define FILTER_SHELF_EQ_HIGH_FREQ "filter shelf EQ: High shelf frequency in Hz (func) [2000.] \n"

#define WARP_INDEX "warp index for reshaping magnitude response (func) [0.] \n"\
"\t\t    Values > 0 expand the dynamic range, \n"\
"\t\t    values < 0 compress the dynamic range. \n"

#define PHASE_VOCODER_ANALYSIS_FILE "phase vocoder analysis file \n"

#define DATA_TIME_HEADER "DATA TIME\n"

#define TIME_POINT_ORIGIN "time point origin (func) [0.] \n"

#define RATE_MULTIPLIER "rate multiplier (func) [1.]\n"\
"\t\t    (1. = rate of original, 2. = twice as fast, etc.)\n"\
"\t\t    (negative = reverse,  0 = stationary) \n"

#define TIME_WINDOW_LOW_BOUNDARY "time window: lower boundary (func) [0.] \n"
#define TIME_WINDOW_HIGH_BOUNDARY "time window: upper boundary (if < 0,  then analysis duration) \n"\
"\t\t						(func) [end of file] \n"

#define TIME_WINDOW_MODE "time window mode: 0 = autostop at boundary, 1 = sampler loop [0]\n"

#define SAMPLER_HEADER "SAMPLER\n"

#define LOOP_MODE "loop mode: [0]\n"\
"\t\t   0 = wrap time into window\n"\
"\t\t   1 = fold time into window bounds\n"\
"\t\t   2 = clip or limit time to nearest window boundary\n"

#define ONSET_RELEASE_SWITCH "onset and release switch: 0 = off, 1 = on [0]\n"
#define LOOP_SMOOTH_TIME "peak loop seam lowpass filter smoothing time (func) [0]\n"
#define LOOP_NORMALIZATION_SWITCH "loop amplitude normalization switch (using boundary levels and interpolation)\n"\
"\t\t	0 = off, 1 = on [0]\n" 

#define SOURCE_HEADER "SOURCE\n"
#define SOURCE_DB_GAIN "source gain in decibels (func) [0.] \n"
#define SOURCE_PITCH_TRANS "source pitch transposition in semitones (func) [0]\n"
#define SOURCE_FREQ_SHIFT "source frequency shift factor \n"\
"\t\t    (bin frequency adder, before pitch transposition )(func) [0.] \n"
#define SOURCE_TIME_DELAY "source time delay in seconds (func) [0.]\n"



#define DB_GAIN "gain in decibels (func) [0.] \n"
#define PITCH_TRANS "pitch transposition in semitones (func) [0]\n"
#define FREQ_SHIFT "frequency shift factor \n"\
"\t\t    (bin frequency adder, before pitch transposition )(func) [0.] \n"
#define TIME_DELAY "time delay in seconds (func) [0.]\n"

#define AMP_ATT_TIME "amplitude attack time  (func) [0.]\n"
#define AMP_RELEASE_TIME "amplitude release time   (func) [0.]\n"

#define AMP_REPORTS "amplitude reports print mode: 0 = off, 1 = on [0]\n" 
#define AMP_REPORTS_TIME_INTERVAL "time interval between amplitude reports [.25]\n" 

#define AUTO_PLAY "auto output sound file play:\n"\
"\t\t    0  = off \n"\
"\t\t   -1 = interactive: Prompt for each play.\n"\
"\t\t   -2 = interactive: Play once, then prompt for more.\n"\
"\t\t    1 or greater = Auto-repeat for specified repetitions. [0]\n"

#define RESCALE_LEVEL "peak rescale level 0 to -96 dB \n"\
"\t\t   1 = Rescale to level of input file.\n"\
"\t\t   2 = Bypass rescaling.\n"\
"\t\t   3 = Rescale only if peak exceeds 0dB. [ 1 ]\n"
 

#define FILTER_FREQ_RESPONSE_SMOOTHING_BW "FILTER FREQUENCY RESPONSE: smoothing bandwidth (in octaves or frequency) (func) [0]\n"\
"\t\t  (The value for each peak is  replaced with the average\n"\
"\t\t   value within the band, centered around the peak. Positive values\n"\
"\t\t  are interpreted as frequency bandwidths and negative values as\n"\
"\t\t    octave bandwidths, once made positive.\n"


#define RESYNTH_THRESHOLD "oscillator re-synthesis threshold in decibels [ -96 ]\n"

#define FRAME_NORMALIZATION "FRAME NORMALIZATION: \n"
#define FRAME_NORMALIZATION_DB_LIMIT "Frame Normalization Decibel Limit: (0-?) \n"\
"\t\t  Scale output frame amps to match or approach input frame amps\n"\
"\t\t  using the (sum of input amps)/(sum of output amps) limited to\n"\
"\t\t  the Decibel limit. 0 dB prevents normalization. [0]\n"
#define FRAME_NORMALIZATION_REFERENCE "Frame Normalization Reference: \n"\
"\t\t  0 = Changing Input Sound, 1 = Constant Filter [0]\n"


// GLOBALS
complex zero = { 0., 0. } ;
complex one = { 1., 0. } ;
float PI ;
float TWOPI ;
float synt ;

// FILE I/O 
char	    ifile[ STRING_SIZE ] ; 
char	    ofile[ STRING_SIZE ] ; 
char	    afile[ STRING_SIZE ] ; 
FILE	    *ifd, *ofd;

bool	    expandSingleChannelInputFileToMultipleDuplicateChannels=false ; 
//char	    ifileAlternate[  STRING_SIZE  ] ;

SNDFILE *infile, *outfile ;

char inputTempChanFileNames[ MAXIMUM_CHANNELS ][ STRING_SIZE ] ; 
FILE *inputTempChanFiles[ MAXIMUM_CHANNELS ] ; 

char outputTempChanFileNames[ MAXIMUM_CHANNELS ][ STRING_SIZE ] ; 
FILE *outputTempChanFiles[ MAXIMUM_CHANNELS ] ; 


// INPUT FILE HEADER INFO
float		idur;

float	ipeakamp[ MAXIMUM_CHANNELS ];

void setupfiles(int argc, char **argv)
;



int		window_type=0, 
		isr,
		ichan,
		idata_offset, 
		iformat, 
		outchan=0
;  
int
		IO_reset = 0, 
		sample=0, 
		begin_sample=0, 
		end_sample, 
		inbuffn=0, 
		doneflag=0, 
		quiet=0, 
		flinflag=0, 
		channelflag,
		beginchan,  
		endchan,   
		channow=0, 
		frame_count=0, 
		outputoff=0, 
		oscilbankon=0, 
		ttlsamps=0, 
		samps=0, 
		nsover[ MAXIMUM_CHANNELS ],
                 autoplayreps=0,
		debugFlag=0 
;

float
		begint=-1., 
		endt=-1.,
		ampstatinc=.25, 
		t=0., 
		tfactor=1, 
		frames_per_sec=200., 
		outdur   
; 

float		ringTime=0.0 ; 
int		ringTimeSamples=0 ; 


// OUTPUT FILE HEADER INFO
float		odur;
int		
		osr,
		ochan,
		ochanModified,
		odata_offset, 
		oformat
;  

void noscbank( float C[], int N, int R, int Nw, int I, float P, float O[] ) ;
void noscbank2( float C[], int N, int R, int Nw, int I, float P, float O[], float CC[],  int NC ) ;
void noscbank3( float C1[], int N1, int R, int Nw, int I, float P, float O[], float C2[],  int N2, float C3[], int N3 ) ;

int findFreqOfPeakFormant( 
    float SP[],  
    int N, 
    float *lowf, 
    float *hif,
    float *fundamental, 
    float *peakamp,
    float *freq
)
;

int invertresponse( 
    float SP[],  
    int N, 
    int normflag
    )
;

int outfile_setup(int argc, char **argv ) ;

int getInputFileDataToSetOutputChannels(int argc, char **argv) ;


int prp( struct func *p,  char *s ) ;

void openfiles() ;

void bannero(), banneri() ; 

int funcStats( struct func *p, float *low, float *hi, float *avg, int *length, float *median  ) ;

void noscbank( float C[], int N, int R, int Nw, int I, float P, float O[] )
;

void 
	unconvert( float C[], float S[], int N2, int I, int R ),
	unconvert1( float C[], float S[], int N2, int I, int R ),
	unconvert2( float C[], float S[], int N2, int I, int R ),
	unconvert3( float C[], float S[], int N2, int I, int R )
;

void overlapadd( float I[], int N, float W[], float O[], int Nw, int n ) ;

void leanconvert(float S[], float C[], int  N2, int  D, int R ) ;
void leanconvert2(float S[], float C[], int  N2, int  D, int R ) ;
void leanconvert3(float S[], float C[], int  N2, int  D, int R ) ;

void leanunconvert( float C[], float S[], int N2, int I, int R ) ;
void leanunconvert2( float C[], float S[], int N2, int I, int R ) ;
void leanunconvert3( float C[], float S[], int N2, int I, int R ) ;



// RESCALE VALUE
float		rescalev=1
		;

//**************

// PRINT FUNCTIONS
int	
	prbanner( char *s,  int w ), 
	prt( char *s ), 	
	pri( int i,  char *s ),  
	prf( float f,  char *s ), 
	prs( char *r,  char *s ), 
	prline( int i, char *s  )
; 

void printUnixCommandLine(
			int argc, 
			char **argv
) ; 

float findMode( 
	float	array[],
	int 	numberOfValues,
	float modesMergeWidth,
	int *modeFoundFlag
) ; 

void sortArray(
	float array[],
	int numberOfValues
) ; 

int findFuncStats( 
	struct func *p, 
	int 	numberOfValues, 
	float 	*low,  
	float 	*high,  
	float 	*range,  
	float 	*average,
	float 	*median, 
	float		*mode, 
	float 	*standarddeviation,
	float 	*sum,
	float 	*begin, 
	float 	*end,
	float 	*middle, 
	int 		printflag,
	int		freqUnitsFlag
) ; 


void findArrayStats(
	float v[],
	int *numberOfValues,
	float *low,  
	float *hi,  
	float *range,  
	float *average,
	float *median,  
	float *mode,
	float *standarddeviation,
	float *sum,
	float *begin, 
	float *end,
	float *middle, 
	int 	printflag
) ; 


int findFilterTimeAndConstrainByWindow(
	float filttinc, 
	struct func *filttorigin, 
	struct func *filtrate,
	int Onset_and_Release_Segment_Mode__off_0__on_1,
	int Mode__sampler_loop_0__autostop_1, 
	int *autostopflag,
	int wrap_0_fold_1_clip_2,
	float *filttnow,
	float *oldfilttnow, 
	struct func *filtwinlow, 
	struct func *filtwinhi,
    	float *dur,
	float analysis_dur,
	int vibratoPeriodsUnitsRateFlag,
	float vibratoPeriodDuration
) ; 

float Hz_to_OPPC( 
    float Hz
) ; 

float OPPC_to_Hz( 
    float OctavePointPitchClass
) ; 

int autoplay(
	int autoplayreps, 
	int formatSwitchflag
) ; 



float halfHannWindow( float v ) ; 
float halfCosWindow( float v ) ; 
float halfWelchWindow( float v ) ; 


float constrainToReleaseSegmentTime(
    float dur,
    float timeRate,
    float analysis_dur,
    float *filttnow,
    int Onset_and_Release_Segment_Mode__off_0__on_1,
    int *useEndSegmentStageFlag
); 

void normalizeLoopAmplitudes(
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
    int analysis_N,
    int ainchan,
    int analysis_chan,
    float analysis_dur, 
    float filttnow
) ; 

float makeLoopSmoothTime (
    float filttnow,
    int wrap_0_fold_1_clip_2,
    int Mode__sampler_loop_0__autostop_1, 
    float dur,  
    struct func *filtwinlow,    
    struct func *filtwinhi,
    struct func *peakLoopSmoothTime
) ; 


int fixTildeInFilename( char *tempstring ) ;

int FileTest_ASCIIorBinaryFloat( char *filename, int printFlag ) ; 

// SHELF EQ
int eq(float SP[], int Nplus2, float dBlow, float dBhi,
	     float freqlow, float freqhi, float fundamental,  
	     float pmult,  float freqadd,  int normflag ) ;

// SHELF EQ
int eq2(float SP[], int N, float dBlow, float dBhi,
	     float freqlow, float freqhi, float fundamental,  
	     float channel_freqdev[],  int normflag ) ;


int eq3(float SP[], int N, float dBlow, float dBhi,
	     float freqlow, float freqhi ) ;

int eq4( float SP[], int N, float dBlow, float dBhi, 
    float freqlow, float freqhi, float eqcf, float dBcf, 
	float eqwarp ) ; 		


// CURVE FUNCTION
float curve( float V1, float  V2, float  n, float   x) ; 
// FIND A FILE FUNCTION VALUE
float fval( struct func *p, float dur, float  t ) ;
// FILL AN ARRAY FROM A FILE
float fillfunc( struct func *p, float *F, int N   ) ;

float semitones_to_mult( float semidev ) ; 
float dB_to_amp( float dB ) ;
float amp_to_dB( float amp ) ;  
float smooth_setup( float t, float *c, float *minusc, float IR ) ; 

float smooth_one_value(

    float A, 
    float old_A, 
    float att,
    float matt, 
    float rel, 
    float mrel

) ; 

int findFuncMinMaxAvg( 
    struct func *p,
    float *minval,
    float *maxval,
    float *avgval	  
) ; 


void getGlobalFunctionValues(
    float values[],  
    int numTones,
    float dur,
    float delayTimes[],
    float timeRateScalers[], 
    struct func *function
) ; 

void setupToneBankOrPartialFunctionValues(
    int numTones,
    float dur,
    float delayTimes[], 
    float timeRateScalers[],
    struct func functions[]
) ; 

char routine[ STRING_SIZE ] ; 


int formatSwitchflag ; 


// STRING PARSE FOR CRACK
FILE *crackstring( char  s[], struct func *p ) ; 
FILE *crackstring_noprint( char  s[], struct func *p ) ; 
FILE *crackstring_bin_only( char  s[], struct func *p ) ; 

// SPECTRUM MAGNITUDE DISTRIBUTION WARPING
int spectmagwarp( float SP[],  int Nplus2,  float warpshape, int normflag ) ; 

// SPECTRUM MAGNITUDE DISTRIBUTION WARPING
int spectmagwarp2( float SP[], float SP_return[], int Nplus2,  float warpshape, int normflag ) ; 

// SPECTRUM MAGNITUDE DISTRIBUTION WARPING FOR INHARMONATOR
int spectmagwarp_inharm( float SP[], float F[],  int Nplus2,  float warpshape ) ; 

// SMOOTH THE SPECTRUM FOR USE AS A BASELINE
int smoothspec( float F[], int N2plus1, float octavesOrFreqBW,  int R ) ;

// SMOOTH THE CHANGES TO THE SPECTRUM
int smooth( float A[], float old_A[], int Nplus2, float att, float matt, float rel, float mrel ) ; 

// SMOOTH THE CHANGES TO THE SPECTRUM
int CartesianSmooth( float A[], float old_A[], int N, float att, float matt, float rel, float mrel ) ; 


float crackfloat( char  s[], char ch ) ; 

int smoothfreqs( 
    float A[], 
    float old_A[], 
    int N, 
    float coef,
    float minuscoef
) ; 


int smoothnew( 
    float channel[], 
    int N,  
    float detectenv[], 
    float newenv[],  
    int N2,  
    float detect, 
    float mdetect, 
    float attackc, 
    float minusattackc, 
    float releasec, 
    float minusreleasec ) ; 




// SMOOTH THE AMP CHANGE MULTIPLIERS
int smooth_amp_change( float A[], float old_A[], int N2, float att, float matt, float rel, float mrel ) ; 

// SMOOTH THE CHANGES TO THE SPECTRUM
int smooth_zero( float A[], float old_A[], int N, float att, 
	float matt, float rel, float mrel,  float F[],  float gain ) ; 

// PRINT THE SPECTRUM VALUES AND GRAPH TO TERMINAL
int tprintspec( float F[], int Nplus2, float fundamental, int freqcutoff) ;
int tprintspec_groupdelay( float F[], int N, float fundamental, int freqcutoff) ;

// COMPRESS THE SPECTRUM
int compress( float SP[],  int Nplus2,  float filtcompthreshamp, float  filtcompamp, float filtcompnormamp ) ; 

// NORMALIZE THE SPECTRUM PEAKAMP
int normalize( float SP[],  int Nplus2,  float peakamp ) ; 

// GAIN SCALE THE MAGNITUDES
int gainscale( float SP[],  int N,  float amp ) ; 

// NORMALIZE THE SPECTRUM PEAKAMP
float RIfindpeak( float buffer[],  int N ) ; 

// PASS BACK TIME NOW
float timenow(float dur ) ; 

float randf( float min,  float max ) ;

// READ IN FFT HEADER VALUES
int readffthead( int *N, int *D, int *R, int *chans, int *win_type,  float peakamps[],  struct func *p, int printFlag ) ; 

// FIND THE PEAK AMP FOR THE FILE'S CHANNELS
float find_peak_amp( float Hwin[], float Wanal[], float Wsyn[], int N, int Nw,  
    int I, int D, int N2,  int R,  int obank,   float buffer[],   
    float channel[],   float input[],  int beginchan, int endchan )  ; 

// MAKE AN AVERAGE OR PEAK FREQRESPONSE
int make_freqresponse( float F[], float Hwin[], float Wanal[], float Wsyn[],   
    int N, int Nw, int I, int D, int N2, int R, int obank,   
    float buffer[], float channel[], float input[],   
    int method, float A_begint,  float A_endt ) ; 

// COMPANSION
int compand( float SP[], int Nplus2, float compthreshamp, float  compamp, 
    float expthreshamp, float expamp  ) ; 

// NOISE RESPONSE THRESHOLD LIMITING
int threshold_limit( float SP[],  int Nplus2,  
	float noise_thresh_limit_dB
    )  ;

float Hz_to_MIDI(
	float Hz
); 
float MIDI_to_Hz(
	float MIDI
); 


float find_spectralflatness(

	float *flatnessCoef, 
	float *flatnessCoefInDecibels,
	float A[],
	float previous_A[], 
    	int N, 
	int methodFlag,
    	float lowf, 
    	float hif, 
    	float fundamental, 
    	float *old_value,
	float amplitudeThreshold

) ;


// CENTROID PLUGIN
float find_centroid(
	float *freq,
	float *amp,

    float A[], 
    int Nplus2, 
    float lowf, 
    float hif, 
    float fundamental, 
    float *old_value
 ) ; 

// FLUXOID PLUGIN
float find_fluxoid(

    float A[], 
    float old_A[], 
    int N, 
    float lowf, 
    float hif, 
    float fundamental, 
    float *old_value, 
    int flux_weight_flag

 ) ; 



// ANALYZE PARTIALS FUNCTION
float AnalyzePartials(

    float A[], 
    int N, 
    float fundamental,
    float analysisFrequency,
    float bandwidth,
    int numberOfPartials, 
    float FrequencyOfPartials[],
    float AmplitudeOfPartials[]

)    ;

void cutDC( 
    float channel[],
    int NC,
    float cutoffFreq
) ; 




int cut_data_lines( 
    char datafile[],  
    char new_datafile[],  
    int data_size

) ;

int readFilterFrame (
	struct func *data,
	float channel[],
	int frame, 
	int analysis_N, 
	int ainchan, 
	int analysis_chan
)
;



int makeInterpolatedFilterFrame (
	struct func *data,
	float F_lower[], 
	float F_higher[],
	float channel[],
	float iframes_per_sec, 
	int analysis_N, 
	float timepoint,
	int ainchan, 
	int analysis_chan
)
; 

int writeSpectrumPlotFile(
    char spectrumOutputFile[],
    float F[],
    int N,
    int decibelsFlag
) ; 

void NormalizeToPeaksOfSpectrumInBand( 
    int normalizeToPeaksFlag, 
    float F[],
    int N,
    int numFormants, 
    int formantIndices[],
    int formantLowStopBandIndices[], 
    int formantHighStopBandIndices[], 
    float expansionIndex
) ; 


int get_formants( 
	int *numFormants, 
	float formantCenterFreqs[],  
	float formantAmps[], 
	float formantBWs[],
	float formantQs[],
	int formantIndices[],
	int formantLowStopBandIndices[], 
	int formantHighStopBandIndices[], 
	float F[], 
	int N,              
	float lowFreqLimit, 
	float highFreqLimit, 
	float minimumFormantDB,
	float Formant_Selection_Threshold__0_to_1,
	char freqStasisPlotFile[],
	int CorrelateWithFreqStasisFlag,
	float nyquist
) ; 

void filesToRemove(
	char *file,
	int removeFlag
) ; 


