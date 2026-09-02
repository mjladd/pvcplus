#include "globals.h"
#include "stdbool.h"

#define dvec( name, size )\
if ( ( name = (double *) calloc( size, sizeof(double) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}


void usage(); 
void pd( int i ) ; 

float sumSincFunctions(
	double isrDouble,
	double dataTpt,
	double sincSumXscaler,
	double sincTruncationXinSeconds,
	long int numberOfFrames,
	double PIinterpolationPoints, 
	float audioData[],
	float halfSincLookupTable[],
	int useTableLookup
) ;

float holdSampleOrDecimate(
	float dataTpt,
	float audioData[],
	double isrDouble,
	long int numberOfFrames

) ; 

float linearlyInterpolate(
	double dataTpt,
	float audioData[],
	double isrDouble,
	long int numberOfFrames
) ;

void makeBoundaries(
	struct func *analysisDatawinlow,
	struct func *analysisDatawinhi,

	float *outputDuration,	
	float inputDuration,
	float t,
	float *lowBoundary,
	float *highBoundary,
	double *dataTpt

	
) ;

void setUpTimeManagementValues(
	struct func *analysisDatatOriginInSeconds, 
	struct func *analysisDatawinlow,
	struct func *analysisDatawinhi,
	float *oldDataTptOriginInSeconds, 
	double *realDataTpt,	
	double *dataTpt,	
	float *lowBoundary,
	float *highBoundary,
	float *windowTpt,
	float *t,
	int *stopFlag,
	long int *numberOfOutputSampleFrames,
	float outputDuration,
	float inputDuration,
	int *numberOfLayers
) ;

void advanceToNextDataTimePoint(

	struct func *analysisDatatOriginInSeconds,
	struct func *analysisDatarateForInput,
	struct func *analysisDatarateForOutput,
	struct func *analysisDatarateForInputInSemitones,
	struct func *analysisDatarateForOutputInSemitones,
	struct func *analysisDatawinlow, 
	struct func *analysisDatawinhi,
	float *lowBoundary, 
	float *highBoundary,
	float *windowTpt,
	double *realDataTpt,
	double *dataTpt,
	float *oldDataTptOriginInSeconds,
	int	*stopFlag, 

	double *sincSumXscaler,

	double isrDouble,
	float inputDuration, 
	int mode__rate_multiply_0__semitones_add_1,
	float *outputDuration, 
	float t, 
	long int numberOfOutputSampleFrames,
	int *numberOfLayers,
	bool printOn,
	int layerMode[],
	double layerDataTpt[],
	bool saveLayerData,
	bool notSearchingForDuration


) ;

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j, k, l,  m, n, nnn=0, i1, i2 ;
float frac ; 
int thisLayer ; 
int R=44100, in, on;
bool notSearchingForDuration=true ; 
int   eof = 0, channelout=0 ;
float tpt, x, v, z ; 
long int xBegin, xEnd ; 
int acceptableOutputDurationFlag=0 ;  
FILE *fopen() ;
char ch,  tempstring[ STRING_SIZE ],  
    scratch[ STRING_SIZE ],  scratch2[ STRING_SIZE ],  *user ;
int stopFlag ; 
bool printOn ; 
double isrDouble ;
int numberOfLayers=1 ;
float sincTruncationAmplitudeLevel ;
double sincTruncationXinSeconds ;
float  dur, prop, propFrac, rampValue ;
int iprop ;
float  temp, temp1,  temp2,  pm,  IR  ;  
int showme=0 ;
int numberOfLayersSave ; 
float max, min, average ; 
bool saveLayerData=false ; 

float *layerHoldTimeDur, *layerRiseTimeDur, *layerFallTimeDur, *layerTimeStartPoint,
	*layerTimeFallPoint ; 


int *layerMode ; // 0: RISE, 1: HOLD PEAK, 2: FALL, 3: OFF
int		numSynthChannels=1 ;
double *layerDataTpt ;
float *layerValue  ; 

float complianceRatio=0., oldComplianceRatio, oldTestOutputDuration, convergenceProp=1. ;

float tempSave ;

bool useInputRateForOutputRate=true ; 

long int lowIndex, highIndex ;
double realIndex, lowRealIndex, highRealIndex ; 
float fracIndex  ; 
int numberOfOutChannels=1, chan, outputChan, inputChan, numFrames ; 
float outputChanPeakAmpSum[2]={-99999999.,-999999999.}, thisOutputChanAmpSum[2],
	PeakAmp ; 
float outputDuration=0. ; 
double sincSumXscaler=1., xScaledForSinc ; 
bool synthesizeOutputDuration=false ; 
int synthesizeOutputDurationAttempts=0 ;
FILE *data ;
int RMScount ; 
float RMS, peakAmplitude=0. ;

float previousOuputDuration ; 


float lowBoundary, highBoundary ; 

float *audioData, sincTruncationDecibelLevel=-60.0, sum ; 

int halfSincFunctionLength ; 

double PIinterpolationPoints=100. ; 
double halfSincFunctionLengthAsDouble ; 

float *halfSincLookupTable ; 

float timeNow ; 

int synthesisMode=0 ; 

char outputFileName[ STRING_SIZE ]="" ; 

float inputDuration=0.0 ; 

int  frameNow ; 

int numsamps ; 
int numSampsBufferedIn, arg_index_Save, normalizeFlag=1 ; 
long int numFramesLeft ; 
int numFramesBufferedIn, blockFrame ; 

int numberOfInputSoundFiles=0, totalFramesBufferedIn, 
	numberOfInputChannels, 
		 thisInputFile,
	numberOfSampsBufferedIn, numberOfFramesBufferedIn, numberOfFramesToTransfer ; 

long int numberOfOutputSampleFrames, numberOfFrames ;

SF_INFO inputSFinfo ;  
SF_INFO outputSFinfo ; 

float duration, outputChannelPeakAmps[ MAXIMUM_CHANNELS ], inputChannelPeakAmps[ MAXIMUM_CHANNELS ], 
	peakInputChannelAmp=0., peakOutputChannelAmp=0., finalOutputChannelPeakAmps[ MAXIMUM_CHANNELS ],
	finalOutputChannelPeakAmp=0. ; 

float sincDecibelsTruncateLevel=-96.0 ; 

float interleavedInputBuffer [ BLOCKSIZE * MAXIMUM_CHANNELS ] ; 
float inputBufferByChannels [ MAXIMUM_CHANNELS ][ BLOCKSIZE ] ; 

bool useTableLookup=true ;
float tempBlock[ BLOCKSIZE ], *allChanInputBlock, *allChanOutputBlock ; 

int mode__rate_multiply_0__semitones_add_1=0 ; 


float oldDataTptOriginInSeconds,  windowTpt, oldWindowTpt ;

double dataTpt, realDataTpt ; 

// SOURCE GAIN
struct  func  amplitudeEnvelopeForOutput ; 
struct  func  amplitudeEnvelopeForInput ; 


// DATA TIME POINT ORIGIN
struct  func  analysisDatatOriginInSeconds ; 

//  DATA  RATES
struct  func  analysisDatarateForInput ; 
struct  func  analysisDatarateForOutput ; 

struct  func  analysisDatarateForInputInSemitones ; 
struct  func  analysisDatarateForOutputInSemitones ; 




// DATA TIME WINDOW LOWER BOUNDARY
struct  func  analysisDatawinlow ; 

// DATA TIME WINDOW UPPER BOUNDARY
struct  func  analysisDatawinhi ; 



// *****
//  ANALYSIS DATA   RATES
analysisDatarateForInput.L = 1. ; analysisDatarateForInput.n = 1. ; 
	analysisDatarateForInput.A[ 0 ] = 1. ; 
analysisDatarateForOutput.L = 1. ; analysisDatarateForOutput.n = 1. ; 
	analysisDatarateForOutput.A[ 0 ] = 1. ; 

analysisDatarateForInputInSemitones.L = 1. ; analysisDatarateForInputInSemitones.n = 1. ; 
	analysisDatarateForInputInSemitones.A[ 0 ] = 0. ; 
analysisDatarateForOutputInSemitones.L = 1. ; analysisDatarateForOutputInSemitones.n = 1. ; 
	analysisDatarateForOutputInSemitones.A[ 0 ] = 0. ; 


// ANALYSIS DATA  TIME ORIGIN
analysisDatatOriginInSeconds.L = 1. ; analysisDatatOriginInSeconds.n = 1. ; analysisDatatOriginInSeconds.A[ 0 ] = 0. ; 

// ANALYSIS DATA  TIME WINDOW LOWER BOUNDARY
analysisDatawinlow.L = 1. ; analysisDatawinlow.n = 1. ; analysisDatawinlow.A[ 0 ] = 0. ; 

// ANALYSIS DATA  TIME WINDOW UPPER BOUNDARY
analysisDatawinhi.L = 1. ; analysisDatawinhi.n = 1. ; analysisDatawinhi.A[ 0 ] = -1. ; 




// AMPLITUDE ENVELOPE 
amplitudeEnvelopeForInput.L = 1. ;  amplitudeEnvelopeForInput.n = 1. ; 
	amplitudeEnvelopeForInput.A[ 0 ] = 0. ; 
amplitudeEnvelopeForOutput.L = 1. ;  amplitudeEnvelopeForOutput.n = 1. ; 
	amplitudeEnvelopeForOutput.A[ 0 ] = 0. ; 






if( argc < 2 )usage() ; 


while( (ch = crack( argc, argv, "_|a|A|b|B|c|C|d|D|e|f|F|g|i|L|m|n|O|Q|r|R|s|S|t|T|w|X|Y|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) { 

    case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;


	case 'm':   osr = (int) crackfloat( arg_option, ch ) ;
				if( osr <= 0 ) useInputRateForOutputRate=true ; 
				else useInputRateForOutputRate=false ; 
			break;

	case 'X':   synthesisMode = (int) crackfloat( arg_option, ch ) ;
			break;


	case 't': 	if( (int) crackfloat( arg_option, ch ) == 1 ) useTableLookup = true ;
				else useTableLookup = false ; 
				break;


	case 'L':   PIinterpolationPoints = floor( crackfloat( arg_option, ch ) ) ;
			break;
		
	case 'B':   sincTruncationDecibelLevel = crackfloat( arg_option, ch ) ;
				if( sincTruncationDecibelLevel >= 0 ){
					prf( sincTruncationDecibelLevel, "\n\nERROR -------> SINC FUNCTION TRUNCATION LEVEL SET TO" ) ;
					prt( "\nMUST BE LESS THAN 0. \nRECOMMEND VALUES GREATER THAN -96." ) ; 
					prt( "VALUES BETWEEN -30 AND -60 WORK WELL.\n\n. . . BYE.\n" ) ;  
					exit( EXIT_FAILURE ) ; 					
				} ; 
			break;


	case 'd':   outputDuration = crackfloat( arg_option, ch ) ;
			break;
	case 'D':   if( (int) crackfloat( arg_option, ch ) == 1 )
					synthesizeOutputDuration = true ;
				else
					synthesizeOutputDuration = false ;

			break;

	case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;

	case 'n':   normalizeFlag = (int) crackfloat( arg_option, ch ) ;
			break;
		


	case 'a':   strcpy(tempstring, arg_option);
			amplitudeEnvelopeForInput.fp = crackstring( tempstring, & amplitudeEnvelopeForInput );
			break;
	case 'A':   strcpy(tempstring, arg_option);
			amplitudeEnvelopeForOutput.fp = crackstring( tempstring, & amplitudeEnvelopeForOutput );
			break;


	case 'b':   strcpy(tempstring, arg_option);
			analysisDatawinlow.fp = crackstring( tempstring, & analysisDatawinlow );
			break;
	case 'e':   strcpy(tempstring, arg_option);
			analysisDatawinhi.fp = crackstring( tempstring, & analysisDatawinhi );
			break;

	case 'O':   strcpy(tempstring, arg_option);
			analysisDatatOriginInSeconds.fp = crackstring( tempstring, 
			    &analysisDatatOriginInSeconds );
			break;

	case 'r':   strcpy(tempstring, arg_option);
			analysisDatarateForInput.fp = crackstring( tempstring, 
			    &analysisDatarateForInput );
			break;
	case 'R':   strcpy(tempstring, arg_option);
			analysisDatarateForOutput.fp = crackstring( tempstring, 
			    &analysisDatarateForOutput );
			break;

	case 's':   strcpy(tempstring, arg_option);
			analysisDatarateForInputInSemitones.fp = crackstring( tempstring, 
			    &analysisDatarateForInputInSemitones );
			break;
	case 'S':   strcpy(tempstring, arg_option);
			analysisDatarateForOutputInSemitones.fp = crackstring( tempstring, 
			    &analysisDatarateForOutputInSemitones );
			break;


	} 
}

if( useTableLookup )
	prt( "USING TABLE LOOKUP" ) ; 
else
	prt( "*NOT* USING TABLE LOOKUP" ) ; 




prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "RATECHANGER", 69 ) ; 
prline( 69,  "-" ) ; 



    // GET NAME OF USER
 user = getlogin(); 
  





// **** SET UPS *****

	PI = 4.*atan(1.) ;
	TWOPI = 8.* (float) atan(1.) ;

// ************

if( argc > 1   ){ 	// 

	// GET INPUT SOUNDFILE NAME
	strcpy( ifile, argv[arg_index] ) ; 


	// OPEN INPUT SOUND FILE IN READ MODE TO GET FORMAT. 

	if(! (infile = sf_open (ifile, SFM_READ, &inputSFinfo ))){
		// FILE DOES NOT EXIST
		prs( ifile, "Input Sound File" ) ; 
		prt( "------> NOT FOUND\n\n . . . . .  BYE.\n\n" ) ; 
		exit( EXIT_FAILURE ) ; 
	}else{
		isr = inputSFinfo.samplerate ; 
		isrDouble = (double) isr ; 
		numberOfInputChannels  = inputSFinfo.channels ;
		numberOfFrames = (long int) inputSFinfo.frames ;
		inputDuration  = (float)((double) numberOfFrames / isrDouble) ; // ???

		prbanner( "INPUT SOUND FILE PARAMETERS:", 69 ) ; 
		fprintf( stderr, "\n -- CHANNELS: %d  SAMPLE RATE: %d FRAMES: %d  DURATION: %f", 
				numberOfInputChannels, isr, (int) numberOfFrames, inputDuration ) ; 
		iformat = inputSFinfo.format ; 

	} ; 

}else{
	// NO INPUT SOUND FILE
	usage() ; 
     exit(EXIT_FAILURE) ;
}  ; 


if( outputDuration <= 0. ) synthesizeOutputDuration = true ; 

for(chan = 0; chan < numberOfInputChannels; chan++ ) inputChannelPeakAmps[chan] = 0. ; 



if( argc > 2   ){ 	// 

	arg_index++ ;

	// GET OUTPUT SOUNDFILE NAME
	strcpy( ofile, argv[arg_index] ) ; 
 	prs( ofile, "OUTPUT FILE" ) ;

}else{
	// NO OUTPUT SOUND FILE
	prt( "\n\nMISSING OUTPUT FILE\n" ) ; 
	usage() ; 
     exit(EXIT_FAILURE) ;
}  ; 


// MAKE SINC FUNCTION VALUES AND LOOKUP TABLE

sincTruncationAmplitudeLevel = dB_to_amp( sincTruncationDecibelLevel ) ; 
sincTruncationXinSeconds = (1.0 / (double) sincTruncationAmplitudeLevel) / isrDouble ;  

prbanner( "SINC FUNCTION PARAMETERS", 69 ) ; 

prf( sincTruncationDecibelLevel, "SINC FUNCTION TRUNCATION POINT DECIBEL LEVEL" ) ; 
fprintf( stderr, "\nSINC FUNCTION WINDOW LENGTH: %f seconds  %d samples", 
		2.0 * (float) sincTruncationXinSeconds, 2 * (int)(isrDouble * (float) sincTruncationXinSeconds) ) ; 


if( useTableLookup ){ 
	halfSincFunctionLengthAsDouble = 10. + (floor(isrDouble * sincTruncationXinSeconds) * 
		PIinterpolationPoints) ;
	halfSincFunctionLength = (int) halfSincFunctionLengthAsDouble ;

	prbanner( "TABLE PARAMETERS", 69 ) ; 
	prt( "USING TABLE . . . " ) ; 
	pri( halfSincFunctionLength, "LOOKUP TABLE LENGTH" ); 
	prf( PIinterpolationPoints, "PI TABLE INTERPOLATION LENGTH" ) ;  
	prt( "" ) ; 
	prline( 69,  "*" ) ; 

	fvec( halfSincLookupTable, halfSincFunctionLength ) ;  

	halfSincLookupTable[ 0 ] = 1. ; 

	for( i = 1, z = 0.5 * halfSincFunctionLengthAsDouble ; 
		i < halfSincFunctionLength; i++, z += 0.5 ){
		x = (float) i / PIinterpolationPoints ;
		halfSincLookupTable[ i ] = (float) sin( (double) (x * PI) ) / (x * PI) ;
			    // APPLY BLACKMAN WINDOW
		v =  (
			.42 - .5 * cos(( TWOPI / (halfSincFunctionLengthAsDouble - 1.)) * z) + 0.08 * 
				cos((2. * TWOPI / (halfSincFunctionLengthAsDouble - 1.)) * z )
		) ;
		halfSincLookupTable[ i ] *= v ;
		

	}; 
} ;

prline( 69,  "*" ) ; 

if(channelout == 0){ // ALL CHANNELS
	channelflag = 0 ; 
	beginchan = 0 ;
	endchan = numberOfInputChannels - 1 ;
	ochan = numberOfInputChannels ;  
} else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ;
	endchan =  beginchan ;
	ochan = 1 ; 
}

if( analysisDatawinlow.A[0] < 0.0 ) analysisDatawinlow.A[0] = 0.0 ; 
 if( analysisDatawinhi.A[0] <= 0.0 ) analysisDatawinhi.A[0] = inputDuration ; 

if( analysisDatawinlow.A[0] > inputDuration ){
	fprintf( stderr, 
	"\n\nERROR ----> LOWER BOUNDARY TIME OF %f EXCEEDS FILE DURATION OF %f.\n\nBYE\n",
		analysisDatawinlow.A[0], inputDuration ) ; 
		exit(EXIT_FAILURE) ; 
} ; 
if( analysisDatawinhi.A[0] > inputDuration ){
	fprintf( stderr, 
	"\n\nERROR ----> UPPER BOUNDARY TIME OF %f EXCEEDS FILE DURATION OF %f.\n\nBYE\n",
		analysisDatawinhi.A[0], inputDuration ) ; 
		exit(EXIT_FAILURE) ; 
} ; 

// SOURCE GAIN
prt(""); 
prt("** AMPLITUDE VALUES OR FUNCTIONS **" ) ; 
prp( &amplitudeEnvelopeForOutput, "DECIBEL VALUE OR ENVELOPE AS FUNCTION OF OUTPUT" ) ; 
prp( &amplitudeEnvelopeForInput, "DECIBEL VALUE OR ENVELOPE AS FUNCTION OF INPUT" ) ; 


//  DATA  RATES
prt(""); 

// DATA TIME POINT ORIGIN
prp( &analysisDatatOriginInSeconds, "TIME ORIGIN OR ORIGIN TRAJECTORY" ) ; 

prp( &analysisDatarateForInput, 
	"RATE MULTIPLIER OR MULTIPLIER TRAJECTORY AS FUNCTION OF INPUT SOUND TIME" ) ; 
prp( &analysisDatarateForOutput, 
	"RATE MULTIPLIER OR MULTIPLIER TRAJECTORY AS FUNCTION OF OUTPUT SOUND TIME" ) ; 
prp( &analysisDatarateForInputInSemitones, 
	"SEMITONE PITCH SHIFT OR SHIFT TRAJECTORY AS FUNCTION OF INPUT SOUND TIME" ) ; 
prp( &analysisDatarateForOutputInSemitones, 
	"SEMITONE PITCH SHIFT OR SHIFT TRAJECTORY AS FUNCTION OF OUTPUT SOUND TIME" ) ; 



// DATA TIME WINDOW LOWER BOUNDARY
prp( &analysisDatawinlow, "LOWER WINDOW BOUNDARY" ) ; 


// DATA TIME WINDOW UPPER BOUNDARY
prp( &analysisDatawinhi, "UPPER WINDOW BOUNDARY" ) ; 


if( synthesizeOutputDuration ) prt( "OUTPUT DURATION WILL BE SYNTHESIZED" ) ; 

if( useTableLookup ) prt( "USING LOOKUP TABLE WITH LINEAR INTERPOLATION" ) ; 

if( synthesizeOutputDuration ){

		// SYNTHESIZE OUTPUT DURATION
		prbanner( "OUTPUT DURATION SYNTHESIS", 69 ) ; 

		prt( "\nNOTE: An appropriate duration will be determined using a convergence" ); 
		prt( "algorithm that aligns boundary termination with the completion of rate" );
		prt( "changing functions.\n\n" ); 
		if( outputDuration == 0. ) outputDuration = inputDuration ; 
		previousOuputDuration = outputDuration ;
		oldComplianceRatio = 0. ; 


		while( acceptableOutputDurationFlag == 0 ){
	

			setUpTimeManagementValues(
				&analysisDatatOriginInSeconds, 
				&analysisDatawinlow,
				&analysisDatawinhi,
				&oldDataTptOriginInSeconds, 
				&realDataTpt,		
				&dataTpt,		
				&lowBoundary,
				&highBoundary,
				&windowTpt,
				&t,
				&stopFlag,
				&numberOfOutputSampleFrames,
				outputDuration,
				inputDuration,
				&numberOfLayers
			) ; 
 
			while( stopFlag == 0 ){

				numberOfOutputSampleFrames++ ; 
				t = (double) numberOfOutputSampleFrames / isrDouble ; // ???

				advanceToNextDataTimePoint(

					&analysisDatatOriginInSeconds,
					 &analysisDatarateForInput,
					&analysisDatarateForOutput,
					 &analysisDatarateForInputInSemitones,
					&analysisDatarateForOutputInSemitones,
					&analysisDatawinlow, 
					&analysisDatawinhi,
					&lowBoundary, 
					&highBoundary,
					&windowTpt,
					&realDataTpt,
					&dataTpt,
					&oldDataTptOriginInSeconds, 
					&stopFlag, 

					&sincSumXscaler,

					isrDouble,
					inputDuration, 
					mode__rate_multiply_0__semitones_add_1,
					&outputDuration, 
					t, 
					numberOfOutputSampleFrames,
					&numberOfLayers,
					false,

					layerMode,
					layerDataTpt,
					false,
					 false // notSearchingForDuration
				) ;
				

			} ; //	STOPFLAG
			
			complianceRatio = t / outputDuration ;
			fprintf( stderr, 
				"\n\nTest Output Duration was: %f seconds\nStopped at %f seconds\nCompliance Ratio: %f\n",
			outputDuration, t, complianceRatio ) ;
 
			if( (complianceRatio < 1.01) && (complianceRatio > (1./1.01))	){
				// ACCEPTABLE
				acceptableOutputDurationFlag = 1 ;
				prt(""); 
				prline( 69,  "*" ) ; 
				prf( outputDuration, "Output Duration in Seconds" ) ;   
				prline( 69,  "*" ) ; 
				prt(""); 
			}else{
				// NOT ACCEPTABLE
				if( (1. - complianceRatio) < (1. - oldComplianceRatio) ){
					// IMPROVED
					previousOuputDuration = outputDuration ;	
					outputDuration = outputDuration + 
						(convergenceProp * (t - outputDuration))  ;
				}else{
					// NOT IMPROVED: SPLIT PROP AND RETRY WITH OLD
					convergenceProp = convergenceProp * .75 ;
					outputDuration = previousOuputDuration + 
							(convergenceProp * (t - previousOuputDuration))  ;
				} ; 
				oldComplianceRatio = complianceRatio ;				
			} ; 

			synthesizeOutputDurationAttempts++ ; 
			if( synthesizeOutputDurationAttempts > 100 ){
				prt( "\n\n. . . TERMINATING DURATION SEARCH\n" ) ; 
				acceptableOutputDurationFlag = 1 ; 
			}  
		
		} ; // ACCEPTABLE


}else{

	prf( outputDuration, "Output Duration in Seconds" ) ;   


} ;

// NO LOOP -- ONE LAYER OUTPUT
numberOfLayers = 1 ; 
ivec( layerMode, numberOfLayers ) ;  ; // 0: RISE, 1: HOLD, 2: FALL, 3: OFF
dvec (layerDataTpt, numberOfLayers ) ;
fvec( layerValue, numberOfLayers ) ;
fvec( layerRiseTimeDur, numberOfLayers ) ; 
fvec( layerHoldTimeDur, numberOfLayers ) ; 
fvec( layerFallTimeDur, numberOfLayers ) ; 
fvec( layerTimeStartPoint, numberOfLayers ) ;  
fvec( layerTimeFallPoint, numberOfLayers ) ; 


layerMode[2] = 1 ;
layerTimeStartPoint[0] = 0. ; 
layerRiseTimeDur[0] = 0. ; 
layerHoldTimeDur[0] = outputDuration ; 
layerFallTimeDur[0] = 0. ;



// **
	
// ZERO PEAK OUTPUT AMPS
for(outputChan = 0; outputChan < ochan; outputChan++ ) outputChannelPeakAmps[outputChan] = 0. ; 

// MAKE MEMORY FOR ONE CHANNEL OF SOUND FILE. 
fvec( audioData, numberOfFrames ) ; 

prline( 69,  "*" ) ; 
if( synthesisMode == 0 ) prt( "SYNTHESIS METHOD: DIRECT CONVOLUTION WITH WINDOWED SINC FUNCTION" );  
if( synthesisMode == 1 ) prt( "SYNTHESIS METHOD: SAMPLE-AND-HOLD/DECIMATION" );  
if( synthesisMode == 2 ) prt( "SYNTHESIS METHOD: LINEAR INTERPOLATION" );  


	
for(chan = beginchan, outputChan = 0; chan <= endchan; chan++, outputChan++ ){

	prline( 69,  "*" ) ; 
	pri( chan + 1, "INPUT SOUND FILE CHANNEL" ) ; 
	pri( outputChan + 1, "OUTPUT SOUND FILE CHANNEL" ) ; 

	// FOR EACH CHANNEL, READ IN AUDIO FROM CHANNEL INTO MEMORY.
	sf_seek( infile, 0, SEEK_SET ) ;  // REWIND	


	totalFramesBufferedIn = 0 ; 
	peakAmplitude = 0. ; 
	RMScount = 0 ; RMS = 0. ; 

	while( (numberOfSampsBufferedIn = 
		sf_read_float (infile, interleavedInputBuffer, BLOCKSIZE * numberOfInputChannels ))
			&&
		( (numberOfFrames - totalFramesBufferedIn) > 0)
	){   

		numberOfFramesBufferedIn = numberOfSampsBufferedIn / numberOfInputChannels ; 
			
		// SET TRANSFER AMOUNT BASED ON MAX NEEDED; TRUNCATE THIS BUFFER IF NECESSARY
		if( (numberOfFrames - totalFramesBufferedIn) >= numberOfFramesBufferedIn )  
			numberOfFramesToTransfer = numberOfFramesBufferedIn ; 
		else
			numberOfFramesToTransfer = numberOfFrames - totalFramesBufferedIn ; 

		// TRANSFER INTERLEAVED CHANNEL TO MEMORY
		for( n = 0; n < numberOfFramesToTransfer; n++ ){
			audioData[ totalFramesBufferedIn + n ] = 
				interleavedInputBuffer[ chan + (n * numberOfInputChannels) ] ;

			if( fabs( audioData[ totalFramesBufferedIn + n ] ) > inputChannelPeakAmps[ chan ] )
				inputChannelPeakAmps[ chan ] = fabs( audioData[ totalFramesBufferedIn + n ] ) ;
		} ;

		totalFramesBufferedIn += numberOfFramesToTransfer ;

	} ; // END OF FILE SAMPS BUFFERED INTO audioData.
	

	// MAKE /tmp OUTPUT FILE
	sprintf( tempstring, "/tmp/%s.OutputChan.%d", user, outputChan ) ; // MAKE FILE NAME
		
	filesToRemove( tempstring, 0 ) ; 
	inputTempChanFiles[ outputChan ] = fopen( tempstring, "wb+" ); 


	// WRITE RATE CHANGED DATA TO /tmp FILE. 

	// INITIALIZE LAYER SWITCHES
	for( thisLayer = 0; thisLayer < numberOfLayers; thisLayer++ ){
		layerMode[ thisLayer ] = (thisLayer == 0) ? 1 : 0 ; 
	} ; 

	layerDataTpt[ 0 ] = analysisDatatOriginInSeconds.A[ 0 ] = 
		fval( &analysisDatatOriginInSeconds, outputDuration, 0. )  ; 




	setUpTimeManagementValues(
		&analysisDatatOriginInSeconds, 
		&analysisDatawinlow,
		&analysisDatawinhi,
		&oldDataTptOriginInSeconds, 
		&realDataTpt,		
		&dataTpt,		
		&lowBoundary,
		&highBoundary,
		&windowTpt,
		&t,
		&stopFlag,
		&numberOfOutputSampleFrames,
		outputDuration,
		inputDuration,
		&numberOfLayers
	) ; 

	k = 0 ; 


	prline( 69,  "*" ) ; 
	prt( "TIME:\t\tRMS AMPLITUDE (dB)\tPEAK AMPLITUDE (dB)" ) ; 
	prline( 69,  "*" ) ; 


	// RESAMPLE LOOP
	while( stopFlag == 0 ){

		sum = 0. ; 

		for( thisLayer = 0; thisLayer < numberOfLayers; thisLayer++ ){

			if( layerMode[ thisLayer ] != 3 ){
				// ACTIVE LAYER


				if( synthesisMode == 0 ){
					layerValue[ thisLayer ] = 
						sumSincFunctions( isrDouble, layerDataTpt[ thisLayer ], 
							sincSumXscaler, sincTruncationXinSeconds,
							numberOfFrames, PIinterpolationPoints, audioData, halfSincLookupTable,
							useTableLookup ) ;

				}else if( synthesisMode == 1 ){

					layerValue[ thisLayer ] = 
						holdSampleOrDecimate( layerDataTpt[ thisLayer ], audioData, isrDouble, numberOfFrames ) ; 

				}else if( synthesisMode == 2 ){
					layerValue[ thisLayer ] = 
						linearlyInterpolate( layerDataTpt[ thisLayer ], audioData, isrDouble, numberOfFrames ) ;
				} ; 

				// ADD RAMPS OR BYPASS
				rampValue = 1. ; 

				if( layerMode[ thisLayer ] == 0 ){ // RISE
					rampValue = 
						(t - layerTimeStartPoint[ thisLayer ]) / layerRiseTimeDur[ thisLayer ] ;

				}else if( layerMode[ thisLayer ] == 2 ){ // FALL
					rampValue = 1. - (
						(t - 
							(layerTimeStartPoint[ thisLayer ] + layerRiseTimeDur[ thisLayer ] + 
									layerHoldTimeDur[ thisLayer ] )
						) / layerFallTimeDur[ thisLayer ]) ;

				} ; 

				// ADD TO SUM OUT
				sum += ( rampValue * layerValue[ thisLayer ] ) ;
			} ; 
			

		} ; 


		oldWindowTpt = windowTpt ;


		// OVER OUTPUT
		amplitudeEnvelopeForInput.A[ 0 ] = 
				fval( & amplitudeEnvelopeForInput, inputDuration, dataTpt );
		amplitudeEnvelopeForOutput.A[ 0 ] = 
				fval( &amplitudeEnvelopeForOutput, outputDuration, t );

		sum *= dB_to_amp( amplitudeEnvelopeForInput.A[ 0 ] + amplitudeEnvelopeForOutput.A[ 0 ] ) ; 

		if( fabs(sum) > outputChannelPeakAmps[ outputChan ] ) outputChannelPeakAmps[ outputChan ] = fabs(sum) ; 
		tempBlock[ k ] = sum ; k++ ;
			
		if( k == BLOCKSIZE ){
			fwrite( &tempBlock, sizeof(float), k, inputTempChanFiles[ outputChan ] ) ;
			k = 0 ; 
		} ; 

		// ???
		numberOfOutputSampleFrames++ ; 
		t = (double) numberOfOutputSampleFrames / isrDouble ; 


		// UPDATE MODES
		for( thisLayer = 0; thisLayer <= numberOfLayers ; thisLayer++ ){

			if( t < layerTimeStartPoint[ thisLayer ] ) 
				layerMode[ thisLayer ] = 3 ;
			else if( t < (layerTimeStartPoint[ thisLayer ] + layerRiseTimeDur[ thisLayer ] ) )
				layerMode[ thisLayer ] = 0 ;
			else if( t < (layerTimeStartPoint[ thisLayer ] + layerRiseTimeDur[ thisLayer ] + 
					layerHoldTimeDur[ thisLayer ] ) )
				layerMode[ thisLayer ] = 1 ;
			else if( t < (layerTimeStartPoint[ thisLayer ] + layerRiseTimeDur[ thisLayer ] + 
				layerHoldTimeDur[ thisLayer ] + layerFallTimeDur[ thisLayer ]) )
				layerMode[ thisLayer ] = 2 ;
			else
				layerMode[ thisLayer ] = 3 ;

	} ; 


		RMS += (sum * sum) ; RMScount += 1 ; 
		if( fabs(sum) > peakAmplitude ) peakAmplitude = fabs(sum) ;

		if( RMScount >= (isr / 2) ){
			RMS = amp_to_dB( (float) sqrt( (double)(RMS / (float) RMScount) ) ) ; // ???

			fprintf( stderr,  "\n%6.1f          %7.4f                %7.3f", 
				t, RMS, amp_to_dB( peakAmplitude ) ) ; 

			RMScount = 0 ; RMS = 0. ; peakAmplitude = 0. ;  

		} ; 

		advanceToNextDataTimePoint(

			&analysisDatatOriginInSeconds,
			&analysisDatarateForInput,
			&analysisDatarateForOutput,
			&analysisDatarateForInputInSemitones,
			&analysisDatarateForOutputInSemitones,
			&analysisDatawinlow, 
			&analysisDatawinhi,
			&lowBoundary, 
			&highBoundary,
			&windowTpt,
			&realDataTpt,
			&dataTpt,
			&oldDataTptOriginInSeconds, 
			&stopFlag, 

			&sincSumXscaler,

			isrDouble,
			inputDuration, 
			mode__rate_multiply_0__semitones_add_1,
			&outputDuration, 
			t, 
			numberOfOutputSampleFrames,
			&numberOfLayers,
			true,
			layerMode,
			layerDataTpt,
			true,
			 true // notSearchingForDuration
		) ;


	} ; // END OF SAMPLES SYNTHESIS



	// WRITE OUT LAST
	if( k > 0 ){
		fwrite( &tempBlock, sizeof(float), k, inputTempChanFiles[ outputChan ] ) ;
		k = 0 ; 
	} ; 
	
	t = (double) numberOfOutputSampleFrames / isrDouble ;  // isr NOT CORRECT HERE. ???



} ; 

//	prt( "CLOSING INPUT SOUND FILE" ) ; 
sf_close( infile ) ;     
prt(""); 


for(chan = 0; chan < ochan; chan++)
	if( inputChannelPeakAmps[chan] >= 1.0 ) inputChannelPeakAmps[chan] = (32767.0 / 32768.0)  ;

// FIND PEAK AMP FOR ALL INPUT CHANNELS.

peakInputChannelAmp = 0. ; 
for(chan = 0; chan < ochan; chan++) 
	if( inputChannelPeakAmps[chan] > peakInputChannelAmp ) peakInputChannelAmp = inputChannelPeakAmps[chan] ; 

// PEAK AMPS IN DB
prbanner( "PEAK INPUT CHANNEL AMPLITUDES", 69 ) ; 

for(inputChan = beginchan; inputChan <= endchan; inputChan++){ 
	fprintf( stderr, "(%d) %f dB", inputChan, amp_to_dB( inputChannelPeakAmps[inputChan] ) ) ; 
	if( inputChannelPeakAmps[inputChan] == peakInputChannelAmp ) fprintf( stderr, " (peak)" ) ; 
		fprintf( stderr, "\n" ) ;
} ; 


// ********** OUTPUT

// ******* INTERLEAVE AND WRITE INTO OUTPUT FILE

// OPEN OUTPUT SOUND FILE IN READ MODE TO GET FORMAT. 
if (! (outfile = sf_open (ofile, SFM_READ, &outputSFinfo )))
{   
	fprintf (stderr, "\n\n -------> OUTPUT SOUND FILE %s NOT FOUND.", ofile ) ;
	fprintf( stderr, "\n -------> PRE-EXISTENT FORMATTED OUTPUT FILE REQUIRED.\n\n . . . . BYE." ) ; 
    	sf_close( outfile ) ;     
	exit(EXIT_FAILURE) ; 

}else{

	// PRE-EXISTING FILE FOUND: SET OUTPUT FORMAT AND RATE TO MATCH OUTPUT FILE.
	oformat = outputSFinfo.format ;

} ;

// CLOSE IT. 
sf_close( outfile ) ;     

prs( ofile, "\nOUTPUT FILE NAME" ) ; 
if( useInputRateForOutputRate ) {
	osr = isr ; 
	pri( osr, "OUTPUT WILL USE INPUT SAMPLE RATE OF" ) ; 
}else{
	pri( osr, "USING NEW (IMPOSED) SAMPLE RATE" ) ; 
} ; 


outputSFinfo.samplerate = osr ;  
outputSFinfo.channels = ochan ;
duration = (double) numberOfOutputSampleFrames / (double) osr ; // ???
fprintf( stderr, "\n -- CHANNELS: %d\n -- SAMPLE RATE: %d\n -- FRAMES: %d\n -- DURATION: %f", 
		outputSFinfo.channels, osr, (int) numberOfOutputSampleFrames, duration ) ; 


// OPEN/CREATE NEW OUTPUT FILE HEADER
if (! (outfile = sf_open (ofile, SFM_WRITE, &outputSFinfo )))
{   
	fprintf (stderr, "\n . . . UNABLE TO CREATE OUTPUT SOUND FILE %s .", ofile ) ;
    bannero() ;
    puts (sf_strerror (NULL)) ;
    exit(EXIT_FAILURE) ;
} ;

if( ! ( sf_format_check (&outputSFinfo) ) ){
		fprintf( stderr, "\nAFTER CREATE OF OUTPUT FILE: INVALID SOUND FILE FORMAT" ) ; 
} ; 

// POSITION AT BEGINNING AND TRUNCATE.
sf_count_t frames = 0 ; 
k = sf_command(outfile, SFC_FILE_TRUNCATE, &frames, sizeof (frames)) ; 

 
// ********** OUTPUT END

// REWIND TEMP FILES
for(outputChan = 0; outputChan < ochan; outputChan++ ){
	fseek( inputTempChanFiles[ outputChan ], 0, SEEK_SET ) ;
} ; 


// FIND PEAK AMP FOR ALL CHANNELS.
peakOutputChannelAmp = 0. ; 
for(outputChan = 0; outputChan < ochan; outputChan++) 
	if( outputChannelPeakAmps[outputChan] > peakOutputChannelAmp ) peakOutputChannelAmp = outputChannelPeakAmps[outputChan] ; 


// PEAK AMPS IN DB
prbanner( "PEAK OUTPUT CHANNEL AMPLITUDES", 69 ) ; 

for(outputChan = 0; outputChan < ochan; outputChan++){ 
	fprintf( stderr, "(%d) %f dB", outputChan + 1, amp_to_dB( outputChannelPeakAmps[outputChan] ) ) ; 
	if( outputChannelPeakAmps[outputChan] == peakOutputChannelAmp ) fprintf( stderr, " (peak)" ) ; 
		fprintf( stderr, "\n" ) ;
} ; 

if( (peakOutputChannelAmp > 1.) && (normalizeFlag == 1) ){
//	prt( "\n\n----> SIGNAL IS CLIPPING <----\n\n" ) ; 
//	prt( "WILL NORMALIZE TO INPUT LEVELS" ); 
} ; 


if( normalizeFlag != 0 ) prbanner( "NORMALIZATION", 69 ) ;
if( normalizeFlag == 1 )fprintf( stderr, "\nNORMALIZING CHANNELS TO INPUT LEVELS"); 
if( normalizeFlag == 2 )fprintf( stderr, "\nNORMALIZING EACH CHANNEL SEPARATELY"); 
if( normalizeFlag == 3 )fprintf( stderr, "\nNORMALIZING CHANNELS TOGETHER"); 
if( normalizeFlag == 4 )fprintf( stderr, "\nWILL NORMALIZE IF PEAK EXCEEDS 0 DECIBELS"); 


// BEGIN INTERLEAVE
// MAKE SPACE
fvec( allChanOutputBlock, BLOCKSIZE * ochan ) ; 


numFramesLeft = numberOfOutputSampleFrames ; // ???
frameNow = 0 ; 

prt( "\nINTERLEAVING /tmp FILES INTO OUTPUT FILE . . . \n" ) ; 

// ZERO PEAK OUTPUT AMPS
for(outputChan = 0; outputChan < ochan; outputChan++ ) finalOutputChannelPeakAmps[outputChan] = 0. ; 


while( numFramesLeft > 0 ){
	// FRAMES LOOP
	
	// ZERO OUTPUT ARRAY
	for(i = 0; i < BLOCKSIZE * ochan; i++) allChanOutputBlock[i] = 0. ; 

	for( outputChan = 0, inputChan = beginchan; outputChan < ochan; outputChan++, inputChan++ ){
			// POSITION IN FILE FOR READ
			fseek( inputTempChanFiles[ outputChan ], frameNow * sizeof(float), SEEK_SET ) ;  
			// READ IN A CHANNEL BLOCK
	    		numFramesBufferedIn = fread( &tempBlock, sizeof(float), BLOCKSIZE,  
					inputTempChanFiles[ outputChan ] ) ; 
			// NORMALIZE IF WANTED

			// NORMALIZE CHANNELS TO SEPARATE INPUT LEVELS
			if( (normalizeFlag == 1) && (outputChannelPeakAmps[ outputChan ] > 0.) )
				for( i = 0; i < numFramesBufferedIn ; i++ ) 
					tempBlock[i] = inputChannelPeakAmps[ inputChan ] *
						tempBlock[i] / ( outputChannelPeakAmps[ outputChan ] ) ;  

			// NORMALIZE CHANNELS SEPARATELY
			if( (normalizeFlag == 2) && (outputChannelPeakAmps[ outputChan ] > 0.) )
				for( i = 0; i < numFramesBufferedIn ; i++ ) 
						tempBlock[i] = (tempBlock[i] / (outputChannelPeakAmps[ outputChan ] )) * (32767.0 / 32768.0) ;  
			// NORMALIZE CHANNELS TOGETHER
			if( (normalizeFlag == 3) && (peakOutputChannelAmp > 0.) )
				for( i = 0; i < numFramesBufferedIn ; i++ ) 
							tempBlock[i] = (tempBlock[i] / (peakOutputChannelAmp )) * (32767.0 / 32768.0) ;  
			// NORMALIZE CHANNELS IF PEAK EXCEEDS 0 DECIBELS
			if( (normalizeFlag == 4) && (peakOutputChannelAmp > 1.) )
				for( i = 0; i < numFramesBufferedIn ; i++ ) 
						tempBlock[i] = ( tempBlock[i] / peakOutputChannelAmp ) * (32767.0 / 32768.0) ;  

			// INTERLEAVE CHANNEL BLOCK INTO OUTPUT ARRAY AND TEST FOR PEAK AMPS
			for(i = 0, k = outputChan; i < numFramesBufferedIn; i++, k += ochan ){ 
				allChanOutputBlock[k] = tempBlock[i] ; 
				if( fabs( tempBlock[i] ) > finalOutputChannelPeakAmps[outputChan] ) 
						finalOutputChannelPeakAmps[outputChan] = fabs( tempBlock[i] ) ;
			} ;

	};  



	// WRITE BUFFER OUT TO SOUNDFILE
	k = numFramesBufferedIn * ochan ; 
	sf_write_float ( outfile, allChanOutputBlock, k  ) ; 

	numFramesLeft -= (long int) numFramesBufferedIn ; frameNow += numFramesBufferedIn ; 

	} ; 


// END OF INTERLEAVE

// CLOSE SOUND FILE. 
sf_close( outfile ) ; 


// FINAL PEAK OUTPUT AMPS IN DB
prbanner( "FINAL PEAK OUTPUT AMPLITUDES", 69 ) ; 


for(outputChan = 0; outputChan < ochan; outputChan++){ 
	if( finalOutputChannelPeakAmps[ outputChan ] > finalOutputChannelPeakAmp ) 
		finalOutputChannelPeakAmp = finalOutputChannelPeakAmps[ outputChan ] ;
} ; 

for(outputChan = 0; outputChan < ochan; outputChan++){ 
	fprintf( stderr, "(%d) %f dB", outputChan + 1, amp_to_dB( finalOutputChannelPeakAmps[outputChan] ) ) ; 
	if( finalOutputChannelPeakAmps[outputChan] == finalOutputChannelPeakAmp ) fprintf( stderr, " (peak)" ) ; 
	fprintf( stderr, "\n" ) ;
} ; 


filesToRemove( NULL, 1 );

// ********

prt( "\nSUCCESS!\n\n" ) ; 

autoplay( autoplayreps, 0 ) ; 


exit(EXIT_SUCCESS) ;




}

