#include "globals.h"
#include "stdbool.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



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



struct stat st = {0};




float sampleRate, nyquist ;
float *sourceCoordinates ; // ={ 0., 0. } ;

struct func source_minimum_distance_from_listener ; 

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
struct func	RO_IR_BPF_low_rolloff_frequency ;
struct func	RO_IR_BPF_high_rolloff_frequency ;
struct func	RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave ;
struct func	RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave ;

bool	convolveTwoArraysReset=false ; 

int	reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2=0 ; 

char wall_IR_response_envelope[ STRING_SIZE ]="EMPTY\0" ; 

char reflection_order_IR_envelope[ STRING_SIZE ]="EMPTY\0" ; 

struct func wall_IR_truncate_duration ;


struct func reflection_order_IR_truncate_duration  ;

struct func reflected_sound_gain_in_decibels ;
struct func direct_sound_gain_in_decibels ;




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

struct func low_order_attack_duration ;
struct func low_order_release_duration ;
struct func high_order_attack_duration ;
struct func high_order_release_duration ;
struct func envelope_shape_index ;

float 	*impulseResponse0, *impulseResponse1 ; 


float	*reflectionOrderImpulseResponseNow ; 
int	reflectionOrderImpulseResponseNowSize ; 
int	reflectionOrderImpulseResponseNowBaseIndex ;
int	reflectionOrderImpulseResponseNowSize ;

char	crackTempString[ STRING_SIZE ]="EMPTY\0" ; 

struct func	reflection_order_IR_convolution_mode ; 

char	reflection_order_dB_gainscale_factors_file[ STRING_SIZE ]="EMPTY\0" ;

char	wall_impulse_response_decibels_presence_file[ STRING_SIZE ]="EMPTY\0" ;

char	reflection_order_impulse_response_decibels_presence_file[ STRING_SIZE ]="EMPTY\0" ;





bool 	testFlag ; 
int	convolutionCount=0 ;

bool	noCropImpulseResponsesFlag=true ; 

struct func endCropDecibelThreshold ;
struct func endCropReleaseTime ; 

struct func impulse_inclusion_threshold_in_dB ; 

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

struct func wall_impulse_and_gainscale_response_mode ; // -1 = last wall, 1 = first wall, -2 = last 2, 2 = first 2. etc.  

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


struct func WIRBPF_low_rolloff_frequency ;
struct func WIRBPF_high_rolloff_frequency ;
struct func WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave ;
struct func WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave ;

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

struct func highOrderLimit ;
struct func lowOrderLimit ; 
struct func numberOfWalls ;
int	numberOfListenerPositions ; 
int	numberOfSourcePositions ;
int	numberOfSpeakerPositions ; 
struct func rotationOfSyntheticRoomInDegrees ;
char 	soundFileName[ STRING_SIZE ]="" ;
char	plotFileName[ STRING_SIZE ]="" ;
int 	numberOfCorners ;
struct func minDistanceToCornerFromOrigin ; 
struct func maxDistanceToCornerFromOrigin ; 

char 	soundPathsPlotFileName[ STRING_SIZE ]="EMPTY\0" ;
FILE	*plotFilePointer ;
FILE	*channelOrderPlotFilePointers[ 100 ] ;
FILE	*impulseFilesFilePointer ;
char	impulseFilesName[ STRING_SIZE ]="" ;
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

struct func polygonAngleRegularityProportion ; 

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
int *viableReflectionCountByOrder ; 

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

struct func airAbsorptionExponentForReflections ; 
struct func airAbsorptionExponentForVirtualSpaceSource ; 
struct func airAbsorptionExponentForRealSpaceSource ; 
float airAbsorptionExponentAtThreshold=20.0 ; 


struct func reflections_time_scaler ; 

struct func room_X_translation_factor ;
struct func room_Y_translation_factor ;
struct func room_negXscaleFactor ;
struct func room_posXscaleFactor ;
struct func room_negYscaleFactor ;
struct func room_posYscaleFactor ;
struct func room_RotationInDegrees ; 
struct func room_ScaleFactor ; 

struct func speaker_X_translation_factor ;
struct func speaker_Y_translation_factor ;
struct func speaker_negXscaleFactor ;
struct func speaker_posXscaleFactor ;
struct func speaker_negYscaleFactor ;
struct func speaker_posYscaleFactor ;
struct func speaker_RotationInDegrees ; 
struct func speaker_ScaleFactor ; 


FILE *data, *plotData ; 

char datafile2[ STRING_SIZE ] = "EMPTY\0" ; 
char polygonCoordinates_datafile[ STRING_SIZE ] = "EMPTY\0" ; 
char listenerCoordinatesDataFile[ STRING_SIZE ] = "EMPTY\0" ; 
char speakerCoordinatesDataFile[ STRING_SIZE ] = "EMPTY\0" ; 

char new_datafile[ STRING_SIZE ] = "EMPTY\0" ; 

float *IR_Data ;


struct func minimumReferenceDistanceInFeet ;
struct func sourceRotationInDegrees ; 
float	sourceRotation ; 
struct func sourceDispersionPatternRolloffInDecibels ; 


