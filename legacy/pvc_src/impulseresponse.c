#include "globals.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


void usage(); 
void pd( int i ) ; 


float findPeakAmp(
    float A[],
    int N2
 
) ; 


int main( argc, argv )
    int argc ; char *argv[] ;
{

float *peakInputChannelAmps ; 

float peakAmp ; 
int peakAmpChannelNumber ; 

float low, hi, length, median; 

fpos_t position ;

long size;

int nn, Lh, L ; 
float oldt ; 
int i,j, k, jj, ii ;
float nyquist,   fundamental ;
double atof();
int R=44100, N=1024, N2, Nw = 2048 ;
int   eof = 0, channelout=0 ;
float P = 1.0 ;
FILE *fopen(), *fp,  *adata, *dataOutput ;
char ch ;
float *input, *buffer, *channel ;
float	 gain=1.  ;
float  temp,  temp2,  temp3,  temp4,  temp5,  temp6  ;
float dur ;
int numOutChannels ; 

char decibelsSpectrumPlotFile[ STRING_SIZE ]="" ; 

int   print_flag=0 ; 
float   n=2048 ;
char tempstring[ STRING_SIZE ] ; 

int normalizationFlag=2 ; 

float normalizationLevelInDecibels=0. ; 
float normalizationLevel ; 


if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, 
	"a|b|C|d|e|N|P|h|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) { 


	    case 'd':   normalizationLevelInDecibels = crackfloat( arg_option, ch ) ;
			break;
	    case 'N':   normalizationFlag = (int) crackfloat( arg_option, ch ) ;
                        break;
                        


	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;


	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'a':   strcpy(decibelsSpectrumPlotFile, arg_option); 
			break;

	    case 'P':   print_flag = (int) crackfloat( arg_option, ch ) ;
			break;
	}
}



prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "IMPULSE RESPONSE", 69 ) ; 
prline( 69,  "-" ) ; 

if( 
   (normalizationFlag != 0) && (normalizationFlag != 1) &&
                           (normalizationFlag != 2) 
) 
{
   pri( normalizationFlag, "ILLEGAL NORMALIZATION MODE" ) ; 
   exit(EXIT_FAILURE) ;
} ;

normalizationLevel = dB_to_amp( normalizationLevelInDecibels ) ; 

prf( normalizationLevelInDecibels, "NORMALIZATION LEVEL (in dB)" ) ; 


if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
} else{
		channelflag = 1 ; 
		beginchan = channelout - 1 ; 
} ; 
    // SET NO OUTPUT FLAG
outputoff=1;

// GET INPUT HEADER INFO
setupfiles(argc, argv) ; 

endchan = beginchan + ochan ; 

// GET NAME OF OUTPUT FILE
arg_index++ ; 

if( arg_index >= argc  ){
	bannero() ;
	sprintf( ofile, "pv.impulseresponse" ) ; 
	fprintf( stderr, "\n\n......USING DEFAULT OUTPUT FILENAME........\n\n" ) ;
}else{
/* GET OUTPUT FILE NAME */
	strcpy( ofile, argv[arg_index] ) ; 
}	 

prs( ofile, "IMPULSE RESPONSE ANALYSIS OUTPUT FILE" ) ; 

    /* OPEN FOR BUSINESS */
    
if( (dataOutput = fopen( ofile, "w+" )) == NULL ){
        printf( "\n\n****** CANNOT OPEN FILE NAMED: %s\n\n", ofile ) ; exit(EXIT_FAILURE) ; 
} ; 

     
// **** SET UPS *****

R = isr ; // SAMPLE RATE EQUALS INPUT FILE

	
	// COMPUTE THE DURATION
dur = (endt - begint) ; 
	// COMPUTE N THAT IS NEXT POWER OF 2 GREATER THAN THE SIZE OF INPUT SOUND IMPULSE FILE.

	// IMPULSE LENGTH
Lh = (int)(dur * (float) R) ; 
L = 2 * Lh - 1 ; 
for( N = 1; N < L; N <<= 1 ) ; 
N2 = N>>1 ; 

//*****
//******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
//*********************************


PI = 4.*atan(1.) ;
TWOPI = 8.*atan(1.) ;
nyquist = R/2.0;
fundamental =  ((float) R / (float) N) ; 

    
//***************** PRINT VALUES
prf( dur, "ANALYSIS SEGMENT DURATION" ) ; 

prbanner( "IMPULSE RESPONSE ANALYSIS PARAMETERS",  69 ) ; 
//pri( N,  "FFT SIZE" ) ; 
//prline( 1,  "*" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 