void usage()
{
	fprintf(stderr, "%s",
	"ratechanger:    \n"
	"ratechanger   [flags] <input sound> <pre-existing output sound file>\n"
	"	    Most formats accepted.\n"
	"	    (Values in brackets denote defaults.)\n"

	"	C:	"RESYNTHESIS_CHANNEL		// channelout
	"	d:	output duration: \n"
	"	D: 	Synthesize duration: off (0), on (1) [0]\n"
	"			In order to synchronize the completion of control functions with the output duration,\n"
	"			this flag invokes a preliminary search for an output duration that synchronizes with\n"
	"			the completion of control functions. (An iterative algorithm is used that converges on an\n"
	"			appropriate duration.) \n"
	"\n		** Dynamic Control Via Input and Output Time **\n\n"
	"		The following flags allow for the dynamic control of rate and amplitude\n"
	"		as a function of either the current input or output sound time. The respective\n"
	"		controls may be used simultaneously, such as in a case in which a pitch\n"
	"		correction function is driven according to the input sound (correction needing\n"
	"		to be synchronized with the input sound) while a second function is used to apply\n"
	"		pitch changes as a function of the output time and a synthesized output duration.\n" 

	"\n		** Rate Change Control **\n"
	"	O:	time point origin (func) [0.]\n"
	"	r:	rate multiplier as a function of input sound time. (func) [1.]\n"
	"	R:	rate multiplier as a function of output sound time. (func) [1.]\n"
	"	s:	semitone pitch shift as a function of input sound time. (func) [0.]\n"
	"	S:	semitone pitch shift as a function of output sound time. (func) [0.]\n"
	"\n		** Amplitude Control **\n"
	"	a:	amplitude envelope as a function of input sound time. (func) [0.]\n"
	"	A:	amplitude envelope as a function of output sound time. (func) [0.]\n"

	"		** Sinc Function Filter Specifications **\n"
	"	B:	sinc function truncation decibel level (-30 to -60) [-60]\n"
	"			The specified decibel level determines the sinc window truncation point\n"
	"			using the amount of dB rolloff; lower levels increase fidelity. Range: -10 to -96\n"   
	"	t:	truncated and windowed (Blackman) sinc function using:\n"
	"				(0) math functions (slow), (1) lookup table with linear interpolation (fast) [1]\n"
	"	L:	Number of samples per Pi sine period for lookup table; value corresponds to the number of\n"
	"			computed table interpolation points between original samples. (e.g. 10-100) [100]\n"


	"		** Input Sound Time Boundaries **\n"
	"	b:	low time point boundary (func): [0.]\n"
	"	e:	high time point boundary (func) [end of file ]\n"
	"	X:	Synthesis modes: sinc summation (0), rectangular (1), linear interpolation (2) [0]\n"
	"			Modes 1 and 2, which produce digital artifacts--aliasing, etc.--are provided\n"
	"			here for demonstration purposes ONLY.\n"  
	"		** Post-Synthesis Processing **\n"
	"	m:	new sample rate: Impose the new sample rate on the resulting data.\n"
	"			0 or less keeps the old sample rate. [0]\n"
	"		Note: For sample rate conversion (i.e. resampling of input signal according to a new sample rate),\n"
	"		 specify, along with a new sample rate (-m), a rate change multiplier that resamples\n"
	"		 the signal in accord with the new and old sample rates (i.e. for either -r or -R, \n"
	"		specify a value equal to [old-sample-rate / new-sample-rate] ).\n" 
	"	n:	Normalization: \n"
	"			0: Do not normalize. \n"
	"			1: Normalize channels to input levels (default).\n"
	"			2: Normalize channels independently.\n"
	"			3: Normalize channels together against channel with peak amplitude.\n"
	"			4: Normalize channels together if any channel exceeds 0 decibels.[1]\n"

	"	_:	 "AUTO_PLAY		// autoplayreps


	);
	exit(EXIT_SUCCESS);
}


