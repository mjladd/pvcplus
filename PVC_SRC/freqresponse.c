#include "globals.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

float findPeakAmp(
    float A[],
    int N2
 
) ; 


void usage(); 

void pd( int i ) ;


void writeOutASCIIformantData(
    char formantsASCIIdataFile[],
    float formantCenterFreqs[], 
    float formantAmps[],
    float formantBWs[], 
    float formantQs[],
    int formantIndices[],
    int formantLowStopBandIndices[],
    int formantHighStopBandIndices[],
    int numFormants
) ; 



int main( argc, argv )
    int argc ; char *argv[] ;
{


float low, hi, avg, length, median; 

int nn,  sec,  min ; 
float oldt ; 
int i,j, k, jj, ii,   exflag;
float nyquist,   fundamental ;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 256, I = 256, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0 ;
float P = 1.0, coef=0.0 ;
FILE *fopen(), *fp,  *adata;
char ch, tempResponseFile[ STRING_SIZE ], scratchString[ STRING_SIZE ] ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, *buffer, *channel, *channel_forFormantSelection,   
	*previous_channel, *output ;
float *F, *FreqStasisSpectrum, FreqStasisSpectrumAmpSum=0.0, *AmplitudeSpectrum, 
		*AmplitudeSpectrumForFormantSelection, *OutputSpectrum, 
		*binAmpSumAndFreqSum, *SD, channel_Save, threshold,
		compandingIndex=0.0 ;
float *PeakWindowedAmps, *AvgWindowedAmps ; 
int numSaveFrames, saveFramesCount ;  
float	gain=1., stdDev  ;
float  temp,  temp2,  temp3,  temp4,  temp5,  temp6  ;
float getthresh();
float peakbinamp = 0.,  avgbinamp=0.,  peakamp,  dur, minDev, maxDev ;
float freqChangePerMilisecond=1.0 ; 

int numFormants, *formantIndices, *formantLowStopBandIndices, *formantHighStopBandIndices ;

float *formantCenterFreqs, *formantAmps, *formantBWs, *formantQs ; 

float lowFreqLimit=0., highFreqLimit=0., minimumFormantDB=-96. ;  

int CorrelateWithFreqStasisFlag=0 ; 

char decibelsSpectrumPlotFile[ STRING_SIZE ]="", formantsFile[ STRING_SIZE ]="",  formantsASCIIdataFile[ STRING_SIZE ]="" ; 

char formantsOutputFile[ STRING_SIZE ]="" ; 



char formantsBarPlotFile[ STRING_SIZE ]="" ;

char freqStasisPlotFile[ STRING_SIZE ]="" ; 

float *barPlot, minFormantAmp, minFormantdB  ; 
int peakFormantFreq ; 

float peakdB, avgdB, diffdB, dBadjust, thisdB ; 

int lowIndex, highIndex, thisHammingIndex, HammingWindowSize=1024 ; 
float ampSum, thisHammingSum, thisAmp, peakAmp, *peakAmps, *avgAmps, *peakAvgProps, *HammingWindow, 
	cf, centroidFreq, sumOfAmps, peakAveragePropIndB ; 





int numClipped=0, normalizeToPeaksFlag=0 ; 
int Formants_N ; 
int weightflag=1 ; 
int   print_flag=0, buffer_count=0 ; 
float   IR, DR,  n=2048 ;
char tempstring[ STRING_SIZE ] ; 

// SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 
int eqnormbypassflag=0 ; 

int method_flag=0 ; 

float totalframesumForWeightingAmplitudeSpectrum=0.,framesumForWeightingAmplitudeSpectrum, 
		*binDiffsframesumForWeightingAmplitudeSpectrum, *binDiffsframesumForWeightingAmplitudeSpectrumSums ; 

// FILTER
struct  func  freqresponse ; 

float Formant_Selection_Threshold__0_to_1=0.5 ; 

// FILTER
freqresponse.L = 1. ; freqresponse.n = 0. ; freqresponse.A[ 0 ] = 0. ; 


if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, 
	"a|A|b|B|c|C|d|D|e|E|F|g|G|h|H|i|I|j|J|k|l|L|m|M|N|o|p|P|R|s|t|w|W|X|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) { // f K


	    case 'N':   N = (int) crackfloat( arg_option, ch );
			break;
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;


	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;



	    case 'p':   normalizeToPeaksFlag = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'B':   eqnormbypassflag = (int) crackfloat( arg_option, ch ) ;
			break;


	    case 'E':   compandingIndex = crackfloat( arg_option, ch ) ;
			break;



	    case 'H':   dBlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'X':   dBhi = crackfloat( arg_option, ch ) ;
			break;
	    case 'm':   freqlow = crackfloat( arg_option, ch ) ;
			break;
	    case 'R':   freqhi = crackfloat( arg_option, ch ) ;
			break;

	    case 'W':   weightflag = (int) crackfloat( arg_option, ch ) ; 
			break;

// *****

	    case 'L':   lowFreqLimit = crackfloat( arg_option, ch ) ;
			break;
	    case 'j':   highFreqLimit = crackfloat( arg_option, ch ) ;
			break;
	    case 'A':   minimumFormantDB = crackfloat( arg_option, ch ) ;
			break;



	    case 'g':   Formant_Selection_Threshold__0_to_1 = crackfloat( arg_option, ch ) ;
			break;

	    case 'a':   strcpy(decibelsSpectrumPlotFile, arg_option); 
			break;
	    case 'o':   strcpy(formantsBarPlotFile, arg_option); 
			break;
	    case 'i':   strcpy(formantsASCIIdataFile, arg_option); 
			break;

	    case 'F':   strcpy(formantsOutputFile, arg_option); 
			break;


	

// ******


	    case 'c':   method_flag = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'P':   print_flag = (int) crackfloat( arg_option, ch ) ;
			break;
	}
    }



prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "FREQRESPONSE", 69 ) ; 
prline( 69,  "-" ) ; 



    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
    // SET NO OUTPUT FLAG
    outputoff=1;

// GET INPUT HEADER INFO
    setupfiles(argc, argv) ; 

    endchan = beginchan + ochan ; 

// GET NAME OF OUTPUT FILE
    arg_index++ ; 

    if( arg_index >= argc  ){
	bannero() ;
	sprintf( ofile, "pv.freqresponse" ) ; 
	fprintf( stderr, "\n\n......USING DEFAULT OUTPUT FILENAME........\n\n" ) ;
    }else{
/* GET OUTPUT FILE NAME */
	strcpy( ofile, argv[arg_index] ) ; 
    }	 

    prs( ofile, "FREQUENCY RESPONSE OUTPUT FILE" ) ; 

    /* OPEN FOR BUSINESS */
    
    if( (ofd = fopen( ofile, "w+" )) == NULL ){
        printf( "\n\n****** CANNOT OPEN FILE NAMED: %s\n\n", ofile ) ; exit(EXIT_FAILURE) ; 
    } ; 

     
// **** SET UPS *****
    R = isr ; // SAMPLE RATE EQUALS INPUT FILE
    if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 200.\n\n" ) ; 
	frames_per_sec = 200 ; 
    }
    D = (int) ((float) R / frames_per_sec) ; 
//*****
//******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
//*********************************


    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
    fundamental =  ((float) R / (float) N) ; 
    obank = P != 0. ;
    if( P == 0.0 ) {P = 1.0;}
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    DR = (float) D / (float) R ; 
    // COMPUTE THE DURATION
    dur = (endt - begint) ; 

	if( highFreqLimit == 0. ) highFreqLimit = nyquist ; 

    
//***************** PRINT VALUES
prf( dur, "ANALYSIS SEGMENT DURATION" ) ; 

