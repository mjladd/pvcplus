#include "globals.h"

#define DECIBEL_THRESH -200

void usage(); 
void pd( int i ) ; 


int main( argc, argv )
    int argc ; char *argv[] ;
{
float crossFadeDur, halfCrossFadeDur, thisBeginT, thisEndT ;

int i,j, k, l,  m, nnn=0, numbins,   stopbin=10,  offset, L, mixfilesNormalizationFlag ;
float binFreq, rolloffdB, rolloffAmp ; 
float nyquist,  fundamental ;
double atof();
int R=44100, N=1024, N2, Nw = 2048, Nw2, Lh = 220, Lhm1, I = 220, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  write_ascii=0 ;
float P = 1.0;
FILE *fopen(), *tdata, *impulseData, *adata ;
FILE	*impulseFilesFilePointer ;
char	impulseFilesName[ STRING_SIZE ]="", command[ 40000 ], 
	fileName[ STRING_SIZE ]="", thisEnv[ STRING_SIZE ]="",
 	outputSoundFile[ STRING_SIZE ]="", mixfilesInputFiles[ 40000 ],
     mixfilesCommand[ 40000 ], crossfadeStartTimesCommand[ 40000 ]  ;

int number_of_movement_function_points=1 ; 

char ch,  tempstring[ STRING_SIZE ],  write_ascii_filename[ STRING_SIZE ]="./ascii.out", 
    scratch[ STRING_SIZE ],  scratch2[ STRING_SIZE ],  *user,
    channelListFileName[ STRING_SIZE ] ;
float *Hwin, *Wanal, *Wsyn, *input, *winput, 
    *inbuffer, *outbuffer, *inputSave, *B, *channel, *output, *impulseAnalysis ;
float threshfac = .001,  threshfacdB=-96.;
float  *binfreq,  dur ;
float  gain, f ;
float  *previous_channel, *channel_freqdev ;
float  temp, temp1,  temp2,  pm  ;  
float getthresh();
struct func IRBPF_LowFreqPoint ;
struct func IRBPF_HighFreqPoint ; 
//struct func outputLowFreqPoint ;  
//struct func outputHighFreqPoint ; 
struct func IRBPF_lowFilterRolloffInDecibelsPerOctave ;
struct func IRBPF_highFilterRolloffInDecibelsPerOctave ; 

struct func SourceInputBPF_LowFreqPoint ;
struct func SourceInputBPF_HighFreqPoint ; 
struct func SourceInputBPF_lowFilterRolloffInDecibelsPerOctave ;
struct func SourceInputBPF_highFilterRolloffInDecibelsPerOctave ;


float  phasediff ; 
int showme=0 ; 
float impulseDuration=1. ; 
float impHyp, sigHyp, sigHypNew, prop ; 

int impulseShapingFlag_Off_0__On_1=0 ; 

float warpshape=0. ; 

int deconvolution_0__convolution_1=1 ; 

float real, imag ; 
// SHELF EQ
int add_ring_time__off_0__on_1=0 ; 

int numberOfImpulseChannels, impulse_SR, impulseChannel=0, impulseChannelNow, sampsRead ; 

char outputDirectory[ STRING_SIZE ]="" ;


int frameBeginSampNow=0 ; 

int lowbin=0,  highbin=-1,  numberframes=0,  n=0 ; 
float lowbinfreq=0,  highbinfreq=-1 ; 
int filttype=0 ; 

int multichannel_output_mode__standard_0__alternate_1=0 ;

// SOURCE GAIN
struct  func  sourceGainIndB ; 


// CONVOLUTION OUTPUT GAIN
struct  func  Convolution_output_gain_in_decibels ; 


struct  func  peak_shaping_amplitude_in_dB ; 
struct  func  shaping_breakpoint_in_dB ; 
struct  func  base_shaping_amplitude_in_dB ; 
struct  func  upper_shaping_curve_index ; 
struct  func  lower_shaping_curve_index ; 


//*****************INITIALIZE
IRBPF_LowFreqPoint.L = 1. ;  IRBPF_LowFreqPoint.n = 1. ;
    IRBPF_LowFreqPoint.A[ 0 ] = 0. ; 

IRBPF_HighFreqPoint.L = 1. ;  IRBPF_HighFreqPoint.n = 1. ;
    IRBPF_HighFreqPoint.A[ 0 ] = 0. ;

//outputLowFreqPoint.L = 1. ;  outputLowFreqPoint.n = 1. ;
//    outputLowFreqPoint.A[ 0 ] = 0. ;

//outputHighFreqPoint.L = 1. ;  outputHighFreqPoint.n = 1. ;
//    outputHighFreqPoint.A[ 0 ] = 0. ;

IRBPF_lowFilterRolloffInDecibelsPerOctave.L = 1. ;  IRBPF_lowFilterRolloffInDecibelsPerOctave.n = 1. ;
    IRBPF_lowFilterRolloffInDecibelsPerOctave.A[ 0 ] = 0. ;

IRBPF_highFilterRolloffInDecibelsPerOctave.L = 1. ;  IRBPF_highFilterRolloffInDecibelsPerOctave.n = 1. ;
    IRBPF_highFilterRolloffInDecibelsPerOctave.A[ 0 ] = 0. ;

SourceInputBPF_LowFreqPoint.L = 1. ;  SourceInputBPF_LowFreqPoint.n = 1. ;
    SourceInputBPF_LowFreqPoint.A[ 0 ] = 0. ;

SourceInputBPF_HighFreqPoint.L = 1. ;  SourceInputBPF_HighFreqPoint.n = 1. ;
    SourceInputBPF_HighFreqPoint.A[ 0 ] = 0. ;

SourceInputBPF_lowFilterRolloffInDecibelsPerOctave.L = 1. ;  SourceInputBPF_lowFilterRolloffInDecibelsPerOctave.n = 1. ;
    SourceInputBPF_lowFilterRolloffInDecibelsPerOctave.A[ 0 ] = 0. ;

SourceInputBPF_highFilterRolloffInDecibelsPerOctave.L = 1. ;  SourceInputBPF_highFilterRolloffInDecibelsPerOctave.n = 1. ;
    SourceInputBPF_highFilterRolloffInDecibelsPerOctave.A[ 0 ] = 0. ;



peak_shaping_amplitude_in_dB.L = 1. ;  peak_shaping_amplitude_in_dB.n = 1. ;
    peak_shaping_amplitude_in_dB.A[ 0 ] = 0. ; 

shaping_breakpoint_in_dB.L = 1. ;  shaping_breakpoint_in_dB.n = 1. ;
    shaping_breakpoint_in_dB.A[ 0 ] = 0. ; 

base_shaping_amplitude_in_dB.L = 1. ;  base_shaping_amplitude_in_dB.n = 1. ;
    base_shaping_amplitude_in_dB.A[ 0 ] = -96. ; 

upper_shaping_curve_index.L = 1. ;  upper_shaping_curve_index.n = 1. ;
 upper_shaping_curve_index.A[ 0 ] = 0. ; 

lower_shaping_curve_index.L = 1. ;  lower_shaping_curve_index.n = 1. ;
 lower_shaping_curve_index.A[ 0 ] = 0. ; 


// SOURCE GAIN
sourceGainIndB.L = 1. ;  sourceGainIndB.n = 1. ; sourceGainIndB.A[ 0 ] = 0. ; 


// CONVOLUTION OUTPUT GAIN
Convolution_output_gain_in_decibels.L = 1. ;  Convolution_output_gain_in_decibels.n = 1. ; Convolution_output_gain_in_decibels.A[ 0 ] = 0. ; 



if( argc < 2 )usage() ; 


    while( (ch = crack( argc, argv, "=|_|a|A|b|B|C|d|D|e|E|f|F|g|G|h|H|i|I|J|l|L|M|p|P|q|r|s|S|t|v|T|x|z|Z|", 0  )) != CRACK_DONE_FLAG ) { // j m Q R  u U v V X y Y
   switch(ch) {


       case 'v':   mixfilesNormalizationFlag = 
         	(int) crackfloat( arg_option, ch ); 
         break;



       case 'I':   strcpy( outputDirectory, arg_option); 
         break;


       case 'M':  multichannel_output_mode__standard_0__alternate_1 = 
         	(int) crackfloat( arg_option, ch );
		break ; 
       case 'x': impulseShapingFlag_Off_0__On_1 = (int) crackfloat( arg_option, ch );
         	break ; 
       case 'P':   strcpy(tempstring, arg_option); 
		peak_shaping_amplitude_in_dB.fp = 
            crackstring( tempstring, & peak_shaping_amplitude_in_dB );
         break;
       case 'B':      strcpy(tempstring, arg_option);
         shaping_breakpoint_in_dB.fp = 
            crackstring( tempstring, & shaping_breakpoint_in_dB );
         break;
       case 'F':   strcpy(tempstring, arg_option);
         base_shaping_amplitude_in_dB.fp = 
            crackstring( tempstring, & base_shaping_amplitude_in_dB );
         break;
       case 'Z':   strcpy(tempstring, arg_option);
         upper_shaping_curve_index.fp = 
            crackstring( tempstring, & upper_shaping_curve_index );
         break;
       case 'z':   strcpy(tempstring, arg_option);
         lower_shaping_curve_index.fp = 
            crackstring( tempstring, & lower_shaping_curve_index );
         break;


//
       case 's':   strcpy(tempstring, arg_option);
         IRBPF_LowFreqPoint.fp = crackstring( tempstring, & IRBPF_LowFreqPoint );
         break;
       case 't':   strcpy(tempstring, arg_option);
         IRBPF_HighFreqPoint.fp = crackstring( tempstring, & IRBPF_HighFreqPoint );
         break;
 
       case 'g':   strcpy(tempstring, arg_option);
         IRBPF_lowFilterRolloffInDecibelsPerOctave.fp = 
            crackstring( tempstring, & IRBPF_lowFilterRolloffInDecibelsPerOctave );
         break;
       case 'G':   strcpy(tempstring, arg_option);
         IRBPF_highFilterRolloffInDecibelsPerOctave.fp = 
            crackstring( tempstring, & IRBPF_highFilterRolloffInDecibelsPerOctave );
         break;
 
       case 'D':   strcpy(tempstring, arg_option);
         SourceInputBPF_LowFreqPoint.fp = crackstring( tempstring, & SourceInputBPF_LowFreqPoint );
         break;
       case 'f':   strcpy(tempstring, arg_option);
         SourceInputBPF_HighFreqPoint.fp = crackstring( tempstring, & SourceInputBPF_HighFreqPoint );
         break;
 
       case 'h':   strcpy(tempstring, arg_option);
         SourceInputBPF_lowFilterRolloffInDecibelsPerOctave.fp = crackstring( tempstring, & SourceInputBPF_lowFilterRolloffInDecibelsPerOctave );
         break;
       case 'H':   strcpy(tempstring, arg_option);
         SourceInputBPF_highFilterRolloffInDecibelsPerOctave.fp = crackstring( tempstring, & SourceInputBPF_highFilterRolloffInDecibelsPerOctave );
         break;


       // **
       case 'A':   strcpy(tempstring, arg_option);
         sourceGainIndB.fp = crackstring( tempstring, &sourceGainIndB );
         break;
       case 'r':   strcpy(tempstring, arg_option);
         Convolution_output_gain_in_decibels.fp = crackstring( tempstring, &Convolution_output_gain_in_decibels );
         break;



       case 'd':   add_ring_time__off_0__on_1 = (int) crackfloat( arg_option, ch );
         break;

       case 'J':   impulseChannel = (int) crackfloat( arg_option, ch );
         break;


       case 'b':   begint = crackfloat( arg_option, ch ) ;
         break;
       case 'e':   endt = crackfloat( arg_option, ch ) ;
         break;

       case 'C':   channelout = (int) crackfloat( arg_option, ch ) ;
         break;

            case 'p':   quiet = (int) crackfloat( arg_option, ch ) ; break;
            case 'i':   ampstatinc = crackfloat( arg_option, ch ) ; break;

           case '_':   autoplayreps = (int) crackfloat( arg_option, ch ) ; break;

           case '=':   rescalev = crackfloat( arg_option, ch ) ; break;




    } 
}




prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "IMPULSE RESPONSE CONVOLUTION SEQUENCER", 69 ) ; 
prline( 69,  "-" ) ; 