void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }


void advanceToNextDataTimePoint(

	struct func *analysisDatatOriginInSeconds,
	struct func *analysisDatarateForInput,
	struct func *analysisDatarateForOutput,
	struct func *analysisDatarateForInputInSemitones,
	struct func *analysisDatarateForOutputInSemitones,
	struct func *analysisDatawinlow, 
	struct func *analysisDatawinhi,
	float *lowBoundary, 
	float *highBoundary,
	float *windowTpt,
	double *realDataTpt,
	double *dataTpt,
	float *oldDataTptOriginInSeconds,
	int	*stopFlag, 
 
	double *sincSumXscaler,

	double isrDouble,
	float inputDuration, 
	int mode__rate_multiply_0__semitones_add_1,
	float *outputDuration, 
	float t, 
	long int numberOfOutputSampleFrames,
	int *numberOfLayers,
	bool printOn,
	int layerMode[],
	double layerDataTpt[],
	bool saveLayerData,
	bool notSearchingForDuration
){
	float prop ; 
	int iprop, k ;
	float propFrac ;
	float temp1, temp2, temp3 ; 
	float rateChangeIncrement, originChangeDifference ;
	static int messageSent=0 ;
	static double oldDataTpt ;
	static float scaledDataTimeLeft ;
	static double increment ;
	static bool releaseTriggered ;
	static bool outsideAboveFlag ; 
	static float amountOutside ;
	int thisLayer ;      
	 
	 
	if( numberOfOutputSampleFrames == 1 ) {
		messageSent = 0 ; 
		oldDataTpt = *dataTpt ; 
		releaseTriggered = false ; 
	} ; 

	// FIND RATE CHANGE CUMMULATIVE INCREMENT
	analysisDatarateForInput->A[ 0 ] = 
		fval( analysisDatarateForInput, inputDuration, *dataTpt );
	analysisDatarateForOutput->A[ 0 ] = 
		fval( analysisDatarateForOutput, *outputDuration, t );
	analysisDatarateForInputInSemitones->A[ 0 ] = 
		fval( analysisDatarateForInputInSemitones, inputDuration, *dataTpt );
	analysisDatarateForOutputInSemitones->A[ 0 ] = 
		fval( analysisDatarateForOutputInSemitones, *outputDuration, t );

	// MODE O: RATE MULTIPLIER
	rateChangeIncrement = (
		(	
			analysisDatarateForInput->A[ 0 ] * 
			analysisDatarateForOutput->A[ 0 ] *
			semitones_to_mult( 
				analysisDatarateForInputInSemitones->A[ 0 ] + 
				analysisDatarateForOutputInSemitones->A[ 0 ] 
			)
		) / isrDouble 
	) ;

	// FIND CHANGE TO THE DATA ORIGIN
	analysisDatatOriginInSeconds->A[ 0 ] = 
		fval(  analysisDatatOriginInSeconds, *outputDuration, t );
	originChangeDifference = 
		(analysisDatatOriginInSeconds->A[ 0 ] - *oldDataTptOriginInSeconds)  ; 
	*oldDataTptOriginInSeconds = analysisDatatOriginInSeconds->A[ 0 ] ;

	// INCREMENT ALL	
	increment = (double) ( ( (double)rateChangeIncrement + (double) originChangeDifference  )) ; // ???
	*realDataTpt +=  increment ; 
	*dataTpt += increment ;


	// INCREMENT LAYERS
	if( saveLayerData ) for( thisLayer = 0; thisLayer < *numberOfLayers ; thisLayer++ ){
		if( layerMode[ thisLayer] != 3 ) layerDataTpt[ thisLayer ] += increment ;
	} ; 

	*sincSumXscaler = 
		isrDouble * fabs((double)rateChangeIncrement + (double) originChangeDifference) ;// ???
	if( *sincSumXscaler < 1. ) *sincSumXscaler = 1. ; 

	makeBoundaries( 
		analysisDatawinlow,
		analysisDatawinhi,

		outputDuration,	
		inputDuration,
		t,
		lowBoundary,
		highBoundary,
		dataTpt
	) ; 


	if( (*dataTpt > *highBoundary) || (*dataTpt < *lowBoundary) ||
			((t >= *outputDuration ) && notSearchingForDuration ) ){
			 *stopFlag = 1 ; 
	} ; 

	*windowTpt =  *dataTpt - *lowBoundary ;


	oldDataTpt = *dataTpt ; 

}

