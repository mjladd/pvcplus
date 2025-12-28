#include "globals.h"

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j, k, l,  m, m1, m2,  nnn=0,    numbins,   stopbin=10,  offset ;
float nyquist,  fundamental ;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 220, I = 220, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  write_ascii=0 ;
float P = 1.0;
FILE *fopen(),  *write_ascii_d,  *tdata, *adata ;
char ch,  tempstring[ STRING_SIZE ],  write_ascii_filename[ STRING_SIZE ]="./ascii.out", 
    scratch[ STRING_SIZE ],  scratch2[ STRING_SIZE ],  *user ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, 
    *buffer, *previous_buffer, *channel, *output ;
float threshfac = .001,  threshfacdB=-96.;
float  *binfreq,  dur ;
float  gain, f ;
float  *previous_channel, *channel_freqdev;
float  temp, temp1,  temp2,  pm,  IR  ;  
float getthresh();
float phasediff ; 
int notFound, residueBinsFlag=0 ; 

int extendTargetFormants=0, bin,
	AddOctavesToTargetExtensionPartials=0, Source_Stopband_Type=0, 
		bank_A_0__banks_A_and_B_1=0 ; 

float thisOctave, added_formants_partial_dB_rolloff_per_partial=0., amplitude_normalization_decibel_gain_limit=200 ; 

float thisSourceFormantDuplicateDBScaler, thisTargetFormantDuplicateDBScaler ; 

float ampInterpControl, freqInterpControl ;  

int thisPartialNumber, transferCode, smoothingFlag, thisCenterBin ; 

float formantsGain, interpControl, formantOverlapDBAdjustSource, formantOverlapDBAdjustTarget,
		formantOverlapAmpAdjust, thisCenterBinFreq ; 

float highStopBandFreqOfLower, lowStopBandFreqOfLower ; 

int extendSourceFormants=0 ; 
float SourceFormantExtensionDbThreshold=-96 ; 
float peakSourceExtendPartial=0 ; 
int AddOctavesToSourceExtensionPartials=0 ; 


float proposedPartialFreq, proposedPartialAmp, partialNumber, peakTargetExtendPartial=0., TargetFormantExtensionDbThreshold=-96, 
	Target_formants_decibel_threshold=-96. ; 

char sourceFormantsFile[ STRING_SIZE ]="", targetFormantsFile[ STRING_SIZE ]="" ; 

float *sourceFormantCenterFreqs, *sourceFormantAmps, *sourceFormantBWs, 
		*sourceFormantQs; 
int sourceNumFormants, sourceN2, *sourceFormantIndices,
	*sourceFormantLowStopBandIndices, *sourceFormantHighStopBandIndices ; 


int extendedSourceNumFormants ; 
float *extendedSourceFormantCenterFreqs, * extendedSourceFormantAmps, * extendedSourceFormantBWs, * extendedSourceFormantQs ; 
int	*extendedSourceFormantPartial ; 
int	* extendedSourceFormantIndices,  *extendedSourceFormantLowStopBandIndices,  * extendedSourceFormantHighStopBandIndices ; 


float *targetFormantCenterFreqs, *targetFormantAmps, *targetFormantBWs, 
			*targetFormantQs; 
int targetNumFormants, targetN2, *targetFormantIndices, 
	*targetFormantLowStopBandIndices, *targetFormantHighStopBandIndices ; 

float *targetFormantCenterFreqs_ORIGINAL, *targetFormantAmps_ORIGINAL, *targetFormantBWs_ORIGINAL, 
			*targetFormantQs_ORIGINAL; 
int targetNumFormants_ORIGINAL, *targetFormantIndices_ORIGINAL, 
	*targetFormantLowStopBandIndices_ORIGINAL, *targetFormantHighStopBandIndices_ORIGINAL ; 

float lowerAmp, upperAmp, lowerFreq, upperFreq ; 

float *extendedTargetFormantCenterFreqs, *extendedTargetFormantAmps ; 
int *extendedTargetFormantPartial ; 

float *extendedTargetFormantBWs, *extendedTargetFormantQs ; 

int *extendedTargetFormantIndices, *extendedTargetFormantLowStopBandIndices, *extendedTargetFormantHighStopBandIndices ; 



int extendedTargetNumFormants, lowerIndex, upperIndex, duplicateFlag ; 

int *targetToSourcePairing, *sourceToTargetPairing ; 
float thisformantCFdiff, formantCFdiff, 
	targetCentroid, sourceCentroid, targetCentroidAmpSum, sourceCentroidAmpSum,
	centroidDiff  ; 

float Pre_Synthesis_Formant_Bandwidth_Extension_Factor=1, thisHalfBW ; 
int halfBWofBins, *sourceFormantLowBin, *sourceFormantHighBin, *sourceFormantCenterBin ; 
int lowHalfBinLimit, highHalfBinLimit ; 

int outputNumFormants, *outputSourceFormants, *outputTargetFormants ; 
float *SourceAmpSumOfSourceFormantsUsingThisSourceFormant, 
		*SourceAmpSumOfTargetFormantsUsingThisSourceFormant, 
			*numberOfThisTargetFormantUsed ; 
float *outputMappedChannel, *outputMappingTransposeMultiplier, * outputMappingSourcePartialMultiplier, 
	*outputInterpolationControlFormantWarp,
    thisFormantInterpolationWarp, interpolationPathDiffusion=0. ; 

float *channelResidue ; 
int 	channelResidueNumBins, *channelResidueBinNumbers, *residueFormantBinUsage ; 

int *outputSourceBinIndices; 

int thisLowBin, thisHighBin, thisOutputSourceFormant, thisOutputTargetFormant ; 
float  thisOutputSourceFormantCF, thisSourcePartialMultiplier, thisOutputTargetFormantCF, thisTransposeMultiplier ;    
float *outputMappingAmpScaler, *outputBinDistanceFromCenterFreqinOctaves, 
		thisOutputSourceFormantAmp, thisOutputTargetFormantAmp, thisAmpScaler ; 

float *outputSourceFormantDuplicateDBScaler, *outputTargetFormantDuplicateDBScaler ; 


int outputNumBins=0 ; 


float Target_formants_low_frequency_boundary=0., Target_formants_high_frequency_boundary=0,
Target_formants_transposition_in_semitones=0.  ; 

float Source_formants_low_frequency_boundary=20., 
	Source_formants_high_frequency_boundary=22050., Source_formants_decibel_threshold=(-200.) ; 


// SHELF EQ



float releasec,  minusreleasec,  attackc,  minusattackc ; 
double ar_dB ; 
float factor,  ampfactor ; 
int lowbin=0,  highbin=-1,  numberframes=0,  n=0 ; 
float lowbinfreq=0,  highbinfreq=-1 ; 



// 

struct func residue_bins_gain_in_decibels ; 

struct func bank_A_frequency_interpolation ; 
struct func bank_A_amplitude_interpolation ;
struct func bank_A_gain_in_decibels ;
struct func bank_A_decibel_rolloff_per_octave ; 

struct func bank_A_pitch_transposition_in_semitones ; 

struct func bank_B_frequency_interpolation ;
struct func bank_B_amplitude_interpolation ;
struct func bank_B_gain_in_decibels ;
struct func bank_B_decibel_rolloff_per_octave ; 

struct func bank_B_pitch_transposition_in_semitones ; 


// FREQUENCY SHIFT ADDER
struct  func  harmadd ; 

// GAIN
struct  func  dBgain ; 

// PITCH MULTIPLIER
struct  func  ptrans ; 

//  RELEASE
struct  func  release ; 

//  ATTACK
struct  func  attack ; 


// SPECTRUM WARPSHAPE INDEX
struct  func  warpshape ; 


//SHELF EQ
struct  func  dBlow;
struct  func  dBhi;
struct  func  freqlow;
struct  func  freqhi ;


//*****************INITIALIZE

residue_bins_gain_in_decibels.L = 1; residue_bins_gain_in_decibels.n = 1 ; residue_bins_gain_in_decibels.A[ 0 ] = 0. ; 


// 
bank_A_frequency_interpolation.L = 1; bank_A_frequency_interpolation.n = 1 ; bank_A_frequency_interpolation.A[ 0 ] = 0. ; 
bank_A_amplitude_interpolation.L = 1; bank_A_amplitude_interpolation.n = 1 ; bank_A_amplitude_interpolation.A[ 0 ] = 0. ; 
bank_A_gain_in_decibels.L = 1; bank_A_gain_in_decibels.n = 1 ; bank_A_gain_in_decibels.A[ 0 ] = 0. ; 
bank_A_decibel_rolloff_per_octave.L = 1; bank_A_decibel_rolloff_per_octave.n = 1 ; bank_A_decibel_rolloff_per_octave.A[ 0 ] = 0. ; 



bank_A_pitch_transposition_in_semitones.L = 1; bank_A_pitch_transposition_in_semitones.n = 1 ; bank_A_pitch_transposition_in_semitones.A[ 0 ] = 0. ; 


bank_B_frequency_interpolation.L = 1; bank_B_frequency_interpolation.n = 1 ; bank_B_frequency_interpolation.A[ 0 ] = 0. ; 
bank_B_amplitude_interpolation.L = 1; bank_B_amplitude_interpolation.n = 1 ; bank_B_amplitude_interpolation.A[ 0 ] = 0. ; 
bank_B_gain_in_decibels.L = 1; bank_B_gain_in_decibels.n = 1 ; bank_B_gain_in_decibels.A[ 0 ] = 0. ; 
bank_B_decibel_rolloff_per_octave.L = 1; bank_B_decibel_rolloff_per_octave.n = 1 ; bank_B_decibel_rolloff_per_octave.A[ 0 ] = 0. ; 

bank_B_pitch_transposition_in_semitones.L = 1; bank_B_pitch_transposition_in_semitones.n = 1 ; bank_B_pitch_transposition_in_semitones.A[ 0 ] = 0. ; 


// FREQUENCY SHIFT ADDER
harmadd.L = 1. ; harmadd.n = 1. ; harmadd.A[ 0 ] = 0. ; 

// GAIN
dBgain.L = 1. ;  dBgain.n = 1. ; dBgain.A[ 0 ] = 0. ; 

// PITCH MULTIPLIER
ptrans.L = 1. ; ptrans.n = 1. ; ptrans.A[ 0 ] = 0. ; 

//  RELEASE
release.L = 1. ; release.n = 1. ; release.A[ 0 ] = 0. ; 

//  ATTACK
attack.L = 1. ; attack.n = 1. ; attack.A[ 0 ] = 0. ; 


// SPECTRUM WARPSHAPE INDEX
warpshape.L = 1. ; warpshape.n = 1. ; warpshape.A[ 0 ] = 0. ; 

// SHELF EQ
dBlow.L = 1. ; dBlow.n = 1. ; dBlow.A[ 0 ] = 0. ; 
dBhi.L = 1. ; dBhi.n = 1. ; dBhi.A[ 0 ] = 0. ; 
freqlow.L = 1. ; freqlow.n = 1. ; freqlow.A[ 0 ] = 200. ; 
freqhi.L = 1. ; freqhi.n = 1. ; freqhi.A[ 0 ] = 2000. ; 


