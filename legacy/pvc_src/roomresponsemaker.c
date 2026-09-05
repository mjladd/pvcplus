#include "globals.h"
#include "stdbool.h"

#define dvec( name, size )\
if ( ( name = (double *) calloc( size, sizeof(double) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}


#define CHECK true

#define livec( name, size )\
if ( ( name = (long *) calloc( size, sizeof(long int) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}

void usage(); 
void pd( int i ) ; 

float source_minimum_distance_from_listener ; 

bool sourceIsBehindSpeakers ;
float sourceSpeakerOrientationSign ;

float maxSpeakerToSpeakerDistance=0. ;
float maxSpeakerToSpeakerDistanceDelayTime ;

bool sourceIsBetweenTwoSpeakers, speakersStraddle ;
float *directSoundSpeakerAmplitudeScalars ; 
float *sourceToSpeakerDistanceForDelays ; 

bool thresholdProximityScalarSwitch=true ; 
int use_collapsed_threshold_amplitudes__no_0__yes_1=0 ; 

int crossFadeSpeaker0, crossFadeSpeaker1 ;


float maxSpeakerToListenerDistance=0. ; 
float maxSpeakerToListenerDistanceDelayTime ; 
float maxSpeakerToForwardSourceDistanceLimit ;

float preEchoDistance ;
float preEchoTime ;

float frontSourceHeadRoomScalar ;


float thresholdAndListenerToSourceIntersection[ 2 ] ;

int numberOfReflections ;
int oldNumberOfReflections ;

int 	numberOfROimpulseResponsePresenceLevels ; 
bool 	useROIRpresenceLevelsFlag=false ; 
float	*ROimpulseResponsePresenceLevels ; 

float	*PCimpulseResponse1, *PCimpulseResponse2 ;
float 	*reflectionOrderTempSpace ;
int 	*reflectionOrderTempSpaceBeginIndex, *reflectionOrderTempSpaceSize ;
int  	*reflectionOrderTempSpaceOrderSizes ;
float 	*tempPC ;


int	listener_space_cross_reflections__include_0__exclude_1=1 ;


int *reflectionOrderCVOrderSequences ; 
int *reflectionOrderCVIRSequences ;
int *reflectionOrderCVOrderSequenceSizes ;
int reflectionOrderCVOrderSequencesMaxSizeLength ;
float *tempSpace ;
int tempSpaceSizeNow ;

int	RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3=0 ;
float	RO_IR_BPF_low_rolloff_frequency=100. ;
float	RO_IR_BPF_high_rolloff_frequency=20000. ;
float	RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave=0. ;
float	RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave=0. ;

bool	convolveTwoArraysReset=false ; 

int	reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2=0 ; 

struct  func  wall_IR_response_envelope ; 

struct  func  reflection_order_IR_envelope ; 

float	wall_IR_truncate_duration=0. ;


float	reflection_order_IR_truncate_duration=0.  ;

float	reflected_sound_gain_in_decibels=0. ;
float	direct_sound_gain_in_decibels=0. ;




int numberOfConcavePoints ; 
bool polygonIsConcave=false ;

float	*endCropTempArray ;  
int	croppedIR_DataLength ; 

float	*orderAverageReflectionTimes ;

float	*sampleOrderAttackDurations ; 
float	*sampleOrderReleaseDurations ; 

int	scale_to_pre_envelope_window_sample_peak_amps__off_0__on_1=0 ; 


int	numberOfReflectionOrderDecibelGainscaleLevels ;

float	*reflectionOrderDecibelGainscaleLevels ;

float	low_order_attack_duration=.1 ;
float	low_order_release_duration=.1 ;
float	high_order_attack_duration=0.1 ;
float	high_order_release_duration=.1 ;
float	envelope_shape_index=0 ;

float 	*impulseResponse0, *impulseResponse1 ; 


float	*reflectionOrderImpulseResponseNow ; 
int	reflectionOrderImpulseResponseNowSize ; 
int	reflectionOrderImpulseResponseNowBaseIndex ;
int	reflectionOrderImpulseResponseNowSize ;

char	crackTempString[ STRING_SIZE ]="EMPTY\0" ; 

int	reflection_order_IR_convolution_mode=0 ; 

char	reflection_order_dB_gainscale_factors_file[ STRING_SIZE ]="EMPTY\0" ;

char	wall_impulse_response_decibels_presence_file[ STRING_SIZE ]="EMPTY\0" ;

char	reflection_order_impulse_response_decibels_presence_file[ STRING_SIZE ]="EMPTY\0" ;





bool 	testFlag ; 
int	convolutionCount=0 ;

bool	noCropImpulseResponsesFlag=true ; 

float	endCropDecibelThreshold=-96. ;
float	endCropReleaseTime=0.1 ; 

float	impulse_inclusion_threshold_in_dB=-96 ; 

//float	*cvBuffer ;

float 	*audioArrayForFilter ;
int 	lengthOfAudioArrayForFilter ;
int	lengthOfAudioArrayForFilterMemorySize ; 

int	*sequenceOfWallIResponses ;
int	*sequenceOfWallIResponsesNow ; 
int	reflectionOrder ;
int	*originalWallReflectionSequence_firstToLast ;


bool	debugFlagRoomResponse=false ; 

float	*tempFilt ; 
int	tempFiltsize ; 
float	*filteredWallImpulseResponseTempSpace ; 

float	*convolutionOutput ;  
int 	convolutionOutputSize ;
int	previousConvolutionOutputMemorySize=-1 ;
int	convolutionOutputMemorySize ;


float	*convolutionOutputPC ;  
int 	convolutionOutputPCSize ;
int	previousconvolutionOutputPCMemorySize=-1 ;
int	convolutionOutputPCMemorySize ;


int	impulseResponseNowMemorySize ;
int	previousImpulseResponseNowMemorySize=-1 ; 

float	*cvBuffer, *cvInBuffer, *cvOutBuffer ;
float   *BthisB ;

float	*cvBufferPC, *cvInBufferPC, *cvOutBufferPC ;
float   *BthisBPC ;








float	*filterTempArray ;

int	highestIndexWritten ;

int	wall_isr ;
double	wall_isrDouble ;
int	wall_numberOfInputChannels ;
long int wall_numberOfFrames=0 ;
float	wall_inputDuration ;
int	wall_iformat ; 

float	*impulseResponseNow ;

float	*wallImpulseResponses ; 
float	*wallImpulseResponsesInterleaved ;
int	*wallImpulseResponsesCroppedSize ; 
SF_INFO inputWallImpulseResponseSFinfo ;

int	wall_impulse_and_gainscale_response_mode=-1 ; // -1 = last wall, 1 = first wall, -2 = last 2, 2 = first 2. etc.  

SNDFILE	*inputWallImpulseResponseFilePointer ;

bool	 reflectionOrderImpulseResponsesFlag=false ;
int	numberOfReflectionOrderChannelAssignments=0 ;
int	reflection_order_numberOfInputChannels ;
int	*reflectionOrderChannelAssignments ;
float	*reflectionOrderImpulseResponses ;
float	*reflectionOrderImpulseResponsesInterleaved ;
SNDFILE	*inputReflectionOrderImpulseResponseFilePointer ; 
SF_INFO	inputReflectionOrderImpulseResponseSFinfo ;


int	numberOfWallImpulseResponsePresenceLevels=0 ;
float	*wallImpulseResponsePresenceLevels ; 

int	reflection_order_isr ;
double	reflection_order_isrDouble ;
long	reflection_order_numberOfFrames ;
float	reflection_order_inputDuration ;
int	reflection_order_iformat ;





float 	crossFadeProportionForClosest=0. ;
float 	crossFadeProportionForNextClosest ; 
float 	*originToSpeakerAngles ;
float 	*originToSpeakerDistances ;


float 	*coordinatesOfCorners ;
float 	*originToCornerAngles ;
float 	*originToCornerDistances ;
float 	*angles ;
float	*randomProportions ;

int	*polygonReflexVertexAngleFlags ; // -1 = reflex, 1 non-reflex, 0 = 180 degrees


float WIRBPF_low_rolloff_frequency=0. ;
float WIRBPF_high_rolloff_frequency=0. ;
float WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave=0. ;
float WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave=0. ;
int wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3=0 ; 
float sampleRate, nyquist ;


int	*reflectionDataSetChannelPointer ; 
float 	*reflectionDistances ;
float 	*reflectionTimes ;
int 	*reflectionOrders ;
float	*sourceOrientationToReflectionAngleDifference ;
int	*reflectionWalls_lastToFirst ;
float	*reflectionSoundPathCoordinates ; 

float 	sourceToFirstMirrorSegmentIntersection[ 4 ] ;
float 	sourceToFirstMirrorSegmentIntersectionAngle ;
float 	angleDiff ;

float listenerToSpeakerAngleMaximum ; 
float listenerToSpeakerAngleMinimum ;

float   listenerToSpeakerAngleMinimum ;
float   listenerToSpeakerAngleMaximum ;
int   	listenerToSpeakerAngleMinimumIndex ;
int   	listenerToSpeakerAngleMaximumIndex ;


float 	sourceToThresholdProximityDistance ; 
float	sourceToThresholdProximityDistanceProportion ;
float	sourceToThresholdProximityVirtualRange=10.0 ;
float	sourceToThresholdProximityRealRange=10.0 ;


float 	*sourceToSpeakerDistances ; 
float	*sourceToSpeakerAngles ;
float	*sourceToSpeakerAngleDifferences ; 
float 	maxSourceToSpeakerDistance ; 
float 	minSourceToSpeakerDistance ; 

int 	thisSpeaker ; 

int	reflectionNumberLimit=100000 ;
int	reflectionMemoryExpansionSize=100000 ; 
float	longestReflectionDistance ;
float 	longestDelayTime ; 
int 	longestDelayTimeIndex ; 
int	maximumNumberOfReflections ; 

int	*fileIRCollectionCodes ; // 
int	*fileIRCollectionCodeLengths ;
long	*fileIRCollectionAddresses ;
float	*fileIRCollectionData ;
long	fileIRCollectionDataMemorySizeNow=10000000 ; 

long	memoryPositionNow=0 ; 
long	*fileIRCollectionLengths ;
int	numberOfSavedIResponses=0 ;

int	*reorderedWallIRSequences_firstToLast ;
int	numberOfSelectedWallReflectionSequences ;

int 	indexOfClosestSpeaker = -1 ;
int 	indexOfNextClosestSpeaker ; 
int 	distanceToClosestSpeaker = -1 ;
int 	distanceToNextClosestSpeaker ; 

float	thisDistance ; 
int 	speaker ; 

int 	highOrderLimit=0, lowOrderLimit=0 ; 
int 	numberOfWalls=4 ;
int	numberOfListenerPositions ; 
int	numberOfSourcePositions ;
int	numberOfSpeakerPositions ; 
float 	rotationOfSyntheticRoomInDegrees=0. ;
char 	soundFileName[ STRING_SIZE ]="" ;
char	plotFileName[ STRING_SIZE ]="" ;
int 	numberOfCorners ;
float 	minMaxDistanceToCornerFromOrigin[ 2 ]={ 30., 30. } ; 

char 	soundPathsPlotFileName[ STRING_SIZE ]="EMPTY\0" ;
FILE	*plotFilePointer ;
FILE	*channelOrderPlotFilePointers[ 100 ] ;
bool	printToPlotFile=false ; 
int	plot_mode__one_file_0__channel_files_1__channel_and_order_files_2=0 ; 

int	reflection_order_impulse_responses__off_0__on_1=0 ; 
char	reflectionOrderImpulseResponseInputSoundFileName[ STRING_SIZE ]="EMPTY\0" ;
char	reflection_order_channel_assignments_file[ STRING_SIZE ]="EMPTY\0" ;


int	wall_impulse_responses__off_0__on_1=0 ; 
bool	wallImpulseResponsesFromSoundFileFlag=false ; 
char	wallImpulseResponseInputSoundFileName[ STRING_SIZE ]="EMPTY\0" ;

// bool    convolveWallImpulseResponses = false ; 
bool	wallPulseModeFlag = true ; 
char	wall_channel_assignments_file[ STRING_SIZE ]="EMPTY\0" ;
int 	*wallChannelAssignments ; 
int	numberOfWallChannelAssignments ; 
char	wall_dB_gainscale_factors_file[ STRING_SIZE ]="EMPTY\0" ;
float	*wallDecibelGainscaleLevels ;
int	numberOfWallDecibelGainscaleLevels ;

float	sourceToSpeakerSegment[ 4 ] ;
float	listenerToSourceSegment[ 4 ] ;
float	sourceToListenerSegment[ 4 ] ;
float	listenerToSourceLength ; 
float 	listenerToSourceAngle ;
float	sourceToListenerAngle ;
float 	rotatedSource ;
float	sourceToListenerAnglePlusRotation ; 
float   sourceToSpeakerOrThresholdAngle ;
float	*listenerToSpeakerAngles ;
int	*listenerToSpeakerAngleStraddleFlags ;

float polygonAngleRegularityProportion=1.0 ; 

int speaker_configuration__sequence_0__polygon_1=0 ; 

float *mirroredPolygonCoordinates ;
float *mirrorSegments ;
float *mirrorSegmentIntersectCoordinates ;
int *mirrorSideNumber ; 

float *coordinatesForThisSoundPath ;

int numberOfPolygonsWrittenToPlotFile=0 ; 

float speedOfSoundInFeetPerSecond=1280. ; 


float originCoordinates[ 2 ]={ 0., 0. } ;
float *listenerCoordinates ; // [ 2 ]={ 0., 0. } ;
float *sourceCoordinates ; // ={ 0., 0. } ;
float *speakerCoordinates ; //[ 2 ]={ 0., 0. } ;

float *speakerSegmentDistances ; 

float speakerCoordinatesNow[ 2 ] ;

float mirrorSegCoordinatesAnglesTemp[ 2 ] ;
//float mirrorSegAngleLimitsTemp[ 2 ]={ 0., 0. } ; 

int *viableReflectionCount ;
int *totalExaminedReflections ;
int *viableReflectionCountByOrder ; // =Array.fill( highOrderLimit, {arg i ; 0 }) ; 

int compoundLevels ; 

FILE *filePointer ;
bool writeToFile=false ; 

float *polygonCoordinates ;
float *sourceCoordinatesForThisPolygon ;

double point[2]= { -63.639610, 21.213211 } ;
double mirrorPoint[2] ;
//float lineSeg[4] = { -1., -1., -1., 6. } ;
double lineSeg[4] = { -63.639610, 21.213211, -21.213203, 21.213203 } ;

float sourceToSpeakerDistanceForAmp ; 
float sourceToSpeakerDistanceForDelay ;
float crossFadeAmpModifier ;  
float sourceToSpeakerTime ; 

int IR_DataLength ;
int previousIR_DataLength =-1 ;

int output_sound__all_0__direct_1__reflections_2=0 ;
int orient_source__to_room_0__to_listener_1=0 ;
bool reflectionsFlag = true ; 
bool directSoundFlag = true ; 

float airAbsorptionExponentForReflections=2.0 ; 
float airAbsorptionExponentForVirtualSpaceSource=2.0 ; 
float airAbsorptionExponentForRealSpaceSource=2.0 ; 
float airAbsorptionExponentAtThreshold=20.0 ; 


float reflections_time_scaler=1.0 ; 

float room_X_translation_factor=0. ;
float room_Y_translation_factor=0. ;
float room_negXscaleFactor=1. ;
float room_posXscaleFactor=1. ;
float room_negYscaleFactor=1. ;
float room_posYscaleFactor=1. ;
float room_RotationInDegrees=0. ; 
float room_ScaleFactor=1.0 ; 

float speaker_X_translation_factor=0. ;
float speaker_Y_translation_factor=0. ;
float speaker_negXscaleFactor=1. ;
float speaker_posXscaleFactor=1. ;
float speaker_negYscaleFactor=1. ;
float speaker_posYscaleFactor=1. ;
float speaker_RotationInDegrees=0. ; 
float speaker_ScaleFactor=1.0 ; 


FILE *data ; 

char datafile2[ STRING_SIZE ] = "EMPTY\0" ; 
char polygonCoordinates_datafile[ STRING_SIZE ] = "EMPTY\0" ; 
char listenerCoordinatesDataFile[ STRING_SIZE ] = "EMPTY\0" ; 
char sourceCoordinatesDataFile[ STRING_SIZE ] = "EMPTY\0" ; 
char speakerCoordinatesDataFile[ STRING_SIZE ] = "EMPTY\0" ; 

char new_datafile[ STRING_SIZE ] = "EMPTY\0" ; 

float *IR_Data ;


float minimumReferenceDistanceInFeet=1.0 ;
float sourceRotationInDegrees=0. ; 
float	sourceRotation ; 
float sourceDispersionPatternRolloffInDecibels=0 ; 



float 	piToDegrees( 
	float v 
) ;
float	degreesToPi( 
	float v 
) ; 

void makeCorneredSpace() ;

void printPolygonCoordinatesPlus(
   char label[],
   float coordinates[],
   float originToPointAngles[],
   float originToPointDistances[],
   int number
) ;

void printPolygonCoordinates(
   char label[],
   float coordinates[],
   int number
) ;


void mirrorPointAroundLineSegment( 
	double point[2], 
	double line[4],
	double mirrorPoint[2]
) ;

bool examineSegmentsForIntersection(
	float w[], 
	float p[],
	float intersectCoordinates[],
         bool printFlag
) ;


bool pointInPolygonTest(
   float polygonCoordinates[],
   float coordinates[],
   int numberOfPoints,
   char string[]
) ;

bool polygonTest(
   float polygon[],
   int numberOfCorners,
   char string[]
) ;

void mirrorPolygonCoordinatesAroundAllSides(
   int outputFileChannelNumber,
   int thisOrder,  // SET TO 1 WITH FIRST LEVEL CALL
   int polygonMirrorSideIndex, // SET TO -1
   float mirrorSegAngleLimitsTempLow,
   float mirrorSegAngleLimitsTempHigh
) ;

void getPolygonCoordinates(
   int polygonCoordinatesSource
) ;

void getListenerCoordinates() ;

void getSourceCoordinates() ;

void getSpeakerCoordinates() ;

void makeRoomReflectionsPlotFiles(
   int outputChannel
) ;

float findSegmentLength(
   float segment[]
) ;

float findSegmentAngle(
   float segment[]
) ;

void makeListenerToSpeakerAngles() ;


bool orderOfAnglesTest( 
   float angles[], 
   int numberOfAngles
) ;

void findMinMaxValues(
   float array[],
   int numberOfValues,
   float *minVal,
   int *minValIndex,
   float *maxVal,
   int *maxValIndex
) ;

void scaleAndRotateCoordinates(
   char label[],
   float coordinates[],
   int   numberOfPoints,
   float originToPointAngles[],
   float originToPointDistances[],
   float X_translation_factor,
   float Y_translation_factor,
   float negXscaleFactor,
   float posXscaleFactor,
   float negYscaleFactor,
   float posYscaleFactor,
   float rotateInDegrees,
   float scaleFactor
) ;

void makeSpeakerSeriesSegmentDistances() ;


int writeReflectionPulsesIntoImpulseResponse(
   int outputFileChannelNumber,
   int numberOfOutputChannels
) ; 

void writeDirectSourcePulsesIntoImpulseResponseOLD(
   int channel
) ;

void writeDirectSourcePulsesIntoImpulseResponse(
   int channel
) ;

void makeSegment(
   float segment[],
   float point0[],
   float point1[]
) ;

void rotatePointToAngle(
   float pointToRotate[], // point to rotate
   float originPoint[], // origin point
   float newAnglePosition, // new angle 
   float newPointPosition[] // new point   
) ;

bool valueIsBetweenTheseTwo(
   float v,
   float b0,
   float b1
) ;


void printSegment(
   float s[],
   char text[]
) ;

void printCoordinates(
   float s[],
   char text[]
) ;

bool sameCoordinatesTest(
   float coordinates0[],
   float coordinates1[]
) ;

bool adjacentSegmentsTest(
   float seg0[],
   float seg1[]
) ;

void makeSourceToSpeakerDistancesAndAngles() ;


void findListenerToSourceSegmentLengthAndAngle() ;


void makeSourceToThresholdProximityDistance() ;

void makeDirectSoundSpeakerAmplitudes() ; 


void makeDirectSoundSpeakerDelayTimes() ; 


void isSourceBehindOrInFrontOfSpeakerThreshold() ;


bool findIntersectionOfLinesContainingSegments(
   float w[],
   float p[],
   float intersectCoordinates[]
) ;

void readInWallImpulseResponses() ;

void convolveTwoArrays(
   float array0[],
   int Lh0,
   float array1[],
   int Lh1
) ;

int cropEndForSilence(
   float array[],
   int *size, 
   float threshold,
   bool shortenMemory
) ;

float findPeakAmp(
   float array[],
   int size
) ;

void filterFFT(
   float fftArray[],
   int N,
   float fundamental, 
   float lowFreq,
   float highFreq,
   float lowRolloffInDBperOctave,
   float highRolloffInDBperOctave,
   int compoundLevels
) ;

void filterAudioArray(
   float sampleRate,
   float lowFreq,
   float highFreq,
   float lowRolloffInDBperOctave,
   float highRolloffInDBperOctave,
   int compoundLevels
) ;

void findWallImpulseResponsesCroppedSize() ;

void smoothReleaseOfCroppedEnd(
   float array[],
   int size, 
   float releaseTime
) ;

void getWallImpulseResponseChannelAssignments() ;

void getWallDecibelGainscaleLevels() ;


void makeSpaceReflectionCoordinates(
   int order
) ;

bool couldThisPointBeInThisSegment(
   float point[],
   float segment[],
   bool includeEnds
) ;

void prb( 
   bool truth,
   char label[ STRING_SIZE ]
) ;

bool isPolygonConcave( 
   float polygon[],
   int numberOfVertices
) ;

int pointToLinePosition(
   float A[],
   float B[],
   float P[]
) ;

void createAndReorderWallReflectionSequence(
  int outputFileChannelNumber 
) ;

void sortSequence(
   int seq[],
   int length
) ;

int isFirstSequenceGreaterLesserOrEqualToSecond(
   int seq0[],
   int length0,
   int seq1[],
   int length1,
   int length
) ;

void addToIRfileCodes(
   int sequenceOfWallIResponses[],
   int numberOfWallsInSequence
) ;

bool testSequence( 
   int sequence[],
   int numberOfWallsInSequence,
   int *IRindex 
) ;

void recallIR( 
   int IRindex 
) ;

void filterAndNormalizeImpulseResponseNow(
   int parameterSet__wall_0__reflection_order_1, 
   float rolloffScaler
) ;

void filterAndNormalizeWallImpulseResponses() ; 

void truncateEnvelopeAndNormalizeWallImpulseResponses() ;

void readInReflectionOrderImpulseResponses() ;

void getReflectionOrderImpulseResponseChannelAssignments() ;

bool makeReflectionOrderImpulseResponseNow(
   int outputFileChannelNumber,
   int reflectionNumber,
   int wallImpulseResponseNumber,
   int *reflectionOrderImpulseResponseNowBaseIndex,
   int *reflectionOrderImpulseResponseNowSize,
   float *windowedSamplePeakAmp
) ;

void findAverageReflectionOrderDelayTimes(
   int outputFileChannelNumber
) ;

int cropIR_DataEndForSilence(
   float threshold,
   bool shortenMemory
) ;

void getReflectionOrderDecibelGainscaleLevels() ;

void findreflectionOrderCVOrderSequences() ;

void copyFloatArray(
   float a[],
   int l,
   float b[]
) ;

void findLargestInteger(
   int *largest,
   int *possible
) ;
 
void filterAndNormalizeReflectionOrderImpulseResponses() ;

void makeOrIncreaseMemorySpaceForSavedWallReflectionPatterns() ;


void truncateEnvelopeAndNormalizeReflectionOrderImpulseResponses() ;

void getWallImpulseResponsePresenceLevels() ;

void balanceWallImpulseResponseAgainstPulseUsingPresence() ;

void makeIR_DataSpace() ;

void getReflectionOrderImpulseResponsePresenceLevels() ;
 
void balanceROimpulseResponseAgainstPulseUsingPresence() ;

void truncateCVOrderSequences() ; 


void preConvolveReflectionOrderImpulseResponsesWithIrconvolver() ;


void filterAndNormalizeReflectionOrderImpulseResponsesPostConvolution() ;

void findMaximumSpeakerToSpeakerDistance() ;

void findMaximumSpeakerToListenerDistance() ;

void makePreEchoValues() ;

void makeSourceToThresholdProximityProportion() ;

bool areCoordinatesTheSame(
   float p0[],
   float p1[]
) ;

void findSourceToSpeakerAngleDifferencesFromSourceToListenerAngle() ;

int main( argc, argv )
    int argc ; char *argv[] ;
{

 

int numberWritten ; 
 
int normalizeFlag=0 ; 
long int numFramesLeft ; 
int numFramesBufferedIn, maxNumFramesBufferedIn, numFramesBufferedOut; 

float masterGainInDecibels=0. ;

int i, k,  m, n;
int outputChannelNumber=0 ;
int polygonCoordinatesSource=0 ;
int numberOfOutputChannels ; 
int beginChannel, endChannel, channel ; 
 

int *channelOffSwitches; 



 
 
 

float outputChannelPeakAmps[ MAXIMUM_CHANNELS ], peakOutputChannelAmp=0., finalOutputChannelPeakAmps[ MAXIMUM_CHANNELS ],
	finalOutputChannelPeakAmp=0. ; 

 
int R=44100;
FILE *fopen() ;
char ch,  tempstring[ STRING_SIZE ],  *user ;
  

float tempBlock[ BLOCKSIZE ], *allChanOutputBlock ; 

int  frameNow ; 


int maxNumberOfOutputSampleFrames=0 ;

  
SF_INFO outputSFinfo ; 




//
int outputFileChannelNumber ; 

int corner ; 



 // 1280




 

//
//  WALL IMPULSE RESPONSE ENVELOPE
wall_IR_response_envelope.L = 1. ; wall_IR_response_envelope.n = 1. ; 
	wall_IR_response_envelope.A[ 0 ] = 0. ; 

//  REFLECTION ORDER IMPULSE RESPONSE ENVELOPE
reflection_order_IR_envelope.L = 1. ; reflection_order_IR_envelope.n = 1. ; 
	reflection_order_IR_envelope.A[ 0 ] = 0. ; 





if( argc < 2 )usage() ; 


while( (ch = crack( argc, argv, 
   "a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|X|x|y|Y|z|Z|_|1|2|3|4|5|6|7|8|9|~|=|@|!|:|/|0|", 0  )) 
   != CRACK_DONE_FLAG ) {
   switch(ch) 
{      



   case '/':	strcpy( crackTempString, arg_option) ; 
                switch( crackTempString[0] )
                {  // a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|
                   // r|R|s|S|t|T|u|U|v|V|w|X|x|y|Y|z|Z|_|1|2|3|4|5|6|7|8|9|~|=|@|!|:|/|0|

                   case 'a': strcpy( reflection_order_dB_gainscale_factors_file, 
                                &crackTempString[1] ) ; break;

                   case 'b': reflection_order_IR_convolution_mode = 
                                crackfloat( &crackTempString[1], ch ) ; break;


                   case 'c': RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3
                              = (int) crackfloat( &crackTempString[1], ch ) ; break;

                   case 'd': RO_IR_BPF_low_rolloff_frequency = 
                                crackfloat( &crackTempString[1], ch ) ; break;

                   case 'e': RO_IR_BPF_high_rolloff_frequency = 
                                crackfloat( &crackTempString[1], ch ) ; break;

                   case 'f': RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave = 
                                crackfloat( &crackTempString[1], ch ) ; break;

                   case 'g': RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave = 
                                crackfloat( &crackTempString[1], ch ) ; break;

                   case 'h': reflection_order_IR_truncate_duration = 
                                crackfloat( &crackTempString[1], ch ) ; break;
                   case 'i': reflection_order_IR_envelope.fp = 
                                 crackstring( &crackTempString[1], 
			    &reflection_order_IR_envelope ); break ;
                   case 'j': strcpy( wall_impulse_response_decibels_presence_file, 
                                &crackTempString[1] ) ; break;

                   case 'k': reflected_sound_gain_in_decibels = 
                                crackfloat( &crackTempString[1], ch ) ; break;
                   case 'l': direct_sound_gain_in_decibels = 
                                crackfloat( &crackTempString[1], ch ) ; break;

                   case 'm': airAbsorptionExponentForVirtualSpaceSource = 
                                crackfloat( &crackTempString[1], ch ) ; break;


                   case 'n': strcpy( reflection_order_impulse_response_decibels_presence_file, 
                                &crackTempString[1] ) ; break;


                   case 'o': listener_space_cross_reflections__include_0__exclude_1 = 
                                (int) crackfloat( &crackTempString[1], ch ) ; break;


                   case 'p': airAbsorptionExponentForRealSpaceSource = 
                                crackfloat( &crackTempString[1], ch ) ; break;

                   case 'q': orient_source__to_room_0__to_listener_1 = 
                                crackfloat( &crackTempString[1], ch ) ; break;





                   case 't': source_minimum_distance_from_listener = 
                                crackfloat( &crackTempString[1], ch ) ; break;


                   case 'u': use_collapsed_threshold_amplitudes__no_0__yes_1 = 
                                (int)  crackfloat( &crackTempString[1], ch ) ; break;

   				case 'v':	high_order_release_duration = 
						 crackfloat( &crackTempString[1], ch ) ; break;



                } ;
                break;

   case '0':	scale_to_pre_envelope_window_sample_peak_amps__off_0__on_1 = 
                    (int) crackfloat( arg_option, ch ) ; break;

   case '~':	low_order_attack_duration = crackfloat( arg_option, ch ) ; break;
   case '=':	low_order_release_duration = crackfloat( arg_option, ch ) ; break;
   case '@':	high_order_attack_duration = crackfloat( arg_option, ch ) ; break;
   case ':':	envelope_shape_index = crackfloat( arg_option, ch ) ; break;
    case 'Q':	reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 = 
                  (int) crackfloat( arg_option, ch ) ; break;

    case 'V':	strcpy( reflection_order_channel_assignments_file, arg_option) ; break;


    case '8':	reflection_order_impulse_responses__off_0__on_1 = (int) crackfloat( arg_option, ch ) ; break;
    case '9':	strcpy( reflectionOrderImpulseResponseInputSoundFileName, arg_option) ; break;


    case '7':	strcpy(tempstring, arg_option);
			wall_IR_response_envelope.fp = crackstring( tempstring, 
			    & wall_IR_response_envelope ); break ; 

    case '6':	wall_IR_truncate_duration = crackfloat( arg_option, ch ) ; break;



    case '3':	impulse_inclusion_threshold_in_dB = crackfloat( arg_option, ch ) ; break;

    case 'v':	endCropReleaseTime = crackfloat( arg_option, ch ) ; break;
   
    case 'U':	endCropDecibelThreshold = crackfloat( arg_option, ch ) ; break;


    case 'l':	highOrderLimit = (int) crackfloat( arg_option, ch ) ; break;
    case 'u':	lowOrderLimit = (int) crackfloat( arg_option, ch ) ; break;



    case 'T':	wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 = 
                   (int) crackfloat( arg_option, ch ) ; break;
    case 'x':	WIRBPF_low_rolloff_frequency = crackfloat( arg_option, ch ) ; break;
    case 'y':	WIRBPF_high_rolloff_frequency = crackfloat( arg_option, ch ) ; break;
    case 'z':	WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave = crackfloat( arg_option, ch ) ; break;
    case 'Z':	WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave = crackfloat( arg_option, ch ) ; break;



    case 't':	wall_impulse_and_gainscale_response_mode = (int) crackfloat( arg_option, ch ) ; break;

    case 'q':	reflections_time_scaler = crackfloat( arg_option, ch ) ; break;

    case '5':	wall_impulse_responses__off_0__on_1 = (int) crackfloat( arg_option, ch ) ; break;
    case 'K':	strcpy( wallImpulseResponseInputSoundFileName, arg_option) ; break;


    case '1':	strcpy( wall_channel_assignments_file, arg_option) ; break;
    case '2':	strcpy( wall_dB_gainscale_factors_file, arg_option) ; break;



    case 'X':	room_X_translation_factor = crackfloat( arg_option, ch ) ; break;
    case 'Y':	room_Y_translation_factor = crackfloat( arg_option, ch ) ; break;
    case 'b':	room_negXscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'B':	room_posXscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'e':	room_negYscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'E':	room_posYscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'A':	room_RotationInDegrees = crackfloat( arg_option, ch ) ; break;
    case 'M':	room_ScaleFactor = crackfloat( arg_option, ch ) ; break;



    case 'F':	speaker_X_translation_factor = crackfloat( arg_option, ch ) ; break;
    case 'g':	speaker_Y_translation_factor = crackfloat( arg_option, ch ) ; break;
    case 'h':	speaker_negXscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'H':	speaker_posXscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'I':	speaker_negYscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'j':	speaker_posYscaleFactor = crackfloat( arg_option, ch ) ; break;
    case 'J':	speaker_RotationInDegrees = crackfloat( arg_option, ch ) ; break;
    case 'k':	speaker_ScaleFactor = crackfloat( arg_option, ch ) ; break;





    case 'S':	speaker_configuration__sequence_0__polygon_1 = 
                   (int) crackfloat( arg_option, ch ) ; break;

    case 'O':	sourceRotationInDegrees = crackfloat( arg_option, ch ) ; break;

    case 'P':	sourceDispersionPatternRolloffInDecibels = crackfloat( arg_option, ch ) ; break;

    case 'm':	minimumReferenceDistanceInFeet = crackfloat( arg_option, ch ) ; break;

    case 'N':    normalizeFlag = (int) crackfloat( arg_option, ch ) ;
			break;

    case 'G':	masterGainInDecibels = crackfloat( arg_option, ch ) ; break;
		

    case 'a':	airAbsorptionExponentForReflections = crackfloat( arg_option, ch ) ; break;


    case 's':	output_sound__all_0__direct_1__reflections_2 = 
       (int) crackfloat( arg_option, ch ) ; break;//


    case 'C':	outputChannelNumber = (int) crackfloat( arg_option, ch ) ; break;

    case 'f':	polygonCoordinatesSource = (int) crackfloat( arg_option, ch ) ; break;


    case 'c':   strcpy( polygonCoordinates_datafile, arg_option); break;

    case 'L':	strcpy( listenerCoordinatesDataFile, arg_option) ; break;

    case 'i':	strcpy( sourceCoordinatesDataFile, arg_option) ; break;

    case 'o':	strcpy( speakerCoordinatesDataFile, arg_option) ; break;

    case 'p':	strcpy( soundPathsPlotFileName, arg_option) ; break;

    case '4':	plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 = 
                           (int) crackfloat( arg_option, ch ) ; break;

    case 'w':	numberOfWalls = (int) crackfloat( arg_option, ch ) ; break;

    case 'r':	rotationOfSyntheticRoomInDegrees = crackfloat( arg_option, ch ) ; break;

    case 'd':	minMaxDistanceToCornerFromOrigin[0] = crackfloat( arg_option, ch ) ; break;

    case 'D':	minMaxDistanceToCornerFromOrigin[1] = crackfloat( arg_option, ch ) ; break;

    case 'R':	polygonAngleRegularityProportion = crackfloat( arg_option, ch ) ; break;

    case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;




	} 
} ;




// **** SET UPS *****

if( output_sound__all_0__direct_1__reflections_2 == 1 )
{
   reflectionsFlag = false ; 
}else if( output_sound__all_0__direct_1__reflections_2 == 2 )
{
   directSoundFlag = false ; 
} ;


thresholdProximityScalarSwitch = (use_collapsed_threshold_amplitudes__no_0__yes_1 == 1) ? 
     true : false ; 


wallImpulseResponsesFromSoundFileFlag = (wall_impulse_responses__off_0__on_1 == 1) ? true : false ; 


PI = 4.*atan(1.) ; TWOPI = 8.* (float) atan(1.) ;

// if( endCropDecibelThreshold < -96. ) endCropDecibelThreshold = -96. ;

sourceRotation =  PI * fmodf( sourceRotationInDegrees, 360. ) / 180. ; // C
	// CHANGE THIS sourceToListenerAnglePlusRotation to rotatedSource
if( orient_source__to_room_0__to_listener_1 == 0) 
	rotatedSource = sourceRotation ;
else
	rotatedSource = sourceToListenerAngle + sourceRotation ; // D

while( rotatedSource > PI ) 
   rotatedSource -= TWOPI ; 
while( rotatedSource < (-1. * PI) ) 
   rotatedSource += TWOPI ; 


if( WIRBPF_low_rolloff_frequency < 0. ) WIRBPF_low_rolloff_frequency = 0. ;
if( WIRBPF_high_rolloff_frequency <= 0. ) WIRBPF_high_rolloff_frequency = nyquist ; 

if(   ( 
         ( WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave == 0.) &&
         ( WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave == 0.)
      ) && (wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 != 0)
)
{
   prt( "TURNING OFF WALL BPF FILTER GIVEN 0 dB LOWER AND UPPER ROLLOFF.\n\n" ) ;
   wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 = 0 ;  
};


if( RO_IR_BPF_low_rolloff_frequency < 0. ) RO_IR_BPF_low_rolloff_frequency = 0. ;
if( RO_IR_BPF_high_rolloff_frequency <= 0. ) RO_IR_BPF_high_rolloff_frequency = nyquist ; 

if(   ( 
         ( RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave == 0.) &&
         ( RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave == 0.)
      ) && (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 != 0)
)
{
   prt( "TURNING OFF REFLECTION ORDER BPF FILTER GIVEN 0 dB LOWER AND UPPER ROLLOFF.\n\n" ) ;
   RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 = 0 ;  
};





prf( sourceRotationInDegrees, 
    "SOURCE ROTATION AWAY FROM LISTENER IN DEGREES (0 = FACING LISTENER)" ) ; 

//**** SEED RANDOM
    srandom(1);


// GET NAME OF USER
user = getlogin(); 

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "ROOM RESPONSE MAKER", 69 ) ; 
prline( 69,  "-" ) ; 


if( highOrderLimit < lowOrderLimit ){
   prt( "--------> ERROR: HIGHEST AND LOWEST ORDER REFLECTION SETTINGS ARE BACKWARDS.\n") ;
   pri( highOrderLimit, "HIGH ORDER" ) ; 
   pri( lowOrderLimit, "LOW ORDER" ) ; 
   prt( "\n\n . . . BYE.\n" ) ; 
   exit( EXIT_FAILURE ) ;
} ;



// *****ROOM POLYGON COORDINATES *******
getPolygonCoordinates( polygonCoordinatesSource ) ;

// *** ROTATE AND SCALE ROOM ****
scaleAndRotateCoordinates( 
   "ROOM",
   coordinatesOfCorners,
   numberOfCorners,
   originToCornerAngles,
   originToCornerDistances,
   room_X_translation_factor,
   room_Y_translation_factor,
   room_negXscaleFactor,
   room_posXscaleFactor,
   room_negYscaleFactor,
   room_posYscaleFactor,
   room_RotationInDegrees,
   room_ScaleFactor
) ;


// TEST ROOM POLYGON FOR NON-INTERSECTING SIDES AND CONCAVITY
prbanner( "ROOM POLYGON TESTS:", 69 ) ; 
if( polygonTest( coordinatesOfCorners, numberOfCorners, "ROOM" ) == false )
{
   exit( EXIT_FAILURE ) ; 
} ;

isPolygonConcave( coordinatesOfCorners, numberOfCorners ) ;

if( polygonIsConcave ) prt( "\nPOLYGON IS CONCAVE.\n\n" ) ; else prt( "\nPOLYGON IS CONVEX.\n\n" ) ; 




// TEST TO SEE IF ORIGIN IS INSIDE POLYGON.
pointInPolygonTest( coordinatesOfCorners, originCoordinates,  1, "ORIGIN" ) ;





// *****SOURCE COORDINATES *******
getSourceCoordinates() ;

// ARE SOURCE POSITIONS INSIDE POLYGON? 
// pointInPolygonTest( coordinatesOfCorners, sourceCoordinates,  numberOfSourcePositions, "SOURCE" ) ;

// *****SPEAKER COORDINATES *******
getSpeakerCoordinates() ;


// *** ROTATE AND SCALE SPEAKER POSITIONS ****
scaleAndRotateCoordinates( 
   "SPEAKERS",
   speakerCoordinates,
   numberOfSpeakerPositions,
   originToSpeakerAngles,
   originToSpeakerDistances,
   speaker_X_translation_factor,
   speaker_Y_translation_factor,
   speaker_negXscaleFactor,
   speaker_posXscaleFactor,
   speaker_negYscaleFactor,
   speaker_posYscaleFactor,
   speaker_RotationInDegrees,
   speaker_ScaleFactor
) ;



// ARE SPEAKER POSITIONS INSIDE POLYGON? 
pointInPolygonTest( coordinatesOfCorners, speakerCoordinates,  numberOfSpeakerPositions, "SPEAKER" ) ;

// IF DESIGNED TO BE A POLYGON, TEST SPEAKER COORDINATES FOR NON-INTERSECTING SIDES.
if( speaker_configuration__sequence_0__polygon_1 == 1 ){ 
   prt( "SPEAKER POSITIONS ARE DESIGNED AS A POLYGON. \nTESTING COORDINATES . . . " ) ; 
    ;
   if( polygonTest( speakerCoordinates, numberOfSpeakerPositions, "SPEAKER POSITIONS" ) == false )
   {
      exit( EXIT_FAILURE ) ; 
   } ;
} ;



// ******LISTENER COORDINATES ******
getListenerCoordinates() ;


pointInPolygonTest( coordinatesOfCorners, listenerCoordinates,  numberOfListenerPositions, "LISTENER" ) ;

// MAKE LISTENER TO SPEAKER ANGLES
makeListenerToSpeakerAngles() ;


// **** TEST SPEAKER COORDINATES FOR ANGULAR ORDER RELATIVE TO LISTENER POSITION(S).
orderOfAnglesTest( listenerToSpeakerAngles, numberOfSpeakerPositions );

// MAKE DISTANCES BETWEEN SPEAKER COORDINATES. 
makeSpeakerSeriesSegmentDistances(); 


// ***TEST LISTENER COORDINATES ****



// OUTPUT CHANNEL
if( outputChannelNumber == 0 ){
   beginChannel = 0; 
   endChannel = ( numberOfSourcePositions > numberOfSpeakerPositions ) ? 
      numberOfSourcePositions - 1 : numberOfSpeakerPositions - 1 ;
   pri( endChannel + 1, "NUMBER OF OUTPUT CHANNELS" ) ;  
} else {
   beginChannel = outputChannelNumber; endChannel = outputChannelNumber ;
   pri( beginChannel + 1, "SELECTED OUTPUT CHANNEL" ) ; 
} ;
numberOfOutputChannels = endChannel - beginChannel + 1 ; 

pri( numberOfOutputChannels, "numberOfOutputChannels" ) ; 

// ******* FIND MAX SPEAKER-TO-SPEAKER DISTANCE
findMaximumSpeakerToSpeakerDistance() ;

prf( maxSpeakerToSpeakerDistance, "maxSpeakerToSpeakerDistance" ) ; 
prf( maxSpeakerToSpeakerDistanceDelayTime, "maxSpeakerToSpeakerDistanceDelayTime" ) ; 


// ******* FIND MAX SPEAKER-TO-LISTENER DISTANCE
findMaximumSpeakerToListenerDistance() ;

makePreEchoValues() ; 


// 
prbanner( "ROOM RESPONSE PARAMETERS", 69 ) ; 
pri( highOrderLimit, "HIGHEST ORDER REFLECTION" ) ; 
pri( lowOrderLimit, "LOWEST ORDER REFLECTION" ) ; 


ivec( originalWallReflectionSequence_firstToLast, highOrderLimit ) ; 
ivec( sequenceOfWallIResponses, (highOrderLimit * 2) + 1 ) ; // THIS IS DOUBLE TO HOLD BOTH THE WALL
                                                             // AND REFLECTION ORDER SEQUENCES. 
ivec( sequenceOfWallIResponsesNow, (highOrderLimit * 2) + 1 ) ; 

ivec( reflectionDataSetChannelPointer, numberOfOutputChannels );
 
fvec( reflectionDistances, reflectionNumberLimit ) ;
fvec( reflectionTimes, reflectionNumberLimit ) ;
ivec( reflectionOrders, reflectionNumberLimit ) ;
ivec( reflectionWalls_lastToFirst, reflectionNumberLimit * highOrderLimit ) ; 
fvec( reflectionSoundPathCoordinates, reflectionNumberLimit * (highOrderLimit + 2) * 2 ) ; 

fvec( sourceOrientationToReflectionAngleDifference, reflectionNumberLimit ) ; 

ivec( viableReflectionCount, numberOfOutputChannels ) ;
ivec( totalExaminedReflections, numberOfOutputChannels ) ;

ivec( viableReflectionCountByOrder, (highOrderLimit + 1) * numberOfOutputChannels ) ; 

fvec( polygonCoordinates, numberOfCorners * 2 * (highOrderLimit + 1) ) ; 
fvec( mirroredPolygonCoordinates, numberOfCorners * 2 * (highOrderLimit + 1) ) ;
fvec( sourceCoordinatesForThisPolygon, (highOrderLimit + 1) * 2 ) ; 



if( argc > 2   ){ 	// 

	// GET OUTPUT SOUNDFILE NAME
	strcpy( ofile, argv[arg_index] ) ; 

 	prs( ofile, "OUTPUT FILE" ) ;

}else{
	// NO OUTPUT SOUND FILE
	prt( "\n\nMISSING OUTPUT FILE\n" ) ; 
	usage() ; 
     exit(EXIT_FAILURE) ;
}  ; 

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
sampleRate = osr = outputSFinfo.samplerate ;  
nyquist = R/2.0;

// CLOSE IT. 
sf_close( outfile ) ;     


prf( WIRBPF_low_rolloff_frequency, "FILTER: LOW ROLLOFF FREQUENCY" ) ; 
prf( WIRBPF_high_rolloff_frequency, "FILTER: HIGH ROLLOFF FREQUENCY" ) ; 
prf( WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave, "FILTER: LOW AMPLITUDE ROLLOFF (dB per octave)" ) ; 
prf( WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave, "FILTER: HIGH AMPLITUDE ROLLOFF (dB per octave)" ) ; 

// ********* WALL IMPULSE RESPONSES *********

if( reflectionsFlag )
{
   prline( 69,  "/" ) ; 
   prline( 69,  "-" ) ; 
   prbanner( "WALL IMPULSE RESPONSES", 69 ) ; 
   prline( 69,  "-" ) ; 

   // READ IN WALL IMPULSE RESPONSE CHANNEL ASSIGNMENTS

   getWallImpulseResponseChannelAssignments() ; 
   getWallDecibelGainscaleLevels() ; 
   readInWallImpulseResponses() ; 
   truncateEnvelopeAndNormalizeWallImpulseResponses() ; 
   getWallImpulseResponsePresenceLevels() ; 
   filterAndNormalizeWallImpulseResponses() ; 
   balanceWallImpulseResponseAgainstPulseUsingPresence() ; 

   findWallImpulseResponsesCroppedSize() ; 


   // ********* REFLECTION ORDER IMPULSE RESPONSES ***********

   prline( 69,  "/" ) ; 
   prline( 69,  "-" ) ; 
   prbanner( "REFLECTION ORDER IMPULSE RESPONSES", 69 ) ; 
   prline( 69,  "-" ) ; 

   getReflectionOrderImpulseResponseChannelAssignments() ; 
   getReflectionOrderDecibelGainscaleLevels() ; 
   readInReflectionOrderImpulseResponses() ; 
   truncateEnvelopeAndNormalizeReflectionOrderImpulseResponses() ; 
   getReflectionOrderImpulseResponsePresenceLevels() ; 
   filterAndNormalizeReflectionOrderImpulseResponses() ; 
   balanceROimpulseResponseAgainstPulseUsingPresence() ;  


   findreflectionOrderCVOrderSequences() ; 

   preConvolveReflectionOrderImpulseResponsesWithIrconvolver() ;
   filterAndNormalizeReflectionOrderImpulseResponsesPostConvolution() ;
   truncateCVOrderSequences() ;


} ;

   // MAKE MEMORY FOR SOURCE AND MIRRORED POLYGONS
fvec( mirrorSegments, highOrderLimit * 4 ) ; 
fvec( mirrorSegmentIntersectCoordinates, highOrderLimit * 2 ) ;
fvec( coordinatesForThisSoundPath, (highOrderLimit + 2) * 2 ) ; 
ivec( mirrorSideNumber, highOrderLimit ) ;  


// FILL FIRST POLYGON 
if( reflectionsFlag )
{   
   for( corner = 0; corner < numberOfCorners ; corner++ ){
	polygonCoordinates[ (corner * 2) + 0] = 
		coordinatesOfCorners[ (corner * 2) + 0 ] ; 	
	polygonCoordinates[ (corner * 2) + 1] = 
		coordinatesOfCorners[ (corner * 2) + 1 ] ; 	
   }; 
} ;


//pri( beginChannel, "BEGIN CHANNEL" ) ; 
//pri( endChannel, "END CHANNEL" ) ; 

// ZERO PEAK OUTPUT AMPS
for(channel = 0; channel < numberOfOutputChannels ; channel++ ) outputChannelPeakAmps[channel] = 0. ; 

// MAKE SOURCE TO SPEAKER DISTANCES AND ANGLES FOR SOURCE PROXIMITY SCALING.
makeSourceToSpeakerDistancesAndAngles() ;

// FIND LISTENER TO SOURCE SEGMENT LENGTH AND ANGLE
findListenerToSourceSegmentLengthAndAngle() ; 


// MAKE SOURCE TO THRESHOLD PROXIMITY DISTANCE
makeSourceToThresholdProximityDistance() ; 

// DETERMINE ORIENTATION OF SOURCE TO SPEAKERS.
isSourceBehindOrInFrontOfSpeakerThreshold() ;

// CREATE AMPLITUDE SCALARS AND CROSSFADE PROPORTION FOR SOURCE.
makeDirectSoundSpeakerAmplitudes() ;

// CREATE THE SOURCE SPEAKER DELAY TIMES
makeDirectSoundSpeakerDelayTimes() ;


// FIND SOURCE TO SPEAKER ANGLE DIFFERENCES FROM SOURCE TO LISTENER ANGLE.
findSourceToSpeakerAngleDifferencesFromSourceToListenerAngle() ;


// LOOP FOR MULTIPLE CHANNELS
for( channel = beginChannel, outputFileChannelNumber = 0; 
	channel <= endChannel; channel++, outputFileChannelNumber++ ){


   
   if( reflectionsFlag )
   {
      if( outputFileChannelNumber == 0 ){
         reflectionDataSetChannelPointer[ outputFileChannelNumber ] = 0 ;
      }else
      {
         reflectionDataSetChannelPointer[ outputFileChannelNumber ] = 
            reflectionDataSetChannelPointer[ outputFileChannelNumber - 1 ] +
            viableReflectionCount[ outputFileChannelNumber - 1 ] ;

      } ;
   } ;




//   pri( channel, "\n\n***************** CHANNEL" ) ; 

   // FILL SOURCE COORDINATES FOR FIRST POLYGON 
   sourceCoordinatesForThisPolygon[ 0 ] = sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ] ;
   sourceCoordinatesForThisPolygon[ 1 ] = sourceCoordinates[ ((channel % numberOfSourcePositions) * 2) + 1 ] ;

   // GET SPEAKER COORDINATES FOR THIS CHANNEL.
   speakerCoordinatesNow[ 0 ] = speakerCoordinates[ (channel % numberOfSpeakerPositions) * 2 ] ;
   speakerCoordinatesNow[ 1 ] = speakerCoordinates[ ((channel % numberOfSpeakerPositions) * 2) + 1 ] ;


   // SET DATA COUNTS AND SAVE
   
   if( reflectionsFlag )
   {
      viableReflectionCount[ outputFileChannelNumber ] = 0 ;    
      for( i = 0; i < highOrderLimit; i++)
         viableReflectionCountByOrder[ (outputFileChannelNumber * highOrderLimit) + i ] = 0 ; 

      mirrorPolygonCoordinatesAroundAllSides(   
         outputFileChannelNumber,
         1, // int thisOrder,  // SET TO 1 WITH FIRST LEVEL CALL
         -1, // int polygonMirrorSideIndex, // SET TO -1
         0., // mirrorSegAngleLimitsTempLow
         0.// mirrorSegAngleLimitsTempHigh
      ) ;
   } ;

   
   if( reflectionsFlag )
   {
      pri( totalExaminedReflections[ outputFileChannelNumber ], "totalExaminedReflections" ) ; 
      pri(  viableReflectionCount[ outputFileChannelNumber ], "viableReflectionCount" ) ; 
      for( i = 0; i < highOrderLimit; i++){
         pri( viableReflectionCountByOrder[ (outputFileChannelNumber * highOrderLimit) + i ],
          "viableReflectionCountByOrder" ) ; 
      } ;

      
      for(i = 0 ; i < viableReflectionCount[ outputFileChannelNumber ]; i++){

         fprintf( stderr, "\nREFLECTION SIDES (first to last): " ) ; 
         for( n = reflectionOrders[ reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i ] - 1 ; 
            n >=  0; n-- ) fprintf( stderr, "%d ", 
               reflectionWalls_lastToFirst[ 
                  (reflectionDataSetChannelPointer[ outputFileChannelNumber ] * highOrderLimit) + 
               (i * highOrderLimit) + n ] ) ; 
      } ;

   } ;
} ;




// NOW WRITE IMPULSES TO TEMP FOR EACH CHANNEL OF REFLECTION DATA.

// FIRST FIND THE LONGEST DELAY TIME AND THE GREATEST NUMBER OF REFLECTIONS. 

longestReflectionDistance = 0. ; longestDelayTime = 0. ; maximumNumberOfReflections = 0 ; 
if( reflectionsFlag ) for( channel = beginChannel, outputFileChannelNumber = 0; channel <= endChannel; 
      channel++, outputFileChannelNumber++ )
{
   pri( outputFileChannelNumber, "C outputFileChannelNumber" ) ; 

   if( viableReflectionCount[ outputFileChannelNumber ] > maximumNumberOfReflections ) 
      maximumNumberOfReflections = viableReflectionCount[ outputFileChannelNumber ] ;   

   for( i = 0; i < viableReflectionCount[ outputFileChannelNumber ]; i++ ){
      if( reflectionTimes[reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i] 
         > longestDelayTime )
      {
         longestDelayTime = 
            reflectionTimes[reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i] ; 
         longestDelayTimeIndex = i ; 

         longestReflectionDistance  = 
            reflectionDistances[ reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i ] ;

      };

      if( reflectionDistances[reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i] < 1.){
         prt( "ERROR" ) ; 
         fprintf( stderr, "\n reflectionTimes[ %d ]: %f", outputFileChannelNumber,
          reflectionDistances[reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i] ) ; 
      } ;
   } ;
} ;
if( reflectionsFlag )
{
   prf( longestDelayTime, "LONGEST REFLECTION DELAY TIME" ) ; 
   prf( longestReflectionDistance, "LONGEST REFLECTION DISTANCE" ) ; 
   pri( maximumNumberOfReflections, "MAXIMUM NUMBER OF OUTPUT CHANNEL REFLECTIONS" ) ; 
} ;

//duration = longestDelayTime + 0.01 ;

//prf( duration, "duration" ) ; 



IR_DataLength = (int)(7. * (float) osr)  ; 


pri( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2,
    "plot_mode__one_file_0__channel_files_1__channel_and_order_files_2" ) ; 

// LOOP FOR OUTPUT SPEAKER CHANNELS
if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 0 )
 	 makeRoomReflectionsPlotFiles( 0 ) ;


// MAKE ARRAY SPACE FOR SORTING THE OUTPUT WALL REFLECTION PATTERNS. 
if( reflectionsFlag )
   ivec( reorderedWallIRSequences_firstToLast, maximumNumberOfReflections * (2 + highOrderLimit) ) ; 

prt( "MAKING IR SEQUENCE MEMORY" ) ; 
makeOrIncreaseMemorySpaceForSavedWallReflectionPatterns() ;
prt( "MEMORY MADE" ) ; 


for( channel = beginChannel, outputFileChannelNumber = 0; 
	channel <= endChannel; channel++, outputFileChannelNumber++ ){

   pri( outputFileChannelNumber, "***  outputFileChannelNumber *** " ) ; 

   
   if( reflectionsFlag ) findAverageReflectionOrderDelayTimes( outputFileChannelNumber ) ; 

   if( 
      (plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 1) ||
      (plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 2)
    ) makeRoomReflectionsPlotFiles( outputFileChannelNumber ) ;

   
   if( reflectionsFlag ) createAndReorderWallReflectionSequence( outputFileChannelNumber ) ; 

   // WRITE PULSES MODIFIED BY DISTANCE INTO IMPULSE RESPONSE.
   highestIndexWritten = 0 ; 

   makeIR_DataSpace() ; 


   // *** WRITE REFLECTIONS
   
   if( reflectionsFlag ) 
   {


     numberWritten = writeReflectionPulsesIntoImpulseResponse( 
        outputFileChannelNumber, numberOfOutputChannels 
      ); 

      pri( highestIndexWritten, "highestIndexWritten" ) ;
      pri( IR_DataLength, "IR_DataLength" ) ;  
      pri( numberWritten, "NUMBER OF REFLECTION PULSES WRITTEN INTO IMPULSE RESPONSE." ) ; 

   } ;

   // *** ADD THE SOURCE IF REQUESTED. 
   if( directSoundFlag ) writeDirectSourcePulsesIntoImpulseResponse( channel ) ;


   // APPLY MASTER GAIN
   for( m = 0 ; m < IR_DataLength ; m++ ){
      IR_Data[ m ] *= dB_to_amp( masterGainInDecibels ) ; 
   } ;





   // FIND PEAK AMP FOR THIS CHANNEL AND SAVE.
   outputChannelPeakAmps[ outputFileChannelNumber ] = 0. ;
   for( m = 0 ; m < IR_DataLength ; m++ ){
      if( fabs( IR_Data[ m ] ) > outputChannelPeakAmps[ outputFileChannelNumber ] ) 
         outputChannelPeakAmps[ outputFileChannelNumber ] = fabs( IR_Data[ m ] ) ;
   } ;
   prf( outputChannelPeakAmps[ outputFileChannelNumber ], "outputChannelPeakAmps" ) ; 

   // MAKE /tmp OUTPUT FILE
   sprintf( tempstring, "/tmp/%s.OutputChan.%d", user, outputFileChannelNumber ) ; // MAKE FILE NAME
	
   prs( tempstring, "TEMP OUTPUT FILE" ) ; 
 	
   prt( "OPENING FILE TO WRITE BINARY" ); 
   inputTempChanFiles[ outputFileChannelNumber ] = fopen( tempstring, "wb" ); 
pri( IR_DataLength, "IR_DataLength" ) ; 


   croppedIR_DataLength = cropIR_DataEndForSilence(
      outputChannelPeakAmps[ outputFileChannelNumber ] * dB_to_amp( endCropDecibelThreshold ),
      false
   ) ;
pri( croppedIR_DataLength, "croppedIR_DataLength" ) ; 
pri( maxNumberOfOutputSampleFrames, "2 maxNumberOfOutputSampleFrames" ) ; 
   if( croppedIR_DataLength > maxNumberOfOutputSampleFrames ) 
           maxNumberOfOutputSampleFrames = croppedIR_DataLength ; 
pri( maxNumberOfOutputSampleFrames, "3 maxNumberOfOutputSampleFrames" ) ; 

   smoothReleaseOfCroppedEnd( IR_Data, croppedIR_DataLength, endCropReleaseTime  ) ;

//pri( IR_DataLength, "ADJUSTED IR_DataLength" ) ; 

//prt( "BEFORE TEMP FILE CHANNEL WRITE" ) ; 

   // WRITE TEMP DATA
//   prt( "WRITING BINARY TO FILE" ); 

   numFramesBufferedOut = 
      fwrite( &IR_Data[ 0 ], sizeof(float), croppedIR_DataLength, 
              inputTempChanFiles[ outputFileChannelNumber ] ) ;

//prt( "AFTER TEMP FILE CHANNEL WRITE" ) ; 

    
//   prt( "CLOSING FILE" ); 

   pri( outputFileChannelNumber, "A outputFileChannelNumber" ) ; 
 
   fclose( inputTempChanFiles[ outputFileChannelNumber ] ) ; 

// prt( "AFTER FILE CLOSE" ) ; 

   if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 1 ) fclose( plotFilePointer ) ;
   if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 2 ) 
   {
      for( i = 0; i < highOrderLimit ; i++ ) fclose( channelOrderPlotFilePointers[ i ] ) ;
   } ;

// prt( "AFTER PLOT FILE CLOSE" ) ; 

} ;

// prt( "AFTER CHANNELS LOOP" ) ; 

free( ROimpulseResponsePresenceLevels ) ; 

free( reflectionOrderCVOrderSequences ) ; 
free( reflectionOrderCVIRSequences ) ;
free( reflectionOrderCVOrderSequenceSizes ) ;

free( orderAverageReflectionTimes ) ; 

free( sampleOrderAttackDurations ) ; 
free( sampleOrderReleaseDurations ) ; 

free( reflectionOrderDecibelGainscaleLevels ) ; 

// ???


if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 0 ) fclose( plotFilePointer ) ;

//maxNumberOfOutputSampleFrames = IR_DataLength ;


prs( ofile, "\nOUTPUT FILE NAME" ) ; 

outputSFinfo.channels = numberOfOutputChannels ; // ;
fprintf( stderr, "\n -- CHANNELS: %d\n -- SAMPLE RATE: %d\n -- FRAMES: %d\n", 
		outputSFinfo.channels, osr, (int) maxNumberOfOutputSampleFrames) ; 

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

// REWIND TEMP FILES
for( channel = beginChannel, outputFileChannelNumber = 0; 
   channel <= endChannel; channel++, outputFileChannelNumber++ ){
   sprintf( tempstring, "/tmp/%s.OutputChan.%d", user, outputFileChannelNumber ) ; 
   // MAKE FILE NAME     
//   prs( tempstring, "TEMP OUTPUT FILE" ) ; 
//   prt( "OPENING FILE TO READ BINARY" ); 

   inputTempChanFiles[ outputFileChannelNumber ] = fopen( tempstring, "rb" ); 
   fseek( inputTempChanFiles[ outputFileChannelNumber ], 0, SEEK_SET ) ;
} ; 

// FIND CHANNEL WITH GREATEST AMP.
peakOutputChannelAmp = 0. ; 
for(outputFileChannelNumber = 0; outputFileChannelNumber < numberOfOutputChannels; outputFileChannelNumber++) 
   if( outputChannelPeakAmps[outputFileChannelNumber] > peakOutputChannelAmp ) 
      peakOutputChannelAmp = outputChannelPeakAmps[ outputFileChannelNumber ] ; 


// PRINT PEAK AMPS IN DB
prbanner( "PEAK OUTPUT CHANNEL AMPLITUDES", 69 ) ; 

for(outputFileChannelNumber = 0; outputFileChannelNumber < numberOfOutputChannels; 
      outputFileChannelNumber ++){ 
   fprintf( stderr, "(%d) %f dB", 
      outputFileChannelNumber + 1, amp_to_dB( outputChannelPeakAmps[outputFileChannelNumber] ) ) ; 
   if( outputChannelPeakAmps[outputFileChannelNumber] == peakOutputChannelAmp ) fprintf( stderr, " (peak)" ) ; 
   if( outputChannelPeakAmps[outputFileChannelNumber] > 1.0 )
      fprintf( stderr, " <--- WARNING: AMPLITUDE EXCEEDS 0 dB!" ) ; 
   fprintf( stderr, "\n" ) ;
} ; 


if( normalizeFlag != 0 ) prbanner( "NORMALIZATION", 69 ) ;
if( normalizeFlag == 1 )fprintf( stderr, "\nNORMALIZING EACH CHANNEL SEPARATELY"); 
if( normalizeFlag == 2 )fprintf( stderr, "\nNORMALIZING CHANNELS TOGETHER"); 
if( normalizeFlag == 3 )fprintf( stderr, "\nWILL NORMALIZE IF PEAK EXCEEDS 0 DECIBELS"); 


// BEGIN INTERLEAVE
// MAKE SPACE
fvec( allChanOutputBlock, BLOCKSIZE * numberOfOutputChannels ) ; 

// ZERO OUTPUT ARRAY
for(i = 0; i < BLOCKSIZE * numberOfOutputChannels; i++) allChanOutputBlock[i] = 0. ; 


pri( maxNumberOfOutputSampleFrames, "1 maxNumberOfOutputSampleFrames" ) ; 
numFramesLeft = maxNumberOfOutputSampleFrames ; //
frameNow = 0 ; 

prt( "\nINTERLEAVING /tmp FILES INTO OUTPUT FILE . . . \n" ) ; 

// ZERO PEAK OUTPUT AMPS
for(outputFileChannelNumber = 0; outputFileChannelNumber < numberOfOutputChannels; 
   outputFileChannelNumber++ ) finalOutputChannelPeakAmps[outputFileChannelNumber] = 0. ; 

pri( numberOfOutputChannels, "NUMBER OF OUTPUT CHANNELS" ); 

ivec( channelOffSwitches, numberOfOutputChannels ) ; 

pri( numFramesLeft, "numFramesLeft" ) ; 

while( numFramesLeft > 0 ){
   // FRAMES LOOP

   // ZERO OUTPUT ARRAY
   for(i = 0; i < BLOCKSIZE * numberOfOutputChannels; i++) allChanOutputBlock[i] = 0. ; 

   maxNumFramesBufferedIn = 0 ; 

   for(outputFileChannelNumber = 0; outputFileChannelNumber < numberOfOutputChannels; 
      outputFileChannelNumber++ ){

//pri( outputFileChannelNumber, "READ IN outputFileChannelNumber" ) ; 
//pri( channelOffSwitches[ outputFileChannelNumber ], "channelOffSwitches[ outputFileChannelNumber ]" ); 

      if( channelOffSwitches[ outputFileChannelNumber ] != 1 ){

         // POSITION IN FILE FOR READ
         fseek( inputTempChanFiles[ outputFileChannelNumber ], frameNow * sizeof(float), SEEK_SET ) ;  
         // READ IN A CHANNEL BLOCK
         numFramesBufferedIn = 
            fread( &tempBlock, sizeof(float), BLOCKSIZE, inputTempChanFiles[ outputFileChannelNumber ] ) ; 


         // NORMALIZE IF WANTED
         // NORMALIZE CHANNELS SEPARATELY
         if( (normalizeFlag == 1) && (outputChannelPeakAmps[ outputFileChannelNumber ] > 0.) )
            for( i = 0; i < numFramesBufferedIn ; i++ ) tempBlock[i] = 
               (tempBlock[i] / (outputChannelPeakAmps[ outputFileChannelNumber ] )) * (32767.0 / 32768.0) ;  
	// NORMALIZE CHANNELS TOGETHER
	if( (normalizeFlag == 2) && (peakOutputChannelAmp > 0.) )
            for( i = 0; i < numFramesBufferedIn ; i++ ) 
               tempBlock[i] = (tempBlock[i] / (peakOutputChannelAmp )) * (32767.0 / 32768.0) ;  
         // NORMALIZE CHANNELS IF PEAK EXCEEDS 0 DECIBELS
         if( (normalizeFlag == 3) && (peakOutputChannelAmp > 1.) )
            for( i = 0; i < numFramesBufferedIn ; i++ ) 
               tempBlock[i] = ( tempBlock[i] / peakOutputChannelAmp ) * (32767.0 / 32768.0) ;  

         if( numFramesBufferedIn < BLOCKSIZE ) channelOffSwitches[ outputFileChannelNumber ] = 1 ; 

         if( numFramesBufferedIn > maxNumFramesBufferedIn ) maxNumFramesBufferedIn = numFramesBufferedIn ;  

         // INTERLEAVE CHANNEL BLOCK INTO OUTPUT ARRAY AND TEST FOR PEAK AMPS
         for(i = 0, k = outputFileChannelNumber; i < numFramesBufferedIn; i++, k += numberOfOutputChannels ){ 
            allChanOutputBlock[k] = tempBlock[i] ; 
            if( fabs( tempBlock[i] ) > finalOutputChannelPeakAmps[ outputFileChannelNumber ] ) 
                   finalOutputChannelPeakAmps[ outputFileChannelNumber ] = fabs( tempBlock[i] ) ;

            // CLIP
            if( allChanOutputBlock[k] > (32767.0 / 32768.0) ) allChanOutputBlock[k] =  (32767.0 / 32768.0) ;
            if( allChanOutputBlock[k] < (-32767.0 / 32768.0) ) allChanOutputBlock[k] =  (-32767.0 / 32768.0) ;
         } ;



      } ;
   };  

   // WRITE BUFFER OUT TO SOUNDFILE
   sf_write_float( outfile, allChanOutputBlock, maxNumFramesBufferedIn * numberOfOutputChannels  ) ; 

   numFramesLeft -=  maxNumFramesBufferedIn ; 
   frameNow += maxNumFramesBufferedIn ; 

} ; 


// END OF INTERLEAVE

// CLOSE SOUND FILE. 
sf_close( outfile ) ; 


// FINAL PEAK OUTPUT AMPS IN DB
prbanner( "FINAL PEAK OUTPUT AMPLITUDES", 69 ) ; 


for(outputFileChannelNumber = 0; outputFileChannelNumber < numberOfOutputChannels; outputFileChannelNumber ++){ 
   if( finalOutputChannelPeakAmps[ outputFileChannelNumber ] > finalOutputChannelPeakAmp ) 
      finalOutputChannelPeakAmp = finalOutputChannelPeakAmps[ outputFileChannelNumber ] ;
} ; 

for(outputFileChannelNumber = 0; outputFileChannelNumber < numberOfOutputChannels; outputFileChannelNumber ++){ 
   fprintf( stderr, "(%d) %f dB", outputFileChannelNumber + 1,
      amp_to_dB( finalOutputChannelPeakAmps[outputFileChannelNumber] ) ) ; 
      if( finalOutputChannelPeakAmps[outputFileChannelNumber] == finalOutputChannelPeakAmp ) 
         fprintf( stderr, " (peak)" ) ; 
      fprintf( stderr, "\n" ) ;
} ; 






// ********

pri( convolutionCount, "NUMBER OF CONVOLUTION CALLS" ) ; 

filesToRemove( tempstring, 0 ) ; 

prt( "\nSUCCESS!\n\n" ) ; 



autoplay( autoplayreps, 0 ) ; 


exit(EXIT_SUCCESS) ;




}



void usage()
{
	fprintf(stderr, "%s",
	"roomresponsemaker:    \n"
	"roomresponsemaker   [flags]\n"
	"	    Most formats accepted.\n"
	"	    (Values in brackets denote defaults.)\n"
	"	    ** All distances are expressed in feet.\n"
	"	f:  polygonal room coordinates source: \n"
	"	       0 = synthesized (see synthesis flags below), \n"
	"	       1 = Cartesian (X-Y) coordinates from file (see -c) \n" 
	"	       2 = Polar (Angle-radius) Coordinates from file (see -c) [0]\n"
	"	c:  polygonal-room, corner coordinates file\n"
	"	    **** POLYGONAL ROOM SYNTHESIS:\n"
	"		(If a coordinates file is not supplied, a room will\n"
	"		be synthesized according to the following constraints.)\n"
	"	w:  number of walls [4]\n"
	"	r:  synthesized room rotation in degrees [0.]\n"
	"	    ** Distance to Room Corners from Origin:\n"
	"	d:  minimum distance in feet [30]\n"
	"	D:  maximum distance  in feet [30]\n"
	"	R:  wall origin-to-corners angle uniformity proportion\n"
	"	    0. (maximum variability) to 1. (maximum uniformity) [1.]\n"
	"	    *****************************\n"
	"	    ** Room Modification Parameters:\n"
	"	X:  room X translation factor [0]\n"
	"	Y:  room Y translation factor [0]\n"
	"	b:  room negative X scale factor [1]\n"
	"	B:  room positive X scale factor [1]\n"
	"	e:  room negative Y scale factor [1]\n"
	"	E:  room positive Y scale factor [1]\n"
	"	A:  room rotation In degrees [0]\n"
	"	M:  room scale factor [1]\n"


	"	C:  output channel 1-?; 0 = all channels[0]\n"
	"	4:  plot mode: 0 = all channels to one file,\n"
	"     	    1 = separate channel files using channel number (i.e. name.1, name.2, etc.)\n"
	"	    2 = separate channel.order files using \n"
	"	    channel and order numbers (i.e. name.1.1, name.1.2)\n"
	"	p:  sound paths plot file name\n"


	"	L: listener coordinates file\n"
	"	     (Multiple listener pairs are applied in sequence to successive channels,\n"
	"	     looping as needed. Standard use is to have one listener position for all\n"
	"	     speaker channels. Coordinates are expressed in feet.)\n"
	"	i: source coordinates file\n"
	"	     (Multiple source pairs are applied in sequence to successive channels,\n"
	"	     looping as needed. Standard use is to have one source position for all\n"
	"	     speaker channels, although multiple source positions can be specified\n"
	"	     for use with one speaker positions so as to audition, across multiple\n"
	"	     channels, several source positions relative to one speaker.\n"
	"             Coordinates are expressed in feet.)\n"
	"	o: speaker coordinates file\n"
	"	     (Multiple speaker pairs are applied in sequence to successive channels,\n"
	"	     looping as needed. Standard use is to apply single listener and source\n"
	"	     positions to a series of speakers. Coordinates are expressed in feet.)\n"
	"	S:  Speaker configuration: \n"
	"	       0 = sequence (i.e. non-enclosing array), 1 = polygon (surround) [0]\n"
	"	    ** Speaker Position Modification Parameters:\n"




	"	F:  speaker coordinates X translation factor [0]\n"
	"	g:  speaker coordinates Y translation factor [0]\n"
	"	h:  speaker coordinates negative X scale factor [1]\n"
	"	H:  speaker coordinates positive X scale factor [1]\n"
	"	I:  speaker coordinates negative Y scale factor [1]\n"
	"	j:  speaker coordinates positive Y scale factor [1]\n"
	"	J:  speaker coordinates rotation In degrees [0]\n"
	"	k:  speaker coordinates scale factor [1]\n"



	"	s:  output mix components -- 0 = both direct and reflected sound\n"
	"		1 = only direct sound, 2 = only reflected sound [0]\n"

	"	/k:  reflected sound gain in decibels [0]\n"
	"	/l:  direct sound gain in decibels [0]\n"

	"	/o:  listener space cross reflections: 0 = include, 1 = exclude [0]\n"



	"	/m:  direct sound air-absorption exponent: \n"
	"  	       amplitude of direct sound = (reference distance / source distance)**exponent\n"
	"	       Positive/negative values produce distance-correlated decrease/increase\n"
	"	       in amplitude, respectively, with severity of decrease or increase \n"
	"	       increasing with greater magnitudes. Exponent of 2 corresponds to \n"
	"	       inverse square law; value of 0 produces all amplitudes to be equal.\n"
	"	       Values between 1 and 2 generally work best. [2.]\n"

	"	/p:	air absorption exponent for real space source (i.e. sound in front\n"
	"						of speakers) [2]\n"
	"	a:  reflections air-absorption exponent: \n"
	"  	       amplitude of reflection = (reference distance / reflection distance)**exponent\n"
	"	       Positive/negative values produce distance-correlated decrease/increase\n"
	"	       in amplitude, respectively, with severity of decrease or increase \n"
	"	       increasing with greater magnitudes. Exponent of 2 corresponds to \n"
	"	       inverse square law; value of 0 produces all amplitudes to be equal.\n"
	"	       Values between 1 and 2 generally work best. [2.]\n"




	"	m:  reference distance for air absorption computation\n"
	"	       Distance, in feet, at which source or reflection will produce amplitude\n"
	"	        of 0dB. Amplitudes for shorter distances are limited to the\n"
	"	       reference distance amplitude, i.e. to 0dB. [1]\n" 


	"	l:  reflection order high limit (greatest number of wall reflections)\n"
	"	       The high reflection order determines the highest number of computed \n"
	"	       reflection surfaces included in response.\n"
	"	u:  reflection order low limit (least number of wall reflections)\n"
	"	       The low reflection order determines the least number of computed \n"
	"	       reflection surfaces included in response.\n"

	"	P: source dispersion pattern rolloff. 0dB to -90dB or less.\n"
	"	       0 = off (i.e. omnidirectional) \n"
	"	       Given the directional or forward orientation of source, rolloff determines \n"
	"	       amplitude of off-axis, non-forward radiations. Rear radiations \n"
	"	       (i.e. deviations of +/- 180 degrees) are attenuated by full rolloff;\n"
	"              lesser angles by corresponding proportion of rolloff. Rolloff is applied\n"
	"	       to both direct (i.e. source-to-speaker) and reflected (i.e. source-to-wall) \n"
	"	       radiations.[0]\n"   
	"	/q: forward orientation of non-rotated source:\n" 
	"		0 = orient to 0 degrees of polar room coordinates (i.e. source as origin)\n"
	"		1 = orient to listener (facing listener, i.e. listener as origin) [0]\n"	
	"	O: rotation of source, in degrees.[0]\n"
	"	       0 = no rotation, +/- 180 degrees = reversed orientation.\n"
	"	       Sources are rotated relative to the orientation specified in -/q.\n"
	"	       Source rotation requires directional (i.e. non-zero) dispersion rolloff.\n"
	"	       (See -P above.)\n"
	"	       ** Source and Impulse Time Position Modification Parameters:\n"
	"	q:     reflection impulses time position scaler: 1 = no change [1]\n"
//

 


	"	  ** REFLECTION ORDER IMPULSE RESPONSES **\n"
	"	8:     reflection order impulse responses switch: 0 = off, 1 = on [0]\n"

	"	9:     reflection order impulse response sound file name\n"
	"	          Channels of audio sound file are used as the impulse responses for\n"
	"	          reflection order. Application of impulse responses are handled\n"
	"	          according to the -? parameter. If no file is given, then single impulses,\n"
	"	          are used, modified in amplitude in accord with  their distance, \n"
	"                 orientation to the source, and reflection order response filter settings.\n"  
	"	/b:  reflection order impulse response convolution mode [-1]\n"
	"	V:  reflection order channel assignments file (1-?)\n"
	"	/a:  reflection order gainscale factors file (in dB)\n"
	"	/n: reflection order impulse response presence levels file (in dB)\n"

	"	/i:   reflection order impulse response truncation envelope file\n"
	"	/h:   reflection order impulse response truncate duration\n"
	"	  ** REFLECTION ORDER IMPULSE RESPONSE FILTER **\n"
	"	/c:   Filter Mode: 0 = off, 1 = pre-filter input response, \n"
	"	        2 = compounded filtering of convolution outputs, 3 = both modes 1 and 2.\n"
	"	/d:   reflection order impulse response filter low rolloff frequency [?]\n"
	"	/e:   reflection order impulse response filter high rolloff frequency [?]\n"
	"	/f:   reflection order impulse response filter low amplitude rolloff per octave in dB [?]\n"
	"	/g:   reflection order impulse response filter high amplitude rolloff per octave in dB [?]\n"
	"	   ** Reflection Order Impulse Response Window Sample Mode **\n"
	"	Q:   reflection order impulse response window sample mode: \n"
        "               0 = off, 1 = on, 2 = on with sync [0]\n"
 	"	~: low order attack duration \n"	 
 	"	=: low order release duration \n"	 
 	"	@: high order attack duration \n"	 
 	"	/v: high order release duration \n"	 
	"	:: envelope shape index \n"
	"	0:  Substitute delay time air absorption amplitude level with \n"
	"	    peak amplitude from sampled impulse response window range\n"
	"	     0 = off, 1 = on [0]\n"
//
	"	  ** WALL IMPULSE RESPONSES **\n"
	"	5:     wall impulse responses switch: 0 = off, 1 = on [0]\n"

	"	K:     wall impulse response sound file name\n"
	"	          Channels of audio sound file are used as the impulse responses of the walls\n"
	"	          of the virtual space. Application of impulse responses are handled\n"
	"	          according to the -? parameter. If no file is given, then single impulses,\n"
	"	          are used, modified in amplitude in accord with  their distance, \n"
	"                 orientation to the source, and wall response filter settings.\n"  
	"	t:  wall impulse response convolution mode [-1]\n"
	"	1:  wall channel assignments file (1-?)\n"
	"	2:  wall gainscale factors file (in dB)\n"
	"	/j: wall impulse response presence levels file (in dB)\n"

	"	7:   wall impulse response truncation envelope file\n"
	"	6:   wall impulse response truncate duration\n"
	"	  ** WALL IMPULSE RESPONSE FILTER **\n"
	"	T:   Filter Mode: 0 = off, 1 = pre-filter input response, \n"
	"	        2 = compounded filtering of convolution outputs, 3 = both modes 1 and 2.\n"
	"	x:   wall impulse response filter low rolloff frequency [?]\n"
	"	y:   wall impulse response filter high rolloff frequency [?]\n"
	"	z:   wall impulse response filter low amplitude rolloff per octave in dB [?]\n"
	"	Z:   wall impulse response filter high amplitude rolloff per octave in dB [?]\n"


	"	/t:	source minimum distance from listener \n"
	"	/u:	use collapsed threshold amplitudes  0 = no, 1 = yes [0]\n"
	"	3:	impulse inclusion threshold in dB [-96]\n"
	"	U:	impulse end truncation threshold in dB [-96]\n"
	"	v:	impulse end truncation release time in seconds [0]\n"


	"	G: master gain in decibels (pre-normalization) [0]\n"
	"	N: normalization:                [1]\n"
	"	   0: Do not normalize. \n"
	"	   1: Normalize channels independently.\n"
	"	   2: Normalize channels together against channel with peak amplitude.\n"
	"	   3: Normalize channels together if any channel exceeds 0 decibels.\n"


	"	_:	 "AUTO_PLAY		// autoplayreps



	);
	exit(EXIT_SUCCESS);
}


void pd( int i ){ 
 if( debugFlagRoomResponse ) fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; 
} 

float piToDegrees( float v){  return( 360.0 * v / TWOPI ) ;   } ;
float degreesToPi( float v){  return( TWOPI * v / 360.0 ) ;   } ;

void makeCorneredSpace(){
   float sum=0. ;
   float radius ;
   int corner ;
   float x, y ;    


      
   for( corner = 0; corner < numberOfCorners ; corner++ ){
      angles[ corner ] = TWOPI / (float) numberOfCorners ;
      randomProportions[ corner ] = randf( 0., 1.); 
      sum += randomProportions[ corner ] ;
   } ;

/*
for( corner = 0 ; corner < numberOfCorners; corner++ )
   fprintf( stderr, "\nCORNER: %d ANGLE: %f", corner, angles[ corner ] ) ;
*/
   // NORMALIZE SUM RANDOM VALUES AND SCALE BY NUMBER OF CORNERS.
   for( corner = 0; corner < numberOfCorners ; corner++ ){
      randomProportions[ corner ] = (randomProportions[ corner ] / sum) * 
         (1.0 - polygonAngleRegularityProportion) * (float) numberOfCorners ;
   } ;   
/*
for( corner = 0 ; corner < numberOfCorners; corner++ )
   fprintf( stderr, "\nCORNER: %d RANDOM PROPORTIONS: %f", corner, randomProportions[ corner ] ) ;
*/
   // MIX REGULAR WITH IRREGULAR 
   for( corner = 0; corner < numberOfCorners ; corner++ ){
      angles[ corner ] =  
         (polygonAngleRegularityProportion * angles[ corner ]) + 
         ( angles[ corner ] * randomProportions[ corner ]  ) ;
   } ;
/*
for( corner = 0 ; corner < numberOfCorners; corner++ )
   fprintf( stderr, "\nCORNER: %d ANGLES MIXED: %f", corner, angles[ corner ] ) ;
*/

   // INTEGRATE TO FIND CORNER ANGLE POSITIONS
   originToCornerAngles[ 0 ] = 0. ;
   for( corner = 1; corner < numberOfCorners ; corner++ ){
      originToCornerAngles[ corner ] = 
      originToCornerAngles[corner - 1] + angles[corner - 1] ; 
   }; 
/*
for( corner = 0 ; corner < numberOfCorners; corner++ )
   fprintf( stderr, "\nCORNER: %d ORIGIN TO CORNER ANGLE: %f", 
      corner, originToCornerAngles[ corner ] ) ;
*/

   // ROTATE
   for( corner = 0; corner < numberOfCorners ; corner++ ){
      originToCornerAngles[ corner ] += degreesToPi( rotationOfSyntheticRoomInDegrees ) ;
   } ;
/*
for( corner = 0 ; corner < numberOfCorners; corner++ )
   fprintf( stderr, "\nCORNER: %d ORIGIN TO CORNER ANGLE ROTATED: %f", 
      corner, originToCornerAngles[ corner ] ) ;
*/
   

   for( corner = 0; corner < numberOfCorners ; corner++ ){
           radius = randf( minMaxDistanceToCornerFromOrigin[ 0 ], 
         minMaxDistanceToCornerFromOrigin[ 1 ] ) ; 
      originToCornerDistances[ corner ] = radius ; 
      y = sin( (double) originToCornerAngles[ corner ] ) * radius ;
           x = cos( (double) originToCornerAngles[ corner ] ) * radius  ;
           coordinatesOfCorners[ corner * 2 ] = x ;
           coordinatesOfCorners[ (corner * 2) + 1 ] = y ; 
   }; 

/*
   if( writeToFile ){
      for( corner = 0; corner <= numberOfCorners ; corner++ ){
         fprintf( filePointer, 
              "\n%f %f", 
            coordinatesOfCorners[ ((corner % numberOfCorners) * 2) + 0 ], 
               coordinatesOfCorners[ ((corner % numberOfCorners) * 2) + 1 ] ) ;
      } ;
   };
*/
} ;   


void mirrorPointAroundLineSegment( 
   double point[2], 
   double line[4],
   double mirrorPoint[2]
){

   double outX, outY ;
   double lineSlope, interceptY ;
   double outLineSlope, outInterceptY;
   double mirrorLineIntersectionX, mirrorLineIntersectionY ; 

   int l0, l1, l2, l3 ;

   l0 = 0 ; // [0][0]
   l1 = 1 ; // [0][1]
   l2 = 2 ; // [1][0]
   l3 = 3 ; // [1][1]


   // 
   if( fabs(line[0] - line[ 2 ]) < 0.000000001 )
   {


      // VERTICAL
      outX = line[ 0 ] + (line[0] - point[ 0 ]) ; 
      outY = point[ 1 ] ; 
   } else {


          // NOT VERTICAL
      if( fabs(line[ 1 ] - line[ 3 ]) < 0.000000001 )
      {
         // HORIZONTAL

         outY = line[ 1 ] + (line[ 1 ] - point[ 1 ]) ; 
         outX = point[ 0 ] ; 
      } else {

         // DIAGONAL
         // FIND PERPENDICULAR INTERSECT OF POINT TO LINE
         lineSlope = (line[ 3 ] - line[ 1 ]) / (line[ 2 ] - line[0]) ;  

         interceptY = line[ 1 ] - ( lineSlope * line[0] ) ;

         outLineSlope = (-1.0) / lineSlope ; 

         outInterceptY = point[ 1 ] - ( outLineSlope * point[ 0 ] )  ;

         // y = mx + b ; 
         // y = lineSlope x + interceptY ;
         // y = outLineSlope x + outInterceptY ;
         // lineSlope x + interceptY = outLineSlope x + outInterceptY
         // lineSlope x - outLineSlope x = outInterceptY - interceptY
         // x(lineSlope - outLineSlope) = outInterceptY - interceptY

         // LINE INTERSECTION POINT
         mirrorLineIntersectionX = (outInterceptY - interceptY) / (lineSlope - outLineSlope) ;
         mirrorLineIntersectionY = (outLineSlope  * mirrorLineIntersectionX) + outInterceptY ;          
         outX = mirrorLineIntersectionX + (mirrorLineIntersectionX - point[ 0 ]) ;
         outY = mirrorLineIntersectionY + (mirrorLineIntersectionY - point[ 1 ]) ;

      }; 
            
   }; 

   mirrorPoint[ 0 ] = outX ; mirrorPoint[ 1 ] = outY ;   
} ;


bool examineSegmentsForIntersection(
   float w[4], 
   float p[4],
   float intersectCoordinates[2],
   bool printFlag
){
   // w and p are line seg arrays of two points containing x and y.

   float x, y ;
   bool segmentsIntersect ;
   float mw, mp, bw, bp;
   float  wxlow, wxhigh, wylow, wyhigh, pxlow, pxhigh, pylow, pyhigh  ;
   
   if( (w[ 2 ] - w[ 0 ]) == 0.0 ){
      // w SEGMENT PARALLEL TO Y-AXIS
      if( (p[ 2 ] - p[ 0 ]) == 0.0 ){
         // BOTH p AND w SEGMENT PARALLEL TO Y-AXIS
//         x = nil ; y = nil ; 
         segmentsIntersect = false ; 
      } else {
         // w PARALLEL, p SEGMENT NOT PARALLEL.
         mp = (p[ 3 ] - p[ 1 ]) / (p[ 2 ] - p[ 0 ]) ; bp = p[ 1 ] - (mp * p[ 0 ]) ;
         x = w[ 0 ] ; y = (mp * x) + bp ; 
         if( (  ( (y >= w[ 1 ]) && (y <= w[ 3 ]) ) || ( (y >= w[ 3 ]) && (y <= w[ 1 ]) )  )
               &&
            (  ( (x >= p[ 0 ]) && (x <= p[ 2 ]) ) || ( (x >= p[ 2 ]) && (x <= p[ 0 ]) )   ) )
         { segmentsIntersect = true ; } else { segmentsIntersect = false ; } ;
      } ; 
   } else {
      // w SEGMENT NOT PARALLEL
      mw = (w[ 3 ] - w[ 1 ]) / (w[ 2 ] - w[ 0 ]) ; bw = w[ 1 ] - (mw * w[ 0 ]) ;
      if( (p[ 2 ] - p[ 0 ]) == 0.0 ){
         // w SEGMENT NOT PARALLEL, p SEGMENT PARALLEL
         x = p[ 0 ] ; y = (mw * x) + bw ; 
         if( (  ( (y >= p[ 1 ]) && (y <= p[ 3 ]) ) || ( (y >= p[ 3 ]) && (y <= p[ 1 ]) )  )
               &&
            (  ( (x >= w[ 0 ]) && (x <= w[ 2 ]) ) || ( (x >= w[ 2 ]) && (x <= w[ 0 ]) )   ) )
         {segmentsIntersect = true ; } else { segmentsIntersect = false ; } ;
      } else {
         // NEITHER SEGMENTS PARALLEL TO Y-AXIS
         mp = (p[ 3 ] - p[ 1 ]) / (p[ 2 ] - p[ 0 ]) ; bp = p[ 1 ] - (mp * p[ 0 ]) ;
         if( mp == mw ){
            // w AND p PARALLEL TO EACH OTHER
//            x = nil ; y = nil ; 
            segmentsIntersect = false ; 
         } else {
            // NO PARALLELISMS OF ANY KIND
            x =  (bw - bp) / (mp - mw) ; y = (mp * x) + bp ; 
            // FIND LOWS AND HIGHS FOR w AND p
            if( w[ 0 ] <= w[ 2 ] ){
               wxlow = w[ 0 ]; wxhigh = w[ 2 ] ;
            } else {
               wxlow = w[ 2 ]; wxhigh = w[ 0 ] ;
            }; 
            if( w[ 1 ] <= w[ 3 ] ){
               wylow = w[ 1 ]; wyhigh = w[ 3 ] ;
            } else {
               wylow = w[ 3 ]; wyhigh = w[ 1 ] ;
            }; 

            if( p[ 0 ] <= p[ 2 ] ){
               pxlow = p[ 0 ]; pxhigh = p[ 2 ] ;
            } else {
               pxlow = p[ 2 ]; pxhigh = p[ 0 ] ;
            } ; 
            if( p[ 1 ] <= p[ 3 ] ){
               pylow = p[ 1 ]; pyhigh = p[ 3 ] ;
            } else {
               pylow = p[ 3 ]; pyhigh = p[ 1 ] ;
            } ; 

            if( 
               (x >= wxlow) && (x <= wxhigh) && 
               (y >= wylow) && (y <= wyhigh) &&
               (x >= pxlow) && (x <= pxhigh) && 
               (y >= pylow) && (y <= pyhigh)
            ){segmentsIntersect = true ; } else {segmentsIntersect = false ; };            
         }; 
      };    
   }; 
   intersectCoordinates[ 0 ] = x ; intersectCoordinates[ 1 ] = y ; 
   
   if( printFlag )
   {
      if( segmentsIntersect ) 
      {
         prt( "SEGMENTS INTERSECT" ) ; 
         printCoordinates( intersectCoordinates, "intersectCoordinates"  ); 
      } else
      {
         prt( "SEGMENTS DO NOT INTERSECT" ) ;
      } ;
   } ;

   return( segmentsIntersect ) ; 
} ;



void mirrorPolygonCoordinatesAroundAllSides(
   int outputFileChannelNumber,
   int thisOrder,  // SET TO 1 WITH FIRST LEVEL CALL
   int polygonMirrorSideIndex, // SET TO -1 FOR FIRST CALL
   float mirrorSegAngleLimitsLow,
   float mirrorSegAngleLimitsHigh
){
   
   float mirrorSegCoordinates[ 4 ] ;
   float mirrorSegCoordinatesAngles[ 2 ] ;
   float reflectionSegmentCoordinates[ 4 ] ;
   float intersectCoordinates[ 2 ] ;
   bool reflectionSegmentTest ;
   bool reflectionSegmentInsidePolygonWallTest ;
   bool intersectionTestOutput ;
   int viableReflectionsForThisOrder=0 ;
   

   bool mirrorSegmentTest ;
   float mirrorSegAngleLimitsTempLow ;
   float mirrorSegAngleLimitsTempHigh ;
   float newMirrorSegAngleLimitsLow ;
   float newMirrorSegAngleLimitsHigh ;

   int mirrorSide, segIndex, corner ;
   float temp, angleAdd, distance ;
   int orderIndex, i, n, k, thisBaseIndex ;
   
   int polygonCoordinatesBaseIndex ;
   int mirroredPolygonCoordinatesBaseIndex ;
   int thisOrderMinusOne ;
   
   int side, previousMirrorSegmentWallToSkip, nextMirrorSegmentWallToSkip ; 
   float testSegment[4] ;
   int higher, lower ; 
   float intersectPointToListenerSegment[ 4 ] ; 
   float intersectPointToSpeakerSegment[ 4 ] ;
   float intersectPointToListenerSegmentLength ; 
   float intersectPointToSpeakerSegmentLength ;


   thisOrderMinusOne = thisOrder - 1 ; 
   polygonCoordinatesBaseIndex = thisOrderMinusOne * (numberOfCorners * 2) ;
   mirroredPolygonCoordinatesBaseIndex = thisOrder * (numberOfCorners * 2) ;


   //pri( thisOrder, "\n\n ********** ENTERED ROUTINE: ORDER LEVEL" ) ; 



   if( thisOrder > highOrderLimit ) return; 

   // MIRROR EVERY SIDE
   for( mirrorSide = 0; mirrorSide < numberOfWalls; mirrorSide++ )
   { 
   //pri( mirrorSide, "\n\nLOOP BEGIN - mirrorSide" ) ;  

/*
      // EXTRACT MIRROR SEGMENT FOR THIS SIDE
      mirrorSegCoordinates[ 0 ] = 
      polygonCoordinates[ polygonCoordinatesBaseIndex + (mirrorSide * 2) ] ;
      mirrorSegCoordinates[ 1 ] = 
      polygonCoordinates[ polygonCoordinatesBaseIndex + ((mirrorSide * 2) + 1) ] ;
      mirrorSegCoordinates[ 2 ] = 
      polygonCoordinates[ polygonCoordinatesBaseIndex + 
         (((mirrorSide + 1) % numberOfWalls) * 2) ] ;
      mirrorSegCoordinates[ 3 ] = 
      polygonCoordinates[ polygonCoordinatesBaseIndex + 
         ((((mirrorSide + 1) % numberOfWalls) * 2) + 1) ] ;
*/

      if( thisOrder > 1 ){ // ???
         n = (thisOrder - 2) * 4 ; 
      } ; 

      // DON'T REMIRROR BY THE PREVIOUS MIRROR SEGMENT.
      // SET BOOLEAN FLAG TO TRUE IF THIS MIRROR SEG IS THE SAME AS THE LAST ONE.

//pri( polygonMirrorSideIndex, "polygonMirrorSideIndex" ) ; 
//pri( mirrorSide, "mirrorSide" ) ; 

      if( mirrorSide == polygonMirrorSideIndex ){
         // SAME AS LAST
      } else {   


         // EXTRACT MIRROR SEGMENT FOR THIS SIDE
         mirrorSegCoordinates[ 0 ] = 
         polygonCoordinates[ polygonCoordinatesBaseIndex + (mirrorSide * 2) ] ;
         mirrorSegCoordinates[ 1 ] = 
         polygonCoordinates[ polygonCoordinatesBaseIndex + ((mirrorSide * 2) + 1) ] ;
         mirrorSegCoordinates[ 2 ] = 
         polygonCoordinates[ polygonCoordinatesBaseIndex + 
            (((mirrorSide + 1) % numberOfWalls) * 2) ] ;
         mirrorSegCoordinates[ 3 ] = 
         polygonCoordinates[ polygonCoordinatesBaseIndex + 
            ((((mirrorSide + 1) % numberOfWalls) * 2) + 1) ] ;

         // DON'T PROCEED THROUGH MIRROR SEGMENT IF
         // NEITHER OF ITS ENDS MAKES A SEGMENT WITH THE SPEAKER 
         // THAT GOES BETWEEN THE PREVIOUS ANGLE SPAN.

         // FIND THE ANGLE FOR THE TWO SEGMENTS FORMED FROM EACH
         // OF THE  mirrorSegCoordinates ENDS AND THE SPEAKER POSITION. 

         mirrorSegCoordinatesAngles[ 0 ] = 
            atan2( 
               mirrorSegCoordinates[ 1 ] - speakerCoordinatesNow[ 1 ], // y
               mirrorSegCoordinates[ 0 ] - speakerCoordinatesNow[ 0 ]  // x
            ) ; 

         mirrorSegCoordinatesAngles[ 1 ] =      
            atan2( 
               mirrorSegCoordinates[ 3 ] - speakerCoordinatesNow[ 1 ], // y
               mirrorSegCoordinates[ 2 ] - speakerCoordinatesNow[ 0 ]  // x
            ) ; 


         if( mirrorSegCoordinatesAngles[ 0 ] > mirrorSegCoordinatesAngles[ 1 ] ){
               // SORT (SWITCH)
            temp = mirrorSegCoordinatesAngles[ 1 ] ;
               mirrorSegCoordinatesAngles[ 1 ] = mirrorSegCoordinatesAngles[ 0 ] ;
               mirrorSegCoordinatesAngles[ 0 ] = temp ; 
         } ;
         

         if( thisOrder == 1)
         {
            // FIRST ONE

            mirrorSegmentTest = true ;
            angleAdd = 0.0 ; 
         } else {
               // TEST MIRROR SEGMENT END ANGLES AGAINST CURRENT LIMITS 

            mirrorSegmentTest = false ;
            if( (copysign( 1., mirrorSegAngleLimitsLow ) == 
               copysign( 1., mirrorSegAngleLimitsHigh ) ) ||
                  (fabs( (double)(mirrorSegAngleLimitsHigh - mirrorSegAngleLimitsLow) ) < PI) 
            ){
               // SAME SIGN OR STRADDLING ZERO
               mirrorSegCoordinatesAnglesTemp[ 0 ] = mirrorSegCoordinatesAngles[ 0 ] ;
               mirrorSegCoordinatesAnglesTemp[ 1 ] = mirrorSegCoordinatesAngles[ 1 ] ;
               mirrorSegAngleLimitsTempLow = mirrorSegAngleLimitsLow ; 
               mirrorSegAngleLimitsTempHigh = mirrorSegAngleLimitsHigh ; 
            } else {
               // STRADDLES pi/-pi
               mirrorSegCoordinatesAnglesTemp[ 0 ] = mirrorSegCoordinatesAngles[ 0 ] ; 
               mirrorSegCoordinatesAnglesTemp[ 1 ] = mirrorSegCoordinatesAngles[ 1 ] ; 
               if( mirrorSegCoordinatesAnglesTemp[ 0 ] < 0.0)
               { mirrorSegCoordinatesAnglesTemp[ 0 ] += TWOPI  ; } ; 
               if( mirrorSegCoordinatesAnglesTemp[ 1 ] < 0.0)
               { mirrorSegCoordinatesAnglesTemp[ 1 ] += TWOPI ; }; 

               // SORT
               if( mirrorSegCoordinatesAnglesTemp[ 0 ] > mirrorSegCoordinatesAnglesTemp[ 1 ] )
               {
                  temp = mirrorSegCoordinatesAnglesTemp[ 0 ] ;
                  mirrorSegCoordinatesAnglesTemp[ 0 ] = mirrorSegCoordinatesAnglesTemp[ 1 ] ;
                  mirrorSegCoordinatesAnglesTemp[ 1 ] = temp ;
               };

               // COPY
               mirrorSegAngleLimitsTempLow = mirrorSegAngleLimitsLow ; 
               mirrorSegAngleLimitsTempHigh = mirrorSegAngleLimitsHigh ; 

               // ROTATE NEGATIVE TO POSITIVE
               if( mirrorSegAngleLimitsLow < 0.0 ){
                  mirrorSegAngleLimitsLow += TWOPI  ; }; 
               if( mirrorSegAngleLimitsLow < 0.0 ){
                  mirrorSegAngleLimitsHigh += TWOPI ; }; 

               if( mirrorSegAngleLimitsLow > mirrorSegAngleLimitsHigh ){
                  temp = mirrorSegAngleLimitsLow ;
               mirrorSegAngleLimitsLow = mirrorSegAngleLimitsHigh ;
            mirrorSegAngleLimitsLow = temp ;  
               } ;
            } ; 
            
            mirrorSegAngleLimitsTempLow += ((-1.0 * PI) / 100.0) ;
            mirrorSegAngleLimitsTempHigh += (PI / 100.0) ;

         if( 
            ( mirrorSegCoordinatesAnglesTemp[ 0 ] >= mirrorSegAngleLimitsTempLow ) && 
            ( mirrorSegCoordinatesAnglesTemp[ 0 ] <= mirrorSegAngleLimitsTempHigh )
         ) { mirrorSegmentTest = true ; } ;  
            if( 
               ( mirrorSegCoordinatesAnglesTemp[ 1 ] >= mirrorSegAngleLimitsTempLow ) && 
               ( mirrorSegCoordinatesAnglesTemp[ 1 ] <= mirrorSegAngleLimitsTempHigh )
            ) { mirrorSegmentTest = true ; } ;  

            if( (mirrorSegCoordinatesAnglesTemp[ 0 ] < mirrorSegAngleLimitsTempLow) &&
               (mirrorSegCoordinatesAnglesTemp[ 1 ] > mirrorSegAngleLimitsTempHigh)
            ) { mirrorSegmentTest = true ; } ; 
            

         }; 

         // $$$
         if( 
            mirrorSegmentTest // TEST OF MIRROR SEGMENT
         ){ 
         
            // MAKE MIRRORED POLYGON AROUND MIRROR SEGMENT
            for(i = 0; i < 4; i++) lineSeg[ i ] = (double) mirrorSegCoordinates[ i ] ;
            for( corner = 0; corner < numberOfCorners; corner++)
            {

               point[ 0 ] = 
                  (double) polygonCoordinates[polygonCoordinatesBaseIndex + (corner * 2)] ; 
               point[ 1 ] = 
                  (double) polygonCoordinates[polygonCoordinatesBaseIndex + ((corner * 2) + 1)] ;

//               for(i = 0; i < 4; i++) lineSeg[ i ] = (double) mirrorSegCoordinates[ i ] ;

               mirrorPointAroundLineSegment( point, lineSeg, mirrorPoint ) ;


               polygonCoordinates[ mirroredPolygonCoordinatesBaseIndex + (corner * 2) ] =
                    (float) mirrorPoint[ 0 ] ;
               polygonCoordinates[ mirroredPolygonCoordinatesBaseIndex + ((corner * 2) + 1) ] =
                    (float) mirrorPoint[ 1 ] ;
            }; 


             // MAKE MIRRORED SOURCE COORDINATES
           point[ 0 ] = (double) sourceCoordinatesForThisPolygon[ (2 * thisOrderMinusOne)] ;
           point[ 1 ] = (double) sourceCoordinatesForThisPolygon[ (2 * thisOrderMinusOne) + 1] ;
           for(i = 0; i < 4; i++) lineSeg[ i ] = (double) mirrorSegCoordinates[ i ] ;

            mirrorPointAroundLineSegment(
               point, // point
               lineSeg, // line
               mirrorPoint // mirrorPoint
            ) ;

            sourceCoordinatesForThisPolygon[ (2 * thisOrder)] = (float) mirrorPoint[ 0 ] ; 
            sourceCoordinatesForThisPolygon[ (2 * thisOrder) + 1] = (float) mirrorPoint[ 1 ] ; 
//            printCoordinates( 
//               &sourceCoordinatesForThisPolygon[ (2 * thisOrder)], "MIRRORED SOURCE POINT" ) ; 

      
            // TEST MIRRORED SOURCE FOR VALID REFLECTION PATH.
            // PATH FROM MIRRORED SOURCE TO SPEAKER SHOULD INTERSECT
            // EVERY MIRROR SEGMENT.
            reflectionSegmentCoordinates[ 0 ] = speakerCoordinatesNow[ 0 ] ;
            reflectionSegmentCoordinates[ 1 ] = speakerCoordinatesNow[ 1 ] ;
            reflectionSegmentCoordinates[ 2 ] = 
            sourceCoordinatesForThisPolygon[ (2 * thisOrder) + 0] ;
            reflectionSegmentCoordinates[ 3 ] = 
               sourceCoordinatesForThisPolygon[ (2 * thisOrder) + 1] ;


            for( segIndex = 0; segIndex < 4; segIndex++ ) 
               mirrorSegments[ (thisOrderMinusOne * 4) + segIndex ] =
                mirrorSegCoordinates[ segIndex ] ;

            mirrorSideNumber[ thisOrderMinusOne ] = mirrorSide ; 



            // TESTING . . . ???
            reflectionSegmentTest = true ; 


            //prt("MIRROR SIDE NUMBERS NOW:" ) ; 
            //for( orderIndex = 0; orderIndex < thisOrder; orderIndex++ )
            //   fprintf( stderr, "\n\t%d: %d", orderIndex, mirrorSideNumber[ orderIndex ] ) ; 
 
            // TESTING LOOP FOR MIRROR SEGMENTS AND MIRROR POLYGON SIDES. 
            //prt( "TESTING LOOP FOR MIRROR SEGMENTS AND MIRROR POLYGON SIDES." ) ; 
            for( orderIndex = 0; orderIndex < thisOrder; orderIndex++ )
            {
               // TEST FOR INTERSECTION OF REFLECTION SEGMENT WITH MIRROR SEGMENTS:
               // **SHOULD** BE TRUE
//               printSegment( &mirrorSegments[ orderIndex * 4], 
//                 "MIRROR SEGMENT BEING TESTED FOR INTERSECTION" ) ; 
               intersectionTestOutput = examineSegmentsForIntersection(
                  &mirrorSegments[ orderIndex * 4], // w: 
                  reflectionSegmentCoordinates, // p:
                  intersectCoordinates, // intersectCoordinates
                  false // printFlag
               );
               mirrorSegmentIntersectCoordinates[ orderIndex * 2 ] =  intersectCoordinates[ 0 ] ;
               mirrorSegmentIntersectCoordinates[ (orderIndex * 2) + 1 ] =  intersectCoordinates[ 1 ] ;
               if( intersectionTestOutput == false )
               { 
                  //prt( "MIRROR SEGMENT INTERSECTION TEST: FAIL." ) ;  
                  reflectionSegmentTest = false ; break ; 
               };
               //prt( "MIRROR SEGMENT INTERSECTION TEST: ** PASS **." ) ;         

               // IF FIRST TEST PASSED, THEN
               // ADD SHORTER-DISTANCE-TO-LISTENER-FROM-LAST-WALL TEST
               // IF DESIRED. TO DO THIS, FIND DISTANCES FROM THE MIRROR-SEGMENT-INTERSECTION
               // TO THE SPEAKER AND TO THE LISTENER. IF THE SPEAKER IS LONGER, THEN
               // THE REFLECTION IS INVALID. 
               if( reflectionSegmentTest && (orderIndex == 0) &&
                  (listener_space_cross_reflections__include_0__exclude_1 == 1) )
               {
             
                  // FIND DISTANCES
                  makeSegment(
                   intersectPointToSpeakerSegment,
                   intersectCoordinates,
                   speakerCoordinatesNow
                  ) ;
                  makeSegment(
                   intersectPointToListenerSegment,
                   intersectCoordinates,
                   listenerCoordinates
                  ) ;

                  intersectPointToSpeakerSegmentLength = 
                     findSegmentLength( intersectPointToSpeakerSegment ) ; 
                  intersectPointToListenerSegmentLength = 
                     findSegmentLength( intersectPointToListenerSegment ) ; 
//                  prf( intersectPointToSpeakerSegmentLength, "intersectPointToSpeakerSegmentLength" ) ; 
//                  prf( intersectPointToListenerSegmentLength, "intersectPointToListenerSegmentLength" ) ; 

                  if( intersectPointToSpeakerSegmentLength > intersectPointToListenerSegmentLength )
                  {
//                     prt( "OMITTING REFLECTION FOR LISTENER PROXIMITY CONDITION" ) ; 
                     reflectionSegmentTest = false ; break ;
                  } ;

                 
              
               } ; 


 
               // TEST FOR ADJACENT, REFLEX-ANGLED MIRROR SEGMENTS.
               if( (orderIndex > 0) && (orderIndex < thisOrder) ){
                  if( mirrorSideNumber[ orderIndex ] > mirrorSideNumber[ orderIndex - 1 ] )
                  {
                     higher = mirrorSideNumber[ orderIndex ] ;
                     lower = mirrorSideNumber[ orderIndex - 1 ] ;

                  }else
                  {
                     lower = mirrorSideNumber[ orderIndex ] ;
                     higher = mirrorSideNumber[ orderIndex - 1 ] ;
                  } ;

                  if( (higher - lower) == 1 )
                  {
                     if( polygonReflexVertexAngleFlags[ higher ] == -1 )
                     {
//                        pri( higher, "REFLEX TRUE FOR higher" ) ; 
//                        pri( lower, "REFLEX TRUE FOR lower" ) ; 
                        reflectionSegmentTest = false ;  break ; 
                     } ;
                  }else if( (higher == (numberOfWalls - 1)) && (lower == 0))
                  {
                     if( polygonReflexVertexAngleFlags[ lower ] == -1 )
                     {
//                        pri( higher, "REFLEX TRUE FOR higher" ) ; 
//                        pri( lower, "REFLEX TRUE FOR lower" ) ; 
                        reflectionSegmentTest = false ;  break ;
                     } ;
                  } ; 
               } ;
            } ;

//if( reflectionSegmentTest ) prt( " ==================> TEST OF MIRROR SEGMENT INTERSECTION PASSED" ) ; 
//else prt( " ==================> TEST OF MIRROR SEGMENT INTERSECTION FAILED" ) ;

            // ***********************************************************
            // ******* CONCAVE TEST
            // ***********************************************************

            if( reflectionSegmentTest && polygonIsConcave )
//            if( reflectionSegmentTest )
            {

               // CONTAINMENT **
               // TEST FIRST REAL POLYGON SPACE FOR INTERSECTION WITH SEGMENTS OTHER THAN
               // MIRROR SEGMENT FOR *NEXT/FIRST* MIRROR POLYGON
               reflectionSegmentInsidePolygonWallTest = true ; 

               previousMirrorSegmentWallToSkip = -1 ; 
               nextMirrorSegmentWallToSkip = mirrorSideNumber[ 0 ] ;


               if( CHECK ) 
               {
//                  printPolygonCoordinates( "** FIRST REAL POLYGON TO TEST", 
//                    &polygonCoordinates[ 0 ], numberOfWalls );
                  //pri( previousMirrorSegmentWallToSkip, "REAL POLYGON: previousMirrorSegmentWallToSkip" ) ; 
                  //pri( nextMirrorSegmentWallToSkip, "REAL POLYGON: nextMirrorSegmentWallToSkip" ) ; 
               } ;

               if( CHECK )for(side = 0; side < numberOfWalls; side++)
               { 
                  if( (side != previousMirrorSegmentWallToSkip) && 
                      (side != nextMirrorSegmentWallToSkip)    
                  )
                  {
//pri( side, "THE FIRST REAL POLYGON: SIDE OF THIS POLYGON  BEING CHECKED" ) ; 
                     makeSegment( 
                       testSegment, 
                       &polygonCoordinates[ (side * 2) ], 
                       &polygonCoordinates[ (( (side + 1) % numberOfWalls) * 2) ] 
                     ); 
//printSegment( testSegment, "POLYGON SEGMENT TO TEST" ) ; 
//printSegment( reflectionSegmentCoordinates, "AGAINST REFLECTION SEGMENT" ) ; 

                     intersectionTestOutput = examineSegmentsForIntersection(
                        testSegment, // w: 
                        reflectionSegmentCoordinates, // p:
                        intersectCoordinates, // intersectCoordinates
                        false // printFlag
                     );
//printCoordinates(  intersectCoordinates, "INTERSECTION COORDINATES" ) ; 
                     if( intersectionTestOutput == true ) reflectionSegmentInsidePolygonWallTest = false ;
//prb( intersectionTestOutput, "SEGMENT INTERSECTION" ) ; 
                  } ;
               } ;

/*
if( reflectionSegmentInsidePolygonWallTest ) 
prt( " -----------------------> TEST OF REAL POLYGON WALL BLOCKAGES PASSED" ) ; 
else 
prt( "------------------------> TEST OF REAL POLYGON WALL BLOCKAGES FAILED" ) ;
*/


               // ***** LOOP FOR TESTING OF PATH CONTAINMENT WITHIN CROSSED MIRROR POLYGONS. 
               for( orderIndex = 0; orderIndex < thisOrder; orderIndex++ )
               {

                  previousMirrorSegmentWallToSkip = mirrorSideNumber[ orderIndex ];

                  // TEST FOR INTERSECTION WITH NON MIRROR-SEGMENT, MIRROR-POLYGON SIDES. 
                  // ** SHOULD BE FALSE **
                  if( orderIndex == (thisOrder - 1) ) nextMirrorSegmentWallToSkip = -1 ;
                  else nextMirrorSegmentWallToSkip = mirrorSideNumber[ orderIndex + 1 ] ;

                  thisBaseIndex = (orderIndex + 1) * (numberOfCorners * 2) ;
                  //pri( thisBaseIndex, "thisBaseIndex" ) ; 

//printPolygonCoordinates( "MIRROR POLYGON", &polygonCoordinates[ thisBaseIndex ], numberOfWalls ); 
//pri( previousMirrorSegmentWallToSkip, "MIRROR POLYGON: previousMirrorSegmentWallToSkip" ) ; 
//pri( nextMirrorSegmentWallToSkip, "MIRROR POLYGON: nextMirrorSegmentWallToSkip" ) ; 


                  //pri( orderIndex, "orderIndex" ) ; 
//                  if( CHECK ) printPolygonCoordinates( "** MIRROR POLYGON TO TEST", 
//                    &polygonCoordinates[ thisBaseIndex ], numberOfWalls );

                  if( CHECK ) for(side = 0; side < numberOfWalls; side++)
                  { 

                     if( (side != previousMirrorSegmentWallToSkip) && 
                         (side != nextMirrorSegmentWallToSkip) 
                     )
                     {
//pri( side, "MIRROR POLYGON: SIDE BEING CHECKED" ) ; 

                        makeSegment( 
                          testSegment, 
                          &polygonCoordinates[ thisBaseIndex + (side * 2)], 
                          &polygonCoordinates[ thisBaseIndex + (( (side + 1) % numberOfWalls) * 2)] 
                        ); 
//printSegment( testSegment, "SEGMENT TEST OF MIRROR POLYGON SIDE" ) ; 
//printSegment( reflectionSegmentCoordinates, "REFLECTION SEGMENT" ) ; 
                        intersectionTestOutput = examineSegmentsForIntersection(
                           testSegment, // w: 
                           reflectionSegmentCoordinates, // p:
                           intersectCoordinates, // intersectCoordinates
                           false // printFlag
                        );
//printCoordinates(  intersectCoordinates, 
//      "INTERSECTION COORDINATES OF REFLECTION  SEGMENT AND MIRROR POLYGON SIDE" ) ; 

                        if( intersectionTestOutput == true ) reflectionSegmentInsidePolygonWallTest = false ;
//prb( intersectionTestOutput, "SEGMENT INTERSECTION" ) ; 
                     } ;
                  } ;

                   
                  if( (! reflectionSegmentTest) || (! reflectionSegmentInsidePolygonWallTest) )
                     reflectionSegmentTest = false ; 

               } ; // END CONTAINMENT TEST

/*
if( reflectionSegmentInsidePolygonWallTest ) 
prt( " ~~~~~~~~~~~~~~~~~~~~~~~> TEST OF MIRROR POLYGON WALL BLOCKAGES PASSED" ) ; 
else 
prt( "~~~~~~~~~~~~~~~~~~~~~~~~> TEST OF MIRROR POLYGON WALL BLOCKAGES FAILED" ) ;
*/
            } ;
   
            totalExaminedReflections[ outputFileChannelNumber ]  += 1 ; 
//pri( thisOrder, "THIS ORDER LEVEL" ) ; 
//prb( reflectionSegmentTest, "TOGETHER -- reflectionSegmentTest" ) ; 
//prb( reflectionSegmentInsidePolygonWallTest, "TOGETHER -- reflectionSegmentInsidePolygonWallTest" ) ; 

            if( reflectionSegmentTest ) // TEST OF MIRRORED SOURCE TO SPEAKER SEGMENT
            {

               viableReflectionCount[ outputFileChannelNumber ] += 1 ; 
               viableReflectionsForThisOrder += 1 ;  

               // ADD REFLECTION TIME TO LIST
               distance =  hypot( 
                  (double) (speakerCoordinatesNow[ 0 ] - 
                  sourceCoordinatesForThisPolygon[ (2 * thisOrder) + 0]),
                  (double) (speakerCoordinatesNow[ 1 ] - 
                  sourceCoordinatesForThisPolygon[ (2 * thisOrder) + 1])
               )  ;
               
               if( viableReflectionCount[ outputFileChannelNumber ] >= reflectionNumberLimit ){
                  prt( ". . . EXPANDING MEMORY" ) ;
                  reflectionNumberLimit += reflectionMemoryExpansionSize ; 
                  reflectionDistances = realloc( reflectionDistances, reflectionNumberLimit * sizeof( float ) ) ;
                  reflectionTimes = realloc( reflectionTimes, reflectionNumberLimit * sizeof( float ) ) ;
                  reflectionOrders = realloc( reflectionOrders, reflectionNumberLimit * sizeof( int ) ) ;
                  sourceOrientationToReflectionAngleDifference = realloc( sourceOrientationToReflectionAngleDifference,
                            reflectionNumberLimit * sizeof( float ) ) ;
                  reflectionWalls_lastToFirst = realloc( reflectionWalls_lastToFirst,
                     reflectionNumberLimit * highOrderLimit * sizeof( int ) ) ;

               } ;
               reflectionDistances[ reflectionDataSetChannelPointer[ outputFileChannelNumber ] + 
                  viableReflectionCount[ outputFileChannelNumber ] - 1 ] = distance ; 
               reflectionTimes[  reflectionDataSetChannelPointer[ outputFileChannelNumber ] + 
                  viableReflectionCount[ outputFileChannelNumber ] - 1 ] = 
                    distance / speedOfSoundInFeetPerSecond ;
               reflectionOrders[ reflectionDataSetChannelPointer[ outputFileChannelNumber ] + 
                  viableReflectionCount[ outputFileChannelNumber ] - 1 ] = thisOrder ;
               for( k = 0; k < thisOrder ; k++ )
                  reflectionWalls_lastToFirst[ 
                  (reflectionDataSetChannelPointer[ outputFileChannelNumber ] * highOrderLimit) + 
                  ((viableReflectionCount[ outputFileChannelNumber ] - 1) * highOrderLimit) + k ] =
                   mirrorSideNumber[ k ] ;

               // MAKE ANGLE DIFFERENCE
               // MAKE SEGMENT FOR SOURCE TO FIRST MIRROR SEGMENT INTERSECTION
               sourceToFirstMirrorSegmentIntersection[ 0 ] = sourceCoordinatesForThisPolygon[ 0 ] ;
               sourceToFirstMirrorSegmentIntersection[ 1 ] = sourceCoordinatesForThisPolygon[ 1 ] ;
               sourceToFirstMirrorSegmentIntersection[ 2 ] = mirrorSegmentIntersectCoordinates[ 0 ] ;
               sourceToFirstMirrorSegmentIntersection[ 3 ] = mirrorSegmentIntersectCoordinates[ 1 ] ;
               // FIND SOURCE TO FIRST MIRROR SEGMENT INTERSECTION ANGLE
     

               sourceToFirstMirrorSegmentIntersectionAngle = 
                  findSegmentAngle( sourceToFirstMirrorSegmentIntersection ); 
               angleDiff = rotatedSource - 
                  sourceToFirstMirrorSegmentIntersectionAngle ;
               while( angleDiff > PI ) angleDiff = angleDiff - TWOPI ;
               while( angleDiff < (-1.0 * PI) ) angleDiff = angleDiff + TWOPI ;
               angleDiff = fabs( angleDiff ) ; 

               sourceOrientationToReflectionAngleDifference[ 
                  reflectionDataSetChannelPointer[ outputFileChannelNumber ] +
                  viableReflectionCount[ outputFileChannelNumber ] - 1 ] = angleDiff ; 

/*
               fprintf( filePointer, "\n" ) ; 
               fprintf( filePointer, "\n%f %f\n%f %f\n",
                  reflectionSegmentCoordinates[0], reflectionSegmentCoordinates[1],
                  reflectionSegmentCoordinates[2], reflectionSegmentCoordinates[3]
               ) ;
*/
                                    // ???
               makeSpaceReflectionCoordinates( thisOrder ) ;
               for( k = 0; k < (thisOrder + 2); k++ ) 
               {  
                  n = (
                           reflectionDataSetChannelPointer[ outputFileChannelNumber ] +
                           viableReflectionCount[ outputFileChannelNumber ] - 1
                         ) * ((highOrderLimit + 2) * 2) ;
                  reflectionSoundPathCoordinates[ n + (k * 2) + 0] = coordinatesForThisSoundPath[ (k * 2) + 0 ] ;
                  reflectionSoundPathCoordinates[ n + (k * 2) + 1] = coordinatesForThisSoundPath[ (k * 2) + 1 ] ;
               } ;
            } else {
               // NO VIABLE REFLECTION

            }; 


/*
for( corner = 0 ; corner < numberOfCorners; corner++ ){
   fprintf( stderr, " [%f %f]", 
      polygonCoordinates[ polygonCoordinatesBaseIndex + (corner * 2) ], 
         polygonCoordinates[ polygonCoordinatesBaseIndex + ((corner * 2) + 1) ] ) ; 
} ;      
*/

/*
            // MAKE MIRRORED POLYGON
               for(i = 0; i < 4; i++) lineSeg[ i ] = (double) mirrorSegCoordinates[ i ] ;
               for( corner = 0; corner < numberOfCorners; corner++)
               {

                  point[ 0 ] = 
                     (double) polygonCoordinates[polygonCoordinatesBaseIndex + (corner * 2)] ; 
                  point[ 1 ] = 
                     (double) polygonCoordinates[polygonCoordinatesBaseIndex + ((corner * 2) + 1)] ;

//                  for(i = 0; i < 4; i++) lineSeg[ i ] = (double) mirrorSegCoordinates[ i ] ;
                  mirrorPointAroundLineSegment( point, lineSeg, mirrorPoint ) ;


                  polygonCoordinates[ mirroredPolygonCoordinatesBaseIndex + (corner * 2) ] =
                       (float) mirrorPoint[ 0 ] ;
                  polygonCoordinates[ mirroredPolygonCoordinatesBaseIndex + ((corner * 2) + 1) ] =
                       (float) mirrorPoint[ 1 ] ;
               }; 
*/


      
               // IF 
               if( thisOrder < highOrderLimit ){

               // MAKE NEW mirrorSegAngleLimits FOR RECURSION
                  if( thisOrder == 1 ){
                     newMirrorSegAngleLimitsLow = mirrorSegCoordinatesAngles[ 0 ] ; 
                     newMirrorSegAngleLimitsHigh = mirrorSegCoordinatesAngles[ 1 ] ; 
                  } else {
                     newMirrorSegAngleLimitsLow = mirrorSegAngleLimitsLow ; 
                     newMirrorSegAngleLimitsHigh = mirrorSegAngleLimitsHigh ; 
                  }; 
               
                  if( mirrorSegCoordinatesAngles[ 0 ] > newMirrorSegAngleLimitsLow ){ 
                     newMirrorSegAngleLimitsLow = mirrorSegCoordinatesAngles[ 0 ] ;
               }; 
                  if( mirrorSegCoordinatesAngles[ 1 ] < newMirrorSegAngleLimitsHigh ){ 
                     newMirrorSegAngleLimitsHigh = mirrorSegCoordinatesAngles[ 1 ] ;
                  }; 

                  newMirrorSegAngleLimitsLow += ( (-1.0 * PI) / 100.0) ;
                  newMirrorSegAngleLimitsHigh += ( PI / 100.0 ) ;



                  mirrorPolygonCoordinatesAroundAllSides(
                     outputFileChannelNumber,
                     thisOrder + 1, // thisOrder: 
                     mirrorSide, // polygonMirrorSideIndex: 
                     newMirrorSegAngleLimitsLow, // mirrorSegAngleLimits: 
                     newMirrorSegAngleLimitsHigh // mirrorSegAngleLimits: 
                  ); 

               };       
            } ;

         }; 

   }; 


   viableReflectionCountByOrder[ (outputFileChannelNumber * highOrderLimit) + thisOrder - 1 ] +=
      viableReflectionsForThisOrder ; 

//   prline( thisOrder - 1,  "\t" ) ;

//fprintf( stderr, "COUNTS: " );
//   for( i = 0; i < highOrderLimit ; i++){
//      fprintf( stderr, " %i ", viableReflectionCountByOrder[i] ) ; 
//   };


} ;


bool pointInPolygonTest(
   float polygonCoordinates[],
   float coordinates[],
   int numberOfPoints,
   char string[]
){
   
   int corner ;
   
   float testSeg[4] ;
   
   float thisPolygonSegment[ 4 ] ;
   float intersectCoordinates[ 2 ] ;
   
    
   int intersectCount=0 ;
   bool thisPointIsInPolygon ;
   bool allPointsAreInPolygon=true ;

   int testPoint ;
   
   
   float distanceToSegmentEnds[ 2 ] ;
   float testPointToSegmentEndAngles[ 2 ] ;
   float testPointToSegmentEndCorner[ 4 ] ;
   int sideSegment ;
   int sideSegmentEnd ; 
   float greaterDistance ;
   float averageAngle ;
    
          

//     pri( numberOfPoints, "NUMBER OF POINTS TO TEST" ) ; 


   // FIND DISTANCE FROM POINT TO EACH SEGMENT END.

   // FOR EACH TEST POINT . . . 
   for( testPoint = 0; testPoint < numberOfPoints; testPoint++ ){

/*
        pri( testPoint, "TEST POINT NUMBER" ) ;
        fprintf( stderr, "\nPOINT COORDINATES: [%f, %f]", 
           coordinates[ (testPoint * 2) + 0 ], coordinates[ (testPoint * 2) + 1 ] ) ;  
*/

      // MAKE A TEST SEG THAT BISECTS EACH OF THE POLYGON'S SIDES.
      for( sideSegment = 0 ; sideSegment < numberOfWalls ; sideSegment ++ ){

         // FIRST FIND WHICH OF THE TWO SEGMENTS FROM THE TEST POINT TO THE 
         // SIDE SEGMENT ENDS IS LONGER. 
         for( sideSegmentEnd = 0 ; sideSegmentEnd < 2 ; sideSegmentEnd++ ){

            // MAKE TEMP SEG FROM TEST POINT TO SIDE SEGMENT END.
            // POINT
            makeSegment( 
               testPointToSegmentEndCorner, 
               &coordinates[ testPoint * 2 ], 
               &polygonCoordinates[ ((sideSegment + sideSegmentEnd) % numberOfWalls) * 2 ] 
            ) ;


/*
              fprintf( stderr, "\n testPointToSegmentEndCorner: [ %f, %f],[ %f, %f]", 
        testPointToSegmentEndCorner[ 0 ], testPointToSegmentEndCorner[ 1 ], 
         testPointToSegmentEndCorner[ 2 ], testPointToSegmentEndCorner[ 3 ] ) ;
*/
            // FIND THE LENGTH OF THE SEGMENT
            distanceToSegmentEnds[ sideSegmentEnd ] = findSegmentLength( testPointToSegmentEndCorner ) ;

            // FIND THE ANGLE OF THE SEGMENT.
            testPointToSegmentEndAngles[ sideSegmentEnd ] = findSegmentAngle( testPointToSegmentEndCorner ) ;

//              prf( piToDegrees( testPointToSegmentEndAngles[ sideSegmentEnd ] ), "THIS ANGLE in Degrees" ) ;  

        } ;
         // FIND THE GREATER DISTANCE OF THE TWO.
         greaterDistance = ( distanceToSegmentEnds[ 0 ] > distanceToSegmentEnds[ 1 ] ) ? 
            distanceToSegmentEnds[ 0 ] : distanceToSegmentEnds[ 1 ] ;
//         prf( greaterDistance, "greaterDistance" ) ; 

         // ADJUST ANGLES IF STRADDLING +/- PI/2
         if( (copysign( 1., testPointToSegmentEndAngles[ 0 ] ) == 
               copysign( 1., testPointToSegmentEndAngles[ 1 ] ) ) ||
                  (fabs( (double)(testPointToSegmentEndAngles[ 1 ] - testPointToSegmentEndAngles[ 0 ]) ) < PI) 
         ){
            // OK
         }else{
            for( sideSegmentEnd = 0 ; sideSegmentEnd < 2 ; sideSegmentEnd++ ){
               if( testPointToSegmentEndAngles[ sideSegmentEnd ] < 0. ) 
                  testPointToSegmentEndAngles[ 0 ] += TWOPI ;
            } ;
         } ;
         // FIND THE AVERAGE ANGLE FOR THE TWO.
//           fprintf( stderr, "\n testPointToSegmentEndAngles: [%f, %f ]", 
//        testPointToSegmentEndAngles[ 0 ], testPointToSegmentEndAngles[ 1 ] ) ;  

         averageAngle = 0.5 * (testPointToSegmentEndAngles[ 0 ] + testPointToSegmentEndAngles[ 1 ]) ;         
         // FIND THE COORDINATES OF THE PROPOSED TEST SEGMENT END THAT LIES AT THE COMPUTED 
         // AVERAGE ANGLE AND GREATER DISTANCE DOUBLED FROM THE POINT. 

//           prf( piToDegrees ( averageAngle ), "averageAngle in Degrees" ) ; 
          
         testSeg[ 0 ] = coordinates[ (testPoint * 2) + 0 ] ;    
         testSeg[ 1 ] = coordinates[ (testPoint * 2) + 1 ] ;
         testSeg[ 2 ] = coordinates[ (testPoint * 2) + 0 ] + ( (greaterDistance * 2) * cos( averageAngle ) ) ;    
         testSeg[ 3 ] = coordinates[ (testPoint * 2) + 1 ] + ( (greaterDistance * 2) * sin( averageAngle ) ) ;    


//           fprintf( stderr, "\n TEST SEG COORDINATES: [ %f, %f],[%f, %f] ", 
//             testSeg[ 0 ], testSeg[ 1 ], testSeg[ 2 ], testSeg[ 3 ] ) ; 

         // FOR THIS TEST SEGMENT, SET THE SIDE SEGMENT INTERSECTION COUNT TO 0.
         intersectCount = 0 ; 

         
         // EXAMINE EACH SIDE FOR INTERSECTION WITH THIS TEST SEGMENT.

//         prt( "*** EXAMINE SIDES FOR INTERSECTION ***" ) ; 

         for( corner = 0; corner < numberOfWalls; corner++ ){

            // TRANSFER CORNER COORDINATES INTO TEMP SEGMENT FOR TESTING. 
            makeSegment( 
               thisPolygonSegment, 
               &polygonCoordinates[corner * 2],
               &polygonCoordinates[((corner + 1) % numberOfWalls) * 2]
            ) ;

/*
        fprintf( stderr, "\nPOLYGON SEGMENT COORDINATES: [ %f, %f][ %f, %f]", 
        thisPolygonSegment[ 0 ], thisPolygonSegment[ 1 ],
           thisPolygonSegment[ 2 ], thisPolygonSegment[ 3 ] ) ; 
*/ 

            if( 
               examineSegmentsForIntersection( testSeg, thisPolygonSegment, intersectCoordinates, false )
            ) intersectCount++ ;

//            pri( intersectCount, "INTERSECTION COUNT NOW" ) ; 


//              fprintf( stderr, "\nINTERSECTION COORDINATES: [ %f, %f]", intersectCoordinates[ 0 ],
//         intersectCoordinates[ 1 ] ) ; 
         } ;


         // IF COUNT IS ODD, THEN A LEGITIMATE TEST SEGMENT WAS FOUND, PROVING
         // THAT THE TEST POINT IS INSIDE THE POLYGON. 
//           pri( intersectCount, "TOTAL INTERSECTION COUNT" ) ; 

         if( (intersectCount % 2) == 1 ) thisPointIsInPolygon = true ;
         else  thisPointIsInPolygon = false ;

//         prb( thisPointIsInPolygon, "thisPointIsInPolygon" ) ;
      } ;

      if( thisPointIsInPolygon == false ) allPointsAreInPolygon = false ; 

//      prb( thisPointIsInPolygon, "thisPointIsInPolygon" ) ; 
//      prb( allPointsAreInPolygon, "allPointsAreInPolygon" ) ; 

   } ;

//   prb( allPointsAreInPolygon, "allPointsAreInPolygon" ) ; 

   if( allPointsAreInPolygon ){
      fprintf( stderr, 
         "\n . . . %s COORDINATES ARE INSIDE POLYGON SPACE.\n", string ) ; 
   }else {
      fprintf( stderr, 
         "\n\nERROR: -------> %s COORDINATES ARE NOT INSIDE POLYGON SPACE. <--------\n\n", string ) ;
      printCoordinates( coordinates, string ) ;
      prt( "\n\n . . . BYE" ) ;  

      makeRoomReflectionsPlotFiles( 0 ) ; fclose( plotFilePointer ) ; 

      exit(EXIT_FAILURE) ;
   } ;

   return( allPointsAreInPolygon ) ; 

} ;

bool polygonTest( //
   float polygon[],
   int numberOfVertices,
   char string[]
){
   int vertex0, vertex1 ;
    
   float seg0[ 4 ], seg1[ 4 ], intersectCoordinates[ 2 ] ;
   bool isThisAPolygon=true ;
     
   

   if( numberOfVertices <= 2 ){
      pri( numberOfVertices, "NUMBER OF VERTICES" ) ; 
      prt( "------> ERROR: POLYGON MUST HAVE THREE OR MORE VERTICES.\n\n" ) ;
      isThisAPolygon = false ; 
      return( isThisAPolygon ) ; 
   } ;

   // TEST FOR DUPLICATED COORDINATES
   for( vertex0 = 0 ; vertex0 < (numberOfVertices - 1) ; vertex0++ ){
      for( vertex1 = vertex0 + 1; vertex1 < numberOfVertices ; vertex1++ ){
         if( sameCoordinatesTest( &polygon[ vertex0 * 2 ], &polygon[ vertex1 * 2 ] ) ) 
         { // DUPLICATE COORDINATES
            isThisAPolygon = false ;
            prt( "\n------ERROR: DUPLICATE COORDINATES" ) ; 
            printCoordinates( &polygon[ vertex0 * 2 ], "COORDINATES 0" ) ;
            printCoordinates( &polygon[ vertex1 * 2 ], "COORDINATES 1" ) ;
            prt( "\n\n" ) ; 
         } ;
      } ;  
   } ;      
   if( isThisAPolygon == false ) return( isThisAPolygon ) ; 

   // TEST FOR INTERSECTING COORDINATES.
   for( vertex0 = 0 ; vertex0 < numberOfVertices; vertex0++ ){
      // MAKE FIRST SEGMENT
      makeSegment( 
         seg0, 
         &polygon[ vertex0 * 2 ], 
         &polygon[ (((vertex0 + 1) % numberOfVertices) * 2) ] 
      ) ;

      for( vertex1 = 0 ; vertex1 < numberOfVertices; vertex1++ ){      

         if( vertex0 != vertex1 ){
            makeSegment( 
               seg1, 
               &polygon[ vertex1 * 2 ], 
               &polygon[ (((vertex1 + 1) % numberOfVertices) * 2) ] 
            ) ;
            // IF SEGMENTS DO NOT SHARE AN END  . . . .
            if( adjacentSegmentsTest( seg0, seg1 ) != true ){
               // TEST FOR INTERSECTION
               if( examineSegmentsForIntersection( seg0, seg1, intersectCoordinates, false )  ){
                  isThisAPolygon = false ;
                  printSegment( seg0, "SEGMENT 0" ) ; 
                  printSegment( seg1, "SEGMENT 1" ) ;
                  printCoordinates( intersectCoordinates, "COORDINATES OF INTERSECTION" ) ;  
               } ;
            } ;
         } ;
      } ;
   } ;


   if( isThisAPolygon ){
      fprintf( stderr, "\n\nPOLYGON COORDINATES FOR THE %s ARE CORRECT.\n", string ) ; 
   } else{
         fprintf( stderr, "%s%s%s%s%s", 
            "\n\n--------> ERROR: <----------\n", 
            "THE POLYGON COORDINATES FOR THE ",
            string,
            " ARE INCORRECT OR OUT OF ORDER,",
            "\nCAUSING ONE OR MORE POLYGON SIDES TO INTERSECT. \n\n. . . .BYE.\n\n\n"
         ) ; 
         exit(EXIT_FAILURE);
   } ; 

   return( isThisAPolygon ); 

} ;


bool isPolygonConcave( 
   float polygon[],
   int numberOfVertices
)
{
   int c0, vertex0, vertex1, vertex2;
   int xSignChangeCount=0, ySignChangeCount=0 ; 
   int xSignNow, ySignNow, xSignLast, ySignLast ; 
   float xDiff, yDiff ;
       
   
   int sideOfLineFlag, sideCount[ 3 ] ;  
   
   xDiff = polygon[ 2 ] - polygon[ 0 ] ;
   yDiff = polygon[ 3 ] - polygon[ 1 ] ;
   xSignLast = (int) copysign( 1., xDiff ) ; 
   ySignLast = (int) copysign( 1., yDiff ) ; 

//   fprintf( stderr, "\nxSignLast: %d ySignLast: %d", xSignLast, ySignLast ) ; 
 
   for(c0 = 1; c0 < numberOfVertices; c0++ )
   {
      vertex0 = c0 ;
      vertex1 = (c0 + 1) % numberOfVertices ;

      xDiff = polygon[ vertex1 * 2 ] - polygon[ vertex0 * 2 ] ;
      yDiff = polygon[ (vertex1 * 2) + 1 ] - polygon[ (vertex0 * 2) + 1 ] ;
      xSignNow = (int) copysign( 1., xDiff ) ; 
      ySignNow = (int) copysign( 1., yDiff ) ; 

//      fprintf( stderr, "\nxSignNow: %d ySignNow: %d", xSignNow, ySignNow ) ; 

      if( xSignNow != xSignLast ) xSignChangeCount++ ;
      if( ySignNow != ySignLast ) ySignChangeCount++ ;
     
      xSignLast = xSignNow ; 
      ySignLast = ySignNow ; 

   } ;

//   fprintf( stderr, "\nxSignChangeCount: %d ySignChangeCount: %d", xSignChangeCount, ySignChangeCount ) ;

   if( (xSignChangeCount > 2) || (ySignChangeCount > 2) ) polygonIsConcave = true ; 
   else polygonIsConcave = false ;

//   prb( polygonIsConcave, "polygonIsConcave" ) ; 

   ivec( polygonReflexVertexAngleFlags, numberOfVertices ) ; 

   if( polygonIsConcave ){

      for(c0 = 0; c0 < 3; c0++ ) sideCount[ c0 ] = 0 ; 

      for(c0 = 0; c0 < numberOfVertices; c0++)
      {
         vertex0 = c0 ; vertex1 = (c0 + 1) % numberOfVertices ; vertex2 = (c0 + 2) % numberOfVertices ;

         sideOfLineFlag = 
            pointToLinePosition( &polygon[vertex0 * 2], &polygon[vertex1 * 2], &polygon[vertex2 * 2]  ) ;
         polygonReflexVertexAngleFlags[ (c0 + 1) % numberOfVertices ] = sideOfLineFlag ;
         sideCount[ sideOfLineFlag + 1 ]++ ;
      } ; 

//      fprintf( stderr, "\n sideCount[ %d, %d, %d ]", sideCount[ 0 ], sideCount[ 1 ], sideCount[ 2 ] ) ; 

/*
      for(c0 = 0; c0 < numberOfVertices; c0++)
        fprintf( stderr, "\npolygonReflexVertexAngleFlags %d: %d", c0, polygonReflexVertexAngleFlags[ c0] ) ; 
*/
      if( sideCount[ 0 ] > sideCount[ 2 ] )
      {
//        prt( "FLIPPING SIGNS" ) ; 
         for(c0 = 0; c0 < numberOfVertices; c0++ ) polygonReflexVertexAngleFlags[ c0 ] *= -1 ; 
      } ;

/*
      for(c0 = 0; c0 < numberOfVertices; c0++ )
      {
         fprintf( stderr, "\n %d: [ %f, %f] flag: %d", c0, polygon[(c0 * 2) + 0], polygon[(c0 * 2) + 1],
           polygonReflexVertexAngleFlags[ c0 ] ) ; 
      } ;
*/


   } ;

   return( polygonIsConcave ) ;

} ;


void getPolygonCoordinates(
   int polygonCoordinatesSource
)
{
   int np, i, j, k, corner ;
   float temp ;
   float segment[ 4 ] ;
   float angle, radius ;   

   if( polygonCoordinatesSource == 0 )
   {
      fprintf( stderr,  "\n\nPOLYGON COORDINATES WILL BE SYNTHESIZED FROM INPUT/DEFAULT DATA.\n\n" ) ;

      // SYNTHESIZED 

      numberOfCorners = numberOfWalls ;

      fvec( coordinatesOfCorners, numberOfCorners * 2 ) ;
      fvec( originToCornerAngles, numberOfCorners ) ;
      fvec( originToCornerDistances, numberOfCorners ) ; 
      fvec( angles, numberOfCorners ) ;
      fvec( randomProportions, numberOfCorners ) ; 

      prbanner( "POLYGONAL ROOM SYNTHESIS PARAMETERS", 69 ) ; 
      pri( numberOfWalls, "NUMBER OF WALLS" ) ; 
      prf( minMaxDistanceToCornerFromOrigin[0], "MINIMUM DISTANCE TO CORNERS FROM ORIGIN (in feet)" ); 
      prf( minMaxDistanceToCornerFromOrigin[1], "MAXIMUM DISTANCE TO CORNERS FROM ORIGIN (in feet)" ); 
      prf( polygonAngleRegularityProportion, "POLYGON SYNTHESIS ANGLE REGULARITY PROPORTION (0-1)" );
      prf( rotationOfSyntheticRoomInDegrees, "ROTATION OF SYNTHESIZED ROOM (in degrees)" ) ; 

      makeCorneredSpace() ;

   }else{

      // FROM CARTESIAN OR POLAR COORDINATES FILE
      if( strcasecmp( polygonCoordinates_datafile, datafile2 ) == 1 ){
         prt( "Missing polygon coordinates file; Please supply file. bye.\n\n" ) ;           
         exit(EXIT_FAILURE);

      } ;

      if( polygonCoordinatesSource == 1 )
      {
         fprintf( stderr, 
			"\n\n%s CARTESIAN POLYGON COORDINATES FILE: ", polygonCoordinates_datafile ) ;  
      } else if( polygonCoordinatesSource == 2 )
      {
         fprintf( stderr, 
		"\n\n%s POLAR POLYGON COORDINATES FILE: ", polygonCoordinates_datafile ) ;
      } else {
         prt( "------> ERROR: ILLEGAL COORDINATES FILE DESIGNATOR\n . . . BYE.\n\n" ) ; 
         exit(EXIT_FAILURE);
      } ;


       // READ IN POLYGON COORDINATES DATA FROM FILE

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( polygonCoordinates_datafile,  new_datafile,  2 ) ; 

      //**************************GET DATA   
      // READ IN COORDINATES
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  polygonCoordinates_datafile ) ;  
         exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      }          

      // TEST FOR PAIRS
      if( (k % 2) != 0 ){
         fprintf( stderr, "\n\nERROR: YOUR POLYGON COORDINATES FILE HAS AN ODD NUMBER OF VALUES." ); 
         fprintf( stderr, "\n ------> FILE MUST INCLUDE X/Y OR ANGLE/RADIUS PAIRS. BYE\n\n" );
         exit(EXIT_FAILURE); 
      } ;
      numberOfCorners = numberOfWalls = k / 2 ; 

      rewind( data ) ;    
      // ALLOCATE SPACE FOR POLYGON COORDINATES.
      fvec( coordinatesOfCorners, numberOfCorners * 2 ) ;
      fvec( originToCornerAngles, numberOfCorners ) ;
      fvec( originToCornerDistances, numberOfCorners ) ; 


      // READ IN VALUES
      np = k / 2 ; 
      k = 0 ;
      for( i = 0; i < np ; i++ ){
         for( j = 0; j < 2 ; j++ ){
     fscanf( data,  " %f ",  &coordinatesOfCorners[ k ] ) ; 
     k++ ;          
         } ;
     } ; 

   }; 
   // IF POLAR COORDINATES, THEN TRANSLATE INTO X-Y COORDINATES. 
   if( polygonCoordinatesSource == 2 )
   {
      // TRANSLATE INTO X-Y COORDINATES
      for( corner = 0; corner < numberOfCorners ; corner++ ){
         angle = coordinatesOfCorners[ (corner * 2) ] ;
         radius = coordinatesOfCorners[ (corner * 2) + 1 ] ;
         coordinatesOfCorners[ (corner * 2) ] = radius * 
			cos( (double) degreesToPi( angle ) ) ;
         coordinatesOfCorners[ (corner * 2) + 1 ] = radius * 
			sin( (double) degreesToPi( angle ) ) ;
      } ;      
   } ;

   // MAKE ORIGIN-TO-CORNER ANGLES IN PI AND RADIAL DISTANCE.
   for( corner = 0; corner < numberOfCorners ; corner++ ){
      originToCornerAngles[ corner ] = 
         atan2( 
            coordinatesOfCorners[ (corner * 2) + 1 ], // y
            coordinatesOfCorners[ (corner * 2) ]   // x
         ); 
      segment[ 0 ] = 0. ;
      segment[ 1 ] = 0. ;
      segment[ 2 ] = coordinatesOfCorners[ (corner * 2) ] ;
      segment[ 3 ] = coordinatesOfCorners[ (corner * 2) + 1 ] ;
      originToCornerDistances[ corner ] = findSegmentLength( segment ) ;
   } ;


   fclose( data ) ;


   printPolygonCoordinatesPlus(
      "ROOM",
      coordinatesOfCorners,
      originToCornerAngles,
      originToCornerDistances,
      numberOfCorners
   ) ;  
 

} ;


void getListenerCoordinates()
{
   int np, i, j, k, position ;
   float temp ;  

   if( (strcasecmp( listenerCoordinatesDataFile, datafile2 ) == 0)){
      numberOfListenerPositions = 1 ; 
      fvec( listenerCoordinates, numberOfListenerPositions * 2 ) ; 
      listenerCoordinates[ 0 ] = 0. ; listenerCoordinates[ 1 ] = 0. ;
      fprintf( stderr, "\n\nUSING DEFAULT LISTENER POSITION." ) ;

   }else{

      // FROM FILE

       prs( listenerCoordinatesDataFile, "\n\nLISTENER COORDINATES FILE" ) ; 

       // READ IN LISTENER COORDINATES DATA FROM FILE

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( listenerCoordinatesDataFile,  new_datafile,  2 ) ; 

      //**************************GET DATA   
      // READ IN COORDINATES.
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  listenerCoordinatesDataFile ) ;  
         exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      }    		

      // TEST FOR PAIRS
      if( (k % 2) != 0 ){
         fprintf( stderr, "\n\nERROR: YOUR LISTENER COORDINATES FILE HAS AN ODD NUMBER OF VALUES." ); 
         fprintf( stderr, "\n ------> FILE MUST INCLUDE X-Y PAIRS. BYE\n\n" );
         exit(EXIT_FAILURE); 
      } ;
      numberOfListenerPositions =  k / 2 ; 


      if( numberOfListenerPositions > 1 )
      {
         pri( numberOfListenerPositions, "NUMBER OF LISTENER COORDINATE PAIRS IN FILE" ) ; 
         prt( "\n\n** ERROR --> ROUTINE IS DESIGNED FOR USE WITH ONLY ONE PAIR OF LISTENER COORDINATES <--" ); 
         prt( "\nLIMIT TO ONE PAIR. . . .  BYE.\n\n" ) ; 
         exit(EXIT_FAILURE); 
      } ;


      rewind( data ) ;    
      // ALLOCATE SPACE FOR LISTENER COORDINATES.
      fvec( listenerCoordinates, numberOfListenerPositions * 2 ) ;


      // READ IN VALUES
      np = k / 2 ; 
      k = 0 ;
      for( i = 0; i < np ; i++ ){
         fprintf( stderr,  "\n" ) ;
         for( j = 0; j < 2 ; j++ ){
	  fscanf( data,  " %f ",  & listenerCoordinates[ k ] ) ; 
	  k++ ; 			
         } ;
     } ; 

   }; 

   fclose( data ) ; 


   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\nLISTENER POSITION COORDINATES:" ) ;  
   for( position = 0 ; position < numberOfListenerPositions; position++ ){
      fprintf( stderr, "\nPOSITION: %d COORDINATES--[X, Y]: [ %f, %f]", 
         position, listenerCoordinates[position * 2], listenerCoordinates[(position * 2) + 1] 
      ) ; 
   } ;
   fprintf( stderr, "\n" )  ; 

} ;





void getSourceCoordinates()
{
   int np, i, j, k, position ;
   float temp ;  

   if( (strcasecmp( sourceCoordinatesDataFile, datafile2 ) == 0)){
      numberOfSourcePositions = 1 ; 
      fvec( sourceCoordinates, numberOfSourcePositions * 2 ) ; 
      sourceCoordinates[ 0 ] = 0. ; sourceCoordinates[ 1 ] = 0. ;
      fprintf( stderr, "\n\nUSING DEFAULT SOURCE POSITION." ) ;

   }else{

      // FROM FILE

       prs( sourceCoordinatesDataFile, "\n\nSOURCE COORDINATES FILE" ) ; 

       // READ IN SOURCE COORDINATES DATA FROM FILE

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( sourceCoordinatesDataFile,  new_datafile,  2 ) ; 

      //**************************GET DATA	
      // READ IN COORDINATES.
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  sourceCoordinatesDataFile ) ;  exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      } 			

      // TEST FOR PAIRS
      if( (k % 2) != 0 ){
         fprintf( stderr, "\n\nERROR: YOUR SOURCE COORDINATES FILE HAS AN ODD NUMBER OF VALUES." ); 
         fprintf( stderr, "\n ------> FILE MUST INCLUDE X-Y PAIRS. BYE\n\n" );
         exit(EXIT_FAILURE); 
      } ;
      numberOfSourcePositions =  k / 2 ; 

      if( numberOfSourcePositions > 1 )
      {
         pri( numberOfSourcePositions, "NUMBER OF SOURCE COORDINATE PAIRS IN FILE" ) ; 
         prt( "\n\n** ERROR --> ROUTINE IS DESIGNED FOR USE WITH ONLY ONE PAIR OF SOURCE COORDINATES <--" ); 
         prt( "\nLIMIT TO ONE PAIR. . . .  BYE.\n\n" ) ; 
         exit(EXIT_FAILURE); 
      } ;


      rewind( data ) ;    
      // ALLOCATE SPACE FOR SOURCE COORDINATES.
      fvec( sourceCoordinates, numberOfSourcePositions * 2 ) ;


      // READ IN VALUES
      np = k / 2 ; 
      k = 0 ;
      for( i = 0; i < np ; i++ ){
         fprintf( stderr,  "\n" ) ;
         for( j = 0; j < 2 ; j++ ){
	  fscanf( data,  " %f ",  & sourceCoordinates[ k ] ) ; 
	  k++ ; 			
         } ;
     } ; 

   }; 

   fclose( data ) ; 


   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\nSOURCE POSITION COORDINATES:" ) ;  
   for( position = 0 ; position < numberOfSourcePositions; position++ ){
      fprintf( stderr, "\nPOSITION: %d COORDINATES--[X, Y]: [ %f, %f]", 
         position, sourceCoordinates[position * 2], sourceCoordinates[(position * 2) + 1] 
      ) ; 
   } ;
   fprintf( stderr, "\n" )  ; 

} ;


void getSpeakerCoordinates()
{
   int np, i, j, k, position ;
   float temp ;
   float segment[ 4 ] ;  

   if( (strcasecmp( speakerCoordinatesDataFile, datafile2 ) == 0)){

      fvec( speakerCoordinates, 2 ) ; 
      numberOfSpeakerPositions = 1 ;
      fvec( speakerCoordinates, numberOfSpeakerPositions * 2 ) ;  
      speakerCoordinates[ 0 ] = 0. ; speakerCoordinates[ 1 ] = 0. ;
      fprintf( stderr, "\n\nUSING DEFAULT OF SINGLE SPEAKER POSITION PLACED AT ORIGIN." ) ;

   }else{

      // FROM FILE

       prs( speakerCoordinatesDataFile, "\nSPEAKER COORDINATES FILE" ) ; 

       // READ IN SPEAKER COORDINATES DATA FROM FILE

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( speakerCoordinatesDataFile,  new_datafile,  2 ) ; 

      //**************************GET DATA	
      // READ IN COORDINATES.
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  speakerCoordinatesDataFile ) ;  
         exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      } 			

      // TEST FOR PAIRS
      if( (k % 2) != 0 ){
         fprintf( stderr, "\n\nERROR: YOUR SPEAKER COORDINATES FILE HAS AN ODD NUMBER OF VALUES." ); 
         fprintf( stderr, "\n ------> FILE MUST INCLUDE X-Y PAIRS. BYE\n\n" );
         exit(EXIT_FAILURE); 
      } ;
      numberOfSpeakerPositions =  k / 2 ; 

      rewind( data ) ;    
      // ALLOCATE SPACE FOR SPEAKER COORDINATES.
      fvec( speakerCoordinates, numberOfSpeakerPositions * 2 ) ;


      // READ IN VALUES
      np = k / 2 ; 
      k = 0 ;
      for( i = 0; i < np ; i++ ){
         fprintf( stderr,  "\n" ) ;
         for( j = 0; j < 2 ; j++ ){
	  fscanf( data,  " %f ",  & speakerCoordinates[ k ] ) ; 
	  k++ ; 			
         } ;
     } ; 

   }; 

   fclose( data ) ; 

   // MAKE ANGLE AND DISTANCES


   fvec( originToSpeakerAngles, numberOfSpeakerPositions ) ; 
   fvec( originToSpeakerDistances, numberOfSpeakerPositions ) ; 
   for( position = 0 ; position < numberOfSpeakerPositions; position++ )
   {
      segment[ 0 ] = 0. ;  
      segment[ 1 ] = 0. ;  
      segment[ 2 ] = speakerCoordinates[position * 2] ;  
      segment[ 3 ] = speakerCoordinates[ (position * 2) + 1] ;  
      originToSpeakerAngles[ position ] = findSegmentAngle( segment ); 
      originToSpeakerDistances[ position ] = findSegmentLength( segment ) ;
   } ;
   

   printPolygonCoordinatesPlus(
         "SPEAKER", 
         speakerCoordinates,
         originToSpeakerAngles,
         originToSpeakerDistances,
         numberOfSpeakerPositions
   ) ;



   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\nSPEAKER POSITION COORDINATES:" ) ;  
   for( position = 0 ; position < numberOfSpeakerPositions; position++ ){
      fprintf( stderr, "\nPOSITION: %d COORDINATES--[X, Y]: [ %f, %f]", 
         position, speakerCoordinates[position * 2], speakerCoordinates[(position * 2) + 1] 
      ) ; 
   } ;
   fprintf( stderr, "\n" )  ; 

} ;

void makeListenerToSpeakerAngles()
{
   int position ;
   int p0, p1, n ;
   float listenerToSpeakerSegment[ 4 ] ;
   
   listenerToSpeakerAngleMaximum = -1. * TWOPI ;
   listenerToSpeakerAngleMinimum = TWOPI ;

   fvec( listenerToSpeakerAngles, numberOfSpeakerPositions ) ;
   ivec( listenerToSpeakerAngleStraddleFlags, numberOfSpeakerPositions ) ; 
   for(position = 0 ; position < numberOfSpeakerPositions ; position++ )
      listenerToSpeakerAngleStraddleFlags[ position ] = 0 ; 
    
   for(position = 0 ; position < numberOfSpeakerPositions ; position++ ){
      listenerToSpeakerSegment[ 0 ] = 
         listenerCoordinates[ ((position % numberOfListenerPositions) * 2) + 0 ] ;
      listenerToSpeakerSegment[ 1 ] = 
         listenerCoordinates[ ((position % numberOfListenerPositions) * 2) + 1 ] ;
      listenerToSpeakerSegment[ 2 ] = 
         speakerCoordinates[ ((position % numberOfSpeakerPositions) * 2) + 0 ] ;
      listenerToSpeakerSegment[ 3 ] = 
         speakerCoordinates[ ((position % numberOfSpeakerPositions) * 2) + 1 ] ;


      listenerToSpeakerAngles[ position ] = findSegmentAngle( listenerToSpeakerSegment ) ;

      fprintf( stderr, "\n listenerToSpeakerSegment: [%f, %f][%f, %f]", 
         listenerToSpeakerSegment[ 0 ],listenerToSpeakerSegment[ 1 ],
         listenerToSpeakerSegment[ 2 ],listenerToSpeakerSegment[ 3 ]
      ) ;
      prf( piToDegrees( listenerToSpeakerAngles[ position ] ), "ANGLE" ) ; 

      if( listenerToSpeakerAngles[ position ] < listenerToSpeakerAngleMinimum ){ 
         listenerToSpeakerAngleMinimum = listenerToSpeakerAngles[ position ] ;
         listenerToSpeakerAngleMinimumIndex = position ;
      } ;
      if( listenerToSpeakerAngles[ position ] > listenerToSpeakerAngleMaximum ){
         listenerToSpeakerAngleMaximum = listenerToSpeakerAngles[ position ] ;
         listenerToSpeakerAngleMaximumIndex = position ; 
      } ;
   } ;

   prf( piToDegrees( listenerToSpeakerAngleMinimum ), "listenerToSpeakerAngleMinimum in degrees" ) ; 
   pri( listenerToSpeakerAngleMinimumIndex, "listenerToSpeakerAngleMinimumIndex" ) ; 

   prf( piToDegrees( listenerToSpeakerAngleMaximum ), "listenerToSpeakerAngleMaximum in degrees" ) ; 
   pri( listenerToSpeakerAngleMaximumIndex, "listenerToSpeakerAngleMaximumIndex" ) ; 

   // STRADDLE ANGLE FLAGS
   if( numberOfSpeakerPositions == 2 ){
      // STEREO PAIR (SEQUENCE)
      prt( "STEREO" ) ; 
      if( (listenerToSpeakerAngleMaximum - listenerToSpeakerAngleMinimum) > PI ){
         // STRADDLES
         listenerToSpeakerAngleStraddleFlags[ 0 ] = 1 ; 
      } ;
   } else
   {
      // THREE OR MORE SPEAKERS
      prt( "THREE OR MORE SPEAKERS" ) ; 
      n = speaker_configuration__sequence_0__polygon_1 == 1 ? 
         numberOfSpeakerPositions : numberOfSpeakerPositions - 1 ; 

//      pri( n, "n" ) ; 

      for(position = 0; position < n ; position++ ){
         p0 = position ; p1 = (p0 + 1) % numberOfSpeakerPositions ;
         if( ((p0 == listenerToSpeakerAngleMinimumIndex) && 
               (p1 == listenerToSpeakerAngleMaximumIndex)) ||
             ((p0 == listenerToSpeakerAngleMaximumIndex) && 
               (p1 == listenerToSpeakerAngleMinimumIndex))
         )
         {
            // STRADDLES
            prt( "IT STRADDLES" ) ; 
            listenerToSpeakerAngleStraddleFlags[ position ] = 1 ;

         } ;
      } ; 

   } ;

   n = speaker_configuration__sequence_0__polygon_1 == 1 ? 
      numberOfSpeakerPositions : numberOfSpeakerPositions - 1 ; 
   for(position = 0; position < n ; position++ )
      pri( listenerToSpeakerAngleStraddleFlags[ position ], "listenerToSpeakerAngleStraddleFlags" ) ; 


} ;

float findSegmentLength(
   float segment[]
){
   return(
      hypot( (double) (segment[0] - segment[2]), (double) (segment[1] - segment[3]) ) 
   ) ;
} ;

float findSegmentAngle(
   float segment[]
){
   
   return(
      atan2(  
         (double) (segment[ 3 ] - segment[ 1 ]),
         (double) (segment[ 2 ] - segment[ 0 ])
      )
   ) ;
} ;

void makeRoomReflectionsPlotFiles(
   int outputChannel
){
   int corner, order, index ; 
   char tempstring[ STRING_SIZE ]="" ; 


   if( (strcasecmp( soundPathsPlotFileName, datafile2 ) == 1) ||
       (strlen( soundPathsPlotFileName ) == 0) ){
      return ;

   } else {

      // MAKE /tmp OUTPUT FILE
      if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 0 )
      {
         // ALL IN ONE
         sprintf( tempstring, "%s", soundPathsPlotFileName ) ;
         plotFilePointer = fopen( tempstring, "w" ) ; 

      }else if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 1 )
      {
         // BY CHANNEL
         sprintf( tempstring, "%s.%d", soundPathsPlotFileName, outputChannel + 1 ) ;
         plotFilePointer = fopen( tempstring, "w" ) ; 

      }else if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 2 )
      {
         // BY CHANNEL AND ORDER
         for(order = 1, index = 0 ; order <= highOrderLimit; order++, index++ )
         {
            sprintf( tempstring, "%s.%d.%d", soundPathsPlotFileName, outputChannel + 1, order ) ;            
            prs( tempstring, "OPENING PLOT FILE" ) ; 
            pri( index, "index" ) ; 
            channelOrderPlotFilePointers[ index ] = fopen( tempstring, "w" ) ; 
         }            

      } ;

      printToPlotFile = true ;

      if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 != 2 )
      {
         // ENTER ROOM COORDINATES
         for( corner = 0 ; corner < (numberOfCorners + 1) ; corner++ ){
            fprintf( plotFilePointer, "%f %f 0.\n", 
               coordinatesOfCorners[ (corner % numberOfCorners) * 2 ], 
               coordinatesOfCorners[ ((corner % numberOfCorners) * 2) + 1 ]
            ) ;
         } ;
         fprintf( plotFilePointer, "\n" ) ; 
      } ;

   } ;

   return ; 
} ;

void findMinMaxValues(
   float array[],
   int numberOfValues,
   float *minVal,
   int *minValIndex,
   float *maxVal,
   int *maxValIndex
){
   int i ; 

   *minVal = array[ 0 ] ; *minValIndex = 0 ; 
   *maxVal = array[ 0 ] ; *maxValIndex = 0 ; 
   for( i = 0; i < numberOfValues ; i++ ){
      if( array[ i ] > *maxVal ){ *maxVal = array[ i ] ; *maxValIndex = i ; } ;
      if( array[ i ] > *minVal ){ * minVal = array[ i ] ; *minValIndex = i ; } ;
   } ;

} ;


bool orderOfAnglesTest( 
   float angles[], 
   int numberOfAngles
){
   int numberOfPositive=0 ; 
   int numberOfNegative=0 ; 
   int i ;
   bool anglesInOrder=true ;  

   if( numberOfSpeakerPositions > 2 ){
      for( i = 0; i < numberOfAngles; i++ )
      {
         if( (angles[ (i + 1) % numberOfAngles ] - angles[ i ]) > 0. ) numberOfPositive++ ; 
         else numberOfNegative++ ;
      } ;
      if( numberOfPositive > numberOfNegative )
      {
         if( numberOfNegative > 1 ) anglesInOrder = false ; 
      } else if( numberOfPositive > 1 )  anglesInOrder = false ; 


      if( anglesInOrder != true ) {
         prt( "\n\nERROR: SPEAKER COORDINATES ARE NOT IN ASCENDING OR DESCENDING ORDER.\n\n. . . BYE\n\n" ) ; 
         exit(EXIT_FAILURE);
      } ;
   } ;      

   return( anglesInOrder ) ; 


} ; 

void scaleAndRotateCoordinates(
   char label[],
   float coordinates[],
   int   numberOfPoints,
   float originToPointAngles[],
   float originToPointDistances[],
   float X_translation_factor,
   float Y_translation_factor,
   float negXscaleFactor,
   float posXscaleFactor,
   float negYscaleFactor,
   float posYscaleFactor,
   float rotateInDegrees,
   float scaleFactor
)
{
   int point ;
   int index, indexPlusOne ;
   float segment[4] ; 

   if( ( rotateInDegrees != 0. ) || ( scaleFactor != 1. ) ||
      (negXscaleFactor != 1.) || (posXscaleFactor != 1.) || 
      (negYscaleFactor != 1.) || (posYscaleFactor != 1.) ||
      (X_translation_factor != 0.) || (Y_translation_factor != 0.)
    ){

      fprintf( stderr, "\n******* %s COORDINATES ARE BEING ROTATED AND/OR SCALED:", label ) ; 
      if( rotateInDegrees != 0. )
         fprintf( stderr, "\n. . . ROTATING %s COORDINATES BY %f DEGREES.", label, rotateInDegrees ) ; 
      if( scaleFactor != 1. )
         fprintf( stderr, "\n. . . SCALING %s COORDINATES BY A FACTOR OF %f.", label, scaleFactor ) ; 


      for( point = 0; point < numberOfPoints ; point++ )
      {
         index = point * 2 ; 
         indexPlusOne = index + 1 ;  
  
         if( X_translation_factor != 0 )
         {
            prf( X_translation_factor, "TRANSLATING X BY" ) ; 
            coordinates[ index ] += X_translation_factor ;
         } ;
         if( Y_translation_factor != 0. )
         {
            prf( Y_translation_factor, "TRANSLATING Y BY" ) ; 
            coordinates[ indexPlusOne ] += Y_translation_factor ;
         } ;

         if( (coordinates[ index ] < 0.) && (negXscaleFactor != 1.) )
         { 
            prf( negXscaleFactor, "SCALING NEGATIVE X RANGE WITH" ) ; 
            coordinates[ index ] *= negXscaleFactor ;
         } ;
         if( (coordinates[ index ] > 0.) && (posXscaleFactor != 1.) ) 
         { 
            prf( posXscaleFactor, "SCALING POSITIVE X RANGE WITH" ) ; 
            coordinates[ index ] *= posXscaleFactor ;
         } ;
         if( (coordinates[ indexPlusOne ] < 0.) && (negYscaleFactor != 1.) ) 
         { 
            prf( negYscaleFactor, "SCALING NEGATIVE Y RANGE WITH" ) ; 
            coordinates[ indexPlusOne ] *= negYscaleFactor ;
         } ;
         if( (coordinates[ indexPlusOne ] > 0.) && (posYscaleFactor != 1.) ) 
         { 
            prf( posYscaleFactor, "SCALING POSITIVE Y RANGE WITH" ) ; 
            coordinates[ indexPlusOne ] *= posYscaleFactor ;
         } ;

         // GLOBAL SCALE
         if( scaleFactor != 1. )
         {
            prf( scaleFactor, "SCALING BOTH X AND Y DIMENSIONS BY" ) ; 
            coordinates[ index ] *= scaleFactor ; 
            coordinates[ indexPlusOne ] *= scaleFactor ; 
         } ;

         // FIND NEW ANGLES
         segment[ 0 ] = 0. ;
         segment[ 1 ] = 0. ;
         segment[ 2 ] = coordinates[ index ] ;
         segment[ 3 ] = coordinates[ indexPlusOne ] ;
         originToPointDistances[ point ] = findSegmentLength( segment ) ;
         originToPointAngles[ point ] = findSegmentAngle( segment ) ; 

         // ROTATE
         if( rotateInDegrees != 0. )
         {
            prf( rotateInDegrees, "ROTATING COORDINATES BY" ) ; 
            originToPointAngles[ point ] += degreesToPi( rotateInDegrees ) ;
            // FIND NEW COORDINATES AND THEN SCALE THEM.
            // X
            coordinates[ index ] = 
               originToPointDistances[ point ] * 
               cos( (double) originToPointAngles[ point ] ); 
            // Y
            coordinates[ indexPlusOne ] = 
               originToPointDistances[ point ] *
               sin( (double) originToPointAngles[ point ] ); 
         } ;

      } ;
      printPolygonCoordinatesPlus(
         label, 
         coordinates,
         originToPointAngles,
         originToPointDistances,
         numberOfPoints
      ) ;

   } ;
} ;

void printPolygonCoordinatesPlus(
   char label[],
   float coordinates[],
   float originToPointAngles[],
   float originToPointDistances[],
   int number
)
{
   int point ; 

   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\n%s COORDINATES:", label ) ;  
   for( point = 0 ; point < number; point++ ){
      fprintf( stderr, "\nPOINT: %d [X, Y]: [ %f, %f]\tANGLE (in degrees): %f\tDISTANCE TO POINT: %f", 
         point, 
         coordinates[point * 2], 
         coordinates[(point * 2) + 1],
         piToDegrees( originToPointAngles[ point ] ),
         originToPointDistances[ point ] 
      ) ; 
   } ;
   fprintf( stderr, "\n" )  ; 

}; 

void printPolygonCoordinates(
   char label[],
   float coordinates[],
   int number
)
{
   int point ; 

   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\n%s COORDINATES:", label ) ;  
   for( point = 0 ; point < number; point++ ){
      fprintf( stderr, "\nPOINT: %d [X, Y]: [ %f, %f]\t", 
         point, 
         coordinates[point * 2], 
         coordinates[(point * 2) + 1]
      ) ; 
   } ;
   fprintf( stderr, "\n" )  ; 

}; 

void makeSpeakerSeriesSegmentDistances(
)
{
   int corner ;
   float segment[ 4 ] ;

   fvec( speakerSegmentDistances, numberOfCorners ) ;
   for( corner = 0; corner < numberOfCorners; corner++ )
   {
      segment[ 0 ] = speakerCoordinates[ (corner * 2) ] ;
      segment[ 1 ] = speakerCoordinates[ (corner * 2) + 1 ] ;
      segment[ 2 ] = speakerCoordinates[ ( ((corner + 1) % numberOfCorners) * 2) ] ;
      segment[ 3 ] = speakerCoordinates[ ( ((corner + 1) % numberOfCorners) * 2) + 1 ] ;
      speakerSegmentDistances[ corner ] = findSegmentLength( segment ); 

   } ; 


} ;

int writeReflectionPulsesIntoImpulseResponse(
   int outputFileChannelNumber,
   int numberOfOutputChannels
)
{
   int j, numberWritten=0, s;
   int n, k;
   float peakAmpWritten=0.;
   float amp, wallGainscaleAmp, reflectionOrderGainscaleAmp ;
   
   float peakAmp ;
   int thisReflectionOrderLimit ;       
   
   int thisReflection ; 
   int wallIndex ;
   int wallImpulseResponseNumber ;
   int croppedImpulseResponseNowSize ;  
   int startN, lastN , incr, numberOfWallsInSequence, numberOfTestWalls ;
   int startWallIndex ;
   int selectedAndSortedReflectionsIndex ;
   int reflectionOrderImpulseResponseNumber ; 

   

   float *transferArray ;
   
   
   int IRindex ;
   bool testFlag ;
   float avgAmp=0. ;
   
   
   bool writeImpulseFlag ;
   float windowedSamplePeakAmp ;
   float airAbsorbMultiplier ;
   float thisAngleDifference, thisAngleDifferenceProportion, thisProportionOfDispersionDecibels;
   float distance, segment[ 4 ] ;    

/*
   if( first )
   {
      // MAKE OUTPUT IMPULSE RESPONSE SPACE
      pri( IR_DataLength, "IR_DataLength" ) ; 
      fvec( IR_Data, IR_DataLength ) ; 
      first = false ; 
   }else
   {
      // ZERO OUT IR_Data
      for(i = 0; i < IR_DataLength; i++) IR_Data[ i ] = 0. ; 
   } ;
*/

 if( reflectionsFlag )
 {



   if( previousImpulseResponseNowMemorySize == -1 )
   {
      // FIRST OUTPUT CHANNEL: MAKE OPENING IMPULSE RESPONSE SPACE
      if( wall_impulse_and_gainscale_response_mode == 0 )
          impulseResponseNowMemorySize = highOrderLimit * wall_numberOfFrames ;
      else 
          impulseResponseNowMemorySize = 
             abs( wall_impulse_and_gainscale_response_mode ) * wall_numberOfFrames ;

      fvec( impulseResponseNow, impulseResponseNowMemorySize ) ;
      previousImpulseResponseNowMemorySize = impulseResponseNowMemorySize ;     
   } ;

   // ZERO OUT impulseResponseNow FOR THE STARTUP OF THIS CHANNEL. 
   for( n = 0 ; n <  impulseResponseNowMemorySize ; n++ ) impulseResponseNow[ n ] = 0. ;  

   // LOOP THROUGH ALL OF THE REFLECTIONS . . . 
   for( selectedAndSortedReflectionsIndex = 0; 
        selectedAndSortedReflectionsIndex < numberOfSelectedWallReflectionSequences ; 
        selectedAndSortedReflectionsIndex++ )
   {

      prt( "\n\n NEW REFLECTION ------------------------------------------" ) ; 
      writeImpulseFlag = true ; 

      thisReflection = reorderedWallIRSequences_firstToLast[ 
         (selectedAndSortedReflectionsIndex * (highOrderLimit + 2)) + 0 // THE NUMBERED REFLECTION
      ] ;

      thisReflectionOrderLimit = reflectionOrders[ 
         reflectionDataSetChannelPointer[ outputFileChannelNumber ] + thisReflection  
       ] ;
//      pri( thisReflectionOrderLimit, "thisReflectionOrderLimit" ) ; 

      // TRANSFER WALL REFLECTION SEQUENCE FROM SELECTION ARRAY.
      numberOfWallsInSequence = reorderedWallIRSequences_firstToLast[ 
         (selectedAndSortedReflectionsIndex * (highOrderLimit + 2)) + 1 
      ] ;

      for( n = 0; n < numberOfWallsInSequence; n++ )
      {
         sequenceOfWallIResponses[ n ] = reorderedWallIRSequences_firstToLast[ 
            (selectedAndSortedReflectionsIndex * (highOrderLimit + 2)) + 2 + n 
         ] ;
      } ;
//prb( reflectionOrderImpulseResponsesFlag, "reflectionOrderImpulseResponsesFlag" ) ; 
      if( reflectionOrderImpulseResponsesFlag )
      {
         reflectionOrderImpulseResponseNumber = (int) 
            fmin( 
               (float) thisReflectionOrderLimit, 
               (float) numberOfReflectionOrderChannelAssignments 
            ) - 1  ;
      } ;
      pri( reflectionOrderImpulseResponseNumber, "reflectionOrderImpulseResponseNumber" ) ; 

      //  *** ADD ORDER IMPULSE RESPONSE TO END IF SPECIFIED.
      //  DISTINGUISH INDEX (WHICH STARTS AT 1) BY MAKING NEGATIVE. 
      if( reflectionOrderImpulseResponsesFlag )
      { // reflectionOrderCVOrderSequences
         for(n = 0; n < reflectionOrderCVOrderSequenceSizes[ reflectionOrderImpulseResponseNumber ]; n++)
         {
            sequenceOfWallIResponses[ numberOfWallsInSequence ] = 
               reflectionOrderCVOrderSequences[ 
                  (reflectionOrderImpulseResponseNumber * highOrderLimit) + n 
               ] * -1 ;
            numberOfWallsInSequence++ ;
         } ;
         // EXPERIMENTAL ORDER REVERSAL
/*
         for(n = 0; n < (numberOfWallsInSequence / 2) ; n++)
         {
            j = sequenceOfWallIResponses[ n ] ;
            sequenceOfWallIResponses[ n ] = sequenceOfWallIResponses[numberOfWallsInSequence - 1 - n] ;
            sequenceOfWallIResponses[numberOfWallsInSequence - 1 - n] = j ;             
         } ;
*/

      } ;

      // MAKE AND PRINT ORIGINAL WALL REFLECTION SEQUENCE (first to last)

//      fprintf( stderr, "\nREFLECTION WALL NUMBER SEQUENCE (first to last): " ) ; 
      for( n = thisReflectionOrderLimit - 1; n >= 0; n-- )
      {
         originalWallReflectionSequence_firstToLast[ n ] = 
           reflectionWalls_lastToFirst[ 
            (reflectionDataSetChannelPointer[ outputFileChannelNumber ] * highOrderLimit) + 
              (thisReflection * highOrderLimit) + n 
           ] ;
//         fprintf( stderr, "%d ", originalWallReflectionSequence_firstToLast[ n ] ) ;   
      } ;


      // **** PRINT IR CONVOLUTION SEQUENCE 
      fprintf( stderr, "\nWALL IR NUMBER SEQUENCE: " ) ; 
      for( n = 0; n < numberOfWallsInSequence; n++ )
         fprintf( stderr, "%d ", sequenceOfWallIResponses[ n ] ) ;   

      // FIRST IN LIST OF WALLS IS THE LAST REFLECTION WALL, AND LAST IS THE FIRST. 
      // 1 = last wall, -1 = first wall, 2 = last 2, -2 = first 2. etc.

      //  *** CONVOLUTION OF WALLS TOGETHER
      // FIRST TRANSFER FIRST WALL (LAST REFLECTION POINT) INTO OUTPUT IMPULSE ARRAY.

//prb( reflectionOrderImpulseResponsesFlag, "reflectionOrderImpulseResponsesFlag" ) ; 
//pri( wall_numberOfFrames, "wall_numberOfFrames" ) ;
//pri( reflection_order_numberOfFrames, "reflection_order_numberOfFrames" ) ;  

      impulseResponseNowMemorySize = wall_numberOfFrames ; 

//      impulseResponseNowMemorySize = (reflectionOrderImpulseResponsesFlag) ? 
//         wall_numberOfFrames : reflection_order_numberOfFrames ; 
//         reflection_order_numberOfFrames : wall_numberOfFrames  ; 


//pri( impulseResponseNowMemorySize, "A. impulseResponseNowMemorySize" ) ; 
       
      if( impulseResponseNowMemorySize > previousImpulseResponseNowMemorySize )
      {
         fvec( transferArray, previousImpulseResponseNowMemorySize ) ; 
         for( n = 0; n < previousImpulseResponseNowMemorySize; n++) transferArray[ n ] = 
            impulseResponseNow[ n ] ; 
         free( impulseResponseNow ) ; fvec( impulseResponseNow, impulseResponseNowMemorySize ) ;
         for( n = 0; n < previousImpulseResponseNowMemorySize; n++) impulseResponseNow[ n ] = 
            transferArray[ n ] ; // 
         previousImpulseResponseNowMemorySize = impulseResponseNowMemorySize ;
         free( transferArray ) ;  
      } ;

      // DETERMINE IF THIS SEQUENCE IS DIFFERENT FROM ANY PREVIOUS SEQUENCES
      // BY TESTING A SHORTER AND SHORTER BEGINNING SEGMENT UNTIL A MATCH, IF ANY, IS FOUND. 
      // 
      numberOfTestWalls = numberOfWallsInSequence ;
//      pri( numberOfTestWalls, "numberOfTestWalls (BEFORE TESTING)" ) ; 
      while( 
          (numberOfTestWalls > 0)
            &&
         !(testFlag = testSequence( sequenceOfWallIResponses, numberOfTestWalls, &IRindex ))
      ){
         numberOfTestWalls-- ;            
      } ;

//      pri( numberOfTestWalls, "numberOfTestWalls (AFTER TESTING)" ) ; 
//      prb( testFlag, "testFlag" ) ; 
      if( testFlag ){
//         prt( "SEQUENCE OR SUBSET WAS FOUND; RECALL IT TO USE OR BUILD OFF OF." ) ; 
         recallIR( IRindex ) ; startWallIndex =  numberOfTestWalls ;
//pri( startWallIndex, "A startWallIndex" ) ; 
      }else
      {
         prt(" NO SEQUENCE OR SUBSET WAS FOUND." ) ;
         pri( sequenceOfWallIResponses[ 0 ], " FILL RESPONSE WITH SEQUENCE" ) ;  

         impulseResponseNowMemorySize = sequenceOfWallIResponses[ 0 ] >= 0 ?
            wall_numberOfFrames : reflection_order_numberOfFrames  ;
         if( impulseResponseNowMemorySize > previousImpulseResponseNowMemorySize )
         {
//            pri( impulseResponseNowMemorySize, "A. RE-ALLOCATING impulseResponseNow" ) ; 
            free( impulseResponseNow ) ; 
            fvec( impulseResponseNow, impulseResponseNowMemorySize ) ;
            previousImpulseResponseNowMemorySize = impulseResponseNowMemorySize ; 
         } ;

         for( n = 0; n < impulseResponseNowMemorySize; n++ )
         {
            if( sequenceOfWallIResponses[ 0 ] >= 0 )
              impulseResponseNow[ n ] = wallImpulseResponses[ 
                  ( sequenceOfWallIResponses[ 0 ] * wall_numberOfFrames) + n  
              ] ; 
            else
              impulseResponseNow[ n ] = reflectionOrderImpulseResponses[
                 ( ((sequenceOfWallIResponses[ 0 ] * -1) - 1) * reflection_order_numberOfFrames) + n
              ] ; 
         } ;
         startWallIndex = 1 ; 

         if( (
                (wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 2)
                   ||
                (wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 3)
             ) 
           && ( wallImpulseResponseNumber >= 0 )
         ) //
         {  
            prt( "FILTERING OUTPUT AFTER WALL CONVOLUTION" ) ; 
            filterAndNormalizeImpulseResponseNow( 0, 1. ) ;
         }else if(
            (
               (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 2) 
                 || 
               (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 3) 
            ) && ( wallImpulseResponseNumber < 0 )
         ) //
         {  

//            prt( "FILTERING OUTPUT AFTER REFLECTION ORDER CONVOLUTION" ) ; 
//            filterAndNormalizeImpulseResponseNow( 1, 1. ) ;
         }  ; 



         fprintf( stderr, "\n SAVING THIS SINGLE WALL SEQUENCE, (WITH FILTERING INCLUDED): " ) ; 
         addToIRfileCodes( sequenceOfWallIResponses, 1 );             
      } ;   

      // CONVOLVE FIRST IMPULSE RESPONSE WITH OTHERS IN SEQUENCE OR 
      // PASS OVER IF NO OTHERS. 
//pri( startWallIndex, "startWallIndex" ) ; 
//pri( numberOfWallsInSequence, "numberOfWallsInSequence" ) ; 
      for( wallIndex = startWallIndex ; wallIndex < numberOfWallsInSequence ; wallIndex++ )
      {
//pri( wallIndex, "wallIndex"); 
//         prt( "ONE OR MORE LEFT TO CONVOLVE WITH." ) ; 

         wallImpulseResponseNumber = sequenceOfWallIResponses[ wallIndex ] ;

         for(n = 0; n <= wallIndex; n++)
            sequenceOfWallIResponsesNow[ n ] = sequenceOfWallIResponses[ n ] ;

         peakAmp = findPeakAmp(impulseResponseNow, impulseResponseNowMemorySize ); 

         croppedImpulseResponseNowSize = impulseResponseNowMemorySize ;

//pri( croppedImpulseResponseNowSize, "croppedImpulseResponseNowSize" ) ; 

         if( wallImpulseResponseNumber < 0 )
         {
            pri( wallImpulseResponseNumber, "A. CONVOLVING WITH REFLECTION ORDER" ) ; 
            writeImpulseFlag = makeReflectionOrderImpulseResponseNow(
               outputFileChannelNumber,
               thisReflection,
               wallImpulseResponseNumber,
               &reflectionOrderImpulseResponseNowBaseIndex,
               &reflectionOrderImpulseResponseNowSize,
               &windowedSamplePeakAmp
            ) ;
//            croppedImpulseResponseNowSize = reflectionOrderImpulseResponseNowSize ;

// prf( reflectionOrderImpulseResponseNow[ reflectionOrderImpulseResponseNowBaseIndex + 16384 ], 
// "A. IS THIS 0.?"); 


            peakAmp = findPeakAmp( 
               &reflectionOrderImpulseResponseNow[ reflectionOrderImpulseResponseNowBaseIndex ],
               reflectionOrderImpulseResponseNowSize ) ; 

//            prf( amp_to_dB( peakAmp ), "PEAK AMP JUST AFTER makeReflectionOrderImpulseResponseNow in dB" ) ; 


//pri( reflectionOrderImpulseResponseNowBaseIndex, "reflectionOrderImpulseResponseNowBaseIndex" ) ; 
// pri( reflectionOrderImpulseResponseNowSize, "reflectionOrderImpulseResponseNowSize" ) ; 
//prf( amp_to_dB( windowedSamplePeakAmp ), "windowedSamplePeakAmp AFTER ROUTINE" ) ; 
            
            if( writeImpulseFlag )
            {
// pri( reflectionOrderImpulseResponseNowSize, "JUST BEFORE CONVOLVE reflectionOrderImpulseResponseNowSize" ) ;  
// pri( croppedImpulseResponseNowSize, "JUST BEFORE CONVOLVE croppedImpulseResponseNowSize" ) ; 
//prt( "B. CONVOLVING WITH REFLECTION ORDER" ) ;



               if( reflectionOrderImpulseResponseNowSize < croppedImpulseResponseNowSize )
               {
//                  prt( "option A" ) ; 
                  convolveTwoArrays(
                     &reflectionOrderImpulseResponseNow[ reflectionOrderImpulseResponseNowBaseIndex ],
                     reflectionOrderImpulseResponseNowSize,
                     impulseResponseNow, 
                     croppedImpulseResponseNowSize
                  ) ;
               } else
               {
//                  prt( "option B" ) ; 
                  convolveTwoArrays(
                     impulseResponseNow, 
                     croppedImpulseResponseNowSize,
                     &reflectionOrderImpulseResponseNow[ reflectionOrderImpulseResponseNowBaseIndex ],
                     reflectionOrderImpulseResponseNowSize
                  ) ;
               } ;

//prf( convolutionOutput[ 16384 ], "B. IS THIS 0.?"); 


            } ;
         }else
         {
            //pri( wallImpulseResponseNumber, "CONVOLVING WITH WALL" ) ; 
            if( wallImpulseResponsesCroppedSize[ wallImpulseResponseNumber ] <
                croppedImpulseResponseNowSize )
            {
//               prt( "option A" ) ; 
               convolveTwoArrays(
                  &wallImpulseResponses[ wallImpulseResponseNumber * wall_numberOfFrames ], 
                  wallImpulseResponsesCroppedSize[ wallImpulseResponseNumber ],
                  impulseResponseNow, 
                  croppedImpulseResponseNowSize
               ) ;
            }else
            {
//               prt( "option B" ) ; 
               convolveTwoArrays(
                  impulseResponseNow, 
                  croppedImpulseResponseNowSize,
                  &wallImpulseResponses[ wallImpulseResponseNumber * wall_numberOfFrames ], 
                  wallImpulseResponsesCroppedSize[ wallImpulseResponseNumber ]
               ) ;
            } ;
         } ;

         if( ! writeImpulseFlag ) break ; 
         convolutionCount++ ;

         impulseResponseNowMemorySize = convolutionOutputSize ;
         if( impulseResponseNowMemorySize > previousImpulseResponseNowMemorySize )
         {
            pri( impulseResponseNowMemorySize, "A. RE-ALLOCATING impulseResponseNow" ) ; 
            free( impulseResponseNow ) ; 
            fvec( impulseResponseNow, impulseResponseNowMemorySize ) ;
            previousImpulseResponseNowMemorySize = impulseResponseNowMemorySize ; 
         } ;

         // TRANSFER INTO RESPONSE ARRAY. 
         for( n = 0; n < impulseResponseNowMemorySize; n++ )
         {
            impulseResponseNow[ n ] = convolutionOutput[ n ] ;
         } ;

//prf( impulseResponseNow[ 16384 ], "C. IS THIS 0.?"); 


         // NORMALIZE
         peakAmp = findPeakAmp(impulseResponseNow, impulseResponseNowMemorySize );
         if( peakAmp > 0. ) for( n = 0; n < impulseResponseNowMemorySize; n++ ) 
            impulseResponseNow[ n ] /= peakAmp ; 

         // SCALE THIS FINAL CONVOLUTION WITH THE REFLECTION ORDER IMPULSE TO 
         // THE PEAK WINDOWED SAMPLE AMP, IF SELECTED.
         if( (wallImpulseResponseNumber < 0) && 
            (reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 == 2) && 
            (scale_to_pre_envelope_window_sample_peak_amps__off_0__on_1 == 1) 
         )
         {
            for( n = 0; n < impulseResponseNowMemorySize; n++ ) 
               impulseResponseNow[ n ] *= windowedSamplePeakAmp ;
         } ; 
         


         // FILTER OUTPUT WALL REFLECTIONS, IF REQUESTED. 
         if( (
                (wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 2)
                   ||
                (wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 3)
             ) 
           && ( wallImpulseResponseNumber >= 0 )
         ) // 
         {  
            prt( "FILTERING OUTPUT AFTER WALL CONVOLUTION" ) ; 
            filterAndNormalizeImpulseResponseNow( 0, 1. ) ;

         }else if(
            (
               (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 2) 
                 || 
               (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 3) 
            ) && ( wallImpulseResponseNumber < 0 )
         ) //
         {  
//            prt( "FILTERING OUTPUT AFTER REFLECTION ORDER CONVOLUTION" ) ; 
//            filterAndNormalizeImpulseResponseNow( 1, 1. ) ;
         }  ; // END OF FILTER

         // ADD CODE TO COLLECTION, CONVOLUTION TO FILE, AND THEN INCREMENT COUNT
         if( (reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 == 2) &&
             (wallImpulseResponseNumber < 0)
         )
         {
            prt( "IN SYNCHRONOUS SAMPLE WINDOW MODE; DON'T SAVE THIS SEQUENCE" ) ; 
         }else
         {
            fprintf( stderr, "\n SAVING THIS SEQUENCE, ANY REFLECTION ORDER FILTERING INCLUDED: " ) ; 
            for(n = 0; n <= wallIndex; n++) fprintf( stderr, " %d", sequenceOfWallIResponsesNow[ n ] ) ; 
//            pri( impulseResponseNowMemorySize, "BEFORE ADD OF SEQUENCE TO DATABASE" )  ; 
            addToIRfileCodes( sequenceOfWallIResponsesNow, wallIndex + 1 );             

//prf( impulseResponseNow[ 16384 ], "C. IS THIS 0.?"); 

         } ;

      } ; // END OF CONVOLUTION LOOP

      // FILTER OUTPUT WALL REFLECTIONS, IF REQUESTED. 
      if( writeImpulseFlag )
      {

 
         croppedImpulseResponseNowSize = impulseResponseNowMemorySize ;
//pri( croppedImpulseResponseNowSize, "A. croppedImpulseResponseNowSize" ) ; 


         j = (int)( preEchoTime + 
               (reflectionTimes[ 
               reflectionDataSetChannelPointer[ outputFileChannelNumber ] + thisReflection] * 
                  reflections_time_scaler * (float) osr) + 0.5
             ) ;
//pri( j, "j" ) ; 

//pri( outputFileChannelNumber, "B outputFileChannelNumber" ) ;
 
//pri( reflectionDataSetChannelPointer[ outputFileChannelNumber ], 
//         "reflectionDataSetChannelPointer[ outputFileChannelNumber ]");

//pri( thisReflection, "thisReflection" ) ;  

//prf( sourceDispersionPatternRolloffInDecibels, "sourceDispersionPatternRolloffInDecibels" ) ; 

         thisAngleDifference = sourceOrientationToReflectionAngleDifference[ 
                reflectionDataSetChannelPointer[ outputFileChannelNumber ] + thisReflection] ;
//prf( thisAngleDifference, "thisAngleDifference" ) ; 
         
         thisAngleDifferenceProportion = thisAngleDifference / PI ; 
//prf( thisAngleDifferenceProportion, "thisAngleDifferenceProportion" ) ; 

		// C: IN writeReflectionPulsesIntoImpulseResponse (TRACKED)
         thisProportionOfDispersionDecibels = thisAngleDifferenceProportion * 
             sourceDispersionPatternRolloffInDecibels ;

//prf( thisProportionOfDispersionDecibels, "thisProportionOfDispersionDecibels" ) ; 

//         amp = thisProportionOfDispersionDecibelsAsAmp = dB_to_amp( 
//                 (float) thisProportionOfDispersionDecibels ) ; 
         amp = (float) pow( (double) 10.0, (double) (thisProportionOfDispersionDecibels / 20.) ) ;

//prf( amp, "amp" ) ; 



/*
         amp = (float) dB_to_amp( sourceDispersionPatternRolloffInDecibels * 
             (sourceOrientationToReflectionAngleDifference[ 
                reflectionDataSetChannelPointer[ outputFileChannelNumber ] + thisReflection] / PI) 
            )  ;
*/
//            prf( amp_to_dB( amp ), "amp JUST BEFORE AIR ABSORB FACTOR in dB" ) ; 

          airAbsorbMultiplier = pow( (double)
               fmin( 1., (double) (minimumReferenceDistanceInFeet / 
                 reflectionDistances[ 
                    reflectionDataSetChannelPointer[ outputFileChannelNumber ] + thisReflection]) ), 
               (double) airAbsorptionExponentForReflections 
            ) ;
//            prf( airAbsorbMultiplier, "airAbsorbMultiplier" ) ; 
            amp *= airAbsorbMultiplier ; 
//            prf( amp_to_dB( amp ), "amp AFTER airAbsorbMultiplier in dB" ) ; 


         // ADD WALL GAINSCALE
         wallGainscaleAmp = 1.0 ; 
         if( wall_impulse_and_gainscale_response_mode > 0 )
         { // FROM FIRST
            startN = 0 ; lastN = wall_impulse_and_gainscale_response_mode - 1 ; 
            incr = +1 ; 
         }else if( wall_impulse_and_gainscale_response_mode > 0 )
         { // FROM LAST
            startN = thisReflectionOrderLimit + wall_impulse_and_gainscale_response_mode ; 
            lastN = thisReflectionOrderLimit - 1 ; incr = +1 ; 
         }else
         {
            startN = 0 ; lastN = thisReflectionOrderLimit - 1  ; incr = +1 ; 
         } ;
         for(k = startN ; k <= lastN ; k += incr )
         { 
            wallGainscaleAmp *= dB_to_amp( 
               wallDecibelGainscaleLevels[ 
                  originalWallReflectionSequence_firstToLast[ k ] % numberOfWallDecibelGainscaleLevels 
               ] 
            );  
         } ;

         amp *= wallGainscaleAmp ; 

//            prf( amp_to_dB( amp ), "amp JUST AFTER wallGainscaleAmp in dB" ) ; 


         k = thisReflectionOrderLimit > numberOfReflectionOrderDecibelGainscaleLevels ?
            numberOfReflectionOrderDecibelGainscaleLevels : thisReflectionOrderLimit ;
//pri( k, "k" ); 

         reflectionOrderGainscaleAmp = dB_to_amp( reflectionOrderDecibelGainscaleLevels[k - 1] ) ;

         amp *= reflectionOrderGainscaleAmp ;

         amp *= dB_to_amp( reflected_sound_gain_in_decibels ) 
                        * frontSourceHeadRoomScalar ;   

         avgAmp += amp ; 



         if( amp >= dB_to_amp( impulse_inclusion_threshold_in_dB ) )
         {
            if( amp > peakAmpWritten ) peakAmpWritten = amp ; 

//pri( croppedImpulseResponseNowSize, "B. croppedImpulseResponseNowSize" ) ; 
//pri( j, "j"  ) ; 
            if( (j + croppedImpulseResponseNowSize) > highestIndexWritten ) 
                  highestIndexWritten = j + croppedImpulseResponseNowSize ;

            if( highestIndexWritten >= IR_DataLength )
            {
               // LENGTHEN IR_Data 
               fvec( transferArray, IR_DataLength ) ; 
               for( s = 0 ; s < IR_DataLength; s++ ) transferArray[ s ] = IR_Data[ s ] ;
               free( IR_Data ) ; 
               pri( highestIndexWritten + 1, "LENGTHENING IR_Data TO" ) ; 
               fvec( IR_Data, highestIndexWritten + 1 ) ; 
               for( s = 0 ; s < IR_DataLength; s++ ) IR_Data[ s ] = transferArray[ s ] ;
               IR_DataLength = highestIndexWritten + 1 ; 
               free( transferArray ) ; 

            } ;
//            pri( croppedImpulseResponseNowSize, "C. JUST BEFORE WRITE croppedImpulseResponseNowSize" ) ; 
            peakAmp = findPeakAmp( impulseResponseNow, croppedImpulseResponseNowSize ) ; 
//            prf( amp_to_dB( peakAmp ), "PEAK AMP JUST BEFORE WRITE in dB" ) ; 
   

//prf( impulseResponseNow[ 16384 ], "D. IS THIS 0.?"); 

            for( s = 0; s < croppedImpulseResponseNowSize; s++ )
            {
               IR_Data[ j + s ] += ( amp * impulseResponseNow[ s ] ) ; 
            } ;

            if( printToPlotFile ) 
            {
               n = ( 
                 reflectionDataSetChannelPointer[ outputFileChannelNumber ] + 
                 thisReflection 
                 ) * ((highOrderLimit + 2) * 2) ;
               for(s = 0; s < (thisReflectionOrderLimit + 2); s++ )
               {
                  if( s == 0 )
                  {
                     distance = 0. ;
                  }else
                  {
                     makeSegment(
                        segment,
                        &reflectionSoundPathCoordinates[n + ((s - 1) * 2)],
                        &reflectionSoundPathCoordinates[n + (s * 2)]
                     ) ;
                     distance += findSegmentLength( segment ) ;
                  } ;

                  if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 2 )
                  {

                     fprintf( channelOrderPlotFilePointers[thisReflectionOrderLimit - 1], "%f %f %f\n",
                        reflectionSoundPathCoordinates[n + (s * 2) + 0], 
                        reflectionSoundPathCoordinates[n + (s * 2) + 1],
                        longestReflectionDistance - distance
                     ) ;
                  }else
                  {
                     fprintf( plotFilePointer, "%f %f %f\n",
                        reflectionSoundPathCoordinates[n + (s * 2) + 0], 
                        reflectionSoundPathCoordinates[n + (s * 2) + 1],
                        longestReflectionDistance - distance
                     ) ;
                  } ;

               } ;

               if( plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 == 2 )
                  fprintf( channelOrderPlotFilePointers[thisReflectionOrderLimit - 1], "\n" ) ; 
               else
                  fprintf( plotFilePointer, "\n" ) ; 
            } ;
            numberWritten++ ;
         } ;
      }else
      {
         prt( "IMPULSE HAS ZERO AMP. SKIPPING WRITE." ) ; 
      } ;
   } ;

   prf( peakAmpWritten, "peakAmpWritten" ) ; 
   prf( amp_to_dB( peakAmpWritten ), "peakAmpWritten in dB" ) ; 

   avgAmp /= numberWritten ;

   return( numberWritten ) ; 
 }else
 {
    return(0) ; 

 } ;
 

} ;

// ***


void writeDirectSourcePulsesIntoImpulseResponse(
   int channel
)
{
   int j ; 

   if( directSoundFlag )
   {

      // FIND THE SOURCE DISTANCE TO SPEAKER. 
      // FIRST MAKE SEGMENT
      prt( "INCLUDING DIRECT SOURCE . . . " ); 

      // ***** ADD SOURCE-DIRECT PULSE TO RESPONSE, MODIFIED BY DISTANCE.
      j = (int)(( 
            (  
                preEchoTime + 
                (sourceSpeakerOrientationSign 
                  * sourceToSpeakerDistanceForDelays[ channel ] / speedOfSoundInFeetPerSecond) 
            )
            * (float) osr) + 0.5) ;


      IR_Data[ j ] += ( directSoundSpeakerAmplitudeScalars[ channel ] ) ;

   } else
   {
      prt( "OMITTING DIRECT SOURCE SOUND." ) ; 

   } ;
} ;



// ***

void writeDirectSourcePulsesIntoImpulseResponseOLD(
   int channel
)
{
   int position ; 
   
   int speaker0, speaker1 ; 
   bool found ;
   bool onePairStraddles ;  
   int j ; 
    
   float listenerToSourceAngleTemp ;
   float listenerToSpeakerAngleMaximumTemp ; 
   float listenerToSpeakerAngleMinimumTemp ;
   float sourceToMinDiff ; 
   float sourceToMaxDiff ;
   float speaker0Diff ; 
   float speaker1Diff ; 
   float angleDiff ; 
   float sourceCoordinatesShiftedToClosestAngle[ 2 ] ;  
   float sourceCoordinatesShiftedToNextClosestAngle[ 2 ] ;  
   float closestSpeakerToNewClosestSpeakerSourceDistance ; // closestSpeakerToNewSourceDistance ; 
   float nextClosestSpeakerToNewClosestSpeakerSourceDistance ;  

   float nextClosestSpeakerToNewNextClosestSpeakerSourceDistance  ; // nextClosestSpeakerToNewSourceDistance ;
   float closestSpeakerToNewNextClosestSpeakerSourceDistance  ;

   float amp1, amp2, diffAmp, finalAmp, distance1, distance2 ; 
   float amp1AtThreshold, amp2AtThreshold, finalAmpAtThreshold, finalAmpInVirtualOrRealSpace ;

   float segment[ 4 ] ;
   
   float rolloffDecibels ;
   
   float thisAirAbsorptionExponent ;   


   if( directSoundFlag )
   {




      // FIND THE SOURCE DISTANCE TO SPEAKER. 
      // FIRST MAKE SEGMENT
      prt( "INCLUDING DIRECT SOURCE . . . " ); 

      // 


//prf( piToDegrees( listenerToSourceAngle ), "listenerToSourceAngle in degrees" ) ; 

      // FIND THE PAIR OF ADJACENT listenerToSpeakerAngles BETWEEN WHICH THE 
      // listenerToSourceAngle LIES. 
      found = false ; 
      speaker0 = 0 ;
      onePairStraddles = false ;  

      
      while( (found == false) && 
         (speaker0 < ((numberOfSpeakerPositions - 1) + speaker_configuration__sequence_0__polygon_1)) )
      {

         speaker1 = (speaker0 + 1) % numberOfSpeakerPositions ;
//pri( speaker0, "HERE speaker0" ) ; 
//pri( speaker1, "HERE speaker1" ) ; 

//pri( listenerToSpeakerAngleMinimumIndex, "listenerToSpeakerAngleMinimumIndex" ) ; 
//pri( listenerToSpeakerAngleMaximumIndex, "listenerToSpeakerAngleMaximumIndex" ) ; 
         // IF THIS PAIR OF SPEAKER COORDINATES ARE THE STRADDLING MIN-MAX COORDINATES . . .  
         if( listenerToSpeakerAngleStraddleFlags[ speaker0 ] == 1 )
         {   // IN STRADDLE REGION
//prt( "STRADDLE ANGLE" ) ;  

            onePairStraddles = true ; 
            // THEN TEST THE listenerToSourceAngle TO SEE IF IT IS BETWEEN THE ANGLE'S
            // .
            if(
               (listenerToSourceAngle > listenerToSpeakerAngleMaximum) || 
               (listenerToSourceAngle < listenerToSpeakerAngleMinimum)
            )
            {
               // IT IS. SO FIND OUT WHICH IS THE CLOSER TO IT. 
               // FIRST, FLIP THE SPEAKER AND SOURCE ANGLES by 90 degrees. 
               listenerToSourceAngleTemp = listenerToSourceAngle - copysign( PI, listenerToSourceAngle ) ;
               listenerToSpeakerAngleMinimumTemp = listenerToSpeakerAngleMinimum -
                  copysign( PI, listenerToSpeakerAngleMinimum ) ;
               listenerToSpeakerAngleMaximumTemp = listenerToSpeakerAngleMaximum -
                  copysign( PI, listenerToSpeakerAngleMaximum ) ;

/*
prf( piToDegrees( listenerToSourceAngleTemp ), "listenerToSourceAngleTemp in degrees" ) ; 
prf( piToDegrees( listenerToSpeakerAngleMinimum ), "listenerToSpeakerAngleMinimum in degrees" ) ; 
prf( piToDegrees( listenerToSpeakerAngleMinimumTemp ), "listenerToSpeakerAngleMinimumTemp in degrees" ) ; 
prf( piToDegrees( listenerToSpeakerAngleMaximum ), "listenerToSpeakerAngleMaximum in degrees" ) ; 
prf( piToDegrees( listenerToSpeakerAngleMaximumTemp ), "listenerToSpeakerAngleMaximumTemp in degrees" ) ; 
*/
               // THEN IDENTIFY WHICH SPEAKER ANGLE VECTOR IS CLOSER AND ASSIGN ACCORDINGLY.
               sourceToMinDiff = fabs( (double) listenerToSpeakerAngleMinimumTemp - 
                  listenerToSourceAngleTemp ) ;
               sourceToMaxDiff = fabs( (double) listenerToSpeakerAngleMaximumTemp -
                  listenerToSourceAngleTemp ) ;
               angleDiff = fabs( (double) listenerToSpeakerAngleMaximumTemp - 
                  listenerToSpeakerAngleMinimumTemp ) ;
               if( sourceToMinDiff < sourceToMaxDiff )
               {
                 indexOfClosestSpeaker = listenerToSpeakerAngleMinimumIndex ; 
                 indexOfNextClosestSpeaker = listenerToSpeakerAngleMaximumIndex ; 
                 crossFadeProportionForClosest = 1. - (sourceToMinDiff / angleDiff) ;
               }else
               {
                 indexOfClosestSpeaker = listenerToSpeakerAngleMaximumIndex ; 
                 indexOfNextClosestSpeaker = listenerToSpeakerAngleMinimumIndex ; 
                 crossFadeProportionForClosest = 1. - (sourceToMaxDiff / angleDiff) ;
               } ;
               found = true ; 
            } ;
         } else
         { // THIS SPEAKER PAIR IS NOT THE STRADDLE PAIR. 
           // SO HANDLE MIN-MAX NORMALLY. 
// prt( "NOT STRADDLING PAIR." ) ;  

            // TEST THE listenerToSourceAngle TO SEE IF IT IS BETWEEN THE MIN-MAX PAIR, 
            // INCLUSIVE OF THE BOUNDARIES. 
            if( 
               valueIsBetweenTheseTwo( 
                  listenerToSourceAngle, 
                  listenerToSpeakerAngles[ speaker0 ],
                  listenerToSpeakerAngles[ speaker1 ]
               )
            )
            {

// prf( piToDegrees( listenerToSpeakerAngles[ speaker0 ] ), "listenerToSpeakerAngles[ speaker0 ] in degrees" ) ; 
// prf( piToDegrees( listenerToSpeakerAngles[ speaker1 ] ), "listenerToSpeakerAngles[ speaker1 ] in degrees" ) ; 

               // IT IS BETWEEN THEM. 

               // IDENTIFY WHICH SPEAKER ANGLE VECTOR IS CLOSER AND ASSIGN ACCORDINGLY.
               speaker0Diff = 
                  fabs( (double) listenerToSpeakerAngles[ speaker0 ] - listenerToSourceAngle ) ;
               speaker1Diff = 
                  fabs( (double) listenerToSpeakerAngles[ speaker1 ] - listenerToSourceAngle ) ;
               angleDiff = fabs( (double) listenerToSpeakerAngles[ speaker0 ] - 
                  listenerToSpeakerAngles[ speaker1 ] ) ;
               if( speaker0Diff < speaker1Diff )
               {
                 indexOfClosestSpeaker = speaker0 ; 
                 indexOfNextClosestSpeaker = speaker1 ; 
                 crossFadeProportionForClosest = 1.0 - (speaker0Diff / angleDiff) ;
               }else
               {
                 indexOfClosestSpeaker = speaker1 ; 
                 indexOfNextClosestSpeaker = speaker0 ; 
                 crossFadeProportionForClosest = 1.0 - (speaker1Diff / angleDiff) ;
               } ;
               found = true ; 
            } ; 
         } ;
         crossFadeProportionForNextClosest = 1. - crossFadeProportionForClosest ;            
         speaker0++ ;   
      } ; // END OF WHILE LOOP SEARCH FOR ADJACENT SPEAKER PAIR 
          // BETWEEN WHICH LISTENER-TO-SOURCE VECTOR SITS. 

      // SEQUENCE
      if( (speaker_configuration__sequence_0__polygon_1 == 0) && (found == false) )
      {
          indexOfClosestSpeaker = indexOfNextClosestSpeaker = -1 ;   
      } ;

         
      pri( indexOfClosestSpeaker, "indexOfClosestSpeaker" ) ; 
      pri( indexOfNextClosestSpeaker, "indexOfNextClosestSpeaker" ) ; 

      // IF SPEAKER BEING PROCESSED FOR THIS CHANNEL IS EITHER THE CLOSEST OR NEXT CLOSEST,
      // THEN MAKE CROSSFADE AMPLITUDE FOR THE ONE BEING PROCESSED. 
      // OTHERWISE, PROCEED NORMALLY. 
      thisSpeaker = channel % numberOfSpeakerPositions ;
      pri( thisSpeaker, "THIS SPEAKER" ) ; 

      thisAirAbsorptionExponent = sourceSpeakerOrientationSign == 1. ? 
         airAbsorptionExponentForVirtualSpaceSource : airAbsorptionExponentForRealSpaceSource ;


      if( (thisSpeaker == indexOfClosestSpeaker) || (thisSpeaker == indexOfNextClosestSpeaker) )
      {   
          // ******* IN CROSSFADE ANGLE ********
       
         prt( "THIS CHANNEL IS PART OF THE CROSSFADE." ) ; 
         fprintf( stderr, "\n\ncrossFadeProportionForClosest: %f crossFadeProportionForNextClosest: %f\n", 
             crossFadeProportionForClosest, crossFadeProportionForNextClosest );  

         // FIND SOURCE-TO-SPEAKER DISTANCES FOR SOURCE POSITION ROTATED AROUND 
         // LISTENER POINT TO CLOSEST AND NEXT-CLOSEST ANGLE POSITIONS. 
         pri( indexOfClosestSpeaker, "BEFORE ROTATE -- indexOfClosestSpeaker" ) ; 
         for( position = 0; position < numberOfSpeakerPositions; position++ )
            fprintf( stderr, "\n%d listenerToSpeakerAngles in degrees: %f", position, 
               piToDegrees( listenerToSpeakerAngles[ position ] ) ) ; 
         rotatePointToAngle(
            &sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ], // point to rotate
            &listenerCoordinates[ (channel % numberOfListenerPositions) * 2 ], // origin point
            listenerToSpeakerAngles[ indexOfClosestSpeaker ], // new angle 
            sourceCoordinatesShiftedToClosestAngle // new point   
         ) ;
         fprintf( stderr, "\n sourceCoordinatesShiftedToClosestAngle: [%f, %f]", 
            sourceCoordinatesShiftedToClosestAngle[0], 
            sourceCoordinatesShiftedToClosestAngle[1]
         ) ;
         rotatePointToAngle(
            &sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ], // point to rotate
            &listenerCoordinates[ (channel % numberOfListenerPositions) * 2 ], // origin point
            listenerToSpeakerAngles[ indexOfNextClosestSpeaker ], // new angle 
            sourceCoordinatesShiftedToNextClosestAngle // new point   
         ) ;
         fprintf( stderr, "\n sourceCoordinatesShiftedToNextClosestAngle: [%f, %f]", 
            sourceCoordinatesShiftedToNextClosestAngle[0], 
            sourceCoordinatesShiftedToNextClosestAngle[1]
         ) ;

         // FIND DISTANCE FROM NEW POINTS TO RESPECTIVE SPEAKERS.
         makeSegment(
            segment,
            &speakerCoordinates[ indexOfClosestSpeaker * 2 ],
            sourceCoordinatesShiftedToClosestAngle
         ) ;
         printSegment( segment, "sourceCoordinatesShiftedToClosestAngle" ); 
         closestSpeakerToNewClosestSpeakerSourceDistance = findSegmentLength( segment ) ; 

         makeSegment(
            segment,
            &speakerCoordinates[ indexOfNextClosestSpeaker * 2 ],
            sourceCoordinatesShiftedToClosestAngle
         ) ;
         nextClosestSpeakerToNewClosestSpeakerSourceDistance = findSegmentLength( segment ) ; 
        


//
         makeSegment(
            segment,
            &speakerCoordinates[ indexOfNextClosestSpeaker * 2 ],
            sourceCoordinatesShiftedToNextClosestAngle
         ) ;

         printSegment( segment, "sourceCoordinatesShiftedToNextClosestAngle" ); 
         nextClosestSpeakerToNewNextClosestSpeakerSourceDistance = findSegmentLength( segment ) ; 

         makeSegment(
            segment,
            &speakerCoordinates[ indexOfClosestSpeaker * 2 ],
            sourceCoordinatesShiftedToNextClosestAngle
         ) ;
         closestSpeakerToNewNextClosestSpeakerSourceDistance = findSegmentLength( segment ) ;
//







//
         prf( closestSpeakerToNewClosestSpeakerSourceDistance, "closestSpeakerToNewClosestSpeakerSourceDistance" ); 
         prf( nextClosestSpeakerToNewNextClosestSpeakerSourceDistance, "nextClosestSpeakerToNewNextClosestSpeakerSourceDistance" ); 
      
// HERE  CROSSFADE
         if( thisSpeaker == indexOfClosestSpeaker )
         {
            crossFadeAmpModifier = crossFadeProportionForClosest ;
            distance1 = closestSpeakerToNewClosestSpeakerSourceDistance ;
            distance2 = nextClosestSpeakerToNewClosestSpeakerSourceDistance ; 
         }else
         {
            crossFadeAmpModifier = crossFadeProportionForNextClosest ;
            distance1 = nextClosestSpeakerToNewNextClosestSpeakerSourceDistance ;
            distance2 = closestSpeakerToNewNextClosestSpeakerSourceDistance ; 
         } ;

         amp1 = 
            pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / distance1 ) ), 
              (double) (thisAirAbsorptionExponent * sourceSpeakerOrientationSign) 
           ) ; 
         amp2 = 
            pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / distance2 ) ), 
              (double) (thisAirAbsorptionExponent * sourceSpeakerOrientationSign) 
           ) ; 

         amp1AtThreshold = 
            pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / distance1 ) ), 
              (double) airAbsorptionExponentAtThreshold 
           ) ; 
         amp2AtThreshold = 
            pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / distance2 ) ), 
              (double) airAbsorptionExponentAtThreshold 
           ) ; 

         diffAmp = amp1 - amp2 ; 
         finalAmpInVirtualOrRealSpace = amp2 + (crossFadeAmpModifier * diffAmp) +
           ((1. - crossFadeAmpModifier) * diffAmp) ;

         diffAmp = amp1AtThreshold - amp2AtThreshold ; 
         finalAmpAtThreshold = amp2AtThreshold + (crossFadeAmpModifier * diffAmp) +
           ((1. - crossFadeAmpModifier) * diffAmp) ;

         finalAmp = ((1. - sourceToThresholdProximityDistanceProportion) * finalAmpInVirtualOrRealSpace) + 
                       (sourceToThresholdProximityDistanceProportion * finalAmpAtThreshold) ;



         makeSegment(
            sourceToSpeakerSegment,
            &sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ],
            &speakerCoordinates[ (channel % numberOfSpeakerPositions) * 2 ]
         ) ;

         sourceToSpeakerDistanceForDelay = findSegmentLength( sourceToSpeakerSegment ); 

         makeSegment(
            sourceToListenerSegment,
            &sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ],
            &listenerCoordinates[ (channel % numberOfListenerPositions) * 2 ]
         ) ;


         sourceToSpeakerOrThresholdAngle = findSegmentAngle( sourceToListenerSegment ) ;
         prf( sourceToSpeakerOrThresholdAngle, "SOURCE TO LISTENER ANGLE" ) ; 

/*
         proximityAmpScaler = 1. ; // FOR CROSSFADE SPEAKERS
         proximityProportion = 
            fminf( 1., sourceToThresholdProximityDistance / 10. ) ; // 1 to 0


prf( proximityProportion, "proximityProportion" ) ;
*/

prf( sourceToSpeakerDistanceForDelay, "sourceToSpeakerDistanceForDelay" ) ; 
prf( sourceToSpeakerDistanceForAmp, "sourceToSpeakerDistanceForAmp" ) ; 
/*
         // IN CROSSFADE
         sourceToSpeakerDistanceForAmp = 
            (proximityProportion * sourceToSpeakerDistanceForDelay) + 
            ((1. - proximityProportion) * sourceToSpeakerDistanceForAmp) ; 
         crossFadeAmpModifier = 
            proximityProportion + 
            ((1. - proximityProportion) * crossFadeAmpModifier) ; 
*/

/*
         sourceToSpeakerDistanceForAmp = sourceToSpeakerDistanceForDelay = 
            findSegmentLength( sourceToSpeakerSegment ); 
         sourceToSpeakerOrThresholdAngle = findSegmentAngle( sourceToSpeakerSegment ) ;

         crossFadeAmpModifier = 1. ; 
*/


      } else {
          // ******* NOT IN CROSSFADE ANGLE ********


         prt( "SPEAKER IS **NOT** THE CLOSEST OR NEXT CLOSEST." ) ; 

         crossFadeAmpModifier = 1. ; 

         makeSegment(
            sourceToSpeakerSegment,
            &sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ],
            &speakerCoordinates[ (channel % numberOfSpeakerPositions) * 2 ]
         ) ;
         // HERE NOT CROSSFADE
         sourceToSpeakerDistanceForAmp = sourceToSpeakerDistanceForDelay = 
            findSegmentLength( sourceToSpeakerSegment ); 
         sourceToSpeakerOrThresholdAngle = findSegmentAngle( sourceToSpeakerSegment ) ;

         prf( sourceToSpeakerOrThresholdAngle, "SOURCE TO SPEAKER ANGLE" ) ; 
         // FOR NON-CROSSFADE SPEAKERS -- REDUCES AMP THE CLOSER SOURCE IS TO THRESHOLD. 

         finalAmpInVirtualOrRealSpace = pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / sourceToSpeakerDistanceForAmp ) ), 
              (double) (thisAirAbsorptionExponent * sourceSpeakerOrientationSign) 
           ) ; 
         finalAmpAtThreshold = pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / sourceToSpeakerDistanceForAmp ) ), 
              (double) (airAbsorptionExponentAtThreshold) 
           ) ; 

         finalAmp = ((1. - sourceToThresholdProximityDistanceProportion) * finalAmpInVirtualOrRealSpace) + 
                       (sourceToThresholdProximityDistanceProportion * finalAmpAtThreshold) ;

      } ;
         
      prf( sourceToSpeakerDistanceForAmp, "sourceToSpeakerDistanceForAmp" ) ; 
      prf( sourceToSpeakerDistanceForDelay, "sourceToSpeakerDistanceForDelay" ) ; 



      // ***** ADD SOURCE-DIRECT PULSE TO RESPONSE, MODIFIED BY DISTANCE.


      
      j = (int)(( 
            (  
                preEchoTime + 
                (sourceSpeakerOrientationSign 
                  * sourceToSpeakerDistanceForDelay / speedOfSoundInFeetPerSecond) 
            )
            * (float) osr) + 0.5) ;
      pri( j, "j" ) ; 


      prf( rotatedSource, "rotatedSource" ) ;
      prf( sourceToSpeakerOrThresholdAngle, "sourceToSpeakerOrThresholdAngle" ) ;  
      angleDiff = rotatedSource - sourceToSpeakerOrThresholdAngle ;
      while( angleDiff > PI ) angleDiff = angleDiff - TWOPI ;
      while( angleDiff < (-1. * PI) ) angleDiff = angleDiff + TWOPI ;
      angleDiff = fabs( angleDiff ) ; 

//      angleDiff = fabs( rotatedSource - (PI * -0.5) ) ;
//      if( angleDiff > PI ) angleDiff -= PI ;
 
     prf( angleDiff, "angleDiff" ) ; 

	// D: in writeDirectSourcePulsesIntoImpulseResponseOLD
      rolloffDecibels = sourceDispersionPatternRolloffInDecibels * (angleDiff / PI) ;
      prf( rolloffDecibels, "rolloffDecibels" ) ; 

prf( sourceToSpeakerDistanceForAmp, "HERE sourceToSpeakerDistanceForAmp" ) ; 
         // NOT IN CROSSFADE: crossFadeAmpModifier = 1, proximityAmpScaler = 1-0 WITH THRESHOLD APPROACH.
         // IN CROSSFADE: crossFadeAmpModifier = 0-1, proximityAmpScaler = 1.
         // crossFadeAmpModifier *  // proximityAmpScaler * 

/*
      IR_Data[ j ] += (
         dB_to_amp( direct_sound_gain_in_decibels )  * dB_to_amp( rolloffDecibels ) * finalAmp *
         frontSourceHeadRoomScalar 
      ) ;
*/

      IR_Data[ j ] += ( directSoundSpeakerAmplitudeScalars[ channel ] ) ;



   } else
   {
      prt( "OMITTING DIRECT SOURCE SOUND." ) ; 

   } ;
} ;


void makeSegment(
   float segment[],
   float point0[],
   float point1[]
)
{
   segment[ 0 ] = point0[ 0 ] ;
   segment[ 1 ] = point0[ 1 ] ;
   segment[ 2 ] = point1[ 0 ] ;
   segment[ 3 ] = point1[ 1 ] ;
} ;

void rotatePointToAngle(
   float pointToRotate[], // point to rotate
   float originPoint[], // origin point
   float newAnglePosition, // new angle 
   float newPointPosition[] // new point   
)
{
   float segment[4] ;
   float segmentLength ; 
 
   printCoordinates( pointToRotate, "pointToRotate" ) ; 
   printCoordinates( originPoint, "originPoint" ) ; 

//   prf( piToDegrees( newAnglePosition ), "INSIDE FUNCTION: newAnglePosition in degrees" ) ;   

   makeSegment( segment, originPoint, pointToRotate  ) ;
   segmentLength = findSegmentLength( segment ); 
   prf( segmentLength, "segmentLength" ) ; 
   
   // MAKE NEW COORDINATES
   newPointPosition[ 0 ] = (segmentLength  * (float) cos( (double) newAnglePosition )) + originPoint[ 0 ] ;
   newPointPosition[ 1 ] = (segmentLength  * (float) sin( (double) newAnglePosition )) + originPoint[ 1 ] ;


} ;


bool valueIsBetweenTheseTwo(
   float v,
   float b0,
   float b1
)
{

   prf( v, "ANGLE" ) ; 
   prf( b0, "BOUNDARY ANGLE 0" ) ;    
   prf( b1, "BOUNDARY ANGLE 1" ) ;    

   if( 
      ( (v >= b0) && (v <= b1) ) 
         ||
      ( (v <= b0) && (v >= b1) ) 
   )
   {
      prt( "TRUE" ) ; 
      return( true ) ; 
   } else
   {
      prt( "FALSE" ) ; 
      return( false ) ; 
   } ;


} ;

void printSegment(
   float s[],
   char text[]
){
   fprintf( stderr, "\n%s: [ %f,  %f][  %f,  %f]",  text, s[0],  s[1], s[2],  s[3] ) ;
} ;

void printCoordinates(
   float s[],
   char text[]
){
   fprintf( stderr, "\n%s: [ %f,  %f]",  text, s[0],  s[1] ) ;
} ;

bool sameCoordinatesTest(
   float coordinates0[],
   float coordinates1[]
)
{

   if( (coordinates0[ 0 ] == coordinates1[ 0 ]) && 
       (coordinates0[ 1 ] == coordinates1[ 1 ])  
   ) 
      return( true ) ;
   else 
      return( false ) ; 
} ;



bool adjacentSegmentsTest(
   float seg0[],
   float seg1[]
)
{
   bool test=false ;

   if( sameCoordinatesTest( &seg0[ 0 ], &seg1[ 0 ]  ) ) test = true ; 
   if( sameCoordinatesTest( &seg0[ 0 ], &seg1[ 2 ]  ) ) test = true ; 
   if( sameCoordinatesTest( &seg0[ 2 ], &seg1[ 0 ]  ) ) test = true ; 
   if( sameCoordinatesTest( &seg0[ 2 ], &seg1[ 2 ]  ) ) test = true ; 
   
   return( test ) ; 

} ;

void makeSourceToSpeakerDistancesAndAngles()
{
   static bool first=true ; 
   int p ;
   float segment[ 4 ] ;

   if( first )
   {
      fvec( sourceToSpeakerDistances, numberOfSpeakerPositions ) ; 
      fvec( sourceToSpeakerAngles, numberOfSpeakerPositions ) ; 
      first = false ; 
   } ;

   prt( "SOURCE TO SPEAKER DISTANCES AND ANGLES (in degrees): " ) ; 
   for( p = 0 ; p < numberOfSpeakerPositions ; p++ )
   {
      makeSegment(
         segment,
//         &sourceCoordinates[ (p % numberOfSourcePositions) * 2 ],
         sourceCoordinates,
            &speakerCoordinates[ (p % numberOfSpeakerPositions) * 2 ]
      ) ;
      sourceToSpeakerDistances[ p ] = findSegmentLength( segment ) ; 
      sourceToSpeakerAngles[ p ] = findSegmentAngle( segment ) ;  
      fprintf( stderr, "\nDISTANCE %f ", sourceToSpeakerDistances[ p ] ) ; 
      fprintf( stderr, "\nANGLE %f ", (sourceToSpeakerAngles[ p ] / PI) * 180. ) ; 
   } ;

   // FIND MIN AND MAX
   minSourceToSpeakerDistance = sourceToSpeakerDistances[ 0 ] ;
   maxSourceToSpeakerDistance = sourceToSpeakerDistances[ 0 ] ;
   for( p = 1 ; p < numberOfSpeakerPositions ; p++ ){
      if( sourceToSpeakerDistances[ p ] < minSourceToSpeakerDistance )
         minSourceToSpeakerDistance =  sourceToSpeakerDistances[ p ] ;
      if( sourceToSpeakerDistances[ p ] < maxSourceToSpeakerDistance )
         maxSourceToSpeakerDistance =  sourceToSpeakerDistances[ p ] ;

   } ;



} ;

void findSourceToSpeakerAngleDifferencesFromSourceToListenerAngle()
{
   static bool first=true ; 
   int p ;

   if( first )
   {
      fvec( sourceToSpeakerAngleDifferences, numberOfSpeakerPositions ) ; 
      first = false ; 
   } ;

//   prt( "SOURCE TO SPEAKER ANGLE DIFFERENCES FROM SOURCE TO LISTENER ANGLE (in degrees): " ) ; 
   for( p = 0; p < numberOfSpeakerPositions ; p++ )
   {
      sourceToSpeakerAngleDifferences[ p ] = sourceToSpeakerAngles[ p ] - sourceToListenerAngle ;
      prf( (sourceToSpeakerAngleDifferences[ p ] / PI) * 180., "" ) ;  
   } ;


} ;



void makeSourceToThresholdProximityProportion()
{
   sourceToThresholdProximityDistanceProportion = 1. - 
      fmin( 
         1., 
         sourceIsBehindSpeakers ?
            sourceToThresholdProximityDistance / sourceToThresholdProximityVirtualRange :
            sourceToThresholdProximityDistance / sourceToThresholdProximityRealRange
      ) ;
   prf( sourceToThresholdProximityDistanceProportion, "sourceToThresholdProximityDistanceProportion" ) ; 

} ;

bool areCoordinatesTheSame(
   float p0[],
   float p1[]
)
{
   if( (p0[ 0 ] == p1[ 0 ]) && (p0[ 1 ] == p1[ 1 ]) ) return( true ) ; 
   return( false ) ;   
} ;


void makeDirectSoundSpeakerDelayTimes()
{
   int channel ;
   float sourceToSpeakerSegment[ 4 ] ; 

   fvec( sourceToSpeakerDistanceForDelays, numberOfSpeakerPositions ) ; 

   for( channel = 0; channel < numberOfSpeakerPositions; channel ++)
   {   
      makeSegment(
         sourceToSpeakerSegment,
         &sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ],
         &speakerCoordinates[ (channel % numberOfSpeakerPositions) * 2 ]
      ) ;
      sourceToSpeakerDistanceForDelays[ channel ] = findSegmentLength( sourceToSpeakerSegment ); 

   } ;

} ;

void makeDirectSoundSpeakerAmplitudes()
{
   float tempAngles[ 2 ] ;
   float tempSourceCoordinates0[ 2 ] ;
   float tempSourceCoordinates1[ 2 ] ;
   float *tempAngleAmpScalars0, *tempAngleAmpScalars1 ;
   float *limitedDiffAngles0, *limitedDiffAngles1 ;
   float tempSourceToSpeakerDistance ; 
   float crossFadeProportion ;
   float listenerToSourceAngleTemp ;
   float segment[ 4 ] ;
   float baseAngle, angle ;
   float *distanceAmpScalars0, *distanceAmpScalars1 ;
   float thisAirAbsorptionExponent ;
   float sourceInFrontProximityGain ;
   float rolloffDecibels, *rolloffDecibels0, *rolloffDecibels1, angleDiff ;    
   float thisRotatedSource ;
    
   float thresholdProximityScalar ; 

   prt( "IN makeDirectSoundSpeakerAmplitudes" ) ; 


   fvec( directSoundSpeakerAmplitudeScalars, numberOfSpeakerPositions ) ; 

   fvec( limitedDiffAngles0, numberOfSpeakerPositions ) ; 
   fvec( limitedDiffAngles1, numberOfSpeakerPositions ) ; 

   fvec( tempAngleAmpScalars0, numberOfSpeakerPositions ) ; 
   fvec( tempAngleAmpScalars1, numberOfSpeakerPositions ) ; 

   fvec( distanceAmpScalars0, numberOfSpeakerPositions ) ; 
   fvec( distanceAmpScalars1, numberOfSpeakerPositions ) ; 
   fvec( rolloffDecibels0, numberOfSpeakerPositions ) ; 
   fvec( rolloffDecibels1, numberOfSpeakerPositions ) ; 

   
   thisAirAbsorptionExponent = sourceIsBehindSpeakers ? 
         airAbsorptionExponentForVirtualSpaceSource : airAbsorptionExponentForRealSpaceSource ;

   sourceInFrontProximityGain = sourceIsBehindSpeakers ? 1. :
      1. / pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / sourceToThresholdProximityDistance ) ), 
              (double) (thisAirAbsorptionExponent) 
        ) ;

   pri( crossFadeSpeaker0, "crossFadeSpeaker0" ) ; 
   pri( crossFadeSpeaker1, "crossFadeSpeaker1" ) ; 
   

   prb( sourceIsBetweenTwoSpeakers, "sourceIsBetweenTwoSpeakers"  ) ; 
   
   if( sourceIsBetweenTwoSpeakers )
   {
      prt( "BETWEEN SPEAKERS" ) ; 
      // CROSSFADE

      // FIND TWO SOURCE CROSSFADE COORDINATES VIA LINE THAT INTERSECTS
      // SPEAKER AND LISTENER POSITIONS.

      // TRANSFER LISTENER TO SPEAKER ANGLES TO TEMP LOCATIONS.
      tempAngles[ 0 ] = listenerToSpeakerAngles[ crossFadeSpeaker0 ] ;
      tempAngles[ 1 ] = listenerToSpeakerAngles[ crossFadeSpeaker1 ] ;

      prf( tempAngles[ 0 ] * 180 / PI, "tempAngles[ 0 ] (in degrees)" ) ;  
      prf( tempAngles[ 1 ] * 180 / PI, "tempAngles[ 1] (in degrees)" ) ;  

      // IF DIFFERENCE BETWEEN THEM IS GREATER THAN 90 DEGREES (HENCE STRADDLING), 
      // THEN ROTATE NEGATIVE VALUE FULL CIRCLE TO CREATE CORRECT PAIR OF ANGLES. 
      prf( listenerToSourceAngle * 180. / PI, "listenerToSourceAngle (in degrees)" ) ; 
      prf( sourceToListenerAngle * 180. / PI, "sourceToListenerAngle (in degrees)" ) ; 

      listenerToSourceAngleTemp = listenerToSourceAngle ;
      if( fabs(tempAngles[ 1 ] - tempAngles[ 0 ]) > PI )
      {
         // ROTATE NEGATIVE ANGLES INTO POSITIVE 
         if( tempAngles[ 0 ] < 0. ) tempAngles[ 0 ] += TWOPI ;
         if( tempAngles[ 1 ] < 0. ) tempAngles[ 1 ] += TWOPI ;
      prf( tempAngles[ 0 ] * 180 / PI, "tempAngles[ 0] (in degrees)" ) ;  
      prf( tempAngles[ 1 ] * 180 / PI, "tempAngles[ 1] (in degrees)" ) ;  
         if( listenerToSourceAngleTemp < 0. ) listenerToSourceAngleTemp += TWOPI ; 
      }

      // MAKE NEW SOURCE POSITION COORDINATES USING THE TWO SPEAKER ANGLES AND THE
      // LISTENER-TO-SOURCE DISTANCE.
      rotatePointToAngle(
         sourceCoordinates, // pointToRotate[], // point to rotate
         listenerCoordinates, // originPoint[], // origin point
         tempAngles[ 0 ], // newAnglePosition, // new angle 
         tempSourceCoordinates0 // newPointPosition[] // new point   
      ) ;
      rotatePointToAngle(
         sourceCoordinates, // pointToRotate[], // point to rotate
         listenerCoordinates, // originPoint[], // origin point
         tempAngles[ 1 ], // newAnglePosition, // new angle 
         tempSourceCoordinates1 // newPointPosition[] // new point   
      ) ;


      printCoordinates( tempSourceCoordinates0, "tempSourceCoordinates0" ) ; 
      printCoordinates( tempSourceCoordinates1, "tempSourceCoordinates1" ) ; 

      // MAKE CROSS FADE PROPORTION
      crossFadeProportion = 1. - 
         ((listenerToSourceAngleTemp - tempAngles[ 0 ]) / (tempAngles[ 1 ] - tempAngles[ 0 ])) ;
      prf( crossFadeProportion, "crossFadeProportion" ) ; 

      // FIND SET OF ANGLE DIFFERENCES FROM EACH RESPECTIVE SOURCE-TO-SPEAKER ANGLE
      // FOR EACH SOURCE.

      // FIRST MAKE BASE ANGLE FROM THIS SOURCE TO THE LISTENER. 
      baseAngle = tempAngles[ 0 ] ;
      if( sourceIsBehindSpeakers )
      {
         baseAngle += + PI ; if( baseAngle > PI ) baseAngle -= TWOPI ; 
      } ;
	// CHANGE THIS: thisSourceToListenerAnglePlusRotation to thisRotatedSource
      if( orient_source__to_room_0__to_listener_1 == 0) 
      	thisRotatedSource = sourceRotation ; // A DIRECT SOUND
      else
      	thisRotatedSource = baseAngle + sourceRotation ; // A DIRECT SOUND

      while( thisRotatedSource > PI ) 
            thisRotatedSource -= TWOPI ; 
      while( thisRotatedSource < (-1. * PI) ) 
            thisRotatedSource += TWOPI ; 


      prt( "SPEAKER0" ) ;
      prf( 180. * baseAngle / PI, "baseAngle (in degrees)" ) ; 
      for( speaker = 0; speaker < numberOfSpeakerPositions; speaker++ )
      {
         // MAKE SEGMENT
         makeSegment( segment, tempSourceCoordinates0, &speakerCoordinates[speaker * 2] ) ;
         if( speaker == crossFadeSpeaker0 )
         {
            limitedDiffAngles0[ speaker ] = 0. ;
            angle = baseAngle ; 
         }else
         {
            // FIND ANGLE
            angle = findSegmentAngle( segment ) ; 
            prf( angle * 180. / PI, "angle (in degrees)" ) ;  
            limitedDiffAngles0[ speaker ] = fmin( PI / 2., fabs( angle - baseAngle ) ) ;
         } ;
         prf( limitedDiffAngles0[ speaker ], "limitedDiffAngles0[ speaker ]" ) ; 
         tempAngleAmpScalars0[ speaker ] = 1. - (limitedDiffAngles0[ speaker ] / 
            (PI / 2.)) ;

         fprintf( stderr, "\n\t%d: %f degrees", speaker, 180. * limitedDiffAngles0[ speaker ] / PI ) ;       
         // FIND LENGTH
         tempSourceToSpeakerDistance = findSegmentLength( segment ) ;         
         // FIND DISTANCE AMP SCALARS 
         distanceAmpScalars0[ speaker ] = pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / tempSourceToSpeakerDistance ) ), 
              (double) (thisAirAbsorptionExponent) 
        ) ; 


         // ROLL OFF DECIBELS
         prf( thisRotatedSource * 180 / PI,
            "thisRotatedSource in degrees" ) ; 
         prf( angle * 180 / PI, "ANGLE FROM TEMP SOURCE TO SPEAKER in degrees" ) ; 
         angleDiff = thisRotatedSource - angle ;
         while( angleDiff > PI ) angleDiff -= TWOPI ; 
         while( angleDiff < (-1. * PI) ) angleDiff += TWOPI ; 
         angleDiff = fabs( angleDiff ) ; 

         prf( angleDiff * 180 / PI, "angleDiff for rolloff in degrees" ) ; 

		// E: IN makeDirectSoundSpeakerAmplitudes
         rolloffDecibels0[ speaker ] = sourceDispersionPatternRolloffInDecibels * 
             (fabs( angleDiff ) / PI) ;
         prf( rolloffDecibels0[ speaker ], "rolloffDecibels0[ speaker ]" ) ; 



      } ;

      // FIRST MAKE BASE ANGLE FROM THIS SOURCE TO THE LISTENER. 
      baseAngle = tempAngles[ 1 ] ;
      if( sourceIsBehindSpeakers)
      {
         baseAngle += + PI ; if( baseAngle > PI ) baseAngle -= TWOPI ; 
      } ;
	// CHANgE THIS: thisSourceToListenerAnglePlusRotation to thisRotatedSource
      if( orient_source__to_room_0__to_listener_1 == 0) 
      	thisRotatedSource = sourceRotation ; // A DIRECT SOUND
      else
      	thisRotatedSource = baseAngle + sourceRotation ; // A DIRECT SOUND

      while( thisRotatedSource > PI ) 
            thisRotatedSource -= TWOPI ; 
      while( thisRotatedSource < (-1. * PI) ) 
            thisRotatedSource += TWOPI ; 



      prt( "SPEAKER1" ) ;
      prf( 180. * baseAngle / PI, "baseAngle (in degrees)" ) ; 
      for( speaker = 0; speaker < numberOfSpeakerPositions; speaker++ )
      {
         // MAKE SEGMENT
         makeSegment( segment, tempSourceCoordinates1, &speakerCoordinates[speaker * 2] ) ;
         if( speaker == crossFadeSpeaker1 )
         {
            limitedDiffAngles1[ speaker ] = 0. ;
            angle = baseAngle ; 
         }else
         {
            // MAKE ANGLE
            angle = findSegmentAngle( segment ) ;
            prf( angle * 180. / PI, "angle (in degrees)" ) ; 
            limitedDiffAngles1[ speaker ] = fmin( PI / 2., fabs( angle - baseAngle ) ) ;
         } ;
         prf( limitedDiffAngles1[ speaker ], "limitedDiffAngles1[ speaker ]" ) ; 
         tempAngleAmpScalars1[ speaker ] = 1. - (limitedDiffAngles1[ speaker ] / 
            (PI / 2.)) ;

         fprintf( stderr, "\n\t%d: %f degrees", speaker, 180. *
              limitedDiffAngles1[ speaker ] / PI ) ;       

         // FIND LENGTH
         tempSourceToSpeakerDistance = findSegmentLength( segment ) ;         
         // FIND DISTANCE AMP SCALARS 
         distanceAmpScalars1[ speaker ] = pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / 
               tempSourceToSpeakerDistance ) ), 
              (double) (thisAirAbsorptionExponent) 
         ) ; 


          // ROLL OFF DECIBELS
         prf( thisRotatedSource * 180 / PI,
            "thisRotatedSource in degrees" ) ; 
         prf( angle * 180 / PI, "ANGLE FROM TEMP SOURCE TO SPEAKER in degrees" ) ; 
         angleDiff = thisRotatedSource - angle ;
         while( angleDiff > PI ) angleDiff -= TWOPI ; 
         while( angleDiff < (-1. * PI) ) angleDiff += TWOPI ; 
         angleDiff = fabs( angleDiff ) ; 

         prf( angleDiff * 180 / PI, "angleDiff for rolloff in degrees" ) ; 

		// A: makeDirectSoundSpeakerAmplitudes (TRACKED)
         rolloffDecibels1[ speaker ] = sourceDispersionPatternRolloffInDecibels * 
             (fabs( angleDiff ) / PI) ;
         prf( rolloffDecibels1[ speaker ], "rolloffDecibels1[ speaker ]" ) ; 

      } ;

      // MAKE FINAL AMP SCALARS USING CROSSFADE OF DIFFERENCE
      for( speaker = 0; speaker < numberOfSpeakerPositions; speaker++ )
      {
         thresholdProximityScalar = thresholdProximityScalarSwitch ?

            ( crossFadeProportion * tempAngleAmpScalars0[ speaker ]) + 
            ( (1. - crossFadeProportion) * tempAngleAmpScalars1[ speaker ]) 
          : 1. ;
         

         directSoundSpeakerAmplitudeScalars[ speaker ] = 
            thresholdProximityScalar * 
            (
               ( crossFadeProportion * distanceAmpScalars0[ speaker ] ) +
               ( (1. - crossFadeProportion) * distanceAmpScalars1[ speaker ]  )
            )  *
            (
               ( crossFadeProportion * dB_to_amp( rolloffDecibels0[ speaker ] ) ) + 
               ( (1. - crossFadeProportion) * dB_to_amp( rolloffDecibels1[ speaker ] ) ) 
            ) * 
             // FRONT SOURCE COMPENSATION GAIN
             sourceInFrontProximityGain * 
             // HEAD ROOM GAIN REDUCTION FOR FRONT
             frontSourceHeadRoomScalar *
             // SOURCE COMPENSATION
             dB_to_amp( direct_sound_gain_in_decibels ) 
             ;
      } ;

      
   }else{
      // NON-CROSSFADE

      for( speaker = 0; speaker < numberOfSpeakerPositions; speaker++ )
      {
         makeSegment( segment, sourceCoordinates, &speakerCoordinates[speaker * 2] ) ;
            // MAKE ANGLE
         angle = findSegmentAngle( segment ) ;
         // FIND LENGTH
         tempSourceToSpeakerDistance = findSegmentLength( segment ) ;         

         // ROLL OFF DECIBELS
         angleDiff = sourceToListenerAnglePlusRotation - angle ;
         while( angleDiff > PI ) angleDiff -= TWOPI ; 
         while( angleDiff < (-1. * PI) ) angleDiff += TWOPI ; 
         angleDiff = fabs( angleDiff ) ; 

		// B: makeDirectSoundSpeakerAmplitudes
         rolloffDecibels = sourceDispersionPatternRolloffInDecibels * 
             (fabs( angleDiff ) / PI) ;
         prf( rolloffDecibels, "rolloffDecibels" ) ; 

         thresholdProximityScalar = thresholdProximityScalarSwitch ?
            (float) cos( 
              (double) fmin( PI / 2., fabs( findSegmentAngle( segment ) - 
                sourceToListenerAngle ) ) 
            ) :
            1. ;                    

         prf( thresholdProximityScalar, "thresholdProximityScalar" ) ; 

         directSoundSpeakerAmplitudeScalars[ speaker ] = 
             // THRESHOLD PROXIMITY SCALAR
            thresholdProximityScalar                    
               *
            // AIR ABSORPTION SCALAR
            pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / 
                  tempSourceToSpeakerDistance ) ), 
              (double) (thisAirAbsorptionExponent) 
            )           
             * 
             // FRONT SOURCE COMPENSATION GAIN
             sourceInFrontProximityGain    
             * 
             // HEAD ROOM GAIN REDUCTION FOR FRONT
             frontSourceHeadRoomScalar   
                                          
             * 
             // SOURCE COMPENSATION
             dB_to_amp( rolloffDecibels ) * dB_to_amp( direct_sound_gain_in_decibels )
         ; 

      } ;
// 
   } ;
   prt( "DIRECT SOUND SPEAKER AMPLITUDE SCALARS" ) ; 

   prf( maxSpeakerToForwardSourceDistanceLimit, "maxSpeakerToForwardSourceDistanceLimit" ) ; 
   prf( amp_to_dB( frontSourceHeadRoomScalar ), "frontSourceHeadRoomScalar in dB" ) ; 

   prf( sourceToThresholdProximityDistance, "sourceToThresholdProximityDistance" ) ;    
   prf( amp_to_dB( sourceInFrontProximityGain ), "sourceInFrontProximityGain in dB" ) ; 
   prf( thisAirAbsorptionExponent, "thisAirAbsorptionExponent" ) ; 
   prf( crossFadeProportion, "crossFadeProportion" ) ; 

   for( speaker = 0; speaker < numberOfSpeakerPositions; speaker++ )
   {
      fprintf( stderr, "\n\t%d: %f (%f dB)", speaker, directSoundSpeakerAmplitudeScalars[ speaker ],
          amp_to_dB( directSoundSpeakerAmplitudeScalars[ speaker ] ) ) ;       
   } ;


} ;

void makeSourceToThresholdProximityDistance()
{ // ???
    
   
   
   float segment0[ 4 ] ;
   float segment1[ 4 ] ;
   
   

// int crossFadeSpeaker0, crossFadeSpeaker1

   // FIND THE PAIR OF ADJACENT listenerToSpeakerAngles BETWEEN WHICH THE 
   // listenerToSourceAngle LIES. 
   sourceIsBetweenTwoSpeakers = false ; 
   crossFadeSpeaker0 = 0 ;

   
   while( (sourceIsBetweenTwoSpeakers == false) && 
      (crossFadeSpeaker0 < ((numberOfSpeakerPositions - 1) 
        + speaker_configuration__sequence_0__polygon_1)) )
   {
      crossFadeSpeaker1 = (crossFadeSpeaker0 + 1) % numberOfSpeakerPositions ;

      // IF THIS PAIR OF SPEAKER COORDINATES ARE THE STRADDLING MIN-MAX 
      // COORDINATES . . .  
      if( listenerToSpeakerAngleStraddleFlags[ crossFadeSpeaker0 ] == 1 )
      {   // IN STRADDLE REGION

         // THEN TEST THE listenerToSourceAngle TO SEE IF IT IS BETWEEN THE ANGLES.
         // .
         if(
            (listenerToSourceAngle > listenerToSpeakerAngleMaximum) || 
            (listenerToSourceAngle < listenerToSpeakerAngleMinimum)
         )
         {
            // IT IS BETWEEN THE TWO.
            sourceIsBetweenTwoSpeakers = true ; speakersStraddle = true ;   
         } ;
      } else
      { // THIS SPEAKER PAIR IS NOT THE STRADDLE PAIR. 
        // SO HANDLE MIN-MAX NORMALLY. 

         // TEST THE listenerToSourceAngle TO SEE IF IT IS BETWEEN THE MIN-MAX PAIR, 
         // INCLUSIVE OF THE BOUNDARIES. 
         if( 
            valueIsBetweenTheseTwo( 
               listenerToSourceAngle, 
               listenerToSpeakerAngles[ crossFadeSpeaker0 ],
               listenerToSpeakerAngles[ crossFadeSpeaker1 ]
            )
         )
         {
            // IT IS BETWEEN THE TWO.
            sourceIsBetweenTwoSpeakers = true ;  speakersStraddle = false ; 
         } ; 
      } ;
      if( sourceIsBetweenTwoSpeakers == false ) crossFadeSpeaker0 ++ ;   
   } ;

   // MAKE DISTANCE IF sourceIsBetweenTwoSpeakers
   if( sourceIsBetweenTwoSpeakers ){
      prt( "ANGLE SPACE sourceIsBetweenTwoSpeakers" ) ; 
      pri( crossFadeSpeaker0, "crossFadeSpeaker0" ) ; 
      pri( crossFadeSpeaker1, "crossFadeSpeaker1" ) ;  

      // MAKE SPEAKER-TO-SPEAKER THRESHOLD SEGMENT
      makeSegment(
         segment0,
         &speakerCoordinates[crossFadeSpeaker0 * 2],
         &speakerCoordinates[crossFadeSpeaker1 * 2]
      ) ;
      // MAKE LISTENER-TO-SOURCE SEGMENT
      makeSegment( segment1, listenerCoordinates, sourceCoordinates ) ;

      // FIND INTERSECTION OF LISTENER TO SOURCE AND SPEAKER SEGMENT.
      printSegment( segment0, "SPEAKER THRESHOLD SEGMENT" ) ; 
      printSegment( segment1, "LISTENER TO SOURCE SEGMENT" ) ; 

      examineSegmentsForIntersection( 
         segment0, segment1, 
         thresholdAndListenerToSourceIntersection, true  
      ) ;

      makeSegment( segment0, thresholdAndListenerToSourceIntersection, sourceCoordinates ) ;
      sourceToThresholdProximityDistance = findSegmentLength( segment0 ) ; 

      prf( sourceToThresholdProximityDistance, "sourceToThresholdProximityDistance" ) ; 

   } ;


   // SEQUENCE
   if( (speaker_configuration__sequence_0__polygon_1 == 0) && (sourceIsBetweenTwoSpeakers == false) )
   {
      // FIND WHICH OF THE SPEAKER SEQUENCE FIRST AND LAST POINTS IS CLOSER TO THE SOURCE POINT.
      // FIRST FIND THE LENGTH OF SOURCE TO EACH.
      // SOURCE TO FIRST SPEAKER. 
      makeSegment( 
         segment0,   
         sourceCoordinates,
         &speakerCoordinates[ 0 ]
      ) ;
      // SOURCE TO LAST SPEAKER      
      makeSegment( 
         segment1,   
         sourceCoordinates,
         &speakerCoordinates[ (numberOfSpeakerPositions - 1) * 2  ]
      ) ;      
      if( findSegmentLength( segment0 ) < findSegmentLength( segment1 )  )
      {   // SOURCE IS CLOSER TO FIRST SPEAKER IN SEQUENCE
         crossFadeSpeaker0 = crossFadeSpeaker1 = 0 ;

         makeSegment(
            segment0,
            &speakerCoordinates[ 0 ],
            &speakerCoordinates[ 2 ]
         ) ;

      }else
      {   // SOURCE IS CLOSER TO LAST SPEAKER IN SEQUENCE
         crossFadeSpeaker0 = crossFadeSpeaker1 = numberOfSpeakerPositions - 1 ;

         makeSegment(
            segment0,
            &speakerCoordinates[ (numberOfSpeakerPositions - 1) * 2  ],
            &speakerCoordinates[ (numberOfSpeakerPositions - 2) * 2  ]
         ) ;
      } ;
      // MAKE LISTENER TO SOURCE SEGMENT
      makeSegment(
         segment1,
         listenerCoordinates,
         sourceCoordinates
      ) ;
      if( findIntersectionOfLinesContainingSegments( segment0, segment1,
              thresholdAndListenerToSourceIntersection ) == false 
      )
      {
         prt( "PROBLEM WITH INTERSECTION WITH PROJECTED THRESHOLD BEYOND SPEAKER SEQUENCE." );
         exit(EXIT_FAILURE);
      }  ;

      // MAKE SOURCE TO PROJECTED THRESHOLD LINE POINT
      makeSegment(
         segment0,
         thresholdAndListenerToSourceIntersection,
         sourceCoordinates
      ) ;
      // FIND LENGTH OF SEGMENT
      sourceToThresholdProximityDistance = findSegmentLength( segment0 ) ;

   } ;


   printCoordinates( thresholdAndListenerToSourceIntersection, 
	"thresholdAndListenerToSourceIntersection" ) ; 


} ;

void isSourceBehindOrInFrontOfSpeakerThreshold()
{
   float listenerToSourceSegment[ 4 ] ;
   float listenerToSpeakerThresholdSegment[ 4 ] ;
   float listenerToSourceSegmentLength ;
   float listenerToSpeakerThresholdSegmentLength ;

   makeSegment(
      listenerToSourceSegment,
      listenerCoordinates,
      sourceCoordinates
   ) ;

   makeSegment(
      listenerToSpeakerThresholdSegment,
      listenerCoordinates,
      thresholdAndListenerToSourceIntersection
   ) ;

   listenerToSourceSegmentLength = findSegmentLength( listenerToSourceSegment ); 
   listenerToSpeakerThresholdSegmentLength = findSegmentLength( listenerToSpeakerThresholdSegment ); 

   prf( listenerToSourceSegmentLength, "listenerToSourceSegmentLength" ) ; 
   prf( listenerToSpeakerThresholdSegmentLength, "listenerToSpeakerThresholdSegmentLength" ) ; 

   sourceIsBehindSpeakers = 
      ( listenerToSourceSegmentLength > listenerToSpeakerThresholdSegmentLength ) ?
      true : false ; 
   sourceSpeakerOrientationSign = sourceIsBehindSpeakers ? 1. : -1. ; 


   prb( sourceIsBehindSpeakers, "sourceIsBehindSpeakers" ) ; 
   prf( sourceSpeakerOrientationSign, "sourceSpeakerOrientationSign" ) ; 
} ;


void findListenerToSourceSegmentLengthAndAngle()
{

     // ****** FIND THE LISTENER TO SOURCE ANGLE
      // FIRST FILL SEGMENT
      printCoordinates( listenerCoordinates, "listenerCoordinates" ) ; 
      printCoordinates( sourceCoordinates, "sourceCoordinates" ) ; 
      makeSegment( 
         listenerToSourceSegment,
//         &listenerCoordinates[ (channel % numberOfListenerPositions) * 2 ],
//         &sourceCoordinates[ (channel % numberOfSourcePositions) * 2 ]
         listenerCoordinates,
         sourceCoordinates
      ) ;
      // FIND LENGTH
      printSegment( listenerToSourceSegment, "listenerToSourceSegment SEGMENT" ) ;
      listenerToSourceLength = findSegmentLength( listenerToSourceSegment ); 
      // THEN FIND ANGLE
      listenerToSourceAngle = findSegmentAngle( listenerToSourceSegment ); 
      sourceToListenerAngle =  ( listenerToSourceAngle + PI ) ;
      if( sourceToListenerAngle > PI ) sourceToListenerAngle -= TWOPI ;     
 
} ;

bool findIntersectionOfLinesContainingSegments(
   float w[],
   float p[],
   float intersectCoordinates[]
)
{
   // w and p are line seg arrays of two points containing x and y.

   float x, y ;
   bool segmentsIntersect = true ;
   bool print=false ; 
   float mw, mp, bw, bp;
   float  wxlow, wxhigh, wylow, wyhigh, pxlow, pxhigh, pylow, pyhigh  ;
   
   if( (w[ 2 ] - w[ 0 ]) == 0.0 ){
      if( print )prt( "w SEGMENT PARALLEL TO Y-AXIS" ) ; 
      if( (p[ 2 ] - p[ 0 ]) == 0.0 ){
         // BOTH p AND w SEGMENT PARALLEL TO Y-AXIS
//         x = nil ; y = nil ; 
         segmentsIntersect = false ; 
      } else {
         if( print )prt( "w PARALLEL, p SEGMENT NOT PARALLEL." ) ; 
         mp = (p[ 3 ] - p[ 1 ]) / (p[ 2 ] - p[ 0 ]) ; bp = p[ 1 ] - (mp * p[ 0 ]) ;
         x = w[ 0 ] ; y = (mp * x) + bp ; 
/*
         if( (  ( (y >= w[ 1 ]) && (y <= w[ 3 ]) ) || ( (y >= w[ 3 ]) && (y <= w[ 1 ]) )  )
               &&
            (  ( (x >= p[ 0 ]) && (x <= p[ 2 ]) ) || ( (x >= p[ 2 ]) && (x <= p[ 0 ]) )   ) )
         { segmentsIntersect = true ; } else { segmentsIntersect = false ; } ;
*/
         segmentsIntersect = true ;
      } ; 
   } else {
      if( print )prt( "w SEGMENT NOT PARALLEL." ) ; 
      mw = (w[ 3 ] - w[ 1 ]) / (w[ 2 ] - w[ 0 ]) ; bw = w[ 1 ] - (mw * w[ 0 ]) ;
      if( (p[ 2 ] - p[ 0 ]) == 0.0 ){
         if( print )prt( "w SEGMENT NOT PARALLEL, p SEGMENT PARALLEL." ) ; 
         x = p[ 0 ] ; y = (mw * x) + bw ; 
/*
         if( (  ( (y >= p[ 1 ]) && (y <= p[ 3 ]) ) || ( (y >= p[ 3 ]) && (y <= p[ 1 ]) )  )
               &&
            (  ( (x >= w[ 0 ]) && (x <= w[ 2 ]) ) || ( (x >= w[ 2 ]) && (x <= w[ 0 ]) )   ) )
         {segmentsIntersect = true ; } else { segmentsIntersect = false ; } ;
*/
         segmentsIntersect = true ;

      } else {
         if( print )prt( "NEITHER SEGMENTS PARALLEL TO Y-AXIS." ) ; 
         mp = (p[ 3 ] - p[ 1 ]) / (p[ 2 ] - p[ 0 ]) ; bp = p[ 1 ] - (mp * p[ 0 ]) ;
         if( mp == mw ){
            if( print )prt( "w AND p PARALLEL TO EACH OTHER." ) ; 
//            x = nil ; y = nil ; 
            segmentsIntersect = false ; 
         } else {
            if( print )prt( "NO PARALLELISMS OF ANY KIND." ) ; 
            x =  (bw - bp) / (mp - mw) ; y = (mp * x) + bp ; 
            // FIND LOWS AND HIGHS FOR w AND p
            if( w[ 0 ] <= w[ 2 ] ){
               wxlow = w[ 0 ]; wxhigh = w[ 2 ] ;
            } else {
               wxlow = w[ 2 ]; wxhigh = w[ 0 ] ;
            }; 
            if( w[ 1 ] <= w[ 3 ] ){
               wylow = w[ 1 ]; wyhigh = w[ 3 ] ;
            } else {
               wylow = w[ 3 ]; wyhigh = w[ 1 ] ;
            }; 

            if( p[ 0 ] <= p[ 2 ] ){
               pxlow = p[ 0 ]; pxhigh = p[ 2 ] ;
            } else {
               pxlow = p[ 2 ]; pxhigh = p[ 0 ] ;
            } ; 
            if( p[ 1 ] <= p[ 3 ] ){
               pylow = p[ 1 ]; pyhigh = p[ 3 ] ;
            } else {
               pylow = p[ 3 ]; pyhigh = p[ 1 ] ;
            } ; 

            segmentsIntersect = true ;
/*
            if( 
               (x >= wxlow) && (x <= wxhigh) && 
               (y >= wylow) && (y <= wyhigh) &&
               (x >= pxlow) && (x <= pxhigh) && 
               (y >= pylow) && (y <= pyhigh)
            ){segmentsIntersect = true ; } else {segmentsIntersect = false ; };            
*/
         }; 
      };    
   }; 
   intersectCoordinates[ 0 ] = x ; intersectCoordinates[ 1 ] = y ; 
//   if( segmentsIntersect && print ) prt( "LINES INTERSECT" ) ; else prt( "LINES DO NOT INTERSECT" ) ;

   return( segmentsIntersect ) ; 
} ;

void readInWallImpulseResponses()
{
   int n, wall ;
   int channel, frame ; 
   int numberOfSampsBufferedIn ;
      


   if( (strcasecmp( wallImpulseResponseInputSoundFileName, datafile2 ) == 1) ||
       (strlen( wallImpulseResponseInputSoundFileName ) == 0) ||
       (wall_impulse_responses__off_0__on_1 == 0)
    ){
      if( wallImpulseResponsesFromSoundFileFlag )
      {
         prt( "\n\n---------ERROR: SOUND FILE WALL IMPULSE RESPONSES IS ON," ) ; 
         prt( "             BUT NO WALL IMPULSE RESPONSE SOUND FILE IS SPECIFIED.\n\n" ) ; 
         prt( "YOU MUST SPECIFY A WALL IMPULSES SOUND FILE TO USE THE SOUND FILE WALL IMPULSE RESPONSE MODE.");
         prt( "\n. . . BYE."); 
         exit( EXIT_FAILURE ) ;   
      }else
      { 
         // WALL IMPULSE RESPONSES IS OFF.
         // PUT PULSES IN FOR WALLS.
         prt( "WALL IMPULSES TURNED OFF. WILL USE PULSES." ) ;
         wallPulseModeFlag = true ; 

         // SET WALL IMPULSE CROP FLAG TO true. 
         noCropImpulseResponsesFlag = true ; 
//         convolveWallImpulseResponses = true ; 


         wall_isr = osr ; 
         wall_isrDouble = (double) wall_isr ; 
         wall_numberOfInputChannels  = 1 ;
         wall_numberOfFrames = (long int) 16384 ;
         wall_inputDuration  = (float)((double) wall_numberOfFrames / wall_isrDouble) ; // 


         // MAKE MEMORY FOR AUDIO DATA
         fvec( wallImpulseResponses, numberOfWallChannelAssignments * wall_numberOfFrames ) ; 
         for(n = 0; n < numberOfWallChannelAssignments; n++ )
            wallImpulseResponses[ (n * wall_numberOfFrames) ] = 1. ;

         wall_numberOfInputChannels = numberOfWallChannelAssignments ;

         return ;
      } ;

   } else {
      // FILE SPECIFIED. TRY TO OPEN. 
      if(! (inputWallImpulseResponseFilePointer = 
           sf_open (wallImpulseResponseInputSoundFileName, SFM_READ, 
           &inputWallImpulseResponseSFinfo ))
      )
      {
         // FILE DOES NOT EXIST
         prs( wallImpulseResponseInputSoundFileName, "WALL IMPULSE RESPONSE SOUND FILE" ) ; 
         prt( "------> FILE NOT FOUND\n\n . . . . .  BYE.\n\n" ) ;
         exit( EXIT_FAILURE ) ; 
      }else
      {
         prt( "WALL IMPULSE RESPONSES FILE FOUND." ) ; 
         prs( wallImpulseResponseInputSoundFileName, "WALL IMPULSE RESPONSE(S) SOUND FILE" ) ; 

         // OPEN FILE AND PARSE INTO MEMORY BY CHANNELS.
 	 // OPEN INPUT SOUND FILE IN READ MODE TO GET FORMAT. 
//         convolveWallImpulseResponses = true ;  

         wallPulseModeFlag = false ;

         wall_isr = inputWallImpulseResponseSFinfo.samplerate ; 
         wall_isrDouble = (double) wall_isr ; 
         wall_numberOfInputChannels  = inputWallImpulseResponseSFinfo.channels ;
         wall_numberOfFrames = (long int) inputWallImpulseResponseSFinfo.frames ;
         wall_inputDuration  = (float)((double) wall_numberOfFrames / wall_isrDouble) ; // 
         
         prbanner( "WALL IMPULSE RESPONSE SOUND FILE PARAMETERS:", 69 ) ; 
         fprintf( stderr, "\n -- CHANNELS: %d  SAMPLE RATE: %d FRAMES: %d  DURATION: %f", 
                        wall_numberOfInputChannels, wall_isr, (int) wall_numberOfFrames, wall_inputDuration ) ; 
         wall_iformat = inputWallImpulseResponseSFinfo.format ; 

         // CHECK FOR ASSIGNMENT OF NON-EXISTENT CHANNELS.
         for(n = 0; n < numberOfWallChannelAssignments; n++)
         {
            // FOR EVERY CHANNEL
            if( wallChannelAssignments[ n ] > wall_numberOfInputChannels )
            {
               prt( "\n\n----- ERROR: WALL IMPULSE RESPONSE CHANNEL ASSIGNMENTS" ) ; 
               fprintf( stderr, "\n\nWALL IMPULSE RESPONSE SOUND FILE CHANNELS: 1 - %d",
                     wall_numberOfInputChannels ) ; 
               prt( "\nWALL IMPULSE RESPONSE CHANNEL ASSIGNMENTS:\n" ) ;
               for( n = 0; n < numberOfWallChannelAssignments; n++ )
                  fprintf( stderr, "%d ", wallChannelAssignments[ n ] ) ; 
               prt( "\nONE OR MORE CHANNEL ASSIGNMENTS EXCEEDS THE NUMBER OF AVAILABLE CHANNELS." ) ; 
               prt( "\n. . .  BYE.\n\n" ) ; 
               exit(EXIT_FAILURE); 
            } ; 
         } ;
      } ;

      // MAKE MEMORY FOR AUDIO DATA
      fvec( wallImpulseResponses, numberOfWallChannelAssignments * wall_numberOfFrames ) ; 
      fvec( wallImpulseResponsesInterleaved, wall_numberOfInputChannels * wall_numberOfFrames ) ; 


      // FOR EACH CHANNEL, READ IN AUDIO FROM CHANNEL INTO MEMORY.
      sf_seek( inputWallImpulseResponseFilePointer, 0, SEEK_SET ) ;  // REWIND	

      // READ AUDIO INTO INTERLEAVED ARRAY
	 numberOfSampsBufferedIn = 
        sf_read_float (inputWallImpulseResponseFilePointer, 
             wallImpulseResponsesInterleaved, wall_numberOfInputChannels * wall_numberOfFrames ) ;

      // FOR EACH WALL, READ THE AUDIO FROM THE ASSIGNED CHANNEL INTO THE 
      // NON-INTERLEAVED ARRAY OF WALL RESPONSES. 
      for( wall = 0; wall < numberOfWallChannelAssignments; wall++ ){
         channel = wallChannelAssignments[ wall ] - 1 ;  // %%%
         for( frame = 0; frame < wall_numberOfFrames; frame++ )
         {
            wallImpulseResponses[ (wall * wall_numberOfFrames) + frame ] = 
               wallImpulseResponsesInterleaved[ (frame * wall_numberOfInputChannels) + channel ] ; // %%%
         } ;
      } ;

      wall_numberOfInputChannels = numberOfWallChannelAssignments ;

   } ;

   sf_close( inputWallImpulseResponseFilePointer ) ; 

} ;

void findWallImpulseResponsesCroppedSize()
{
   int channel, n, peakAmp ; 

   pri( numberOfWallChannelAssignments, "numberOfWallChannelAssignments" ) ; 
   prt( "BEFORE ivec" ) ; 
   ivec( wallImpulseResponsesCroppedSize, numberOfWallChannelAssignments ) ; 
   prt( "AFTER ivec" ) ; 

   for( channel = 0; channel < numberOfWallChannelAssignments; channel++ )
   {

      if( noCropImpulseResponsesFlag )
      {
         wallImpulseResponsesCroppedSize[ channel ] = wall_numberOfFrames ;
      }else
      {
         n = (int) wall_numberOfFrames ;
         peakAmp = findPeakAmp( 
            &wallImpulseResponses[ channel * wall_numberOfFrames ], 
            n 
         ); 
         wallImpulseResponsesCroppedSize[ channel ] = cropEndForSilence(
            &wallImpulseResponses[ channel * wall_numberOfFrames ],
            &n, 
            peakAmp * dB_to_amp( endCropDecibelThreshold ),
            false
         ) ;

         smoothReleaseOfCroppedEnd( 
           &wallImpulseResponses[ channel * wall_numberOfFrames ], 
           wallImpulseResponsesCroppedSize[ channel ], 
           endCropReleaseTime  
         ) ;
      } ;

   } ;

} ;


void convolveTwoArrays(
   float array0[],
   int Lh0,
   float array1[],
   int Lh1
)
{
   static int L0, N, N2, previousN=-1, previousLh0m1=-1, i, j, k, Lh0m1 ;
   static int Nw = 2048, temp ; // ?
   
   static int sampsToRead ;
   static int array1Index ;
   static float real, imag ;

   
    
   float fundamental ; 


   

   prt( "CONVOLVING" ) ; 
   array1Index = 0 ;

	// IMPULSE LENGTH
   L0 = 2 * Lh0 - 1 ; 
   for( N = 1; N < L0; N <<= 1 ) ; 
   N2 = N>>1 ; 
   Lh0m1 = Lh0 - 1 ; L0 = 2 * Lh0 - 1 ;

//   prt("Q0" ) ; 


//   pri( N, "N" ) ; pri( previousN, "previousN" ) ; 
//   pri( Lh0m1, "Lh0m1" ) ; pri( previousLh0m1, "previousLh0m1" ) ;  

   fundamental =  ((float) sampleRate / (float) N) ; 

//   prt("Q1" ) ; 

   //*****
   //******  WINDOW SETUP/ADJUSTMENT ***************************
   // MAKE WINDOW SIZE TWICE FFT IF SET TO 0
   if( Nw <= 0 ) Nw = 2 * N ; // ?
//   prt("Q2" ) ; 

   if( previousN == -1 )
   {
//   prt("Q3" ) ; 
      // FIRST TIME; INIT ALL MEMORY
      prt( "A FIRST TIME - MAKING SPACE cvBuffer, cvInBuffer,  cvOutBuffer, BthisB" ) ; 
   //   prt( "cvBuffer" ) ; 
      fvec( cvBuffer, N ) ;
   //   prt( "cvInBuffer" ) ; 
      fvec( cvInBuffer, N ) ;      
   //   prt( "cvOutBuffer" ) ; 
      fvec( cvOutBuffer, N ) ;
   //   prt( "MAKING BthisB" ) ; 
//      pri( Lh0m1, "Lh0m1" ) ;
      fvec( BthisB, Lh0m1 ) ;
   //   prt( "BthisB AFTER MAKING" ) ; 
      previousN = N ; previousLh0m1 = Lh0m1 ; 
   } ;
//   prt("Q4" ) ; 

   if( convolveTwoArraysReset )
   {
      // RESET; REINIT ALL MEMORY
      prt( "RESETTING convolveTwoArrays . . . " ) ; 
      prt( "A FREEING SPACE: cvBuffer, cvInBuffer,  cvOutBuffer, BthisB" ) ; 

      //   prt( "cvBuffer" ) ; 
      free( cvBuffer ) ; 
      //   prt( "cvInBuffer" ) ; 
      free( cvInBuffer ) ; 
   //   prt( "cvOutBuffer" ) ; 
      free( cvOutBuffer ) ; 
   //   prt( "FREEING BthisB" ) ; 
      free( BthisB ) ; 

      prt( "B MAKING SPACE: cvBuffer, cvInBuffer,  cvOutBuffer, BthisB" ) ; 

      fvec( cvBuffer, N ) ;
      fvec( cvInBuffer, N ) ;      
      fvec( cvOutBuffer, N ) ;
   //   prt( "MAKING BthisB" ) ; 
      fvec( BthisB, Lh0m1 ) ;
      previousN = N ; previousLh0m1 = Lh0m1 ; 
      convolveTwoArraysReset = false ; 

   } ;
//   prt("Q5" ) ; 

   if( N > previousN )
   {
      prt( "B FREEING SPACE: cvBuffer, cvInBuffer,  cvOutBuffer" ) ; 
   //   prt( "cvBuffer" ) ;
      free( cvBuffer ) ;
   //   prt( "cvInBuffer" ) ; 
      free( cvInBuffer ) ;      
      free( cvOutBuffer ) ;

      prt( "MAKING SPACE: cvBuffer, cvInBuffer,  cvOutBuffer" ) ; 
      fvec( cvBuffer, N ) ;
      fvec( cvInBuffer, N ) ;      
   //   prt( "cvOutBuffer" ) ; 
      fvec( cvOutBuffer, N ) ;
      previousN = N ; 
   } ;

//   pri( Lh0m1, "Lh0m1" ) ; pri( previousLh0m1, "previousLh0m1" ) ;  
//   prt("Q6" ) ; 

   if( Lh0m1 > previousLh0m1 )
   {
   //   prt( "FREEING BthisB" ) ;
      free( BthisB ) ;
   //   prt( "making new BthisB" ) ;
      fvec( BthisB, Lh0m1 ) ;
      previousLh0m1 = Lh0m1 ;
   } ;

//   prt("Q7" ) ; 
 

   temp = Lh0 + Lh1  ;   
   convolutionOutputMemorySize = 0 ; 
   while( convolutionOutputMemorySize < temp ) convolutionOutputMemorySize += N ;


   if( previousConvolutionOutputMemorySize == -1 ){   
      fvec( convolutionOutput, convolutionOutputMemorySize ) ;
      previousConvolutionOutputMemorySize = convolutionOutputMemorySize ; 
   }else{
      if( convolutionOutputMemorySize > previousConvolutionOutputMemorySize ){
         free( convolutionOutput ) ; 
         fvec( convolutionOutput, convolutionOutputMemorySize ) ;
         previousConvolutionOutputMemorySize = convolutionOutputMemorySize ; 
      } ;
   } ;
   convolutionOutputSize = 0 ;


   for(i = 0; i < N; i++) cvBuffer[i] = 0. ;

   // TRANSFER FIRST ARRAY INTO BUFFER.
   for(i = 0; i < Lh0; i++) cvBuffer[ i ] = array0[ i ] ;

   // TRANSFORM TO COMPLEX FREQ
   rfft( cvBuffer, N2, FORWARD ) ;


   // ZERO B ARRAY
//   pri( Lh0m1, "Lh0m1" ) ; 

   for(i = 0; i < Lh0m1; i++) BthisB[i] = 0. ; 


   while ( array1Index < Lh1 ) {


      // ZERO cvInBuffer ; 
      for(i = 0; i < N; i++) cvInBuffer[i] = 0. ;  

      // READ IN INPUT 
      sampsToRead = ( (Lh1 - array1Index) < Lh0 ) ? (Lh1 - array1Index) : Lh0 ;

      for (i = 0, k = array1Index; i < sampsToRead; i++, k++ ) cvInBuffer[i] = array1[ k ] ;

      rfft( cvInBuffer, N2, FORWARD ) ;


      // CONVOLVE FFT THROUGH COMPLEX MULTIPLY  
      cvOutBuffer[0] = cvInBuffer[0] * cvBuffer[0] ; 
      cvOutBuffer[1] = cvInBuffer[1] * cvBuffer[1] ; 
      for(i = 2, j = 3;  i < N; i += 2, j += 2){
         real = (cvInBuffer[i] * cvBuffer[i]) - (cvInBuffer[j] * cvBuffer[j]) ; 
         imag = (cvInBuffer[i] * cvBuffer[j]) + (cvInBuffer[j] * cvBuffer[i]) ; 
         cvOutBuffer[i] = real ; cvOutBuffer[j] = imag ; 
      } ;


      rfft( cvOutBuffer, N2, INVERSE ) ;



      // OVERLAP/ADD
       for( i = 0; i < Lh0m1; i++ ){
         cvOutBuffer[i] += BthisB[i] ; 
         BthisB[i] = cvOutBuffer[Lh0 + i] ; 
      } ;

      convolutionOutputSize = array1Index  + sampsToRead ;
      for( i = 0, k = array1Index ; i < sampsToRead; i++, k++ ) 
         convolutionOutput[ k ] = cvOutBuffer[ i ] ;

      array1Index += sampsToRead ;

   } ;

   // FLUSH OUT B
   convolutionOutputSize += Lh0m1 ;
   for( i = 0, k = array1Index ; i < Lh0m1; i++, k++ ) 
      convolutionOutput[ k ] = BthisB[ i ] ;



} ;





int cropEndForSilence(
   float array[],
   int *size, 
   float threshold,
   bool shortenMemory
)
{
   int index, i, sameSize ;
   

   index = *size - 1 ;
   shortenMemory = false ;

   while( (array[ index ] < threshold) && (index >= 0) ) index-- ;  

   if( index < (*size - 1) )
   {
      // CROPPING
      if( shortenMemory )
      { // SHORTEN MEMORY
         fvec( endCropTempArray, *size ) ;
         for( i = 0; i < *size; i++ ) endCropTempArray[ i ] = array[ i ] ;
         free( array ) ;
         *size = index + 1 ; 
         fvec( array, *size ) ; 
         for( i = 0; i < *size; i++ ) array[ i ] = endCropTempArray[ i ] ;
         free( endCropTempArray ) ; 
         return( index + 1 ) ; 
      } else
      {
         // 
         return( index + 1 ) ;
      } ;

   }else
   {
      sameSize = *size ;
      return( sameSize ) ; 
   } ;


};

int cropIR_DataEndForSilence(
   float threshold,
   bool shortenMemory
)
{
   int index;
   

   index = IR_DataLength - 1 ;
   shortenMemory = false ;

   while( (IR_Data[ index ] < threshold) && (index >= 0) ) index-- ;  

   index += ((float) osr * endCropReleaseTime ) ;
   if( index > IR_DataLength ) index = IR_DataLength ;  
   return( index ) ; 

};




float findPeakAmp(
   float array[],
   int size
)
{
   float peakAmp=0., absvalAmp ;
   int n ; 

   for( n = 0; n < size; n++ )
   {
      absvalAmp = fabs( array[ n ] ) ;
      if(  absvalAmp > peakAmp) peakAmp = absvalAmp ; 
   } ;
   if( peakAmp == 0. )prt( "========> WARNING: PEAK AMP FOUND IS 0. <=========" ) ; 

   return( peakAmp ) ; 
} ;

void filterFFT(
   float fftArray[],
   int N,
   float fundamental, 
   float lowFreq,
   float highFreq,
   float lowRolloffInDBperOctave,
   float highRolloffInDBperOctave,
   int compoundLevels
)
{
   float binFreq, rolloffdB, rolloffAmp  ; 
   int i, j, k, l ; 

   for( i = 0, j = 1, k = 0; k < (N / 2); i += 2, j += 2, k++ ){
      binFreq = (float) k * fundamental ; 
      if( binFreq < lowFreq ){

         if( k == 0 ){
             fftArray[i] = 0.0 ; fftArray[j] = 0.0 ;  

         }else{
              rolloffdB = 
                ((Hz_to_MIDI( lowFreq ) - Hz_to_MIDI( binFreq )) / 12.0) *
                  lowRolloffInDBperOctave ;
              rolloffAmp = dB_to_amp( rolloffdB ) ;

              for(l = 0; l < compoundLevels; l++)
              {
                 fftArray[i] *= rolloffAmp ; fftArray[j] *= rolloffAmp ;
              } ;
        } ; 

      }else if( binFreq > highFreq ){
           rolloffdB = ((Hz_to_MIDI( binFreq ) - Hz_to_MIDI( highFreq )) / 12.0) *
                  highRolloffInDBperOctave ;
             
           rolloffAmp = dB_to_amp( rolloffdB ) ;
           for(l = 0; l < compoundLevels; l++)
           {
              fftArray[i] *= rolloffAmp ; fftArray[j] *= rolloffAmp ;
           } ;
      } ; 
   } ; 
} ;

void filterAudioArray(
   float sampleRate,
   float lowFreq,
   float highFreq,
   float lowRolloffInDBperOctave,
   float highRolloffInDBperOctave,
   int compoundLevels
)
{

//float 	*audioArrayForFilter ;
// int 	*lengthOfAudioArrayForFilter ;


   float fundamental ;
   int L, N, N2, lengthMinus1, i;
   float *filtBuffer ;
    

//prt( " ---------------> FILTERING IN filterAudio <----------------"); 



   L = 2 * lengthOfAudioArrayForFilter - 1 ; 
   for( N = 1; N < L; N <<= 1 ) ;
   N2 = N>>1 ; 
   lengthMinus1 = lengthOfAudioArrayForFilter - 1 ; 

   fundamental = ( sampleRate / (float) N) ; 

   fvec( filtBuffer, N ) ; 
   for(i = 0; i < lengthOfAudioArrayForFilter; i++ ) filtBuffer[ i ] = audioArrayForFilter[ i ] ; 

   rfft( filtBuffer, N2, FORWARD ) ;


   filterFFT(
      filtBuffer,
      N,
      fundamental, 
      lowFreq,
      highFreq,
      lowRolloffInDBperOctave,
      highRolloffInDBperOctave,
      compoundLevels
   ) ;

   rfft( filtBuffer, N2, INVERSE ) ;
   
/*
   if( N != lengthOfAudioArrayForFilterMemorySize ){
      prt( "REALLOCATING audioArrayForFilter" ) ; 
      free( audioArrayForFilter ) ;  fvec( audioArrayForFilter, N ) ; 
   } ;
*/
   for( i = 0; i < N; i++ ) audioArrayForFilter[ i ] = filtBuffer[ i ] ;

//   lengthOfAudioArrayForFilter = N  ; 

   free( filtBuffer ) ; 

} ;

void smoothReleaseOfCroppedEnd(
   float array[],
   int size, 
   float releaseTime
)
{
   int numSamples ;
   float dur, releaseTimeProportion ;
   int i, j ;

   dur = (float) size / sampleRate ;
   releaseTimeProportion = releaseTime / dur ;  


   if( releaseTimeProportion > .33)
   {
      numSamples = (int) ( .5 + ( (float) size * .33) ) ;
   } else
   {
      numSamples = (int) ( .5 + ( (float) size * releaseTimeProportion) ) ;
   } ;

   for(i = 0, j = size - numSamples; i < numSamples; i++, j++ ) 
      array[ j ] *= curve( 1., 0., (float) i / (float) (numSamples - 1), 0. ) ; 


} ;



void getWallImpulseResponseChannelAssignments()
{ //
   int i, k, wall ;
   float temp ;  

   if( (strcasecmp( wall_channel_assignments_file, datafile2 ) == 1) ||
       (strlen( wall_channel_assignments_file ) == 0)
    ){

      fprintf( stderr, "%s",
         "\n\nWALL IMPULSE RESPONSE CHANNEL ASSIGNMENTS: NO FILE SPECIFIED."
         "\nWILL USE FIRST CHANNEL FOR ALL WALLS." 
      ) ;
      numberOfWallChannelAssignments = numberOfWalls ; 
       // ALLOCATE SPACE FOR ASSIGNMENTS.
      ivec( wallChannelAssignments, numberOfWallChannelAssignments ) ;
      // FILL ARRAY WITH ALL FIRST CHANNELS
      for( i = 0; i < numberOfWalls; i++ ) wallChannelAssignments[ i ] = 1 ;  

   }else{

       prs( wall_channel_assignments_file, "\n\nWALL IMPULSE RESPONSE CHANNEL ASSIGNMENT FILE" ) ; 

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( wall_channel_assignments_file,  new_datafile,  2 ) ; 

      //**************************GET DATA   
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  wall_channel_assignments_file ) ;  
         exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      }    		

      pri( k, "NUMBER OF CHANNEL ASSIGNMENTS IN FILE" ) ;
      if( k > numberOfWalls){
         pri( numberOfWalls, "NUMBER OF WALLS" ) ; 
         prt( "----> NUMBER OF ASSIGNMENTS EXCEEDS THE NUMBER OF WALLS; WILL TRUNCATE TO NUMBER OF WALLS." ) ;   
      }else if( k < numberOfWalls )
      {
         prt( "----> NUMBER OF ASSIGNMENTS IS LESS THAN THE NUMBER OF WALLS;" ) ;
         prt( "ASSIGNMENTS WILL BE LOOP_ASSIGNED TO MEET THE NUMBER OF WALLS." ) ;   
         numberOfWallChannelAssignments = k ;
      }else
      {
         prt( "NUMBER OF WALL CHANNEL ASSIGNMENTS MATCHES THE NUMBER OF WALLS" ) ; 
      } ;
      numberOfWallChannelAssignments = numberOfWalls ; 

//      pri( numberOfWallChannelAssignments, "numberOfWallChannelAssignments" ) ; 

       // ALLOCATE SPACE FOR ASSIGNMENTS.
      ivec( wallChannelAssignments, numberOfWallChannelAssignments ) ;

      // READ IN VALUES
      for(i = 0; i < numberOfWallChannelAssignments; i++ ){
         if( (i % k) == 0 ) rewind( data ) ;
	 fscanf( data,  " %f ",  &temp ) ; 
         wallChannelAssignments[ i ] = (int) temp ; // %%%
         if( wallChannelAssignments[ i ] <= 0 )
         {
            prt( "\n\n" ) ; 
            pri( wallChannelAssignments[ i ], "WALL IMPULSE RESPONSE CHANNEL ASSIGNMENT" ) ; 
            prt( "--------> ERROR: CHANNELS ARE NUMBERED FROM  CHANNEL 1." ) ; 
            prt( ". . . BYE.\n\n" ) ;
            exit(EXIT_FAILURE); 
         } ;
      } ;
   }; 

   fclose( data ) ; 


   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\nWALL IMPULSE RESPONSE CHANNEL ASSIGNMENTS:" ) ;  
   for( wall = 0 ; wall < numberOfWallChannelAssignments; wall++ ){
      fprintf( stderr, "\nWALL: %d CHANNEL: %d", wall, wallChannelAssignments[ wall ] ) ; // %%%
   } ;
   fprintf( stderr, "\n" )  ; 

} ;

void getWallDecibelGainscaleLevels()
{ // 
   int i, k, wall ;
   float temp ;  

   if( (strcasecmp( wall_dB_gainscale_factors_file, datafile2 ) == 1) ||
       (strlen( wall_dB_gainscale_factors_file ) == 0)
    ){

      fprintf( stderr, "%s", "\n\nWALL DB GAINSCALE LEVELS: NO FILE SPECIFIED." 
         "\nWILL ASSIGN UNITY GAIN TO ALL WALLS."
       ) ;
      numberOfWallDecibelGainscaleLevels = numberOfWalls ; 
      fvec( wallDecibelGainscaleLevels, numberOfWallDecibelGainscaleLevels ) ;
      for( i = 0; i < numberOfWallDecibelGainscaleLevels; i++ )
         wallDecibelGainscaleLevels[ i ] = 0. ;
   }else{
       prs( wall_dB_gainscale_factors_file, "\n\nWALL DB GAINSCALE LEVELS FILE" ) ; 

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( wall_dB_gainscale_factors_file,  new_datafile,  2 ) ; 

      //**************************GET DATA   
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  wall_dB_gainscale_factors_file ) ;  
         exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      }    		

      pri( k, "NUMBER OF WALL GAINSCALE LEVELS IN FILE" ) ;
      pri( numberOfWalls, "NUMBER OF WALLS" ) ; 
      if( k > numberOfWalls){
         prt( "----> NUMBER OF LEVELS EXCEEDS THE NUMBER OF WALLS; WILL TRUNCATE TO THE NUMBER OF WALLS." ) ;   
      }else if( k < numberOfWalls )
      {
         prt( "----> NUMBER OF LEVELS IS LESS THAN THE NUMBER OF WALLS;" ) ; 
         prt( "WILL LOOP-ASSIGN LEVELS TO MEET THE NUMBER OF WALLS." ) ;   
      } ;

      numberOfWallDecibelGainscaleLevels = numberOfWalls ; 

       // ALLOCATE SPACE FOR ASSIGNMENTS.
      fvec( wallDecibelGainscaleLevels, numberOfWallDecibelGainscaleLevels ) ;

       

      // READ IN VALUES
      for(i = 0; i < numberOfWallDecibelGainscaleLevels; i++ )
      {
         if( (i % k) == 0 ) rewind( data ) ;
	 fscanf( data,  " %f ",  &wallDecibelGainscaleLevels[ i ] ) ; 
      } ;
   }; 

   fclose( data ) ; 


   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\nWALL DB GAINSCALE LEVELS:" ) ;  
   for( wall = 0 ; wall < numberOfWallDecibelGainscaleLevels; wall++ ){
      fprintf( stderr, "\nWALL: %d LEVEL in dB: %f", wall, wallDecibelGainscaleLevels[ wall ] ) ; 
   } ;
   fprintf( stderr, "\n" )  ; 

} ;

void makeSpaceReflectionCoordinates(
   int order
)
{
   int startOrder, thisOrder, thisMirrorSegment, i ;
   double newIntersectCoordinates[2] ;
   double segment[ 4 ], point[ 2 ] ;    
   float segmentf[ 4 ], pointf[ 2 ] ;    

//   pri( order, "\n ***** ORDER IN makeSpaceReflectionCoordinates" ) ; 

/*
   for(i = 0; i < order; i++){
      printSegment( &mirrorSegments[i * 4], "MIRROR SEGMENT" ) ; 
      printCoordinates( &mirrorSegmentIntersectCoordinates[ i * 2 ], "INTERSECTION" ) ;
   } ;
*/

   for(i = 0 ; i < ((highOrderLimit + 2) * 2 ); i++ ) coordinatesForThisSoundPath[ i ] = 0. ; 

   // ADD SOURCE COORDINATES
   coordinatesForThisSoundPath[ ((order + 1) * 2) + 0 ] = sourceCoordinatesForThisPolygon[ 0 ] ;
   coordinatesForThisSoundPath[ ((order + 1) * 2) + 1 ] = sourceCoordinatesForThisPolygon[ 1 ] ;

   // ADD SPEAKER COORDINATES
   coordinatesForThisSoundPath[ 0 ] = speakerCoordinatesNow[ 0 ] ;
   coordinatesForThisSoundPath[ 1 ] = speakerCoordinatesNow[ 1 ] ;


   // ADD FIRST INTERSECTION (NON-MIRRORED)
   coordinatesForThisSoundPath[ (1 * 2) + 0 ] = mirrorSegmentIntersectCoordinates[ 0 ] ;
   coordinatesForThisSoundPath[ (1 * 2) + 1 ] = mirrorSegmentIntersectCoordinates[ 1 ] ;

   startOrder = 1 ;
   while( startOrder < order ){
//pri( startOrder, "startOrder" ) ; 
      for(thisOrder = startOrder, thisMirrorSegment = 0; 
            thisOrder < order ; thisOrder++, thisMirrorSegment++ )
      {
//pri( thisOrder, "thisOrder" ) ; 
//pri( thisMirrorSegment, "thisMirrorSegment" ) ; 
         // MIRROR
         point[ 0 ] = (double) mirrorSegmentIntersectCoordinates[ (thisOrder * 2) ] ;
         point[ 1 ] = (double) mirrorSegmentIntersectCoordinates[ (thisOrder * 2) + 1 ] ;

         pointf[ 0 ] = mirrorSegmentIntersectCoordinates[ (thisOrder * 2) ] ;
         pointf[ 1 ] = mirrorSegmentIntersectCoordinates[ (thisOrder * 2) + 1 ] ;

         for(i = 0; i < 4; i++)
         { 
            segment[ i ] = (double) mirrorSegments[ (thisMirrorSegment * 4) + i ] ;
            segmentf[ i ] = mirrorSegments[ (thisMirrorSegment * 4) + i ] ;
         } ;

//         printCoordinates( pointf, "MIRRORED POINT" ) ;
//         printSegment( segmentf, "MIRROR SEGMENT"); 

         mirrorPointAroundLineSegment( 
	    point, 
	    segment,
	    newIntersectCoordinates
         ) ;
         mirrorSegmentIntersectCoordinates[ (thisOrder * 2) ]  = (float) newIntersectCoordinates[ 0 ] ;
         mirrorSegmentIntersectCoordinates[ (thisOrder * 2) + 1 ]  = (float) newIntersectCoordinates[ 1 ] ;

         pointf[ 0 ] = (float) newIntersectCoordinates[ 0 ] ;
         pointf[ 1 ] = (float) newIntersectCoordinates[ 1 ] ;
//         printCoordinates( pointf, "UN-MIRRORED POINT" ) ;

         if( thisMirrorSegment == 0 )
         {
            coordinatesForThisSoundPath[ ((startOrder + 1) * 2) + 0 ] = (float) newIntersectCoordinates[ 0 ] ;
            coordinatesForThisSoundPath[ ((startOrder + 1) * 2) + 1 ] = (float) newIntersectCoordinates[ 1 ] ;
         } ;
      } ;
      startOrder++ ;
   } ;


} ;


bool couldThisPointBeInThisSegment(
   float point[],
   float segment[],
   bool includeEnds
)
{
   float lowX, highX, lowY, highY ;

   if( segment[0] < segment[2] ){ lowX = segment[0] ; highX = segment[2] ; }
   else{ lowX = segment[2] ; highX = segment[0] ; } ;

   if( segment[1] < segment[3] ){ lowY = segment[1] ; highY = segment[3] ; }
   else{ lowY = segment[3] ; highY = segment[1] ; } ;

   if( includeEnds )
   { // END POINTS INCLUDED
      if( 
         ((point[0] >= lowX) && (point[0] <= highX))
         &
         ((point[1] >= lowY) && (point[1] <= highY))
      ){
         return( true ) ;
      }else
      { 
         return( false ) ; 
      } ;
   }else
   { // END POINTS EXCLUDED
      if( 
         ((point[0] > lowX) && (point[0] < highX))
         &
         ((point[1] > lowY) && (point[1] < highY))
      ){
         return( true ) ;
      }else
      { 
         return( false ) ; 
      } ;
   } ;
} ;

void prb( 
   bool truth,
   char label[ STRING_SIZE ]
)
{
   fprintf( stderr, "\n%s: ", label ) ; 
   if( truth ) fprintf( stderr, " TRUE " ) ; else fprintf( stderr, " FALSE " ) ;
};

int pointToLinePosition(
   float A[],
   float B[],
   float P[]
)
{
   float position ; 
   position = ( ((double) B[ 0 ] - (double) A[ 0 ]) * 
              ( (double) P[ 1 ] - (double) A[ 1 ]) - 
              ((double) B[ 1 ]-(double) A[ 1 ]) * ( (double) P[ 0 ] - (double) A[ 0 ]) ) ;
   return( (int) copysign( 1., position )  ) ; 
} ;


void createAndReorderWallReflectionSequence(
  int outputFileChannelNumber 
)
{
   int thisReflection, thisReflectionOrderLimit ;
   int startN, incr, i, k, n ;
   
   int numberOfWallsInSequence ;
   int *wallSequence_firstToLast ;
   bool changeMade=true ;
    

   ivec( wallSequence_firstToLast, highOrderLimit + 2 ) ;  

   numberOfSelectedWallReflectionSequences = 0 ; 

//   fprintf( stderr, "\nWALL NUMBER REFLECTION SEQUENCES: " ) ; 

//   pri( lowOrderLimit, "lowOrderLimit" ) ; 
//   pri( highOrderLimit, "highOrderLimit" ) ; 

   // LOOP THROUGH ALL OF THE REFLECTIONS . . . 
   for( thisReflection = 0; 
      thisReflection < viableReflectionCount[ outputFileChannelNumber ]; thisReflection++ )
   {

      thisReflectionOrderLimit = reflectionOrders[ 
         reflectionDataSetChannelPointer[ outputFileChannelNumber ] + thisReflection  
       ] ;
//      pri( thisReflectionOrderLimit, "thisReflectionOrderLimit" ) ; 

      // DOES REFLECTION MEET THE SELECTION CRITERIA?
      if( (thisReflectionOrderLimit >= lowOrderLimit) && (thisReflectionOrderLimit <= highOrderLimit) )
      {
         // SAVE INDEX FOR THIS SEQUENCE
         reorderedWallIRSequences_firstToLast[ 
            (numberOfSelectedWallReflectionSequences * (2 + highOrderLimit)) + 0 
         ] = thisReflection ;

// ****

         // MAKE ********* wallReflectionSequence
         if( wall_impulse_and_gainscale_response_mode > 0 )
         { // 1.-> FROM FIRST REFLECTION WALL 
            startN = thisReflectionOrderLimit - 1 ; incr = -1 ; 
            numberOfWallsInSequence = (int) 
               fminf( (float) abs( wall_impulse_and_gainscale_response_mode ), 
                  (float) thisReflectionOrderLimit ) ;   
         } else if( wall_impulse_and_gainscale_response_mode < 0 )
         { // 2.-> FROM LAST REFLECTION WALL 
             numberOfWallsInSequence = (int) 
               fminf( (float) abs( wall_impulse_and_gainscale_response_mode ), 
                 (float) thisReflectionOrderLimit ) ;   
            startN = numberOfWallsInSequence - 1 ; incr = -1 ;  
         }else
         { // 3.-> ALL REFLECTION WALLS 
            startN = thisReflectionOrderLimit - 1 ; incr = -1 ; 
            numberOfWallsInSequence = thisReflectionOrderLimit ;   
         } ;

         // SAVE SEQUENCE LENGTH
         reorderedWallIRSequences_firstToLast[ 
            (numberOfSelectedWallReflectionSequences * (2 + highOrderLimit)) + 1 
         ] = numberOfWallsInSequence ;


         // ***** USING THE ABOVE, CREATE SEQUENCE OF WALLS TO CONVOLVE, 
         // ****** INCLUDING ONE (i.e. NO CONVOLVE).
         // ZERO OUT wallSequence_firstToLast
         for(i = 0; i < highOrderLimit; i++) wallSequence_firstToLast[ i ] = 0 ; 
         for( i = 0, n = startN ; i < numberOfWallsInSequence; i++, n += incr )
         { 
            wallSequence_firstToLast[ i ] = reflectionWalls_lastToFirst[ 
               (reflectionDataSetChannelPointer[ outputFileChannelNumber ] * highOrderLimit) +
               (thisReflection * highOrderLimit) + n
            ]  ;
         } ;

         // TRANSFER FULL ZEROED SEQUENCE INTO ARRAY
         for( i = 0; i < numberOfWallsInSequence; i++ )
         { 
            reorderedWallIRSequences_firstToLast[ 
               (numberOfSelectedWallReflectionSequences * (2 + highOrderLimit)) + 2 + i 
            ] = wallSequence_firstToLast[ i ] % wall_numberOfInputChannels ; 
         } ;


         // **** PRINT SEQUENCE

/*
         fprintf( stderr, "\n IRs: " ) ;  
         for( n = 0; n < numberOfWallsInSequence; n++ )
            fprintf( stderr, "%d ", reorderedWallIRSequences_firstToLast[ 
               (numberOfSelectedWallReflectionSequences * (2 + highOrderLimit)) + 2 + n 
            ] ) ;   
*/

         numberOfSelectedWallReflectionSequences++ ;

      } ;

   } ;
//  *****

   // SORT REFLECTIONS

   changeMade = true ; 
   while( changeMade )
   {
      changeMade = false ;  
      for(i = 0; i < (numberOfSelectedWallReflectionSequences - 1) ; i++)
      {
         k = isFirstSequenceGreaterLesserOrEqualToSecond(
            & reorderedWallIRSequences_firstToLast[ (i * (2 + highOrderLimit)) + 2 ],
            reorderedWallIRSequences_firstToLast[ (i * (2 + highOrderLimit)) + 1 ],
            & reorderedWallIRSequences_firstToLast[ ((i + 1) * (2 + highOrderLimit)) + 2 ],
            reorderedWallIRSequences_firstToLast[ ((i + 1) * (2 + highOrderLimit)) + 1 ],
            highOrderLimit
         ) ;
         if( k == 1 )
         {   // SWITCH THEM
           
            for( n = 0; n < (highOrderLimit + 2); n++ )
               wallSequence_firstToLast[ n ] = 
                 reorderedWallIRSequences_firstToLast[ (i * (2 + highOrderLimit)) + n ] ;
            for( n = 0; n < (highOrderLimit + 2); n++ )
               reorderedWallIRSequences_firstToLast[ (i * (2 + highOrderLimit)) + n ] =
                  reorderedWallIRSequences_firstToLast[ ((i + 1) * (2 + highOrderLimit)) + n ] ;
            for( n = 0; n < (highOrderLimit + 2); n++ )
               reorderedWallIRSequences_firstToLast[ ((i + 1) * (2 + highOrderLimit)) + n ] =
                  wallSequence_firstToLast[ n ] ; 

            changeMade = true ; 
         } ;

      } ;
   } ;

         // **** PRINT SEQUENCES
/*
   fprintf( stderr, "\nWALL NUMBER REFLECTION SEQUENCES (LIST SORTED): " ) ; 
   for(i = 0; i < numberOfSelectedWallReflectionSequences ; i++)
   {
         fprintf( stderr, "\n [%d]: ",
           reorderedWallIRSequences_firstToLast[ (i * (2 + highOrderLimit)) ]
         ) ;
         numberOfWallsInSequence = 
             reorderedWallIRSequences_firstToLast[ (i * (2 + highOrderLimit)) + 1 ] ;  
         for( n = 0; n < numberOfWallsInSequence; n++ )
            fprintf( stderr, "%d ", reorderedWallIRSequences_firstToLast[ 
                (i * (2 + highOrderLimit)) + 2 + n ] 
            ) ;   
   } ;      
*/
   free( wallSequence_firstToLast ) ; 


} ;

void sortSequence(
   int seq[],
   int length
)
{
   bool changeMade = true ;
   int i, tempi ;  

   // BUBBLE SORT
   while( changeMade )
   {
      changeMade = false ; 
      for(i = 0; i < (length - 1); i++)
      {
         if( seq[ i ] > seq[i + 1] )
         {
            tempi = seq[ i ] ; seq[ i ] = seq[i + 1] ; seq[i + 1] = tempi ; 
            changeMade = true ; 
         } ;
      } ;
   } ;   
} ;

int isFirstSequenceGreaterLesserOrEqualToSecond(
   int seq0[],
   int length0,
   int seq1[],
   int length1,
   int length
)
{  
   int i = 0 ;
   bool notYet=true ;
   
   // -1 Lesser, 1 Greater, 0 = Equal

   while( notYet )
   {
      if( seq0[ i ] == seq1[ i ] )
      {
         i++ ; if( i == length ) notYet = false ; 
      }else
      {
         notYet = false ; 
         if( seq0[ i ] >= seq1[ i ] ) return( 1 ) ; 
         else return( -1 ) ;  
      } ;
   } ;

   if( length0 > length1 ) return( 1 ) ;
   if( length0 < length1 ) return( -1 ) ;
   return( 0 ) ; 

} ;
   
void addToIRfileCodes(
   int sequenceOfWallIResponses[],
   int numberOfWallsInSequence
)
{
   int start, i, j, n, newSize ;
   float *transferArray ; 


   if( numberOfSavedIResponses == numberOfReflections )
      makeOrIncreaseMemorySpaceForSavedWallReflectionPatterns(); 

   // SAVE SORTED SEQUENCE FOR SEARCH LATER. 
   pri( numberOfSavedIResponses, "(In addToIRfileCodes) SAVING TO SEQUENCE NUMBER" ) ;
//   pri( numberOfWallsInSequence, "numberOfWallsInSequence" ) ; 
   prt("\n" ) ;  
   for(n = 0; n < numberOfWallsInSequence; n++) fprintf( stderr, " %d", sequenceOfWallIResponses[ n ] ) ;

   start = numberOfSavedIResponses * (highOrderLimit * 2) ; 
   for(i = start, j = 0; j < numberOfWallsInSequence; i++, j++ )
      fileIRCollectionCodes[ i ] = sequenceOfWallIResponses[ j ] ;

   // SAVE THE LENGTH OF THE SEQUENCE.
   fileIRCollectionCodeLengths[ numberOfSavedIResponses ] = numberOfWallsInSequence ;
//   pri( fileIRCollectionCodeLengths[ numberOfSavedIResponses ], 
//        "fileIRCollectionCodeLengths[ numberOfSavedIResponses ]" ) ; 

   // SAVE THIS POSITION FOR RECALL LATER.
   fileIRCollectionAddresses[ numberOfSavedIResponses ] = memoryPositionNow ; 

//   pri( numberOfSavedIResponses, "...........................................=> SAVING, RESPONSE" ) ; 
//   pri( memoryPositionNow, " MEMORY POSITION INDEX" ) ; 

   if( (memoryPositionNow + impulseResponseNowMemorySize) > fileIRCollectionDataMemorySizeNow )
   {
      prt( "EXPANDING MEMORY FOR SAVED CONVOLVED IMPULSE RESPONSES" ) ; 
      fvec( transferArray, fileIRCollectionDataMemorySizeNow ) ; 
      for(i = 0; i < fileIRCollectionDataMemorySizeNow; i++) transferArray[ i ] =
         fileIRCollectionData[ i ] ;
      newSize = fileIRCollectionDataMemorySizeNow  + 10000000 ;
      free( fileIRCollectionData ) ; fvec( fileIRCollectionData, newSize ) ; 
      for(i = 0; i < fileIRCollectionDataMemorySizeNow; i++) fileIRCollectionData[ i ] =
         transferArray[ i ] ;
      fileIRCollectionDataMemorySizeNow = newSize ; 
      free( transferArray ) ; 
   } ;

   // WRITE THE DATA
   for(i = 0 ; i < impulseResponseNowMemorySize; i++)
      fileIRCollectionData[ memoryPositionNow + i] = impulseResponseNow[ i ] ; 
   // SAVE THE DATA LENGTH IN FLOATS
   fileIRCollectionLengths[ numberOfSavedIResponses ] = impulseResponseNowMemorySize ;

   // INCREMENT MEMORY POSITION
   memoryPositionNow += impulseResponseNowMemorySize ;

   numberOfSavedIResponses++ ;
//   pri( numberOfSavedIResponses, "AT END OF addToIRfileCodes, numberOfSavedIResponses" ) ; 
} ;          

bool testSequence( 
   int sequence[],
   int numberOfTestWalls,
   int *IRindex 
)
{
   bool found=false ;
   int i, n ; 

//pri( numberOfTestWalls, "AT BEGIN OF testSequence numberOfTestWalls" ) ; 
//pri( numberOfSavedIResponses, "AT BEGIN OF testSequence numberOfSavedIResponses" ) ; 

   for(i = 0 ; i < numberOfSavedIResponses; i++)
   {

//fprintf( stderr, "\n%d: fileIRCollectionCodeLengths[ i ]: %d", i,  fileIRCollectionCodeLengths[ i ] ) ; 
      found = false ; 
      if(  (long int) numberOfTestWalls == fileIRCollectionCodeLengths[ i ] )
      {
         found = true ; 
         for(n = 0; n < numberOfTestWalls; n++)
         {
            if( fileIRCollectionCodes[ (i * (highOrderLimit * 2)) + n] != sequence[ n ] )
            {
               found = false ; break ;
            } ;
         } ; 
      } ;
      if( found ) break ; 
   } ;

   if( found ) {
      *IRindex = i ; 

      
//pri( numberOfTestWalls, "numberOfTestWalls" ) ; 

      fprintf( stderr, "\n FOUND: Sequence No. %d [ ", *IRindex ) ; 
      for(n = 0; n < numberOfTestWalls; n++)fprintf( stderr, "%d ", sequence[ n ] ) ;
      fprintf( stderr, "]" ) ;    
/*
      fprintf( stderr, "\n" ) ; 
      for(n = 0; n < numberOfTestWalls; n++)
          fprintf( stderr, "%d ", fileIRCollectionCodes[ (*IRindex * (highOrderLimit * 2)) + n] ) ;   
*/
      return( true ) ;
   } else
   {
      *IRindex = -1 ; 
      return( false ) ;  
   } ;
} ;

void recallIR( 
   int IRindex 
)
{
   int n, k, length ;

   pri( IRindex, "* * * * * * * * * * * * * * RECALLING RESPONSE" ) ;  
   length = fileIRCollectionLengths[ IRindex ] ;

   if( length > impulseResponseNowMemorySize )
   {
      free( impulseResponseNow ) ; 
      fvec( impulseResponseNow, length ) ;
      previousImpulseResponseNowMemorySize = impulseResponseNowMemorySize ; 
      impulseResponseNowMemorySize = length ; 
   }else
   {
      // ZERO
      for(n = length; n < impulseResponseNowMemorySize; n++) impulseResponseNow[ n ] = 0. ; 
   } ;

   for(n = 0, k = fileIRCollectionAddresses[ IRindex ] ; n < length; n++, k++ )
      impulseResponseNow[ n ] = fileIRCollectionData[ k ] ;  

} ;


void filterAndNormalizeImpulseResponseNow(
   int parameterSet__wall_0__reflection_order_1, 
   float rolloffScaler
)
{
   int L, N, n ;
   float peakAmp ;  

   prt(" . . . FILTERING impulseResponseNow . . . " ) ;

   if(  parameterSet__wall_0__reflection_order_1 == 0 ) prt( "USING WALL PARAMETER SETTINGS." ); 
   else prt( "USING REFLECTION ORDER PARAMETER SETTINGS." );

   lengthOfAudioArrayForFilter = impulseResponseNowMemorySize ;

   L = 2 * lengthOfAudioArrayForFilter - 1 ; 
   for( N = 1; N < L; N <<= 1 ) ;
   lengthOfAudioArrayForFilterMemorySize = N ; 

   fvec( audioArrayForFilter, lengthOfAudioArrayForFilterMemorySize ) ;
   for(n = 0; n < impulseResponseNowMemorySize; n++ )
        audioArrayForFilter[ n ] =  impulseResponseNow[ n ] ;

   // FILTER AND NORMALIZE
   if(  parameterSet__wall_0__reflection_order_1 == 0 )
      filterAudioArray(
         sampleRate,
         WIRBPF_low_rolloff_frequency,
         WIRBPF_high_rolloff_frequency,
         WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave * rolloffScaler,
         WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave * rolloffScaler,
         1 // compound levels
      ) ;
   else
      filterAudioArray(
         sampleRate,
         RO_IR_BPF_low_rolloff_frequency,
         RO_IR_BPF_high_rolloff_frequency,
         RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave * rolloffScaler,
         RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave * rolloffScaler,
         1 // compound levels
      ) ;

   peakAmp = findPeakAmp(audioArrayForFilter, lengthOfAudioArrayForFilter );
   if( peakAmp > 0. ) for( n = 0; n < lengthOfAudioArrayForFilter; n++ ) 
      audioArrayForFilter[ n ] /= peakAmp ;

   // TRANSFER BACK INTO impulseResponseNow 
   if( lengthOfAudioArrayForFilter > impulseResponseNowMemorySize )
   {
      free( impulseResponseNow ) ; fvec( impulseResponseNow, lengthOfAudioArrayForFilter ) ; 
      impulseResponseNowMemorySize = lengthOfAudioArrayForFilter ;
   } ;
   for( n = 0; n < lengthOfAudioArrayForFilter; n++ )
      impulseResponseNow[ n ] = audioArrayForFilter[ n ] ;

} ;

void filterAndNormalizeWallImpulseResponses()
{
   int i, n, k ;
   float *transferArray ; 


   if(
       (wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 1)
          ||
       (wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 3)
   )
   {
      prt( ". . . PRE-FILTERING WALL IMPULSE RESPONSES." ) ; 


      for(i = 0; i < wall_numberOfInputChannels; i++)
      {
         fvec( impulseResponseNow, wall_numberOfFrames ) ;
         impulseResponseNowMemorySize = wall_numberOfFrames ;
         for(n = 0; n < wall_numberOfFrames; n++)
            impulseResponseNow[ n ] = 
               wallImpulseResponses[(i * wall_numberOfFrames) + n] ;

         filterAndNormalizeImpulseResponseNow( 0, 1. ) ;
            
         if( (long int) impulseResponseNowMemorySize > wall_numberOfFrames )
         {
            // LONGER
            fvec( transferArray, wall_numberOfInputChannels * wall_numberOfFrames ) ; 
            for(n = 0; n < (wall_numberOfInputChannels * wall_numberOfFrames); n++ )
               transferArray[ n ] = wallImpulseResponses[ n ] ;
            free( wallImpulseResponses ) ; 
            fvec( wallImpulseResponses, wall_numberOfInputChannels * impulseResponseNowMemorySize ) ;
            for(k = 0; k < wall_numberOfInputChannels; k++)
            {
               for(n = 0; n < wall_numberOfFrames; n++)
                  wallImpulseResponses[ (k * impulseResponseNowMemorySize) + n ] = 
                     transferArray[ (k * wall_numberOfFrames) + n ] ;
            } ;
            free( transferArray ) ; 
            for(n = 0; n < impulseResponseNowMemorySize; n++)
               wallImpulseResponses[ (i * impulseResponseNowMemorySize) + n ] = 
                  impulseResponseNow[ n ] ;
            wall_numberOfFrames = impulseResponseNowMemorySize ;
         }else
         {
            // SAME OR SHORTER
            for(n = 0; n < wall_numberOfFrames; n++ ) 
               wallImpulseResponses[ (i * wall_numberOfFrames) + n ] = 0. ; 
            for(n = 0; n < impulseResponseNowMemorySize; n++ )
               wallImpulseResponses[ (i * wall_numberOfFrames) + n ] = impulseResponseNow[ n ] ; 
         } ;

         free( impulseResponseNow ) ; 
      } ;
   } ;
};


void filterAndNormalizeReflectionOrderImpulseResponses()
{
   int i, n, k ;
   float *transferArray ; 


   if( 
      (reflection_order_impulse_responses__off_0__on_1 == 1) &&
      (
         (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 1) ||
         (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 3)
      ) 
   )
   {
      prt( ". . . PRE-FILTERING REFLECTION ORDER IMPULSE RESPONSES." ) ; 


      for(i = 0; i < reflection_order_numberOfInputChannels; i++)
      {
         fvec( impulseResponseNow, reflection_order_numberOfFrames ) ;
         impulseResponseNowMemorySize = reflection_order_numberOfFrames ;
         for(n = 0; n < reflection_order_numberOfFrames; n++)
            impulseResponseNow[ n ] = 
               reflectionOrderImpulseResponses[(i * reflection_order_numberOfFrames) + n] ;

         filterAndNormalizeImpulseResponseNow( 1, 1. ) ;
            
         if( (long int) impulseResponseNowMemorySize > reflection_order_numberOfFrames )
         {
            // LONGER
            fvec( transferArray, reflection_order_numberOfInputChannels * reflection_order_numberOfFrames ) ; 
            for(n = 0; n < (reflection_order_numberOfInputChannels * reflection_order_numberOfFrames); n++ )
               transferArray[ n ] = reflectionOrderImpulseResponses[ n ] ;
            free( reflectionOrderImpulseResponses ) ; 
            fvec( reflectionOrderImpulseResponses, 
               reflection_order_numberOfInputChannels * impulseResponseNowMemorySize ) ;
            for(k = 0; k < reflection_order_numberOfInputChannels; k++)
            {
               for(n = 0; n < reflection_order_numberOfFrames; n++)
                  reflectionOrderImpulseResponses[ (k * impulseResponseNowMemorySize) + n ] = 
                     transferArray[ (k * reflection_order_numberOfFrames) + n ] ;
            } ;
            free( transferArray ) ; 
            for(n = 0; n < impulseResponseNowMemorySize; n++)
               reflectionOrderImpulseResponses[ (i * impulseResponseNowMemorySize) + n ] = 
                  impulseResponseNow[ n ] ;
            reflection_order_numberOfFrames = impulseResponseNowMemorySize ;
         }else
         {
            // SAME OR SHORTER
            for(n = 0; n < reflection_order_numberOfFrames; n++ ) 
               reflectionOrderImpulseResponses[ (i * reflection_order_numberOfFrames) + n ] = 0. ; 
            for(n = 0; n < impulseResponseNowMemorySize; n++ )
               reflectionOrderImpulseResponses[ (i * reflection_order_numberOfFrames) + n ] = 
                  impulseResponseNow[ n ] ; 
         } ;

         free( impulseResponseNow ) ; 
      } ;
   } ;
};



void filterAndNormalizeReflectionOrderImpulseResponsesPostConvolution()
{
   int i, n, k ;
   float *transferArray, rolloffScaler ; 


   if( 
      (reflection_order_impulse_responses__off_0__on_1 == 1) &&
      (
         (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 2) ||
         (RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 == 3)
      ) 
   )
   {
      prt( ". . . FILTERING REFLECTION ORDER IMPULSE RESPONSES FOLLOWING CONVOLUTIONS." ) ; 


      for(i = 0; i < reflection_order_numberOfInputChannels; i++)
      {
         fvec( impulseResponseNow, reflection_order_numberOfFrames ) ;
         impulseResponseNowMemorySize = reflection_order_numberOfFrames ;

         for(n = 0; n < reflection_order_numberOfFrames; n++)
            impulseResponseNow[ n ] = 
               reflectionOrderImpulseResponses[(i * reflection_order_numberOfFrames) + n] ;


         rolloffScaler = (float) (reflectionOrderCVOrderSequenceSizes[ i ] - 1) ; 
         if( rolloffScaler >= 1.) filterAndNormalizeImpulseResponseNow( 1, rolloffScaler ) ;
            
         if( (long int) impulseResponseNowMemorySize > reflection_order_numberOfFrames )
         {
            // LONGER
            fvec( transferArray, reflection_order_numberOfInputChannels * reflection_order_numberOfFrames ) ; 
            for(n = 0; n < (reflection_order_numberOfInputChannels * reflection_order_numberOfFrames); n++ )
               transferArray[ n ] = reflectionOrderImpulseResponses[ n ] ;
            free( reflectionOrderImpulseResponses ) ; 
            fvec( reflectionOrderImpulseResponses, 
               reflection_order_numberOfInputChannels * impulseResponseNowMemorySize ) ;
            for(k = 0; k < reflection_order_numberOfInputChannels; k++)
            {
               for(n = 0; n < reflection_order_numberOfFrames; n++)
                  reflectionOrderImpulseResponses[ (k * impulseResponseNowMemorySize) + n ] = 
                     transferArray[ (k * reflection_order_numberOfFrames) + n ] ;
            } ;
            free( transferArray ) ; 
            for(n = 0; n < impulseResponseNowMemorySize; n++)
               reflectionOrderImpulseResponses[ (i * impulseResponseNowMemorySize) + n ] = 
                  impulseResponseNow[ n ] ;
            reflection_order_numberOfFrames = impulseResponseNowMemorySize ;
         }else
         {
            // SAME OR SHORTER
            for(n = 0; n < reflection_order_numberOfFrames; n++ ) 
               reflectionOrderImpulseResponses[ (i * reflection_order_numberOfFrames) + n ] = 0. ; 
            for(n = 0; n < impulseResponseNowMemorySize; n++ )
               reflectionOrderImpulseResponses[ (i * reflection_order_numberOfFrames) + n ] = 
                  impulseResponseNow[ n ] ; 
         } ;

         free( impulseResponseNow ) ; 


      } ;
   } ;
};



void truncateEnvelopeAndNormalizeWallImpulseResponses()
{
   bool useEnvelope ;
   float *transferArray, ampEnv, peakAmp ;
   int i, n ;
   int newWall_numberOfFrames ; 

   prf( wall_IR_truncate_duration, "WALL IMPULSE RESPONSE TRUNCATE DURATION" ) ;

   if( wallImpulseResponsesFromSoundFileFlag && (wall_IR_truncate_duration > 0.) && ! wallPulseModeFlag )
   {
      // NON-ZERO TRUNCATE DURATION
        
      if( wall_inputDuration <= wall_IR_truncate_duration )
      {
         // NO TRUNCATION
         prf( wall_inputDuration, "WALL IMPULSE RESPONSE SOUND FILE DURATION" ) ; 
         prt( "BYPASSING TRUNCATION AS IMPULSE RESPONSE DURATION IS LESS THAN THE TRUNCATE DURATION." ) ;
      }else
      { ;
         prf( wall_IR_truncate_duration, 
              "TRUNCATING WALL IMPULSE RESPONSES TO DURATION (in seconds)" ) ; 

         // ENVELOPE FLAG
         if( wall_IR_response_envelope.n != 1 )
         { 
            // APPLY ENVELOPE
            useEnvelope = true ;
            prt( ". . . WILL USE ENVELOPE FROM SUPPLIED FILE." ) ;  
         }else
         {
            // NO ENVELOPE FUNCTION
            prt( "NO ENVELOPE FUNCTION FILE; WILL APPLY RAW TRUNCATION OF IMPULSE." ) ;
            useEnvelope = false ;  
         } ;


         // TRUNCATE
         newWall_numberOfFrames = (int)
            ((float) wall_numberOfFrames * (wall_IR_truncate_duration / wall_inputDuration)) ;
         fvec( transferArray, wall_numberOfInputChannels * newWall_numberOfFrames ) ; 
         wall_inputDuration = wall_IR_truncate_duration ;
         for(i = 0; i < wall_numberOfInputChannels; i++)
         {
            for(n = 0; n < newWall_numberOfFrames; n++)
            { 
               if( useEnvelope ) 
               { 
                  wall_IR_response_envelope.A[ 0 ] = 
                     fval( &wall_IR_response_envelope, wall_inputDuration, 
                     wall_inputDuration * (float) n / (float)(newWall_numberOfFrames - 1) ) ;
                  ampEnv = wall_IR_response_envelope.A[ 0 ] ;
               }else 
               { 
                  ampEnv = 1. ; 
               } ;
               transferArray[ (i * newWall_numberOfFrames) + n ] = ampEnv * 
                  wallImpulseResponses[(i * wall_numberOfFrames) + n] ;
            } ;
            peakAmp = findPeakAmp( 
               &transferArray[ (i * newWall_numberOfFrames) ], newWall_numberOfFrames ) ;
            if( peakAmp > 0. ) for( n = 0; n < newWall_numberOfFrames; n++ ) 
               transferArray[ (i * newWall_numberOfFrames) + n ] /= peakAmp ;
         } ;
         free( wallImpulseResponses ) ; 
         wall_numberOfFrames = newWall_numberOfFrames ;
         fvec( wallImpulseResponses, wall_numberOfFrames * wall_numberOfInputChannels ) ; 
         for(i = 0; i < (wall_numberOfFrames  * wall_numberOfInputChannels) ; i++)
            wallImpulseResponses[ i ] = transferArray[ i ] ;
         free( transferArray ) ; 
      } ;  
   } ;
} ;

void truncateEnvelopeAndNormalizeReflectionOrderImpulseResponses()
{
   bool useEnvelope ;
   float *transferArray, ampEnv, peakAmp ;
   int i, n ;
   int newReflection_Order_numberOfFrames ; 

   prf( reflection_order_IR_truncate_duration, "REFLECTION ORDER IMPULSE RESPONSE TRUNCATE DURATION" ) ;
   pri( reflection_order_impulse_responses__off_0__on_1, "reflection_order_impulse_responses__off_0__on_1" ) ; 


   if( (reflection_order_impulse_responses__off_0__on_1 == 1) 
       && (reflection_order_IR_truncate_duration > 0.)
   )
   {
      // NON-ZERO TRUNCATE DURATION
        
      if( reflection_order_inputDuration <= reflection_order_IR_truncate_duration )
      {
         // NO TRUNCATION
         prf( reflection_order_inputDuration, "REFLECTION ORDER IMPULSE RESPONSE SOUND FILE DURATION" ) ; 
         prt( "BYPASSING TRUNCATION AS DURATION IS LESS THAN TRUNCATE DURATION." ) ;
      }else
      { ;
         prf( reflection_order_IR_truncate_duration, 
              "TRUNCATING REFLECTION ORDER IMPULSE RESPONSES TO DURATION (in seconds)" ) ; 

         // ENVELOPE FLAG
         if( reflection_order_IR_envelope.n != 1 )
         { 
            // APPLY ENVELOPE
            useEnvelope = true ;
            prt( ". . . WILL USE ENVELOPE FROM SUPPLIED FILE." ) ;  
         }else
         {
            // NO ENVELOPE FUNCTION
            prt( "NO ENVELOPE FUNCTION FILE; WILL APPLY RAW TRUNCATION OF IMPULSE." ) ;
            useEnvelope = false ;  
         } ;

         // TRUNCATE
         newReflection_Order_numberOfFrames = (int)
            ((float) reflection_order_numberOfFrames * (reflection_order_IR_truncate_duration / 
               reflection_order_inputDuration)) ;
         fvec( transferArray, reflection_order_numberOfInputChannels * newReflection_Order_numberOfFrames ) ; 
         reflection_order_inputDuration = reflection_order_IR_truncate_duration ;
         for(i = 0; i < reflection_order_numberOfInputChannels; i++)
         {
            for(n = 0; n < newReflection_Order_numberOfFrames; n++)
            { 
               if( useEnvelope ) 
               { 
                  reflection_order_IR_envelope.A[ 0 ] = 
                     fval( &reflection_order_IR_envelope, reflection_order_inputDuration, 
                     reflection_order_inputDuration * (float) n / 
                        (float)(newReflection_Order_numberOfFrames - 1) ) ;
                  ampEnv = reflection_order_IR_envelope.A[ 0 ] ;
               }else 
               { 
                  ampEnv = 1. ; 
               } ;
               transferArray[ (i * newReflection_Order_numberOfFrames) + n ] = ampEnv * 
                  reflectionOrderImpulseResponses[(i * reflection_order_numberOfFrames) + n] ;
            } ;
            peakAmp = findPeakAmp( 
               &transferArray[ (i * newReflection_Order_numberOfFrames) ], 
                  newReflection_Order_numberOfFrames ) ;
            if( peakAmp > 0. ) for( n = 0; n < newReflection_Order_numberOfFrames; n++ ) 
               transferArray[ (i * newReflection_Order_numberOfFrames) + n ] /= peakAmp ;
         } ;
         free( reflectionOrderImpulseResponses ) ; 
         reflection_order_numberOfFrames = newReflection_Order_numberOfFrames ;
         fvec( reflectionOrderImpulseResponses, 
             reflection_order_numberOfFrames * reflection_order_numberOfInputChannels ) ; 
         for(i = 0; i < (reflection_order_numberOfFrames  * reflection_order_numberOfInputChannels) ; i++)
            reflectionOrderImpulseResponses[ i ] = transferArray[ i ] ;
         free( transferArray ) ; 
         prt("\n\n" ) ; 
      } ;  
   } ;
} ;

void readInReflectionOrderImpulseResponses()
{
   int n, order ;
   int channel, frame ; 
   int numberOfSampsBufferedIn ;
   float peakAmp ;   


   if( reflection_order_impulse_responses__off_0__on_1 == 1 )
   {
      // REFLECTION ORDER IMPULSE RESPONSES IS **ON**.
      prs( reflectionOrderImpulseResponseInputSoundFileName,
       "reflectionOrderImpulseResponseInputSoundFileName" ) ; 


      if( (strcasecmp( reflectionOrderImpulseResponseInputSoundFileName, datafile2 ) == 1) ||
          (strlen( reflectionOrderImpulseResponseInputSoundFileName ) == 0) ||
          (reflection_order_impulse_responses__off_0__on_1 == 0)
      ){
         // ** NO FILE SPECIFIED. 
            // REFLECTION ORDER IMPULSES IS ON, BUT THERE IS NO FILE!          
            prt( "\n\n---------ERROR: NO REFLECTION ORDER IMPULSE RESPONSE SOUND FILE.\n\n" ) ; 
            prt( "TO USE REFLECTION ORDER IMPULSE RESPONSES (WHICH IS ON), YOU MUST SPECIFY A " ) ; 
            prt( "REFLECTION ORDER IMPULSES SOUND FILE.");
            prt( "\n. . . BYE."); 
            exit( EXIT_FAILURE ) ;   
      }else 
      {
         // ** FILE SPECIFIED. TRY TO OPEN. 
         if(! (inputReflectionOrderImpulseResponseFilePointer = 
              sf_open (reflectionOrderImpulseResponseInputSoundFileName, SFM_READ, 
              &inputReflectionOrderImpulseResponseSFinfo ))
         )
         {
            // ** FILE DOES NOT EXIST
            prs( reflectionOrderImpulseResponseInputSoundFileName, 
               "REFLECTION ORDER IMPULSE RESPONSE SOUND FILE" ) ; 
            prt( "------> FILE NOT FOUND\n\n . . . . .  BYE.\n\n" ) ;
            exit( EXIT_FAILURE ) ; 
         }else
         {
            // ** FILE EXISTS. 
            prt( "REFLECTION ORDER IMPULSE RESPONSES FILE FOUND." ) ; 
            prs( reflectionOrderImpulseResponseInputSoundFileName, 
               "REFLECTION ORDER IMPULSE RESPONSE(S) SOUND FILE" ) ; 

            // OPEN FILE AND PARSE INTO MEMORY BY CHANNEL ASSIGNMENTS.
 	 // OPEN INPUT SOUND FILE IN READ MODE TO GET FORMAT. 

            reflectionOrderImpulseResponsesFlag = true ;

            reflection_order_isr = inputReflectionOrderImpulseResponseSFinfo.samplerate ; 
            reflection_order_isrDouble = (double) reflection_order_isr ; 
            reflection_order_numberOfInputChannels  = inputReflectionOrderImpulseResponseSFinfo.channels ;
            reflection_order_numberOfFrames = (long int) inputReflectionOrderImpulseResponseSFinfo.frames ;
            reflection_order_inputDuration  = 
               (float)((double) reflection_order_numberOfFrames / reflection_order_isrDouble) ; // 
            
            prbanner( "REFLECTION ORDER IMPULSE RESPONSE SOUND FILE PARAMETERS:", 69 ) ; 
            fprintf( stderr, "\n -- CHANNELS: %d  SAMPLE RATE: %d FRAMES: %d  DURATION: %f", 
               reflection_order_numberOfInputChannels, reflection_order_isr, 
               (int) reflection_order_numberOfFrames, reflection_order_inputDuration ) ; 
            reflection_order_iformat = inputReflectionOrderImpulseResponseSFinfo.format ; 
// **

            // CHECK FOR ASSIGNMENT OF NON-EXISTENT CHANNELS.
            for(n = 0; n < numberOfReflectionOrderChannelAssignments; n++)
            {
               // FOR EVERY CHANNEL
               if( reflectionOrderChannelAssignments[ n ] > reflection_order_numberOfInputChannels )
               {
                  prt( "\n\n----- ERROR: REFLECTION ORDER CHANNEL ASSIGNMENTS" ) ; 
                  fprintf( stderr, "\n\nREFLECTION ORDER IMPULSE RESPONSE SOUND FILE CHANNELS: 1 - %d",
                     reflection_order_numberOfInputChannels ) ; 
                  prt( "\nREFLECTION ORDER CHANNEL ASSIGNMENTS:\n" ) ;
                  for( n = 0; n < numberOfReflectionOrderChannelAssignments; n++ )
                  fprintf( stderr, "%d ", reflectionOrderChannelAssignments[ n ] ) ; 
                  prt( "\nONE OR MORE CHANNEL ASSIGNMENTS EXCEEDS THE NUMBER OF AVAILABLE CHANNELS." ) ; 
                  prt( "\n. . .  BYE.\n\n" ) ; 
                  exit(EXIT_FAILURE); 
               } ; 
            } ;

            // MAKE MEMORY FOR AUDIO DATA
            fvec( reflectionOrderImpulseResponses, 
               numberOfReflectionOrderChannelAssignments * reflection_order_numberOfFrames ) ; 
            fvec( reflectionOrderImpulseResponsesInterleaved, 
               reflection_order_numberOfInputChannels * reflection_order_numberOfFrames ) ; 
            fvec( reflectionOrderImpulseResponseNow, reflection_order_numberOfFrames ) ; 

            // FOR EACH CHANNEL, READ IN AUDIO FROM CHANNEL INTO MEMORY.
            sf_seek( inputReflectionOrderImpulseResponseFilePointer, 0, SEEK_SET ) ;  // REWIND	

	    numberOfSampsBufferedIn = 
              sf_read_float (inputReflectionOrderImpulseResponseFilePointer, 
                 reflectionOrderImpulseResponsesInterleaved, 
                 reflection_order_numberOfInputChannels * reflection_order_numberOfFrames ) ;

            for( order = 0; order < numberOfReflectionOrderChannelAssignments; order++ ){
               channel = reflectionOrderChannelAssignments[ order ] ;  // %%%
               pri( channel, "ASSIGNED CHANNEL" ) ; 
               for( frame = 0; frame < reflection_order_numberOfFrames; frame++ )
               {
                  reflectionOrderImpulseResponses[ (order * reflection_order_numberOfFrames) + frame ] = 
                     reflectionOrderImpulseResponsesInterleaved[ 
                        (frame * reflection_order_numberOfInputChannels) + (channel - 1) ] ; // %%%
               } ;
            } ;
            reflection_order_numberOfInputChannels = numberOfReflectionOrderChannelAssignments ;

            for( order = 0; order < reflection_order_numberOfInputChannels; order++ ){
               pri( order, "CHANNEL" ); 
               peakAmp = findPeakAmp(
                  &reflectionOrderImpulseResponses[ (order * reflection_order_numberOfFrames) ],
                  reflection_order_numberOfFrames
               ); 
               prf( amp_to_dB( peakAmp ), "PEAK AMP (in dB)" ) ;  
            } ;

         } ; 

      } ;

      sf_close( inputReflectionOrderImpulseResponseFilePointer ) ; 
      free( reflectionOrderImpulseResponsesInterleaved ) ; 

   } ;

} ;



void getReflectionOrderImpulseResponseChannelAssignments()
{ //
   int i, k, order ;
   float temp ;  

   if( reflection_order_impulse_responses__off_0__on_1 == 1 )
   {
      // ** SET TO USE REFLECTION ORDER IMPULSE RESPONSES

      if( (strcasecmp( reflection_order_channel_assignments_file, datafile2 ) == 1) ||
          (strlen( reflection_order_channel_assignments_file ) == 0)
       ){
         // FILE **NOT** FOUND.
         prt( "FLAG SET TO USE RELFECTION ORDER IMPULSE RESPONSES," ) ; 
         prt( "BUT CHANNEL ASSIGNMENTS FILE HAS NOT BEEN SPECIFIED.\n\n" ) ; 
         prt( "\n\n . . . BYE" ) ; 
         exit( EXIT_FAILURE ) ; 
      }else
      {
         // FILE FOUND
          prs( reflection_order_channel_assignments_file, 
              "\n\nREFLECTION ORDER IMPULSE RESPONSE CHANNEL ASSIGNMENT FILE" ) ; 

          // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
          cut_data_lines( reflection_order_channel_assignments_file,  new_datafile,  2 ) ; 

         //**************************GET DATA   
         // OPEN FILE
         if( (data = fopen( new_datafile, "r")) == NULL ){
            fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  
            reflection_order_channel_assignments_file ) ;  
            prt( "\n\n . . . BYE" ) ; 
            exit(EXIT_FAILURE); 
         }
 
         // COUNT VALUES IN FILE
         k = 0 ; 
         while( fscanf( data,  " %f ",  &temp ) != EOF ){
            k++ ;
         }    		

         pri( k, "NUMBER OF REFLECTION ORDER SOUND FILE CHANNEL ASSIGNMENTS IN FILE" ) ;

         if( k > highOrderLimit ){
            // MORE THAN
            pri( highOrderLimit, "HIGH ORDER REFLECTION LIMIT" ) ; 
            prt( "----> NUMBER OF ASSIGNMENTS EXCEEDS THE HIGH ORDER REFLECTION LIMIT." ) ; 
            prt( "NUMBER OF ASSIGNMENTS WILL BE TRUNCATED TO THE HIGH ORDER REFLECTION LIMIT." ) ;   
            pri( k, "ADJUSTED NUMBER OF REFLECTION ORDER SOUND FILE CHANNEL ASSIGNMENTS IN FILE" ) ;
         }else if( k < highOrderLimit )
         {
            // LESS THAN
            pri( highOrderLimit, "HIGH ORDER REFLECTION LIMIT" ) ; 
            prt( "----> NUMBER OF ASSIGNMENTS IS LESS THAN THE HIGH ORDER REFLECTION LIMIT." ) ; 
            prt( "ASSIGNMENTS WILL BE LOOP-ASSIGNED TO REMAINING HIGH ORDER REFLECTION LEVELS." ) ;   
            numberOfReflectionOrderChannelAssignments = k ;
         }
         numberOfReflectionOrderChannelAssignments = highOrderLimit ; 

          // ALLOCATE SPACE FOR ASSIGNMENTS.
         ivec( reflectionOrderChannelAssignments, numberOfReflectionOrderChannelAssignments ) ;


         // READ IN VALUES
         for(i = 0; i < numberOfReflectionOrderChannelAssignments; i++ ){
            if( (i % k) == 0 ) rewind( data ) ; 
	    fscanf( data,  " %f ",  &temp ) ; 

            reflectionOrderChannelAssignments[ i ] = (int) temp ; // %%%
            // CHECK FOR ZERO OR LESS.
            if( reflectionOrderChannelAssignments[ i ] <= 0 )
            {
               prt( "\n\n" ) ; 
               pri( reflectionOrderChannelAssignments[ i ], 
                  "REFLECTION ORDER  IMPULSE RESPONSE CHANNEL ASSIGNMENT" ) ; 
               prt( "--------> ERROR: CHANNELS ARE NUMBERED FROM  CHANNEL 1." ) ; 
               prt( ". . . BYE.\n\n" ) ;
               exit(EXIT_FAILURE); 
            } ;
         } ;
      }; 

      fclose( data ) ; 


      fprintf( stderr, "\n" )  ;
      fprintf( stderr, "\nREFLECTION ORDER IMPULSE RESPONSE CHANNEL ASSIGNMENTS:" ) ;  
      for( order = 0 ; order < numberOfReflectionOrderChannelAssignments; order++ ){
         fprintf( stderr, "\nREFLECTION ORDER: %d CHANNEL: %d", order + 1,  
            reflectionOrderChannelAssignments[ order ] ) ; // %%% 
      } ;
      fprintf( stderr, "\n" )  ; 

   } ;


} ;

bool makeReflectionOrderImpulseResponseNow(
   int outputFileChannelNumber,
   int reflectionNumber,
   int reflectionOrderImpulseResponseNumber,
   int *reflectionOrderImpulseResponseNowBaseIndex,
   int *reflectionOrderImpulseResponseNowSize,
   float *windowedSamplePeakAmp
)
{
   int i, n ;
   int windowedSampleBeginIndex, windowedSampleEndIndex, windowedSampleNumberOfFrames ;
   int windowedSampleNumberOfAttackFrames, windowedSampleNumberOfReleaseFrames ; 
   int thisOrder ; 
   float ROamp, impulseAmp ;  
   
   float prop, ampEnv;
   int ROimpulseResponsePresenceLevelsIndex ;   

   static bool first=true ; 

//   prt( "IN makeReflectionOrderImpulseResponseNow" ) ; 
//   pri( reflectionOrderImpulseResponseNumber, "reflectionOrderImpulseResponseNumber" ) ; 

   for(i = 0; i < reflection_order_numberOfFrames; i++)
      reflectionOrderImpulseResponseNow[ i ] = reflectionOrderImpulseResponses[ 
            (((-1 * reflectionOrderImpulseResponseNumber) - 1) * reflection_order_numberOfFrames) + i 
      ] ;

   if(reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 == 0)
   {
//      prt( "MAKING -> REGULAR <- FORM OF ReflectionOrderImpulseResponseNow" ) ; 

      *reflectionOrderImpulseResponseNowBaseIndex = 0 ; 
      *reflectionOrderImpulseResponseNowSize = reflection_order_numberOfFrames ;
      return( true ) ; 

   }else
   {
//      prt( "making WINDOWED ReflectionOrderImpulseResponseNow" ) ; 

      // SAMPLED AND WINDOWED
      if( first )
      {
//         prt( "IN makeReflectionOrderImpulseResponseNow FIRST TIME" ) ; 
         // MAKE ORDER SAMPLE DURATION LENGTHS
         fvec( sampleOrderAttackDurations, highOrderLimit ) ; 
         fvec( sampleOrderReleaseDurations, highOrderLimit ) ; 
         for(i = 0; i < highOrderLimit ; i++)
         {
            prop = (highOrderLimit <= 1) ? 1. : (float) i / (float) (highOrderLimit - 1) ;
            sampleOrderAttackDurations[ i ] = 
               curve( low_order_attack_duration, high_order_attack_duration, prop, 0. ); 
            sampleOrderReleaseDurations[ i ] = 
               curve( low_order_release_duration, high_order_release_duration, prop, 0. );    
 
           fprintf( stderr, "\nORDER: %d\tATTACK: %f\tRELEASE: %f", 
              i + 1, sampleOrderAttackDurations[ i ], sampleOrderReleaseDurations[ i ] ) ; 
        } ;
         first = false ; 
      } ;

//pri( thisOrder, "thisOrder" ) ; 
      thisOrder = reflectionOrders[ 
            reflectionDataSetChannelPointer[ outputFileChannelNumber ] + reflectionNumber ] ; 

      if( reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 == 2 )
      {
         windowedSampleBeginIndex = (int)((reflectionTimes[ 
            reflectionDataSetChannelPointer[ outputFileChannelNumber ] + reflectionNumber] * 
                (float) osr) + 0.5) ;
      }else
      {

         windowedSampleBeginIndex = (int)((orderAverageReflectionTimes[thisOrder - 1] * 
                (float) osr) + 0.5) ;
      } ;

//prf( orderAverageReflectionTimes[thisOrder - 1], "orderAverageReflectionTimes[thisOrder - 1]" ) ; 
//pri( windowedSampleBeginIndex, "windowedSampleBeginIndex" ) ; 

      windowedSampleEndIndex = windowedSampleBeginIndex + 
         ( ( (sampleOrderAttackDurations[thisOrder - 1] + sampleOrderReleaseDurations[thisOrder - 1]) * 
            (float) osr) + 0.5) ;

//pri( windowedSampleEndIndex, "windowedSampleEndIndex" ) ; 

      // IF DELAY IS BEYOND END OF IMPULSE RESPONSE, THEN SKIP AND RETRUN FALSE FOR NO WRITE OF REFLECTION.
      if( windowedSampleBeginIndex > reflection_order_numberOfFrames ) return( false ) ; 

      // ENVELOPE VALUES
      windowedSampleNumberOfFrames = windowedSampleEndIndex - windowedSampleBeginIndex ;
      windowedSampleNumberOfAttackFrames = sampleOrderAttackDurations[thisOrder - 1] * (float) osr ;
      windowedSampleNumberOfReleaseFrames = windowedSampleNumberOfFrames - 
         windowedSampleNumberOfAttackFrames ;

//pri( windowedSampleNumberOfFrames, "windowedSampleNumberOfFrames" ) ; 
//pri( windowedSampleNumberOfAttackFrames, "windowedSampleNumberOfAttackFrames" ) ; 
//pri( windowedSampleNumberOfReleaseFrames, "windowedSampleNumberOfReleaseFrames" ) ; 


      *reflectionOrderImpulseResponseNowBaseIndex = windowedSampleBeginIndex ;
      n = windowedSampleBeginIndex ; 
      *windowedSamplePeakAmp = 0. ; 
      ROimpulseResponsePresenceLevelsIndex = 
         ((-1 * reflectionOrderImpulseResponseNumber) - 1) % numberOfROimpulseResponsePresenceLevels ;

pri( numberOfROimpulseResponsePresenceLevels, "numberOfROimpulseResponsePresenceLevels" ) ; 
pri( ROimpulseResponsePresenceLevelsIndex, "ROimpulseResponsePresenceLevelsIndex" ) ; 

      ROamp = (useROIRpresenceLevelsFlag ) ?
         dB_to_amp( ROimpulseResponsePresenceLevels[ ROimpulseResponsePresenceLevelsIndex ] ) :
         1.0 ; 
      impulseAmp = 1. - ROamp ; 
      for(i = 0; i < windowedSampleNumberOfAttackFrames; i++)
      {
         if( n < reflection_order_numberOfFrames )
         {
            prop = (float) i / (float) windowedSampleNumberOfAttackFrames ;
            ampEnv = curve( 0., 1., prop, envelope_shape_index ); 
//prf( ampEnv, "attack ampEnv" ) ; 
            reflectionOrderImpulseResponseNow[ n ] *= (ampEnv * ROamp) ;
            if( reflectionOrderImpulseResponseNow[ n ] > *windowedSamplePeakAmp ) 
               *windowedSamplePeakAmp = reflectionOrderImpulseResponseNow[ n ] ;
            n++ ;
         }else
         {
//prt( "attack break" ) ; 
            break ; 
         } ;
      } ;
      for(i = 0; i < windowedSampleNumberOfReleaseFrames; i++)
      {
         if( n < reflection_order_numberOfFrames )
         {
            prop = (float) i / (float) (windowedSampleNumberOfReleaseFrames - 1) ;
            ampEnv = curve( 1., 0., prop, -1. * envelope_shape_index ); 
//prf( ampEnv, "release ampEnv" ) ; 
            reflectionOrderImpulseResponseNow[ n ] *= (ampEnv * ROamp) ;
            if( reflectionOrderImpulseResponseNow[ n ] > *windowedSamplePeakAmp ) 
               *windowedSamplePeakAmp = reflectionOrderImpulseResponseNow[ n ] ;
            n++ ;
         }else
         {
//prt( "release break" ) ; 
            break ; 
         } ;
      } ;
      if( useROIRpresenceLevelsFlag  )
         reflectionOrderImpulseResponseNow[ windowedSampleBeginIndex ] += impulseAmp ; 

      *reflectionOrderImpulseResponseNowSize = n - windowedSampleBeginIndex ; 

      // ADD PRESENCE



//pri( *reflectionOrderImpulseResponseNowSize, "*reflectionOrderImpulseResponseNowSize" ) ; 

      if( *windowedSamplePeakAmp == 0. ) return( false ) ; 
      else return( true ) ; 
   } ;

} ;

void findAverageReflectionOrderDelayTimes(
   int outputFileChannelNumber
)
{
   static bool first=true ; 
   int i, thisOrder, order, count ; 

   if( first )
   {
      fvec( orderAverageReflectionTimes, highOrderLimit ) ; 
      first = false ; 
   } ;   

   // ZERO
   for(i = 0; i < highOrderLimit; i++ ) orderAverageReflectionTimes[ i ] = 0. ; 

   // SUM

   prt( "AVERAGE REFLECTION TIMES BY ORDER" ) ; 
   for(order = 1; order <= highOrderLimit; order++)
   {
      count = 0 ; 
      for( i = 0; i < viableReflectionCount[ outputFileChannelNumber ]; i++ ){
         thisOrder = reflectionOrders[reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i] ;
         if( thisOrder == order ){
            orderAverageReflectionTimes[order - 1] += 
               reflectionTimes[reflectionDataSetChannelPointer[ outputFileChannelNumber ] + i] ;
            count++ ;
         } ;
      } ;
      orderAverageReflectionTimes[order - 1] /= (float) count ; 
      fprintf( stderr, "\n%d: %f seconds", order, orderAverageReflectionTimes[order - 1] ) ; 
   } ;
} ;


void getReflectionOrderDecibelGainscaleLevels()
{ // 
   int i, k;
   float temp ;  

   int	numberOfReflectionOrders ;

   numberOfReflectionOrders = highOrderLimit ; 


   if( (strcasecmp( reflection_order_dB_gainscale_factors_file, datafile2 ) == 1) ||
       (strlen( reflection_order_dB_gainscale_factors_file ) == 0)
    ){

      fprintf( stderr, 
         "\n\nREFLECTION ORDER DB GAINSCALE LEVELS: NO FILE SPECIFIED. USING UNITY GAIN FOR ALL." ) ;
      numberOfReflectionOrderDecibelGainscaleLevels = 1 ; 
      fvec( reflectionOrderDecibelGainscaleLevels, numberOfReflectionOrderDecibelGainscaleLevels ) ;
      reflectionOrderDecibelGainscaleLevels[ 0 ] = 0. ;

   }else{
       prs( reflection_order_dB_gainscale_factors_file, "\n\nREFLECTION ORDER DB GAINSCALE LEVELS FILE" ) ; 

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( reflection_order_dB_gainscale_factors_file,  new_datafile,  2 ) ; 

      //**************************GET DATA   
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  reflection_order_dB_gainscale_factors_file ) ;  
         exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      }    		

      if( k > numberOfReflectionOrders){
//         pri( k, "NUMBER OF REFLECTION ORDER GAINSCALE LEVELS IN FILE" ) ;
         pri( numberOfReflectionOrders, "NUMBER OF REFLECTION ORDERS" ) ; 
         prt( "----> NUMBER OF LEVELS EXCEEDS THE NUMBER OF REFLECTION ORDERS; " ) ; 
         prt( "      WILL TRUNCATE TO THE NUMBER OF REFLECTION ORDERS." ) ;   
         numberOfReflectionOrderDecibelGainscaleLevels = numberOfReflectionOrders ; 
      }else
      {
         numberOfReflectionOrderDecibelGainscaleLevels = k ;
      } ;
      pri( numberOfReflectionOrderDecibelGainscaleLevels, "numberOfReflectionOrderDecibelGainscaleLevels" ) ; 

       // ALLOCATE SPACE FOR ASSIGNMENTS.
      fvec( reflectionOrderDecibelGainscaleLevels, numberOfReflectionOrderDecibelGainscaleLevels ) ;

      rewind( data ) ; 

      // READ IN VALUES
      for(i = 0; i < numberOfReflectionOrderDecibelGainscaleLevels; i++ )
	 fscanf( data,  " %f ",  &reflectionOrderDecibelGainscaleLevels[ i ] ) ; 

   }; 

   fclose( data ) ; 


   fprintf( stderr, "\n" )  ;
   fprintf( stderr, "\nREFLECTION ORDER DB GAINSCALE LEVELS:" ) ;  
   for( reflectionOrder = 0 ; reflectionOrder < numberOfReflectionOrderDecibelGainscaleLevels; 
      reflectionOrder++ ){
      fprintf( stderr, "\nREFLECTION ORDER: %d LEVEL in dB: %f", reflectionOrder + 1, 
         reflectionOrderDecibelGainscaleLevels[ reflectionOrder ] ) ; 
   } ;
   fprintf( stderr, "\n" )  ; 

} ;


void findreflectionOrderCVOrderSequences()
{
   int seqBegin, index, i, seqVal, order, k;
   
    
   
    
   
 

   if( reflectionOrderImpulseResponsesFlag )
   {
      // MAKE MEMORY FOR HOLDING CONVOLUTION SEQUENCES.
      prt( "MAKING SPACE: reflectionOrderCVOrderSequences" ) ;
      ivec( reflectionOrderCVOrderSequences, highOrderLimit * highOrderLimit ) ; 
      prt( "MAKING SPACE: reflectionOrderCVOrderSequenceSizes" ) ;
      ivec( reflectionOrderCVIRSequences, highOrderLimit * highOrderLimit ) ;
      reflectionOrderCVOrderSequencesMaxSizeLength = 0 ; 
      ivec( reflectionOrderCVOrderSequenceSizes, highOrderLimit ) ; 

      // MAKE SEQUENCES
      for( order = 1, index = 0; order <= highOrderLimit; order++, index++ )
      {
         seqBegin = order + reflection_order_IR_convolution_mode ;
         if( seqBegin < 1 ) seqBegin = 1 ; 
         if( seqBegin > highOrderLimit ) seqBegin = highOrderLimit ;
         reflectionOrderCVOrderSequenceSizes[ index ] = abs(order - seqBegin) + 1 ;

         if( reflectionOrderCVOrderSequenceSizes[ index ] > 
                reflectionOrderCVOrderSequencesMaxSizeLength )
            reflectionOrderCVOrderSequencesMaxSizeLength = 
               reflectionOrderCVOrderSequenceSizes[ index ] ; 

         // FILL SEQUENCE WITH ADDRESSES.
         for(i = 0, seqVal = seqBegin; i < reflectionOrderCVOrderSequenceSizes[ index ]; 
            i++, seqVal -= copysign( 1, reflection_order_IR_convolution_mode) )
         { 

            reflectionOrderCVOrderSequences[ (index * highOrderLimit) + i ] = seqVal ;
//            l = seqVal > numberOfReflectionOrderChannelAssignments ? 
//               numberOfReflectionOrderChannelAssignments: seqVal ;
            reflectionOrderCVIRSequences[  (index * highOrderLimit) + i  ] = seqVal ;
         } ;
      } ;

      // PRINT
      prt( "\n\n ******** REFLECTION ORDER CONVOLUTION SEQUENCES ***********" ) ; 
      for( order = 1, index = 0; order <= highOrderLimit; order++, index++ )
      {
         fprintf( stderr, "\norder %d: ", order ) ;
         for(i = 0; i < reflectionOrderCVOrderSequenceSizes[ index ]; i++ )
         {
            k = (index * highOrderLimit) + i ;
            fprintf( stderr, " (%d) %d", 
              reflectionOrderCVOrderSequences[ k ], reflectionOrderCVIRSequences[ k ] ) ;
         } ;
      } ;
   } ;
} ;



void copyFloatArray(
   float a[],
   int l,
   float b[]
)
{   // COPY a OF LENGTH l INTO b.
   int i; 
   for(i = 0; i < l; i++) b[ i ] = a[ i ] ;
} ;




void findLargestInteger(
   int *largest,
   int *possible
)
{
   if( *possible > *largest ) *largest = *possible ; 

} ;


// MAKE ARRAY SPACE FOR SORTING THE OUTPUT WALL REFLECTION PATTERNS. 
void makeOrIncreaseMemorySpaceForSavedWallReflectionPatterns()
{
   static bool first = true ; 
   static int startSize=10, expandSize=100 ; 
 
   int *fileIRCollectionCodesTEMP ;
   int *fileIRCollectionCodeLengthsTEMP ;
   long int *fileIRCollectionAddressesTEMP ;
   long int *fileIRCollectionLengthsTEMP ;

   int i ; 

   if( first )
   {
      prt( ". . . MAKING MEMORY SPACE FOR SAVED IMPULSE RESPONSES." ) ;  
      numberOfReflections = startSize ;
      oldNumberOfReflections = numberOfReflections ; 
      fvec( fileIRCollectionData, fileIRCollectionDataMemorySizeNow ) ;  
   }else
   {
      prt( ". . . ENLARGING MEMORY SPACE FOR SAVED IMPULSE RESPONSES." ) ;  
      oldNumberOfReflections = numberOfReflections ;
      numberOfReflections += expandSize ;
      pri( oldNumberOfReflections, "PREVIOUS NUMBER OF REFLECTION PATHS" ) ; 
      pri( numberOfReflections, "NEW NUMBER OF REFLECTION PATHS" ) ; 
   } ;

   if( first == false )
   {
      // MAKE TEMP MEMORY
      ivec( fileIRCollectionCodesTEMP, oldNumberOfReflections * (highOrderLimit * 2) ) ; 
      ivec( fileIRCollectionCodeLengthsTEMP, oldNumberOfReflections ) ; 
      livec( fileIRCollectionAddressesTEMP, oldNumberOfReflections ) ;
      livec( fileIRCollectionLengthsTEMP, oldNumberOfReflections ) ;
      
      // TRANSFER DATA TO TEMP
      for(i = 0; i < (oldNumberOfReflections * (highOrderLimit * 2)) ; i++)
         fileIRCollectionCodesTEMP[i] = fileIRCollectionCodes[i] ;

      for(i = 0; i < (oldNumberOfReflections) ; i++)
      {
         fileIRCollectionCodeLengthsTEMP[i] = fileIRCollectionCodeLengths[i] ;
         fileIRCollectionAddressesTEMP[i] = fileIRCollectionAddresses[i] ;
         fileIRCollectionLengthsTEMP[i] = fileIRCollectionLengths[i] ;
      } ;
      // FREE POINTERS
      free( fileIRCollectionCodes ) ;
      free( fileIRCollectionCodeLengths ) ;
      free( fileIRCollectionAddresses ) ;
      free( fileIRCollectionLengths ) ;
   } ;

   // MAKE SPACE
   ivec( fileIRCollectionCodes, numberOfReflections * (highOrderLimit * 2) ) ; 
   ivec( fileIRCollectionCodeLengths, numberOfReflections ) ; 
   livec( fileIRCollectionAddresses, numberOfReflections ) ;
   livec( fileIRCollectionLengths, numberOfReflections ) ;

   if( first == false )
   {
      // TRANSFER DATA OUT OF TEMP
      for(i = 0; i < (oldNumberOfReflections * (highOrderLimit * 2)) ; i++)
         fileIRCollectionCodes[i] = fileIRCollectionCodesTEMP[i] ;

      for(i = 0; i < (oldNumberOfReflections) ; i++)
      {
         fileIRCollectionCodeLengths[i] = fileIRCollectionCodeLengthsTEMP[i] ;
         fileIRCollectionAddresses[i] = fileIRCollectionAddressesTEMP[i] ;
         fileIRCollectionLengths[i] = fileIRCollectionLengthsTEMP[i] ;
      } ;
      // FREE TEMP SPACE
      free( fileIRCollectionCodesTEMP ) ;
      free( fileIRCollectionCodeLengthsTEMP ) ;
      free( fileIRCollectionAddressesTEMP ) ;
      free( fileIRCollectionLengthsTEMP ) ;
   } ;
   first = false ; 

} ;


// ****

void getWallImpulseResponsePresenceLevels()
{ //
   int i, k, wall ;
   float temp ;  

   prt( "\n\n***** WALL IMPULSE RESPONSE PRESENCE LEVELS:" ) ;
   if( (strcasecmp( wall_impulse_response_decibels_presence_file, datafile2 ) == 1) ||
       (strlen( wall_impulse_response_decibels_presence_file ) == 0)
    ){

      prt( "\nNO FILE SPECIFIED." ) ; 
      prt( "ASSIGNING FULL (PRE-GAINSCALE) PRESENCE TO ALL WALL IMPULSE RESPONSES." ) ; 
      numberOfWallImpulseResponsePresenceLevels = numberOfWalls ; 
      fvec( wallImpulseResponsePresenceLevels, numberOfWallImpulseResponsePresenceLevels ) ; 
      for(i = 0; i < numberOfWallImpulseResponsePresenceLevels; i++)
         wallImpulseResponsePresenceLevels[ i ] = 0. ; 
   }else
   {

       prs( wall_impulse_response_decibels_presence_file, 
           "\n\nWALL IMPULSE RESPONSE DECIBELS PRESENCE FILE" ) ; 

       // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
       cut_data_lines( wall_impulse_response_decibels_presence_file,  new_datafile,  2 ) ; 

      //**************************GET DATA   
      // OPEN FILE
      if( (data = fopen( new_datafile, "r")) == NULL ){
         fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  
              wall_impulse_response_decibels_presence_file ) ;  
         exit(EXIT_FAILURE); 
      }
 
      // COUNT VALUES IN FILE
      k = 0 ; 
      while( fscanf( data,  " %f ",  &temp ) != EOF ){
         k++ ;
      }    		

      if( k > numberOfWalls ){
         pri( numberOfWalls, "NUMBER OF WALLS" ) ; 
         prt( "----> NUMBER OF ASSIGNMENTS EXCEEDS THE NUMBER OF WALLS." ) ; 
         prt( "NUMBER OF ASSIGNMENTS WILL BE TRUNCATED TO THE NUMBER OF WALLS." ) ;   
      }else if( k < numberOfWalls )
      {
         numberOfWallImpulseResponsePresenceLevels = k ;
         pri( numberOfWalls, "NUMBER OF WALLS" ) ; 
         prt( "----> NUMBER OF LEVELS IN FILE IS LESS THAN THE NUMBER OF WALLS." ) ; 
         prt( "FILE LEVELS WILL BE LOOP-ASSIGNED TO REMAINING WALLS, AS NEEDED." ) ;   
      } ;

      numberOfWallImpulseResponsePresenceLevels = numberOfWalls ; 


       // ALLOCATE SPACE FOR LEVELS.
      fvec( wallImpulseResponsePresenceLevels, numberOfWallImpulseResponsePresenceLevels ) ;


      rewind( data ) ; 

      // READ IN VALUES
      for(i = 0; i < numberOfWalls; i++ ){
         if( (i % numberOfWallImpulseResponsePresenceLevels) == 0 ) rewind( data ) ;
         fscanf( data,  " %f ",  &temp ) ; 
         if( temp > 0. )
         {
            prt( "\n\n" ) ; 
            pri( temp, "WALL IMPULSE RESPONSE PRESENCE LEVEL." ) ; 
            prt( "--------> ERROR: WALL IMPULSE RESPONSE PRESENCE LEVELS MUST BE 0 dB OR LESS." ) ; 
            prt( ". . . BYE.\n\n" ) ;
            exit(EXIT_FAILURE); 
         } ;
         wallImpulseResponsePresenceLevels[ i ] = (int) temp ; // %%%
      } ;
   }; 

   fclose( data ) ; 


   fprintf( stderr, "\n" )  ;
   for( wall = 0 ; wall < numberOfWallImpulseResponsePresenceLevels; wall++ ){
      fprintf( stderr, "\n%d) %f dB", wall + 1,  
         wallImpulseResponsePresenceLevels[ wall ] ) ; // %%% 
   } ;
   fprintf( stderr, "\n" )  ; 

} ;



void balanceWallImpulseResponseAgainstPulseUsingPresence()
{
   float wallAmp, impulseAmp ;
   int i, n ;
      

   if( wallImpulseResponsesFromSoundFileFlag )
   {

      for(i = 0; i < numberOfWallChannelAssignments; i++)
      {
         wallAmp = dB_to_amp( wallImpulseResponsePresenceLevels[ i ] ); 
         impulseAmp = 1. - wallAmp ; 
         for(n = 0; n < wall_numberOfFrames; n++)
         {
            wallImpulseResponses[ (i * wall_numberOfFrames) + n ] *= wallAmp ; 
         } ;
         wallImpulseResponses[ (i * wall_numberOfFrames) ] += impulseAmp ; 
      } ;
   } ;

} ;

void balanceROimpulseResponseAgainstPulseUsingPresence()
{
   float ROamp, impulseAmp ;
   int n, order ;
      


   if( (reflection_order_impulse_responses__off_0__on_1 == 1) && useROIRpresenceLevelsFlag )
   {
      // REFLECTION ORDER IMPULSE RESPONSES IS **ON**.

      if( reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 == 0 )
      {
          // ** NOT IN SAMPLE MODE **

         for(order = 0; order < numberOfReflectionOrderChannelAssignments; order++)
         {
            ROamp = dB_to_amp( 
               ROimpulseResponsePresenceLevels[ order % numberOfROimpulseResponsePresenceLevels ] ); 
            impulseAmp = 1. - ROamp ; 
            for(n = 0; n < reflection_order_numberOfFrames; n++)
            {
               reflectionOrderImpulseResponses[ (order * reflection_order_numberOfFrames) + n ] *= ROamp ; 
            } ;
            reflectionOrderImpulseResponses[ (order * reflection_order_numberOfFrames) ] += impulseAmp ; 
         } ;
      } ;
   } ;
} ;


void preConvolveReflectionOrderImpulseResponsesWithIrconvolver()
{
   
   
   int order, index, k, i;

   
   
   
   
   char	soundFileName[ STRING_SIZE ] ;
   char	outputSoundFileName[ STRING_SIZE ]="" ;
   char	outputSoundFilesNameString[ STRING_SIZE ]="" ;
   char	command[ 40000 ] ;
   



   SF_INFO SFinfoPC ;
   SNDFILE *audioFilePointer ; 
  

   if( 
       reflectionOrderImpulseResponsesFlag &&
      (reflection_order_IR_convolution_mode != 0) && (highOrderLimit > 1) )
   {
      if( (abs( reflection_order_IR_convolution_mode ) + 1) > highOrderLimit )
      {
         prt( "ADJUSTING REFLECTION ORDER CONVOLUTION MODE TO NOT EXCEED HIGHEST ORDER." ) ; 
         reflection_order_IR_convolution_mode = reflection_order_IR_convolution_mode < 0 ?         
            (highOrderLimit - 1) * -1 :  (highOrderLimit - 1) ; 
      } ;
      pri( reflection_order_IR_convolution_mode, 
         "PRE-CONVOLVING REFLECTION ORDER IMPULSE RESPONSES FOR CONVOLUTION MODE:" ) ; 


      // MAKE A SOUND FILE FOR EVERY REFLECTION ORDER IMPULSE RESPONSE IN /tmp.
      SFinfoPC.channels = 1 ; 
      SFinfoPC.samplerate = osr ; 
      SFinfoPC.format = oformat ;
      SFinfoPC.frames = (long int) reflection_order_numberOfFrames ;

      for( order = 1, index = 0; order <= highOrderLimit; order++, index++ )
      {
         fprintf( stderr, "\nREFLECTION ORDER %d: ", order ) ;
         sprintf( soundFileName, "/tmp/soundFile%d.au", order ) ; 
           
         prs( soundFileName, "IMPULSE RESPONSE SOUND FILE NAME" ) ; 
         fprintf( stderr, "\n -- CHANNELS: %d\n -- SAMPLE RATE: %d\n -- FRAMES: %d\n", 
		SFinfoPC.channels, SFinfoPC.samplerate, (int) reflection_order_numberOfFrames ) ; 
         // OPEN/CREATE NEW OUTPUT FILE HEADER
         if (! (audioFilePointer = sf_open (soundFileName, SFM_WRITE, & SFinfoPC )))
         {   
            fprintf (stderr, "\n . . . UNABLE TO CREATE SOUND FILE %s .", soundFileName ) ;
            bannero() ; puts(sf_strerror (NULL)) ; exit(EXIT_FAILURE) ;
         } ;
         if( ! ( sf_format_check (&SFinfoPC) ) ){
		fprintf( stderr, "\nAFTER CREATE OF OUTPUT FILE: INVALID SOUND FILE FORMAT" ) ; 
         } ; 

         // POSITION AT BEGINNING AND TRUNCATE.
         sf_count_t frames = 0 ; 
         k = sf_command(audioFilePointer, SFC_FILE_TRUNCATE, &frames, sizeof (frames)) ; 

         // WRITE BUFFER OUT TO SOUNDFILE
         sf_write_float( audioFilePointer, 
            &reflectionOrderImpulseResponses[index * reflection_order_numberOfFrames],
               reflection_order_numberOfFrames  ) ; 
            sf_close( audioFilePointer ) ; 

      } ;

      // CONVOLUTION SEQUENCES
      for( order = 1, index = 0; order <= highOrderLimit; order++, index++ )
      {
         fprintf( stderr, "\nREFLECTION ORDER %d: ", order ) ;
         fprintf( stderr, "\nSEQUENCE: " ) ;
         for(i = 0; i < reflectionOrderCVOrderSequenceSizes[ index ]; i++ )
            fprintf( stderr, "%d ", reflectionOrderCVOrderSequences[(index * highOrderLimit) + i] ) ; 

         // MAKE EMPTY COMMAND
         command[0] = '\0' ;

         sprintf( command, "%scp /tmp/soundFile%d.au /tmp/tempOutput.au ; ", 
            command, reflectionOrderCVOrderSequences[ (index * highOrderLimit) + 0 ] ) ;          

         for(i = 1; i < reflectionOrderCVOrderSequenceSizes[ index ]; i++ )
         {
            // COPY/MOVE tempOutputFileName TO FIRST INPUT FOR COMPOUND RECURSION RUN; 
            sprintf( command, "%scp /tmp/tempOutput.au /tmp/soundFile.INPUT1.au ; ", command ) ; 
            // COPY THE NEXT CONVOLVE SEQUENCE FILE INTO SECOND FILE SPACE
            sprintf( command, "%scp /tmp/soundFile%d.au /tmp/soundFile.INPUT2.au ; ", 
               command, reflectionOrderCVOrderSequences[ (index * highOrderLimit) + i ] ) ; 
            sprintf( command, "%s %s", command, 

              "impulseresponse -b0 -e0 -P0 -a/tmp/sp -N2 -d-0 -C0 "
              "/tmp/soundFile.INPUT1.au /tmp/Impulse_Response_Data_File ; "

              "irconvolver -E/tmp/Impulse_Response_Data_File -a1 -J0 -A-1000 -q0 -r0 "
              " -s2000 -t20000 -g-0 -G-0 -D1000 -f20000 -h-0 -H-0 -x0 -P-0 -B-10 -F-96 "
              " -Z-3 -z-2 -C1 -M1 -b0 -e0 -d1 -_0 -=0 -p0 -i1  /tmp/soundFile.INPUT2.au "
              " /tmp/tempOutput.au ; "

            ) ;
         } ;

         sprintf( outputSoundFileName, "/tmp/soundFile%d.cv.FINAL.OUTPUT.au", order ) ; 
         // COPY/MOVE /tmp/tempOutput.au TO FINAL OUTPUT FILE
         sprintf( command, "%scp /tmp/tempOutput.au %s ; ", command, outputSoundFileName ) ;          
         sprintf( outputSoundFilesNameString, "%s %s", outputSoundFilesNameString, outputSoundFileName ) ;          

         prt( command ) ; 
         system( command ) ; 


         // COLLECT OUTPUT FILES INTO ONE FILE WITH channelcollect.
         sprintf( command, "mv /tmp/tempOutput.au /tmp/soundFile.FINAL.OUTPUT.ALL.au ; " ) ; 
         sprintf( command, "%schannelcollect -i -o/tmp/soundFile.FINAL.OUTPUT.ALL.au %s",
           command, outputSoundFilesNameString ) ;
         prt( command ) ; 
         system( command ) ; 

      } ;

// pri( numberOfReflectionOrderChannelAssignments, "numberOfReflectionOrderChannelAssignments" ) ; 
      for( i = 0; i < numberOfReflectionOrderChannelAssignments; i++ )
         reflectionOrderChannelAssignments[ i ] = i + 1 ; 

      free( reflectionOrderImpulseResponses ) ; 

      strcpy( reflectionOrderImpulseResponseInputSoundFileName, "/tmp/soundFile.FINAL.OUTPUT.ALL.au" ) ; 

      prt( "READING IN NEW CONVOLVED REFLECTION ORDER IMPULSE RESPONSES . . . " ) ; 
      readInReflectionOrderImpulseResponses() ;




   } ;


//   exit(EXIT_SUCCESS) ; 

} ;

void truncateCVOrderSequences()
{
   int index, k, l, i, j ; 
   if( reflectionOrderImpulseResponsesFlag )
   {
      prt( "TRUNCATING REFLECTION ORDER CONVOLUTION SEQUENCES TO SINGLE TERMINATING ORDER RESPONSE." ) ; 

      for(index = 0; index < highOrderLimit; index++)
      {
         pri( reflectionOrderCVOrderSequenceSizes[ index ], "reflectionOrderCVOrderSequenceSizes[ index ]" ) ; 
         k = index * highOrderLimit ;
         fprintf( stderr, "\nSEQUENCE: " ) ; 
         for(i = k, j = 0; j < reflectionOrderCVOrderSequenceSizes[ index ]; i++, j++ )
            fprintf( stderr, "%d ", reflectionOrderCVOrderSequences[ i ] ) ; 

         l = reflectionOrderCVOrderSequences[ 
              k + reflectionOrderCVOrderSequenceSizes[ index ] - 1 
            ] ;
         pri( l, "l" ) ; 
         reflectionOrderCVOrderSequences[ k ] = l ;
         reflectionOrderCVOrderSequenceSizes[ index ] = 1 ; 
      } ;
   } ;
} ;



void makeIR_DataSpace()
{
   static bool first = true ; 
   int i ; 

   if( first )
   {
      // MAKE OUTPUT IMPULSE RESPONSE SPACE
      pri( IR_DataLength, "IR_DataLength" ) ; 
      fvec( IR_Data, IR_DataLength ) ; 
      first = false ; 
   }else
   {
      // ZERO OUT IR_Data
      for(i = 0; i < IR_DataLength; i++) IR_Data[ i ] = 0. ; 
   } ;


} ;



void getReflectionOrderImpulseResponsePresenceLevels()
{ //
   int i, k;
   float temp ;  


   if( reflection_order_impulse_responses__off_0__on_1 == 1 )
   {
      // REFLECTION ORDER IMPULSE RESPONSES IS **ON**.



      prt( "\n\n***** REFLECTION ORDER IMPULSE RESPONSE PRESENCE LEVELS:" ) ;

      if( (strcasecmp( reflection_order_impulse_response_decibels_presence_file, datafile2 ) == 1) ||
          (strlen( reflection_order_impulse_response_decibels_presence_file ) == 0)
      ){
         // ** NO FILE **
         prt( "\nNO FILE SPECIFIED." ) ; 
         useROIRpresenceLevelsFlag = false ;

      }else
      {
      // ** FILE SPECIFIED **

          prs( reflection_order_impulse_response_decibels_presence_file, 
           "\n\nREFLECTION ORDER IMPULSE RESPONSE DECIBELS PRESENCE FILE" ) ; 

          // MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
          cut_data_lines( reflection_order_impulse_response_decibels_presence_file,  new_datafile,  2 ) ; 

         //**************************GET DATA   
         // OPEN FILE
         if( (data = fopen( new_datafile, "r")) == NULL ){
            fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  
                 reflection_order_impulse_response_decibels_presence_file ) ;  
            exit(EXIT_FAILURE); 
         } ;
 
         // COUNT VALUES IN FILE
         k = 0 ; 
         while( fscanf( data,  " %f ",  &temp ) != EOF ){
            k++ ;
         }    		

         if( k > highOrderLimit ){
            prt( "----> NUMBER OF LEVELS EXCEEDS THE HIGH REFLECTION ORDER LIMIT." ) ; 
            prt( "WILL TRUNCATE TO THE HIGH LIMIT." ) ;   
         }else if( k < highOrderLimit )
         {
            prt( "----> NUMBER OF LEVELS IS LESS THAN THE HIGH REFLECTION ORDER LIMIT." ) ; 
            prt( "WILL LOOP_ASSIGN, AS NEEDED, TO FILL REMAINING HIGH ORDERS." ) ;   
         } ;
         numberOfROimpulseResponsePresenceLevels = highOrderLimit ; 

          // ALLOCATE SPACE FOR LEVELS.
         fvec( ROimpulseResponsePresenceLevels, numberOfROimpulseResponsePresenceLevels ) ;

         // READ IN VALUES
         for(i = 0; i < numberOfROimpulseResponsePresenceLevels; i++ ){
            if( (i % k) == 0 ) rewind( data ) ; 
            fscanf( data,  " %f ",  &temp ) ; 
            if( temp > 0. )
            {
               prt( "\n\n" ) ; 
               pri( temp, "REFLECTION ORDER IMPULSE RESPONSE PRESENCE LEVEL." ) ; 
               prt( "\n--------> ERROR: REFLECTION ORDER IMPULSE RESPONSE PRESENCE LEVELS" ) ; 
               prt( "MUST BE 0 dB OR LESS." ) ; 
               prt( ". . . BYE.\n\n" ) ; exit(EXIT_FAILURE); 
            } ;
            ROimpulseResponsePresenceLevels[ i ] = temp ; // %%%
         } ;

         useROIRpresenceLevelsFlag = true ;
      }; 

      fclose( data ) ; 

      fprintf( stderr, "\n" )  ;
      for( i = 0 ; i < numberOfROimpulseResponsePresenceLevels; i++ ){
         fprintf( stderr, "\n%d) %f dB", i + 1, ROimpulseResponsePresenceLevels[ i ] ) ; // %%% 
      } ;
      fprintf( stderr, "\n" )  ; 

   } ;
} ;


// **
void makePreEchoValues()
{

   prf( maxSpeakerToListenerDistance, "maxSpeakerToListenerDistance" ) ; 
   prf( maxSpeakerToListenerDistanceDelayTime, "maxSpeakerToListenerDistanceDelayTime" ) ; 

   preEchoTime = (maxSpeakerToSpeakerDistanceDelayTime > maxSpeakerToListenerDistanceDelayTime) ? 
    maxSpeakerToSpeakerDistanceDelayTime : maxSpeakerToListenerDistanceDelayTime ;

   prf( preEchoTime, "preEchoTime" ) ; 

   preEchoDistance = preEchoTime * speedOfSoundInFeetPerSecond ;

/*
   frontSourceHeadRoomScalar = 1.0 / pow( (double)
              fmin( 1., (double)(minimumReferenceDistanceInFeet / preEchoDistance ) ), 
              (double) (airAbsorptionExponentForRealSpaceSource * -1.) 
           ) ; 
*/

   maxSpeakerToForwardSourceDistanceLimit = 
      maxSpeakerToListenerDistance - source_minimum_distance_from_listener ;
   frontSourceHeadRoomScalar = (maxSpeakerToForwardSourceDistanceLimit <= minimumReferenceDistanceInFeet) ? 
      1. :
      pow( 
         (double) (minimumReferenceDistanceInFeet / maxSpeakerToForwardSourceDistanceLimit ), 
         (double) (airAbsorptionExponentForRealSpaceSource ) 
      ) ; 

} ; 


void findMaximumSpeakerToListenerDistance()
{
   int sp, pos ;
   float segment[ 4 ], length ;

   maxSpeakerToListenerDistance = 0. ;  

   for(pos = 0; pos < numberOfListenerPositions; pos++ )
   {

      for(sp = 0; sp < numberOfSpeakerPositions; sp++ )
      {
         makeSegment(
            segment,
            &speakerCoordinates[ sp * 2 ],
            &listenerCoordinates[ (pos % numberOfListenerPositions) * 2 ]
         ) ;

         length = findSegmentLength( segment ) ;
         if( length > maxSpeakerToListenerDistance) maxSpeakerToListenerDistance = length ;  
      } ;
   } ;

   maxSpeakerToListenerDistanceDelayTime = maxSpeakerToListenerDistance / speedOfSoundInFeetPerSecond  ;

   
} ;



void findMaximumSpeakerToSpeakerDistance()
{
   int sp0, sp1 ;
   float segment[ 4 ], length ; 

   for(sp0 = 0; sp0 < (numberOfSpeakerPositions - 1); sp0++ )
   {
      for(sp1 = 1; sp1 < numberOfSpeakerPositions; sp1++ )
      {
         makeSegment(
            segment,
            &speakerCoordinates[ sp0 * 2 ],
            &speakerCoordinates[ sp1 * 2 ]
         ) ;

         length = findSegmentLength( segment ) ;
         if( length > maxSpeakerToSpeakerDistance)
         {
            maxSpeakerToSpeakerDistance = length ;  
         } ; 
      } ;
   } ;

   maxSpeakerToSpeakerDistanceDelayTime = maxSpeakerToSpeakerDistance / speedOfSoundInFeetPerSecond  ;

   
} ;