void setUpTimeManagementValues(
	struct func *analysisDatatOriginInSeconds, 
	struct func *analysisDatawinlow,
	struct func *analysisDatawinhi,
	float *oldDataTptOriginInSeconds, 
	double *realDataTpt,	
	double *dataTpt,	
	float *lowBoundary,
	float *highBoundary,
	float *windowTpt,
	float *t,
	int *stopFlag,
	long int *numberOfOutputSampleFrames,
	float outputDuration,
	float inputDuration,
	int *numberOfLayers
){
	// SET UP TIME MANAGEMENT VALUES
	analysisDatatOriginInSeconds->A[ 0 ] = 
		fval( analysisDatatOriginInSeconds, outputDuration, 0.0 );
	*oldDataTptOriginInSeconds = analysisDatatOriginInSeconds->A[ 0 ] ;
	*realDataTpt = (double) *oldDataTptOriginInSeconds ; // ???
	*dataTpt = *realDataTpt ; 


	analysisDatawinlow->A[ 0 ] = fval( analysisDatawinlow, outputDuration, 0.0 );
	analysisDatawinhi->A[ 0 ] = fval( analysisDatawinhi, outputDuration, 0.0 );
	if( analysisDatawinhi->A[ 0 ] >= analysisDatawinlow->A[ 0 ] ){
			*lowBoundary = analysisDatawinlow->A[ 0 ] ; *highBoundary = analysisDatawinhi->A[ 0 ] ;
	}else{
		prt( "SWITCHING LOW AND HIGH BOUNDARIES" ) ; 
		*lowBoundary = analysisDatawinhi->A[ 0 ] ; *highBoundary = analysisDatawinlow->A[ 0 ] ;
	} ; 

	if( *lowBoundary < 0. ) *lowBoundary = 0. ; 
	if( *highBoundary > inputDuration ) *highBoundary = inputDuration ;
	


 	*t = 0. ; 

	*stopFlag = 0 ;
	*numberOfOutputSampleFrames = 0 ;
	*numberOfLayers = 1 ; 


} 