int main( argc, argv )
    int argc ; char *argv[] ;
{

int numSampsBufferedIn, arg_index_Save, normalizeFlag=0 ; 

struct func masterGainInDecibels ;
int outputChannelNumber=0 ;
int polygonCoordinatesSource=0 ;


int L, N ; 

FILE *data ;

float prop, rawProp, soundPathPositionProportion, sourceX, sourceY, angleInRadians ; 

int i,j, k, l,  m, n, nnn=0, i1, i2, np ;

float frac ; 
int R=44100, in, on;
FILE *fopen() ;
char ch,  tempstring[ STRING_SIZE ],  
    scratch[ STRING_SIZE ],  scratch2[ STRING_SIZE ], soundFormatTemplateFile[ STRING_SIZE ],
outputDirectory[ STRING_SIZE ], source_coordinates_plot_file[ STRING_SIZE ],			
 *user,
		command[ 40000 ], thisCommand[ 40000 ], thisImpulseFileName[ STRING_SIZE ] ;
float  temp, temp1,  temp2,  pm,  IR  ;  

float tempBlock[ BLOCKSIZE ], *allChanOutputBlock ; 

SF_INFO inputSFinfo ;  
SF_INFO outputSFinfo ; 


// X COORDINATE FUNCTION
struct  func  xCoordinateFunction ; 

// Y COORDINATE FUNCTION
struct  func  yCoordinateFunction ; 

struct func sourceAngleFromOrigin ;
struct func sourceRadiusFromOrigin ;




struct func sound_path_movement ;

struct func sound_path_movement_warp ;

int number_of_movement_function_points=1 ; 



reflection_order_IR_convolution_mode.L = 1. ; reflection_order_IR_convolution_mode.n = 1. ; 
	reflection_order_IR_convolution_mode.A[ 0 ] = 0. ; 




RO_IR_BPF_low_rolloff_frequency.L = 1. ; RO_IR_BPF_low_rolloff_frequency.n = 1. ; 
	RO_IR_BPF_low_rolloff_frequency.A[ 0 ] = 0. ; 
RO_IR_BPF_high_rolloff_frequency.L = 1. ; RO_IR_BPF_high_rolloff_frequency.n = 1. ; 
	RO_IR_BPF_high_rolloff_frequency.A[ 0 ] = 0. ; 
RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave.L = 1. ; RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave.n = 1. ; 
	RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 0. ; 
RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave.L = 1. ; RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave.n = 1. ; 
	RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 0. ; 


reflection_order_IR_truncate_duration.L = 1. ; reflection_order_IR_truncate_duration.n = 1. ; 
	reflection_order_IR_truncate_duration.A[ 0 ] = 0. ; 


reflected_sound_gain_in_decibels.L = 1. ; reflected_sound_gain_in_decibels.n = 1. ; 
	reflected_sound_gain_in_decibels.A[ 0 ] = 0. ; 

direct_sound_gain_in_decibels.L = 1. ; direct_sound_gain_in_decibels.n = 1. ; 
	direct_sound_gain_in_decibels.A[ 0 ] = 0. ; 


airAbsorptionExponentForVirtualSpaceSource.L = 1. ; airAbsorptionExponentForVirtualSpaceSource.n = 1. ; 
	airAbsorptionExponentForVirtualSpaceSource.A[ 0 ] = 0. ; 


airAbsorptionExponentForRealSpaceSource.L = 1. ; airAbsorptionExponentForRealSpaceSource.n = 1. ; 
	airAbsorptionExponentForRealSpaceSource.A[ 0 ] = 0. ; 

source_minimum_distance_from_listener.L = 1. ; source_minimum_distance_from_listener.n = 1. ; 
	source_minimum_distance_from_listener.A[ 0 ] = 0. ; 


low_order_attack_duration.L = 1. ; low_order_attack_duration.n = 1. ; 
	low_order_attack_duration.A[ 0 ] = 0. ; 
low_order_release_duration.L = 1. ; low_order_release_duration.n = 1. ; 
	low_order_release_duration.A[ 0 ] = 0. ; 
high_order_attack_duration.L = 1. ; high_order_attack_duration.n = 1. ; 
	high_order_attack_duration.A[ 0 ] = 0. ; 
high_order_release_duration.L = 1. ; high_order_release_duration.n = 1. ; 
	high_order_release_duration.A[ 0 ] = 0. ; 
envelope_shape_index.L = 1. ; envelope_shape_index.n = 1. ; 
	envelope_shape_index.A[ 0 ] = 0. ; 

wall_IR_truncate_duration.L = 1. ; wall_IR_truncate_duration.n = 1. ; 
	wall_IR_truncate_duration.A[ 0 ] = 0. ; 


impulse_inclusion_threshold_in_dB.L = 1. ; impulse_inclusion_threshold_in_dB.n = 1. ; 
	impulse_inclusion_threshold_in_dB.A[ 0 ] = 0. ; 


endCropReleaseTime.L = 1. ; endCropReleaseTime.n = 1. ; 
	endCropReleaseTime.A[ 0 ] = 0. ; 


endCropDecibelThreshold.L = 1. ; endCropDecibelThreshold.n = 1. ; 
	endCropDecibelThreshold.A[ 0 ] = 0. ; 

highOrderLimit.L = 1. ; highOrderLimit.n = 1. ; 
	highOrderLimit.A[ 0 ] = 0. ; 
lowOrderLimit.L = 1. ; lowOrderLimit.n = 1. ; 
	lowOrderLimit.A[ 0 ] = 0. ; 

WIRBPF_low_rolloff_frequency.L = 1. ; WIRBPF_low_rolloff_frequency.n = 1. ; 
	WIRBPF_low_rolloff_frequency.A[ 0 ] = 0. ; 
WIRBPF_high_rolloff_frequency.L = 1. ; WIRBPF_high_rolloff_frequency.n = 1. ; 
	WIRBPF_high_rolloff_frequency.A[ 0 ] = 0. ; 
WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave.L = 1. ; WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave.n = 1. ; 
	WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 0. ; 
WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave.L = 1. ; WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave.n = 1. ; 
	WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 0. ; 

wall_impulse_and_gainscale_response_mode.L = 1. ; wall_impulse_and_gainscale_response_mode.n = 1. ; 
	wall_impulse_and_gainscale_response_mode.A[ 0 ] = 0. ; 


reflections_time_scaler.L = 1. ; reflections_time_scaler.n = 1. ; 
	reflections_time_scaler.A[ 0 ] = 0. ; 



room_X_translation_factor.L = 1. ; room_X_translation_factor.n = 1. ; 
	room_X_translation_factor.A[ 0 ] = 0. ; 
room_Y_translation_factor.L = 1. ; room_Y_translation_factor.n = 1. ; 
	room_Y_translation_factor.A[ 0 ] = 0. ; 
room_negXscaleFactor.L = 1. ; room_negXscaleFactor.n = 1. ; 
	room_negXscaleFactor.A[ 0 ] = 0. ; 
room_posXscaleFactor.L = 1. ; room_posXscaleFactor.n = 1. ; 
	room_posXscaleFactor.A[ 0 ] = 0. ; 
room_negYscaleFactor.L = 1. ; room_negYscaleFactor.n = 1. ; 
	room_negYscaleFactor.A[ 0 ] = 0. ; 
room_posYscaleFactor.L = 1. ; room_posYscaleFactor.n = 1. ; 
	room_posYscaleFactor.A[ 0 ] = 0. ; 
room_RotationInDegrees.L = 1. ; room_RotationInDegrees.n = 1. ; 
	room_RotationInDegrees.A[ 0 ] = 0. ; 
room_ScaleFactor.L = 1. ; room_ScaleFactor.n = 1. ; 
	room_ScaleFactor.A[ 0 ] = 0. ; 


speaker_X_translation_factor.L = 1. ; speaker_X_translation_factor.n = 1. ; 
	speaker_X_translation_factor.A[ 0 ] = 0. ; 
speaker_Y_translation_factor.L = 1. ; speaker_Y_translation_factor.n = 1. ; 
	speaker_Y_translation_factor.A[ 0 ] = 0. ; 
speaker_negXscaleFactor.L = 1. ; speaker_negXscaleFactor.n = 1. ; 
	speaker_negXscaleFactor.A[ 0 ] = 0. ; 
speaker_posXscaleFactor.L = 1. ; speaker_posXscaleFactor.n = 1. ; 
	speaker_posXscaleFactor.A[ 0 ] = 0. ; 
speaker_negYscaleFactor.L = 1. ; speaker_negYscaleFactor.n = 1. ; 
	speaker_negYscaleFactor.A[ 0 ] = 0. ; 
speaker_posYscaleFactor.L = 1. ; speaker_posYscaleFactor.n = 1. ; 
	speaker_posYscaleFactor.A[ 0 ] = 0. ; 
speaker_RotationInDegrees.L = 1. ; speaker_RotationInDegrees.n = 1. ; 
	speaker_RotationInDegrees.A[ 0 ] = 0. ; 
speaker_ScaleFactor.L = 1. ; speaker_ScaleFactor.n = 1. ; 
	speaker_ScaleFactor.A[ 0 ] = 0. ; 



sourceRotationInDegrees.L = 1. ; sourceRotationInDegrees.n = 1. ; 
	sourceRotationInDegrees.A[ 0 ] = 0. ; 

sourceDispersionPatternRolloffInDecibels.L = 1. ; sourceDispersionPatternRolloffInDecibels.n = 1. ; 
	sourceDispersionPatternRolloffInDecibels.A[ 0 ] = 0. ; 
minimumReferenceDistanceInFeet.L = 1. ; minimumReferenceDistanceInFeet.n = 1. ; 
	minimumReferenceDistanceInFeet.A[ 0 ] = 0. ; 


masterGainInDecibels.L = 1. ; masterGainInDecibels.n = 1. ; 
	masterGainInDecibels.A[ 0 ] = 0. ; 

airAbsorptionExponentForReflections.L = 1. ; airAbsorptionExponentForReflections.n = 1. ; 
	airAbsorptionExponentForReflections.A[ 0 ] = 0. ; 



numberOfWalls.L = 1. ; numberOfWalls.n = 1. ; 
	numberOfWalls.A[ 0 ] = 0. ; 


rotationOfSyntheticRoomInDegrees.L = 1. ; rotationOfSyntheticRoomInDegrees.n = 1. ; 
	rotationOfSyntheticRoomInDegrees.A[ 0 ] = 0. ; 

minDistanceToCornerFromOrigin.L = 1. ; minDistanceToCornerFromOrigin.n = 1. ; 
	minDistanceToCornerFromOrigin.A[ 0 ] = 0. ; 

maxDistanceToCornerFromOrigin.L = 1. ; maxDistanceToCornerFromOrigin.n = 1. ; 
	maxDistanceToCornerFromOrigin.A[ 0 ] = 0. ; 

polygonAngleRegularityProportion.L = 1. ; polygonAngleRegularityProportion.n = 1. ; 
	polygonAngleRegularityProportion.A[ 0 ] = 0. ; 

/* HERE

???.L = 1. ; ???.n = 1. ; 
	???.A[ 0 ] = 0. ; 
*/



// X COORDINATE FUNCTION
xCoordinateFunction.L = 1. ; xCoordinateFunction.n = 1. ; xCoordinateFunction.A[ 0 ] = 0. ; 

// Y COORDINATE FUNCTION
yCoordinateFunction.L = 1. ; yCoordinateFunction.n = 1. ; yCoordinateFunction.A[ 0 ] = 0. ; 



sourceAngleFromOrigin.L = 1. ; sourceAngleFromOrigin.n = 1. ; sourceAngleFromOrigin.A[ 0 ] = 0. ; 


sourceRadiusFromOrigin.L = 1. ; sourceRadiusFromOrigin.n = 1. ; sourceRadiusFromOrigin.A[ 0 ] = 0. ; 


// SOUND PATH MOVEMENT FUNCTION
sound_path_movement.L = 1. ; sound_path_movement.n = 1. ; sound_path_movement.A[ 0 ] = 0. ; 

// SOUND PATH MOVEMENT FUNCTION
sound_path_movement_warp.L = 1. ; sound_path_movement_warp.n = 1. ; sound_path_movement_warp.A[ 0 ] = 0. ; 








if( argc < 2 )usage() ; 




fprintf( stderr, "\ncommand: \n%s", command ) ; 

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

                   case 'b': strcpy(tempstring, arg_option);
                             reflection_order_IR_convolution_mode.fp = 
		        crackstring( &crackTempString[1], &reflection_order_IR_convolution_mode);  
                             break;

                   case 'c': RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3
                              = (int) crackfloat( &crackTempString[1], ch ) ; break;

                   case 'd': strcpy(tempstring, arg_option);
                             RO_IR_BPF_low_rolloff_frequency.fp = 
		        crackstring( &crackTempString[1], 
			&RO_IR_BPF_low_rolloff_frequency);  
		      break;

                   case 'e': strcpy(tempstring, arg_option);
                             RO_IR_BPF_high_rolloff_frequency.fp = 
		        crackstring( &crackTempString[1], 
			& RO_IR_BPF_high_rolloff_frequency);  
		      break;

                   case 'f': strcpy(tempstring, arg_option);
                             RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave.fp = 
		        crackstring( &crackTempString[1], 
			& RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave);  
		      break;

                   case 'g': strcpy(tempstring, arg_option);
                             RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave.fp = 
		        crackstring( &crackTempString[1], 
			& RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave);  
		      break;

                   case 'h': strcpy(tempstring, arg_option);
                             reflection_order_IR_truncate_duration.fp = 
		        crackstring( &crackTempString[1], 
			& reflection_order_IR_truncate_duration);  
		      break;
                   case 'i': strcpy( reflection_order_IR_envelope, 
                                   &crackTempString[1] ); break ;
                   case 'j': strcpy( wall_impulse_response_decibels_presence_file, 
                                &crackTempString[1] ) ; break;
                   case 'k': strcpy(tempstring, arg_option);
                             reflected_sound_gain_in_decibels.fp = 
		        crackstring( &crackTempString[1], 
			& reflected_sound_gain_in_decibels);  
		      break;

                   case 'l': strcpy(tempstring, arg_option);
                             direct_sound_gain_in_decibels.fp = 
		        crackstring( &crackTempString[1], 
			& direct_sound_gain_in_decibels);  
		      break;

                   case 'm': strcpy(tempstring, arg_option);
                             airAbsorptionExponentForVirtualSpaceSource.fp = 
		        crackstring( &crackTempString[1], 
			& airAbsorptionExponentForVirtualSpaceSource);  
		      break;

                   case 'n': strcpy( reflection_order_impulse_response_decibels_presence_file, 
                                &crackTempString[1] ) ; break;

                   case 'o': listener_space_cross_reflections__include_0__exclude_1 = 
                                (int) crackfloat( &crackTempString[1], ch ) ; break;

                   case 'p': strcpy(tempstring, arg_option);
                             airAbsorptionExponentForRealSpaceSource.fp = 
		        crackstring( &crackTempString[1], 
			& airAbsorptionExponentForRealSpaceSource);  
		      break;
                   case 'q': orient_source__to_room_0__to_listener_1 = 
                                crackfloat( &crackTempString[1], ch ) ; break;

                   case 't': strcpy(tempstring, arg_option);
                             source_minimum_distance_from_listener.fp = 
		        crackstring( &crackTempString[1], 
			& source_minimum_distance_from_listener);  
		      break;
                   case 'u': use_collapsed_threshold_amplitudes__no_0__yes_1 = 
                                (int)  crackfloat( &crackTempString[1], ch ) ; break;

	// NEW 

                 case 'v': strcpy(tempstring, arg_option);
			xCoordinateFunction.fp = 
				crackstring( &crackTempString[1], &xCoordinateFunction); 
			break;
                  case 'w': strcpy(tempstring, arg_option);
			yCoordinateFunction.fp = 
				crackstring( &crackTempString[1], &yCoordinateFunction); 
			break;

                  case 'C': strcpy(tempstring, arg_option);
			sourceAngleFromOrigin.fp = 
				crackstring( &crackTempString[1], & sourceAngleFromOrigin ); 
			break;
                  case 'D': strcpy(tempstring, arg_option);
			sourceRadiusFromOrigin.fp = 
				crackstring( &crackTempString[1], &sourceRadiusFromOrigin ); 
			break;


                  case 'x': strcpy(tempstring, arg_option);
			sound_path_movement.fp = 
				crackstring( &crackTempString[1], &sound_path_movement ); 
			break;
                  case 'y': number_of_movement_function_points = 
                                (int)  crackfloat( &crackTempString[1], ch ) ; break;

                  case 'A': strcpy( outputDirectory, &crackTempString[1] ) ; break;

                  case 'B': strcpy( soundFormatTemplateFile, &crackTempString[1] ) ; break;


                   case 'E': strcpy( source_coordinates_plot_file, 
                                &crackTempString[1] ) ; break;


			case 'z': strcpy(tempstring, arg_option);
				high_order_release_duration.fp = 
					crackstring( &crackTempString[1], & high_order_release_duration );



                } ;
                break;


   case '0':	scale_to_pre_envelope_window_sample_peak_amps__off_0__on_1 = 
                    (int) crackfloat( arg_option, ch ) ; break;


                   case '~': strcpy(tempstring, arg_option);
                             low_order_attack_duration.fp = 
		        crackstring( tempstring, 
			& low_order_attack_duration);  
		      break;
                   case '=': strcpy(tempstring, arg_option);
                             low_order_release_duration.fp = 
		        crackstring( tempstring, 
			& low_order_release_duration);  
		      break;
                   case '@': strcpy(tempstring, arg_option);
                             high_order_attack_duration.fp = 
		        crackstring( tempstring, 
			& high_order_attack_duration);  
		      break;
                   case ':': strcpy(tempstring, arg_option);
                             envelope_shape_index.fp = 
		        crackstring( tempstring, 
			& envelope_shape_index);  
		      break;

    case 'Q':	reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 = 
                  (int) crackfloat( arg_option, ch ) ; break;

    case 'V':	strcpy( reflection_order_channel_assignments_file, arg_option) ; break;


    case '8':	reflection_order_impulse_responses__off_0__on_1 = (int) crackfloat( arg_option, ch ) ; break;

    case '9':	strcpy( reflectionOrderImpulseResponseInputSoundFileName, arg_option) ; break;


    case '7':	strcpy( wall_IR_response_envelope, arg_option) ; break; 

    case '6': strcpy(tempstring, arg_option);
                             wall_IR_truncate_duration.fp = 
		        crackstring( tempstring, 
			& wall_IR_truncate_duration);  
		      break;

    case '3': strcpy(tempstring, arg_option);
                             impulse_inclusion_threshold_in_dB.fp = 
		        crackstring( tempstring, 
			& impulse_inclusion_threshold_in_dB);  
		      break;
    case 'v': strcpy(tempstring, arg_option);
                             endCropReleaseTime.fp = 
		        crackstring( tempstring, 
			& endCropReleaseTime);  
		      break;

    case 'U': strcpy(tempstring, arg_option);
                             endCropDecibelThreshold.fp = 
		        crackstring( tempstring, 
			& endCropDecibelThreshold);  
		      break;

    case 'l': strcpy(tempstring, arg_option);
                             highOrderLimit.fp = 
		        crackstring( tempstring, 
			& highOrderLimit);  
		      break;
    case 'u': strcpy(tempstring, arg_option);
                             lowOrderLimit.fp = 
		        crackstring( tempstring, 
			& lowOrderLimit);  
		      break;

    case 'T': 
	wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 = 
                   (int) crackfloat( arg_option, ch ) ; break;

    case 'x': strcpy(tempstring, arg_option);
                             WIRBPF_low_rolloff_frequency.fp = 
		        crackstring( tempstring, 
			& WIRBPF_low_rolloff_frequency);  
		      break;
    case 'y': strcpy(tempstring, arg_option);
                             WIRBPF_high_rolloff_frequency.fp = 
		        crackstring( tempstring, 
			& WIRBPF_high_rolloff_frequency);  
		      break;
    case 'z': strcpy(tempstring, arg_option);
                             WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave.fp = 
		        crackstring( tempstring, 
			& WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave);  
		      break;
    case 'Z': strcpy(tempstring, arg_option);
                             WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave.fp = 
		        crackstring( tempstring, 
			& WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave);  
		      break;

    case 't': strcpy(tempstring, arg_option);
                             wall_impulse_and_gainscale_response_mode.fp = 
		        crackstring( tempstring, 
			& wall_impulse_and_gainscale_response_mode);  
		      break;

    case 'q': strcpy(tempstring, arg_option);
                             reflections_time_scaler.fp = 
		        crackstring( tempstring, 
			& reflections_time_scaler);  
		      break;


    case '5': 
	wall_impulse_responses__off_0__on_1 = (int) crackfloat( arg_option, ch ) ; break;

    case 'K':	strcpy( wallImpulseResponseInputSoundFileName, arg_option) ; break;

    case '1':	strcpy( wall_channel_assignments_file, arg_option) ; break;
    case '2':	strcpy( wall_dB_gainscale_factors_file, arg_option) ; break;


    case 'X': strcpy(tempstring, arg_option);
                             room_X_translation_factor.fp = 
		        crackstring( tempstring, 
			& room_X_translation_factor);  
		      break;
    case 'Y': strcpy(tempstring, arg_option);
                             room_Y_translation_factor.fp = 
		        crackstring( tempstring, 
			& room_Y_translation_factor);  
		      break;
    case 'b': strcpy(tempstring, arg_option);
                             room_negXscaleFactor.fp = 
		        crackstring( tempstring, 
			& room_negXscaleFactor);  
		      break;
    case 'B': strcpy(tempstring, arg_option);
                             room_posXscaleFactor.fp = 
		        crackstring( tempstring, 
			& room_posXscaleFactor);  
		      break;
    case 'e': strcpy(tempstring, arg_option);
                             room_negYscaleFactor.fp = 
		        crackstring( tempstring, 
			& room_negYscaleFactor);  
		      break;
    case 'E': strcpy(tempstring, arg_option);
                             room_posYscaleFactor.fp = 
		        crackstring( tempstring, 
			& room_posYscaleFactor);  
		      break;
    case 'A': strcpy(tempstring, arg_option);
                             room_RotationInDegrees.fp = 
		        crackstring( tempstring, 
			& room_RotationInDegrees);  
		      break;
    case 'M': strcpy(tempstring, arg_option);
                             room_ScaleFactor.fp = 
		        crackstring( tempstring, 
			& room_ScaleFactor);  
		      break;

    case 'F': strcpy(tempstring, arg_option);
                             speaker_X_translation_factor.fp = 
		        crackstring( tempstring, 
			& speaker_X_translation_factor);  
		      break;
    case 'g': strcpy(tempstring, arg_option);
                             speaker_Y_translation_factor.fp = 
		        crackstring( tempstring, 
			& speaker_Y_translation_factor);  
		      break;
    case 'h': strcpy(tempstring, arg_option);
                             speaker_negXscaleFactor.fp = 
		        crackstring( tempstring, 
			& speaker_negXscaleFactor);  
		      break;
    case 'H': strcpy(tempstring, arg_option);
                             speaker_posXscaleFactor.fp = 
		        crackstring( tempstring, 
			& speaker_posXscaleFactor);  
		      break;
    case 'I': strcpy(tempstring, arg_option);
                             speaker_negYscaleFactor.fp = 
		        crackstring( tempstring, 
			& speaker_negYscaleFactor);  
		      break;
    case 'j': strcpy(tempstring, arg_option);
                             speaker_posYscaleFactor.fp = 
		        crackstring( tempstring, 
			& speaker_posYscaleFactor);  
		      break;
    case 'J': strcpy(tempstring, arg_option);
                             speaker_RotationInDegrees.fp = 
		        crackstring( tempstring, 
			& speaker_RotationInDegrees);  
		      break;
   case 'k': strcpy(tempstring, arg_option);
                             speaker_ScaleFactor.fp = 
		        crackstring( tempstring, 
			& speaker_ScaleFactor);  
		      break;

    case 'S':	speaker_configuration__sequence_0__polygon_1 = 
                   (int) crackfloat( arg_option, ch ) ; break;


    case 'O': strcpy(tempstring, arg_option);
                             sourceRotationInDegrees.fp = 
		        crackstring( tempstring, 
			& sourceRotationInDegrees);  
		      break;

    case 'P': strcpy(tempstring, arg_option);
                             sourceDispersionPatternRolloffInDecibels.fp = 
		        crackstring( tempstring, 
			& sourceDispersionPatternRolloffInDecibels);  
		      break;
    case 'm': strcpy(tempstring, arg_option);
                             minimumReferenceDistanceInFeet.fp = 
		        crackstring( tempstring, 
			& minimumReferenceDistanceInFeet);  
		      break;

    case 'N':    normalizeFlag = (int) crackfloat( arg_option, ch ) ;
			break;


    case 'G': strcpy(tempstring, arg_option);
                             masterGainInDecibels.fp = 
		        crackstring( tempstring, 
			& masterGainInDecibels);  
		      break;

    case 'a': strcpy(tempstring, arg_option);
                             airAbsorptionExponentForReflections.fp = 
		        crackstring( tempstring, 
			& airAbsorptionExponentForReflections);  
		      break;

    case 's':	output_sound__all_0__direct_1__reflections_2 = 
       (int) crackfloat( arg_option, ch ) ; break;

    case 'C':	outputChannelNumber = (int) crackfloat( arg_option, ch ) ; break;

    case 'f':	polygonCoordinatesSource = (int) crackfloat( arg_option, ch ) ; break;

    case 'c':   strcpy( polygonCoordinates_datafile, arg_option); break;

    case 'L':	strcpy( listenerCoordinatesDataFile, arg_option) ; break;

    case 'o':	strcpy( speakerCoordinatesDataFile, arg_option) ; break;

    case 'p':	strcpy( soundPathsPlotFileName, arg_option) ; break;

    case '4':	plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 = 
                           (int) crackfloat( arg_option, ch ) ; break;
    case 'w': strcpy(tempstring, arg_option);
                             numberOfWalls.fp = 
		        crackstring( tempstring, 
			& numberOfWalls);  
		      break;


    case 'r': strcpy(tempstring, arg_option);
                             rotationOfSyntheticRoomInDegrees.fp = 
		        crackstring( tempstring, 
			& rotationOfSyntheticRoomInDegrees);  
		      break;

    case 'd': strcpy(tempstring, arg_option);
                             minDistanceToCornerFromOrigin.fp = 
		        crackstring( tempstring, 
			& minDistanceToCornerFromOrigin);  
		      break;

    case 'D': strcpy(tempstring, arg_option);
                             maxDistanceToCornerFromOrigin.fp = 
		        crackstring( tempstring, 
			& maxDistanceToCornerFromOrigin);  
		      break;

    case 'R': strcpy(tempstring, arg_option);
                             polygonAngleRegularityProportion.fp = 
		        crackstring( tempstring, 
			& polygonAngleRegularityProportion);  
		      break;

    case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;


	} 
} ;



