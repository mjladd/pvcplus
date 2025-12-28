#include "globals.h"

#define NUM_PARAMETERS 7

void usage(); 
void pd( int i ) ;

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k, l,  i1,  i2, ipartial,   n, exflag, 
     NC, NC2,   np,  nf=2,  first=1  ;
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;

int analysis_N,  analysis_D, analysis_R, analysis_chan,  niframes ;  
float normamp[MAXIMUM_CHANNELS] ; 

int   eof = 0, obank = 0,  sflag = 0,  channelout=0 ;
float P = 1.0;
FILE *fopen(), *fp;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float *previous_channel,  *F,  *FT,  *SUM,  *harmony ; 
 float threshfac = .001;
float	gain=1. ;
float  temp,  temp2,  temp3,  temp4,  temp5,  temp6 ;  
float getthresh();
float peakbinamp = 0.,  avgbinamp=0., avgdelay,  averagedB,  peakamp=0,  peakfreq=-9999, minamp, 
    maxdelay=-99999,  mindelay=999999., 
    dBedge=0,  dbdown1,  dbdown2,  percentofbins, dBrolloff ;
float fundamental ; 
int   invert_flag=0,  nbins=0,  spectmethod=0 ;
float   IR,  dur=0.;

// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 
int eqnormbypassflag=0 ; 

FILE *data ; 
char datafile[ STRING_SIZE ] = "EMPTY\0", new_datafile[ STRING_SIZE ]  ; 
char datafile2[ STRING_SIZE ] = "EMPTY\0" ; 
float *PP, *domainamp, *partialamp,    fund=100.,  freqdiff,  target_gain,  SOURCE_gain,  midC,  part ; 
int method=1,  nd=0,  vb=0 ; 
float bw1,  bw2,  bw,  default_delay_time=0.,  default_dB=0.,  default_amp ; 
char tempstring[ STRING_SIZE ] ; 

void value_from_methods(

    int spectmethod, 
    float F[],  
    float PP[],  
    float ampval,  
    int j,  
    int ipartial 
    
    ) ;

// DATA
struct  func  filter ; 

//*****************INITIALIZE

// FILTER
filter.L = 1. ; filter.n = 0. ; filter.A[ 0 ] = 0. ; 



if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, "f|F|D|i|I|s|v|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'D':   dBedge = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 's':   spectmethod = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'i':   default_dB = crackfloat( arg_option, ch ) ;
			break;
	    case 'I':   default_delay_time = crackfloat( arg_option, ch ) ;
			break;

	    case 'F':   strcpy(datafile, arg_option);
			break;

	    case 'f':   strcpy(tempstring, arg_option); fixTildeInFilename( tempstring ) ; 
			filter.fp = crackstring_bin_only( tempstring, 
			    &filter );
			break;


	    case 'v':	vb = (int) crackfloat( arg_option, ch ) ;
			break;

	}
    }

//*****************
prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "GROUP DELAY RESPONSE MAKER", 69 ) ; 
prline( 69,  "-" ) ; 

    // READ IN FFT HEADER VALUES
    if( readffthead(  &analysis_N,  &analysis_D,  &analysis_R,  &analysis_chan, &k,  normamp, &filter, 1 ) == -1){
        fprintf( stderr, "CHECK YOUR ANALYSIS FILE.\t\t. . . BYE.\n\n\n" ) ; exit(EXIT_FAILURE) ; 
    } ; 



    // ADOPT RATE AND FFT LENGTH FROM ANALYSIS FILE.
    R =  analysis_R ; N = analysis_N ; 

    midC = (220.*pow(2., (3./12.))) ; 
    freqdiff = (float) R / (float) N ; 
    nyquist = R/2.0;
    fundamental =  ((float) R / (float) N) ; 

//fprintf( stderr,  "\nN: %d, R: %d,    BASEFREQ: %f",N, R,    freqdiff ) ; 

// GET NAME OF OUTPUT FILE

/* GET OUTPUT FILE NAME */
	strcpy( ofile, argv[arg_index] ) ; 

    /* OPEN FOR BUSINESS */
    if( (ofd = fopen( ofile, "w+" )) == NULL ){
        printf( "\n\n****** CANNOT OPEN FILE NAMED: %s\n\n", ofile ) ; exit(EXIT_FAILURE) ; 
    } ; 
    


