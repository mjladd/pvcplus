#include "globals.h"

#define NUM_PARAMETERS 2

void usage() ; 

void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,k, l,  i1,  i2, ipartial,   n, exflag, 
     NC, NC2,   np,  nf=2,  first=1  ;
float nyquist;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0 ;
float P = 1.0;
FILE *fopen(), *fp ; 
SNDFILE *infile;
char ch;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel,  *output ;
float *previous_channel,  *F,  *FT,  *SUM,  *harmony ; 
 float threshfac = .001;
float	gain=1. ;
float  temp,  temp2,  temp3,  temp4,  temp5,  temp6 ;  
float getthresh();
float peakbinamp = 0.,  avgbinamp=0.,  averagedB,  peakamp=0,  peakfreq, 
    dBedge=0,  dbdown1,  dbdown2,  percentofbins, dBrolloff ;
float fundamental ; 
float freq1, freq2 ; 
int   invert_flag=0,  nbins=0,  spectmethod=0 ;
float   IR,  dur=0.;
int flag ; 
char dBonlyspectfile[ STRING_SIZE ] ;
int  dBSpectFileflag=0 ; 

int N_limit=0 ; 
// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 
int eqnormbypassflag=0 ; 

FILE *data ; 
char datafile[ STRING_SIZE ] = "EMPTY\0",  new_datafile[ STRING_SIZE ] ; 
char datafile2[ STRING_SIZE ] = "EMPTY\0" ;
char soundfile[ STRING_SIZE ] ;  
SF_INFO inputSFinfo ; 

float *PP, *domainamp, *partialamp,    fund=100.,  freqdiff,  target_gain,  SOURCE_gain,  midC,  part ; 
int nd=0,  vb=0 ; 
float bw1,  bw2,  bw ; 
char tempstring[ STRING_SIZE ] ; 

if( argc < 2 )usage() ; 
 

    while( (ch= crack( argc, argv, "a|A|f|F|R|N|Z|D|s|v|i|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':	N = (int) crackfloat( arg_option, ch );
			break;
	    case 'A':	N_limit = (int) crackfloat( arg_option, ch );
			break;

	    case 'i':   invert_flag = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'F':   strcpy(datafile, arg_option);
			break;

	    case 'f':   strcpy( soundfile, arg_option); 		
				fixTildeInFilename( soundfile ) ; 
			break ; 

	    case 'a':   strcpy(dBonlyspectfile, arg_option); dBSpectFileflag = 1 ; 
			break;

	    case 'v':	vb = (int) crackfloat( arg_option, ch ) ;
			break;

	}
    }

//*****************
prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "FILTRESPONSEMAKER", 69 ) ; 
prline( 69,  "-" ) ; 

// GET RATE FROM TARGET SOUNDFILE.
    if (! (infile = sf_open( soundfile, SFM_READ, &inputSFinfo )))
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
prline( 1,  "*" ) ; 
    if( invert_flag )prt( "RESPONSE MODE: REJECTION " );
    else prt( "RESPONSE MODE: PASS" ) ;
prline( 1,  "*" ) ; 
//*********************************



//fprintf( stderr,  "\n datafile = %s, datafile2 = %s  ",  datafile,  datafile2 )  ; 
if( (strcasecmp( datafile, datafile2 ) == 0)){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A DATA FILE. BYE.\n\n" ) ;
    usage();
}


//**************************GET DATA	
// MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED

    cut_data_lines( datafile,  new_datafile,  NUM_PARAMETERS ) ; 

// READ IN FILTER BREAKPOINT DUPLES
// OPEN FILE
	if( (data = fopen( new_datafile, "r")) == NULL ){
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  new_datafile ) ; 
	    exit(EXIT_FAILURE); 
	}
// COUNT VALUES IN FILE
	k = 0 ; 
	while( fscanf( data,  " %f ",  &temp ) != EOF ) k++ ; 			

	rewind( data ) ;    