// MAKE DIRECTORY IF NECESSARY



fprintf(stderr, "outputDirectory: %s", outputDirectory ) ;

if (stat(outputDirectory, &st) == -1) {
    mkdir(outputDirectory, 0700);
} ;

sprintf( command, "rm %s/ir.*.au* %s/impulseFileNames", 
	outputDirectory, outputDirectory ) ; 
system( command ) ; 

sprintf( impulseFilesName, "%s/impulseFileNames", outputDirectory ) ; 
impulseFilesFilePointer  = fopen( impulseFilesName, "w" ) ; 
fprintf( impulseFilesFilePointer, "%i\n", number_of_movement_function_points ) ;  

PI = 4.*atan(1.) ;

plotData = fopen( source_coordinates_plot_file, "w" ) ; 

// MAKE COMMAND
  

   for(i = 0; i < number_of_movement_function_points; i++)
   {
      if( number_of_movement_function_points == 1 ) prop = 0. ; 
      else prop = (float) i / (float) (number_of_movement_function_points - 1) ;

      sprintf( thisCommand,"%s ", "roomresponsemaker " );

      // -/a
      sprintf( thisCommand,"%s-/a%s ", 
	thisCommand, reflection_order_dB_gainscale_factors_file ) ; 

      // -/b
      reflection_order_IR_convolution_mode.A[ 0 ] = 
	fval( &reflection_order_IR_convolution_mode, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/b%i ", 
	thisCommand, (int) (reflection_order_IR_convolution_mode.A[ 0 ] + 0.5) ) ; 

      // -/c
      sprintf( thisCommand,"%s-/c%i ", 
      thisCommand, 
       RO_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 ) ; 

      // -/d
      RO_IR_BPF_low_rolloff_frequency.A[ 0 ] = 
	fval( & RO_IR_BPF_low_rolloff_frequency, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/d%f ", 
	thisCommand, (RO_IR_BPF_low_rolloff_frequency.A[ 0 ]) ) ; 
      // -/e
      RO_IR_BPF_high_rolloff_frequency.A[ 0 ] = 
	fval( & RO_IR_BPF_high_rolloff_frequency, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/e%f ", 
	thisCommand, (RO_IR_BPF_high_rolloff_frequency.A[ 0 ]) ) ; 

      // -/f
      RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 
	fval( & RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/f%f ", 
	thisCommand, (RO_IR_BPF_low_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ]) ) ; 

      // -/g
      RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 
	fval( & RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/g%f ", 
	thisCommand, (RO_IR_BPF_high_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ]) ) ; 

      // -/h
      reflection_order_IR_truncate_duration.A[ 0 ] = 
	fval( & reflection_order_IR_truncate_duration, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/h%f ", 
	thisCommand, (reflection_order_IR_truncate_duration.A[ 0 ]) ) ; 
      // -/i
      sprintf( thisCommand,"%s-/i%s ", 
	thisCommand, reflection_order_IR_envelope ) ; 
      // -/j
      sprintf( thisCommand,"%s-/j%s ", 
	thisCommand, wall_impulse_response_decibels_presence_file ) ; 

      // -/k
      reflected_sound_gain_in_decibels.A[ 0 ] = 
	fval( & reflected_sound_gain_in_decibels, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/k%f ", 
	thisCommand, (reflected_sound_gain_in_decibels.A[ 0 ]) ) ; 
      // -/l
      direct_sound_gain_in_decibels.A[ 0 ] = 
	fval( & direct_sound_gain_in_decibels, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/l%f ", 
	thisCommand, (direct_sound_gain_in_decibels.A[ 0 ]) ) ; 
      // -/m
      airAbsorptionExponentForVirtualSpaceSource.A[ 0 ] = 
	fval( & airAbsorptionExponentForVirtualSpaceSource, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/m%f ", 
	thisCommand, (airAbsorptionExponentForVirtualSpaceSource.A[ 0 ]) ) ; 
      // -/n
      sprintf( thisCommand,"%s-/n%s ", 
	thisCommand, reflection_order_impulse_response_decibels_presence_file ) ; 
      // -/o
      sprintf( thisCommand,"%s-/o%i ", 
	thisCommand, listener_space_cross_reflections__include_0__exclude_1 ) ; 
      // -/p
      airAbsorptionExponentForRealSpaceSource.A[ 0 ] = 
	fval( & airAbsorptionExponentForRealSpaceSource, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/p%f ", 
	thisCommand, (airAbsorptionExponentForRealSpaceSource.A[ 0 ]) ) ; 
      // -/q
      sprintf( thisCommand,"%s-/q%i ", 
	thisCommand, orient_source__to_room_0__to_listener_1 ) ; 
      // -/t
      source_minimum_distance_from_listener.A[ 0 ] = 
	fval( & source_minimum_distance_from_listener, 1.0, prop ) ;
      sprintf( thisCommand,"%s-/t%f ", 
	thisCommand, (source_minimum_distance_from_listener.A[ 0 ]) ) ; 
      // -/u
      sprintf( thisCommand,"%s-/u%i ", 
	thisCommand, use_collapsed_threshold_amplitudes__no_0__yes_1 ) ; 



      // -0
      sprintf( thisCommand,"%s-0%i ", 
	thisCommand, scale_to_pre_envelope_window_sample_peak_amps__off_0__on_1 ) ; 

      // -~
      low_order_attack_duration.A[ 0 ] = 
	fval( &low_order_attack_duration, 1.0, prop ) ;
      sprintf( thisCommand,"%s-~%f ", 
	thisCommand, (low_order_attack_duration.A[ 0 ]) ) ; 

      // -=
      low_order_release_duration.A[ 0 ] = 
	fval( & low_order_release_duration, 1.0, prop ) ;
      sprintf( thisCommand,"%s-=%f ", 
	thisCommand, (low_order_release_duration.A[ 0 ]) ) ; 

      // -@
      high_order_attack_duration.A[ 0 ] = 
	fval( & high_order_attack_duration, 1.0, prop ) ;
      sprintf( thisCommand,"%s-@%f ", 
	thisCommand, (high_order_attack_duration.A[ 0 ]) ) ; 

      // -!
      high_order_release_duration.A[ 0 ] = 
	fval( & high_order_release_duration, 1.0, prop ) ;
      sprintf( thisCommand,"%s-!%f ", 
	thisCommand, (high_order_release_duration.A[ 0 ]) ) ; 

      // -:
      envelope_shape_index.A[ 0 ] = 
	fval( & envelope_shape_index, 1.0, prop ) ;
      sprintf( thisCommand,"%s-:%f ", 
	thisCommand, (envelope_shape_index.A[ 0 ]) ) ; 

      // -Q
      sprintf( thisCommand,"%s-Q%i ", 
	thisCommand, reflection_order_IR_window_sample_mode__off_0__on_1__on_with_sync_2 ) ; 
      // -V
      sprintf( thisCommand,"%s-V%s ", 
	thisCommand, reflection_order_channel_assignments_file ) ; 
      // -8
      sprintf( thisCommand,"%s-8%i ", 
	thisCommand, reflection_order_impulse_responses__off_0__on_1 ) ; 
      // -9
      sprintf( thisCommand,"%s-9%s ", 
	thisCommand, reflectionOrderImpulseResponseInputSoundFileName ) ; 


      // -7
      sprintf( thisCommand,"%s-7%s ", 
	thisCommand, wall_IR_response_envelope ) ; 


      // -6 
      wall_IR_truncate_duration.A[ 0 ] = 
	fval( & wall_IR_truncate_duration, 1.0, prop ) ;
      sprintf( thisCommand,"%s-6%f ", 
	thisCommand, (wall_IR_truncate_duration.A[ 0 ]) ) ; 


      // -3
      impulse_inclusion_threshold_in_dB.A[ 0 ] = 
	fval( & impulse_inclusion_threshold_in_dB, 1.0, prop ) ;
      sprintf( thisCommand,"%s-3%f ", 
	thisCommand, (impulse_inclusion_threshold_in_dB.A[ 0 ]) ) ; 

      // -v
      endCropReleaseTime.A[ 0 ] = 
	fval( & endCropReleaseTime, 1.0, prop ) ;
      sprintf( thisCommand,"%s-v%f ", 
	thisCommand, (endCropReleaseTime.A[ 0 ]) ) ; 

      // -U
      endCropDecibelThreshold.A[ 0 ] = 
	fval( & endCropDecibelThreshold, 1.0, prop ) ;
      sprintf( thisCommand,"%s-U%f ", 
	thisCommand, (endCropDecibelThreshold.A[ 0 ]) ) ; 

      // -l
      highOrderLimit.A[ 0 ] = 
	fval( & highOrderLimit, 1.0, prop ) ;
      sprintf( thisCommand,"%s-l%i ", 
	thisCommand, (int)(highOrderLimit.A[ 0 ] + 0.5) ) ; 
      // -u
      lowOrderLimit.A[ 0 ] = 
	fval( & lowOrderLimit, 1.0, prop ) ;
      sprintf( thisCommand,"%s-u%i ", 
	thisCommand, (int)(lowOrderLimit.A[ 0 ] + 0.5) ) ; 
      // -T
      sprintf( thisCommand,"%s-T%i ", thisCommand,
	 wall_IR_BPF_and_normalize__off_0__input_1__compounded_convolution_outputs_2__both_3 ) ; 
      // -x
      WIRBPF_low_rolloff_frequency.A[ 0 ] = 
	fval( & WIRBPF_low_rolloff_frequency, 1.0, prop ) ;
      sprintf( thisCommand,"%s-x%f ", 
	thisCommand, (WIRBPF_low_rolloff_frequency.A[ 0 ]) ) ; 
      // -y
      WIRBPF_high_rolloff_frequency.A[ 0 ] = 
	fval( & WIRBPF_high_rolloff_frequency, 1.0, prop ) ;
      sprintf( thisCommand,"%s-y%f ", 
	thisCommand, (WIRBPF_high_rolloff_frequency.A[ 0 ]) ) ; 
      // -z
      WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 
	fval( & WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave, 1.0, prop ) ;
      sprintf( thisCommand,"%s-z%f ", 
	thisCommand, (WIRBPF_low_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ]) ) ; 
      // -Z
      WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ] = 
	fval( & WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave, 1.0, prop ) ;
      sprintf( thisCommand,"%s-Z%f ", 
	thisCommand, (WIRBPF_high_edge_amplitude_rolloff_in_dB_per_octave.A[ 0 ]) ) ; 

      // -t
      wall_impulse_and_gainscale_response_mode.A[ 0 ] = 
	fval( & wall_impulse_and_gainscale_response_mode, 1.0, prop ) ;
      sprintf( thisCommand,"%s-t%i ", 
	thisCommand, (int)(wall_impulse_and_gainscale_response_mode.A[ 0 ] + 0.5) ) ; 



      // -q
      reflections_time_scaler.A[ 0 ] = 
	fval( & reflections_time_scaler, 1.0, prop ) ;
      sprintf( thisCommand,"%s-q%f ", 
	thisCommand, (reflections_time_scaler.A[ 0 ]) ) ; 


      // -5
      sprintf( thisCommand,"%s-5%i ", 
	thisCommand, wall_impulse_responses__off_0__on_1 ) ; 


      // -K
      sprintf( thisCommand,"%s-K%s ", 
	thisCommand, wallImpulseResponseInputSoundFileName ) ; 

      // -1
      sprintf( thisCommand,"%s-1%s ", 
	thisCommand, wall_channel_assignments_file ) ; 
      // -2
      sprintf( thisCommand,"%s-2%s ", 
	thisCommand, wall_dB_gainscale_factors_file ) ; 


      // -X
      room_X_translation_factor.A[ 0 ] = 
	fval( & room_X_translation_factor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-X%f ", 
	thisCommand, (room_X_translation_factor.A[ 0 ]) ) ; 
      // -Y
      room_Y_translation_factor.A[ 0 ] = 
	fval( & room_Y_translation_factor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-Y%f ", 
	thisCommand, (room_Y_translation_factor.A[ 0 ]) ) ; 
      // -b
      room_negXscaleFactor.A[ 0 ] = 
	fval( & room_negXscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-b%f ", 
	thisCommand, (room_negXscaleFactor.A[ 0 ]) ) ; 
      // -B
      room_posXscaleFactor.A[ 0 ] = 
	fval( & room_posXscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-B%f ", 
	thisCommand, (room_posXscaleFactor.A[ 0 ]) ) ; 
      // -e
      room_negYscaleFactor.A[ 0 ] = 
	fval( & room_negYscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-e%f ", 
	thisCommand, (room_negYscaleFactor.A[ 0 ]) ) ; 
      // -E
      room_posYscaleFactor.A[ 0 ] = 
	fval( & room_posYscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-E%f ", 
	thisCommand, (room_posYscaleFactor.A[ 0 ]) ) ; 
      // -A
      room_RotationInDegrees.A[ 0 ] = 
	fval( & room_RotationInDegrees, 1.0, prop ) ;
      sprintf( thisCommand,"%s-A%f ", 
	thisCommand, (room_RotationInDegrees.A[ 0 ]) ) ; 
      // -M
      room_ScaleFactor.A[ 0 ] = 
	fval( & room_ScaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-M%f ", 
	thisCommand, (room_ScaleFactor.A[ 0 ]) ) ; 


      // -F
      speaker_X_translation_factor.A[ 0 ] = 
	fval( & speaker_X_translation_factor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-F%f ", 
	thisCommand, (speaker_X_translation_factor.A[ 0 ]) ) ; 
      // -g
      speaker_Y_translation_factor.A[ 0 ] = 
	fval( & speaker_Y_translation_factor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-g%f ", 
	thisCommand, (speaker_Y_translation_factor.A[ 0 ]) ) ; 
      // -h
      speaker_negXscaleFactor.A[ 0 ] = 
	fval( & speaker_negXscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-h%f ", 
	thisCommand, (speaker_negXscaleFactor.A[ 0 ]) ) ; 
      // -H
      speaker_posXscaleFactor.A[ 0 ] = 
	fval( & speaker_posXscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-H%f ", 
	thisCommand, (speaker_posXscaleFactor.A[ 0 ]) ) ; 
      // -I
      speaker_negYscaleFactor.A[ 0 ] = 
	fval( & speaker_negYscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-I%f ", 
	thisCommand, (speaker_negYscaleFactor.A[ 0 ]) ) ; 
      // -j
      speaker_posYscaleFactor.A[ 0 ] = 
	fval( & speaker_posYscaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-j%f ", 
	thisCommand, (speaker_posYscaleFactor.A[ 0 ]) ) ; 
      // -J
      speaker_RotationInDegrees.A[ 0 ] = 
	fval( & speaker_RotationInDegrees, 1.0, prop ) ;
      sprintf( thisCommand,"%s-J%f ", 
	thisCommand, (speaker_RotationInDegrees.A[ 0 ]) ) ; 
      // -k
      speaker_ScaleFactor.A[ 0 ] = 
	fval( & speaker_ScaleFactor, 1.0, prop ) ;
      sprintf( thisCommand,"%s-k%f ", 
	thisCommand, (speaker_ScaleFactor.A[ 0 ]) ) ; 


      // -S
      sprintf( thisCommand,"%s-S%i ", 
	thisCommand, speaker_configuration__sequence_0__polygon_1 ) ; 


      // -O
      sourceRotationInDegrees.A[ 0 ] = 
	fval( & sourceRotationInDegrees, 1.0, prop ) ;
      sprintf( thisCommand,"%s-O%f ", 
	thisCommand, (sourceRotationInDegrees.A[ 0 ]) ) ; 

      // -P
      sourceDispersionPatternRolloffInDecibels.A[ 0 ] = 
	fval( & sourceDispersionPatternRolloffInDecibels, 1.0, prop ) ;
      sprintf( thisCommand,"%s-P%f ", 
	thisCommand, (sourceDispersionPatternRolloffInDecibels.A[ 0 ]) ) ; 
      // -m
      minimumReferenceDistanceInFeet.A[ 0 ] = 
	fval( & minimumReferenceDistanceInFeet, 1.0, prop ) ;
      sprintf( thisCommand,"%s-m%f ", 
	thisCommand, (minimumReferenceDistanceInFeet.A[ 0 ]) ) ; 

      // -N
      sprintf( thisCommand,"%s-N%i ", 
	thisCommand, normalizeFlag ) ; 


      // -G
      masterGainInDecibels.A[ 0 ] = 
	fval( & masterGainInDecibels, 1.0, prop ) ;
      sprintf( thisCommand,"%s-G%f ", 
	thisCommand, (masterGainInDecibels.A[ 0 ]) ) ; 

      // -a
      airAbsorptionExponentForReflections.A[ 0 ] = 
	fval( & airAbsorptionExponentForReflections, 1.0, prop ) ;
      sprintf( thisCommand,"%s-a%f ", 
	thisCommand, (airAbsorptionExponentForReflections.A[ 0 ]) ) ; 

      // -s
      sprintf( thisCommand,"%s-s%i ", 
	thisCommand, output_sound__all_0__direct_1__reflections_2 ) ; 
      // -C
      sprintf( thisCommand,"%s-C%i ", 
	thisCommand, outputChannelNumber ) ; 
      // -f
      sprintf( thisCommand,"%s-f%i ", 
	thisCommand, polygonCoordinatesSource ) ; 
      // -c
      sprintf( thisCommand,"%s-c%s ", 
	thisCommand, polygonCoordinates_datafile ) ; 
      // -L
      sprintf( thisCommand,"%s-L%s ", 
	thisCommand, listenerCoordinatesDataFile ) ; 
      // -o
      sprintf( thisCommand,"%s-o%s ", 
	thisCommand, speakerCoordinatesDataFile ) ; 
      // -p
      sprintf( thisCommand,"%s-p%s ", 
	thisCommand, soundPathsPlotFileName ) ; 
      // -4
      sprintf( thisCommand,"%s-4%i ", 
	thisCommand, plot_mode__one_file_0__channel_files_1__channel_and_order_files_2 ) ; 
      // -w
      numberOfWalls.A[ 0 ] = 
	fval( & numberOfWalls, 1.0, prop ) ;
      sprintf( thisCommand,"%s-w%i ", 
	thisCommand, (int)(numberOfWalls.A[ 0 ] + (0.5)) ) ; 


      // -r
      rotationOfSyntheticRoomInDegrees.A[ 0 ] = 
	fval( & rotationOfSyntheticRoomInDegrees, 1.0, prop ) ;
      sprintf( thisCommand,"%s-r%f ", 
	thisCommand, (rotationOfSyntheticRoomInDegrees.A[ 0 ]) ) ; 

      // -d
      minDistanceToCornerFromOrigin.A[ 0 ] = 
	fval( & minDistanceToCornerFromOrigin, 1.0, prop ) ;
      sprintf( thisCommand,"%s-d%f ", 
	thisCommand, (minDistanceToCornerFromOrigin.A[ 0 ]) ) ; 

      // -D
      maxDistanceToCornerFromOrigin.A[ 0 ] = 
	fval( & maxDistanceToCornerFromOrigin, 1.0, prop ) ;
      sprintf( thisCommand,"%s-D%f ", 
	thisCommand, (maxDistanceToCornerFromOrigin.A[ 0 ]) ) ; 

      // -R
      polygonAngleRegularityProportion.A[ 0 ] = 
	fval( & polygonAngleRegularityProportion, 1.0, prop ) ;
      sprintf( thisCommand,"%s-R%f ", 
	thisCommand, (polygonAngleRegularityProportion.A[ 0 ]) ) ; 

      // -i
      sprintf( thisCommand,"%s-i/tmp/sourceCoordinates ", 
	thisCommand ) ;

      // -_
      sprintf( thisCommand,"%s-_%i ", 
	thisCommand, autoplayreps ) ; 




      soundPathPositionProportion = sound_path_movement.A[ 0 ] = 
	fval( &sound_path_movement, 1.0, prop );
      while( soundPathPositionProportion > 1. ) 
         soundPathPositionProportion = soundPathPositionProportion - 1. ; 
      while( soundPathPositionProportion < 0. ) 
         soundPathPositionProportion = soundPathPositionProportion + 1. ; 

      xCoordinateFunction.A[ 0 ] = 
	fval( &xCoordinateFunction, 1.0, soundPathPositionProportion ); 
      yCoordinateFunction.A[ 0 ] = 
	fval( &yCoordinateFunction, 1.0, soundPathPositionProportion ); 

      sourceAngleFromOrigin.A[ 0 ] = 
	fval( & sourceAngleFromOrigin, 1.0, soundPathPositionProportion ); 
