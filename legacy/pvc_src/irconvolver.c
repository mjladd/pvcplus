#include "globals.h"

#define DECIBEL_THRESH -200

void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j, k, L ;
float binFreq, rolloffdB, rolloffAmp ; 
float nyquist,  fundamental ;
double atof();
int R=44100, N=1024, N2, Lh = 220, Lhm1, I = 220, in, on;
int   eof = 0, obank = 0,  channelout=0;
float P = 1.0;
FILE *fopen(), *impulseData;
char ch,  tempstring[ STRING_SIZE ],  *user;
float *inbuffer, *outbuffer, *inputSave, *B, *impulseAnalysis ;

float dur ;


float  temp,  temp2;  
float getthresh();
float IRBPF_LowFreqPoint=0.,  IRBPF_HighFreqPoint=0. ; 
float IRBPF_lowFilterRolloffInDecibelsPerOctave=0.,IRBPF_highFilterRolloffInDecibelsPerOctave=0. ; 

float SourceInputBPF_LowFreqPoint=0., SourceInputBPF_HighFreqPoint=0., 
   SourceInputBPF_lowFilterRolloffInDecibelsPerOctave=0.,  SourceInputBPF_highFilterRolloffInDecibelsPerOctave=0. ;


 
 
 
 

int impulseShapingFlag_Off_0__On_1=0 ; 

 

int deconvolution_0__convolution_1=1 ; 

float real, imag ; 
// SHELF EQ
int add_ring_time__off_0__on_1=0 ; 

int numberOfImpulseChannels, impulse_SR, impulseChannel=0, impulseChannelNow, sampsRead ; 

char IMPULSE_freq_response_data_file[ STRING_SIZE ]="" ; 

int frameBeginSampNow=0 ; 

 
 
 

int multichannel_output_mode__standard_0__alternate_1=0 ;

// SOURCE GAIN
struct  func  sourceGainIndB ; 

// CONVOLUTION INPUT GAIN
struct  func  Convolution_input_gain_in_decibels ; 

// CONVOLUTION OUTPUT GAIN
struct  func  Convolution_output_gain_in_decibels ; 


struct  func  peak_shaping_amplitude_in_dB ; 
struct  func  shaping_breakpoint_in_dB ; 
struct  func  base_shaping_amplitude_in_dB ; 
struct  func  upper_shaping_curve_index ; 
struct  func  lower_shaping_curve_index ; 


//*****************INITIALIZE

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

// CONVOLUTION INPUT GAIN
Convolution_input_gain_in_decibels.L = 1. ;  Convolution_input_gain_in_decibels.n = 1. ; Convolution_input_gain_in_decibels.A[ 0 ] = 0. ; 

// CONVOLUTION OUTPUT GAIN
Convolution_output_gain_in_decibels.L = 1. ;  Convolution_output_gain_in_decibels.n = 1. ; Convolution_output_gain_in_decibels.A[ 0 ] = 0. ; 



