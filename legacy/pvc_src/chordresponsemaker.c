#include "globals.h"
#define NUM_PARAMETERS 6

void usage() ; 
void pd( int i ) ;

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k,  i1,  i2, ipartial,   np;
float nyquist;
double atof();
int R=44100, N=1024;


FILE *fopen(); 
SNDFILE *infile;
char ch, soundfile[ STRING_SIZE ] ;
char dBonlyspectfile[ STRING_SIZE ] ;

int  dBSpectFileflag=0 ; 

SF_INFO inputSFinfo ; 

float *F; 
 

float  temp,  temp3,  temp5,  temp6 ;  
float getthresh();
float avgbinamp=0.,  peakamp=0,  peakfreq, 
    dBedge=-96,  dbdown1,  dbdown2, dBrolloff ;
float fundamental ; 
int   invert_flag=0,  nZeroBins=0,  spectmethod=0 ;


// SHELF EQ
 
 

float partfreq,  fundfreq,  octroll,  partdB ; 

FILE *data ; 
char datafile[ STRING_SIZE ] = "EMPTY\0" ; 
char datafile2[ STRING_SIZE ] = "EMPTY\0" ; 
char new_datafile[ STRING_SIZE ] = "EMPTY\0" ; 
float *PP,  freqdiff,  midC; 
int vb=0 ; 
float bw ; 
 

int window_t=0 ; 




if( argc < 2 )usage() ; 

//	CASE -> USAGE
//	a -> ? f -> ? h -> ?

    while( (ch= crack( argc, argv, "a|f|R|N|F|D|s|v|i|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':	N = (int) crackfloat( arg_option, ch );
			break;


	    case 'D':   window_t = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 's':   spectmethod = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'i':   invert_flag = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'F':   strcpy(datafile, arg_option);
			break;
	    case 'v':	vb = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'a':   strcpy(dBonlyspectfile, arg_option); dBSpectFileflag = 1 ; 
			break;
	    case 'f':   strcpy( soundfile, arg_option); 		
				fixTildeInFilename( soundfile ) ; 
			break ; 
	    case 'h':	usage();

	}
    }

//*****************
prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "CHORDRESPONSEMAKER", 69 ) ; 
prline( 69,  "-" ) ; 

// GET RATE FROM TARGET SOUNDFILE.
    if (! (infile = sf_open (soundfile, SFM_READ, &inputSFinfo )))
    {   
	// Open failed so print an error message.
	banneri(); 
	fprintf(stderr,  
	"\n\n********\nCANNOT OPEN INPUT SOUNDFILE:\n\n------>  %s <------ TO ACQUIRE SR.\n", 			soundfile);
	// Print the error message from libsndfile.
	puts (sf_strerror (NULL)) ;
	exit(EXIT_FAILURE) ; 

    }; 

   R = inputSFinfo.samplerate ; 
   sf_close( infile ) ; 

    midC = (220.*pow(2., (3./12.))) ; 
    freqdiff = (float) R / (float) N ; 
    nyquist = R/2.0;
    fundamental =  ((float) R / (float) N) ; 


if( window_t == 0 )dBedge = -96. ; 
else if( window_t == 1 ) dBedge = 0. ; 
else {
    prt( "ILLEGAL WINDOW TYPE" ) ; 
    exit(EXIT_FAILURE) ; 
}


//fprintf( stderr,  "\nN: %d, R: %d,    BASEFREQ: %f",N, R,    freqdiff ) ; 

// GET NAME OF OUTPUT FILE

/* GET OUTPUT FILE NAME */
	strcpy( ofile, argv[arg_index] ) ; 

    /* OPEN FOR BUSINESS */

    /* OPEN FOR BUSINESS */
    if( (ofd = fopen( ofile, "w+" )) == NULL ){
        fprintf( stderr, "\n\n****** CANNOT OPEN FILE NAMED: %s\n\n", ofile ) ; exit(EXIT_FAILURE) ; 
    } ; 
    

//*********************************
prs( ofile, "FREQUENCY RESPONSE FUNCTION OUTPUT FILE" ) ; 
prline( 1,  "*" ) ; 
prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 

if( window_t == 0 )prt( "USING TRIANGULAR WINDOW" ) ; 
else prt( "USING RECTANGULAR WINDOW" ) ; 

prline( 1,  "*" ) ; 
    if( spectmethod == 0 )prt( "SPECTRUM ACCUMULATION METHOD: PEAK" );
    else prt( "SPECTRUM ACCUMULATION METHOD: SUM" ) ;
