#include "globals.h"


void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j, k, n;




FILE *fopen() ;
char ch,  tempstring[ STRING_SIZE ],  *user ;


float  temp;  
 
int chan; 
 

FILE *data ; 

int outputMode=-1 ; 

int numFramesOut=0 ; 

float totalTime, timeNow ; 

 

char channelOrderFile[ STRING_SIZE ] = "",  new_ChannelOrderFile[ STRING_SIZE ],
	channelGainscaleFileInDecibels[ STRING_SIZE ] = "",
	new_ChannelGainscaleFileInDecibels[ STRING_SIZE ] ; 



int channelOutputLength=0 ; 

int channelOrderFlag__input_order_0__file_order_1, frameNow ; 
int *outputChannelIndices, numberOfOutputChannelIndices ;
int numberOfGainscaleFileValues ;  
float *outputChannelDecibelScalers ; 

int channelGainscaleFlag__off_0__on_1=0 ; 

int numFramesLeft, arg_index_Save, normalizeFlag=0 ; 
int numFramesBufferedIn; 

int numberOfInputSoundFiles=0, totalNumberOfInputChannels=0, totalFramesBufferedIn, 
	numberOfInputChannels, numberOfFrames, maxNumberOfFrames=0, minNumberOfFrames, thisTempFile,
	numberOfSampsBufferedIn, numberOfFramesBufferedIn, numberOfFramesToTransfer, silentBufferSize ; 

SF_INFO inputSFinfo ;  
SF_INFO outputSFinfo ; 

float duration, *silentBuffer, channelPeakAmps[ MAXIMUM_CHANNELS ], peakChannelAmp=0. ; 


float interleavedInputBuffer [ BLOCKSIZE * MAXIMUM_CHANNELS ] ; 
float inputBufferByChannels [ MAXIMUM_CHANNELS ][ BLOCKSIZE ] ; 

 

float tempBlock[ BLOCKSIZE ], *allChanOutputBlock ; 

float truncateDuration=-1. ; 

int truncateDurationFlag=0, truncateDurationInFrames ; 

// SOURCE GAIN
struct  func  amplitudeEnvelope ; 

amplitudeEnvelope.L = 1. ;  amplitudeEnvelope.n = 1. ; amplitudeEnvelope.A[ 0 ] = 1. ; 


if( argc < 2 )usage() ; 


while( (ch = crack( argc, argv, "a|g|c|d|f|i|l|n|o|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {


	case 'i':  outputMode = 0 ;
			break;
	case 'c': outputMode = 1 ;
			break;
		
	case 'l':  channelOutputLength = (int) crackfloat( arg_option, ch ) ;
			break;
		
	case 'f':   strcpy(channelOrderFile, arg_option);
			break;

	case 'g':   strcpy(channelGainscaleFileInDecibels, arg_option);
			break;

	case 'a':   strcpy(tempstring, arg_option);
			amplitudeEnvelope.fp = crackstring( tempstring, &amplitudeEnvelope );
			break;

	case 'd':   truncateDuration = crackfloat( arg_option, ch ) ;
			break;

	case 'n':   normalizeFlag = (int) crackfloat( arg_option, ch ) ;
			break;
		
	case 'o':   strcpy(ofile, arg_option) ; fixTildeInFilename( ofile ) ;
			break;


	} 
}


if( outputMode == 0 ) prt( "INPUT CHANNELS WILL BE INTERLEAVED." ) ; 
else if( outputMode == 1 ) prt( "INPUT CHANNELS WILL BE CONCATENATED." ) ; 
else{
	prt( "\n\nERROR: OUTPUT MODE NOT SPECIFIED; -c or -i FLAG REQUIRED.\n\n. . . . BYE.\n\n" ) ; 
	exit( EXIT_FAILURE ) ; 
}; 

if( truncateDuration > 0. ) truncateDurationFlag = 1 ; 

prs( ofile, "OUTPUT FILE" ) ; 


prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "CHANNELCOLLECT", 69 ) ; 
prline( 69,  "-" ) ; 

// GET NAME OF USER
user = pvc_user_tag(); 

  

// ************

arg_index_Save = arg_index ; 

//pri( arg_index, "arg_index" ) ; 