// *******

fvec( buffer, N ) ;		/* FFT buffer */
fvec( channel, N+2 ) ;	/* analysis channels */

// OPEN INPUT  AND OUTPUT FILES
openfiles() ; 



//*********************************************
// LOOP FOR CHANNELS
//*********************************************

//outchan = beginchan ;  channow = 0;

numOutChannels = (endchan - beginchan) ; 

pri( numOutChannels, "NUMBER OF OUTPUT CHANNELS" ) ; 
fvec( peakInputChannelAmps, numOutChannels ) ; 


for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

   prline( 69,   "=" ) ; 
   pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

   //*****   REINITS
   eof = 0 ;  t = 0 ;

   
   // ZERO buffer
   for(i = 0; i < N; i++) buffer[i] = 0. ; 

   // BUFFER SOUND FILE SAMPLES FOR CHANNEL INTO buffer
//   k = 0 ; 
//       while( (( bufferin( &buffer[k] ) ) != 0) && (k < Lh) ) k++ ; 



   k = 0; 
   while( fread( &buffer[k], sizeof(float), 1,  inputTempChanFiles[ outchan ] ) == 1 ){
      k++ ; 
   } ; 	
   fclose( inputTempChanFiles[ outchan ] ) ; 


   if( channow == 0 ){
      Lh = k ; 

      // WRITE RESPONSE HEADER DATA
      // NUMBER OF CHANNELS
      fseek( dataOutput, 0, SEEK_SET ); 

      fwrite( &numOutChannels, sizeof(int), 1, dataOutput ) ; 
      // FFT SIZE
      fwrite( &N, sizeof(int), 1, dataOutput ) ; 
      // IMPULSE LENGTH
      fwrite( &Lh, sizeof(int), 1, dataOutput ) ; 
      // IMPULSE SAMPLE RATE
      fwrite( &R, sizeof(int), 1, dataOutput ) ; 

      pri( numOutChannels, "NUMBER OF IMPULSE OUTPUT CHANNELS" ) ; 
      pri( N, "FFT SIZE" ) ; 
      pri( Lh, "IMPULSE LENGTH IN SAMPLES" ) ; 
      pri( R, "IMPULSE SAMPLE RATE" ) ; 


   } ; 
//pri( k, "k" ) ; 


/*
    // SECRET PLOT
   sprintf( tempstring, "/tmp/thisImpulse%d", channow ) ; 
   prs( tempstring, "WRITING" ) ; 
    adata = fopen( tempstring, "w+" ) ; 
    for(i = 0; i < Lh; i++){
   fwrite( &buffer[i], sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 
*/




   // TRANSFORM TO COMPLEX FREQ
   rfft( buffer, N2, FORWARD ) ;

   // CREATE POLAR FOR SPECTRUM
   convert( buffer, channel, N2, Lh, R ) ;

   peakInputChannelAmps[ channow ] = findPeakAmp( channel, N );  
//   prf( amp_to_dB( peakInputChannelAmps[ channow ] ), "PEAK INPUT AMP" ) ; 


/*
   // NORMALIZE buffer
   temp = -999999999999. ; 
   for( i = 0; i < N; i += 2){
      if(fabs( channel[i] ) > temp ) temp = fabs( channel[i] ) ; 
   } ; 
   if( temp > 0.){
      for( i = 0; i < N; i++) buffer[i] *= (1./temp) ;   
      for( i = 0; i < N; i += 2) channel[i] *= (1./temp) ;   
   } ; 
*/


   // WRITE RESPONSE DATA TO file
//   for( i = 0; i < N; i++ )fwrite( &buffer[ i ], sizeof(float), 1, dataOutput ) ;
   fwrite( buffer, sizeof(float), N, dataOutput ) ;


   // WRITE RESPONSE TO BINARY DECIBELS SPECTRUM FILE
   if( strcmp( decibelsSpectrumPlotFile, "" ) != 0 ){
      sprintf( tempstring, "%s%d",  decibelsSpectrumPlotFile, channow + 1 ) ; 
      writeSpectrumPlotFile( tempstring, channel, (N + 2), 1 ) ; 
      fprintf( stderr, "\nSPECTRUM PLOT FILE FOR IMPULSE WRITTEN TO %s\n", tempstring ) ; 

   }
   if( print_flag )tprintspec( channel, (N + 2), fundamental,  print_flag) ;



}

//for( i = 0; i < numOutChannels ; i++ )
//   prf( amp_to_dB( peakInputChannelAmps[i] ), "PEAK INPUT AMP in decibels" ) ; 