prline( 1,  "*" ) ; 
    if( invert_flag )prt( "BAND REJECT " );
    else prt( "BAND PASS" ) ;
prline( 1,  "*" ) ; 
//*********************************




//fprintf( stderr,  "\n datafile = %s, datafile2 = %s  ",  datafile,  datafile2 )  ; 
if( (strcasecmp( datafile, datafile2 ) == 0)){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A DATA FILE. BYE.\n\n" ) ;
    usage();
}




// MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
    cut_data_lines( datafile,  new_datafile,  NUM_PARAMETERS ) ; 

//**************************GET DATA	
// READ IN CHORD TONES, SHIFTPOINT, NUMBER_OF_PARTIALS, BW, AND DB
// OPEN FILE
	if( (data = fopen( new_datafile, "r")) == NULL ){
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  datafile ) ; 
	    exit(EXIT_FAILURE); 
	}
 
// COUNT VALUES IN FILE
	k = 0 ; 
	while( fscanf( data,  " %f ",  &temp ) != EOF ){
	 k++ ;
//		pd(k); 
	} 			

	rewind( data ) ;    
// ALLOCATE SPACE FOR CHORD TONES
    fvec( PP, k ) ;	/* PARTIALS */
// COMPUTE NUMBER OF TONES
    np = k / NUM_PARAMETERS ;

fprintf( stderr,  "\nCHORDRESPONSEMAKER:  \n- datafile values - " ) ;
fprintf( stderr,  "\n******************************************************************************" ) ; 
fprintf( stderr,  "\nFrequency or|Number of   |BW          |Decibels    |Partials    |dB Rolloff  |" ) ; 
fprintf( stderr,  "\nOct.PClass  |Partials    |Proportion  |            |Spacing Prop|per Octave  |" ) ;

fprintf( stderr,  "\n.............................................................................|" ) ; 

// READ IN VALUES
k = 0 ; 
for( i = 0; i < np ; i++ ){
    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < NUM_PARAMETERS ; j++ ){
	    fscanf( data,  " %f ",  &PP[ k ] ) ; 
	    fprintf( stderr,  "%-12.3f|", PP[ k ]  ) ;
	     k++ ; 			
    }
} 
    fprintf( stderr,  "\n******************************************************************************" ) ; 

// ALLOCATE FILTER SPACE
fvec( F,  N+2  ) ;	/* filter array */

