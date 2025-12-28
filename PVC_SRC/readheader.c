#include "globals.h"

void usage() ;

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k,  i1,  i2;
float i1p,  i2p ; 
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int analysis_N,  analysis_D, analysis_R, analysis_chan,  niframes ;  
float analysis_dur,  iframes_per_sec ; 
int   eof = 0 ;
FILE *fopen(), *fp, *home ;
char ch, homeDirectory[ STRING_SIZE ];
float  temp,  temp2,  temp3 ;  
float normamp[MAXIMUM_CHANNELS],  normamppk ;
float   IR,  dur=0.;
float fundamental ;
float analysis_fundamental ;  

char tempstring[ STRING_SIZE ] ; 


// DATA
struct  func  filter ; 

//*****************INITIALIZE

// FILTER
filter.L = 1. ; filter.n = 0. ; filter.A[ 0 ] = 0. ; 



if( argc < 2 )usage() ; 

arg_index++ ; 

while( arg_index < argc ){

	strcpy( tempstring, argv[arg_index] ) ; 
	filter.fp = crackstring_bin_only( tempstring, &filter );



	prline( 69,  "/" ) ; 
	prline( 69,  "-" ) ; 
	prbanner( "READ HEADER", 69 ) ; 
	prline( 69,  "-" ) ; 

    	// READ IN FFT HEADER VALUES
    	readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &k,  normamp, &filter, 1 ) ; 




	// ************ TEST SIZE TO SEE IF DATA FILE IS POSSIBLY WRONG
    	temp = ( ((filter.n  - (float) FFT_HEADER_SIZE) / (float) (analysis_N + 2) ) != 0 ) ; 
    	temp2 = temp - (float) ( (int) temp ) ; 
    	if( (analysis_N <= 0) || (analysis_D <= 0) || (analysis_R <= 0) || 
	    	(analysis_chan <= 0) || ( temp2 != 0. ) ){
		prt( "YOUR ANALYSIS FILE HAS QUESTIONABLE DATA. BYE." ) ; 
	    	pri( analysis_N,  "INPUT ANALYSIS: FFT SIZE" ) ; 
	    	pri( analysis_R,  "INPUT ANALYSIS: SAMPLE RATE" ) ; 
	    	pri( analysis_D,  "INPUT ANALYSIS: DECIMATION" ) ; 
	    	pri( analysis_chan,  "INPUT ANALYSIS: NUMBER OF CHANNELS" ) ; 
	    	pri( filter.n,  "INPUT ANALYSIS: FILE SIZE (in 32-bit floats)" ) ; 
	    	pri( (filter.n * 4),  "INPUT ANALYSIS: FILE SIZE (in bytes)" ) ; 
	    	prf( ((float) (filter.n * 4 ) /(1024.*1024.)),  "INPUT ANALYSIS: FILE SIZE (in Mbytes)" ) ; 
	    	prline( 69,  "-" ) ;
	    	prt( "\n" ) ;  
	    
		exit(EXIT_FAILURE) ; 
    	}


	//*******
	// FIND DURATION OF INPUT FILE

    	niframes = (((filter.n - (float) FFT_HEADER_SIZE) / (float) (analysis_N + 2))) / analysis_chan ; 
    	analysis_dur = (float) (niframes) / ((float) analysis_R / (float) analysis_D ) ; 
    	iframes_per_sec = (float) analysis_R /  (float) analysis_D ; 



    	pri( analysis_N,  "INPUT ANALYSIS: FFT SIZE" ) ; 
    	pri( analysis_R,  "INPUT ANALYSIS: SAMPLE RATE" ) ; 
    	pri( analysis_D,  "INPUT ANALYSIS: DECIMATION" ) ; 
    	pri( (analysis_R / analysis_D), "INPUT ANALYSIS: FRAMES PER SECOND" ) ; 
    	pri( niframes,  "INPUT ANALYSIS: TOTAL NUMBER OF FRAMES" ) ; 
    	prf( analysis_dur,  "INPUT ANALYSIS: DURATION" ) ; 
    	pri( analysis_chan,  "INPUT ANALYSIS: NUMBER OF CHANNELS" ) ; 
    	pri( filter.n,  "INPUT ANALYSIS: FILE SIZE (in 32-bit floats)" ) ; 
    	pri( (filter.n * 4),  "INPUT ANALYSIS: FILE SIZE (in bytes)" ) ; 
    	prf( ((float) (filter.n * 4 ) /(1024.*1024.)),  "INPUT ANALYSIS: FILE SIZE (in Mbytes)" ) ; 

	/*
    	prt( "PEAK CHANNEL AMPLITUDES:" ) ; 
    	for( i = 0; i < analysis_chan; i++ )
		fprintf( stderr, "\n\t%d. %f (%d dB)", i, normamp[i], (int) amp_to_dB( normamp[i] ) ) ; 		
	*/

    	prline( 69,  "-" ) ; 
    	prt( "\n" ) ;  

	fclose(filter.fp ) ;
	arg_index++ ; 
} ; 


exit(EXIT_SUCCESS) ;

}
void usage()
{
    fprintf(stderr, "%s",
	"readheader:  read the headers of pvanalysis files\n"
	"readheader   <pvanalysis file(s)>\n"
	);
    exit(EXIT_SUCCESS);
}


void pd( i ) int i ; { fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