prbanner( "FREQRESPONSE ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 

pri( frames_per_sec,  "FRAMES/SECOND" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
prline( 1,  "*" ) ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
if(eqnormbypassflag == 0)prt( "EQ WITH NORMALIZATION: ON" ) ; 
    else prt( "EQ WITH NORMALIZATION: OFF" ) ;
prt( "*...........................................*" ) ;  
prt( "SPECTRAL METHOD:" ); 
if(method_flag == 0) prt( "\tAMPLITUDE-WEIGHTED AVERAGE OF FRAMES" ) ;
if(method_flag == 1) prt( "\tPEAK AMPLITUDE FRAME" ) ;

if(normalizeToPeaksFlag == 1) prt("NORMALIZING SPECTRUM TO FORMANT PEAKS" ) ; 
prf(compandingIndex, "COMPANDER INDEX" ) ;


prf( lowFreqLimit, "FORMANT ANALYSIS: LOW FREQUENCY LIMIT" ) ; 
prf( highFreqLimit, "FORMANT ANALYSIS: HIGH FREQUENCY LIMIT" ) ; 

prf( minimumFormantDB, "FORMANT ANALYSIS: MINIMUM FORMANT AMPLITUDE IN DB" ) ; 
prf( Formant_Selection_Threshold__0_to_1, "0-1 Formant Selection/Rejection Threshold" ) ; 


	
//if( CorrelateWithFreqStasisFlag == 1)prt( "CORRELATING FORMANTS WITH FREQUENCY STASIS" ) ;



// *******

    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( channel_forFormantSelection, N+2 ) ;	/* analysis channels */
    fvec( previous_channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */


	fvec( peakAmps, N2+1 ) ; fvec( avgAmps, N2+1 ) ;

	fvec( binDiffsframesumForWeightingAmplitudeSpectrum, N2+1 ) ; 
	fvec( binDiffsframesumForWeightingAmplitudeSpectrumSums, N2+1 ) ; 

	for( i = 0; i < (N2 + 1); i++ ) binDiffsframesumForWeightingAmplitudeSpectrumSums[i] = 0. ; 

// MAKE SPECTRUM AVERAGE/PEAK/MINIMUM ARRAY
    fvec( AmplitudeSpectrum, N+2 ) ;	/* spectrum average or peak array */

    	for(i = 0; i < (N+2); i++ ){
		AmplitudeSpectrum[ i ] = 0. ;
	} ; 

	fvec( FreqStasisSpectrum, N+2 ) ; 
	for( i = 0; i < (N+2); i++ ) FreqStasisSpectrum[ i ] = 0. ; 



    fvec( binAmpSumAndFreqSum, N+2 ) ;	/* spectrum average or peak array */
    for(i = 0; i < (N+2); i++ ) binAmpSumAndFreqSum[ i ] = 0. ; 


    fvec( OutputSpectrum, N+2 ) ; 


	// FIRST MAKE HAMMING WINDOW 
	fvec( HammingWindow, HammingWindowSize ) ; 
	for ( i = 0 ; i < HammingWindowSize ; i++ ){
          HammingWindow[i] = 0.54 - (0.46 * cos( (double)(TWOPI * (float) i / (float)(HammingWindowSize - 1)  ) ) ) ;
	} ;


// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 


    minDev = 999999999999.0 ; maxDev = -9999999.0 ; avg = 0.0 ; 

//    		adata = fopen( "/tmp/frameSum", "w+" ) ; 



//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; 



    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;


	oldt = 0. ; 
	sec = 0 ;
	min = 0 ;  
	nn = 0 ;
	samps = 0 ;   
	
//*********************************************
// LOOP FOR FRAMES
//*********************************************

    while ( !eof ) {
	in += D ;
	on += I ;
	timenow( dur ) ;

	 	  eof = shiftin( input, Nw, D ) ;
	  	  fold( input, Wanal, Nw, buffer, N, in ) ;
	  	  rfft( buffer, N2, FORWARD ) ;
	  	  convert( buffer, channel, N2, D, R ) ;

		for(i = 0; i < (N + 2); i++) channel_forFormantSelection[i] = channel[i]; 

	if( frame_count == 0 ) 
		for( i = 0; i < (N + 2); i++ ) previous_channel[i] =  channel[i] ;  



	// PASSIFIER PRINT
	if( (t - oldt) > .25 ){
	    // PRINT 
	    if( nn == 0 ){

		if( sec == 0 ){
		    // PRINT MINUTE
		    if( min != 0 ) {
			if( min == 1) 
				fprintf( stderr, "\n\n ---------> %d minute ......................\n\n", min ) ;
			else fprintf( stderr, "\n\n ---------> %d minutes ......................\n\n", min ) ;
		    }else{
			fprintf( stderr, "\n\n" ) ; 
		    
		    } 
		    min++ ; 
		}
		
		// PRINT SECONDS
		
		    fprintf( stderr, " %d ",  sec ) ; 
		    sec++ ; if( sec >= 60 ) sec -= 60 ; 
		
	    }else{
		// PRINT QUARTER SECOND STARS
		fprintf( stderr, "*" ) ; 
	    }
	    // UPDATE TIME
	    oldt = oldt + .25 ;
	      
	    nn++ ; if( nn >= 4 ) nn -= 4 ;  

	}

	samps += D ; 


//*****************
// MODIFICATIONS LOOP
//*****************

	// NEW WEIGHTING SCHEME, AGAINST LOCAL PEAK
	// FIND CENTROID
	centroidFreq = 0.0 ; sumOfAmps = 0.0 ; 
	for( i = 0; i < (N + 2); i += 2) {
		centroidFreq += (channel_forFormantSelection[i + 1] * channel_forFormantSelection[i]) ; 
		sumOfAmps += channel_forFormantSelection[i] ; 
    	} ; 
    	centroidFreq = centroidFreq / sumOfAmps ; 



	if( weightflag == 1 ){

		framesumForWeightingAmplitudeSpectrum = 0. ; 

		// SUM FRAME AMPLITUDES
		for( i = 0; i < (N + 2); i+= 2) framesumForWeightingAmplitudeSpectrum += pow( channel[i], 5. )   ; 

	} ; 

	//

	for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){
 
	    if( method_flag == 0){
	        	// AVERAGE METHOD: SUM WEIGHTED AMPLITUDE FRAMES
			if( weightflag == 1 ){
		    		// WEIGHTED MODE
		    		AmplitudeSpectrum[i - 1] += (framesumForWeightingAmplitudeSpectrum * channel[i - 1]) ;
			}else{
		    		// NON-WEIGHTED MODE
		    		AmplitudeSpectrum[i - 1] = AmplitudeSpectrum[i - 1]  + channel[i - 1] ;
			} ; 
			AmplitudeSpectrum[ i ] += channel[ i ] ;	

	    }else if( method_flag == 1){
		// PEAK METHOD
			if(channel[i - 1] > AmplitudeSpectrum[i - 1]) AmplitudeSpectrum[i - 1] = channel[i - 1] ;
			AmplitudeSpectrum[ i ] += channel[ i ] ;	

	    } ; 
		
			// COLLECT FREQ STATS		
	    binAmpSumAndFreqSum[i - 1] += channel[i - 1] ;
	    binAmpSumAndFreqSum[i] += (channel[i] * channel[i - 1]) ; 
	
	} ; 

	totalframesumForWeightingAmplitudeSpectrum += framesumForWeightingAmplitudeSpectrum ; 

	frame_count++ ; 
	buffer_count++ ;
	for( i = 0; i < (N + 2); i++ ) previous_channel[i] = 
			((1.0 - coef) * channel[i])  + (coef * previous_channel[i]) ;  



// FRAMES LOOP END
    }

// CHANNELS LOOP END
} 

    // CLOSE  INPUT FILE
fclose(ifd) ;  

//    fclose( adata ) ; 




	// MAKE AVERAGE FREQ
for( i = 1; i < (N + 2); i+= 2)
    AmplitudeSpectrum[ i ] = binAmpSumAndFreqSum[ i ] / binAmpSumAndFreqSum[i - 1]  ;


// TAKE AVERAGE NOW IF AVERAGE METHOD
if( method_flag == 0 ){	// AMPLITUDE
    for( i = 1, j = 0; i < (N + 2); i+= 2, j++){
	if( weightflag == 1 ){
	// WEIGHTED MODE
		AmplitudeSpectrum[i - 1] /= totalframesumForWeightingAmplitudeSpectrum ;
	}else{
	   // NON-WEIGHTED MODE
	    AmplitudeSpectrum[i - 1] = AmplitudeSpectrum[i - 1] / (float) buffer_count ;
	}
    }	
} ; 

    // TRANSFER TO OUTPUT SPECTRUM
for( i = 0; i < (N + 2); i++ ) OutputSpectrum[i] = AmplitudeSpectrum[i] ; 

    // IF FLAG OFF, EQUALIZE AND NORMALIZE THE OUPUT SPECTRUM
if( eqnormbypassflag != 1 ){
    eq( OutputSpectrum,  (N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1, 0, 1 ) ; 
} ; 


temp = findPeakAmp( OutputSpectrum, (N + 2) );  


/*
    // SECRET PLOT OF newAmps
    adata = fopen( "/tmp/testf", "w+" ) ; 
    for(i = 0; i < (N + 2); i += 2){
	temp = amp_to_dB( AmplitudeSpectrumForFormantSelection[i] ) ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 

    // SECRET PLOT OF newAmps
    adata = fopen( "/tmp/testo", "w+" ) ; 
    for(i = 0; i < (N + 2); i += 2){
	temp = amp_to_dB( OutputSpectrum[i] ) ; 
	fwrite( &temp, sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 
*/


fvec( formantCenterFreqs, N2+1 ) ; fvec( formantAmps, N2+1 ) ; 
fvec( formantBWs, N2+1 ) ; fvec( formantQs, N2+1 ) ; 
ivec( formantIndices, N2+1 ) ; 
ivec( formantLowStopBandIndices, N2+1 ) ; ivec( formantHighStopBandIndices, N2+1 ) ; 






get_formants( 
    &numFormants,  
    formantCenterFreqs, formantAmps, formantBWs, formantQs, 
    formantIndices, formantLowStopBandIndices, formantHighStopBandIndices, 
    OutputSpectrum, (N + 2), 
    lowFreqLimit, highFreqLimit, minimumFormantDB,
    Formant_Selection_Threshold__0_to_1, freqStasisPlotFile, CorrelateWithFreqStasisFlag, nyquist 
) ; 



temp = findPeakAmp( OutputSpectrum, (N + 2) );  


    // WRITE OUT BAR PLOT FILE OF ZEROS AND FORMANT DBS AT RESPECTIVE FREQUENCIES.
if( strcmp( formantsBarPlotFile, "" ) != 0 ){
    
    peakFormantFreq = (int) (formantCenterFreqs[ numFormants - 1]) ; 
    fvec( barPlot, 22050 ) ; 

    minFormantAmp = 9999999999.0 ; 
    for( i = 0; i < numFormants; i++){
        if( formantAmps[ i ] < minFormantAmp ) minFormantAmp = formantAmps[ i ] ; 
    } ; 
    minFormantdB = amp_to_dB( minFormantAmp ) * (5.0 / 4.0) ; 
    
    for( i = 0; i < 22050; i++) barPlot[i]= minFormantdB ;
 
    for( i = 0; i < numFormants; i++){
        k = (int) formantCenterFreqs[ i ] ; 
        temp = amp_to_dB( formantAmps[ i ] );
        if( temp < minFormantdB ) temp = minFormantdB ;  
        barPlot[ k ] = temp ; 
    } ; 

    adata  = fopen( formantsBarPlotFile,  "w+") ;
    for( i = 0; i < 22050; i++ ) fwrite( &barPlot[ i ], sizeof(float), 1, adata ) ;
    fclose( adata ) ; 

} ; 

if( strcmp( formantsOutputFile, "" ) != 0){
    // WRITE FORMANTS TO FILE
    adata  = fopen( formantsOutputFile,  "w+") ;
    fwrite( &numFormants, sizeof(int), 1, adata ) ;
    fwrite( &N2, sizeof(int), 1, adata ) ;
    for( i = 0; i < numFormants; i++){
        fwrite( &formantCenterFreqs[i], sizeof(float), 1, adata ) ; 
        fwrite( &formantAmps[i], sizeof(float), 1, adata ) ;
        fwrite( &formantBWs[i], sizeof(float), 1, adata ) ;
        fwrite( &formantQs[i], sizeof(float), 1, adata ) ;
        fwrite( & formantIndices[i], sizeof(int), 1, adata ) ;
        fwrite( & formantLowStopBandIndices[i], sizeof(int), 1, adata ) ;
        fwrite( & formantHighStopBandIndices[i], sizeof(int), 1, adata ) ;
    } ; 
    fclose( adata ) ; 
} ; 

writeOutASCIIformantData(   
    formantsASCIIdataFile,
    formantCenterFreqs, 
    formantAmps,
    formantBWs, 
    formantQs,
    formantIndices,
    formantLowStopBandIndices,
    formantHighStopBandIndices,
    numFormants
) ; 




NormalizeToPeaksOfSpectrumInBand( normalizeToPeaksFlag, OutputSpectrum, (N + 2), numFormants, 
			formantIndices, formantLowStopBandIndices, formantHighStopBandIndices, 
	compandingIndex ) ; 

temp = findPeakAmp( OutputSpectrum, (N + 2) );  
//prf( temp, "after NormalizeToPeaksOfSpectrumInBand peakAmp" ) ; 
//pri( normalizeToPeaksFlag, "normalizeToPeaksFlag" ) ; 


// WRITE RESPONSE TO file
for( i = 0; i < (N + 2); i++ )fwrite( &OutputSpectrum[ i ], sizeof(float), 1, ofd ) ;

fclose(ofd) ; 

// WRITE RESPONSE TO BINARY DECIBELS SPECTRUM FILE
if( strcmp( decibelsSpectrumPlotFile, "" ) != 0 ) 
{
   if( eqnormbypassflag == 0 )
   {
      writeSpectrumPlotFile( decibelsSpectrumPlotFile, OutputSpectrum, (N + 2), 1 ) ; 
   }else
   {
      writeSpectrumPlotFile( decibelsSpectrumPlotFile, OutputSpectrum, (N + 2), 0 ) ; 
   } ;
} ;

if( print_flag )tprintspec( OutputSpectrum, (N + 2), fundamental,  print_flag) ;


fprintf( stderr,  "\n\nNUMBER OF ANALYSIS OUTPUT AMP/FREQ PAIRS:    %d\n",  N/2 ) ; 
    
prt( "*********************************************" ) ;  
prs( ifile, "INPUT SOUND FILE" ) ;
prt( "DATA OUTPUT FILES:");  
if( strcmp( formantsOutputFile, "" ) != 0)
	prs( formantsOutputFile, "BINARY FILE" ) ; 
if( strcmp( formantsASCIIdataFile, "" ) != 0)
	prs( formantsASCIIdataFile, "ASCII FILE" ) ; 

prt( "***** PLOT FILES ****************************" ) ;  
//if( strcmp( freqStasisPlotFile, "" ) != 0)
//    prs( freqStasisPlotFile, "FREQUENCY STASIS PLOT FILE" ) ; 
if( strcmp( formantsBarPlotFile, "" ) != 0 )
    prs( formantsBarPlotFile, "FORMANT BAR PLOT FILE" ) ; 
if( strcmp( decibelsSpectrumPlotFile, "" ) != 0 ) 
    prs( decibelsSpectrumPlotFile, "AMPLITUDE SPECTRUM PLOT FILE (in dB)" ) ; 


fprintf(stderr,"\n\nFREQ RESPONSE : ANALYSIS COMPLETED\n\n");
exit(EXIT_SUCCESS) ;
}
void usage()
{
    fprintf(stderr, "%s",
	"freqresponse:  analysis-driven frequency response maker \n"
	"freqresponse   [flags] [input sound file] [output freqresponse file]\n"
	"	N:	"FFT_LENGTH 		// N
	"	M:	"WINDOW_SIZE 		// Nw

	"	w:	"WINDOW_TYPE 		// window_type 

	"	D:	"ANALYSIS_FRAMES_PER_SEC 	// frames_per_sec

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	C:	"RESYNTHESIS_CHANNEL		// channelout


	"		Output Spectrum Type:\n"
	"	c:	0 = average amplitude, 1 = peak amplitude\n"

	"	W:	average amplitude weighting: 0 = off,  1 = on [0] \n"
	"		    (For -c0, -c2, and -c3, the average is weighted by the frame's\n"
	"			sum,  thus causing amplitude to bias toward the louder frames.)\n"
	"	p:	Spectrum Formant Normalization Flag\n"
	"	 	   Normalize spectrum according to formant peaks; bins inbetween formants\n"
	"		   use cross-faded gain from respective bounding formants\n"
	"		   0 = off, 1 = on [0]\n"
	"	E:      Warp index for mid-formant amplitude compression/expansion.\n"
	"		   Values > 0: expand dynamic range between formants,\n"
	"		   values < 0 compress dynamic range.[0]\n"  


	"	L:   low frequency limit [0.]\n"
	"	j:   high frequency limit [nyquist]\n"
	"	A:	minimum formant peak amplitude in dB [-96.]\n"
	"	g:   formant selection threshold, 0-1 [.5]\n"
	"			Higher thresholds produce fewer formants by selecting\n"
	"			for stronger spikes.\n" 

	"	B:	EQ with normalization (0 = yes, 1 = no) [0] \n"
	"		 (This is an administrative flag for silent use with \n"
	"		   noisefilter and compander. All other uses of freqresponse)\n"
	"		   normalize the output response.)\n"
	"	     "SHELF_EQ_HEADER
	"	H:	"SHELF_EQ_LOW_GAIN		// dBlow
	"	X:	"SHELF_EQ_HIGH_GAIN		// dBhi
	"	m:	"SHELF_EQ_LOW_FREQ		// freqlow
	"	R:	"SHELF_EQ_HIGH_FREQ		// freqhi




	"	a:	Decibels Spectrum Plot File\n"
	"	o:	Formants Bar Plot File \n"
	"	i:	Formants ACII Data File \n"
	"	F:	Formants Data Output File (binary)\n"


	"	P:	"FREQUENCY_RESPONSE_PRINTOUT

	);
    exit(EXIT_SUCCESS);
}



void writeOutASCIIformantData(
    char formantsASCIIdataFile[],
    float formantCenterFreqs[], 
    float formantAmps[],
    float formantBWs[], 
    float formantQs[],
    int formantIndices[],
    int formantLowStopBandIndices[],
    int formantHighStopBandIndices[],
    int numFormants
)
{
    int i ; char tempString[ STRING_SIZE ], plotFlag ;  
    FILE *adata ;     

    plotFlag = strcmp( formantsASCIIdataFile, "" ) ; 

    if( plotFlag ) adata = fopen( formantsASCIIdataFile,  "w+") ;

    prt( "\n\n" ) ; 
    sprintf( tempString, "Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\n" ) ; 
    fprintf( stderr, "%s", tempString ) ; if( plotFlag ) fprintf( adata, "%s", tempString ) ;  
    for( i = 0; i < numFormants; i++){
        sprintf( tempString, 
            "%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\n",
            (i + 1), 
            (int) formantCenterFreqs[i], 
            (int) amp_to_dB( formantAmps[i] ),
            (int) formantBWs[i], 
            (int)formantQs[i],
            formantIndices[i],
	    formantLowStopBandIndices[i],
	    formantHighStopBandIndices[i]
        ) ; 
    fprintf( stderr, "%s", tempString ) ; if( plotFlag ) fprintf( adata, "%s", tempString ) ;  
    } ; 

    if( plotFlag ) fclose( adata ) ; 
}


float findPeakAmp(
    float A[],
    int N2
 
){
    float peakAmp ;
    int i ;  

   // FIND STRONGEST FREQ
    peakAmp = -999999.0 ;
    for( i = 0; i < (N2 + 1); i += 2) if( A[i] > peakAmp ) peakAmp = A[i] ; 

    return( peakAmp ) ; 

} ; 

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