// MAKE FFT IMPULSE RESPONSES 

sprintf( impulseFilesName, "%s/impulseFileNames", outputDirectory ) ; 
prs( impulseFilesName, "IMPULSE FILES NAME" ) ; 

impulseFilesFilePointer  = fopen( impulseFilesName, "r" ) ; 

fscanf( impulseFilesFilePointer, " %d ", & number_of_movement_function_points );


pri( number_of_movement_function_points, "NUMBER OF AUDIO IMPULSE RESPONSES" ) ;  


//getInputFileDataToSetOutputChannels(argc, argv) ;
setupfiles(argc, argv) ; 

prf( idur, "idur" ) ; 
prf( endt, "endt" ) ; 
prf( begint, "begint" ) ; 

if( (number_of_movement_function_points == 1) ||
     (number_of_movement_function_points == 2) ) 
   crossFadeDur = endt - begint ;
else
   crossFadeDur = 2.0 * (endt - begint) / (float) (number_of_movement_function_points + 1) ;
;
halfCrossFadeDur = 0.5 * crossFadeDur ; 

prf( crossFadeDur, "crossFadeDur" ) ; 

// INPUT ENVELOPE:
if( number_of_movement_function_points == 1 ) 
sprintf( command, 
	"gen4 -L1000 0 1.0 0   0.5 1.0 0    1.0 1.0 | reshape -t0 > /tmp/envBegin ; " 
); 
else
sprintf( command, 
	"gen4 -L1000 0 1.0 0   0.5 1.0 0    1.0 %f | reshape -t0 > /tmp/envBegin ; ", 
		dB_to_amp( -96 )
); 