if( argc < 2 )usage() ; 
    while( (ch = crack( argc, argv,
    ":|/|~|@|=|_|a|A|b|B|c|C|d|D|e|E|f|F|g|G|h|H|i|I|j|J|k|K|l|L|m|M|n|N|o|O|p|P|q|Q|r|R|s|S|t|T|u|U|v|V|w|W|x|X|y|Y|z|Z|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {  // ! 


	    case 's':   strcpy(tempstring, arg_option);
			residue_bins_gain_in_decibels.fp = crackstring( tempstring, &residue_bins_gain_in_decibels); 
			break; // OK

	    case 'x': residueBinsFlag = (int) crackfloat( arg_option, ch );
			break;

	    case 'h':   strcpy(tempstring, arg_option);
			bank_A_decibel_rolloff_per_octave.fp = crackstring( tempstring, & bank_A_decibel_rolloff_per_octave); 
			break; // OK

	    case 'B':   strcpy(tempstring, arg_option);
			bank_B_decibel_rolloff_per_octave.fp = crackstring( tempstring, & bank_B_decibel_rolloff_per_octave); 
			break; // OK
	    case 'r':   strcpy(tempstring, arg_option);
			bank_A_frequency_interpolation.fp = crackstring( tempstring, &bank_A_frequency_interpolation); 
			break; 	// OK
	    case 'v':   strcpy(tempstring, arg_option); // ADD
			bank_A_amplitude_interpolation.fp = crackstring( tempstring, & bank_A_amplitude_interpolation); 
			break; 	// OK
	    case 'G':   strcpy(tempstring, arg_option);
			bank_A_gain_in_decibels.fp = crackstring( tempstring, & bank_A_gain_in_decibels); 
			break;		// OK
	    case 'f':   strcpy(tempstring, arg_option);
			bank_A_pitch_transposition_in_semitones.fp = crackstring( tempstring, & bank_A_pitch_transposition_in_semitones ); 
			break;	// OK
	    case ':':   strcpy(tempstring, arg_option);
			bank_B_frequency_interpolation.fp = crackstring( tempstring, & bank_B_frequency_interpolation); 
			break;	// OK
	    case 'J':   strcpy(tempstring, arg_option);
			bank_B_amplitude_interpolation.fp = crackstring( tempstring, & bank_B_amplitude_interpolation); 
			break;	// OK
	    case 'F':   strcpy(tempstring, arg_option);
			bank_B_gain_in_decibels.fp = crackstring( tempstring, & bank_B_gain_in_decibels); 
			break;	// OK
	    case 'W':   strcpy(tempstring, arg_option);
			bank_B_pitch_transposition_in_semitones.fp = crackstring( tempstring, & bank_B_pitch_transposition_in_semitones ); 
			break;
	    case '/': bank_A_0__banks_A_and_B_1 =  (int) crackfloat( arg_option, ch );
			break;	// OK
	    case '~': amplitude_normalization_decibel_gain_limit =  crackfloat( arg_option, ch );
			break;	// OK
	    case 'Y': added_formants_partial_dB_rolloff_per_partial =  crackfloat( arg_option, ch );
			break;	// OK
	    case 'z': Source_formants_low_frequency_boundary =  crackfloat( arg_option, ch );
			break;	// OK
	    case 'Z': Source_formants_high_frequency_boundary =  crackfloat( arg_option, ch );
			break;	// OK
	    case '@': Source_formants_decibel_threshold = crackfloat( arg_option, ch );
			break;
        case 'n':   extendSourceFormants = (int) crackfloat( arg_option, ch );
			break; 	// OK
        case 'U':   SourceFormantExtensionDbThreshold = crackfloat( arg_option, ch );
			break; 
        case 'V':   peakSourceExtendPartial = crackfloat( arg_option, ch );
			break; 	// OK
        case 'y':   AddOctavesToSourceExtensionPartials = crackfloat( arg_option, ch );
			break; 	// OK
	    case 'k':   AddOctavesToTargetExtensionPartials = (int) crackfloat( arg_option, ch );
			break;
	    case 'u': interpolationPathDiffusion =  crackfloat( arg_option, ch );
			break; 	// OK
	    case 'T': Target_formants_transposition_in_semitones =  crackfloat( arg_option, ch );
			break;	// OK
	    case 'K':   Target_formants_decibel_threshold =  crackfloat( arg_option, ch );
			break;	// OK
	    case 'o': Target_formants_low_frequency_boundary =  crackfloat( arg_option, ch );
			break;	// OK
	    case 'O': Target_formants_high_frequency_boundary =  crackfloat( arg_option, ch );
			break;	// OK
	    case 'j':   peakTargetExtendPartial = (int) crackfloat( arg_option, ch );
			break;	// OK
	    case 'd':   TargetFormantExtensionDbThreshold = crackfloat( arg_option, ch );
			break;	// OK
	    case 'c':   extendTargetFormants = (int) crackfloat( arg_option, ch );
			break;	// OK
	    case 'S':   Pre_Synthesis_Formant_Bandwidth_Extension_Factor = crackfloat( arg_option, ch );
			break;	// OK
	    case 'N':   N = (int) crackfloat( arg_option, ch );
			break;	// OK
	    case 'M':   Nw = (int) crackfloat( arg_option, ch );
			break;	// OK
	    case 'w':   window_type = (int) crackfloat( arg_option, ch ) ;
			break;	// OK
	    case 'D':   frames_per_sec = crackfloat( arg_option, ch );
			break;	// OK
	    case 'I':   tfactor = crackfloat( arg_option, ch );
			break;	// OK
	    case 'P':   strcpy(tempstring, arg_option);
			ptrans.fp = crackstring( tempstring, &ptrans); 
			break;	// OK
	    case 't':   threshfacdB = crackfloat( arg_option, ch );
			break;
	    case 'a':   strcpy(tempstring, arg_option);
			harmadd.fp = crackstring( tempstring, &harmadd );
			break;	// OK
	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;
	    case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
			break;
	    case 'H':   strcpy(tempstring, arg_option);
			dBlow.fp = crackstring( tempstring, &dBlow );
			break;
	    case 'X':   strcpy(tempstring, arg_option);
			dBhi.fp = crackstring( tempstring, &dBhi );
			break;
	    case 'm':   strcpy(tempstring, arg_option);
			freqlow.fp = crackstring( tempstring, &freqlow );
			break;
	    case 'R':   strcpy(tempstring, arg_option);
			freqhi.fp = crackstring( tempstring, &freqhi );
			break;
	    case 'A':   strcpy(tempstring, arg_option);
			dBgain.fp = crackstring( tempstring, &dBgain );
			break;
	    case 'l':   strcpy(tempstring, arg_option);
			attack.fp = crackstring( tempstring, 
			    &attack );
			break;
	    case 'L':   strcpy(tempstring, arg_option);
			release.fp = crackstring( tempstring, 
			    &release );
			break;
	    case 'E':   strcpy(sourceFormantsFile, arg_option);  
			break;

	    case 'g':   strcpy(targetFormantsFile, arg_option);  
			break;

		case 'p':	quiet = (int) crackfloat( arg_option, ch ) ; break;
		case 'i':	ampstatinc = crackfloat( arg_option, ch ) ; break;
		case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;
		case '=':	rescalev = crackfloat( arg_option, ch ) ; break;




} }



prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "FORMANTSMAPPER", 69 ) ; 
prline( 69,  "-" ) ; 

if(channelout == 0){ // ALL CHANNELS
	channelflag = 0 ; 
	beginchan = 0 ;
} else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
} ; 
 
// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
setupfiles(argc, argv) ; 

endchan = beginchan + ochan ; 

    // GET NAME OF USER
user = getlogin(); 
  
// **** SET UPS *****
R = isr ; // SAMPLE RATE EQUALS INPUT FILE
if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 200.\n\n" ) ; 
	frames_per_sec = 200 ; 
} ; 
D = (int) ((float) R / frames_per_sec) ; 

if(tfactor <= 0.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY A TIME FACTOR > 0. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 1.\n\n" ) ; 
	tfactor = 1. ; 
} ; 
I = (int) ((float) D * tfactor ) ; 

//******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
if( Nw <= 0 ) Nw = 2 * N ;
if( Nw < I ){
	// INCREASE WINDOW SIZE TO ACCOMODATE INTERPOLATION
	Nw = 2 ; while( Nw <= I )Nw *= 2 ;
	prt( "\n----> INCREASING WINDOW SIZE TO ACCOMODATE TIME RESYNTHESIS INTERPOLATION. <---" ) ;
	pri( Nw,  "NEW WINDOW SIZE" ) ; 
} ; 
//*********************************

nyquist = R/2.0;
fundamental =  ((float) R / (float) N) ; 
PI = 4.*atan(1.) ;
TWOPI = 8.* (float) atan(1.) ;
ar_dB =  (double) pow( (double) 10.0, (double) ( -60. / 20.) );	
ampfactor = 1. / pow( (double) 10.0, (double) (-100./20.) );	
N2 = N>>1 ;
Nw2 = Nw>>1 ;
// HERE
IR = (float) I / (float) R ;
factor = (float) R / ((float) D * TWOPI);

// HERE
// COMPUTE THE DURATION
dur = (endt - begint) * (float) I / (float) D ; 

// DETERMINE OVERLAP/ADD OR OSCIL BANK RESYNTHESIS
	// OSC BANK
	P = 1. ; obank = 1 ;  
    
