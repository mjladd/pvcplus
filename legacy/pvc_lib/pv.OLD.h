/* #include <stdlib.h> */
#include <stdio.h>
#include <math.h>
#include <unistd.h>


/* CRACK STUFF */
extern int arg_index;
extern char *arg_option;
extern char *pvcon;
extern char *index();



#define FORWARD 1
#define INVERSE 0

#define flog10 log10
#define log10f log10



//  PARAMETER FILE CONTROL FUNCTION STRUCTURE

struct func {
    float A[ 8 ];
    float n ; 
    float L ;
    FILE *fp;
    char fname[ 128 ] ; 
}  ;
 




typedef struct { float re; float im; } complex;

typedef struct {
  float    min;
  float    max;
} Bound;

#define CABS(x) hypot( (x).re, (x).im )

complex cadd(), csub(), cmult(), smult(), cdiv(), conjg(), csqrt();

extern complex zero;
extern complex one;
/* extern char *malloc(), *calloc(); */
extern float synt;

// GLOBALS
extern char	    *ifile ; 
extern char	    *ofile ; 
extern FILE	    *ifd;
extern int	    ofd;


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
		channow, 
		frame_count, 
		outputoff, 
		oscilbankon, 
		ttlsamps, 
		samps, 
		nsover[ 4 ] 
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

extern float		idur;

extern float		ipeakamp[ 4 ];


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
		odata_offset, 
		oformat, 
		outputformat		
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

// SHELF EQ
extern int eq(float SP[], int N, float dBlow, float dBhi,
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



// SPECTRUM MAGNITUDE DISTRIBUTION WARPING
extern int spectmagwarp( float SP[],  int N,  float warpshape, int normflag ) ; 

// SPECTRUM MAGNITUDE DISTRIBUTION WARPING
extern int spectmagwarp2( float SP[], float SP_return[], int N,  float warpshape, int normflag ) ; 

// SPECTRUM MAGNITUDE DISTRIBUTION WARPING FOR INHARMONATOR
extern int spectmagwarp_inharm( float SP[], float F[],  int N,  float warpshape ) ; 

// SMOOTH THE SPECTRUM FOR USE AS A BASELINE
extern int smoothspec( float F[], int N2, float octaves,  int R ) ;
 
// PRINT THE SPECTRUM VALUES AND GRAPH TO TERMINAL
extern int tprintspec( float F[], int N, float fundamental, int freqcutoff) ;
extern int tprintspec_groupdelay( float F[], int N, float fundamental, int freqcutoff) ;


// COMPRESS THE SPECTRUM
extern int compress( float SP[],  int N,  float filtcompthreshamp, float  filtcompamp, float filtcompnormamp ) ; 

// NORMALIZE THE SPECTRUM PEAKAMP
extern int normalize( float SP[],  int N,  float peakamp ) ; 

// GAIN SCALE THE MAGNITUDES
extern int gainscale( float SP[],  int N,  float amp ) ; 

// NORMALIZE THE SPECTRUM PEAKAMP
extern float RIfindpeak( float buffer[],  int N ) ; 

// PASS BACK TIME NOW
extern float timenow(float dur) ; 

extern float randf( float min,  float max ) ;

// READ IN FFT HEADER VALUES
extern int readffthead( int *N, int *D, int *R, int *chans, int *win_type,  float peakamps[],  struct func *p ) ; 

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


// SMOOTH THE CHANGES TO THE SPECTRUM
extern int smooth( float A[], float old_A[], int N, float att, float matt, float rel, float mrel ) ; 


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
extern int compand( float SP[], int N, float compthreshamp, float  compamp, 
    float expthreshamp, float expamp  ) ; 


// NOISE RESPONSE THRESHOLD LIMITING
extern int threshold_limit( float SP[],  int N,  
	float noise_thresh_limit_dB
    ) ; 


// CENTROID PLUGIN
extern float find_centroid(

    float A[], 
    int N, 
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




extern int cut_data_lines( 
    char datafile[],  
    char new_datafile[],  
    int data_size

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