sprintf( command, 
	"%s gen4 -L1000 0 %f 0   0.5 1.0 0    1.0 %f | reshape -t0 > /tmp/env ; ", 
		command , dB_to_amp( -96 ), dB_to_amp( -96 )
); 
sprintf( command, 
	"%s gen4 -L1000 0 %f 0   0.5 1.0 0    1.0 1.0 | reshape -t0 > /tmp/envEnd ; ", 
		command, dB_to_amp( -96 )
); 

system( command ) ; 

sprintf( crossfadeStartTimesCommand, "echo " ) ; 

for( i = 0 ; i < number_of_movement_function_points ; i++ )
{
   if( number_of_movement_function_points == 1 ) prop = 0. ; 
   else prop = (float) i / (float) (number_of_movement_function_points - 1) ;


   fscanf( impulseFilesFilePointer, " %s ", fileName );
   prs( fileName, "IMPULSE RESPONSE FILE NAME" ) ; 

   // MAKE FFT IMPULSE RESPONSE
   sprintf( command, "impulseresponse -b0 -e0 -P2000 -N0 -d0 -C0 %s %s.fft", 
		fileName, fileName ) ;    

   prs( command, "COMMAND" ) ; 
   system( command ) ; 

   // CONVOLVE CROSSFADE SEGMENT WITH IR FFT
   thisBeginT = begint + ((float) i * halfCrossFadeDur) ;
   thisEndT = thisBeginT + crossFadeDur ;
   prf( thisBeginT, "thisBeginT" ) ; 
   prf( thisEndT, "thisEndT" ) ; 

   sprintf( crossfadeStartTimesCommand, "%s%f ", crossfadeStartTimesCommand, 
	thisBeginT - begint ) ; 

   sourceGainIndB.A[ 0 ] = 
      fval( & sourceGainIndB, 1.0, prop ) ;

   if( i == 0 ) sprintf( thisEnv, "/tmp/envBegin" ) ; 
   else if( i == (number_of_movement_function_points - 1) ) sprintf( thisEnv, "/tmp/envEnd" ) ; 
   else sprintf( thisEnv, "/tmp/env" ) ;


   Convolution_output_gain_in_decibels.A[ 0 ] = 
      fval( & Convolution_output_gain_in_decibels, 1.0, prop ) ;

   peak_shaping_amplitude_in_dB.A[ 0 ] = 
      fval( & peak_shaping_amplitude_in_dB, 1.0, prop ) ;


   shaping_breakpoint_in_dB.A[ 0 ] = 
      fval( & shaping_breakpoint_in_dB, 1.0, prop ) ;


   base_shaping_amplitude_in_dB.A[ 0 ] = 
      fval( & base_shaping_amplitude_in_dB, 1.0, prop ) ;

   upper_shaping_curve_index.A[ 0 ] = 
      fval( & upper_shaping_curve_index, 1.0, prop ) ;

   lower_shaping_curve_index.A[ 0 ] = 
      fval( & lower_shaping_curve_index, 1.0, prop ) ;

//

   IRBPF_LowFreqPoint.A[ 0 ] = 
      fval( & IRBPF_LowFreqPoint, 1.0, prop ) ;

   IRBPF_HighFreqPoint.A[ 0 ] = 
      fval( & IRBPF_HighFreqPoint, 1.0, prop ) ;

   IRBPF_lowFilterRolloffInDecibelsPerOctave.A[ 0 ] = 
      fval( & IRBPF_lowFilterRolloffInDecibelsPerOctave, 1.0, prop ) ;

   IRBPF_highFilterRolloffInDecibelsPerOctave.A[ 0 ] = 
      fval( & IRBPF_highFilterRolloffInDecibelsPerOctave, 1.0, prop ) ;

   SourceInputBPF_LowFreqPoint.A[ 0 ] = 
      fval( & SourceInputBPF_LowFreqPoint, 1.0, prop ) ;

   SourceInputBPF_HighFreqPoint.A[ 0 ] = 
      fval( & SourceInputBPF_HighFreqPoint, 1.0, prop ) ;

   SourceInputBPF_lowFilterRolloffInDecibelsPerOctave.A[ 0 ] = 
      fval( & SourceInputBPF_lowFilterRolloffInDecibelsPerOctave, 1.0, prop ) ;

   SourceInputBPF_highFilterRolloffInDecibelsPerOctave.A[ 0 ] = 
      fval( & SourceInputBPF_highFilterRolloffInDecibelsPerOctave, 1.0, prop ) ;

//



   sprintf( outputSoundFile, "%s.IRCV.au", fileName ) ;  

   sprintf( command, "cp %s %s", fileName, outputSoundFile ) ; 
	

   sprintf( mixfilesInputFiles, "%s%s ", mixfilesInputFiles, outputSoundFile ) ; 
   system( command ) ; 
   
   sprintf( command, 
      "irconvolver -E%s.fft -a%d -J%d -A%f -q%s -r%f -s%f -t%f -g%f -G%f -D%f -f%f -h%f -H%f ", 
      fileName, deconvolution_0__convolution_1, impulseChannel, sourceGainIndB.A[ 0 ], thisEnv,
      Convolution_output_gain_in_decibels.A[ 0 ], IRBPF_LowFreqPoint.A[ 0 ], IRBPF_HighFreqPoint.A[ 0 ],
      IRBPF_lowFilterRolloffInDecibelsPerOctave.A[ 0 ], IRBPF_highFilterRolloffInDecibelsPerOctave.A[ 0 ],
      SourceInputBPF_LowFreqPoint.A[ 0 ], SourceInputBPF_HighFreqPoint.A[ 0 ], 
      SourceInputBPF_lowFilterRolloffInDecibelsPerOctave.A[ 0 ], 
      SourceInputBPF_highFilterRolloffInDecibelsPerOctave.A[ 0 ]   
 ) ; 
   sprintf( command, 
      "%s -x%d -P%f -B%f -F%f -Z%f -z%f -C%d -M%d -b%f -e%f -d%d -_%d -=%f -p%d -i%f %s %s ",
      command, 
      impulseShapingFlag_Off_0__On_1, peak_shaping_amplitude_in_dB.A[ 0 ],
      shaping_breakpoint_in_dB.A[ 0 ], base_shaping_amplitude_in_dB.A[ 0 ],
      upper_shaping_curve_index.A[ 0 ], lower_shaping_curve_index.A[ 0 ],
      channelout, multichannel_output_mode__standard_0__alternate_1,
      thisBeginT, thisEndT, 
      add_ring_time__off_0__on_1, autoplayreps, rescalev, quiet, ampstatinc,
      ifile, outputSoundFile 
   ) ;



   prs( command, "command" ) ; 

   system( command ) ; 



} ;



// MIX CONVOLVED FILES TOGETHER

sprintf( crossfadeStartTimesCommand, "%s > /tmp/starttimes", crossfadeStartTimesCommand ) ;
system(  crossfadeStartTimesCommand ) ; 

prs( mixfilesInputFiles, "mixfilesInputFiles" ) ; 

prs( ofile, "OUTPUT FILE" ) ; 

sprintf( mixfilesCommand,"mixfiles -o%s -n%d -f/tmp/starttimes %s", 
    ofile, mixfilesNormalizationFlag, mixfilesInputFiles ) ; 

prs( mixfilesCommand, "mixfilesCommand" ) ; 

system( mixfilesCommand ) ; 

fclose(  impulseFilesFilePointer ) ; 
    
    fprintf(stderr,"\nIRCONVOLVESEQUENCER: RESYNTHESIS COMPLETED\n");
    exit(EXIT_SUCCESS) ;
}


void usage()
{ //  
    fprintf(stderr, "%s",
   "irconvolvesequencer:  fast impulse response convolution  \n"
   "irconvolvesequencer   [flags] [input file] [output file]\n"
   "       Most formats accepted. Output format copied from input file.\n"
   "       (Values in brackets denote defaults.)\n"

	"	b:   "BEGIN_TIME      // begint
	"	e:   "END_TIME         // endt
	"	C:   "RESYNTHESIS_CHANNEL      // channelout

	"	I:	output directory \n"	"	J:	impulse channel [-0.]\n"	"	d:	add ring time  0 = off, 1 = on [0]\n"	"	v:	normalization code:  0, 1, 2, 3 [0]\n"


	"	M:   multi-channel output mode: standard = 0, alternate = 1 [0]\n"

	"	   Gain Controls:\n"
	"	A:   input source gain in dB, for mix with convolution output (func) [-0.]\n"
	"	r:   convolution source output gain in dB (func)[-0.]\n"

	"	** INPUT SOUND: BAND PASS FILTER\n"
	"	D:	low rolloff frequency (func) [-0.]\n"	"	f:	high rolloff frequency (func) [-0.]\n"	"	h:	low edge amplitude rolloff in decibels per octave (func) [-0.]\n"	"	H:	high edge amplitude rolloff in decibels per octave (func) [-0.]\n"

	"	   *** IMPULSE RESPONSE FILTER BAND ***\n"
	"	s:   impulse response low frequency rolloff point [0.]\n"
	"	t:   impulse response high frequency rolloff point [Nyquist frequency]\n"
	"	g:   frequency band low edge amplitude rolloff in Decibels per octave [0.]\n"
	"	G:   frequency band low edge amplitude rolloff in Decibels per octave [0.]\n"

	"	**** Impulse Response Signal Shaping:\n"
	"	x:   impulse response shaping switch off = 0, on = 1 [0]\n"
	"	P:   peak amplitude level in dB (func)[0]\n"
	"	B:   function breakpoint in dB (func) [0]\n"
	"	F:   base or floor amplitude level in dB (func) [-96]\n"
	"	Z:   upper curve shape index (func) [0]\n" 
	"	z:   lower curve shape index (func) [0]\n" 



	"	p:   "AMP_REPORTS      // quiet 
	"	i:   "AMP_REPORTS_TIME_INTERVAL   // ampstatinc 

	"	_:    "AUTO_PLAY      // autoplayreps

	"	=:    "RESCALE_LEVEL      // rescalev

   );
    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