if( argc > 1   ){ 	// 

	prbanner( "INPUT SOUND FILES", 69 ) ; 

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

			numberOfInputSoundFiles++ ; 

			if( numberOfInputSoundFiles == 1 ){
	    			isr = inputSFinfo.samplerate ; 
			}else{
				if( inputSFinfo.samplerate != isr ){
					pri( inputSFinfo.samplerate, "\tSAMPLE RATE" ) ; 
					prt( "SAMPLE RATE DOES NOT MATCH PREVIOUS FILE(S).(ALL RATES MUCH MATCH.)\n\n . . . . . BYE.\n" ) ; 
					exit( EXIT_FAILURE ) ; 
				} ; 			

			} ; 
			numberOfInputChannels  = inputSFinfo.channels ;
			totalNumberOfInputChannels += numberOfInputChannels ; 

			numberOfFrames = inputSFinfo.frames ;
			duration  = (float) isr / (float) numberOfFrames ;

			fprintf( stderr, "\n -- CHANNELS: %d  SAMPLE RATE: %d FRAMES: %d  DURATION: %f", 
				numberOfInputChannels, isr, numberOfFrames, duration ) ; 


			if( numberOfInputSoundFiles > 1 ){
				if( numberOfFrames > maxNumberOfFrames ) maxNumberOfFrames = numberOfFrames ;  
				if( numberOfFrames < minNumberOfFrames ) minNumberOfFrames = numberOfFrames ;  
			}else{
				maxNumberOfFrames = numberOfFrames ;  minNumberOfFrames = numberOfFrames ;  
			} ; 

			iformat = inputSFinfo.format ; 


    			// CLOSE IT. 
    			sf_close( infile ) ;     
		} ; 

		arg_index++ ; 

	}  ;

	prline( 69,  "*" ) ; 
 

	fprintf( stderr, "\nTOTAL SOUND FILES: %d\tTOTAL CHANNELS: %d", numberOfInputSoundFiles, totalNumberOfInputChannels ) ; 

	if( totalNumberOfInputChannels > MAXIMUM_CHANNELS )
	{
	     prline( 69, "=" ); 
	     prt( "NUMBER OF CHANNELS EXCEEDS CURRENT MAXIMUM SETTING." ); 
	     prt( ""); 
	     fprintf( stderr, "\n\nIN FILE pvc_lib/pv.h, CHANGE MAXIMUM CHANNEL SETTING OF %d TO %d.\n\n",
			MAXIMUM_CHANNELS, numberOfInputChannels ) ; 
	     prt( "THEN RECOMPILE AND REINSTALL." ) ; 
	     prline( 69, "=" ); 
	     prt( ""); 
	     prt( ""); 
	     exit(EXIT_FAILURE) ; 
	} ; 

	prline( 69,  "*" ) ; 
 



	fprintf( stderr, "\nMAXIMUM DURATION: %f (FRAMES: %d)\nMINIMUM DURATION: %f (FRAMES: %d)", 
			(float) maxNumberOfFrames / (float) isr, maxNumberOfFrames, 	(float) minNumberOfFrames / (float) isr, minNumberOfFrames ) ; 
	prline( 69,  "*" ) ; 
 
	if( truncateDurationFlag == 1 ){
		truncateDurationInFrames = (int)(0.5 + ( truncateDuration * (float) isr) ) ; 
		if( truncateDurationInFrames >= maxNumberOfFrames  ){
			// TRUNCATE DURATION GREATER THAN MAX; TURN OFF TRUNCATE (NO EFFECT)
			truncateDurationFlag = 0 ; 			
		}else{
			// TRUNCATE
			maxNumberOfFrames = truncateDurationInFrames ; 
			if( minNumberOfFrames > maxNumberOfFrames ) minNumberOfFrames = maxNumberOfFrames ; 
			fprintf( stderr, "\n\nTRUNCATING INPUT CHANNELS TO %f SECONDS", truncateDuration ) ; 

		} ; 

	} ; 

	totalTime = (float) maxNumberOfFrames / (float) isr ; 


	silentBufferSize = maxNumberOfFrames - minNumberOfFrames + 1 ; 
	fvec( silentBuffer, silentBufferSize ) ; 
	for(i = 0 ; i < silentBufferSize; i++ ) silentBuffer[i] = 0. ; 


	// ZERO PEAK AMPS
	for(chan = 0; chan < totalNumberOfInputChannels; chan++ ) channelPeakAmps[chan] = 0. ; 



	// OPEN EACH FILE AND TRANSFER CHANNELS TO MONO FILES IN /tmp
	// ADD SILENCE TO EQUAL THE MAX LENGTH FOUND ABOVE.

	arg_index = arg_index_Save ; thisTempFile = 0 ; 

	while( arg_index < argc ){
	
		// GET OUTPUT SOUNDFILE NAME
		strcpy( ifile, argv[arg_index] ) ; 
		prs( ifile, "\nINPUT SOUND FILE" ); 

		// OPEN INPUT SOUND FILE IN READ MODE TO GET FORMAT. 

		totalFramesBufferedIn = 0 ; 

		if(! (infile = sf_open (ifile, SFM_READ, &inputSFinfo ))){
			// FILE DOES NOT EXIST
			prt( "------> NOT FOUND\n\n . . . . .  BYE.\n\n" ) ; 
			exit( EXIT_FAILURE ) ; 
		}else{

			numberOfInputChannels  = inputSFinfo.channels ;

			// MAKE NAME AND OPEN /tmp FILES FOR ALL CHANNELS.
			for(chan = 0; chan < numberOfInputChannels; chan++ ){
				sprintf( tempstring, "/tmp/%s.%d.InputChan.%d", user, (int) getpid(), thisTempFile + chan ) ; 
				prs( tempstring, "\t/tmp FILE NAME" ) ; 
				filesToRemove( tempstring, 0 ) ; 
				inputTempChanFiles[ thisTempFile + chan ] = fopen( tempstring, "wb" ); 
			} ; 
			
			// READ IN BLOCKS OF AUDIO DATA AND WRITE TO /tmp FILES


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
						// FIND TIME
						timeNow = (float) (totalFramesBufferedIn + n) / (float) isr ; 
						
						// FIND AMPLITUDE ENVELOPE VALUE FOR timeNow
						amplitudeEnvelope.A[ 0 ] =  fval( &amplitudeEnvelope, totalTime, timeNow );

						// APPLY ENVELOPE
						interleavedInputBuffer[ chan + (n * numberOfInputChannels) ] *= amplitudeEnvelope.A[ 0 ] ; 
							
		   				inputBufferByChannels[ chan ][ n ] = interleavedInputBuffer[ chan + (n * numberOfInputChannels) ] ; 	
						// LOOK FOR PEAK AMP IN ALL CHANNELS
						if( fabs( inputBufferByChannels[ chan ][ n ]) > channelPeakAmps[thisTempFile + chan] ) 
							channelPeakAmps[thisTempFile + chan] = fabs( inputBufferByChannels[ chan ][ n ] ) ;

					} ; 
            		} ; 

				totalFramesBufferedIn += numberOfFramesToTransfer ; 

					


				// WRITE OUT CHANNELS TO CORRESPONDING /tmp FILE
				for(chan = 0; chan < numberOfInputChannels; chan++ )
					fwrite( &inputBufferByChannels[ chan ], sizeof(float), 
							numberOfFramesToTransfer, inputTempChanFiles[ thisTempFile + chan ] ) ; 

			} ; 


			// TOP OFF WITH SILENCE IF NEEDED
			k = maxNumberOfFrames - totalFramesBufferedIn ; 
			if( outputMode == 0 ) 
				for(chan = 0; chan < numberOfInputChannels; chan++ ){
					if( k > 0 ) fwrite( &silentBuffer[0], sizeof(float), k, inputTempChanFiles[ thisTempFile + chan ] ) ;
				} ; 

			for(chan = 0; chan < numberOfInputChannels; chan++ )fclose( inputTempChanFiles[ thisTempFile + chan ] ); 


			thisTempFile += numberOfInputChannels ; 


    			// CLOSE IT. 
    			sf_close( infile ) ;     
		} ; 

		arg_index++ ; 

	}  ;


	prbanner( "NORMALIZATION", 69 ) ;
	if( normalizeFlag == 0 )fprintf( stderr, "NORMALIZATION IS OFF"); 
	if( normalizeFlag == 1 )fprintf( stderr, "NORMALIZING EACH CHANNEL SEPARATELY"); 
	if( normalizeFlag == 2 )fprintf( stderr, "NORMALIZING CHANNELS TOGETHER"); 

	// CHANNEL ORDER FILE
	prbanner( "CHANNEL OUTPUT ORDER", 69 ) ;

	if( strcmp( channelOrderFile, "") == 0  ){
		// NO CHANNEL REORDER/NUMBER FILE: USE ALL IN INPUT ORDER 

		channelOrderFlag__input_order_0__file_order_1 = 0 ; 
		fprintf( stderr, "** NO CHANNEL REORDER FILE **\n -- USING INPUT ORDER " ) ;

		if( (channelOutputLength == 0) || (channelOutputLength == totalNumberOfInputChannels) ){
			fprintf( stderr, "AND NUMBER, " ) ; channelOutputLength = totalNumberOfInputChannels ; 
		}else{
			if( channelOutputLength < totalNumberOfInputChannels )
				fprintf( stderr, "TRUNCATED TO %d CHANNELS.", channelOutputLength ) ; 
			else
				fprintf( stderr, "EXTENDED, BY LIST REPETITION, TO %d CHANNELS.", channelOutputLength ) ; 
		} ; 

		// ALLOCATE SPACE FOR INDICES
    		ivec( outputChannelIndices, channelOutputLength ) ;
		prt( "\nOUTPUT CHANNEL INDICES: \n" ) ; 
		for(i = 0; i < channelOutputLength; i++ ) {
			outputChannelIndices[i] = i % totalNumberOfInputChannels ;  
			fprintf( stderr, "%d ", outputChannelIndices[i] ) ; 
		} ; 
		prt( "" ) ; 
	} else{
		channelOrderFlag__input_order_0__file_order_1 = 1 ; 
		prs( channelOrderFile, "CHANNEL ORDER/NUMBER FILE" ) ; 
	
		// MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
    		cut_data_lines( channelOrderFile,  new_ChannelOrderFile,  1 ) ; 

  		if( (data = fopen( new_ChannelOrderFile, "r")) == NULL ){
	    		fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  channelOrderFile ) ; 
	    		exit(EXIT_FAILURE); 
		}

		// COUNT VALUES IN FILE
		numberOfOutputChannelIndices = 0 ; 
		while( fscanf( data,  " %f ",  &temp ) != EOF ) numberOfOutputChannelIndices++ ; 			

		fprintf( stderr, "\nNUMBER OF INDICES: %d", numberOfOutputChannelIndices  ) ; 

		if( numberOfOutputChannelIndices == 0 ){
			// FILE EMPTY
			prt( "ERROR: FILE EMPTY!" ) ; exit( EXIT_FAILURE ) ; 
		}else{
			fprintf( stderr, "\n -- OUTPUT WILL " ) ;
			if( (channelOutputLength == 0) || (channelOutputLength == numberOfOutputChannelIndices) ){
				fprintf( stderr, "USE ALL %d CHANNEL INDICES", numberOfOutputChannelIndices ) ; 
				channelOutputLength = numberOfOutputChannelIndices ; 
			}else{
				if( channelOutputLength < numberOfOutputChannelIndices )
					fprintf( stderr, "BE TRUNCATED TO FIRST %d CHANNELS.", channelOutputLength ) ; 
			else
				fprintf( stderr, "BE EXTENDED, BY LIST REPETITION, TO %d CHANNELS.", channelOutputLength ) ; 
		} ; 




			// ALLOCATE SPACE FOR INDICES
    			ivec( outputChannelIndices, channelOutputLength ) ;

			// READ IN INDICES
			prt( "\nOUTPUT CHANNEL INDICES (INDEXING CHANNEL INPUT ORDER):\n" ) ;  
    			for( j = 0; j < channelOutputLength ; j++ ){
				if( (j % numberOfOutputChannelIndices) == 0 ) rewind( data ) ;
				fscanf( data,  " %d ",  &outputChannelIndices[ j ] ) ; 
				fprintf( stderr,  "%d ", (int) outputChannelIndices[ j ]  ) ;
				if( (outputChannelIndices[ j ] < 0) || (outputChannelIndices[ j ] >=
								 totalNumberOfInputChannels ) ){
					prt( "ERROR: LAST CHANNEL INDEX ABOVE IS OUT OF LIST RANGE" ) ; 
					pri( totalNumberOfInputChannels, "TOTAL NUMBER OF INPUT CHANNELS." ) ;
					fprintf( stderr, "\nALLOWABLE INDEX RANGE: 0-%d", totalNumberOfInputChannels - 1 ) ; 
					exit( EXIT_FAILURE ) ; 
				} ; 
			} ; 
			prt( "" ) ; 		
		} ; 
		fclose( data ) ; 
	} ; 