//   SOURCE DATA
if( strcmp( sourceFormantsFile, "" ) == 0 ){
    prt( "ERROR: --------> SOURCE FORMANTS FILE IS NOT SPECIFIED <-----------\n\nBYE.\n\n" ) ; exit(EXIT_FAILURE) ; 
}else{
    if( (adata=fopen( sourceFormantsFile, "rb" )) == NULL){
	prt( "\nERROR: -----> SOURCE FORMANTS FILE NOT FOUND! <---------\n" ) ; 
	prs( sourceFormantsFile, "SOURCE FORMANTS FILE" ) ; 
	prt( "\n\nBYE.\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
    }else{
	// READ IN DATA
	prs( sourceFormantsFile, "SOURCE FORMANTS FILE" ) ; 
	fread( &sourceNumFormants, sizeof(int), 1, adata ) ; 
	fread( &sourceN2, sizeof(int), 1, adata ) ; // NUMBER OF BINS IN ORIGINAL ANALYSIS
	// EXIT IF NUMBER OF ANALYSIS BINS != THOSE FOR channel.
	if(sourceN2 != N2){
	    prt( "\nError: -----------> SOURCE FORMANT ANALYSIS FFT NOT EQUAL TO CURRENT ANALYSIS. <--------------" ); exit(EXIT_FAILURE); 
	    prt( "\n\nBYE.\n\n" ) ; 
	}; 
	// CREATE ARRAY SPACES
	fvec( sourceFormantCenterFreqs,  sourceNumFormants ) ; 
	fvec( sourceFormantAmps,  sourceNumFormants ) ; 
	fvec( sourceFormantBWs,  sourceNumFormants ) ; 
	fvec( sourceFormantQs,  sourceNumFormants ) ; 
	ivec( sourceFormantIndices,  sourceNumFormants ) ; 
	ivec( sourceFormantLowStopBandIndices,  sourceNumFormants ) ; 
	ivec( sourceFormantHighStopBandIndices,  sourceNumFormants ) ; 

	// READ IN DATA
	for(i = 0; i < sourceNumFormants; i++){
	    fread( &sourceFormantCenterFreqs[i], sizeof(float), 1, adata ) ; 
	    fread( &sourceFormantAmps[i], sizeof(float), 1, adata ) ;
	    fread( &sourceFormantBWs[i], sizeof(float), 1, adata ) ;
	    fread( &sourceFormantQs[i], sizeof(float), 1, adata ) ;
	    fread( &sourceFormantIndices[i], sizeof(int), 1, adata ) ;
	    fread( &sourceFormantLowStopBandIndices[i], sizeof(int), 1, adata ) ;
	    fread( &sourceFormantHighStopBandIndices[i], sizeof(int), 1, adata ) ;
	}; 

	// PRINT SOURCE DATA
	fprintf( stderr, "\n\n** ORIGINAL SOURCE FORMANT DATA **:\n" ) ; 
	fprintf( stderr, "(%d Analysis Bins)\n", sourceN2 ) ; 
	prt( "\n\n" ) ; 
	fprintf( stderr, 
		"Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\n" 
	) ; 
	for( i = 0; i < sourceNumFormants; i++){
	    fprintf( stderr, 
 	           "%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\n",
        	(i + 1), 
		(int) sourceFormantCenterFreqs[i], 
		(int) amp_to_dB( sourceFormantAmps[i] ),
		(int) sourceFormantBWs[i], 
		(int)sourceFormantQs[i],
		sourceFormantIndices[i],
		sourceFormantLowStopBandIndices[i],
		sourceFormantHighStopBandIndices[i]
    	   ) ; 
	} ; 
    } ; 
} ; 

// RESET STOP BANDS IF DESIRED
if( Pre_Synthesis_Formant_Bandwidth_Extension_Factor != 1. ){

    for( i = 0; i < sourceNumFormants ; i++){

        sourceFormantLowStopBandIndices[i] = sourceFormantIndices[i] - 
		( Pre_Synthesis_Formant_Bandwidth_Extension_Factor * 
				(sourceFormantIndices[i] - sourceFormantLowStopBandIndices[i]) ) ;
        if( sourceFormantLowStopBandIndices[i] < 0 ) sourceFormantLowStopBandIndices[i] = 0 ; 
        if( i != 0 ){
            if( sourceFormantLowStopBandIndices[i] <= sourceFormantHighStopBandIndices[i - 1] ) 
			sourceFormantLowStopBandIndices[i] =  sourceFormantHighStopBandIndices[i - 1] + 1 ; 
        } ; 

        sourceFormantHighStopBandIndices[i] = sourceFormantIndices[i] + 
		( Pre_Synthesis_Formant_Bandwidth_Extension_Factor * 
				(sourceFormantHighStopBandIndices[i] - sourceFormantIndices[i]) ) ; 
        if( sourceFormantHighStopBandIndices[i] > (N2 - 1) ) sourceFormantHighStopBandIndices[i] = N2 - 1 ; 
        if( i != (sourceNumFormants - 1) ){
            if( sourceFormantHighStopBandIndices[i] >= sourceFormantLowStopBandIndices[i + 1] ) 
			sourceFormantHighStopBandIndices[i] =  sourceFormantLowStopBandIndices[i + 1] - 1 ; 
        } ; 
    } ; 

	// PRINT SOURCE DATA
	fprintf( stderr, "\n\n** SOURCE FORMANT DATA: MODIFIED STOP-BANDS **:\n" ) ; 
	fprintf( stderr, "(%d Analysis Bins)\n", sourceN2 ) ; 
	prt( "\n\n" ) ; 
	fprintf( stderr, 
		"Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\n" 
	) ; 
	for( i = 0; i < sourceNumFormants; i++){
	    fprintf( stderr, 
 	           "%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\n",
        	(i + 1), 
		(int) sourceFormantCenterFreqs[i], 
		(int) amp_to_dB( sourceFormantAmps[i] ),
		(int) sourceFormantBWs[i], 
		(int)sourceFormantQs[i],
		sourceFormantIndices[i],
		sourceFormantLowStopBandIndices[i],
		sourceFormantHighStopBandIndices[i]
    	   ) ; 
	} ; 


} ; 

// ****** FILTER SOURCE FORMANTS

    // OMIT FORMANTS WITH AMPLITUDE BELOW THRESHOLD
    if( Source_formants_low_frequency_boundary < 0. ) Source_formants_low_frequency_boundary = 0. ; 
    if( Source_formants_high_frequency_boundary <= 0. ) Source_formants_high_frequency_boundary = nyquist ; 
    k = 0; 

    for( i = 0; i < sourceNumFormants; i++){
        if( 
            (amp_to_dB( sourceFormantAmps[i] ) >= Source_formants_decibel_threshold) && 
            (sourceFormantCenterFreqs[i] >= Source_formants_low_frequency_boundary) &&
            (sourceFormantCenterFreqs[i] <= Source_formants_high_frequency_boundary) &&
            (sourceFormantCenterFreqs[i] <= nyquist) &&
            (sourceFormantCenterFreqs[i] > 0.)
	  ){
	    sourceFormantCenterFreqs[k] = sourceFormantCenterFreqs[i] ; 
	    sourceFormantAmps[k] = sourceFormantAmps[i] ; 
	    sourceFormantBWs[k] = sourceFormantBWs[i] ;
	    sourceFormantQs[k] = sourceFormantQs[i] ;
	    sourceFormantIndices[k] = sourceFormantIndices[i] ;
	    sourceFormantLowStopBandIndices[k] = sourceFormantLowStopBandIndices[i] ;
	    sourceFormantHighStopBandIndices[k] = sourceFormantHighStopBandIndices[i] ;
          k++ ;            

        } ; 

    } ;     

    if( k == 0 ){
         prt( "\n\nERROR: -------> NO FORMANTS REMAIN FOLLOWING FILTERING OF SOURCE FORMANTS <--------\n\n . . . . . . . BYE.\n\n" ); 
	   exit(EXIT_FAILURE); 
    }; 


    if( k != sourceNumFormants ){
        sourceNumFormants = k ; 
	  // PRINT SOURCE DATA
	  fprintf( stderr, "\n\n** FILTERED SOURCE FORMANT DATA **:\n" ) ; 
	  prt( "\n\n" ) ; 
	  fprintf( stderr, 
		"Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\n" 
	  ) ; 
	  for( i = 0; i < sourceNumFormants; i++){
	      fprintf( stderr, 
 	           "%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\n",
        	(i + 1), 
		(int) sourceFormantCenterFreqs[i], 
		(int) amp_to_dB( sourceFormantAmps[i] ),
		(int) sourceFormantBWs[i], 
		(int)sourceFormantQs[i],
		sourceFormantIndices[i],
		sourceFormantLowStopBandIndices[i],
		sourceFormantHighStopBandIndices[i]
    	      ) ; 
	  } ; 
    } ; 


// ******


if( extendSourceFormants == 1 ){
// EXTEND NUMBER OF SOURCE FORMANTS USING OVERTONES OF FORMANTS
// FIRST FIND OUT HOW MANY MORE THERE WILL BE.

    for(l = 0; l < 2; l++){
        extendedSourceNumFormants = sourceNumFormants ; 
        for(i = 0; i < sourceNumFormants; i++){
			// TEST FOR SUITABLE AMPLITUDE IN BASE FORMANT
            if( sourceFormantAmps[i] > dB_to_amp( SourceFormantExtensionDbThreshold ) ){
			// START WITH PARTIAL 2
                 partialNumber = 2. ;  thisOctave = 1. ; thisPartialNumber = (int)(partialNumber * thisOctave) ;
			proposedPartialFreq = sourceFormantCenterFreqs[i] * thisPartialNumber ; 
			proposedPartialAmp = sourceFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;                

			// WHILE PROPOSED PARTIAL FREQ IS BELOW THE NYQ AND PARTIAL NUMBER IS BELOW THE LIMIT OR ALL ARE INCLUDED, THEN
                while( (proposedPartialFreq < nyquist) && 
				((partialNumber <= peakSourceExtendPartial) || (peakSourceExtendPartial == 0.))  
				 		){
                    
				// WHILE PROPOSED (OCTAVE-ADDED) PARTIAL FREQ IS STILL BELOW THE NYQUIST
                    proposedPartialFreq = sourceFormantCenterFreqs[i] * (float) thisPartialNumber ; 
                    proposedPartialAmp = sourceFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;

                    while( proposedPartialFreq < nyquist ){
                                         
                        if(l == 1) {
                        //         thisPartialNumber = (int)(partialNumber * thisOctave) ; 
						extendedSourceFormantCenterFreqs[extendedSourceNumFormants] = proposedPartialFreq ; // SAVE CENTER FREQ
						extendedSourceFormantBWs[extendedSourceNumFormants] = sourceFormantBWs[i] ; // SAVE BW
						extendedSourceFormantQs[extendedSourceNumFormants] = 
										proposedPartialFreq / sourceFormantBWs[i]  ; // SAVE Q
						// SAVE AMP AS PARTIAL-CORRELATED PROPORTION OF FUNDAMENTAL AMP. 
                            	extendedSourceFormantAmps[extendedSourceNumFormants] = sourceFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;
						// SAVE INDICES
						extendedSourceFormantIndices[extendedSourceNumFormants] = 
							(int)(
								(extendedSourceFormantCenterFreqs[extendedSourceNumFormants] / fundamental)
							+ 0.5) ; 
						extendedSourceFormantLowStopBandIndices[extendedSourceNumFormants] = 
							(int) ( ( ( extendedSourceFormantCenterFreqs[extendedSourceNumFormants] - 
										(0.5 * extendedSourceFormantBWs[extendedSourceNumFormants])
								) / fundamental)  + 0.5) ; 
						extendedSourceFormantHighStopBandIndices[extendedSourceNumFormants] = 
							(int) ( ( ( extendedSourceFormantCenterFreqs[extendedSourceNumFormants] + 
										(0.5 * extendedSourceFormantBWs[extendedSourceNumFormants])
								) / fundamental)  + 0.5) ; 
                                 extendedSourceFormantPartial[extendedSourceNumFormants] = thisPartialNumber ;   

                        } ;  
                        extendedSourceNumFormants++ ;  

					// IF OCTAVE ADD IS ON, THEN DOUBLE FREQ; OTHERWISE, SET TO NYQUIST IN ORDER TO EXIT LOOP
                        thisOctave = thisOctave * 2. ; thisPartialNumber = (int)(partialNumber * thisOctave) ;
                        proposedPartialFreq = (AddOctavesToSourceExtensionPartials == 1) ? 
					sourceFormantCenterFreqs[i] * (float) thisPartialNumber : nyquist ;
                        proposedPartialAmp = sourceFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ; 

                    } ; 

                    partialNumber += 1. ; thisOctave = 1. ; thisPartialNumber = (int)(partialNumber * thisOctave) ;
                    proposedPartialFreq = sourceFormantCenterFreqs[i] * thisPartialNumber ;
                    proposedPartialAmp = sourceFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;


                }; 
            } ; 
        } ; 

//        pri( sourceNumFormants, "sourceNumFormants" ) ; 
//        pri( extendedSourceNumFormants, "extendedSourceNumFormants" ) ; 


	if(l == 0) {
		fvec( extendedSourceFormantCenterFreqs, extendedSourceNumFormants ); 
		fvec( extendedSourceFormantAmps, extendedSourceNumFormants ); 
		fvec( extendedSourceFormantBWs,  extendedSourceNumFormants ) ; 
		fvec( extendedSourceFormantQs,  extendedSourceNumFormants ) ; 
		ivec( extendedSourceFormantPartial, extendedSourceNumFormants ) ; 
		ivec( extendedSourceFormantIndices,  extendedSourceNumFormants ) ; 
		ivec( extendedSourceFormantLowStopBandIndices,  extendedSourceNumFormants ) ; 
		ivec( extendedSourceFormantHighStopBandIndices,  extendedSourceNumFormants ) ; 

		for(n = 0; n < extendedSourceNumFormants; n++){
			extendedSourceFormantCenterFreqs[n] = -1. ; 
			extendedSourceFormantAmps[n] = -1. ; 
			extendedSourceFormantBWs[n] = -1. ; 
			extendedSourceFormantQs[n] = -1. ; 
			extendedSourceFormantIndices[n] = -1 ; 
			extendedSourceFormantLowStopBandIndices[n] = -1 ; 
			extendedSourceFormantHighStopBandIndices[n] = -1 ;
			extendedSourceFormantPartial[n] = -1 ;  
		} ; 

           // COPY EXISTING FREQS AND AMPS FORMANTS TO NEW ARRAYS.
           for(i = 0; i < sourceNumFormants; i++){
		    extendedSourceFormantCenterFreqs[i] = sourceFormantCenterFreqs[i] ; 
		    extendedSourceFormantAmps[i] = sourceFormantAmps[i] ; 
		    extendedSourceFormantBWs[i] = sourceFormantBWs[i] ; 
		    extendedSourceFormantQs[i] = sourceFormantQs[i] ; 
		    extendedSourceFormantIndices[i] = sourceFormantIndices[i] ; 
		    extendedSourceFormantLowStopBandIndices[i] = sourceFormantLowStopBandIndices[i] ; 
		    extendedSourceFormantHighStopBandIndices[i] = sourceFormantHighStopBandIndices[i] ; 

//fprintf( stderr, "-1. SOURCE FORMANT %d: -- LOW BIN: %d\t HIGH BIN: %d\n",
//		i, extendedSourceFormantLowStopBandIndices[i], extendedSourceFormantHighStopBandIndices[i] );


		    extendedSourceFormantPartial[i] = 1 ; 

           } ; 


	} ; 





    }; 

/*
prt( "A\n" ); 
for(i = 0; i < extendedSourceNumFormants; i++)
    fprintf( stderr, "%d) FREQ: %f\tDB: %f\n",  (i + 1), extendedSourceFormantCenterFreqs[i], amp_to_dB( extendedSourceFormantAmps[i] ) ) ; 
*/


    // SORT DATA BY FREQS.    
    n = 0 ; // UNSORTED
    while( n == 0 ){
        n = 1 ; 
        for(i = 0; i < extendedSourceNumFormants - 1; i++){
            if( extendedSourceFormantCenterFreqs[i] > extendedSourceFormantCenterFreqs[i + 1] ){
                // SWITCH AND FLAG
                temp = extendedSourceFormantCenterFreqs[i] ; 
                extendedSourceFormantCenterFreqs[i] = extendedSourceFormantCenterFreqs[i + 1] ; 
                extendedSourceFormantCenterFreqs[i + 1] = temp ; 

                temp = extendedSourceFormantAmps[i] ; 
                extendedSourceFormantAmps[i] = extendedSourceFormantAmps[i + 1] ; 
                extendedSourceFormantAmps[i + 1] = temp ; 

                temp = extendedSourceFormantBWs[i] ; 
                extendedSourceFormantBWs[i] = extendedSourceFormantBWs[i + 1] ; 
                extendedSourceFormantBWs[i + 1] = temp ; 

                temp = extendedSourceFormantQs[i] ; 
                extendedSourceFormantQs[i] = extendedSourceFormantQs[i + 1] ; 
                extendedSourceFormantQs[i + 1] = temp ; 
			
                k = extendedSourceFormantPartial[i] ; 
                extendedSourceFormantPartial[i] = extendedSourceFormantPartial[i + 1] ; 
                extendedSourceFormantPartial[i + 1] = k ; 

                k = extendedSourceFormantIndices[i] ; 
                extendedSourceFormantIndices[i] = extendedSourceFormantIndices[i + 1] ; 
                extendedSourceFormantIndices[i + 1] = k ; 

                k = extendedSourceFormantLowStopBandIndices[i] ; 
                extendedSourceFormantLowStopBandIndices[i] = extendedSourceFormantLowStopBandIndices[i + 1] ; 
                extendedSourceFormantLowStopBandIndices[i + 1] = k ; 

                k = extendedSourceFormantHighStopBandIndices[i] ; 
                extendedSourceFormantHighStopBandIndices[i] = extendedSourceFormantHighStopBandIndices[i + 1] ; 
                extendedSourceFormantHighStopBandIndices[i + 1] = k ; 


                n = 0 ; 

            } ;  
        }; 
    } ; 

/*
prt( "SORTED\n" ); 
for(i = 0; i < extendedSourceNumFormants; i++)
    fprintf( stderr, "%d) FREQ: %f\tDB: %f\n",  (i + 1), extendedSourceFormantCenterFreqs[i], amp_to_dB( extendedSourceFormantAmps[i] ) ) ; 
*/
	// REMOVE THE LESSER OF OVERLAPPING ADDED FORMANTS (PARTIAL 2 OR GREATER)
    k = 0 ; 
    for( i = 0; i < extendedSourceNumFormants; i++){
         
        if( i == 0 ){
            // FIRST -- TRANSFER IT
            transferCode = 1 ; // 0: no transfer, 	1: add i to list at k, incr k		2: replace (k - 1) with i; (no incr of k) 
        }else{
            if( (extendedSourceFormantPartial[k - 1] + extendedSourceFormantPartial[i]) > 2 ){
		     // ONE OF THE TWO, OR BOTH, IS AN ADDED PARTIAL
 		     // CHECK FOR FORMANT BW OVERLAP
                if( extendedSourceFormantHighStopBandIndices[k - 1] > extendedSourceFormantLowStopBandIndices[i] ){
                    // OVERLAP -- OMIT ONE
                    // CHECK FOR ORIGINAL; IF SO, KEEP ORIGINAL
                   if( (extendedSourceFormantPartial[k - 1] == 1) && (extendedSourceFormantPartial[i] != 1) ){
                        // KEEP OLD AND ADD NOTHING
                       transferCode = 0 ; 
                   }else if( (extendedSourceFormantPartial[k - 1] != 1) && (extendedSourceFormantPartial[i] == 1) ){
                        // PUT NEW (AN ORIGINAL FORMANT) IN PLACE OF (k - 1)
                        transferCode = 2 ; 
                   }else{
                        // BOTH ARE SYNTHETIC, NON-FUNDAMENTAL PARTIALS; 
                        // KEEP THE ONE WITH THE STRONGER (THEORETICAL) AMP.
                        if( extendedSourceFormantAmps[k - 1] > extendedSourceFormantAmps[i] ){
                             // LAST IS STRONGER; ADD NOTHING
                             transferCode = 0 ; 
                        }else{
                             // NEW SYNTHETIC IS STRONGER; PUT NEW IN PLACE OF (k - 1).
                             transferCode = 2 ;
                        } ;  

                   } ; 

                }else{
                    // NO OVERLAP; ADD NEW TO LIST
                    transferCode = 1 ;
                } ; 


            }else{
                // BOTH ARE ORIGINAL FORMANTS -- TRANSFER NEW INTO LIST
                transferCode = 1 ;
            } ; 

        } ; 

        // TRANSFER OR REPLACE
        if( transferCode == 1 ){
            m1 = k ; k++ ;    
        }else if( transferCode == 2 ){
            m1 = k - 1 ;
        } ; 

        if( (transferCode == 1) || (transferCode == 2) ){
		    extendedSourceFormantCenterFreqs[m1] = extendedSourceFormantCenterFreqs[i] ; 
		    extendedSourceFormantAmps[m1] = extendedSourceFormantAmps[i] ; 
		    extendedSourceFormantBWs[m1] = extendedSourceFormantBWs[i] ; 
		    extendedSourceFormantQs[m1] = extendedSourceFormantQs[i] ; 
		    extendedSourceFormantIndices[m1] = extendedSourceFormantIndices[i] ; 
		    extendedSourceFormantLowStopBandIndices[m1] = extendedSourceFormantLowStopBandIndices[i] ; 
		    extendedSourceFormantHighStopBandIndices[m1] = extendedSourceFormantHighStopBandIndices[i] ; 
		    extendedSourceFormantPartial[m1] = extendedSourceFormantPartial[i] ; 
        

        } ; 


    } ; 

    extendedSourceNumFormants = k ; 

	// PRINT SOURCE DATA
    fprintf( stderr, "\n\n** EXTENDED SOURCE FORMANT DATA WITH ADDED HARMONIC PARTIALS **:\n" ) ; 
    fprintf( stderr, "(%d Analysis Bins)\n", sourceN2 ) ; 
    prt( "\n\n" ) ; 
    fprintf( stderr, 
	  "Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\tPartial\n" 
    ) ; 
    for( i = 0; i < extendedSourceNumFormants; i++){
        if( extendedSourceFormantPartial[i] > 1 ){
    		   fprintf( stderr, 
            	"%i.added\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\t\t\t%i\n",
    			(i + 1), 
			(int) extendedSourceFormantCenterFreqs[i], 
			(int) amp_to_dB( extendedSourceFormantAmps[i] ),
			(int) extendedSourceFormantBWs[i], 
			(int)extendedSourceFormantQs[i],
			extendedSourceFormantIndices[i],
			extendedSourceFormantLowStopBandIndices[i],
			extendedSourceFormantHighStopBandIndices[i],
			extendedSourceFormantPartial[i]
    		  ) ; 
        }else{
    		 fprintf( stderr, 
            	"%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\t\t\t%i\n",
    			(i + 1), 
			(int) extendedSourceFormantCenterFreqs[i], 
			(int) amp_to_dB( extendedSourceFormantAmps[i] ),
			(int) extendedSourceFormantBWs[i], 
			(int)extendedSourceFormantQs[i],
			extendedSourceFormantIndices[i],
			extendedSourceFormantLowStopBandIndices[i],
			extendedSourceFormantHighStopBandIndices[i],
			extendedSourceFormantPartial[i]
    		 ) ; 
       }; 
    } ; 





}else{
    extendedSourceNumFormants = sourceNumFormants ; 
    fvec( extendedSourceFormantCenterFreqs, extendedSourceNumFormants ); 
    fvec( extendedSourceFormantAmps, extendedSourceNumFormants ); 
    fvec( extendedSourceFormantBWs,  extendedSourceNumFormants ) ; 
    fvec( extendedSourceFormantQs,  extendedSourceNumFormants ) ; 
    ivec( extendedSourceFormantPartial, extendedSourceNumFormants ) ; 
    ivec( extendedSourceFormantIndices,  extendedSourceNumFormants ) ; 
    ivec( extendedSourceFormantLowStopBandIndices,  extendedSourceNumFormants ) ; 
    ivec( extendedSourceFormantHighStopBandIndices,  extendedSourceNumFormants ) ; 


    // COPY EXISTING FREQS AND AMPS FORMANTS TO NEW ARRAYS.
    for(i = 0; i < extendedSourceNumFormants; i++){
        extendedSourceFormantCenterFreqs[i] = sourceFormantCenterFreqs[i] ; 
        extendedSourceFormantAmps[i] = sourceFormantAmps[i] ; 
        extendedSourceFormantBWs[i] = sourceFormantBWs[i] ; 
        extendedSourceFormantQs[i] = sourceFormantQs[i] ; 
        extendedSourceFormantPartial[i] = 1 ; 
        extendedSourceFormantIndices[i] = sourceFormantIndices[i] ; 
        extendedSourceFormantLowStopBandIndices[i] = sourceFormantLowStopBandIndices[i] ; 
        extendedSourceFormantHighStopBandIndices[i] = sourceFormantHighStopBandIndices[i] ; 


//fprintf( stderr, "0. SOURCE FORMANT %d: -- LOW BIN: %d\t HIGH BIN: %d\n",
//		i, extendedSourceFormantLowStopBandIndices[i], extendedSourceFormantHighStopBandIndices[i] );


    } ; 
} ; 


// ****


//  ************************* TARGET DATA ********************
if( strcmp( targetFormantsFile, "" ) == 0){
    prt( "ERROR: --------> TARGET FORMANTS FILE IS NOT SPECIFIED <-----------\n\nBYE.\n\n" ) ; exit(EXIT_FAILURE) ; 
}else{
    if( (adata=fopen( targetFormantsFile, "rb" )) == NULL){
	prt( "\nERROR: -----> TARGET FORMANTS FILE NOT FOUND! <---------\n" ) ; 
	prs( targetFormantsFile, "TARGET FORMANTS FILE" ) ; 
	prt( "\n\nBYE.\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
    }else{
	// READ IN DATA
	prs( targetFormantsFile, "TARGETS FORMANTS FILE" ) ; 
	fread( &targetNumFormants, sizeof(int), 1, adata ) ; 
	fread( &targetN2, sizeof(int), 1, adata ) ; // NUMBER OF BINS IN ORIGINAL ANALYSIS
	if(targetN2 != N2){
	    prt( "SOURCE FORMANT ANALYSIS FFT NOT EQUAL TO CURRENT ANALYSIS." ); 
	    prt("\n\n"); 
	    exit(EXIT_FAILURE); 
	}; 
	// CREATE ARRAY SPACES
	fvec( targetFormantCenterFreqs,  targetNumFormants ) ; 
	fvec( targetFormantAmps,  targetNumFormants ) ; 
	fvec( targetFormantBWs,  targetNumFormants ) ; 
	fvec( targetFormantQs,  targetNumFormants ) ; 
	ivec( targetFormantIndices,  targetNumFormants ) ; 
	ivec( targetFormantLowStopBandIndices,  targetNumFormants ) ; 
	ivec( targetFormantHighStopBandIndices,  targetNumFormants ) ; 


	// CREATE ARRAY SAVE SPACES
	targetNumFormants_ORIGINAL = targetNumFormants ; 
	fvec( targetFormantCenterFreqs_ORIGINAL,  targetNumFormants_ORIGINAL ) ; 
	fvec( targetFormantAmps_ORIGINAL,  targetNumFormants_ORIGINAL ) ; 
	fvec( targetFormantBWs_ORIGINAL,  targetNumFormants_ORIGINAL ) ; 
	fvec( targetFormantQs_ORIGINAL,  targetNumFormants_ORIGINAL ) ; 
	ivec( targetFormantIndices_ORIGINAL,  targetNumFormants_ORIGINAL ) ; 
	ivec( targetFormantLowStopBandIndices_ORIGINAL,  targetNumFormants_ORIGINAL ) ; 
	ivec( targetFormantHighStopBandIndices_ORIGINAL,  targetNumFormants_ORIGINAL ) ; 


	// READ IN DATA
	for(i = 0; i <  targetNumFormants; i++){   
	    fread( &targetFormantCenterFreqs[i], sizeof(float), 1, adata ) ; 
	    fread( &targetFormantAmps[i], sizeof(float), 1, adata ) ;
	    fread( &targetFormantBWs[i], sizeof(float), 1, adata ) ; // NOT USED
	    fread( &targetFormantQs[i], sizeof(float), 1, adata ) ; // NOT USED
	    fread( &targetFormantIndices[i], sizeof(int), 1, adata ) ; // NOT USED
	    fread( &targetFormantLowStopBandIndices[i], sizeof(int), 1, adata ) ; // NOT USED
	    fread( &targetFormantHighStopBandIndices[i], sizeof(int), 1, adata ) ;// NOT USED
	}; 
	// COPY INTO SAVE SPACE
	for(i = 0; i <  targetNumFormants_ORIGINAL; i++){   
	    targetFormantCenterFreqs_ORIGINAL[i] = targetFormantCenterFreqs[i] ;  
	    targetFormantAmps_ORIGINAL[i] = targetFormantAmps[i] ;
	    targetFormantBWs_ORIGINAL[i] = targetFormantBWs[i] ; // NOT USED
	    targetFormantQs_ORIGINAL[i] = targetFormantQs[i] ; // NOT USED
	    targetFormantIndices_ORIGINAL[i] = targetFormantIndices[i] ; // NOT USED
	    targetFormantLowStopBandIndices_ORIGINAL[i] = targetFormantLowStopBandIndices[i] ; // NOT USED
	    targetFormantHighStopBandIndices_ORIGINAL[i] = targetFormantHighStopBandIndices[i] ; // NOT USED
	}; 



	// PRINT TARGET DATA
	fprintf( stderr, "\n\n** ORIGINAL TARGET FORMANT DATA **:\n" ) ; 
	fprintf( stderr, "(%d Analysis Bins)\n", targetN2 ) ; 
	prt( "\n\n" ) ; 
	fprintf( stderr, 
		"Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\n" 
	) ; 
	for( i = 0; i < targetNumFormants; i++){
	    fprintf( stderr, 
 	           "%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\n",
        	(i + 1), 
		(int) targetFormantCenterFreqs[i], 
		(int) amp_to_dB( targetFormantAmps[i] ),
		(int) targetFormantBWs[i], 
		(int)targetFormantQs[i],
		targetFormantIndices[i],
		targetFormantLowStopBandIndices[i],
		targetFormantHighStopBandIndices[i]
    	   ) ; 
	} ; 
    } ; 

    // TRANSPOSE TARGET FORMANTS
    for(i = 0; i < targetNumFormants; i++ ){
        temp = semitones_to_mult( Target_formants_transposition_in_semitones ) ; 
	  targetFormantCenterFreqs[i] *= temp ; targetFormantBWs[i] *= temp ;  
    } ; 

    // OMIT FORMANTS WITH AMPLITUDE BELOW THRESHOLD 
	// FIRST PREPARE BOUNDARIES.
    if( Target_formants_low_frequency_boundary < 0. ) Target_formants_low_frequency_boundary = 0. ; 
    if( (Target_formants_high_frequency_boundary <= 0.) || (Target_formants_high_frequency_boundary > nyquist) ) 
			Target_formants_high_frequency_boundary = nyquist ; 
    k = 0; 

    for( i = 0; i < targetNumFormants; i++){
        if( 
            (amp_to_dB( targetFormantAmps[i] ) >= Target_formants_decibel_threshold) && 
            (targetFormantCenterFreqs[i] >= Target_formants_low_frequency_boundary) &&
            (targetFormantCenterFreqs[i] <= Target_formants_high_frequency_boundary) &&
            (targetFormantCenterFreqs[i] <= nyquist) &&
            (targetFormantCenterFreqs[i] > 0.)
	  ){
	    targetFormantCenterFreqs[k] = targetFormantCenterFreqs[i] ; 
	    targetFormantAmps[k] = targetFormantAmps[i] ; 
	    targetFormantBWs[k] = targetFormantBWs[i] ;
	    targetFormantQs[k] = targetFormantQs[i] ;
	    targetFormantIndices[k] = targetFormantIndices[i] ;
	    targetFormantLowStopBandIndices[k] = targetFormantLowStopBandIndices[i] ;
	    targetFormantHighStopBandIndices[k] = targetFormantHighStopBandIndices[i] ;
          k++ ;            

        } ; 

    } ;     

    if( k == 0 ){
         prt( "\n\nERROR: -------> NO FORMANTS REMAIN FOLLOWING FILTERING OF TARGET FORMANTS <--------\n\n . . . . . . . BYE.\n\n" ); 
	   exit(EXIT_FAILURE); 
    }; 




    if( k != targetNumFormants ){
        targetNumFormants = k ; 
	  // PRINT TARGET DATA
	  fprintf( stderr, "\n\n** FILTERED TARGET FORMANT DATA **:\n" ) ; 
	  prt( "\n\n" ) ; 
	  fprintf( stderr, 
		"Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\n" 
	  ) ; 
	  for( i = 0; i < targetNumFormants; i++){
	      fprintf( stderr, 
 	           "%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\n",
        	(i + 1), 
		(int) targetFormantCenterFreqs[i], 
		(int) amp_to_dB( targetFormantAmps[i] ),
		(int) targetFormantBWs[i], 
		(int)targetFormantQs[i],
		targetFormantIndices[i],
		targetFormantLowStopBandIndices[i],
		targetFormantHighStopBandIndices[i]
    	      ) ; 
	  } ; 
    } ; 
} ; 

// ** BEGIN OF NEW TARGET EXTEND
// ******


if( extendTargetFormants == 1 ){
// EXTEND NUMBER OF TARGET FORMANTS USING OVERTONES OF FORMANTS
// FIRST FIND OUT HOW MANY MORE THERE WILL BE.

    for(l = 0; l < 2; l++){
        extendedTargetNumFormants = targetNumFormants ; 
        for(i = 0; i < targetNumFormants; i++){
			// TEST FOR SUITABLE AMPLITUDE IN BASE FORMANT
            if( targetFormantAmps[i] > dB_to_amp( TargetFormantExtensionDbThreshold ) ){
			// START WITH PARTIAL 2
                 partialNumber = 2. ;  thisOctave = 1. ; thisPartialNumber = (int)(partialNumber * thisOctave) ;
			proposedPartialFreq = targetFormantCenterFreqs[i] * thisPartialNumber ; 
			proposedPartialAmp = targetFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;                

			// WHILE PROPOSED PARTIAL FREQ IS BELOW THE NYQ AND PARTIAL NUMBER IS BELOW THE LIMIT OR ALL ARE INCLUDED, THEN
                while( (proposedPartialFreq < nyquist) && 
				((partialNumber <= peakTargetExtendPartial) || (peakTargetExtendPartial == 0.)) 
//					&& (proposedPartialAmp >= dB_to_amp( TargetFormantExtensionDbThreshold )) 		
				){
                    
				// WHILE PROPOSED (OCTAVE-ADDED) PARTIAL FREQ IS STILL BELOW THE NYQUIST
                    proposedPartialFreq = targetFormantCenterFreqs[i] * (float) thisPartialNumber ; 
                    proposedPartialAmp = targetFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;

                    while( (proposedPartialFreq < nyquist)  
//					&& (proposedPartialAmp >= dB_to_amp( TargetFormantExtensionDbThreshold )) 
				){
                                         
                        if(l == 1) {
						extendedTargetFormantCenterFreqs[extendedTargetNumFormants] = proposedPartialFreq ; // SAVE CENTER FREQ
						extendedTargetFormantBWs[extendedTargetNumFormants] = targetFormantBWs[i] ; // SAVE BW
						extendedTargetFormantQs[extendedTargetNumFormants] = 
										proposedPartialFreq / targetFormantBWs[i]  ; // SAVE Q
						// SAVE AMP AS PARTIAL-CORRELATED PROPORTION OF FUNDAMENTAL AMP. 
                            	extendedTargetFormantAmps[extendedTargetNumFormants] = targetFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;

                                 extendedTargetFormantPartial[extendedTargetNumFormants] = thisPartialNumber ;   

                        } ;  
                        extendedTargetNumFormants++ ;  

					// IF OCTAVE ADD IS ON, THEN DOUBLE FREQ; OTHERWISE, SET TO NYQUIST IN ORDER TO EXIT LOOP
                        thisOctave = thisOctave * 2. ; thisPartialNumber = (int)(partialNumber * thisOctave) ;
                        proposedPartialFreq = (AddOctavesToTargetExtensionPartials == 1) ? 
					targetFormantCenterFreqs[i] * (float) thisPartialNumber : nyquist ;
                        proposedPartialAmp = targetFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ; 

                    } ; 

                    partialNumber += 1. ; thisOctave = 1. ; thisPartialNumber = (int)(partialNumber * thisOctave) ;
                    proposedPartialFreq = targetFormantCenterFreqs[i] * thisPartialNumber ;
                    proposedPartialAmp = targetFormantAmps[i] * 
							dB_to_amp( added_formants_partial_dB_rolloff_per_partial * (thisPartialNumber - 1.) ) ;


                }; 
            } ; 
        } ; 

        pri( targetNumFormants, "targetNumFormants" ) ; 
        pri( extendedTargetNumFormants, "extendedTargetNumFormants" ) ; 


	if(l == 0) {
		fvec( extendedTargetFormantCenterFreqs, extendedTargetNumFormants ); 
		fvec( extendedTargetFormantAmps, extendedTargetNumFormants ); 
		fvec( extendedTargetFormantBWs,  extendedTargetNumFormants ) ; 
		fvec( extendedTargetFormantQs,  extendedTargetNumFormants ) ; 
		ivec( extendedTargetFormantIndices,  extendedTargetNumFormants ) ; 
		ivec( extendedTargetFormantLowStopBandIndices,  extendedTargetNumFormants ) ; 
		ivec( extendedTargetFormantHighStopBandIndices,  extendedTargetNumFormants ) ; 
           ivec( extendedTargetFormantPartial, extendedTargetNumFormants ) ; 


		for(n = 0; n < extendedTargetNumFormants; n++){
			extendedTargetFormantCenterFreqs[n] = -1. ; 
			extendedTargetFormantAmps[n] = -1. ; 
			extendedTargetFormantBWs[n] = -1. ; 
			extendedTargetFormantQs[n] = -1. ; 
			extendedTargetFormantIndices[n] = -1 ; 
			extendedTargetFormantLowStopBandIndices[n] = -1 ; 
			extendedTargetFormantHighStopBandIndices[n] = -1 ;
			extendedTargetFormantPartial[n] = -1 ; 
		} ; 

           // COPY EXISTING FREQS AND AMPS OF FORMANTS TO NEW ARRAYS.
           for(i = 0; i < targetNumFormants; i++){
		    extendedTargetFormantCenterFreqs[i] = targetFormantCenterFreqs[i] ; 
		    extendedTargetFormantAmps[i] = targetFormantAmps[i] ; 
		    extendedTargetFormantBWs[i] = targetFormantBWs[i] ; 
		    extendedTargetFormantQs[i] = targetFormantQs[i] ; 
		    extendedTargetFormantIndices[i] = targetFormantIndices[i] ; 
		    extendedTargetFormantLowStopBandIndices[i] = targetFormantLowStopBandIndices[i] ; 
		    extendedTargetFormantHighStopBandIndices[i] = targetFormantHighStopBandIndices[i] ; 
		    extendedTargetFormantPartial[i] = 1 ; 

           } ; 


	} ; 





    }; 

/*
prt( "A\n" ); 
for(i = 0; i < extendedTargetNumFormants; i++)
    fprintf( stderr, "%d) FREQ: %f\tDB: %f\n",  (i + 1), extendedTargetFormantCenterFreqs[i], amp_to_dB( extendedTargetFormantAmps[i] ) ) ; 
*/


    // SORT DATA BY FREQS.    
    n = 0 ; // UNSORTED
    while( n == 0 ){
        n = 1 ; 
        for(i = 0; i < extendedTargetNumFormants - 1; i++){
            if( extendedTargetFormantCenterFreqs[i] > extendedTargetFormantCenterFreqs[i + 1] ){
                // SWITCH AND FLAG
                temp = extendedTargetFormantCenterFreqs[i] ; 
                extendedTargetFormantCenterFreqs[i] = extendedTargetFormantCenterFreqs[i + 1] ; 
                extendedTargetFormantCenterFreqs[i + 1] = temp ; 

                temp = extendedTargetFormantAmps[i] ; 
                extendedTargetFormantAmps[i] = extendedTargetFormantAmps[i + 1] ; 
                extendedTargetFormantAmps[i + 1] = temp ; 

                temp = extendedTargetFormantBWs[i] ; 
                extendedTargetFormantBWs[i] = extendedTargetFormantBWs[i + 1] ; 
                extendedTargetFormantBWs[i + 1] = temp ; 

                temp = extendedTargetFormantQs[i] ; 
                extendedTargetFormantQs[i] = extendedTargetFormantQs[i + 1] ; 
                extendedTargetFormantQs[i + 1] = temp ; 
			
                k = extendedTargetFormantPartial[i] ; 
                extendedTargetFormantPartial[i] = extendedTargetFormantPartial[i + 1] ; 
                extendedTargetFormantPartial[i + 1] = k ; 

                n = 0 ; 

            } ;  
        }; 
    } ; 

/*
prt( "SORTED\n" ); 
for(i = 0; i < extendedTargetNumFormants; i++)
    fprintf( stderr, "%d) FREQ: %f\tDB: %f\n",  (i + 1), extendedTargetFormantCenterFreqs[i], amp_to_dB( extendedTargetFormantAmps[i] ) ) ; 
*/
	// REMOVE THE LESSER OF OVERLAPPING ADDED FORMANTS (PARTIAL 2 OR GREATER)
    k = 0 ; 
    for( i = 0; i < extendedTargetNumFormants; i++){
         
        if( i == 0 ){
            // FIRST -- TRANSFER IT
            transferCode = 1 ; // 0: no transfer, 	1: add i to list at k, incr k		2: replace (k - 1) with i; (no incr of k) 
        }else{
            if( (extendedTargetFormantPartial[k - 1] + extendedTargetFormantPartial[i]) > 2 ){
		     // ONE OF THE TWO, OR BOTH, IS AN ADDED PARTIAL
 		     // CHECK FOR FORMANT BW OVERLAP
                highStopBandFreqOfLower = extendedTargetFormantCenterFreqs[k - 1] + (0.5 * extendedTargetFormantBWs[k - 1]) ;  
                lowStopBandFreqOfLower = extendedTargetFormantCenterFreqs[i] - (0.5 * extendedTargetFormantBWs[i]) ;  
                if( highStopBandFreqOfLower > lowStopBandFreqOfLower ){
                    // OVERLAP -- OMIT ONE
                    // CHECK FOR ORIGINAL; IF SO, KEEP ORIGINAL
                   if( (extendedTargetFormantPartial[k - 1] == 1) && (extendedTargetFormantPartial[i] != 1) ){
                        // KEEP OLD AND ADD NOTHING
                       transferCode = 0 ; 
                   }else if( (extendedTargetFormantPartial[k - 1] != 1) && (extendedTargetFormantPartial[i] == 1) ){
                        // PUT NEW (AN ORIGINAL FORMANT) IN PLACE OF (k - 1)
                        transferCode = 2 ; 
                   }else{
                        // BOTH ARE SYNTHETIC, NON-FUNDAMENTAL PARTIALS; 
                        // KEEP THE ONE WITH THE STRONGER (THEORETICAL) AMP.
                        if( extendedTargetFormantAmps[k - 1] > extendedTargetFormantAmps[i] ){
                             // LAST IS STRONGER; ADD NOTHING
                             transferCode = 0 ; 
                        }else{
                             // NEW SYNTHETIC IS STRONGER; PUT NEW IN PLACE OF (k - 1).
                             transferCode = 2 ;
                        } ;  

                   } ; 

                }else{
                    // NO OVERLAP; ADD NEW TO LIST
                    transferCode = 1 ;
                } ; 


            }else{
                // BOTH ARE ORIGINAL FORMANTS -- TRANSFER NEW INTO LIST
                transferCode = 1 ;
            } ; 

        } ; 

        // TRANSFER OR REPLACE
        if( transferCode == 1 ){
            m1 = k ; k++ ;    
        }else if( transferCode == 2 ){
            m1 = k - 1 ;
        } ; 

        if( (transferCode == 1) || (transferCode == 2) ){
		    extendedTargetFormantCenterFreqs[m1] = extendedTargetFormantCenterFreqs[i] ; 
		    extendedTargetFormantAmps[m1] = extendedTargetFormantAmps[i] ; 
		    extendedTargetFormantBWs[m1] = extendedTargetFormantBWs[i] ; 
		    extendedTargetFormantQs[m1] = extendedTargetFormantQs[i] ; 
		    extendedTargetFormantPartial[m1] = extendedTargetFormantPartial[i] ; 
        

        } ; 


    } ; 

    extendedTargetNumFormants = k ; 

	// PRINT TARGET DATA
    fprintf( stderr, "\n\n** EXTENDED TARGET FORMANT DATA WITH ADDED HARMONIC PARTIALS **:\n" ) ; 
    fprintf( stderr, "(%d Analysis Bins)\n", targetN2 ) ; 
    prt( "\n\n" ) ; 
    fprintf( stderr, 
	  "Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\tPartial\n" 
    ) ; 
    for( i = 0; i < extendedTargetNumFormants; i++){
        if( extendedTargetFormantPartial[i] > 1 ){
    		   fprintf( stderr, 
            	"%i.added\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\t\t\t%i\n",
    			(i + 1), 
			(int) extendedTargetFormantCenterFreqs[i], 
			(int) amp_to_dB( extendedTargetFormantAmps[i] ),
			(int) extendedTargetFormantBWs[i], 
			(int)extendedTargetFormantQs[i],
			extendedTargetFormantIndices[i],
			extendedTargetFormantLowStopBandIndices[i],
			extendedTargetFormantHighStopBandIndices[i],
			extendedTargetFormantPartial[i]
    		  ) ; 
        }else{
    		 fprintf( stderr, 
            	"%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\t\t\t%i\n",
    			(i + 1), 
			(int) extendedTargetFormantCenterFreqs[i], 
			(int) amp_to_dB( extendedTargetFormantAmps[i] ),
			(int) extendedTargetFormantBWs[i], 
			(int)extendedTargetFormantQs[i],
			extendedTargetFormantIndices[i],
			extendedTargetFormantLowStopBandIndices[i],
			extendedTargetFormantHighStopBandIndices[i],
			extendedTargetFormantPartial[i]
    		 ) ; 
       }; 
    } ; 





}else{
    extendedTargetNumFormants = targetNumFormants ; 
    fvec( extendedTargetFormantCenterFreqs, extendedTargetNumFormants ); 
    fvec( extendedTargetFormantAmps, extendedTargetNumFormants ); 
    fvec( extendedTargetFormantBWs,  extendedTargetNumFormants ) ; 
    fvec( extendedTargetFormantQs,  extendedTargetNumFormants ) ; 
    ivec( extendedTargetFormantPartial, extendedTargetNumFormants ) ; 
    ivec( extendedTargetFormantIndices,  extendedTargetNumFormants ) ; 
    ivec( extendedTargetFormantLowStopBandIndices,  extendedTargetNumFormants ) ; 
    ivec( extendedTargetFormantHighStopBandIndices,  extendedTargetNumFormants ) ; 


    // COPY EXISTING FREQS AND AMPS FORMANTS TO NEW ARRAYS.
    for(i = 0; i < extendedTargetNumFormants; i++){
        extendedTargetFormantCenterFreqs[i] = targetFormantCenterFreqs[i] ; 
        extendedTargetFormantAmps[i] = targetFormantAmps[i] ; 
        extendedTargetFormantBWs[i] = targetFormantBWs[i] ; 
        extendedTargetFormantQs[i] = targetFormantQs[i] ; 
        extendedTargetFormantPartial[i] = 1 ; 
        extendedTargetFormantIndices[i] = targetFormantIndices[i] ; 
        extendedTargetFormantLowStopBandIndices[i] = targetFormantLowStopBandIndices[i] ; 
        extendedTargetFormantHighStopBandIndices[i] = targetFormantHighStopBandIndices[i] ; 
    } ; 
} ; 


// *** END OF NEW TARGET EXTEND

/*

if( extendTargetFormants == 1 ){
// EXTEND NUMBER OF TARGET FORMANTS USING OVERTONES OF FORMANTS
// FIRST FIND OUT HOW MANY MORE THERE WILL BE.

    for(l = 0; l < 2; l++){
        extendedTargetNumFormants = targetNumFormants ; 
        for(i = 0; i < targetNumFormants; i++){
            if( targetFormantAmps[i] > dB_to_amp( TargetFormantExtensionDbThreshold ) ){
                partialNumber = 2. ;  proposedPartialFreq = targetFormantCenterFreqs[i] * partialNumber ; 
                
                while( (proposedPartialFreq < nyquist) && 
				((partialNumber <= peakTargetExtendPartial) || (peakTargetExtendPartial == 0.)) ) {
                    
                    while( proposedPartialFreq < nyquist ){
                                         
                        // IF SECOND TIME THROUGH, THEN CHECK IF THIS IS A DUPLICATE.
                        duplicateFlag = 0 ; 
                        if(l == 1){
                            for(k = 0; k < extendedTargetNumFormants; k++){
                                if( fabs( proposedPartialFreq - extendedTargetFormantCenterFreqs[k] ) < (3. * fundamental) ) duplicateFlag = 1 ; 
                            } ; 

                        } ; 
                        if( duplicateFlag == 0 ){
                            if(l == 1) {
						extendedTargetFormantCenterFreqs[extendedTargetNumFormants] = proposedPartialFreq ; 
                            	extendedTargetFormantAmps[extendedTargetNumFormants] = targetFormantAmps[i] * -1. ; 
					 } ; 
                            extendedTargetNumFormants++ ;  
                        } ; 
                        proposedPartialFreq = (AddOctavesToTargetExtensionPartials == 1) ? proposedPartialFreq * 2. : nyquist ; 

                    } ; 

                    partialNumber += 1. ; 
                    proposedPartialFreq = targetFormantCenterFreqs[i] * partialNumber ;


                }; 
            } ; 
        } ; 

//        pri( targetNumFormants, "targetNumFormants" ) ; 
//        pri( extendedTargetNumFormants, "extendedTargetNumFormants" ) ; 

	if(l == 0) {
		fvec( extendedTargetFormantCenterFreqs, extendedTargetNumFormants ); 
		fvec( extendedTargetFormantAmps, extendedTargetNumFormants ); 
		fvec( extendedTargetFormantBWs,  extendedTargetNumFormants ) ; 
		fvec( extendedTargetFormantQs,  extendedTargetNumFormants ) ; 
		ivec( extendedTargetFormantIndices,  extendedTargetNumFormants ) ; 
		ivec( extendedTargetFormantLowStopBandIndices,  extendedTargetNumFormants ) ; 
		ivec( extendedTargetFormantHighStopBandIndices,  extendedTargetNumFormants ) ; 

		for(n = 0; n < extendedTargetNumFormants; n++){
			extendedTargetFormantCenterFreqs[n] = -1. ; 
			extendedTargetFormantAmps[n] = -1. ; 
			extendedTargetFormantBWs[n] = -1. ; 
			extendedTargetFormantQs[n] = -1. ; 
			extendedTargetFormantIndices[n] = -1 ; 
			extendedTargetFormantLowStopBandIndices[n] = -1 ; 
			extendedTargetFormantHighStopBandIndices[n] = -1 ; 
		} ; 


	} ; 




        // COPY EXISTING FREQS AND AMPS FORMANTS TO NEW ARRAYS.
        if(l == 0) for(i = 0; i < targetNumFormants; i++){
		extendedTargetFormantCenterFreqs[i] = targetFormantCenterFreqs[i] ; 
		extendedTargetFormantAmps[i] = targetFormantAmps[i] ; 
		extendedTargetFormantBWs[i] = targetFormantBWs[i] ; 
		extendedTargetFormantQs[i] = targetFormantQs[i] ; 
		extendedTargetFormantIndices[i] = targetFormantIndices[i] ; 
		extendedTargetFormantLowStopBandIndices[i] = targetFormantLowStopBandIndices[i] ; 
		extendedTargetFormantHighStopBandIndices[i] = targetFormantHighStopBandIndices[i] ; 
        } ; 

    }; 

//
// prt( "A\n" ); 
// for(i = 0; i < extendedTargetNumFormants; i++)
//     fprintf( stderr, "%d) FREQ: %f\tDB: %f\n",  (i + 1), extendedTargetFormantCenterFreqs[i], amp_to_dB( extendedTargetFormantAmps[i] ) ) ; 
//


    // SORT FREQS WITH PAIRED AMPS.    
    n = 0 ; // UNSORTED
    while( n == 0 ){
        n = 1 ; 
        for(i = 0; i < extendedTargetNumFormants - 1; i++){
            if( extendedTargetFormantCenterFreqs[i] > extendedTargetFormantCenterFreqs[i + 1] ){
                // SWITCH AND FLAG
                temp = extendedTargetFormantCenterFreqs[i] ; 
                extendedTargetFormantCenterFreqs[i] = extendedTargetFormantCenterFreqs[i + 1] ; 
                extendedTargetFormantCenterFreqs[i + 1] = temp ; 

                temp = extendedTargetFormantAmps[i] ; 
                extendedTargetFormantAmps[i] = extendedTargetFormantAmps[i + 1] ; 
                extendedTargetFormantAmps[i + 1] = temp ; 

                temp = extendedTargetFormantBWs[i] ; 
                extendedTargetFormantBWs[i] = extendedTargetFormantBWs[i + 1] ; 
                extendedTargetFormantBWs[i + 1] = temp ; 

                temp = extendedTargetFormantQs[i] ; 
                extendedTargetFormantQs[i] = extendedTargetFormantQs[i + 1] ; 
                extendedTargetFormantQs[i + 1] = temp ; 


                k = extendedTargetFormantIndices[i] ; 
                extendedTargetFormantIndices[i] = extendedTargetFormantIndices[i + 1] ; 
                extendedTargetFormantIndices[i + 1] = k ; 

                k = extendedTargetFormantLowStopBandIndices[i] ; 
                extendedTargetFormantLowStopBandIndices[i] = extendedTargetFormantLowStopBandIndices[i + 1] ; 
                extendedTargetFormantLowStopBandIndices[i + 1] = k ; 

                k = extendedTargetFormantHighStopBandIndices[i] ; 
                extendedTargetFormantHighStopBandIndices[i] = extendedTargetFormantHighStopBandIndices[i + 1] ; 
                extendedTargetFormantHighStopBandIndices[i + 1] = k ; 


                n = 0 ; 

            } ;  
        }; 
    } ; 

//
//prt( "SORTED\n" ); 
//for(i = 0; i < extendedTargetNumFormants; i++)
//    fprintf( stderr, "%d) FREQ: %f\tDB: %f\n",  (i + 1), extendedTargetFormantCenterFreqs[i], amp_to_dB( extendedTargetFormantAmps[i] ) ) ; 
//



    // MAKE MISSING AMPS USING INTERPOLATION, IF AVAILABLE.
    for(i = 1; i < extendedTargetNumFormants; i++){
        if( extendedTargetFormantAmps[i] < 0. ){
            // FIND LOWER AMP AND FREQ, IF IT EXISTS
            n = 0 ; lowerAmp = -1. ; 
            while( ( targetFormantCenterFreqs_ORIGINAL[n] <= extendedTargetFormantCenterFreqs[i] ) && (n <= (targetNumFormants_ORIGINAL - 1)) ){
                lowerAmp = targetFormantAmps_ORIGINAL[n] ; lowerFreq = targetFormantCenterFreqs_ORIGINAL[n] ; 
                n++ ;  
            } ;  

             // FIND UPPER AMP AND FREQ, IF IT EXISTS
            n = targetNumFormants_ORIGINAL - 1 ; upperAmp = -1. ; 
            while( ( targetFormantCenterFreqs_ORIGINAL[n] >= extendedTargetFormantCenterFreqs[i] ) && (n >= 0) ){
                upperAmp = targetFormantAmps_ORIGINAL[n] ; upperFreq = targetFormantCenterFreqs_ORIGINAL[n] ; 
                n-- ;  
            } ;  

            if( lowerAmp == -1. ){
		    temp = upperAmp ; 
		}else if(upperAmp == -1.){
		    temp = lowerAmp ; 
		}else{
                temp = (upperFreq - lowerFreq) != 0. ?
                    lowerAmp + 
                    ( 
                       (  
                            (extendedTargetFormantCenterFreqs[i] - lowerFreq) / 
                            (upperFreq - lowerFreq)
                       )  *  
                       (upperAmp - lowerAmp)
                    ) : lowerAmp ; 

            }; 

            extendedTargetFormantAmps[i] = temp ; // * (fabs( extendedTargetFormantAmps[i] )) ; 

        } ; 

    } ; 



//
//    prt( "DONE\n" ); 
//    for(i = 0; i < extendedTargetNumFormants; i++)
//        fprintf( stderr, "%d) FREQ: %f\tDB: %f\n",  (i + 1), extendedTargetFormantCenterFreqs[i], amp_to_dB( extendedTargetFormantAmps[i] ) ) ; 
//


    // PRINT TARGET DATA
    fprintf( stderr, "\n\n** TARGET FORMANT DATA WITH ADDED HARMONIC PARTIALS **:\n" ) ; 
    prt( "\n\n" ) ; 
    fprintf( stderr, 
		"Formant\t\tCenter Freq\tAmplitude\tBandwidth\tQ\tCF-Index Stopband Index(Low, High)\n" 
    ) ; 
    for( i = 0; i < extendedTargetNumFormants; i++){

        if(extendedTargetFormantBWs[i] != -1. ){ 
		fprintf( stderr, 
              	"%i.\t\t%i Hz\t\t%i dB\t\t%i Hz\t\t%i Q\t%i\t%i\t%i\n",
        		(i + 1), 
			(int) extendedTargetFormantCenterFreqs[i], 
			(int) amp_to_dB( extendedTargetFormantAmps[i] ),
			(int) extendedTargetFormantBWs[i], 
			(int)extendedTargetFormantQs[i],
			extendedTargetFormantIndices[i],
			extendedTargetFormantLowStopBandIndices[i],
			extendedTargetFormantHighStopBandIndices[i]
            ) ; 
        }else{
		fprintf( stderr, 
              	"%i. (added)\t\t%i Hz\t\t%i dB\n",
        		(i + 1), 
			(int) extendedTargetFormantCenterFreqs[i], 
			(int) amp_to_dB( extendedTargetFormantAmps[i] ) 
		) ; 
        } ; 


    } ; 



}else{
    extendedTargetNumFormants = targetNumFormants ; 
    fvec( extendedTargetFormantCenterFreqs, extendedTargetNumFormants ); 
    fvec( extendedTargetFormantAmps, extendedTargetNumFormants ); 
    // COPY EXISTING FREQS AND AMPS FORMANTS TO NEW ARRAYS.
    for(i = 0; i < extendedTargetNumFormants; i++){
        extendedTargetFormantCenterFreqs[i] = targetFormantCenterFreqs[i] ; extendedTargetFormantAmps[i] = targetFormantAmps[i] ; 
    } ; 
} ; 

*/

// END OF TARGET FORMANTS EXTENSION


// DETERMINE FORMANT PAIRINGS; THE TOTAL NUMBER IS EQUAL TO THE SOURCE OR TARGET SET WITH THE 
// LARGEST NUMBER
ivec( targetToSourcePairing, extendedTargetNumFormants ) ; 
ivec( sourceToTargetPairing, extendedSourceNumFormants ) ; 

// SOURCES
for(i = 0; i <  extendedSourceNumFormants; i++){
    formantCFdiff = 999999999.0 ; 
    for(k = 0;  k <  extendedTargetNumFormants; k++){
	thisformantCFdiff = fabs( (extendedSourceFormantCenterFreqs[i] - extendedTargetFormantCenterFreqs[k]) ); 
	if(thisformantCFdiff < formantCFdiff){
	    formantCFdiff = thisformantCFdiff; 
	    j = k; 
//	    if(extendedTargetFormantCenterFreqs[k] >  extendedSourceFormantCenterFreqs[i]) j -= 1 ; 
	    if(j >= (extendedTargetNumFormants - 1)) j = extendedTargetNumFormants - 1; 
	    if(j < 0) j = 0; 
	} ;  
    } ; 
    sourceToTargetPairing[i] = j ; 



/*
    fprintf( stderr, "Source Formant %d: %d Hz -> Target Formant %d: %d Hz\n", 
	i, (int) extendedSourceFormantCenterFreqs[i], sourceToTargetPairing[i], 
		(int) extendedTargetFormantCenterFreqs[ sourceToTargetPairing[i] ] ) ;  
*/
} ;  

// TARGETS
for(i = 0; i <  extendedTargetNumFormants; i++){
    formantCFdiff = 999999999.0 ; 
    for(k = 0;  k <  extendedSourceNumFormants; k++){
	// CLOSEST ABOVE OR BELOW
	thisformantCFdiff = fabs( extendedTargetFormantCenterFreqs[i] - extendedSourceFormantCenterFreqs[k] ); 
	if(thisformantCFdiff < formantCFdiff){
	    formantCFdiff = thisformantCFdiff; 
	    j = k; 
//	    if(extendedSourceFormantCenterFreqs[k] < extendedTargetFormantCenterFreqs[i]) j += 1 ; 
	    if(j >= (extendedSourceNumFormants - 1)) j = extendedSourceNumFormants - 1; 
	    if(j < 0) j = 0; 
	} ;  

    } ; 
    targetToSourcePairing[i] = j ; 

/*
    fprintf( stderr, "Target Formant %d: %d Hz -> Source Formant %d: %d Hz\n", 
	i, (int) extendedTargetFormantCenterFreqs[i], targetToSourcePairing[i], 
		(int) extendedSourceFormantCenterFreqs[ targetToSourcePairing[i] ] ) ;  
*/


} ;  

// MAKE SOURCE BIN FORMANT BOUNDARY INDICES FROM CENTER FREQ AND BANDWIDTH.
ivec( sourceFormantCenterBin, extendedSourceNumFormants ) ; 
ivec( sourceFormantLowBin, extendedSourceNumFormants ) ; 
ivec( sourceFormantHighBin, extendedSourceNumFormants ) ; 
for(i = 0; i < extendedSourceNumFormants; i++){
	  sourceFormantCenterBin[i] = extendedSourceFormantIndices[i] ;  
        sourceFormantLowBin[i] = extendedSourceFormantLowStopBandIndices[i] ; 
        sourceFormantHighBin[i] = extendedSourceFormantHighStopBandIndices[i] ; 


//fprintf( stderr, "A. SOURCE FORMANT %d: -- LOW BIN: %d\t HIGH BIN: %d\n",
//		i, sourceFormantLowBin[i], sourceFormantHighBin[i] );


    if( sourceFormantLowBin[i] < 0) { sourceFormantLowBin[i] = 0; prt("LOW: CHECK THIS!") ; } 
    if( sourceFormantHighBin[i] > (N2 - 1)) { sourceFormantHighBin[i] = N2 - 1;  prt("HIGH: CHECK THIS!") ; }
	

}; 



//for(i = 0; i < extendedSourceNumFormants; i++)
//    fprintf( stderr, "B. SOURCE FORMANT %d: -- LOW BIN: %d\t HIGH BIN: %d\n",
//		i, sourceFormantLowBin[i], sourceFormantHighBin[i] ); 


//pri( extendedSourceNumFormants, "extendedSourceNumFormants" ) ; 
//pri( extendedTargetNumFormants, "extendedTargetNumFormants" ) ; 

if(extendedSourceNumFormants > extendedTargetNumFormants){
    outputNumFormants = extendedSourceNumFormants ; 
    ivec( outputSourceFormants, outputNumFormants ); ivec( outputTargetFormants, outputNumFormants ) ; 
    fvec( SourceAmpSumOfSourceFormantsUsingThisSourceFormant, outputNumFormants ) ; 
	fvec( SourceAmpSumOfTargetFormantsUsingThisSourceFormant, outputNumFormants ) ; 
    for(i = 0; i < outputNumFormants; i++){
	outputSourceFormants[i] = i; outputTargetFormants[i] = sourceToTargetPairing[ i ]; 
    } ; 
}else{
    outputNumFormants = extendedTargetNumFormants ; 
    ivec( outputSourceFormants, outputNumFormants ); ivec( outputTargetFormants, outputNumFormants ) ; 
    fvec( SourceAmpSumOfSourceFormantsUsingThisSourceFormant, outputNumFormants ) ; 
	fvec( SourceAmpSumOfTargetFormantsUsingThisSourceFormant, outputNumFormants ) ; 
    for(i = 0; i < outputNumFormants; i++){
	outputSourceFormants[i] = targetToSourcePairing[ i ] ; outputTargetFormants[i] = i; 
    } ; 
}; 

pri( outputNumFormants, "outputNumFormants" ) ;  

// SUM UP THE AMPS FOR THE TIMES THE SOURCE FORMANT IS DUPLICATED WHEN REPRESENTING THE SOURCE
// AND THEN THE TARGET.
for(i = 0; i < outputNumFormants; i++){  
	SourceAmpSumOfSourceFormantsUsingThisSourceFormant[i] = 0. ; 
	SourceAmpSumOfTargetFormantsUsingThisSourceFormant[i] = 0. ; 
	for(j = 0; j < outputNumFormants; j++){ 
		thisOutputSourceFormant = outputSourceFormants[j] ; 
		if( outputSourceFormants[i] == outputSourceFormants[j] ){
			 SourceAmpSumOfSourceFormantsUsingThisSourceFormant[i] += extendedSourceFormantAmps[ thisOutputSourceFormant ] ;
		} ; 
		if( outputTargetFormants[i] == outputTargetFormants[j] ){
			SourceAmpSumOfTargetFormantsUsingThisSourceFormant[i] += extendedSourceFormantAmps[ thisOutputSourceFormant ] ; 
		} ; 
	} ; 
} ; 

/*
for(i = 0; i < outputNumFormants; i++)
    fprintf( stderr, "OUTPUT FORMANT %d: SOURCE FORMANT AMP-SUM(dB) - %d / %f \tTARGET FORMANT AMP-SUM - %d/%f\n", 
		i, outputSourceFormants[i], SourceAmpSumOfSourceFormantsUsingThisSourceFormant[i], 
				outputTargetFormants[i], SourceAmpSumOfTargetFormantsUsingThisSourceFormant[i] ); 
*/

//pri( outputNumFormants, "outputNumFormants" ) ; 

// COMPUTE TOTAL NUMBER OF OUTPUT BINS NEEDED
//pri( outputNumBins, "BEFORE LOOP outputNumBins" ) ;

//pri( extendedSourceNumFormants, "extendedSourceNumFormants" ) ; 

// ???
for(i = 0; i < outputNumFormants; i++){
	thisOutputSourceFormant = outputSourceFormants[i] ; 
	outputNumBins += (sourceFormantHighBin[ thisOutputSourceFormant ] -
		sourceFormantLowBin[thisOutputSourceFormant] + 1); 
}; 	




//pri( outputNumBins, "AFTER LOOP outputNumBins" ) ;

// MAKE ARRAY SPACE FOR AMPS AND FREQS, AND FOR TRANSPOSE MULTIPLIER.
if( bank_A_0__banks_A_and_B_1 == 0 ){
	fvec( outputMappedChannel, (outputNumBins * 2) + 2 ); 
}else{
	fvec( outputMappedChannel, (outputNumBins * 2 * 2) + 2 ); 
} ; 
ivec( outputSourceBinIndices, outputNumBins ); 
fvec( outputMappingTransposeMultiplier, outputNumBins ) ; 
fvec( outputMappingSourcePartialMultiplier, outputNumBins ) ; 
fvec( outputMappingAmpScaler, outputNumBins ) ; 
fvec( outputBinDistanceFromCenterFreqinOctaves, outputNumBins ) ; 

fvec( outputSourceFormantDuplicateDBScaler, outputNumBins ) ; 
fvec( outputTargetFormantDuplicateDBScaler, outputNumBins ) ; 

fvec( outputInterpolationControlFormantWarp, outputNumBins ) ; 


//SET UP BIN MAPPING INDICES AND TRANSPOSE MULTIPLIERS.
k = 0; 
for(i = 0; i < outputNumFormants; i++){
    thisOutputSourceFormant = outputSourceFormants[i] ; 
    thisOutputTargetFormant = outputTargetFormants[i] ; 
    thisLowBin = sourceFormantLowBin[ thisOutputSourceFormant ] ;     
    thisHighBin = sourceFormantHighBin[ thisOutputSourceFormant ] ;     
    thisCenterBin = sourceFormantCenterBin[ thisOutputSourceFormant ] ; 


    thisOutputSourceFormantCF = extendedSourceFormantCenterFreqs[ thisOutputSourceFormant ] ; 
    thisSourcePartialMultiplier = (float) extendedSourceFormantPartial[ thisOutputSourceFormant ] ; ; 

    thisOutputTargetFormantCF = extendedTargetFormantCenterFreqs[ thisOutputTargetFormant ] ; 
    thisTransposeMultiplier =  thisOutputTargetFormantCF / thisOutputSourceFormantCF ; 


    thisOutputSourceFormantAmp = extendedSourceFormantAmps[ thisOutputSourceFormant ] ; 
    thisOutputTargetFormantAmp = extendedTargetFormantAmps[ thisOutputTargetFormant ] ; 

    thisAmpScaler = thisOutputTargetFormantAmp / thisOutputSourceFormantAmp ; 



/*
fprintf( stderr, "lowBin: %d\thighbin: %d\tsourceformant: %d\t, targetformant: %d\t, sourceformantCF: %d\ttargetformantCF: %d transposemult: %f\n", 
	thisLowBin, thisHighBin, thisOutputSourceFormant, thisOutputTargetFormant, 
		(int) thisOutputSourceFormantCF, (int) thisOutputTargetFormantCF, thisTransposeMultiplier ) ; 
*/

    thisFormantInterpolationWarp = interpolationPathDiffusion == 0 ? 0. : randf( -1. * interpolationPathDiffusion, interpolationPathDiffusion ) ; 

   thisSourceFormantDuplicateDBScaler = amp_to_dB( 
		extendedSourceFormantAmps[ thisOutputSourceFormant ] / SourceAmpSumOfSourceFormantsUsingThisSourceFormant[ thisOutputSourceFormant ] 
		) ; 
   thisTargetFormantDuplicateDBScaler = amp_to_dB( 
		extendedSourceFormantAmps[ thisOutputSourceFormant ] / SourceAmpSumOfTargetFormantsUsingThisSourceFormant[ thisOutputSourceFormant ] 
		) ; 

    thisCenterBinFreq = fundamental * (float) thisCenterBin ; 



	for(j = thisLowBin; j <= thisHighBin; j++){
		outputSourceBinIndices[ k ] = j; 
		outputMappingTransposeMultiplier[ k ] = thisTransposeMultiplier ; 
		outputMappingSourcePartialMultiplier[ k ] = thisSourcePartialMultiplier ; 

		outputSourceFormantDuplicateDBScaler[k] = thisSourceFormantDuplicateDBScaler ; 
		outputTargetFormantDuplicateDBScaler[k] = thisTargetFormantDuplicateDBScaler ; 



		outputMappingAmpScaler[ k ] = amp_to_dB( thisAmpScaler ) > amp_to_dB( amplitude_normalization_decibel_gain_limit) ? 
			dB_to_amp( amplitude_normalization_decibel_gain_limit ) : thisAmpScaler ; 

      	outputInterpolationControlFormantWarp[ k ] = thisFormantInterpolationWarp ; 


		if( j != 0){
			outputBinDistanceFromCenterFreqinOctaves[k] = 12. * fabs( 
				log10( (double) ( ((float) j * fundamental) / thisCenterBinFreq) ) / log10( 2. ) ) ; 
		}else{
			outputBinDistanceFromCenterFreqinOctaves[k] = 0. ; 

		} ; 


		k++; 
    }; 



};   

// IDENTIFY RESIDUE BINS
ivec( residueFormantBinUsage, N2 ) ; 
if( residueBinsFlag == 1 ){ 
	for(l = 0; l < 2; l++){
		channelResidueNumBins = 0 ; 
		for( i = 0; i < N2;  i++ ){
			if(l == 1) residueFormantBinUsage[i] = 0 ; 
			k = 0 ; notFound = 1 ;  
			while( (k < outputNumBins) && (notFound == 1) ){
				if( i == outputSourceBinIndices[k] ) notFound = 0 ;
				k++ ;  
			} ;  
			if( notFound == 1 ){
				if(l == 1){
					channelResidueBinNumbers[ channelResidueNumBins ] = i ; 
					residueFormantBinUsage[i] = 1 ;
				} ; 
				channelResidueNumBins++ ; 
			} ; 
		} ; 
		if( l == 0 ){
			pri( channelResidueNumBins, "NUMBER OF RESIDUE BINS" ) ; 
			if( channelResidueNumBins == 0  ){
				residueBinsFlag = 0; break ; 
			}else{
				ivec( channelResidueBinNumbers, channelResidueNumBins ) ; 
				fvec( channelResidue, channelResidueNumBins * 2 ) ; 
				residueBinsFlag = 1 ; 
			} ; 
		} ; 
	}; 
}; 



if( (release.n > 1.) || (attack.n > 1.) || (release.A[0] > 0.) || (attack.A[0] > 0.) ) smoothingFlag = 1 ; 
else smoothingFlag = 0 ; 



/*
for(i = 0; i < outputNumBins; i++){
    fprintf( stderr, "BIN: %d\toutputSourceBinIndices: %d\t outputMappingTransposeMultiplier: %f\n",
	i, outputSourceBinIndices[i], outputMappingTransposeMultiplier[i] ) ; 
}; 
*/

//***************** PRINT VALUES
prf( dur, "OUTPUT FILE: DURATION" ) ; 

prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
//pri( R,  "SAMPLE RATE" ) ; 

pri( frames_per_sec,  "FRAMES/SECOND" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 

prbanner( "RESYNTHESIS PARAMETERS",  69 ) ; 

prf( tfactor,  "TIME EXPANSION/CONTRACTION FACTOR" ) ; 
prline( 1,  "*" ) ; 
pri( I,  "      INTERPOLATION SAMPLES (samples between resynthesis frames)" ) ; 
prline( 1,  "*" ) ; 
prf( threshfacdB, "OSCILLATOR RESYNTHESIS THRESHOLD (in dB)" ) ; 
prline( 1,  "*" ) ; 
prp( &dBgain,  "GAIN (in dB)"  ) ; 
prp( &ptrans,  "PITCH TRANSPOSITION (in semitones)"  ) ; 
prp( &harmadd,  "FREQUENCY SHIFT (in Hz)"  ) ; 
prline( 1,  "*" ) ; 
if( smoothingFlag == 1){
    prt( "CARTESIAN SMOOTHING" ) ; 
    prt( "SMOOTHING IS ON" ) ; 
    prp( &attack,  "ENVELOPE ATTACK TIME (in seconds)"  ) ; 
    prp( &release,  "ENVELOPE RELEASE TIME (in seconds)"  ) ; 
}else{
	prt( "SMOOTHING IS OFF" ) ; 
} ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prp( &freqlow, "LOW SHELF FREQUENCY" ) ; 
prp( &dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prp( &freqhi, "HIGH SHELF FREQUENCY" ) ; 
prp( &dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prt( "*...........................................*" ) ;  
prline( 1,  "*" ) ; 

prf( Target_formants_decibel_threshold, "TARGET FORMANTS AMPLITUDE THRESHOLD IN DB" ) ;
prf( Target_formants_low_frequency_boundary, "TARGET FORMANTS LOW FREQUENCY BOUNDARY" ) ;
prf( Target_formants_high_frequency_boundary, "TARGET FORMANTS HIGH FREQUENCY BOUNDARY" ) ; 

prf( TargetFormantExtensionDbThreshold, "TARGETS: DECIBEL THRESHOLD FOR FORMANT OVERTONES EXTENSION" ) ;  
prf( peakTargetExtendPartial, "PEAK TARGET EXTENSION PARTIAL LIMIT" ) ; 
if( AddOctavesToTargetExtensionPartials == 1 )prt( "EXTENSION PARTIALS WILL BE AUGMENTED BY UPPER OCTAVES." ) ; 



    // *******

// SET UP ARRAYS


fvec( Wanal, Nw ) ;		/* analysis window */
fvec( Wsyn, Nw ) ;		/* synthesis window */
fvec( input, Nw ) ;		/* input buffer */
fvec( Hwin, Nw ) ;		/* plain Hamming window */
fvec( winput, Nw ) ;	/* windowed input buffer */
fvec( buffer, N ) ;		/* FFT buffer */
fvec( previous_buffer, N ) ;		// FFT buffer 
fvec( channel, N+2 ) ;	/* analysis channels */
fvec( output, Nw ) ;	/* output buffer */
fvec( previous_channel, N+2 ) ;	/* previous analysis channels */

fvec( binfreq, N2 + 1 ) ;	/* bin frequencies */

fvec( channel_freqdev,  N + 2 ) ;	// channel SORT ARRAY ACCUMULATOR


// SET UP BIN FREQUENCIES
for( i = 0; i < (N2 + 1);  i++ ) binfreq[i] = (float) i * fundamental ; 

// MAKE THRESH AMP
threshfac = pow( (double) 10.0, (double) (threshfacdB / 20.) );	


// OPEN INPUT  AND OUTPUT FILES
openfiles() ; 



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
//	convert( buffer, channel, N2, D, R ) ;

	// SETUP PREVIOUS CHANNEL
//	if( !frame_count )for(i = 0; i < (N + 2); i++) previous_channel[ i ] = channel[ i ] ; 
	// SETUP FREQDEV HISTORY ARRAY
	for( i = 1; i < (N + 2); i+= 2 ) {
	    channel_freqdev[ i - 1 ] = 0. ; channel_freqdev[ i ] = 1. ; 
	} ; 


	//*************************
	// GET THE VALUES
	//*************************

    
	bank_A_frequency_interpolation.A[ 0 ] = fval( & bank_A_frequency_interpolation, dur, t );
	bank_A_amplitude_interpolation.A[ 0 ] = fval( & bank_A_amplitude_interpolation, dur, t );
	bank_A_gain_in_decibels.A[ 0 ] = fval( & bank_A_gain_in_decibels, dur, t );
	bank_A_decibel_rolloff_per_octave.A[ 0 ] = fval( & bank_A_decibel_rolloff_per_octave, dur, t );



	bank_A_pitch_transposition_in_semitones.A[ 0 ] = fval( & bank_A_pitch_transposition_in_semitones, dur, t ) ;
 
   

	if( bank_A_0__banks_A_and_B_1 == 1 ){
		bank_B_frequency_interpolation.A[ 0 ] = fval( & bank_B_frequency_interpolation, dur, t );
		bank_B_amplitude_interpolation.A[ 0 ] = fval( & bank_B_amplitude_interpolation, dur, t );
		bank_B_gain_in_decibels.A[ 0 ] = fval( & bank_B_gain_in_decibels, dur, t );
		bank_B_decibel_rolloff_per_octave.A[ 0 ] = fval( & bank_B_decibel_rolloff_per_octave, dur, t );
		bank_B_pitch_transposition_in_semitones.A[ 0 ] = fval( & bank_B_pitch_transposition_in_semitones, dur, t );
	} ; 




	harmadd.A[ 0 ] = fval( &harmadd, dur, t );
	dBgain.A[ 0 ] = fval( &dBgain, dur, t );
	gain = dB_to_amp( dBgain.A[ 0 ] ) ; 
	ptrans.A[ 0 ] = fval( &ptrans, dur, t ) ;
	pm = semitones_to_mult( ptrans.A[ 0 ] ) ;

	attack.A[ 0 ] = fval( &attack, dur, t );
	smooth_setup( attack.A[ 0 ], &attackc, &minusattackc, IR ) ; 
	release.A[ 0 ] = fval( &release, dur, t );
	smooth_setup( release.A[ 0 ], &releasec, &minusreleasec, IR ) ; 

	dBlow.A[ 0 ] =  fval( &dBlow, dur, t );
	dBhi.A[ 0 ] =  fval( &dBhi, dur, t );
	freqlow.A[ 0 ] =  fval( &freqlow, dur, t );
	freqhi.A[ 0 ] =  fval( &freqhi, dur, t );

	//*************************
		

    if( smoothingFlag == 1 ){
	        // SETUP PREVIOUS CHANNEL
        if( !frame_count )for(i = 0; i < N; i++ ) previous_buffer[ i ] = buffer[ i ] ; 
        // SMOOTH THE CHANGES TO THE SPECTRUM
        CartesianSmooth( buffer, previous_buffer, N, attackc, minusattackc, releasec, minusreleasec ) ; 
    } ; 

	convert( buffer, channel, N2, D, R ) ;


	// CHANGE THE AMPLITUDES 

	for( i = 1, j = 0; i < (N + 2); i+= 2, j++ ){
		     
	    // ADD THIS MULTIPLIER TO THE STORE MULTIPLIERS
	    channel_freqdev[ i ] *= pm ; 
	    // ADD THIS ADDER TO THE STORE ADDERS
	    channel_freqdev[ i - 1 ] += harmadd.A[ 0 ]  ; 
	        
	        
	    // NEUTOR OUT OF BOUNDS FREQ BINS
	    temp = pm  * (harmadd.A[ 0 ]  + channel[i]) ;
	        
	    if((temp > 0.) && (temp < nyquist) ) channel[i] = temp ;
	    else channel[i - 1] = 0. ;  
	} ; 

	// ADD GAIN
//	for( i = 1; i < (N + 2); i+= 2 ) channel[i - 1] = channel[i - 1] * gain ;

	//*************EQUALIZE THE OUTPUT SPECTRUM
	eq2( channel,  (N + 2),  dBlow.A[0],  dBhi.A[0], freqlow.A[0],  freqhi.A[0],  
		fundamental, channel_freqdev, 0 ) ; 



    
	// TRANSFER SELECTED channel BINS INTO outputMappedChannel; ADD TRANSPOSE
	// SINGLE BANK: BANK A		???
	for(i = 0, j = 0; i < outputNumBins; i++, j += 2){
 
		// AMP
        ampInterpControl = curve( 0., 1., bank_A_amplitude_interpolation.A[ 0 ], outputInterpolationControlFormantWarp[i] ) ; 
        temp = 1. + (ampInterpControl * (outputMappingAmpScaler[i] - 1.) ) ; 
	  outputMappedChannel[j] = gain * dB_to_amp( 
				bank_A_gain_in_decibels.A[ 0 ] + 
				(outputBinDistanceFromCenterFreqinOctaves[i] * bank_A_decibel_rolloff_per_octave.A[0]) 
			) * 
			channel[ outputSourceBinIndices[i] * 2 ] *  ( 1.0 + (temp - 1.0) ) ;  

		// FREQ
        freqInterpControl = curve( 0., 1., bank_A_frequency_interpolation.A[ 0 ], outputInterpolationControlFormantWarp[i] ) ; 
	   outputMappedChannel[j + 1] =  semitones_to_mult( bank_A_pitch_transposition_in_semitones.A[ 0 ] ) * 
			channel[ (outputSourceBinIndices[i] * 2) + 1 ] * 	
				( 1.0 + ( freqInterpControl * (outputMappingTransposeMultiplier[i] - 1.0) ) ) ;  
	};  		

    if( bank_A_0__banks_A_and_B_1 == 1 ){
		// DOUBLE BANK: ADD BANK B
	    for(i = 0, j = 0; i < outputNumBins; i++, j += 2){
 
		// AMP
            ampInterpControl = curve( 0., 1., bank_B_amplitude_interpolation.A[ 0 ], outputInterpolationControlFormantWarp[i] ) ; 
            temp = 1. + (ampInterpControl * (outputMappingAmpScaler[i] - 1.) ) ; 
	      outputMappedChannel[(outputNumBins * 2) + j] = gain * dB_to_amp( 
				bank_B_gain_in_decibels.A[ 0 ] + 
				(outputBinDistanceFromCenterFreqinOctaves[i] * bank_B_decibel_rolloff_per_octave.A[0]) 
			) * 
					 channel[ outputSourceBinIndices[i] * 2 ] *  ( 1.0 + (temp - 1.0) ) ;  

		// FREQ
            freqInterpControl = curve( 0., 1., bank_B_frequency_interpolation.A[ 0 ], outputInterpolationControlFormantWarp[i] ) ; 
	       outputMappedChannel[(outputNumBins * 2) + j + 1] =  semitones_to_mult( bank_B_pitch_transposition_in_semitones.A[ 0 ] ) *
				channel[ (outputSourceBinIndices[i] * 2) + 1 ] * 	
		    			( 1.0 + ( freqInterpControl * (outputMappingTransposeMultiplier[i] - 1.0) ) ) ;  
	    };  		


    } ; 

	if( residueBinsFlag == 1 ){
		residue_bins_gain_in_decibels.A[ 0 ] =  fval( & residue_bins_gain_in_decibels, dur, t );
		for(i = 0, j = 0, k = 1; i < channelResidueNumBins; i++, j += 2, k += 2 ){
			channelResidue[j] = dB_to_amp( residue_bins_gain_in_decibels.A[ 0 ] ) * channel[ (channelResidueBinNumbers[i] * 2) ] ; 
			channelResidue[k] = channel[ (channelResidueBinNumbers[i] * 2) + 1 ] ; 
		} ; 
	} ; 


       synt = getthresh( outputMappedChannel, outputNumBins * 2, threshfac );

      if( bank_A_0__banks_A_and_B_1 == 0 ){
		if( residueBinsFlag == 1 ){ 
			noscbank2( outputMappedChannel, outputNumBins, R, Nw, I, P, output, channelResidue,  channelResidueNumBins ) ; 
		}else{ 
			noscbank(outputMappedChannel, outputNumBins, R, Nw, I, P, output); 
		} ; 
      }else{
	    if( residueBinsFlag == 1 ){ 
			noscbank2( outputMappedChannel, outputNumBins, R, Nw, I, P, output, channelResidue,  channelResidueNumBins ) ; 
		} else {
			noscbank(outputMappedChannel, outputNumBins * 2, R, Nw, I, P, output);
		} ; 
	}; 

	shiftout( output, Nw, I, on+Nw-I, 0 ) ;

	frame_count++ ; 


// FRAMES LOOP END

    } ; 

    // FLUSH OUT AND CLOSE OUTPUT FILE
    shiftout( output, Nw, I, 1, 1 ) ;

    
// CHANNELS LOOP END
} ; 






// CLOSE  INPUT FILE
fclose(ifd) ;  


if( harmadd.n != 1. ) fclose(harmadd.fp ) ;
if( dBgain.n != 1. ) fclose(dBgain.fp ) ;
if( ptrans.n != 1. ) fclose(ptrans.fp ) ;
if( release.n != 1. ) fclose(release.fp ) ;
if( attack.n != 1. ) fclose(attack.fp ) ;
if( warpshape.n != 1. ) fclose(warpshape.fp ) ;
if( dBlow.n != 1. ) fclose(dBlow.fp ) ;
if( dBhi.n != 1. ) fclose(dBhi.fp ) ;
if( freqlow.n != 1. ) fclose(freqlow.fp ) ;
if( freqhi.n != 1. ) fclose(freqhi.fp ) ;

fprintf(stderr,"\nFORMANTSMAPPER: RESYNTHESIS COMPLETED\n");
exit(EXIT_SUCCESS) ;
}


void usage()	
{


    fprintf(stderr, "%s",
	"formantsmapper:  generic phase vocoder with dynamic controls  \n"
	"formantsmapper   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"

	"	h:	bank_A_decibel_rolloff_per_octave (func) [0.]\n"
	"	B:	Bank B Decibel Rolloff per Octave (func) [0.]\n"
	"	r:	Bank A Frequency Interpolation (func) [0]\n"
	"	v:	Bank A Amplitude Interpolation (func) [0.]\n"
	"	G:	Bank A Gain in Decibels (func) [0.]\n"
	"	f:	bank_A_pitch_transposition_in_semitones (func) [0]\n"
	"	::	Bank B Frequency Interpolation (func) [0.]\n"
	"	J:	Bank B Amplitude Interpolation [0]\n"
	"	F:	bank_B_gain_in_decibels (func) [0.]\n"
	"	W:	Bank B Pitch Transposition in Semitones (func) [0.] \n"
	"	/:	Number of Banks -- 0: bank A, 1: banks A and B [0]\n" 
	"	~:	Amplitude Normalization Decibel Gain Limit [200]\n"
	"	Y:	Added Formants Partial dB Rolloff per Partial [0.]\n"
	"			The amplitude of extended partial formants rolls\n"
	"			off as a function of the partial number and the specified value.\n"
	"	@:	Source Formants Decibel Threshold [-200.]\n"	"	Z:	Source Formants High Frequency Boundary [Nyquist]\n"	"	z:	Source Formants Low Frequency Boundary [20.]\n"
	"			Source formants falling below the amplitude threshold or\n"
	"			outside the frequency bounds are omitted.\n"

	"	n:	Source Formants Partials Extension Switch -- 0: off, 1: on [0]\n"
	"	u:	Interpolation Path Diffusion Magnitude [0]\n"
	"			Values greater than 0 cause diffusion of the interpolation paths through\n"
	"			a variety of different curves. Higher magnitudes proportionally increase the\n"
	"			difference in curve paths.\n"  
	"	T:	Target_formants_transposition_in_semitones (func) [0]\n"
	"	K:	Target Formants Decibel Threshold [-96]\n"
	"	O:	Target Formants High Frequency Boundary [0]\n"	"	o:	Target Formants Low Frequency Boundary [0]\n"	"			Following transposition to a fixed (pre-synthesis) position\n"
	"			target formants with amplitude less than the threshold or whose \n"
	"			center frequency is outside the low/high boundaries are omitted.\n"   
	"	j:	Highest Partial for Extended Target Partials; 0: All below Nyquist [0]\n"
	"	d:	Decibel Threshold for Formant Overtones Extension [-96]\n"
	"			Formants whose amplitude is above threshold are extended.\n"
	"	c:	Extend Target Formants Switch -- 0: off 1: on [0]\n"
	"	S:	Pre Synthesis Formant Bandwidth Extension Factor [1]\n"
	"			Before synthesis, the bandwidth of formants, measured in bins, \n"
	"			can be expanded by the specified factor. A value of 0 will reduce all formants\n"
	"			to a single bin, and a value of 1 will bypass expansion.\n" 
	"	N:	FFT length (must be a power of 2) [1024]\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	w:	window type: 0 = hamming,  1 = rectangular  \n"
	"		    2 = Blackman,  3 = Bartlett triangular [0.]\n"
	"		    4-12 = Kaiser windows for alpha = 4-12,  respectively\n"
	"		    (representative sidelobe levels for alpha: \n"
	"		      4 = -30dB,  8 = -58 dB,  12 = -90 dB)\n"
	"	D:	analysis frames per second [200]\n"
	"	I:	time expansion/contraction factor  [1.] \n"
	"		  (duration = duration * factor, 1. = original time) \n"
	"	P:	pitch transposition in semitones (func) [0]\n"
	"	a:	frequency shift factor \n"
	"		    (bin frequency adder, before -P )(func) [0.] \n"
	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds ( 0. = end of file) [0.] \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"
	"	     SHELF EQ:(post transpose/shift)\n"
	"	H:	SHELF EQ: Low shelf gain in dB (func) [0.] \n"
	"	X:	SHELF EQ: High shelf gain in dB (func) [0.] \n"
	"	m:	SHELF EQ: Low shelf frequency in Hz (func) [200.] \n"
	"	R:	SHELF EQ: High shelf frequency in Hz (func) [2000.] \n"
	"	A:	gain in decibels (func) [0.] \n"
	"	l:      envelope attack time  (func) [0.]\n"
	"	L:      envelope release time   (func) [0.]\n"
	"	V:	Highest Partial for Extended Source Partials; 0: All below Nyquist [0]\n"

	"	k:	Target Partials Extension -- Octaves Switch 0: off, 1: on [0]\n"	"	y:	Source Partials Extension -- Octaves Switch 0: off, 1: on  [0]\n"

	"			Adds upper octaves to source or target partials.\n"
	"	x:	Residue Bins -- 0: off, 1: on [0]\n"
	"	s:	Residue Bins Gain in decibels (func) [0]\n"

	"	E:	Target Formants File \n"
	"	g:	Target Formants File \n"

	"	U:	Source Formant Extension Decibel Threshold  [-96.]\n"
	"			Formants whose amplitudes are greater than or equal to the threshold\n"
	"			are extended through added partials.\n"
	"	p:	amplitude reports print mode: 0 = off, 1 = on [0]\n" 
	"	i:	time interval between amplitude reports [.25]\n" 

	"	_:	 AUTO OUTPUT SOUND FILE PLAY:\n"
	"		    0  = off \n"
	"		    -1 = interactive: Prompt for each play.\n"
	"		    -2 = interactive: Play once, then prompt for more.\n"
        "                   1 or greater = Auto-repeat for specified repetitions. [0]\n" 

	"	=:	 PEAK RESCALE LEVEL 0 to -96 dB \n"
	"		    1 = Rescale to level of input file.\n" 
	"		    2 = Bypass rescaling. [ 1 ]\n" 

	"	t:	oscillator resynthesis threshold in decibels [ -96 ]\n"

	);

    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