// CLOSE INPUT IMPULSE FILE
if(ifd)fclose(ifd);  

// FIND GREATEST PEAK AMP
peakAmp = peakInputChannelAmps[ 0 ] ; peakAmpChannelNumber = 1 ;
if( numOutChannels > 1 ){
   for( channow = 1 ; channow < numOutChannels ; channow++ ){
      if( peakInputChannelAmps[ channow ] > peakAmp ){
         peakAmp = peakInputChannelAmps[ channow ] ;
         peakAmpChannelNumber = channow + 1 ; 
      } ;
   } ;
} else
{
   peakAmp = peakInputChannelAmps[ 0 ] ;
} ;


if( normalizationFlag != 0 ){

   fseek( dataOutput, 4 * sizeof(int), SEEK_SET ); 
   for( channow = 0; channow < numOutChannels ; channow++ ){
      pri( channow + 1, "CHANNEL" ) ; 


      // SAVE DATA START FOR THIS CHANNEL
      fgetpos( dataOutput, &position ) ; 
      // READ THE IMPULSE RESPONSE BACK IN.
      k = fread( buffer, sizeof(float), N,  dataOutput ) ; 
//      pri( k, "NUMBER READ IN" ) ; 
      // NORMALIZE
      if( peakAmp != 0. )
      {
//         prf( amp_to_dB( peakInputChannelAmps[ channow ] ), "peakInputChannelAmps[ channow ] in dB" ) ; 
            if( normalizationFlag == 2 ) temp = normalizationLevel / peakAmp ; 
            else if( normalizationFlag == 1 ) temp = normalizationLevel / peakInputChannelAmps[ channow ] ;
            for( i = 0; i < N; i++ ) buffer[ i ] *= temp ;
            // CREATE POLAR FOR SPECTRUM
            convert( buffer, channel, N2, Lh, R ) ;
            temp = findPeakAmp( channel, N ) ; 
            fprintf( stderr, "\nNORMALIZED TO DECIBELS PEAK OF %5.2f", amp_to_dB( temp )  ) ; 
      } ;
      // RESET FILE POINTER TO DATA BEGIN POINT AND WRITE BACK MODIFIED DATA TO FILE. 
      fsetpos (dataOutput, &position ) ;      
      fwrite( buffer, sizeof(float), N, dataOutput ) ;
   } ;

   if( normalizationFlag == 1 ) 
      fprintf( stderr, "\n\nIMPULSE RESPONSE CHANNELS NORMALIZED INDEPENDENTLY TO %5.2f dB.\n",
         normalizationLevelInDecibels ) ;   
   else
      fprintf( stderr, "%s%5.2f%s%d%s",
         "\n\nIMPULSE RESPONSE CHANNELS NORMALIZED TO ", normalizationLevelInDecibels,
         " dB\nAGAINST PEAK IN CHANNEL ", peakAmpChannelNumber, "."
      ) ;   


} else
{
   prt( "NORMALIZATION IS OFF. (Amplitudes will be *very* low.)" ) ; 
} ;


// CLOSE OUTPUT IMPULSE ANALYSIS DATA FILE
fclose(dataOutput) ; 



prt( "*********************************************" ) ;  
prs( ifile, "INPUT SOUND FILE" ) ; 

fprintf(stderr,"\n\nIMPULSE RESPONSE : ANALYSIS COMPLETED\n\n");
exit(EXIT_SUCCESS) ;
}

void usage(){

fprintf(stderr, "%s",
   "impulseresponse:  impulse response analysis \n"
   "impulseresponse   [flags] [input sound file] [output impulseresponse file]\n"
   "	b:	"BEGIN_TIME		// begint
   "	e:	"END_TIME			// endt
   "	C:	"RESYNTHESIS_CHANNEL		// channelout
   "	d:	amplitude normalization level in decibels [0]\n" 
   "	N:       normalization:                [1]\n"
	"	   0: Do not normalize. \n"
	"	   1: Normalize channels independently.\n"
	"	   2: Normalize channels together against channel with peak amplitude.\n"

   "	a:	Decibels Spectrum Plot File\n"

   "	P:	"FREQUENCY_RESPONSE_PRINTOUT
);
exit(EXIT_SUCCESS);
}


void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

float findPeakAmp(
    float A[],
    int N2
 
){
    float peakAmp ;
    int i ;  

   // FIND STRONGEST FREQ
    peakAmp = -999999.0 ;
    for( i = 0; i < N2; i += 2) if( A[i] > peakAmp ) peakAmp = A[i] ; 

    return( peakAmp ) ; 

} ; 