// **** GAIN SCALE VALUES
	// GAIN SCALE VALUES FILE
	prbanner( "GAIN SCALE VALUES", 69 ) ;

	if( strcmp( channelGainscaleFileInDecibels, "") == 0  ){
		// NO GAIN SCALE FILE: SET FLAG TO NO GAINSCALE. 

		channelGainscaleFlag__off_0__on_1 = 0 ; 
		fprintf( stderr, "** NO GAIN SCALE FILE **\n" ) ;

		prt( "" ) ; 
	} else{
		channelGainscaleFlag__off_0__on_1 = 1 ; 
		prs( channelGainscaleFileInDecibels, "CHANNEL GAINSCALE FILE" ) ; 
	
		// MAKE NEW DATA FILE WITH COMMENTED LINES REMOVED
    		cut_data_lines( channelGainscaleFileInDecibels,  new_ChannelGainscaleFileInDecibels,  1 ) ; 

  		if( (data = fopen( new_ChannelGainscaleFileInDecibels, "r")) == NULL ){
	    		fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  channelGainscaleFileInDecibels ) ; 
	    		exit(EXIT_FAILURE); 
		}

		// COUNT VALUES IN FILE
		numberOfGainscaleFileValues = 0 ; 
		while( fscanf( data,  " %f ",  &temp ) != EOF ) numberOfGainscaleFileValues++ ; 			

		fprintf( stderr, "\nNUMBER OF GAIN SCALE FILE VALUES: %d", numberOfGainscaleFileValues  ) ; 

		if( numberOfGainscaleFileValues == 0 ){
			// FILE EMPTY
			prt( "ERROR: FILE EMPTY!" ) ; exit( EXIT_FAILURE ) ; 
		}else{
			fprintf( stderr, "\n -- GAIN SCALE VALUES WILL " ) ;
			if( (numberOfGainscaleFileValues == 0) || (numberOfGainscaleFileValues == channelOutputLength) ){
				fprintf( stderr, "WILL USE ALL %d GAIN SCALE VALUES", numberOfGainscaleFileValues ) ; 
				numberOfGainscaleFileValues = channelOutputLength ; 
			}else{
				if( numberOfGainscaleFileValues < channelOutputLength )
					fprintf( stderr, "BE TRUNCATED TO FIRST %d CHANNELS.", channelOutputLength ) ; 
			else
				fprintf( stderr, "BE EXTENDED, BY LIST REPETITION, TO %d CHANNELS.", channelOutputLength ) ; 
		} ; 



			// ALLOCATE SPACE FOR DECIBEL VALUES
    			fvec( outputChannelDecibelScalers, channelOutputLength ) ;

			// READ IN VALUES
			prt( "\nOUTPUT CHANNEL GAIN SCALE VALUES IN DECIBELS:\n" ) ;  
    			for( j = 0; j < channelOutputLength ; j++ ){
				if( (j % numberOfOutputChannelIndices) == 0 ) rewind( data ) ;
				fscanf( data,  " %f ",  &outputChannelDecibelScalers[ j ] ) ; 
				fprintf( stderr,  "%f ", outputChannelDecibelScalers[ j ]  ) ;
			} ; 
			prt( "" ) ; 		
		} ; 
		fclose( data ) ;

	} ; 