prf( sourceAngleFromOrigin.A[ 0 ], "sourceAngleFromOrigin.A[ 0 ]" ) ; 
      angleInRadians = ( (double) sourceAngleFromOrigin.A[ 0 ] * PI ) / 180. ;
prf( angleInRadians, "angleInRadians" ) ; 
      sourceRadiusFromOrigin.A[ 0 ] = 
	fval( &sourceRadiusFromOrigin, 1.0, soundPathPositionProportion ); 
prf( sourceRadiusFromOrigin.A[ 0 ], "sourceRadiusFromOrigin.A[ 0 ]" ) ; 


      sourceX = xCoordinateFunction.A[ 0 ] + 
	(sourceRadiusFromOrigin.A[ 0 ] * cos( angleInRadians ) ) ;
      sourceY = yCoordinateFunction.A[ 0 ] + 
	(sourceRadiusFromOrigin.A[ 0 ] * sin( angleInRadians ) ) ;



      fprintf( stderr, "\n sourceX: %f\tsourceY: %f", sourceX, sourceY ) ;

      data = fopen( "/tmp/sourceCoordinates", "w" );
      fprintf( data, "%f %f\n", sourceX, sourceY ) ; 
      fclose( data ) ; 

      fprintf( plotData, "%f %f\n", sourceX, sourceY ) ; 
      
      sprintf( thisImpulseFileName, "%s/ir.", outputDirectory ) ;  
      n = (int) log10( (float) number_of_movement_function_points ) ; 
      m = i == 0 ? 0 : (int) log10( (float) i ) ; 
      for( k = 0; k < (n - m); k++ ) sprintf( thisImpulseFileName, 
          "%s0", thisImpulseFileName ) ; 
      sprintf( thisImpulseFileName, "%s%d.au", thisImpulseFileName, i ) ;  

      fprintf( stderr, "\n\nthisImpulseFileName: %s", thisImpulseFileName ) ;
      fprintf( impulseFilesFilePointer, "%s\n", thisImpulseFileName ) ;  

      sprintf( tempstring, "cp %s %s", soundFormatTemplateFile, thisImpulseFileName ) ; 
      system( tempstring ) ; 

      sprintf( thisCommand, "%s%s", thisCommand, thisImpulseFileName ) ; 
      fprintf( stderr, "\n\nthisCommand: %s", thisCommand ) ; 

      system( thisCommand ) ; 

      fprintf( stderr, "\n\nDONE\n" ) ; 

   } ;

   fclose( impulseFilesFilePointer ) ; 

   fclose( plotData ) ; 