// INITIALIZE ARRAY
    for(i = 1,  j = 0;  i < (N + 2) ; i += 2,  j++){
	 F[i - 1] = 0. ;  // SET AMP TO ZERO
	 F[i] = (float) (j)  * freqdiff  ; // SET FREQ
    }

    
    // FOR EACH TONE, REDUCE THE AMP BY ITS SPECIFIED AMOUNT

    // LOOP FOR  TONES

    for( j = 0; j < (np*NUM_PARAMETERS) ; j += NUM_PARAMETERS ){

	// FIND THE TONES FREQUENCY AND MAKE THE BW MULTIPLIER
	if( PP[j] <= 12. ){
 
	// TRANSLATE OCTAVE.PT.PITCHCLASS INTO FREQUENCY AND REPLACE IN ARRAY
	    fundfreq = temp = OPPC_to_Hz( PP[j] ) ; 	

		    // FREQUENCY METHOD
	} else {
 
	    // MOVE FREQUENCY INTO TEMP
	    fundfreq = temp = PP[j] ;
	}
	    // MAKE BW ADDER
	    bw = PP[j + 2] > 1. ? PP[j + 2] : temp * PP[j + 2] * .5 ;

	    // IF NUMBER OF PARTIALS IS 0, COMPUTE NUMBER UP TO NYQUIST
	    if( (int) PP[j + 1] <= 0 ){
		PP[j + 1] = (float) ((int)((nyquist - temp) / (temp * PP[j + 4]))) ; 
	    }

	    // CREATE PARTIAL DB ROLLOFF INCREMENT

		dBrolloff = PP[j + 5] ; 






	// LOOP FOR PARTIALS
	for( k = 1; k <= (int) PP[j + 1]; k++ ){

	// MAKE PARTIAL FREQUENCY
	    partfreq = temp3 = temp + (temp * PP[j + 4] * (float) (k - 1)) ; 
	// INDEX OF THE FREQ BIN BELOW THE PARTIAL
	i1 = 1 + (2 * ( rint((double)(temp3 - bw) / freqdiff)) ) ; 
	// INDEX OF THE FREQ BIN ABOVE THE PARTIAL
	i2 = 1 + (2 * ( rint((double)(temp3 + bw) / freqdiff)) )  ;  
	// INDEX OF THE FREQ BIN AT THE PARTIAL
	ipartial = 1 + (2 * ( rint((double)(temp3) / freqdiff)) )  ;  


	// PROPORTION OF PARTIAL TO FUNDAMENTAL IN OCTAVES
	octroll = log10( (double) ( partfreq / fundfreq ) ) / log10( 2. )  ; 


	// FIND THE DB LEVEL OF THE PARTIAL
	partdB = (PP[j + 3] + (octroll * dBrolloff)) ; 
	partdB = ( partdB < -96 ) ? -96 : partdB ; 

	// FIND DECIBEL DOWN INCREMENT ABOVE AND BELOW
	    // BELOW
	temp6 = (float) (ipartial - i1) / 2 ;  
	if((temp6 == 0.) || (window_t == 1) ) dbdown1 = 0. ; 
	else dbdown1 = (dBedge - partdB) / temp6 ; 
	    // ABOVE
	temp6 = (float) (i2 - ipartial) / 2 ;  
	if((temp6 == 0.)  || (window_t == 1)  ) dbdown2 = 0. ; 
	else dbdown2 =  (dBedge - partdB) / temp6  ; 

//fprintf( stderr,  "\npartdB: %f,  dBedge: %f,  dbdown1: %f,  dbdown2: %f",
//	    partdB,  dBedge,  dbdown1,  dbdown2 ) ;  


	// SETUP PARTIAL
	    // IF IN RANGE.....
	    if((ipartial > 0) && (ipartial < N) ){

		// AMP
		if(F[ ipartial - 1] == 0.) nZeroBins ++ ; 

		    // MAKE FINAL AMP
		    temp5 = pow( (double) 10.0, (double) partdB / 20. ) ;



		// PEAK OR SUM RESPONSE
		if(spectmethod == 0){
		    // PEAK
		    if(temp5 > F[ ipartial - 1] )  F[ ipartial - 1] = temp5 ; 
		}else{
		    // SUM
		    F[ ipartial - 1] = F[ ipartial - 1] + temp5 ; 
		}

	    }


	// LOOP FOR THE ipartial-i1 BINS
	for(i = i1; i < ipartial; i += 2){
	    // IN THE NEXT (ipartial-i1)*2 SPACES IN THE FILTER ARRAY 
	    // PLACE THE APPROPRIATE AMP VALUE FOR THIS TONE
	    
	    // IF IN RANGE.....
	    if((i > 0) && (i < N) ){
		// AMP
		if(F[ i - 1] == 0.) nZeroBins ++ ; 
		temp6 = (float) ((ipartial - i) / 2) * dbdown1 ; 
		temp5 =  (PP[j + 3] + (octroll * dBrolloff) + temp6) ;
		temp5 = (temp5 < -96.) ? -96 : temp5 ;  
		temp5 = pow( (double) 10.0, (double) temp5 / 20. ) ;


		// PEAK OR SUM RESPONSE
		if(spectmethod == 0){
		    if(temp5 > F[ i - 1] )  F[ i - 1] = temp5 ; 
		}else{
		    F[ i - 1] = F[ i - 1] + temp5 ; 
		}


	    }
	}


	// LOOP FOR THE ipartial-i2 BINS
	for(i = i2; i > ipartial; i -= 2){
	    // IN THE NEXT (ipartial-i2)*2 SPACES IN THE FILTER ARRAY 
	    // PLACE THE APPROPRIATE AMP VALUE FOR THIS TONE
	    
	    // IF IN RANGE.....
	    if((i > 0) && (i < N) ){
		// AMP
		if(F[ i - 1] == 0.) nZeroBins ++ ; 
		temp6 = (float) ((i - ipartial) / 2) * dbdown2 ; 
		temp5 =  (PP[j + 3] + (octroll * dBrolloff) + temp6) ;
		temp5 = (temp5 < -96.) ? -96 : temp5 ;  
		temp5 = pow( (double) 10.0, (double) temp5 / 20. ) ;


		// PEAK OR SUM RESPONSE
		if(spectmethod == 0){
		    if(temp5 > F[ i - 1] ) F[ i - 1] = temp5 ; 
		}else{
		    F[ i - 1] = F[ i - 1] + temp5 ; 
		}

	    }
	}

      }

    }



// FIND THE PEAK AND AVERAGE
    peakamp = 0 ; peakfreq = 0 ; avgbinamp = 0 ; 
    for( i = 1; i < (N + 2) ; i += 2 ){
	avgbinamp += F[ i - 1] ; 
	if( F[ i - 1] > peakamp ) {
	    peakamp = F[ i - 1] ;
	    peakfreq = F[ i ] ;  
	}
    }

