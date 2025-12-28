#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sndfile.h>
#include <string.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stdbool.h"

#define MAXIMUM_CHANNELS 500
#define FFT_HEADER_SIZE 32
#define BLOCKSIZE 1024


#define NULL_CHAR '\0'
#define CRACK_DONE_FLAG NULL_CHAR

#define STRING_SIZE 1000

/*
#define float double
#define sf_write_float sf_write_double
#define sf_read_float sf_read_double
*/

/* CRACK STUFF */
extern int arg_index;
extern char *arg_option;
extern char *pvcon;
extern char *index();

extern
	char crack(int argc, char **argv, char *flags, int ign)
;


extern
int getInputFileDataToSetOutputChannels(int argc, char **argv) ;

extern SNDFILE *infile, *outfile ;

extern char inputTempChanFileNames[ MAXIMUM_CHANNELS ][ STRING_SIZE ] ; 
extern FILE *inputTempChanFiles[ MAXIMUM_CHANNELS ] ; 

extern char outputTempChanFileNames[ MAXIMUM_CHANNELS ][ STRING_SIZE ] ; 
extern FILE *outputTempChanFiles[ MAXIMUM_CHANNELS ] ; 


#define FORWARD 1
#define INVERSE 0

#define flog10 log10
#define log10f log10

#define FFT_HEADER_SIZE 32



//  PARAMETER FILE CONTROL FUNCTION STRUCTURE

struct func {
    float A[ 8 ];
    float n ; 
    float L ;
    FILE *fp;
    char fname[  STRING_SIZE  ] ; 
}  ;
 




typedef struct { float re; float im; } complex;

typedef struct {
  float    min;
  float    max;
} Bound;

#define CABS(x) hypot( (x).re, (x).im )

// complex cadd(), csub(), cmult(), smult(), cdiv(), conjg(), csqrt();
complex cadd(), csub(), cmult(), smult(), cdiv(), conjg();

extern complex zero;
extern complex one;
/* extern char *malloc(), *calloc(); */
extern float synt;

// GLOBALS
extern char	    ifile[  STRING_SIZE  ] ; // INPUT FILE NAME
extern char	    ofile[  STRING_SIZE  ] ; // OUTPUT FILE NAME
extern FILE	    *ifd;
extern FILE	    *ofd;
extern char	    afile[  STRING_SIZE  ] ; // ANALYSIS FILE NAME READ WITH readffthead

extern bool	    expandSingleChannelInputFileToMultipleDuplicateChannels ; 

extern char routine[  STRING_SIZE  ] ; 

extern float crackfloat( char  s[], char ch ) ; 

extern void 
		shiftout( float A[], int N, int I, int n, int flushflag ),
		makewindows( float H[], float A[], float S[], int Nw, int N, int I, int osc ),
		fold( float I[], float W[], int Nw, float O[], int N, int n ),
		rfft( float x[], int N, int forward ),

		convert( float S[], float C[], int N2, int D, int R ),
		convert1( float S[], float C[], int N2, int D, int R ),
		convert2( float S[], float C[], int N2, int D, int R ),
		convert3( float S[], float C[], int N2, int D, int R )
;		
extern int
		shiftin( float A[],int N,int D ),
		bufferout( float *outbuff,  int I, int flushflag ),
		bufferin( float *V ) 
;


extern int 
		writeSpectrumPlotFile(
    			char spectrumOutputFile[],
    			float F[],
    			int N,
    			int decibelsFlag
)
;


extern int	
		window_type,
		IO_reset, 
		sample, 
		begin_sample, 
		end_sample, 
		inbuffn, 
		doneflag, 
		quiet, 
		flinflag, 
		channelflag, 
		beginchan,  
		endchan,   
		channow, 
		frame_count, 
		outputoff, 
		oscilbankon, 
		ttlsamps, 
		samps, 
		nsover[ MAXIMUM_CHANNELS ],
                autoplayreps,
		debugFlag 
;

extern float	
		begint, 
		endt, 
		ampstatinc, 
		t, 
		tfactor, 
		frames_per_sec, 
		outdur  