//*********************************
prs( ofile, "GROUP DELAY RESPONSE FUNCTION OUTPUT FILE" ) ; 
prline( 1,  "*" ) ; 
prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
pri( R,  "SAMPLE RATE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 
    if( spectmethod == 0 ){
	prt( "RESPONSE ACCUMULATION METHOD: SHORTEST TIME" );
    }else if( spectmethod == 0 ){
	prt( "RESPONSE ACCUMULATION METHOD: LONGEST TIME" );
    }else {
	prt( "RESPONSE ACCUMULATION METHOD: AVERAGE" );
    }
prline( 1,  "*" ) ; 
prf( default_dB, " DEFAULT DECIBEL LEVEL" ) ; 
prf( default_delay_time, " DEFAULT DELAY TIME" ) ; 
//*********************************

default_amp = dB_to_amp( default_dB ) ; 

//fprintf( stderr,  "\n datafile = %s, datafile2 = %s  ",  datafile,  datafile2 )  ; 
if( (strcasecmp( datafile, datafile2 ) == 0)){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A DATA FILE. BYE.\n\n" ) ;
    usage();
}



//**************************GET DATA	
// READ IN CHORD TONES, SHIFTPOINT, NUMBER_OF_PARTIALS, BW, AND DB

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
// ALLOCATE SPACE FOR CHORD TONES
    fvec( PP, k ) ;	/* PARTIALS */
// COMPUTE NUMBER OF TONES
    np = k / 7 ;

fprintf( stderr,  "\nCHORDRESPONSEMAKER:  \n- datafile values - " ) ;
if(method == 1){
    fprintf( stderr,  "\n*******************************************************************************************" ) ; 
    fprintf( stderr,  "\nOctave.Pitch|Number of   |BW          |Decibels    |Partials    |Top Partial |Delay Time  |" ) ; 
    fprintf( stderr,  "\nClass       |Partials    |Proportion  |            |Spacing Prop|Relative dB |( seconds ) |" ) ; 

}else{
    fprintf( stderr,  "\n*******************************************************************************************" ) ; 
    fprintf( stderr,  "\nFrequency   |Number of   |BW          |Decibels    |Partials    |Top Partial |Delay Time  |" ) ; 
    fprintf( stderr,  "\n            |Partials    |Proportion  |            |Spacing Prop|Relative dB |Delay Time  |" ) ;

}
    fprintf( stderr,  "\n..........................................................................................|" ) ; 




// READ IN VALUES
k = 0 ; 
for( i = 0; i < np ; i++ ){
    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < 7 ; j++ ){
	    fscanf( data,  " %f ",  &PP[ k ] ) ; 
	    fprintf( stderr,  "%-12.3f|", PP[ k ]  ) ;
	     k++ ; 			
    }
} 
    fprintf( stderr,  "\n*******************************************************************************************" ) ; 

// ALLOCATE FILTER SPACE
fvec( F,  N+2  ) ;	/* filter array */