// ALLOCATE SPACE FOR FILTER BREAKPOINTS
    fvec( PP, (k + 4) ) ;	/* PARTIALS */
// COMPUTE NUMBER OF BREAKPOINTS
    np = k / 2 ;
// ABORT IF TOO FEW
    if( np < 2 ){
	prt( "NOT ENOUGH FILTER BREAKPOINTS. BYE" ) ; 
	exit(EXIT_FAILURE) ; 
    }
// READ IN VALUES
k = 0 ; 
for( i = 0; i < np ; i++ ){
//    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < 2 ; j++ ){
	    fscanf( data,  " %f ",  &PP[ k ] ) ; 
	     k++ ; 			
    }
} 
// SORT THEM
flag = 0 ; 

while( flag == 0 ){
    flag = 1 ; 
    for( i = 1,  j = 2,  k = 0; i < np ; i++,  j += 2, k += 2  ){
	freq1 = ( PP[j] <= 12 ) ? OPPC_to_Hz( PP[j] ) : PP[j] ; 
	freq2 = ( PP[k] <= 12 ) ? OPPC_to_Hz( PP[k] ) : PP[k] ; 
	if( freq1 <  freq2 ){
	    // SWITCH
	    temp2 = PP[ j ] ; temp3 = PP[ j + 1 ] ; 
	    PP[ j ] = PP[ k ] ; PP[ j + 1 ] = PP[ k + 1 ] ; 
	    PP[ k ] = temp2 ; PP[ k + 1 ] = temp3 ; 
	    flag = 0 ; 
	}
    } 
}


// PRINT VALUES

fprintf( stderr,  "\nFILTRESPONSEMAKER:  \n- datafile values - " ) ;
    fprintf( stderr,  "\n**************************" ) ; 
    fprintf( stderr,  "\nFrequency or|Decibels    |" ) ; 
    fprintf( stderr,  "\nOct.PClass  |            |" ) ; 

    fprintf( stderr,  "\n.........................|" ) ; 


k = 0 ; 
for( i = 0; i < np ; i++ ){
    fprintf( stderr,  "\n" ) ;
    for( j = 0; j < 2 ; j++ ){
	    fprintf( stderr,  "%-12.3f|", PP[ k ]  ) ; k++ ; 			
    }
} 


    fprintf( stderr,  "\n**************************" ) ; 

// TRANSLATE TO FREQ IF NECESSARY
	for( j = 0; j < (np*2) ; j += 2 ){
	    if( PP[j] <= 12. ) PP[j] = OPPC_to_Hz( PP[j] ); 
	}



// PUT O Hz LINE IN IF ABSENT
if( PP[ 0 ] > 0.){
    // SHIFT THEM
    for( i = ((2 * np) - 1) ; i >=  0; i-- )PP[ i + 2 ] = PP[ i ] ; 
    PP[ 0 ] = 0. ; PP[ 1 ] = PP[ 3 ] ; 
    np++ ; 
}

// PUT Nyquist Hz IN IF ABSENT
if( PP[ (np - 1) * 2 ] < nyquist){
    np++ ; 
    PP[ (np - 1) * 2 ] = nyquist ; 
    PP[ ((np - 1) * 2) + 1 ] = PP[ ((np - 2) * 2) + 1 ] ; 
}