void makeBoundaries(
	struct func *analysisDatawinlow,
	struct func *analysisDatawinhi,

	float *outputDuration,	
	float inputDuration,
	float t,
	float *lowBoundary,
	float *highBoundary,
	double *dataTpt
	
){
	static float lowBoundarySave ;
	static float highBoundarySave ;

	lowBoundarySave = *lowBoundary ; 
	highBoundarySave = *highBoundary ;

	// FIND NEW WINDOW BOUNDARIES
	analysisDatawinlow->A[ 0 ] = fval( analysisDatawinlow, *outputDuration, t );
	analysisDatawinhi->A[ 0 ] = fval( analysisDatawinhi, *outputDuration, t );
		// FLIP THEM IF THEY HAVE CROSSED.
	if( analysisDatawinhi->A[ 0 ] >= analysisDatawinlow->A[ 0 ] ){
		*lowBoundary = analysisDatawinlow->A[ 0 ] ; *highBoundary = analysisDatawinhi->A[ 0 ] ;
	}else{
		*lowBoundary = analysisDatawinhi->A[ 0 ] ; *highBoundary = analysisDatawinlow->A[ 0 ] ;
	} ; 

		// SET LOW TO 0 IF BELOW!
	if( *lowBoundary < 0. ) *lowBoundary = 0. ; 
		// SET HIGH TO inputDuration IF ABOVE AND BEYOND!
	if( *highBoundary > inputDuration ) *highBoundary = inputDuration ;




} ;  