// *****



	peakChannelAmp = 0. ; 
	for(j = 0; j < channelOutputLength ; j++) 
		if( channelPeakAmps[ outputChannelIndices[ j ] ] > peakChannelAmp ) 
			peakChannelAmp = channelPeakAmps[ outputChannelIndices[ j ] ] ; 

	// PEAK AMPS IN DB
	prbanner( "(ORDERED AND AND SELECTED) OUTPUT CHANNEL PEAK AMPLITUDES", 69 ) ; 

	for(j = 0; j < channelOutputLength; j++){ 
		fprintf( stderr, "(%d) %f dB", j, amp_to_dB( channelPeakAmps[ outputChannelIndices[ j ] ] ) ) ; 
		if( channelPeakAmps[ outputChannelIndices[ j ] ] == peakChannelAmp ) fprintf( stderr, " (peak)" ) ;
		fprintf( stderr, "\n" ) ; 
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

	prs( ofile, "OUTPUT FILE NAME" ) ; 


    	outputSFinfo.samplerate = osr = isr ;  
	if( outputMode == 0 ){
		outputSFinfo.channels = channelOutputLength ;
		duration = (float) maxNumberOfFrames / (float) osr ; 
		fprintf( stderr, "\n -- CHANNELS: %d\n -- SAMPLE RATE: %d\n -- FRAMES: %d\n -- DURATION: %f", 
				outputSFinfo.channels, osr, maxNumberOfFrames, duration ) ; 
	}else{
		outputSFinfo.channels = 1 ;
	}; 


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


	// OPEN TEMP FILES
	prt( "\nOPENING TEMP FILES . . .\n" ) ; 
	for(chan = 0; chan < totalNumberOfInputChannels; chan++ ){
		sprintf( tempstring, "/tmp/%s.%d.InputChan.%d", user, (int) getpid(), chan ) ; 
		inputTempChanFiles[ chan ] = fopen( tempstring, "rb" ); 
		fprintf( stderr, "%d ", chan ) ; 
	} ; 





	if( outputMode == 0 ){
		// BEGIN INTERLEAVE
		// MAKE SPACE
		fvec( allChanOutputBlock, BLOCKSIZE * channelOutputLength ) ; 


		numFramesLeft = maxNumberOfFrames ; 
		frameNow = 0 ; 

		prt( "\nINTERLEAVING FILES . . . \n" ) ; 

		while( numFramesLeft > 0 ){
			// FRAMES LOOP
		
			// ZERO OUTPUT ARRAY
			for(i = 0; i < BLOCKSIZE * channelOutputLength; i++) allChanOutputBlock[i] = 0. ; 

			for( chan = 0; chan < channelOutputLength; chan++ ){
				//fprintf( stderr, "%d ", chan ) ; 
				// POSITION IN FILE FOR READ
				fseek( inputTempChanFiles[ outputChannelIndices[chan] ], frameNow * sizeof(float), SEEK_SET ) ;  
				// READ IN A CHANNEL BLOCK
	    			numFramesBufferedIn = fread( &tempBlock, sizeof(float), BLOCKSIZE,  
					inputTempChanFiles[ outputChannelIndices[chan] ] ) ; 
					
				// APPLY CHANNEL GAINS HERE.
				if( channelGainscaleFlag__off_0__on_1 == 1 ){
					for( i = 0; i < numFramesBufferedIn ; i++ ) 
						tempBlock[i] *= dB_to_amp( outputChannelDecibelScalers[ chan ] ) ;
				} ; 

				// NORMALIZE IF WANTED
				if( (normalizeFlag == 1) && (channelPeakAmps[ outputChannelIndices[chan] ] > 0.) ){
					for( i = 0; i < numFramesBufferedIn ; i++ ) 
						tempBlock[i] /= (channelPeakAmps[ outputChannelIndices[chan] ] * 1.01) ;  
				} ; 
				if( (normalizeFlag == 2) && (peakChannelAmp > 0.) ){
					for( i = 0; i < numFramesBufferedIn ; i++ ) tempBlock[i] /= (peakChannelAmp * 1.01) ;  
				} ; 
				if( normalizeFlag == 0 ){
					for( i = 0; i < numFramesBufferedIn ; i++ ) tempBlock[i] /= 1.01 ;  
				} ; 

				// INTERLEAVE CHANNEL BLOCK INTO OUTPUT ARRAY
				for(i = 0, k = chan; i < numFramesBufferedIn; i++, k += channelOutputLength ){ 
						allChanOutputBlock[k] = tempBlock[i] ; 
				} ; 
			};  



			// WRITE BUFFER OUT TO SOUNDFILE
			k = numFramesBufferedIn * channelOutputLength ; 
			numFramesOut += k ; 
			if( numFramesOut > (osr * channelOutputLength) ){
				fprintf( stderr, "*" ) ; numFramesOut -= (osr * channelOutputLength) ;  
			} ; 
			sf_write_float ( outfile, allChanOutputBlock, k  ) ; 

			numFramesLeft -= numFramesBufferedIn ; frameNow += numFramesBufferedIn ; 

		} ; 


		// END OF INTERLEAVE
	}else{
		// CONCATENATE
		prt( "\nCONCATENATING FILES . . . " ) ; 


		for(chan = 0; chan < channelOutputLength; chan++){
			//fprintf( stderr, "%d ", chan ) ;
			fseek( inputTempChanFiles[ outputChannelIndices[chan] ], 0, SEEK_END ) ;
			maxNumberOfFrames = ftell(inputTempChanFiles[ outputChannelIndices[chan] ]) / sizeof(float) ;
			rewind( inputTempChanFiles[ outputChannelIndices[chan] ] ) ;

			numFramesLeft = maxNumberOfFrames ; 

			while( numFramesLeft > 0 ){
				numFramesBufferedIn = fread( &tempBlock, sizeof(float), BLOCKSIZE,  
								inputTempChanFiles[ outputChannelIndices[chan] ] ) ;

				// NORMALIZE IF WANTED
				if( (normalizeFlag == 1) && (channelPeakAmps[ outputChannelIndices[chan] ] > 0.) ){
					for( i = 0; i < numFramesBufferedIn ; i++ ) 
						tempBlock[i] /= (channelPeakAmps[ outputChannelIndices[chan] ] * 1.01) ;  
				} ; 
				if( (normalizeFlag == 2) && (peakChannelAmp > 0.) ){
					for( i = 0; i < numFramesBufferedIn ; i++ ) tempBlock[i] /= (peakChannelAmp * 1.01) ;  
				} ; 
				if( normalizeFlag == 0 ){
					for( i = 0; i < numFramesBufferedIn ; i++ ) tempBlock[i] /= 1.01 ;  
				} ; 

				// WRITE BUFFER OUT TO SOUNDFILE 
				sf_write_float( outfile, tempBlock, numFramesBufferedIn  ) ; 

				numFramesLeft -= numFramesBufferedIn ; 
			} ; 
		} ; 
	} ; 

	// CLOSE SOUND FILE. 
	sf_close( outfile ) ; 


	for( chan = 0; chan < totalNumberOfInputChannels; chan++ ) fclose( inputTempChanFiles[ chan ] ); 
	filesToRemove( NULL, 1 );

	// ********

	prt( "\n\nSUCCESS!\n\n" ) ; 
	exit(EXIT_SUCCESS) ;




}else{
	// NO SOUND FILES
	usage() ; 
}  ; 


 


}