// FIND SMALLEST BREAKPOINT FREQUENCY DIFFERENCE AND AUTO ADJUST
if( N_limit > 0 ){
    temp = 99999999.; 
    for( j = 2; j < (np*2) ; j += 2 ){
	if( (PP[ j ] - PP[ j - 2 ]) < temp ) temp = (PP[ j ] - PP[ j - 2 ]) ; 
    }
    if( (temp < freqdiff) ){
	// ADJUST
	prt( "-----> FFT IS TOO SMALL FOR BREAKPOINT FREQUENCY DIFFERENCES <-----------" );
	prt( "-----> RESETTING FFT SIZE..............................." ); 
	while( (temp < freqdiff) && ( (N * 2) < N_limit) ){
	    N *= 2 ; 
	    freqdiff = (float) R / (float) N ;
	}
	pri( N,  "------> NEW FFT SIZE" ) ; 
	fundamental = (float) (R / N) ; 
	prf( fundamental, "------> NEW FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
    
    }
}

// ALLOCATE FILTER SPACE
fvec( F,  N+2  ) ;	/* filter array */

// INITIALIZE ARRAY
    for(i = 1,  j = 0;  i < (N + 2) ; i += 2,  j++){
	 F[i - 1] = 0 ;  // SET AMP TO ZERO
	 F[i] = (float) (j)  * freqdiff  ; // SET FREQ
    }

    
// MAKE RESPONSE
    k = 0 ;
    for( i = 1,  j = 0; i < (N + 2); i += 2,  j++ ){

	while( PP[ k + 2 ] < F[i]   ){
	    // INCREMENT TO NEXT BREAKPOINT
	    k += 2 ; 
	}
	temp = (F[i] - PP[ k ]) / (PP[ k + 2 ] - PP[ k ]) ; 
	temp3 = curve( PP[ k + 1], PP[k + 3], temp,  0. ) ; 
	F[i - 1] = dB_to_amp( temp3 ) ; 

    }



// FIND THE PEAK AND AVERAGE
    peakamp = 0 ; peakfreq = 0 ; avgbinamp = 0 ; 
    for( i = 1; i < (N + 2); i += 2 ){
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


//    avgbinamp = avgbinamp / (float) (nbins) ; 
//    averagedB = amp_to_dB( avgbinamp ) ; 
//    percentofbins = (float) nbins / (float) (N / 2) ; 



// PRINT STATS
//    fprintf( stderr,  "\n\nAVERAGE BIN AMP: %f dB", averagedB ) ;  
//    fprintf( stderr,  "\n\nNUMBER OF NON-ZERO BINS: %d", nbins ) ;  
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
    for( i = 0; i < (N + 2); i++ )fwrite(&F[ i ], sizeof(float), 1, ofd ) ;

    fclose(ofd) ; 

fprintf( stderr,  "\n\nNUMBER OF SYNTHESIS OUTPUT AMP/FREQ PAIRS:    %d\n",  i/2 ) ; 
    
    
    fprintf(stderr,"\nFILTRESPONSEMAKER : SYNTHESIS OF RESPONSE COMPLETED\n");
  
    fclose( data ) ; 

    sprintf( tempstring,  "rm %s", new_datafile ) ; 
    system( tempstring ) ; 

 
    exit(EXIT_SUCCESS) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"filtresponsemaker:  frequency response synthesizer\n"
	"			(frequency gradient format)  \n"
	"			(values in brackets represent defaults)\n"
	"filtresponsemaker   [flags]  [output freqresponse file]\n"
	"	N:	"FFT_LENGTH 		// N
	"	A:	Auto-adjust FFT size limit (0 = auto-adjust off) [0.]\n"
	"		    (In auto-adjust mode, the smallest  difference between\n"
	"		     adjacent breakpoint frequencies is used to set the FFT size, \n"
	"		    -A sets the maximum size; 0 turns auto-adjustment off.)\n"
	"	F:	(ascii) DATA FILE of unordered breakpoint duples:\n"
	"		     Each represented by: \n"
	"		     (1) octave.pitchclass or frequency \n"
	"			(Values of 12 or less are treated as octave.pitchclass.)\n"
	"		     (2) decibels\n"
	"	f:	target sound file name \n"
	"		     The frequency response sample rate is taken from the\n"
	"			target sound file.\n" 
	"	i:	filter type: bandpass (0),  band reject (1) [0]\n"
	"	a:	decibels spectrum plot file\n"
	"	v:	"FREQUENCY_RESPONSE_PRINTOUT


	);
	exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