float sumSincFunctions(
	double isrDouble,
	double dataTpt,
	double sincSumXscaler,
	double sincTruncationXinSeconds,
	long int numberOfFrames,
	double PIinterpolationPoints, 
	float audioData[],
	float halfSincLookupTable[],
	int useTableLookup
){
	long int xBegin ;
	long int xEnd ; 
	float temp, v ; 
	int n, i1, i2 ; 
	float z, x, frac, sum, xScaledForSinc ;

			// SUM SINC FUNCTIONS 

			xBegin = (long int)((isrDouble * // ???
				(dataTpt - (sincSumXscaler * sincTruncationXinSeconds))) - 0.5) ;
			if( xBegin < 0 ) xBegin = 0 ; 
			xEnd = (long int)(( isrDouble * (dataTpt + // ???
				(sincSumXscaler * sincTruncationXinSeconds))) + 0.5) ;
			if( xEnd >= numberOfFrames ) xEnd = numberOfFrames - 1 ; 
			sum = 0.0 ; 
			temp = (float)(xEnd - xBegin + 1) ;

			for(n = xBegin, z = temp * 0.5 ; n <= xEnd; n++, z += 0.5 ){ 
				x = (float) fabs(  ((double) ((double) dataTpt * isrDouble) - (double) n) ) ; // ???
				xScaledForSinc = x / sincSumXscaler ; 
				if( useTableLookup ){
					temp = xScaledForSinc * PIinterpolationPoints ;
					i1 = (int) temp ;
					i2 = i1 + 1 ;
					frac = temp - (float) i1 ; 
					sum += (audioData[ n ] * 
						(halfSincLookupTable[ i1 ] + 
						(frac * (halfSincLookupTable[ i2 ] - halfSincLookupTable[ i1 ])))
						) ;
				}else{
					// APPLY SINC
					v = xScaledForSinc > 0. ? 
						(audioData[ n ] * (float) sin((double) (xScaledForSinc * PI) ) / 
							 (xScaledForSinc * PI) )
						: 0.  ; 
					// ADD BLACKMAN WINDOW // ???
					v *=  ( 
						.42 - .5 * 
							cos(( TWOPI / (double) (isrDouble * sincTruncationXinSeconds * 2.) ) * 
							((double) (x + (isrDouble * sincTruncationXinSeconds)) )) + 0.08 * 
							cos((2. * TWOPI / (double) (isrDouble * sincTruncationXinSeconds * 2.) ) * 
							((double) (x + (isrDouble * sincTruncationXinSeconds)) ))
						) ;
					sum += v ;
				} ; 
			} ; // END OF SUMMATION LOOP 


	return( sum ) ; 

} ; 