; 

extern int formatSwitchflag ; 
extern float	ringTime ; 
extern int	ringTimeSamples ; 

extern float	idur;

extern float	ipeakamp[ MAXIMUM_CHANNELS ];


extern int		
		isr,
		ichan,
		idata_offset, 
		iformat, 
		outchan
;  

// OUTPUT FILE HEADER INFO
extern float		
		odur
		;

extern int		
		osr,
		ochan,
		ochanModified,
		odata_offset, 
		oformat		
;  

// RESCALE VALUE
extern float	 rescalev
		;



// PRINT FUNCTIONS
extern int
	prbanner( char *s,  int w ), 
	prt( char *s ), 	
	pri( int i,  char *s ),  
	prf( float f,  char *s ), 
	prs( char *r,  char *s ), 
	prline( int i, char *s  )
; 

extern void printUnixCommandLine(
			int argc, 
			char **argv
) ; 

extern int findFuncStats( 
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


extern float findMode( 
	float	array[],
	int 	numberOfValues,
	float modesMergeWidth,
	int *modeFoundFlag
) ; 

extern void sortArray(
	float array[],
	int numberOfValues
) ; 


extern void findArrayStats(
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

// SHELF EQ
extern int eq(float SP[], int Nplus2, float dBlow, float dBhi,
	     float freqlow, float freqhi, float fundamental,  
	     float pmult,  float freqadd,  int normflag ) ;

extern int eq2(float SP[], int N, float dBlow, float dBhi,
	     float freqlow, float freqhi, float fundamental,  
	     float channel_freqdev[],  int normflag ) ;

extern int eq3(float SP[], int N, float dBlow, float dBhi,
	     float freqlow, float freqhi ) ;

extern int eq4( float SP[], int N, float dBlow, float dBhi, 
    float freqlow, float freqhi, float eqcf, float dBcf, 
	float eqwarp ) ; 		


extern float Hz_to_OPPC( 
    float Hz
) ; 


extern float OPPC_to_Hz( 
    float OctavePointPitchClass
) ; 

extern int findFuncMinMaxAvg( 
    struct func *p,
    float *minval,
    float *maxval,
    float *avgval	  
) ; 


extern float halfHannWindow( float v ) ; 
extern float halfCosWindow( float v ) ; 
extern float halfWelchWindow( float v ) ; 


// CURVE FUNCTION
extern float curve( float V1, float  V2, float  n, float   x) ; 
// FIND A FILE FUNCTION VALUE
extern float fval( struct func *p, float dur, float  t ) ;
// FILL AN ARRAY FROM A FILE
extern float fillfunc( struct func *p, float *F, int N   ) ;

// STRING PARSE FOR CRACK
extern FILE *crackstring( char  s[], struct func *p ) ; 
extern FILE *crackstring_noprint( char  s[], struct func *p ) ; 
extern FILE *crackstring_bin_only( char  s[], struct func *p ) ; 


extern int autoplay(
	int autoplayreps, 
	int formatSwitchflag
) ; 


// SPECTRUM MAGNITUDE DISTRIBUTION WARPING
extern int spectmagwarp( float SP[],  int Nplus2,  float warpshape, int normflag ) ; 

// SPECTRUM MAGNITUDE DISTRIBUTION WARPING
extern int spectmagwarp2( float SP[], float SP_return[], int Nplus2,  float warpshape, int normflag ) ; 

// SPECTRUM MAGNITUDE DISTRIBUTION WARPING FOR INHARMONATOR
extern int spectmagwarp_inharm( float SP[], float F[],  int Nplus2,  float warpshape ) ; 

// SMOOTH THE SPECTRUM FOR USE AS A BASELINE
extern int smoothspec( float F[], int N2plus1, float octavesOrFreqBW,  int R ) ;
 
// PRINT THE SPECTRUM VALUES AND GRAPH TO TERMINAL
extern int tprintspec( float F[], int Nplus2, float fundamental, int freqcutoff) ;
extern int tprintspec_groupdelay( float F[], int N, float fundamental, int freqcutoff) ;


// COMPRESS THE SPECTRUM
extern int compress( float SP[],  int Nplus2,  float filtcompthreshamp, float  filtcompamp, float filtcompnormamp ) ; 

// NORMALIZE THE SPECTRUM PEAKAMP
extern int normalize( float SP[],  int Nplus2,  float peakamp ) ; 

// GAIN SCALE THE MAGNITUDES
extern int gainscale( float SP[],  int N,  float amp ) ; 

// NORMALIZE THE SPECTRUM PEAKAMP
extern float RIfindpeak( float buffer[],  int N ) ; 

// PASS BACK TIME NOW
extern float timenow(float dur) ; 

extern float randf( float min,  float max ) ;

// READ IN FFT HEADER VALUES
extern int readffthead( int *N, int *D, int *R, int *chans, int *win_type,  float peakamps[],  struct func *p, int printFlag ) ; 

extern float PI;
extern float TWOPI;

extern float semitones_to_mult( float semidev ) ; 
extern float dB_to_amp( float dB ) ; 
extern float amp_to_dB( float amp ) ; 
extern float smooth_setup( float t, float *c, float *minusc, float IR ) ; 

extern float smooth_one_value(

    float A, 
    float old_A, 
    float att,
    float matt, 
    float rel, 
    float mrel

) ; 

extern void bannero(), banneri() ; 

extern
	void setupfiles(int argc, char **argv)
;

extern
int invertresponse( 
    float SP[],  
    int N, 
    int normflag
    )
;

extern
	int prp( struct func *p,  char *s ) 
;

extern
void openfiles() ;

extern
void phaselock( float channel[], int N2 ) ;

extern
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

extern 
int outfile_setup(int argc, char **argv ) ;


extern
void noscbank( float C[], int N, int R, int Nw, int I, float P, float O[] ) ;

extern
void noscbank2( float C[], int N, int R, int Nw, int I, float P, float O[], float CC[],  int NC ) ;

extern
void noscbank3( float C1[], int N1, int R, int Nw, int I, float P, float O[], float C2[],  int N2, float C3[], int N3 ) ;


extern
int funcStats( struct func *p, float *low, float *hi, float *avg, int *length, float *median  ) ;

extern void leanconvert(float S[], float C[], int  N2, int  D, int R ) ;
extern void leanconvert2(float S[], float C[], int  N2, int  D, int R ) ;
extern void leanconvert3(float S[], float C[], int  N2, int  D, int R ) ;

extern void leanunconvert( float C[], float S[], int N2, int I, int R ) ;
extern void leanunconvert2( float C[], float S[], int N2, int I, int R ) ;
extern void leanunconvert3( float C[], float S[], int N2, int I, int R ) ;


extern void 
	unconvert( float C[], float S[], int N2, int I, int R ),
	unconvert1( float C[], float S[], int N2, int I, int R ),
	unconvert2( float C[], float S[], int N2, int I, int R ),
	unconvert3( float C[], float S[], int N2, int I, int R )
;

extern
void overlapadd( float I[], int N, float W[], float O[], int Nw, int n ) ;

// SMOOTH THE CHANGES TO THE SPECTRUM
extern int smooth( float A[], float old_A[], int Nplus2, float att, float matt, float rel, float mrel ) ; 

// SMOOTH THE CHANGES TO THE SPECTRUM
extern int CartesianSmooth( float A[], float old_A[], int N, float att, float matt, float rel, float mrel ) ; 


extern int smoothfreqs( 
    float A[], 
    float old_A[], 
    int N, 
    float coef,
    float minuscoef
) ; 


extern int smoothnew( 
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


extern int findFilterTimeAndConstrainByWindow(	
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

extern void normalizeLoopAmplitudes(
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

extern float makeLoopSmoothTime (
    float filttnow,
    int wrap_0_fold_1_clip_2,
    int Mode__sampler_loop_0__autostop_1, 
    float dur,  
    struct func *filtwinlow,    
    struct func *filtwinhi,
    struct func *peakLoopSmoothTime
) ; 



extern int fixTildeInFilename( char *tempstring ) ;

extern int FileTest_ASCIIorBinaryFloat( char *filename, int printFlag ) ; 

// SMOOTH THE AMP CHANGE MULTIPLIERS
extern int smooth_amp_change( float A[], float old_A[], int N2, float att, float matt, float rel, float mrel ) ; 


// SMOOTH THE CHANGES TO THE SPECTRUM
extern int smooth_zero( float A[], float old_A[], int N, float att, 
	float matt, float rel, float mrel,  float F[],  float gain ) ; 



// FIND THE PEAK AMP FOR THE FILE'S CHANNELS
extern float find_peak_amp( float Hwin[], float Wanal[], float Wsyn[], int N, int Nw,  
    int I, int D, int N2,  int R,  int obank,   float buffer[],   
    float channel[],   float input[],  int beginchan, int endchan )  ; 

// MAKE AN AVERAGE OR PEAK FREQRESPONSE
extern int make_freqresponse( float F[], float Hwin[], float Wanal[], float Wsyn[],   
    int N, int Nw, int I, int D, int N2, int R, int obank,   
    float buffer[], float channel[], float input[],   
    int method, float A_begint,  float A_endt ) ; 

// COMPANSION
extern int compand( float SP[], int Nplus2, float compthreshamp, float  compamp, 
    float expthreshamp, float expamp  ) ; 


// NOISE RESPONSE THRESHOLD LIMITING
extern int threshold_limit( float SP[],  int Nplus2,  
	float noise_thresh_limit_dB
    ) ; 

extern float Hz_to_MIDI(
	float Hz
); 
extern float MIDI_to_Hz(
	float MIDI
); 



// CENTROID PLUGIN
extern float find_centroid(
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
extern float find_fluxoid(

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
extern float AnalyzePartials(

    float A[], 
    int N, 
    float fundamental,
    float analysisFrequency,
    float bandwidth,
    int numberOfPartials, 
    float FrequencyOfPartials[],
    float AmplitudeOfPartials[]


)    ;

extern void cutDC( 
    float channel[],
    int NC,
    float cutoffFreq
) ; 


extern void NormalizeToPeaksOfSpectrumInBand( 
    int normalizeToPeaksFlag, 
    float F[],
    int N,
    int numFormants, 
    int formantIndices[],
    int formantLowStopBandIndices[], 
    int formantHighStopBandIndices[], 
    float expansionIndex
) ; 


extern int writeDecibelsSpectrumPlotFile(
    char spectrumOutputFile[],
    float F[],
    int N,
    int decibelsFlag
) ; 

extern void filesToRemove(
	char *file,
	int removeFlag
) ; 




extern int get_formants( 
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


extern int cut_data_lines( 
    char datafile[],  
    char new_datafile[],  
    int data_size

) ;

extern int readFilterFrame (
	struct func *data,
	float channel[],
	int frame, 
	int analysis_N, 
	int ainchan, 
	int analysis_chan
)
;

extern int makeInterpolatedFilterFrame (
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

extern void getGlobalFunctionValues(
    float values[],  
    int numTones,
    float dur,
    float delayTimes[],
    float timeRateScalers[], 
    struct func *function
) ; 


extern void setupToneBankOrPartialFunctionValues(
    int numTones,
    float dur,
    float delayTimes[], 
    float timeRateScalers[],
    struct func functions[]
) ; 



/*
 * memory allocation macro
 */
#define fvec( name, size )\
if ( ( name = (float *) calloc( size, sizeof(float) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}
#define ivec( name, size )\
if ( ( name = (int *) calloc( size, sizeof(int) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}
#define sivec( name, size )\
if ( ( name = (short int *) calloc( size, sizeof(short int) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}

#define SWAPBYTES (0) /* only on intel - otherwise set to 0 */
#define SIZ0_HEADER 1024 
 /* just use first 28 bytes for NeXT header. A BICSF/IRCAM header
 is 1024 bytes long. Set accordingly. */