if( argc < 2 )usage() ; 


    while( (ch = crack( argc, argv, "=|_|a|A|b|B|C|d|D|e|E|f|F|g|G|h|H|i|J|l|L|M|p|P|q|r|s|S|t|T|x|z|Z|", 0  )) != CRACK_DONE_FLAG ) { //  I j m Q R  u U v V X y Y
   switch(ch) {




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


       case 's':   IRBPF_LowFreqPoint = crackfloat( arg_option, ch );
         break;
       case 't':   IRBPF_HighFreqPoint = crackfloat( arg_option, ch );
         break;
 
       case 'g':   IRBPF_lowFilterRolloffInDecibelsPerOctave = crackfloat( arg_option, ch );
         break;
       case 'G':   IRBPF_highFilterRolloffInDecibelsPerOctave = crackfloat( arg_option, ch );
         break;
 
       // **
       case 'D':   SourceInputBPF_LowFreqPoint = crackfloat( arg_option, ch );
         break;
       case 'f':   SourceInputBPF_HighFreqPoint = crackfloat( arg_option, ch );
         break;
 
       case 'h':   SourceInputBPF_lowFilterRolloffInDecibelsPerOctave = crackfloat( arg_option, ch );
         break;
       case 'H':   SourceInputBPF_highFilterRolloffInDecibelsPerOctave = crackfloat( arg_option, ch );
         break;


       // **
       case 'A':   strcpy(tempstring, arg_option);
         sourceGainIndB.fp = crackstring( tempstring, &sourceGainIndB );
         break;
       case 'q':   strcpy(tempstring, arg_option);
         Convolution_input_gain_in_decibels.fp = crackstring( tempstring, & Convolution_input_gain_in_decibels );
         break;
       case 'r':   strcpy(tempstring, arg_option);
         Convolution_output_gain_in_decibels.fp = crackstring( tempstring, &Convolution_output_gain_in_decibels );
         break;



       case 'a':   deconvolution_0__convolution_1 = (int) crackfloat( arg_option, ch );
         break;

       case 'd':   add_ring_time__off_0__on_1 = (int) crackfloat( arg_option, ch );
         break;

       case 'J':   impulseChannel = (int) crackfloat( arg_option, ch );
         break;

       case 'E':   strcpy( IMPULSE_freq_response_data_file, arg_option); 
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
prbanner( "IMPULSE RESPONSE CONVOLVER", 69 ) ; 
prline( 69,  "-" ) ; 

/*
*/ 



// ******** IMPULSE FREQ RESPONSE FILE SETUPS
   impulseData = fopen(  IMPULSE_freq_response_data_file, "r" ); 

   fread( &numberOfImpulseChannels, sizeof(int) , 1, impulseData ) ; 
   fread( &N, sizeof(int), 1, impulseData ) ; 
   fread( &Lh, sizeof(int), 1, impulseData ) ; 
   fread( &impulse_SR, sizeof(int), 1, impulseData ) ; 


   pri( numberOfImpulseChannels, "NUMBER OF IMPULSE ANALYSIS CHANNELS" ) ; 
   pri( N, "IMPULSE RESPONSE FFT SIZE" ) ; 
   pri( Lh, "IMPULSE RESPONSE LENGTH IN SAMPLES" ) ; 
   Lhm1 = Lh - 1 ; L = 2 * Lh - 1 ; 
   N2 = N>>1 ; 

   pri( impulse_SR, "IMPULSE RESPONSE SAMPLE RATE" ) ; 

   if( impulseChannel == 0 ){
      // ALL CHANNELS
      impulseChannelNow = 0 ; 
      prt( "USING ALL IMPULSE CHANNELS, LOOP-ASSIGNED, BEGINNING WITH CHANNEL 1, TO INPUT CHANNELS, AS NEEDED." ) ; 
   } else {
      // ONE CHANNEL
      if( (impulseChannel - 1) >= numberOfImpulseChannels  ){
         pri( impulseChannel, "-----------> IMPULSE CHANNEL" ) ; 
         prt("--------------> NON-EXISTENT IMPULSE CHANNEL. BYE.\n\n\n") ;
         exit(EXIT_FAILURE) ; 
      } ; 
      impulseChannelNow = impulseChannel - 1 ; 
   } ; 


   if( add_ring_time__off_0__on_1 == 1 ) ringTime = (float) Lh / (float) R ; 
   else ringTime = 0. ; 
   

    // GET NAME OF USER
    user = getlogin(); 

if( (multichannel_output_mode__standard_0__alternate_1 == 1) && (channelout == 0) )
{
   prt( "\n\n---> ERROR: ALTERNATE OUTPUT MODE REQUIRES SELECTION OF A SINGLE" ) ; 
   prt( "INPUT CHANNEL THROUGH THE output_channel PARAMETER (cannot be 0).\n") ;
   prt( "CHANNELS ARE NUMBERED BEGINNING FROM 1.\n\n" ) ; exit(EXIT_FAILURE) ;
} ;



if(channelout == 0){ // ALL CHANNELS
   channelflag = 0 ; 
   beginchan = 0 ;
} else{
   beginchan = channelout - 1 ; 
   if( multichannel_output_mode__standard_0__alternate_1 == 0 )
   {
      channelflag = 1 ;
   } else if( multichannel_output_mode__standard_0__alternate_1 == 1 )
   {
      expandSingleChannelInputFileToMultipleDuplicateChannels = true ; 
      channelflag = 1 ; ochanModified = numberOfImpulseChannels ;
   } else {
      prt( "ERROR: ILLEGAL MULTI-CHANNEL OUTPUT MODE");
      exit(EXIT_FAILURE) ;
   } ;
} ;


// GET INPUT HEADER INFO AND SET UP OUTPUT FILE
    setupfiles(argc, argv) ; 


    endchan = beginchan + ochan ; 

 

 
// **** SET UPS *****
   R = isr ; // SAMPLE RATE EQUALS INPUT FILE

   if( impulse_SR != R ){
      pri( impulse_SR, "\n\nIMPULSE SAMPLE RATE" ) ; 
      pri( R, "INPUT SOUND SAMPLE RATE" ) ; 
      prt( "SAMPLE RATES OF IMPULSE RESPONSE AND INPUT SOUND DO NOT MATCH! \n\nBYE.\n\n" ) ; 
      exit(EXIT_FAILURE) ; 
   } ; 


    I = Lh ; 


//*********************************

    nyquist = R/2.0;

   if( IRBPF_LowFreqPoint < 0. ) IRBPF_LowFreqPoint = 0. ; 
   if( IRBPF_HighFreqPoint <= 0. ) IRBPF_HighFreqPoint = nyquist ; 

   if( SourceInputBPF_LowFreqPoint < 0. ) SourceInputBPF_LowFreqPoint = 0. ; 
   if( SourceInputBPF_HighFreqPoint <= 0. ) SourceInputBPF_HighFreqPoint = nyquist ; 


    fundamental =  ((float) R / (float) N) ; 
    PI = 4.*atan(1.) ;
    TWOPI = 8.* (float) atan(1.) ;


    // COMPUTE THE DURATION
   dur = (endt - begint) * (float) I / (float) Lh ; 
//   durInSamps = dur * (float) R ; 
   

   // OVERLAPP-ADD
   P = 0. ; obank = 0 ;  
    



    
//***************** PRINT VALUES

prf( dur, "OUTPUT FILE: DURATION" ) ; 

prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 

pri( frames_per_sec,  "FRAMES/SECOND" ) ; 

prbanner( "RESYNTHESIS PARAMETERS",  69 ) ; 

prp( &sourceGainIndB,  "SOURCE GAIN (in dB)"  ) ; 
prp( & Convolution_input_gain_in_decibels,  "CONVOLUTION INPUT GAIN (in dB)"  ) ; 
prp( & Convolution_output_gain_in_decibels,  "CONVOLUTION OUTPUT GAIN (in dB)"  ) ; 

prt( "IMPULSE RESPONSE BANDPASS FILTER:" ); 
prf( IRBPF_LowFreqPoint, "IMPULSE RESPONSE FREQUENCY BAND LOW ROLLOFF FREQUENCY" ) ; 
prf( IRBPF_HighFreqPoint, "IMPULSE RESPONSE FREQUENCY BAND HIGH ROLLOFF FREQUENCY" ) ; 
prf( IRBPF_lowFilterRolloffInDecibelsPerOctave, "FREQUENCY BAND LOW EDGE AMPLITUDE ROLLOFF IN DECIBELS PER OCTAVE" ) ; 
prf( IRBPF_highFilterRolloffInDecibelsPerOctave, "FREQUENCY BAND HIGH EDGE AMPLITUDE ROLLOFF IN DECIBELS PER OCTAVE" ) ; 


prt( "INPUT SOUND BANDPASS FILTER:" ); 
prf( SourceInputBPF_LowFreqPoint, "SOURCE INPUT BPF LOW ROLLOFF FREQUENCY" ) ; 
prf( SourceInputBPF_HighFreqPoint, "SOURCE INPUT BPF HIGH ROLLOFF FREQUENCY" ) ; 
prf( SourceInputBPF_lowFilterRolloffInDecibelsPerOctave, "SOURCE INPUT BPF LOW EDGE AMPLITUDE ROLLOFF IN DECIBELS PER OCTAVE" ) ; 
prf( SourceInputBPF_highFilterRolloffInDecibelsPerOctave, "SOURCE INPUT BPF HIGH EDGE AMPLITUDE ROLLOFF IN DECIBELS PER OCTAVE" ) ; 



if( impulseShapingFlag_Off_0__On_1 == 1 ){
   prp( &peak_shaping_amplitude_in_dB, "PEAK SHAPING AMPLITUDE (in dB)" ) ;  
   prp( &shaping_breakpoint_in_dB, "SHAPING BREAKPOINT (in dB)" ) ;  
   prp( &base_shaping_amplitude_in_dB, "BASE SHAPING AMPLITUDE (in dB)" ) ;  
   prp( &upper_shaping_curve_index, "UPPER SHAPING CURVE INDEX" ) ;  
   prp( &lower_shaping_curve_index, "LOWER SHAPING CURVE INDEX" ) ;  
} ;


prline( 1,  "*" ) ; 
prline( 1,  "*" ) ; 

prf( rescalev, "DECIBEL RESCALE VALUE" ) ; 


    // *******

// SET UP ARRAYS

    fvec( inbuffer, N ) ;      /* FFT buffer */
    fvec( outbuffer, N ) ;      /* FFT buffer */
    fvec( B, Lhm1 ) ; 
   fvec( inputSave, Lh ) ; 
   fvec( impulseAnalysis, N ) ; 


    
// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 


//*********************************************
// LOOP FOR CHANNELS
//*********************************************
if( (impulseChannel == 0) && (numberOfImpulseChannels > 1) ){
   prt( ". . . . . . . USING MULTIPLE IMPULSE ANALYSES WITH AUDIO INPUT CHANNELS." ); 
}else{
   prt( ". . . . . . . USING ONE IMPULSE ANALYSIS FOR ALL AUDIO INPUT CHANNELS." );
} ;

// SOURCE GAIN, SOURCE CONVOLUTION INPUT GAIN, CONVOLUTION OUTPUT GAIN

for(outchan = beginchan, channow = 0; outchan < endchan; outchan++, channow++ ){

    prline( 69,   "=" ) ; 
    pri( (outchan+1), "AUDIO INPUT: CHANNEL" ) ; 

    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ;
   frameBeginSampNow = 0 ;  


   pri( impulseChannelNow + 1, "IMPULSE ANALYSIS CHANNEL" ) ; 

   if( (impulseChannel == 0) && (numberOfImpulseChannels > 1) ){
      // MULTIPLE IMPULSE CHANNELS FOR MULTIPLE INPUT CHANNELS
      // FIRST FRAME -- POSITION TO 0 CHANNEL DATA BEGIN.
      fseek(impulseData,  sizeof(int) * 4, SEEK_SET) ; 
      fseek(impulseData,  sizeof(float) * N * impulseChannelNow, SEEK_CUR) ; 
      // READ IN IMPULSE ANALYSIS.
      fread( impulseAnalysis, sizeof(float), N, impulseData ) ; 
   }else{
      // ONE IMPULSE CHANNEL FOR ALL INPUT CHANNELS
      // FIRST FRAME -- POSITION TO impulseChannelNow CHANNEL DATA BEGIN.
      fseek(impulseData,  sizeof(int) * 4, SEEK_SET) ; 
      fseek(impulseData,  sizeof(float) * N * impulseChannelNow, SEEK_CUR) ; 
      // READ IN IMPULSE ANALYSIS.
      fread( impulseAnalysis, sizeof(float), N, impulseData ) ; 
   }; 


/*
    // SECRET PLOT
   sprintf( tempstring, "/tmp/impulse%d", channow ) ; 
    adata = fopen( tempstring, "w+" ) ; 
    for(i = 0; i < N; i++){
   fwrite( &impulseAnalysis[i], sizeof(float), 1, adata ) ; 
    }; 
    fclose( adata ) ; 
*/

   // APPLY BAND PASS FILTER WITH ROLLOFF TO IMPULSE RESPONSE.
   for( i = 0, j = 1, k = 0; k < (N / 2); i += 2, j += 2, k++ ){
      binFreq = (float) k * fundamental ; 
      if( binFreq < IRBPF_LowFreqPoint ){

         if( k == 0 ){
            impulseAnalysis[i] = 0.0 ;  

         }else{
             rolloffdB = 
          ((Hz_to_MIDI( IRBPF_LowFreqPoint ) - Hz_to_MIDI( binFreq )) / 12.0) *
                 IRBPF_lowFilterRolloffInDecibelsPerOctave ;
          rolloffAmp = dB_to_amp( rolloffdB ) ;
          impulseAnalysis[i] *= rolloffAmp ; impulseAnalysis[j] *= rolloffAmp ;
            
        } ; 

      }else if( binFreq > IRBPF_HighFreqPoint ){
          rolloffdB = ((Hz_to_MIDI( binFreq ) - Hz_to_MIDI( IRBPF_HighFreqPoint )) / 12.0) *
                 IRBPF_highFilterRolloffInDecibelsPerOctave ;
         
          rolloffAmp = dB_to_amp( rolloffdB ) ;
          impulseAnalysis[i] *= rolloffAmp ; impulseAnalysis[j] *= rolloffAmp ;
      } ; 
   } ; 











   // ZERO B ARRAY
   for(i = 0; i < Lh; i++) B[i] = 0. ; 

   eof = 0 ;  

   //*********************************************
   // LOOP FOR FRAMES
   //*********************************************
   while ( !eof ) {


      in += Lh ;
      on += I ;
      timenow( dur ) ;

      // ZERO inbuffer
      for(i = 0; i < N; i++ ) inbuffer[i] = 0. ; 


      // READ IN INPUT 
      sampsRead = 0 ; 
      for (i = 0; i < Lh; i++) {
         if ( ( bufferin( &inbuffer[i] ) ) == 0 ) {
            eof = 1 ; 
            break;
         }
         sampsRead++ ; 
      } ; 

      for(i = 0; i < sampsRead; i++){

         // SAVE INPUT FOR DYNAMIC MIXING OF SOURCE
         // ADD GAIN WITH TRANSFER
         temp = (float)(frameBeginSampNow + i) / (float) R ;
         sourceGainIndB.A[ 0 ] =  fval( &sourceGainIndB, dur, temp );
         inputSave[i] = inbuffer[i] * dB_to_amp( sourceGainIndB.A[ 0 ] ) ; 

         if( impulseShapingFlag_Off_0__On_1 == 1){
            peak_shaping_amplitude_in_dB.A[ 0 ] =  
               fval( & peak_shaping_amplitude_in_dB, dur, temp );
            shaping_breakpoint_in_dB.A[ 0 ] =    
               fval( & shaping_breakpoint_in_dB, dur, temp );
            base_shaping_amplitude_in_dB.A[ 0 ] =    
               fval( & base_shaping_amplitude_in_dB, dur, temp );
            upper_shaping_curve_index.A[ 0 ] =    
               fval( & upper_shaping_curve_index, dur, temp );
            lower_shaping_curve_index.A[ 0 ] =    
               fval( & lower_shaping_curve_index, dur, temp );
         
             if( inbuffer[i] != 0. ){
            temp = amp_to_dB( fabs( inbuffer[i] ) ); 
            if( temp >= shaping_breakpoint_in_dB.A[ 0 ] ){
               // AMPLITUDES ABOVE BREAK POINT            
               temp2 = (temp - shaping_breakpoint_in_dB.A[ 0 ]) / 
                  fabs( shaping_breakpoint_in_dB.A[ 0 ] ) ; // SPACE ABOVE BREAK AS PROPORTION
               temp2 = curve(shaping_breakpoint_in_dB.A[ 0 ], 
                        peak_shaping_amplitude_in_dB.A[ 0 ], 
                           temp2, upper_shaping_curve_index.A[ 0 ] ); 
               temp2 = dB_to_amp( temp2 ); 
               inbuffer[i] = (inbuffer[i] > 0.) ? temp2 : -1. * temp2 ; 
            }else{
               // AMPLITUDES BELOW BREAK POINT
               if( temp >= -96. ){
                  temp2 = (temp + 96.) /  
                     ( shaping_breakpoint_in_dB.A[ 0 ] + 96.  ) ;
                  temp2 = curve( base_shaping_amplitude_in_dB.A[ 0 ],
                            shaping_breakpoint_in_dB.A[ 0 ], 
                              temp2, lower_shaping_curve_index.A[ 0 ] ); 
                  temp2 = dB_to_amp( temp2 ); 
                  inbuffer[i] = (inbuffer[i] > 0.) ? temp2 : -1. * temp2 ; 
               } ; 
            } ;          
             } ; 
         } ; 
      

         // APPLY GAIN TO INPUT
         Convolution_input_gain_in_decibels.A[ 0 ] =  
            fval( &Convolution_input_gain_in_decibels, dur, 
                     (float)(frameBeginSampNow + i) / (float) R );
         inbuffer[i] *= dB_to_amp( Convolution_input_gain_in_decibels.A[ 0 ] ) ; 

      } ; 

      rfft( inbuffer, N2, FORWARD ) ;

      // ADD EQ HERE
   // APPLY BAND PASS FILTER WITH ROLLOFF TO INPUT SOUND.
   for( i = 0, j = 1, k = 0; k < (N / 2); i += 2, j += 2, k++ ){
      binFreq = (float) k * fundamental ; 
      if( binFreq < SourceInputBPF_LowFreqPoint ){

         if( k == 0 ){
            inbuffer[i] = 0.0 ;  

         }else{
             rolloffdB = 
          ((Hz_to_MIDI( SourceInputBPF_LowFreqPoint ) - Hz_to_MIDI( binFreq )) / 12.0) *
                 SourceInputBPF_lowFilterRolloffInDecibelsPerOctave ;
          rolloffAmp = dB_to_amp( rolloffdB ) ;
          inbuffer[i] *= rolloffAmp ; inbuffer[j] *= rolloffAmp ;

        } ; 

      }else if( binFreq > SourceInputBPF_HighFreqPoint ){
          rolloffdB = ((Hz_to_MIDI( binFreq ) - Hz_to_MIDI( SourceInputBPF_HighFreqPoint )) / 12.0) *
                 SourceInputBPF_highFilterRolloffInDecibelsPerOctave ;
         
          rolloffAmp = dB_to_amp( rolloffdB ) ;
          inbuffer[i] *= rolloffAmp ; inbuffer[j] *= rolloffAmp ;
      } ; 
   } ; 

      // END EQ

      if( deconvolution_0__convolution_1 == 1 ){
         // CONVOLVE FFT THROUGH COMPLEX MULTIPLY  
         outbuffer[0] = inbuffer[0] * impulseAnalysis[0] ; 
         outbuffer[1] = inbuffer[1] * impulseAnalysis[1] ; 
         for(i = 2, j = 3;  i < N; i += 2, j += 2){
            real = (inbuffer[i] * impulseAnalysis[i]) - (inbuffer[j] * impulseAnalysis[j]) ; 
            imag = (inbuffer[i] * impulseAnalysis[j]) + (inbuffer[j] * impulseAnalysis[i]) ; 
            outbuffer[i] = real ; outbuffer[j] = imag ; 

         } ; 
      }else if( deconvolution_0__convolution_1 == 0 ){
         // DECONVOLVE FFT THROUGH COMPLEX DIVIDE
         outbuffer[0] = inbuffer[0] / impulseAnalysis[0] ; 
         outbuffer[1] = inbuffer[1] / impulseAnalysis[1] ; 
         for(i = 2, j = 3;  i < N; i += 2, j += 2){
//            temp = (c * c) + (d * d) ; 
//            real = ((a * c) + (b * d)) / temp ; 
//            imag = ((b * c) - (a * d)) / temp ; 
            temp = (impulseAnalysis[i] * impulseAnalysis[i]) + (impulseAnalysis[j] * impulseAnalysis[j]) ; 
//            if( (temp < dB_to_amp( DECIBEL_THRESH )) && (temp > (-1. * dB_to_amp( DECIBEL_THRESH ))) ) 
//                  temp = copysign( dB_to_amp( DECIBEL_THRESH ), temp ) ; 
            real = ((inbuffer[i] * impulseAnalysis[i]) + (inbuffer[j] * impulseAnalysis[j])) / temp ; 
            imag = ((inbuffer[j] * impulseAnalysis[i]) - (inbuffer[i] * impulseAnalysis[j])) / temp ; 
            outbuffer[i] = real ; outbuffer[j] = imag ; 
         } ; 

      }else{
         // 
         pri( deconvolution_0__convolution_1, "\n\nILLEGAL PROCESS PARAMETER" ) ;
         prt( "\n\n. . . . BYE.\n\n" ) ;  
         exit( EXIT_FAILURE ) ; 
      } ; 

/*
      // SET BINS TO 0 THAT LIE OUTSIDE OF BOUNDARIES
      for( i = 0, j = 1, k = 0; k < (N / 2); i += 2, j += 2, k++ ){
         binFreq = (float) k * fundamental ; 
         if( (binFreq < outputLowFreqPoint) || (binFreq > outputHighFreqPoint) ){
            outbuffer[i] = 0. ; outbuffer[j] = 0. ;             
         } ; 
      } ; 
*/



      rfft( outbuffer, N2, INVERSE ) ;

      // OVERLAP/ADD
       for( i = 0; i < Lhm1; i++ ){
         outbuffer[i] += B[i] ; 
         B[i] = outbuffer[Lh + i] ; 
      } ; 


      for( i = 0; i < sampsRead; i++ ){

         // APPLY GAIN TO OUTPUT
         Convolution_output_gain_in_decibels.A[ 0 ] =  
            fval( &Convolution_output_gain_in_decibels, dur, (float)(frameBeginSampNow + i) / (float) R );
         outbuffer[i] *= dB_to_amp( Convolution_output_gain_in_decibels.A[ 0 ] ) ; 
         // ADD IN SOURCE
         outbuffer[i] += inputSave[i] ; 

      }; 


      bufferout(outbuffer, sampsRead, 0)  ; 

      frameBeginSampNow += sampsRead ; 

      // FRAMES LOOP END

    }



    // FLUSH OUT AND CLOSE OUTPUT FILE
   bufferout(outbuffer, 0, 1)  ;

   // ADVANCE IMPULSE CHANNEL 
   if( impulseChannel == 0 ) {
      impulseChannelNow += 1 ; 
      while( impulseChannelNow >= numberOfImpulseChannels ) impulseChannelNow -= numberOfImpulseChannels ; 
   } ; 
    
   // CHANNELS LOOP END
} 




// CLOSE  INPUT FILE
if(ifd)fclose(ifd);  

// CLOSE IMPULSE FILE
fclose( impulseData ) ; 


   if( sourceGainIndB.n != 1. ) fclose(sourceGainIndB.fp ) ;

     
    fprintf(stderr,"\nIRCONVOLVER: RESYNTHESIS COMPLETED\n");
    exit(EXIT_SUCCESS) ;
}



void usage()
{
    fprintf(stderr, "%s",
   "irconvolver:  fast impulse response convolution  \n"
   "irconvolver   [flags] [input file] [output file]\n"
   "       Most formats accepted. Output format copied from input file.\n"
   "       (Values in brackets denote defaults.)\n"

	"	b:   "BEGIN_TIME      // begint
	"	e:   "END_TIME         // endt
	"	C:   "RESYNTHESIS_CHANNEL      // channelout
	"	E:   impulse response data file\n"

	"	M:   multi-channel output mode: standard = 0, alternate = 1 [0]\n"

	"	   Gain Controls:\n"
	"	A:   input source gain in dB, for mix with convolution output (func)[-0.]\n"
	"	q:   convolution source input gain in dB (func)[-0.]\n"
	"	r:   convolution source output gain in dB (func)[-0.]\n"


	"	J:	impulse channel [0]\n"
	"	a:	Spectrum DB Plot File \n"
	"	d:	amplitude normalization level in dB [0]\n"

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

	"		INPUT SOUND FILTER BAND:\n"
	"	D:	input sound filter band low rolloff frequency [0]\n"
	"	H:	input sound filter band high edge amplitude rolloff in decibels per octave [0]\n"
	"	f:	input sound filter band high rolloff frequency [0]\n"
	"	h:	input sound filter band low edge amplitude rolloff in decibels per octave [0]\n" 


	"	p:   "AMP_REPORTS      // quiet 
	"	i:   "AMP_REPORTS_TIME_INTERVAL   // ampstatinc 

	"	_:    "AUTO_PLAY      // autoplayreps

	"	=:    "RESCALE_LEVEL      // rescalev

   );
    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