exit(EXIT_SUCCESS) ;




}


void usage()
{
	fprintf(stderr, "%s",
	"roomresponsesequencer:    \n"
	"roomresponsesequencer   [flags]\n"
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
	"	w:  number of walls (func) [4]\n"
	"	r:  synthesized room rotation in degrees (func) [0.]\n"
	"	    ** Distance to Room Corners from Origin:\n"
	"	d:  minimum distance in feet (func) [30]\n"
	"	D:  maximum distance  in feet (func) [30]\n"
	"	R:  wall origin-to-corners angle uniformity proportion\n"
	"	    0. (maximum variability) to 1. (maximum uniformity) (func) [1.]\n"
	"	    *****************************\n"
	"	    ** Room Modification Parameters:\n"
	"	X:  room X translation factor (func) [0]\n"
	"	Y:  room Y translation factor (func) [0]\n"
	"	b:  room negative X scale factor (func) [1]\n"
	"	B:  room positive X scale factor (func) [1]\n"
	"	e:  room negative Y scale factor (func) [1]\n"
	"	E:  room positive Y scale factor (func) [1]\n"
	"	A:  room rotation In degrees (func) [0]\n"
	"	M:  room scale factor (func) [1]\n"


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
	"	    Cartesian source sound path coordinates:\n"
	"	/v:  X cordinate (func) [0]\n"
	"	/w:  Y cordinate (func) [0]\n"
	"	/x:  0-1 normalized/windowed source sound path position (func) [0[\n" 
	"	/y:  number of transition points\n"
	"	/A:  output directory\n"
	"	/B:  impulse response sound format template sound file\n"
	"	o: speaker coordinates file\n"
	"	     (Multiple speaker pairs are applied in sequence to successive channels,\n"
	"	     looping as needed. Standard use is to apply single listener and source\n"
	"	     positions to a series of speakers. Coordinates are expressed in feet.)\n"
	"	S:  Speaker configuration: \n"
	"	       0 = sequence (i.e. non-enclosing array), 1 = polygon (surround) [0]\n"
	"	    ** Speaker Position Modification Parameters:\n"




	"	F:  speaker coordinates X translation factor (func) [0]\n"
	"	g:  speaker coordinates Y translation factor (func) [0]\n"
	"	h:  speaker coordinates negative X scale factor (func) [1]\n"
	"	H:  speaker coordinates positive X scale factor (func) [1]\n"
	"	I:  speaker coordinates negative Y scale factor (func) [1]\n"
	"	j:  speaker coordinates positive Y scale factor (func) [1]\n"
	"	J:  speaker coordinates rotation In degrees (func) [0]\n"
	"	k:  speaker coordinates scale factor (func) [1]\n"

	"	/C: source_position_angle (func) \n"	"	/D: source_position_radius (func)\n"
	"	/E: source_coordinates_plot_file \n"

	"	s:  output mix components -- 0 = both direct and reflected sound\n"
	"		1 = only direct sound, 2 = only reflected sound [0]\n"

	"	/k:  reflected sound gain in decibels (func) [0]\n"
	"	/l:  direct sound gain in decibels (func) [0]\n"

	"	/o:  listener space cross reflections: 0 = include, 1 = exclude [0]\n"



	"	/m:  direct sound air-absorption exponent: \n"
	"  	       amplitude of direct sound = (reference distance / source distance)**exponent\n"
	"	       Positive/negative values produce distance-correlated decrease/increase\n"
	"	       in amplitude, respectively, with severity of decrease or increase \n"
	"	       increasing with greater magnitudes. Exponent of 2 corresponds to \n"
	"	       inverse square law; value of 0 produces all amplitudes to be equal.\n"
	"	       Values between 1 and 2 generally work best. (func) [2.]\n"
	"	/p: air absorption exponent for real space source (func) \n"	"	a:  reflections air-absorption exponent: \n"
	"  	       amplitude of reflection = (reference distance / reflection distance)**exponent\n"
	"	       Positive/negative values produce distance-correlated decrease/increase\n"
	"	       in amplitude, respectively, with severity of decrease or increase \n"
	"	       increasing with greater magnitudes. Exponent of 2 corresponds to \n"
	"	       inverse square law; value of 0 produces all amplitudes to be equal.\n"
	"	       Values between 1 and 2 generally work best. (func) [2.]\n"


	"	m:  reference distance for air absorption computation\n"
	"	       Distance, in feet, at which source or reflection will produce amplitude\n"
	"	        of 0dB. Amplitudes for shorter distances are limited to the\n"
	"	       reference distance amplitude, i.e. to 0dB. (func) [1]\n" 

	"	/t: source minimum distance from listener (func) \n"	"	/u: use collapsed threshold amplitudes  0 = no, 1 = yes \n"


	"	l:  reflection order high limit (greatest number of wall reflections)\n"
	"	       The high reflection order determines the highest number of computed \n"
	"	       reflection surfaces included in response. (func) \n"
	"	u:  reflection order low limit (least number of wall reflections)\n"
	"	       The low reflection order determines the least number of computed \n"
	"	       reflection surfaces included in response. (func) \n"

	"	P: source dispersion pattern rolloff. 0dB to -90dB or less.\n"
	"	       0 = off (i.e. omnidirectional) \n"
	"	       Given the directional or forward orientation of source, rolloff determines \n"
	"	       amplitude of off-axis, non-forward radiations. Rear radiations \n"
	"	       (i.e. deviations of +/- 180 degrees) are attenuated by full rolloff;\n"
	"              lesser angles by corresponding proportion of rolloff. Rolloff is applied\n"
	"	       to both direct (i.e. source-to-speaker) and reflected (i.e. source-to-wall) \n"
	"	       radiations. (func) [0]\n"   
	"	/q: forward orientation of non-rotated source:\n" 
	"		0 = orient to 0 degrees of polar room coordinates (i.e. source as origin)\n"
	"		1 = orient to listener (facing listener, i.e. listener as origin) [0]\n"	
	"	O: rotation of source, in degrees. (func) [0]\n"
	"	       0 = no rotation, +/- 180 degrees = reversed orientation.\n"
	"	       Sources are rotated relative to the orientation specified in -/q.\n"
	"	       Source rotation requires directional (i.e. non-zero) dispersion rolloff.\n"
	"	       (See -P above.)\n"
	"	       ** Source and Impulse Time Position Modification Parameters:\n"
	"	q:     reflection impulses time position scaler: 1 = no change (func) [1]\n"
//

 


	"	  ** REFLECTION ORDER IMPULSE RESPONSES **\n"
	"	8:     reflection order impulse responses switch: 0 = off, 1 = on [0]\n"

	"	9:     reflection order impulse response sound file name\n"
	"	          Channels of audio sound file are used as the impulse responses for\n"
	"	          reflection order. Application of impulse responses are handled\n"
	"	          according to the -? parameter. If no file is given, then single impulses,\n"
	"	          are used, modified in amplitude in accord with  their distance, \n"
	"                 orientation to the source, and reflection order response filter settings.\n"  
	"	/b:  reflection order impulse response convolution mode (func) [-1]\n"
	"	V:  reflection order channel assignments file (1-?)\n"
	"	/a:  reflection order gainscale factors file (in dB)\n"
	"	/n: reflection order impulse response presence levels file (in dB)\n"

	"	/i:   reflection order impulse response truncation envelope file\n"
	"	/h:   reflection order impulse response truncate duration (func) \n"
	"	  ** REFLECTION ORDER IMPULSE RESPONSE FILTER **\n"
	"	/c:   Filter Mode: 0 = off, 1 = pre-filter input response, \n"
	"	        2 = compounded filtering of convolution outputs, 3 = both modes 1 and 2.\n"
	"	/d:   reflection order impulse response filter low rolloff frequency (func) \n"
	"	/e:   reflection order impulse response filter high rolloff frequency (func) \n"
	"	/f:   reflection order impulse response filter low amplitude rolloff per octave in dB (func) \n"
	"	/g:   reflection order impulse response filter high amplitude rolloff per octave in dB (func) \n"
	"	   ** Reflection Order Impulse Response Window Sample Mode **\n"
	"	Q:   reflection order impulse response window sample mode: \n"
        "               0 = off, 1 = on, 2 = on with sync [0]\n"
 	"	~: low order attack duration (func) \n"	 
 	"	=: low order release duration (func) \n"	 
 	"	@: high order attack duration (func) \n"	 
 	"	/z: high order release duration (func) \n"	 
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
	"	t:  wall impulse response convolution mode (func) [-1]\n"
	"	1:  wall channel assignments file (1-?)\n"
	"	2:  wall gainscale factors file (in dB)\n"
	"	/j: wall impulse response presence levels file (in dB)\n"

	"	7:   wall impulse response truncation envelope file\n"
	"	6:   wall impulse response truncate duration (func) \n"
	"	  ** WALL IMPULSE RESPONSE FILTER **\n"
	"	T:   Filter Mode: 0 = off, 1 = pre-filter input response, \n"
	"	        2 = compounded filtering of convolution outputs, 3 = both modes 1 and 2.\n"
	"	x:   wall impulse response filter low rolloff frequency (func) \n"
	"	y:   wall impulse response filter high rolloff frequency (func) \n"
	"	z:   wall impulse response filter low amplitude rolloff per octave in dB (func) \n"
	"	Z:   wall impulse response filter high amplitude rolloff per octave in dB (func) \n"

	"	3: impulse inclusion threshold in dB (func) \n"	"	U: impulse end truncation threshold in dB (func) \n"	"	v: impulse end truncation release time in seconds (func) \n"


	"	C: room impulse response output channel [0]\n"
	"	       0 = Maximum number set by larger number of source and speaker positions.\n"
	"	       1 or greater outputs a specific channel.)\n" 
	"	G: master gain in decibels (pre-normalization) (func) [0]\n"
	"	N: normalization:                [1]\n"
	"	   0: Do not normalize. \n"
	"	   1: Normalize channels independently.\n"
	"	   2: Normalize channels together against channel with peak amplitude.\n"
	"	   3: Normalize channels together if any channel exceeds 0 decibels.\n"


	"	_:	 "AUTO_PLAY		// autoplayreps



	);
	exit(EXIT_SUCCESS);
}