// INITIALIZE ARRAY
    for(i = 1,  j = 0;  i < (N + 2); i += 2,  j++){
	 F[i - 1] = -1 ;  // SET AMP TO -1 AS FLAG FOR LAST PASS
	 F[i] = -1  ; // SET TIME TO -1 AS FLAG FOR LAST PASS
    }

    
    // FOR EACH TONE, REDUCE THE AMP BY ITS SPECIFIED AMOUNT

    // LOOP FOR  TONES

    for( j = 0; j < (np*7) ; j += 7 ){

	// FIND THE TONES FREQUENCY AND MAKE THE BW MULTIPLIER
	if( PP[j] <= 12. ){
 
	// TRANSLATE OCTAVE.PT.PITCHCLASS INTO FREQUENCY AND REPLACE IN ARRAY
	    temp = (float) ((int) PP[j] ) ; // INTEGER PART
	    temp =  (( 12. * (temp - 8.)) + 
		    (100. * (PP[j] - temp)) ) / 12. ;

	    temp = midC * pow( 2., (double) temp ) ; 
		    // FREQUENCY METHOD
	} else{
 
	    // MOVE FREQUENCY INTO TEMP
	    temp = PP[j] ;
	}

	if( PP[j + 2]<= 1. ){
	    // MAKE BW ADDER
	    bw = temp * PP[j + 2] * .5 ;
	}else{
	    bw = PP[j + 2] * .5 ;
	} ; 


	    // IF NUMBER OF PARTIALS IS 0, COMPUTE NUMBER UP TO NYQUIST
	    if( (int) PP[j + 1] <= 0 ){
		PP[j + 1] = (float) ((int)((nyquist - temp) / (temp * PP[j + 4]))) ; 
	    }

	    // CREATE PARTIAL DB ROLLOFF INCREMENT
	    if(PP[j + 1] > 1.){
		// MORE THAN 1 PARTIAL
		dBrolloff = PP[j + 5] / (PP[j + 1] - 1.) ; 
	    }else{
		// JUST 1
		dBrolloff = PP[j + 5] ; 
	    }







	// LOOP FOR PARTIALS
	for( k = 1; k <= (int) PP[j + 1]; k++ ){
	// MAKE PARTIAL FREQUENCY
	    temp3 = temp + (temp * PP[j + 4] * (float) (k - 1)) ; 
	// INDEX OF THE FREQ BIN BELOW THE PARTIAL
	i1 = 1 + (2 * ( rint((double)(temp3 - bw) / freqdiff)) ) ; 
	// INDEX OF THE FREQ BIN ABOVE THE PARTIAL
	i2 = 1 + (2 * ( rint((double)(temp3 + bw) / freqdiff)) )  ;  
	// INDEX OF THE FREQ BIN AT THE PARTIAL
	ipartial = 1 + (2 * ( rint((double)(temp3) / freqdiff)) )  ;  

	// FIND DECIBEL DOWN INCREMENT ABOVE AND BELOW
	    // BELOW
	temp6 = (float) (ipartial - i1) / 2 ;  
	if(temp6 == 0. ) dbdown1 = 0. ; 
	else dbdown1 = dBedge / temp6 ; 
	    // ABOVE
	temp6 = (float) (i2 - ipartial) / 2 ;  
	if(temp6 == 0. ) dbdown2 = 0. ; 
	else dbdown2 = dBedge / temp6 ; 
//fprintf( stderr,  "\ndBedge: %f,  dbdown1: %f,  dbdown2: %f",dBedge,  dbdown1,  dbdown2 ) ;  

    

	// SETUP PARTIAL
	    // IF IN RANGE.....
	    if((ipartial > 0) && (ipartial < N) ){
		// AMP
		temp5 = pow( (double) 10.0, (double)
		 ((PP[j + 3] + (dBrolloff * (float) (k - 1) ) )/20.) ) ;

		value_from_methods( spectmethod, F, PP, temp5, j, ipartial ) ; 
		
	    }


	// LOOP FOR THE ipartial-i1 BINS
	for(i = i1; i < ipartial; i += 2){
	    // IN THE NEXT (ipartial-i1)*2 SPACES IN THE FILTER ARRAY 
	    // PLACE THE APPROPRIATE AMP VALUE FOR THIS TONE
	    
	    // IF IN RANGE.....
	    if((i > 0) && (i < N) ){
		// AMP
		temp6 = (float) ((ipartial - i) / 2) * dbdown1 ; 
		temp5 = pow( (double) 10.0, (double)
		 ((PP[j + 3] + (dBrolloff * (float) (k - 1) ) + temp6 )/20.) ) ;

		value_from_methods( spectmethod, F, PP, temp5, j, i ) ; 


	    }
	}


	// LOOP FOR THE ipartial-i2 BINS
	for(i = i2; i > ipartial; i -= 2){
	    // IN THE NEXT (ipartial-i2)*2 SPACES IN THE FILTER ARRAY 
	    // PLACE THE APPROPRIATE AMP VALUE FOR THIS TONE
	    
	    // IF IN RANGE.....
	    if((i > 0) && (i < N) ){
		// AMP
		temp6 = (float) ((i - ipartial) / 2) * dbdown2 ; 
		temp5 = pow( (double) 10.0, (double)
		 ((PP[j + 3] + (dBrolloff * (float) (k - 1) ) + temp6 )/20.) ) ;


		value_from_methods( spectmethod, F, PP, temp5, j, i ) ; 

	    }
	}

      }

    }

// REPLACE UNTOUCHED BINS WITH DEFAULTS
    for( i = 1; i < (N + 2); i += 2 ){
	if(F[ i ] == -1.){
	    F[ i - 1 ] = default_amp ; 
	    F[ i ] = default_delay_time ; 
	}

    }
    


// NORMALIZE
//*************NORMALIZE THE SYNTHESIZED SPECTRUM
    peakamp = -9999999. ;
    for( i = 1; i < (N + 2); i += 2 ) if( F[ i - 1] > peakamp ) peakamp = F[ i - 1] ;
    normalize(  F,  (N + 2),   peakamp  ) ;

//    for( i = 1; i < N; i += 2 )
//	fprintf( stderr, "\n %f,  %f",  F[i],  F[i - 1] ) ; 