float holdSampleOrDecimate(
	float dataTpt,
	float audioData[],
	double isrDouble,
	long int numberOfFrames

){
	long int lowIndex ;
	float sum  ; 

	// RECTANGULAR
	lowIndex = (long int)(((dataTpt * isrDouble)) + 0.5) ; 
	if( lowIndex < 0 ) lowIndex = 0 ;
	if( lowIndex >= numberOfFrames ) lowIndex = numberOfFrames - 1 ;
	sum = audioData[ lowIndex ] ;
	
	return( sum ); 
} ; 


float linearlyInterpolate(
	double dataTpt,
	float audioData[],
	double isrDouble,
	long int numberOfFrames
){
	long int lowIndex, highIndex ;
	double realIndex, lowRealIndex, highRealIndex ; 
	float fracIndex, sum  ; 


	// LINEAR INTERPOLATION
	realIndex = dataTpt * isrDouble ;   
	lowIndex = (long int) realIndex ; 		
	if( lowIndex < 0 ) lowIndex = 0 ;
	if( lowIndex >= (numberOfFrames - 1) ) lowIndex = numberOfFrames - 2 ;
	highIndex = lowIndex + 1 ;
	fracIndex = (float) (realIndex - floor( realIndex )) ;  		
	sum = audioData[ lowIndex ] + (fracIndex * (audioData[ highIndex ] - audioData[ lowIndex ]) ) ;

	return( sum ); 
} ; 