// NORMALIZE
//*************NORMALIZE THE SYNTHESIZED SPECTRUM
    normalize(  F,  (N + 2),   peakamp  ) ;

// FIND THE PEAK AND AVERAGE
    peakamp = 0 ; peakfreq = 0 ; avgbinamp = 0 ; 
    for( i = 1; i < (N + 2); i += 2 ){
	avgbinamp += F[ i - 1] ; 
	if( F[ i - 1] > peakamp ) {
	    peakamp = F[ i - 1] ;
	    peakfreq = F[ i ] ;  
	}
    }


//    avgbinamp = avgbinamp / (float) (N/2) ; 
//    averagedB = (float) (20. * log10( (double) avgbinamp )) ; 
//    percentofbins = (float) ((N/2) - nZeroBins) / (float) (N / 2) ; 



// PRINT STATS
//    fprintf( stderr,  "\n\nAVERAGE BIN AMP: %f dB", averagedB ) ;  
//    fprintf( stderr,  "\n\nNUMBER OF NON-ZERO BINS: %d", (N/2) - nZeroBins ) ;  
//    fprintf( stderr,  "\n\nPROPORTION OF NON-ZERO BINS: %f \%", percentofbins ) ;  

// INVERT FILTER IF BAND REJECT
    if( invert_flag != 0 ){
	for(i = 1; i < (N + 2); i += 2){
	    F[i - 1] = 1. - F[i - 1] ; 
	}
    }

prf( fundamental,  "FUNDAMENTAL" ) ; 

    // WRITE RESPONSE TO BINARY DECIBELS ONLY SPECTRUM FILE
    if( dBSpectFileflag ) writeSpectrumPlotFile( dBonlyspectfile, F, (N + 2), 1 ) ; 


// LET'S SEE THE FILTER
    if( vb )tprintspec( F, (N + 2), fundamental,  vb ) ;

 
// WRITE RESPONSE TO stdout
    for( i = 0; i < (N + 2); i++ )fwrite(  &F[ i ], sizeof(float), 1, ofd ) ;

    if(ofd)fclose(ofd); 

fprintf( stderr,  "\n\nNUMBER OF SYNTHESIS OUTPUT AMP/FREQ PAIRS:    %d\n",  i/2 ) ; 
    
    
    fprintf(stderr,"\nCHORDRESPONSEMAKER : SYNTHESIS OF RESPONSE COMPLETED\n");
  
    fclose( data ) ; 
 
    exit(EXIT_SUCCESS) ;
}
void usage()
{

    fprintf(stderr, "%s",
	"chordresponsemaker:  chordal frequency response synthesizer\n"
	"			(multiple harmonic tone format)  \n"
	"			(values in brackets represent defaults)\n"
	"chordresponsemaker   [flags]  [output freqresponse file]\n"
	"	N:	fft length [1024]\n"
	"	R:	sampling rate [44100]\n"
	"	F:	(ascii) DATA FILE of unordered sextuples:\n"
	"		     Each consisting of: \n"
	"		     (1) pitch (octave.pitchclass) or frequency\n"
	"			   (Values of 12 or less are interpreted as \n"
	"				octave.pitchclass; otherwise, units are in Hertz.)\n"
	"		     (2) number of partials (0 = all partials below Nyquist)\n"
	"		     (3) bandwidth as proportion of fundamental frequency\n"
	"			(Values of 1. or less are interpreted as proportions \n"
	"				of the fundamental frequency; otherwise, units are in Hertz.\n"
	"		     (4) decibels\n"
	"		     (5) partial spacing as proportion of fundamental frequency\n"
	"		     (6) decibel rolloff (+ or -) per octave of partials \n"
	"	s:	Frequency Response Accumulation Method: [0]\n"
	"		    (treatment of overlapping bin values)\n"
	"		    0 = peak response (uses highest of all)\n"
	"		    1 = sum response (uses sum of all)\n"
	"	D:	Band Window Type: 0 = triangle,  1 = rectangle [0]\n"
	"		    (As reflected in decibels)\n"
	"	i:	filter type: bandpass (0),  band reject (1) [0]\n"
	"	v:	frequency response stderr printout: high cutoff frequency in Hz [0]\n"
	"		    (0 = off)\n"


	"	a:	decibels spectrum plot file\n"
	"	h:	usage\n"




	);

    exit(EXIT_SUCCESS);

}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