// FIND THE PEAK AND AVERAGE
    minamp = 9999999.; peakamp = -9999999. ; nbins = 0 ; 
    avgbinamp = 0, avgdelay=0.,  mindelay=999999.,  maxdelay=-9999999. ; 
    for( i = 1; i < (N + 2); i += 2 ){
	avgbinamp += F[ i - 1] ; 
	avgdelay += F[ i ] ; 
	if( F[ i ] < mindelay ) mindelay = F[ i ] ; 
	if( F[ i ] > maxdelay ) maxdelay = F[ i ] ; 
	if( F[ i - 1] > peakamp ) peakamp = F[ i - 1] ; 
	if( F[ i - 1] < minamp ) minamp = F[ i - 1] ; 
	if( F[ i ] != 0. ) nbins++ ; 
    }

    avgbinamp = avgbinamp / (float) (N) ; 
    avgdelay = avgdelay / (float) (N) ; 
    averagedB = (float) (20. * log10( (double) avgbinamp )) ; 
    percentofbins = (float) nbins / (float) (N / 2) ; 



// PRINT STATS

// INVERT FILTER IF BAND REJECT
  //  if( invert_flag != 0 ){
//	for(i = 1; i < N; i += 2){
//	    F[i - 1] = 1. - F[i - 1] ; 
//	}
//    }

prf( fundamental,  "FUNDAMENTAL" ) ; 

// LET'S SEE THE FILTER   
    if( vb )tprintspec_groupdelay( F, (N + 2), fundamental,  vb ) ;
    fprintf( stderr,  "\nDELAY RANGE: %6.3f - %6.3f seconds", mindelay,  maxdelay ) ;  
    fprintf( stderr,  "\nAMP RANGE: %6.4f - %6.4f", minamp,  peakamp ) ;  

    fprintf( stderr,  "\nAVERAGE BIN AMP: %4.2f dB", averagedB ) ;  
    fprintf( stderr,  "\nAVERAGE DELAY: %5.2f dB", avgdelay ) ;  
    fprintf( stderr,  "\nNUMBER OF DELAYED BINS: %d in %d, (%3.0f per cent)", 
	    nbins, N / 2, 100. * ((float) nbins / (float) N) ) ;  

 
// WRITE RESPONSE TO stdout
//    for( i = 0; i < N; i++ )write(ofd,  &F[ i ], sizeof(float) ) ;
    for( i = 0; i < (N + 2); i++ ) fwrite( &F[ i ], sizeof(float), 1, ofd ) ;

//    close(ofd) ; 
    fclose( ofd ) ; 

//fprintf( stderr,  "\n\nNUMBER OF SYNTHESIS OUTPUT AMP/FREQ PAIRS:    %d\n",  i/2 ) ; 
    
    
    fprintf(stderr,"\nGROUP DELAY MAKER : SYNTHESIS COMPLETE\n");
  
    fclose( data ) ; 
 
   sprintf( tempstring,  "rm %s", new_datafile ) ; 
   system( tempstring ) ; 


    exit(EXIT_SUCCESS) ;
}


void usage()
{
    fprintf(stderr, "%s",
	"groupdelaymaker: group delay synthesizer\n"
	"groupdelaymaker   [flags]  [output  file]\n"
	"   (This routine makes a response file of dB/delay-time pairs for each of\n"
	"    the FFT's frequency bins (storing delay in place of frequency). \n"
	"    Like chordresponsemaker, it makes the response using multiple tones, \n"
	"	with the added feature of time delay for each tone. )  \n"
	"			(values in brackets represent defaults)\n"
	"	f:	pvanalysis data file (Use full path name.)\n"
	"		   (for determining sample rate and FFT length.)\n" 
	"	F:	(ascii) DATA FILE of unordered septuples:\n"
	"		     Each consisting of: \n"
	"		     (1) pitch or frequency\n"
	"				values <= 12 are interpreted as octave.pitchclass\n"
	"				values > 12 are interpreted as frequency\n"
	"		     (2) number of partials (0 = all partials below Nyquist)\n"
	"		     (3) bandwidth as proportion of fundamental frequency\n"
	"			      values <= 1. are interpreted as proportions of the fundamental\n"
	"				 frequency\n"
	"				values > 1. are interpreted as Hertz\n"
	"		     (4) dB\n"
	"		     (5) partial spacing as proportion of fundamental frequency\n"
	"		     (6) decibel of top partial relative to fundamental\n"
	"		     (7) time delay\n"
	"	D:	dB level at boundaries of partial band [0]\n"
	"		    (relative to partial dB) \n"
	"	s:	GROUP DELAY: Partial Overlap Resolution Method: [2]\n"
	"		    ( When multiple tones produce overlapping data, the\n"
	"		      bin settings are determined using one of the methods below.)\n" 
	"			methods specified below.)\n"
	"		    0 = Use the one with the shortest delay.\n"
	"		    1 = Use the one with the longest delay.\n"
	"		    2 = Take averages of both time delay and loudness. \n"
	"		    3 = Use the loudest one.\n"
	"		    4 = Use the softest one.\n"
	"		    5 = Use the loudest one if it has the shortest delay.\n"
	"		    6 = Use the loudest one if it has the longest delay.\n"
	"	    DEFAULT DB AND DELAY:\n"
	"		( Bins not covered by the data file have their dB and delay time\n"
	"		  set to the default values below.)\n"
	"	i:	Default decibel level [0]\n"
	"	I:	Default delay time [0]\n"
	"	v:	GROUP DELAY RESPONSE PRINTOUT: high cutoff frequency in Hz [0.]\n"
        "	            (0. = off)\n"
	);
    	fprintf(stderr, "%s", "\n\n" ) ; 
	exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }


void value_from_methods(

    int spectmethod, 
    float F[],  
    float PP[],  
    float ampval,  
    int j,  
    int ipartial 
    
    ){
    
    		// SELECT BETWEEN METHODS
		if(spectmethod == 0){
		    // ****** SHORTEST DELAY
		    if( (PP[ j + 6 ] < F[ ipartial ]) || (F[ ipartial ] == -1.) ){
			// USE NEW AMP AND DELAY TIME
			F[ ipartial - 1] = ampval ; F[ ipartial] = PP[ j + 6 ] ;
		    }else{
			// USE OLD
		    }
		}else if(spectmethod == 1){
		    // ****** LONGEST DELAY
		    if( (PP[ j + 6 ] > F[ ipartial ]) || (F[ ipartial ] == -1.) ){
			// USE NEW AMP AND DELAY TIME
			F[ ipartial - 1] = ampval ; F[ ipartial] = PP[ j + 6 ] ;
		    }else{
			// USE OLD
		    }
		}else if(spectmethod == 2){
		    // ****** AVERAGE DELAY AND LEVEL
		    if( F[ ipartial ] == -1. ){
			// NO OLD USE NEW AMP AND DELAY TIME ONLY
			F[ ipartial - 1] = ampval ; F[ ipartial] = PP[ j + 6 ] ;
		    }else{
			// USE AVERAGE OF OLD AND NEW
			F[ ipartial - 1] = (ampval + F[ ipartial - 1]) * .5 ;
			F[ ipartial] = ( PP[ j + 6 ]  + F[ ipartial] ) * .5 ;
		    }
		}else if(spectmethod == 3){
		    // ****** LOUDEST AMP
		    if( (ampval > F[ ipartial - 1 ]) || (F[ ipartial ] == -1.) ){
			// USE NEW AMP AND DELAY TIME
			F[ ipartial - 1] = ampval ; F[ ipartial] = PP[ j + 6 ] ;
		    }else{
			// USE OLD
		    }
		}else if(spectmethod == 4){
		    // ****** SOFTEST AMP
		    if( (ampval < F[ ipartial - 1 ]) || (F[ ipartial ] == -1.) ){
			// USE NEW AMP AND DELAY TIME
			F[ ipartial - 1] = ampval ; F[ ipartial] = PP[ j + 6 ] ;
		    }else{
			// USE OLD
		    }
		}else if(spectmethod == 5){
		    // ****** LOUDEST IF SHORTEST
		    if( 
			((ampval > F[ ipartial - 1 ]) && (PP[ j + 6 ] < F[ ipartial ])) 
			
			|| (F[ ipartial ] == -1.) ){
			// USE NEW AMP AND DELAY TIME
			F[ ipartial - 1] = ampval ; F[ ipartial] = PP[ j + 6 ] ;
		    }else{
			// USE OLD
		    }
		}else if(spectmethod == 6){
		    // ****** LOUDEST IF LONGEST
		    if( 
			((ampval > F[ ipartial - 1 ]) && (PP[ j + 6 ] > F[ ipartial ])) 
			
			|| (F[ ipartial ] == -1.) ){
			// USE NEW AMP AND DELAY TIME
			F[ ipartial - 1] = ampval ; F[ ipartial] = PP[ j + 6 ] ;
		    }else{
			// USE OLD
		    }
		}
}