void usage()
{
	fprintf(stderr, "%s",
		"channelcollect:    \n"
		"channelcollect   [flags] [input files]\n"
		"	    Most formats accepted.\n"
		"	    (Values in brackets denote defaults.)\n"

	"	o:	output file\n"
	"		OUTPUT MODE:\n"
	"	i:	Interleave input channels into a multi-channel output file;\n"
	"			Channel interleave order corresponds to input order or -f file, if specified.\n"
	"	c:	Concatenate input channels into a one-channel output file;\n"
	"			Channel sequence order corresponds to input order or -f file, if specified.\n"
	"	n:	normalize all output channels (as selected): 0 = off, 1 = normalize channels independently, \n"
	"			2 = normalize channels together against channel with peak amplitude. [0]\n"
	"	d: 	maximum truncate duration [off]\n"
	"	a: 	amplitude envelope file (optional) [off]\n"
	"	f:	optional order/selection file for output channels\n"
	"			The ASCII integer file determines the selection, number, and order of output channels;\n"
	"			input channels are indexed as listed, beginning at 0. Input channel order serves as default.\n"
	"	l:	number of output channels  (optional) [all]\n"
	"			When  left unspecified, full input or file index list of channels is output, by default.\n"
	"			Values less than list length truncate output; values greater than list length are extended by\n"
	"			padding through repeat of output channel list, as needed.\n"         
	"	g:	optional decibel gain values file for output channels, as selected and ordered;\n"
	"			The ASCII file of decibel values are applied to the output channels, as listed\n"
	"			or as selected, ordered, and limited in number by the -f flag file.\n"
	"			Lists longer than output files list are truncated; shorter lists are extended\n"
	"			through repetition of the list, as needed.\n"
	);
	exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }



