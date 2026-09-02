#include "globals.h"


void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j, k, l,  m, n, nnn=0 ;
float nyquist,  fundamental ;
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 220, I = 220, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  write_ascii=0 ;
float P = 1.0;
FILE *fopen() ;
char ch,  tempstring[ STRING_SIZE ],  
    scratch[ STRING_SIZE ],  scratch2[ STRING_SIZE ],  *user, fileDelayTimesFileName[ STRING_SIZE ]="",
		new_fileDelayTimesFileName[ STRING_SIZE ]="", fileGainLevelModifiersFileName[ STRING_SIZE ]="",
		 new_fileGainLevelModifiersFileName[ STRING_SIZE ]="" ;
int delaysFlag=0, catFilesFlag=0, gainLevelsFlag=0, file ; 
float  dur ;
float  gain, f ;
float  temp, temp1,  temp2,  pm,  IR  ;  
int showme=0 ; 
int numberOfOutChannels=1, chan, outputChan, inputChan, fileSizeInBytes, numFrames ; 
float outputChanPeakAmpSum[2]={-99999999.,-999999999.}, thisOutputChanAmpSum[2],
	PeakAmp ; 

FILE *data, *delayFile, *gainLevelModifiersFile ;

float *fileDelayTimes, *fileGainLevelModifiers, delayTime, previousDelayTime=0.0,  gainLevel ;  

float *inputSoundFileDurationsInSeconds ; 

int soundFileIndex ; 

int outputMode=-1, filepos ; 

float timeNow ; 

char outputFileName[ STRING_SIZE ]="" ; 

char channelOrderFile[ STRING_SIZE ] = "",  new_ChannelOrderFile[ STRING_SIZE ] ; 

int channelOutputLength=0 ; 

int channelOrderFlag__input_order_0__file_order_1, frameNow ; 
int numberOfDelayFileDelayTimes ; 


int numSampsBufferedIn, numFramesLeft, arg_index_Save, normalizeFlag=0 ; 
int numFramesBufferedIn, blockFrame ; 

int numberOfInputSoundFiles=0, totalFramesBufferedIn, 
	numberOfInputChannels, largestNumberOfInputChannels=0, largestNumberOfFrames=0, 
		 numberOfFrames, maxNumberOfFrames=0, minNumberOfFrames, thisInputFile,
	numberOfSampsBufferedIn, numberOfFramesBufferedIn, numberOfFramesToTransfer, silentBufferSize ; 

SF_INFO inputSFinfo ;  
SF_INFO outputSFinfo ; 

float duration, *silentBuffer, channelPeakAmps[ MAXIMUM_CHANNELS ], peakChannelAmp=0. ; 


float interleavedInputBuffer [ BLOCKSIZE * MAXIMUM_CHANNELS ] ; 
float inputBufferByChannels [ MAXIMUM_CHANNELS ][ BLOCKSIZE ] ; 

char *home_directory ; 

float tempBlock[ BLOCKSIZE ], *allChanInputBlock, *allChanOutputBlock ; 

float truncateDuration=-1. ; 

int truncateDurationFlag=0, truncateDurationInFrames ; 

// SOURCE GAIN
struct  func  amplitudeEnvelope ; 

amplitudeEnvelope.L = 1. ;  amplitudeEnvelope.n = 1. ; amplitudeEnvelope.A[ 0 ] = 1. ; 


if( argc < 2 )usage() ; 


while( (ch = crack( argc, argv, "c|g|f|l|n|o|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {


	case 'l':  channelOutputLength = (int) crackfloat( arg_option, ch ) ;
			break;
		
	case 'n':   normalizeFlag = (int) crackfloat( arg_option, ch ) ;
			break;
		
	case 'o':   strcpy(ofile, arg_option) ; fixTildeInFilename( ofile ) ;
			break;

	case 'c':	delaysFlag = 0 ; catFilesFlag = 1 ;  
			break;

	case 'f':	strcpy(fileDelayTimesFileName, arg_option) ; fixTildeInFilename( fileDelayTimesFileName ) ;
				delaysFlag = 1 ; 
			break;


	case 'g':	strcpy(fileGainLevelModifiersFileName, arg_option) ; fixTildeInFilename( fileGainLevelModifiersFileName ) ;
				gainLevelsFlag = 1 ; 
			break;


	} 
}


prs( ofile, "OUTPUT FILE" ) ; 





prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "MIXFILES", 69 ) ; 
prline( 69,  "-" ) ; 

// GET NAME OF USER
user = getlogin(); 


if( strcmp( fileDelayTimesFileName, "") != 0  ){

	prs( fileDelayTimesFileName, "DELAY TIMES FILE" ) ; 
	
	// MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
    	cut_data_lines( fileDelayTimesFileName,  new_fileDelayTimesFileName,  1 ) ; 

  	if( (delayFile = fopen( new_fileDelayTimesFileName, "r")) == NULL ){
	    	fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  new_fileDelayTimesFileName ) ; 
	    	exit(EXIT_FAILURE); 
	}

	// COUNT VALUES IN FILE.
	numberOfDelayFileDelayTimes = 0 ; 
	rewind( delayFile ) ; 
	while( fscanf( delayFile,  " %f ",  &delayTime ) != EOF ){
			numberOfDelayFileDelayTimes++ ; 			
	} ;  
} ; 


  

// ************

arg_index_Save = arg_index ; 

//pri( arg_index, "arg_index" ) ; 

if( argc > 1   ){ 	// 

	prbanner( "INPUT SOUND FILES", 69 ) ; 

	if( delaysFlag == 1 ) rewind( delayFile ) ; 
	k = 0 ; 

	
	fvec( inputSoundFileDurationsInSeconds, argc - arg_index ) ; 

	soundFileIndex = 0 ; 

	while( arg_index < argc ){
	
		// GET INPUT SOUNDFILE NAME
		strcpy( ifile, argv[arg_index] ) ; 

		fprintf( stderr, "\n(%d) %s", numberOfInputSoundFiles, ifile ) ; 

		// OPEN INPUT SOUND FILE IN READ MODE TO GET FORMAT. 

		if(! (infile = sf_open (ifile, SFM_READ, &inputSFinfo ))){
			// FILE DOES NOT EXIST
			prt( "------> NOT FOUND\n\n . . . . .  BYE.\n\n" ) ; 
			exit( EXIT_FAILURE ) ; 
		}else{

			 

			if( numberOfInputSoundFiles == 0 ){
	    			isr = inputSFinfo.samplerate ; 
			}else{
				if( inputSFinfo.samplerate != isr ){
					pri( inputSFinfo.samplerate, "\tSAMPLE RATE" ) ; 
					prt( "SAMPLE RATE DOES NOT MATCH PREVIOUS FILE(S).(ALL RATES MUCH MATCH.)\n\n . . . . . BYE.\n" ) ; 
					exit( EXIT_FAILURE ) ; 
				} ; 			

			} ; 
			numberOfInputChannels  = inputSFinfo.channels ;
			if( numberOfInputChannels > largestNumberOfInputChannels ) 
					largestNumberOfInputChannels = numberOfInputChannels ; 

			numberOfFrames = inputSFinfo.frames ;
			duration  = (float) numberOfFrames / (float) isr ;
			inputSoundFileDurationsInSeconds[ soundFileIndex ] = duration ; 


			delayTime = 0 ; 

			if( (delaysFlag == 1) && 
					( (arg_index - arg_index_Save) < numberOfDelayFileDelayTimes ) ){
				fscanf( delayFile,  " %f ",  &delayTime ) ;
			} ; 
			if( catFilesFlag == 1 ){
				previousDelayTime = delayTime = soundFileIndex == 0 ? 0.0 : 
					previousDelayTime + inputSoundFileDurationsInSeconds[ soundFileIndex - 1 ] ;
			} ; 


			fprintf( stderr, "\n -- CHANNELS: %d  SAMPLE RATE: %d FRAMES: %d  DURATION: %f  DELAY TIME: %f", 
				numberOfInputChannels, isr, numberOfFrames, inputSoundFileDurationsInSeconds[ soundFileIndex ], 
					delayTime ) ; 
			
			n = numberOfFrames + (int)( (float) isr * delayTime ) ; 
			if( numberOfInputSoundFiles > 0 ){
				if( n > maxNumberOfFrames ) 
					maxNumberOfFrames = n ;  
			}else{
				maxNumberOfFrames = n ;  
			} ; 

			iformat = inputSFinfo.format ; 

			numberOfInputSoundFiles++ ;

    			// CLOSE IT. 
    			sf_close( infile ) ;     
		} ; 

		arg_index++ ; 
		soundFileIndex++ ; 


	}  ;

	prline( 69,  "*" ) ; 


 

	fprintf( stderr, "\nTOTAL SOUND FILES: %d\tMAXIMUM NUMBER OF CHANNELS: %d", 
			numberOfInputSoundFiles, 		largestNumberOfInputChannels ) ; 

	fvec( fileDelayTimes, numberOfInputSoundFiles ) ; 
	for( file = 0; file < numberOfInputSoundFiles; file++ ) fileDelayTimes[ file ] = 0. ; 
	if( delaysFlag == 1 ) {
		rewind( delayFile ) ; 
		k = numberOfInputSoundFiles <  numberOfDelayFileDelayTimes ? 
				numberOfInputSoundFiles  : numberOfDelayFileDelayTimes ; 
		for( file = 0; file < k ; file++ ){ 
			fscanf( delayFile,  " %f ",  &fileDelayTimes[ file ] ) ; 
		} ; 
	} ; 

	if( catFilesFlag == 1 ){
		for( file = 0; file < numberOfInputSoundFiles ; file++ ){ 
			fileDelayTimes[ file ] = file == 0 ? 0.0 : 
				fileDelayTimes[file - 1] + inputSoundFileDurationsInSeconds[file - 1] ;
		} ; 
	} ; 

	// ***

	fvec( fileGainLevelModifiers, numberOfInputSoundFiles ) ; 
	for( file = 0; file < numberOfInputSoundFiles; file++ ) fileGainLevelModifiers[ file ] = 1. ; 

	if( strcmp( fileGainLevelModifiersFileName, "") != 0  ){
		prs( fileGainLevelModifiersFileName, "GAIN LEVELS FILE" ) ; 
	    	cut_data_lines( fileGainLevelModifiersFileName,  new_fileGainLevelModifiersFileName,  1 ) ; 
		if( (gainLevelModifiersFile = fopen( new_fileGainLevelModifiersFileName, "r")) == NULL ){
	    		fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  new_fileDelayTimesFileName ) ; 
	    		exit(EXIT_FAILURE); 
		}
		file = 0 ; 
		while( fscanf( gainLevelModifiersFile,  " %f ",  &gainLevel ) != EOF ){
			fileGainLevelModifiers[ file ] = dB_to_amp( gainLevel ) ; file++ ; 			
		} ;  
	} ; 

	// ***
	prt( "\nFile\tDelay Time\tGain" ) ; 
	for( file = 0; file < numberOfInputSoundFiles ; file++ ) 
		fprintf( stderr, "\n%d.\t%f\t%f dB", 
			file + 1, fileDelayTimes[ file ], amp_to_dB( fileGainLevelModifiers[ file ] ) ) ; 





	if( channelOutputLength > largestNumberOfInputChannels ){
		fprintf( stderr, "\n\nWARNING: SPECIFIED NUMBER OF OUTPUT CHANNELS EXCEEDS INPUT FILES;" );
		fprintf( stderr, "\n\nTRUNCATING TO INPUT NUMBER OF %d.", largestNumberOfInputChannels ) ; 
		channelOutputLength = largestNumberOfInputChannels ; 
	} ; 


	if( largestNumberOfInputChannels > MAXIMUM_CHANNELS )
	{
	     prline( 69, "=" ); 
	     prt( "NUMBER OF CHANNELS EXCEEDS CURRENT MAXIMUM SETTING." ); 
	     prt( ""); 
	     fprintf( stderr, "\n\nIN FILE pvc_lib/pv.h, CHANGE MAXIMUM CHANNEL SETTING OF %d TO %d.\n\n",
			MAXIMUM_CHANNELS, largestNumberOfInputChannels ) ; 
	     prt( "THEN RECOMPILE AND REINSTALL." ) ; 
	     prline( 69, "=" ); 
	     prt( ""); 
	     prt( ""); 
	     exit(EXIT_FAILURE) ; 
	} ; 

	prline( 69,  "*" ) ; 

	fprintf( stderr, "\nMAXIMUM DURATION: %f (FRAMES: %d)", 
			(float) maxNumberOfFrames / (float) isr, maxNumberOfFrames 	 ) ; 
	prline( 69,  "*" ) ; 

 


	// ZERO PEAK AMPS
	for(chan = 0; chan < largestNumberOfInputChannels; chan++ ) channelPeakAmps[chan] = 0. ; 

	fvec( silentBuffer, BLOCKSIZE ) ; 
	for(i = 0; i < BLOCKSIZE; i++)silentBuffer[i] = 0. ; 

	for(chan = 0; chan < largestNumberOfInputChannels; chan++ ){
		sprintf( tempstring, "/tmp/%s.InputChan.%d", user, chan ) ; // MAKE FILE NAME
		prs( tempstring, "\t/tmp FILE NAME" ) ; 
		filesToRemove( tempstring, 0 ) ; 
		inputTempChanFiles[ chan ] = fopen( tempstring, "wb+" ); 
		// WRITE OUT THE FULL BLANK FILE
		numFramesLeft = maxNumberOfFrames ; 
		while( numFramesLeft > 0){
			numberOfFramesToTransfer = numFramesLeft < BLOCKSIZE ? numFramesLeft : BLOCKSIZE ; 
			fwrite( silentBuffer, sizeof(float), 
				numberOfFramesToTransfer, inputTempChanFiles[ chan ] ) ;
			numFramesLeft -=  numberOfFramesToTransfer ; 
		} ; 
		// REWIND FILE
		fseek( inputTempChanFiles[ chan ], 0, SEEK_SET ) ;
	} ; 




	// OPEN EACH FILE AND MIX EACH CHANNEL WITH THE CORRESPONDING MONO FILE IN /tmp

	arg_index = arg_index_Save ; 

	while( arg_index < argc ){


		// FIRST POSITION POINTERS IN /tmp FILES TO DELAY TIME POINT FOR THIS FILE.
		for(chan = 0; chan < numberOfInputChannels; chan++ ){
			filepos =  sizeof(float) * (int)(fileDelayTimes[ arg_index - arg_index_Save ] * (float) isr) ; 
			fseek( inputTempChanFiles[ chan ], filepos, SEEK_SET ) ; 
		} ;  

	
		// GET INPUT SOUNDFILE NAME
		strcpy( ifile, argv[arg_index] ) ; 
//		prs( ifile, "\nINPUT SOUND FILE" ); 

		// OPEN INPUT SOUND FILE IN READ MODE TO GET FORMAT. 

		totalFramesBufferedIn = 0 ; 

		if(! (infile = sf_open (ifile, SFM_READ, &inputSFinfo ))){
			// FILE DOES NOT EXIST
			prt( "------> NOT FOUND\n\n . . . . .  BYE.\n\n" ) ; 
			exit( EXIT_FAILURE ) ; 
		}else{


			numberOfInputChannels  = inputSFinfo.channels ;
			// READ IN BLOCKS OF AUDIO DATA AND MIX WITH RESPECTIVE /tmp FILES
			while( (numberOfSampsBufferedIn = 
				sf_read_float (infile, interleavedInputBuffer, BLOCKSIZE * numberOfInputChannels ))
				&&
				( (maxNumberOfFrames - totalFramesBufferedIn) > 0)
			){   

	    			numberOfFramesBufferedIn = numberOfSampsBufferedIn / numberOfInputChannels ; 
				
				// SET TRANSFER AMOUNT BASED ON MAX NEEDED; TRUNCATE THIS BUFFER IF NECESSARY
				if( (maxNumberOfFrames - totalFramesBufferedIn) >= numberOfFramesBufferedIn )  
					numberOfFramesToTransfer = numberOfFramesBufferedIn ; 
				else
					numberOfFramesToTransfer = maxNumberOfFrames - totalFramesBufferedIn ; 


				// DE-INTERLEAVE interleavedInputBuffer INTO inputBufferByChannels.

            		for(chan = 0; chan < numberOfInputChannels; chan++ ){
					for( n = 0; n < numberOfFramesToTransfer; n++ ){
		   				inputBufferByChannels[ chan ][ n ] = 
							interleavedInputBuffer[ chan + (n * numberOfInputChannels) ] ;
					} ; 
            		} ; 

				totalFramesBufferedIn += numberOfFramesToTransfer ; 

				// MIX INPUT CHANNELS WITH CORRESPONDING /tmp FILES
				for(chan = 0; chan < numberOfInputChannels; chan++ ){
					// FIRST GET THE FILE POSITION FOR REPOSITIONING FOR WRITE. 
					filepos = ftell( inputTempChanFiles[ chan ] ) ; 
					// READ IN A BLOCK FROM THIS CHANNEL
	    				numFramesBufferedIn = fread( &tempBlock, sizeof(float), numberOfFramesToTransfer,  
						inputTempChanFiles[ chan ] ) ; 
					// MIX WITH DE-INTERLEAVED INPUT CHANNEL
					for(n = 0; n < numberOfFramesToTransfer; n++){
						inputBufferByChannels[ chan ][ n ] =
							(inputBufferByChannels[ chan ][ n ] * 
								fileGainLevelModifiers[ arg_index - arg_index_Save ] ) 
									+ tempBlock[ n ] ;
						// CHECK FOR PEAK
						if( fabs( inputBufferByChannels[ chan ][ n ] ) > channelPeakAmps[chan] ) 
							channelPeakAmps[chan] = fabs( inputBufferByChannels[ chan ][ n ] ) ; 
 					} ; 
					fseek( inputTempChanFiles[ chan ], filepos, SEEK_SET ) ; 
					fwrite( &inputBufferByChannels[ chan ], sizeof(float), 
							numberOfFramesToTransfer, inputTempChanFiles[ chan ] ) ; 
				} ; 

			} ; 

    			// CLOSE IT. 
    			sf_close( infile ) ;     
		} ; 

		arg_index++ ; 

	}  ;

	// FIND PEAK AMP FOR ALL CHANNELS.
	peakChannelAmp = 0. ; 
	for(chan = 0; chan < largestNumberOfInputChannels; chan++) 
		if( channelPeakAmps[chan] > peakChannelAmp ) peakChannelAmp = channelPeakAmps[chan] ; 

	// PEAK AMPS IN DB
	prbanner( "CHANNEL PEAK AMPLITUDES", 69 ) ; 

	for(chan = 0; chan < largestNumberOfInputChannels; chan++){ 
		fprintf( stderr, "(%d) %f dB", chan + 1, amp_to_dB( channelPeakAmps[chan] ) ) ; 
		if( channelPeakAmps[chan] == peakChannelAmp ) fprintf( stderr, " (peak)" ) ; 
		fprintf( stderr, "\n" ) ;
	} ; 

	prbanner( "NORMALIZATION", 69 ) ;
	if( normalizeFlag == 0 )fprintf( stderr, "\nNORMALIZATION IS OFF"); 
	if( normalizeFlag == 1 )fprintf( stderr, "\nNORMALIZING EACH CHANNEL SEPARATELY"); 
	if( normalizeFlag == 2 )fprintf( stderr, "\nNORMALIZING CHANNELS TOGETHER"); 
	if( normalizeFlag == 3 )fprintf( stderr, "\nWILL NORMALIZE IF PEAK EXCEEDS 0 DECIBELS"); 


	if( (channelOutputLength == 0) || (channelOutputLength == largestNumberOfInputChannels) ){
			channelOutputLength = largestNumberOfInputChannels ; 
	}else{
			fprintf( stderr, "\nOUTPUT CHANNELS TRUNCATED TO FIRST %d CHANNELS.", channelOutputLength ) ; 
	} ; 

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


    	outputSFinfo.samplerate = osr = isr ;  
	outputSFinfo.channels = channelOutputLength ;
	duration = (float) maxNumberOfFrames / (float) osr ; 
	fprintf( stderr, "\n -- CHANNELS: %d\n -- SAMPLE RATE: %d\n -- FRAMES: %d\n -- DURATION: %f", 
			outputSFinfo.channels, osr, maxNumberOfFrames, duration ) ; 


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

 
	// ************


	// REWIND TEMP FILES
	for(chan = 0; chan < channelOutputLength; chan++ ){
		fseek( inputTempChanFiles[ chan ], 0, SEEK_SET ) ;
	} ; 





	// BEGIN INTERLEAVE
	// MAKE SPACE
	fvec( allChanOutputBlock, BLOCKSIZE * channelOutputLength ) ; 


	numFramesLeft = maxNumberOfFrames ; 
	frameNow = 0 ; 

	prt( "\nINTERLEAVING /tmp FILES INTO OUTPUT FILE . . . \n" ) ; 

	while( numFramesLeft > 0 ){
		// FRAMES LOOP
		
		// ZERO OUTPUT ARRAY
		for(i = 0; i < BLOCKSIZE * channelOutputLength; i++) allChanOutputBlock[i] = 0. ; 

		for( chan = 0; chan < channelOutputLength; chan++ ){
			//fprintf( stderr, "\n%d ", chan ) ; 
			// POSITION IN FILE FOR READ
			fseek( inputTempChanFiles[ chan ], frameNow * sizeof(float), SEEK_SET ) ;  
			// READ IN A CHANNEL BLOCK
	    		numFramesBufferedIn = fread( &tempBlock, sizeof(float), BLOCKSIZE,  
				inputTempChanFiles[ chan ] ) ; 

			// NORMALIZE IF WANTED
			// NORMALIZE CHANNELS SEPARATELY
			if( (normalizeFlag == 1) && (channelPeakAmps[ chan ] > 0.) )
				for( i = 0; i < numFramesBufferedIn ; i++ ) 
						tempBlock[i] /= (channelPeakAmps[ chan ] * 1.01 ) ;  
			// NORMALIZE CHANNELS TOGETHER
			if( (normalizeFlag == 2) && (peakChannelAmp > 0.) )
				for( i = 0; i < numFramesBufferedIn ; i++ ) tempBlock[i] /= (peakChannelAmp * 1.01 ) ;  
			// NORMALIZE CHANNELS IF PEAK EXCEEDS 0 DECIBELS
			if( (normalizeFlag == 3) && (peakChannelAmp > 1.) )
				for( i = 0; i < numFramesBufferedIn ; i++ ) tempBlock[i] /= (peakChannelAmp * 1.01 ) ;  

			// INTERLEAVE CHANNEL BLOCK INTO OUTPUT ARRAY
			for(i = 0, k = chan; i < numFramesBufferedIn; i++, k += channelOutputLength ) 
						allChanOutputBlock[k] = tempBlock[i] ; 
		};  



		// WRITE BUFFER OUT TO SOUNDFILE
		k = numFramesBufferedIn * channelOutputLength ; 
		sf_write_float ( outfile, allChanOutputBlock, k  ) ; 

		numFramesLeft -= numFramesBufferedIn ; frameNow += numFramesBufferedIn ; 

	} ; 


	// END OF INTERLEAVE

	// CLOSE SOUND FILE. 
	sf_close( outfile ) ; 


//	for( chan = 0; chan < largestNumberOfInputChannels; chan++ ) fclose( inputTempChanFiles[ chan ] ); 
	filesToRemove( NULL, 1 );

	// ********

	prt( "\nSUCCESS!\n\n" ) ; 
	exit(EXIT_SUCCESS) ;




}else{
	// NO SOUND FILES
	usage() ; 
}  ; 


 


}

void usage()
{
	fprintf(stderr, "%s",
	"mixfiles:    \n"
	"mixfiles   [flags] [input files]\n"
	"	    Most formats accepted.\n"
	"	    (Values in brackets denote defaults.)\n"

	"	o:	output file\n"
	"	n:	Normalization: \n"
	"			0: off\n"
	"			1: Normalize channels independently.\n"
	"			2: Normalize channels together against channel with peak amplitude.\n"
	"			3: Normalize channels together if any channel exceeds 0 decibels.[0]\n"
	"	l:	Number of Output Channels  (optional) \n"
	"			When  left unspecified, the number of channels corresponds to the file with the largest number \n"
	"				of channels. Values less than the largest found output the specified number of channels\n"
	"				in order from the first. [largest number found]\n"         
	"	f:	Delay Times File\n"
	"			The values of the specified ASCII file are matched, in order, with the input sound file\n"
	"			list and treated as delay times, in seconds, for those files. The number of file values\n"
	"			need not match the number of sound files; files not matched to\n"
	"			delay times (short lists) are assigned a delay time of zero.[no delays]\n" 	
	"	c:	Concatenate Files\n"
	"			Delay files as though sequenced in concatenation. \n"
	"			The -c flag overrides any specified delay times file.\n"
	"	g:	Audio Gain Modifiers File (in decibels)\n"
	"			The values of the specified ASCII file are matched, in order, with the input sound file\n"
	"			list and treated as gain level modifiers, treated as decibels, for those files. The number of file values\n"
	"			need not match the number of sound files; files not matched to\n"
	"			a level modifier (short lists) are not modified.[no change]\n" 	
	);
	exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }



