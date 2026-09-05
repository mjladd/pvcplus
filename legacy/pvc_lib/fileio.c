#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pv.h"
#include <sndfile.h>
#include <string.h>
#include <ctype.h>
#include "stdbool.h"

#include <unistd.h>


#define AMPSTATINC .25
#define OSCILBANKGAIN 1.7782794



// CRACK STUFF
extern int arg_index;
extern char *arg_option;
extern char *pvcon;
extern char *index();

int bannerflag=0 ;
 
void banneri(){
    if( bannerflag == 0 )
    fprintf( stderr,  "\n\n*************************************\n*********  INPUT SOUNDFILE  **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}

void bannero(){
    if( bannerflag == 0 )
    fprintf( stderr,  "\n\n*************************************\n*********  OUTPUT SOUNDFILE  **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}


int getInputFileDataToSetOutputChannels(int argc, char **argv)
{

    static int first=1;
    SF_INFO inputSFinfo ;


    if( first == 0 ) return(0) ; 
    first = 0 ; 

//prs( argv[ arg_index ], "argv[ arg_index ]" ) ; 

	// GET SOUNDFILE NAME AND HEADER INFO
    strcpy( ifile, argv[ arg_index ] ) ; // INPUT SOUND FILE NAME


	// OPEN INPUT FILE; READ HEADER DATA INTO inputSFinfo STRUCTURE.
    inputSFinfo.format = 0 ; 
//prt( "H0"); 
    if (! (infile = sf_open (ifile, SFM_READ, &inputSFinfo )))
    {   
		// OPEN FAILED; PRINT ERROR MESSAGE AND EXIT.
		banneri(); 
		fprintf(stderr,  
		"\n\n********\nCANNOT OPEN INPUT SOUNDFILE:\n\n------>  %s <------\n********", ifile);
		// Print the error message from libsndfile.
		puts (sf_strerror (NULL)) ;
		exit(EXIT_FAILURE) ; 

    } else {

		// TRANSFER SF HEADER DATA INTO INPUT FILE VARIABLES.
		ichan= inputSFinfo.channels; // NUMBER OF INPUT CHANNELS
		isr= inputSFinfo.samplerate; // S-RATE
		idur = (float) ((double) inputSFinfo.frames / (double) inputSFinfo.samplerate) ;
		iformat = inputSFinfo.format;

//fprintf( stderr, "beginchan : %d, ichan: %d, isr: %d, idur: %f", beginchan , ichan, isr, idur ) ; 

		// TEST FOR TOO MANY CHANNELS.
        if( ichan > MAXIMUM_CHANNELS )
        {
             prline( 69, "=" ); 
             prt( "NUMBER OF CHANNELS EXCEEDS CURRENT MAXIMUM SETTING." ); 
             prt( ""); 
             fprintf( stderr, "\n\nIN FILE pvc_lib/pv.h, CHANGE MAXIMUM CHANNEL SETTING OF %d TO %d.\n\n",
			MAXIMUM_CHANNELS, ichan ) ; 
             prt( "THEN RECOMPILE AND REINSTALL." ) ; 
             prline( 69, "=" ); 
             prt( ""); 
             prt( ""); 
             exit(EXIT_FAILURE) ; 
        } ; 



		// TEST FOR CHANNEL COMPATIBILITY.
        if( beginchan >= ichan ) 
        {
              fprintf( stderr, "\n\nTHE AVAILABLE CHANNELS ARE 1-%d, NOT %d. BYE.\n\n",
	          ichan, beginchan + 1 ) ;
             exit(EXIT_FAILURE)  ;
        } ; 

 	// SET/INTERPRET BEGIN/END TIMES. 
	if( endt <= 0. ) endt = idur ; 
	if( begint <= 0. ) begint = 0. ; 

    	prbanner( "INPUT SOUNDFILE",  69 ) ; 
	prs( ifile,  "INPUT FILE: FILENAME " );

	fprintf( stderr, "\n" ) ;
	{
		SF_FORMAT_INFO format_info ;
		format_info.format = inputSFinfo.format ;
		if( sf_command( NULL, SFC_GET_FORMAT_INFO, &format_info, sizeof( format_info ) ) == 0 && format_info.name != NULL ){
			prs( (char *) format_info.name, "INPUT FILE: FORMAT" ) ;
		}
	}
	pri( (int) inputSFinfo.frames, "INPUT FILE: FRAMES" ) ;

	pri( isr,  "INPUT FILE: SAMPLE RATE" ) ;
	pri( ichan,  "INPUT FILE: NUMBER OF CHANNELS" ) ; 

//        fprintf( stderr, "INPUT FILE: FORMAT = 0x%08X\n", inputSFinfo.format) ;




	prf( idur,  "INPUT FILE: DURATION" ) ; 
	prf( begint,	"INPUT FILE: BEGIN TIME" ) ; 
	prf( endt,	"INPUT FILE: END TIME"	 ) ; 



        } ;
	
		// TEST BEGIN AND END TIMES.
        if( (endt != -1.) && (endt <= begint)){
	        banneri() ;
	        fprintf( stderr, "\n\nBEGIN TIME = %f\nEND TIME = %f\n\n",  begint,  endt  ) ;

	        fprintf( stderr,
	           "\n\nYOUR END TIME IS BEFORE OR EQUAL TO YOUR BEGIN TIME! BYE.\n\n" ) ;
	        exit(EXIT_FAILURE) ; 
        } ;
	

		// TEST END TIME. 
        if( endt > idur ){
	    banneri() ;
	    fprintf( stderr, "%s%f%s%f%s", "\nYOUR END TIME OF  ", 
		endt, 
		"  SECONDS \nEXCEEDS THE FILE DURATION OF  ", 
		idur, 
		" SECONDS.\nYOUR END TIME WILL BE RESET TO THE DURATION OF THE FILE.\n\n" 
		 ) ;
	    endt = idur ; 
        } ; 

        end_sample = (int) (endt * (float) isr ) ; 
        sample = begin_sample =  (int) (begint * (float) isr ) ;
 
	// SET FILE POINTER TO BEGIN FRAME. 
//prt( "H1"); 
	sf_seek( infile, begin_sample, SEEK_SET ) ; 	

//pri( channelflag, "channelflag" ) ; 


		// SET OUTPUT FILE VALUES
        if( outputoff == 1 ) {
        	// NO AUDIO  FILE OUTPUT: SET ochan AND EXIT.
//	    	fprintf( stderr, "\n\n...................NO AUDIO OUTPUT FILE\n" ) ; 
           if( (channelflag == 0) || (channelflag == -2) ){
	        	ochan = ichan ; 
	    	}else{
	        	ochan = 1 ; 
           }
        } else{     
		// NUMBER OF OUTPUT CHANNELS
		if( channelflag == 0 ){
			// SET OUTPUT NUMBER TO INPUT
			ochan = ichan ; 
    	    	}else if( channelflag == -2 ){
			// PRESELECTED
			ochan = ichan ; 
    	    	}else if( channelflag == -1 ){
			// PRESELECTED
			ochan = ochan ; 
    	    	}else{
			// JUST ONE
			ochan = 1 ; 
    	    	}
       } ; 

       endchan = beginchan + ochan ;

//pri( beginchan, "IN get... beginchan" ) ; 
//pri( endchan, "IN get... endchan" ) ; 

	return(1) ; 

} ; 



// ************************************************************

void setupfiles(int argc, char **argv)
{
    char *user,
		scratchString[ STRING_SIZE ];

    int		n,
                j,
		numberOfSampsBufferedIn,
		numberOfFramesBufferedIn,
		framesLeftToBuffer,
		numberOfFramesToBuffer,
		thisoutchan		
    ;
	
     
		
    float	temp    ;

     // HERE?

   FILE *inputFilePointer, *tempFilePointer ;
   long size, sizeCopy ; 
   int beginchansave ;

    
   SF_INFO 	outputSFinfo ; 

   

   int numSilentBuffers, remainingSamps ; 
   float silentBuffer[ BLOCKSIZE ] ; 


   float interleavedInputBuffer [ BLOCKSIZE * MAXIMUM_CHANNELS ] ; // HERE?
   float inputBufferByChannels [ MAXIMUM_CHANNELS ][ BLOCKSIZE ] ; 

    formatSwitchflag = 0 ; 

//prs( argv[ arg_index ], "argv[ arg_index ]" ) ; 

//pri( endchan, "IN setupfiles BEFORE getInputFileDataToSetOutputChannels: endchan" ) ; 
//pri( beginchan, "IN setupfiles BEFORE getInputFileDataToSetOutputChannels: beginchan" ) ; 

   getInputFileDataToSetOutputChannels(argc, argv); 
        
//pri( endchan, "IN setupfiles AFTER getInputFileDataToSetOutputChannels: endchan" ) ; 
//pri( beginchan, "IN setupfiles AFTER getInputFileDataToSetOutputChannels: beginchan" ) ; 


	// OPEN /tmp INPUT FILES--ONE FOR EACH CHANNEL BEING PROCESSED (EITHER ALL OR ONE.)
	user = pvc_user_tag(); 
	for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ ){
 	    	sprintf( scratchString, "/tmp/%s.%d.InputChan.%d", user, (int) getpid(), thisoutchan ) ;
		inputTempChanFiles[ thisoutchan ] = fopen( scratchString, "wb" );
        		strcpy( inputTempChanFileNames[ thisoutchan ], scratchString ) ; 
	} ; 	

//	fprintf( stderr, "\n/tmp FILES OPENED. \n") ; 

//	for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
//	    fprintf( stderr, "HERE INPUT FILE: %s\n", inputTempChanFileNames[ thisoutchan ] ) ; 

        //


	// READ IN BLOCKS OF INPUT FILE AND DE-INTERLEAVE INTO /tmp FILES.
        framesLeftToBuffer = end_sample - begin_sample ;  
//prt( "H2"); 
	while( (numberOfSampsBufferedIn = 
			sf_read_float (infile, interleavedInputBuffer, BLOCKSIZE * ichan ))
                  &&
              (framesLeftToBuffer > 0)
	){   

	    numberOfFramesBufferedIn = numberOfSampsBufferedIn / ichan ; 
		// DE-INTERLEAVE interleavedInputBuffer INTO inputBufferByChannels.
//	    fprintf( stderr, "numberOfFramesBufferedIn: %d, endchan: %d\n", numberOfFramesBufferedIn, endchan ) ; 
            for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ ){

//			fprintf( stderr, "thisoutchan NOW: %d .......\n", thisoutchan ) ; 

			for( n = 0; n < numberOfFramesBufferedIn; n++ ){
		   		inputBufferByChannels[ thisoutchan ][ n ] = interleavedInputBuffer[ thisoutchan + (n * ichan) ] ; 	
              	} ; 
            } ; 


            if( framesLeftToBuffer >= numberOfFramesBufferedIn )
            { numberOfFramesToBuffer = numberOfFramesBufferedIn ; }
            else { numberOfFramesToBuffer = framesLeftToBuffer ; } ;  

	     // WRITE OUT CHANNELS TO CORRESPONDING /tmp FILE
	     for( thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
		fwrite( &inputBufferByChannels[ thisoutchan ], sizeof(float), 
				numberOfFramesToBuffer, inputTempChanFiles[ thisoutchan ] );
             framesLeftToBuffer = framesLeftToBuffer - numberOfFramesToBuffer ; 
        } ;
//	fprintf(stderr, "FILES WRITTEN TO /tmp \n" ) ; 
	// ADD RING TIME IF GREATER THAN 0.0 ; 
	if( ringTime > 0.0 ){
//prf( ringTime, "ringTime" ) ; 
//pri( isr, "isr" ) ; 

	    // FIND THE NUMBER OF SILENT BUFFERS NEEDED
	    ringTimeSamples = (int)(ringTime * (float) isr) ; 
	    numSilentBuffers = ringTimeSamples / BLOCKSIZE ;
	    remainingSamps =  ringTimeSamples - (numSilentBuffers * BLOCKSIZE) ;  
//pri( ringTimeSamples, "ringTimeSamples" ) ; 
//pri( numSilentBuffers, "numSilentBuffers" ) ; 
//pri( remainingSamps, "remainingSamps" ) ; 
	    // FILL SILENT BUFFER.
	    for( n = 0; n < BLOCKSIZE; n++ ) silentBuffer[n] = 0.0 ; 
	    // FOR EACH OUTPUT CHANNEL FILE.....
	    for( thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ ){
		// WRITE OUT THE NECESSARY NUMBER OF SILENT BUFFERS
		for( n = 0; n < numSilentBuffers; n++ ){		
	        	fwrite( &silentBuffer, sizeof(float), 
				BLOCKSIZE, inputTempChanFiles[ thisoutchan ] );	
		} ; 
		for( n = 0; n < remainingSamps; n++ )
	        	fwrite( &silentBuffer[0], sizeof(float), 
				1, inputTempChanFiles[ thisoutchan ] );	
		 
	    } ; 
	} ; 

        // CLOSE INPUT CHANNEL FILES.
	for( thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
	   fclose( inputTempChanFiles[ thisoutchan ] );

//	fprintf(stderr, "/tmp FILES CLOSED. \n" ) ; 

	if( expandSingleChannelInputFileToMultipleDuplicateChannels ){

 
                // MOVE /tmp FILE INTO ALTERNATE FILE.
	   inputFilePointer = fopen( inputTempChanFileNames[ beginchan ], "rb" ) ; 
	   fseek( inputFilePointer, 0, SEEK_END ) ; 
 	   size = ftell( inputFilePointer ) ; sizeCopy = size / sizeof(float) ;
//	   fprintf( stderr, "\n size: %ld", size );  
	   fseek( inputFilePointer, 0, SEEK_SET );

    	   sprintf( scratchString, "/tmp/%s.%d.tempChan", user, (int) getpid() ) ;  
	   tempFilePointer = fopen( scratchString, "wb" ) ;  

	   while( sizeCopy > 0 ){
	      fread( &temp, sizeof(float), 1, inputFilePointer ) ; 
	      fwrite( &temp, sizeof(float), 1, tempFilePointer ) ;
	      sizeCopy-- ;
 	   } ; 
 
	   fclose( inputFilePointer ) ; fclose( tempFilePointer ) ;
	   tempFilePointer = fopen( scratchString, "rb" ) ;  


	   beginchansave = beginchan ; 
	   ochan = ichan = ochanModified ;
	   beginchan = 0 ; endchan = beginchan + ochan ; 
 

	   for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ ){
 	      sprintf( scratchString, "/tmp/%s.%d.InputChan.%d", user, (int) getpid(), thisoutchan ) ;
        	      strcpy( inputTempChanFileNames[ thisoutchan ], scratchString ) ;
	      inputTempChanFiles[ thisoutchan ] = fopen( inputTempChanFileNames[ thisoutchan ], "wb" );
 
	   } ; 	


	   sizeCopy = size / sizeof(float) ; 
	   while( sizeCopy > 0 ){
	      fread( &temp, sizeof(float), 1, tempFilePointer ) ; 
	      for( thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
 	      { 
	         fwrite( &temp, sizeof(float), 1, inputTempChanFiles[ thisoutchan ] ) ;
	      } ;
	      sizeCopy-- ;
 	   } ; 
	   for( thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
	      fclose( inputTempChanFiles[ thisoutchan ] );
	   fclose( tempFilePointer ) ; 
	   fprintf( stderr, "\nINPUT CHANNEL %d HAS BEEN DUPLICATED TO %d CHANNELS.", 
		beginchansave + 1, ochan ) ;


/*
	   sprintf( tempFileName, "/tmp/%s.%d.tempChan", user, (int) getpid() ) ;  
	   sprintf( scratchString, "mv %s %s", 
	      inputTempChanFileNames[ beginchan ], tempFileName
	   ) ;
//	   prs( scratchString, "scratchString" ) ;  
//	   system( scratchString ) ;
 
	   ochan = ichan = ochanModified ;
	   beginchan = 0 ; endchan = beginchan + ochan ; 
                for( thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
 	   {
	      sprintf( thisFileName, "/tmp/%s.%d.InputChan.%d", user, (int) getpid(), thisoutchan ) ;
	      strcpy( inputTempChanFileNames[ thisoutchan ], thisFileName ) ;
//	      prs( thisFileName, "### thisFileName" ) ; 
	      sprintf( scratchString, "%s ; cp %s %s", scratchString, tempFileName, thisFileName ) ; 
	   } ; 
	      prs( scratchString, "$$$ scratchString" ) ;  
	      system( scratchString ) ;   
	   fprintf( stderr, "SINGLE INPUT CHANNEL HAS BEEN DUPLICATED TO %d CHANNELS.", ochan ) ; 
*/
	} ;

	// CLOSE INPUT FILE
	sf_close (infile) ;

//	fprintf(stderr, "INPUT FILE CLOSED. \n" ) ; 


	// ZERO PEAK AMPS ON INPUT FILE
	for( thisoutchan = beginchan, j = 0; thisoutchan < endchan; thisoutchan++, j++ )
                     ipeakamp[ j ] = 0. ; 


// ****************************************************************
// *************** SET UP OUTPUT FILE
// ****************************************************************

    if( outputoff == 1 ) return ; 

    arg_index++ ; 

    if( arg_index >= argc  ){ 
		// OUTPUT FILE NOT SPECIFIED; USE DEFAULT.
		bannero() ;
		sprintf( ofile,  "pvc.out" )  ; 
		fprintf( stderr, "\n\n......USING DEFAULT OUTPUT FILENAME........\n\n" ) ;
		// SET FLAG TO DENOTE THAT THERE IS NO SPECIFIED OUTPUT FILE. 
		oformat = iformat ; outputSFinfo.format = oformat ;
    }else{
	// GET OUTPUT SOUNDFILE NAME
	  strcpy( ofile, argv[arg_index] ) ; 

        // GET THE FILE FORMAT FROM THE SPECIFIED OUTPUT FILE AND LOG SUCCESS.
        // IF FILE DOES NOT EXIST, LOG FAILURE TO GET FORMAT, WHICH WILL CAUSE
        // INPUT SOUND FILE FORMAT TO BE USED INSTEAD. 

        // OPEN OUTPUT SOUND FILE IN READ/WRITE MODE; GET FORMAT. 
//		prs( ofile, "A1. OPENING SOUND FILE (FOR FORMAT)" ) ; 
//prt( "H4"); 
        if (! (outfile = sf_open (ofile, SFM_READ, &outputSFinfo )))
        {   
			// PRE-EXISTING SOUND FILE NOT FOUND.  USE INPUT FILE FORMAT INSTEAD. 
               fprintf (stderr, "\n\nFORMAT-SUPPLYING OUTPUT SOUND FILE %s  WAS NOT FOUND.\n", ofile ) ;
 			fprintf( stderr, "FILE FORMAT WILL BE COPIED FROM INPUT SOUND FILE INSTEAD. \n" ) ; 
			fprintf( stderr, "\n==> NOTE: FILE NAME EXTENSION MAY NOT MATCH HEADER FORMAT. <==\n\n" ) ; 
	           bannero() ;
                 formatSwitchflag = 1 ; 
                // SET OUTPUT FORMAT AND RATE TO MATCH INPUT FILE.
                oformat = iformat ; outputSFinfo.format = oformat ;

        }else{
		    // PRE-EXISTING FILE FOUND: SET OUTPUT FORMAT AND RATE TO MATCH OUTPUT FILE.
               oformat = outputSFinfo.format ;



//		format_info.format = outputSFinfo.format ;
//		sf_command (NULL, SFC_GET_FORMAT_MAJOR, & format_info, sizeof (format_info)) ;
//		printf ("%s  (extension \"%s\")\n", format_info.name, format_info.extension) ;


       } ;

        // CLOSE IT. 

// pri( outfile, "outfile" ) ;  
//		prs( ofile, "B4. CLOSING SOUND FILE" ) ; 
//prt( "H5"); 
        sf_close( outfile ) ;     
    } ;  



    // SET OUTPUT FORMAT AND RATE TO MATCH INPUT FILE. ??? HERE setupfiles
//    oformat = iformat ; outputSFinfo.format = oformat ;

    osr = isr ; outputSFinfo.samplerate = osr ; 

   if( channelflag == -2 )
	outputSFinfo.channels = 1 ;
    else
	outputSFinfo.channels = ochan ;

//pri( outputSFinfo.channels,  "BEFORE CREATE OF OUTPUT FILE: NUMBER OF CHANNELS" ) ; 
//prt( "H6"); 


// HERE FOR BEGIN OF TEST MOVE TO END OF SYNTHESIS IN bufferout

    if( ! ( sf_format_check (&outputSFinfo) ) )
    {
        fprintf( stderr, "\nBEFORE CREATE OF OUTPUT FILE: INVALID SOUND FILE FORMAT\n" ) ; 
    } ; 


    // OPEN/CREATE THE OUTPUT FILE.
//	prs( ofile, "A2. OPENING SOUND FILE (TO WRITE NEW HEADER)" ) ; 
//prt( "H7"); 

    if (! (outfile = sf_open (ofile, SFM_WRITE, &outputSFinfo )))
//    if (! (outfile = sf_open (ofile, SFM_RDWR, &outputSFinfo )))
    {   printf ("1. Not able to open output file %s.\n", ofile ) ;
	    bannero() ;
           puts (sf_strerror (NULL)) ;
           exit(EXIT_FAILURE) ;
    } ;

    prbanner( "OUTPUT SOUNDFILE",  69  ) ; 
    prs( ofile,  "OUTPUT FILE: FILENAME " );
    pri( osr,  "OUTPUT FILE: SAMPLE RATE" ) ; 
    pri( outputSFinfo.channels,  "OUTPUT FILE: NUMBER OF CHANNELS" ) ; 
   
    // POSITION AT BEGINNING.
//prt( "H8"); 
    sf_count_t frames = 0 ; 
//prt( "H9"); 
    n = sf_command(outfile, SFC_FILE_TRUNCATE, &frames, sizeof (frames)) ; 

//    if( n == 0 ) { fprintf( stderr, "TRUNCATE SUCCESSFUL!\n" ) ; } 
//    else { fprintf( stderr, "TRUNCATE NOT SUCCESSFUL!\n" )   ; } ; 
 


//	prs( ofile, "B1. CLOSING SOUND FILE" ) ; 
//prt( "H10"); 
//    sf_close( outfile ) ; // HERE

// END OF POSSIBLE TEST MOVE

   // CREATE AND CLOSE /tmp OUTPUT FILES--ONE FOR EACH CHANNEL.
   user = pvc_user_tag(); 
   for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ ) {
	sprintf( scratchString, "/tmp/%s.%d.OutputChan.%d", user, (int) getpid(), thisoutchan ) ;
//	fprintf( stderr, "FILE: %s\n", scratchString ) ;
        outputTempChanFiles[ thisoutchan ] = fopen( scratchString, "wb" ); 
	strcpy( outputTempChanFileNames[ thisoutchan ], scratchString ) ; 
        fclose( outputTempChanFiles[ thisoutchan ] ) ; 
//	fprintf( stderr, "FILE: %s\n", outputTempChanFileNames[ thisoutchan ] ) ;
   } ; 	


}

int rescaleThisBuffer( float peakamp[], float outputBufferByChannels[][ BLOCKSIZE ], int bufferinsamps ){

    static float peakofout,  peakifout, ampval, tempy ;
    static int first=1 ; 
    int j,jc, thisoutchan ; 
    
    // FIND PEAKS
    if( first == 1 )
    {

//fprintf( stderr, "RESCALE SETUP........................\n" ) ; 
        prline( 69,  "-" ) ;

        // FIND GREATEST PEAK AMONG CHANNELS OF OUTPUT FILE
//for(j = 0;  j < ochan; j++)prf( peakamp[j], "RESCALE SETUP: PEAKAMPS" )  ; 

        peakofout = -999999. ; 
        for(j = 0;  j < ochan; j++)
            if( peakamp[ j ] >  peakofout ) peakofout =  peakamp[ j ] ; 

	if( peakofout <= 0. ) {
             fprintf( stderr, "\n\n PEAK OUTPUT AMP IS 0. RESCALE ABORTED.\n\n" ) ; 
             return( 0 ) ; 
	} ; 
	    
	// FIND GREATEST PEAK AMONG CHANNELS OF INPUT FILE

        peakifout = -999999. ; 
        for(j = 0;  j < ochan; j++)
             if( ipeakamp[j] >  peakifout ) peakifout =  ipeakamp[j] ; 

        if( peakifout <= 0. ) {
                fprintf( stderr, "\n\n PEAK INPUT AMP IS 0. RESCALE ABORTED.\n\n" ) ; 
                return( 0 ) ; 
        } ; 
	    
	// MAKE AMP RESCALE VALUE

	if( rescalev == 1. ){	// 1
	    // RESCALE TO PEAKAMP OF INPUT FILE 
	    ampval = peakifout  /  peakofout  ; 
	    prf( amp_to_dB( peakifout ), "(INPUT SOUND FILE) AMPLITUDE RESCALE LEVEL (in dB) " ) ;  

	}else if( rescalev == 3. ){	// 3
		if( peakofout > 0.9999 ){
			ampval = 0.9999 / peakofout ; 
			prf( amp_to_dB( 0.9999 ), "\nPEAK EXCEEDS 0dB, TRIGGERED RESCALE LEVEL (in dB)\n" ); 
		}else{
			ampval = 1.0 ; 
		} ; 

	}else{
	    // USE INPUT RESCALE VALUE: CONVERT TO AMP
	    tempy = dB_to_amp( rescalev ) ; 
	    ampval = tempy / peakofout   ; 

	    prf( amp_to_dB( tempy ), "(USER-SPECIFIED) AMPLITUDE RESCALE LEVEL (in dB)" ) ;  
	} ; 

	// RESET PEAKAMP LEVELS  AND SAMPLES OVER FOR NEW DETECTION
	for(j = 0;  j < ochan; j++) peakamp[ j ] = -999999. ; 
	for(j = 0;  j < ochan; j++) nsover[ j ] = 0. ; 

        first = 0 ; 
        
    } ; 

	// ******** RESCALE BLOCK
        for(thisoutchan = beginchan, jc = 0; thisoutchan < endchan; thisoutchan++, jc++ )
        {
             for( j = 0; j < bufferinsamps; j++ )
             {
                 outputBufferByChannels[ thisoutchan ][ j ] *= ampval ; 

                 // GET NEW STATS ON OUTPUT FILE
                 if ( fabs( outputBufferByChannels[ thisoutchan ][ j ] )  > peakamp[ jc ] )
                 {
                     peakamp[ jc ] = fabs( outputBufferByChannels[ thisoutchan ][ j ] ) ; 
                 } ; 
                 // CHECK FOR OVERFLOW AND CLIP IT AND COUNT IT
                 if( fabs( outputBufferByChannels[ thisoutchan ][ j ] )  > 1.0 ){
                     // CLIP
                     if( outputBufferByChannels[ thisoutchan ][ j ] > 1.0 ) 
                           outputBufferByChannels[ thisoutchan ][ j ] = .9999999 ; 
                     if( outputBufferByChannels[ thisoutchan ][ j ] < -1.0 ) 
                           outputBufferByChannels[ thisoutchan ][ j ] = -.999999 ; 
                     // COUNT IT
                     nsover[ jc ]++ ; 
                 }

             } ; 
        } ; 
    
    return( 1 ) ; 
}





int bufferout(float *outbuff,  int I, int flushflag)
{
   
	float outputBufferByChannels [ MAXIMUM_CHANNELS ][ BLOCKSIZE ] ; // HERE?
	float interleavedOutputBuffer [ BLOCKSIZE * MAXIMUM_CHANNELS ] ; // HERE?

	  
	 
	int bufferinsamps ; 

	

	static int	numsamps=0, 
		blkcount=0,
		n, j, 
		flag=0, 
		nblockover=0,
		blockpeakn, 
		outbuffpt,     
		first=1,
		first2=1,  
		buffoutchan=0,
		finaloutchan,
		thisoutchan
		 ; 
    
	 

	static float    in[ BLOCKSIZE ] 
		    ; 
    		
	static float blockpeakt,
		peakampt[ MAXIMUM_CHANNELS ],   
		peakamp[ MAXIMUM_CHANNELS ],
		lastpeakamp=0, 
		nextt,
		blockpeakamp=0,
		statpeakamp=0, 
		temp, 
		gain    
	;

	

	SF_INFO 	outputSFinfo ; 


    

	if( (buffoutchan != channow) || (first) )
	{
		nextt = ampstatinc ;
		peakamp[channow]=0 ;
		lastpeakamp=0 ; 
		blockpeakamp=0 ;
		statpeakamp=0 ;
		nsover[channow]=0;
		numsamps = 0 ;
		flag=0;
		first = 0 ; 
		first2 = 1 ;  
		buffoutchan = channow ;		
		if(oscilbankon){
			gain =  OSCILBANKGAIN ;
		} else {
			gain = 1. ; 
		} ; 
	} ; 

	if((quiet != 0) && (first2))
	{
		fprintf( stderr,  "\n*********************************************************************");
		fprintf( stderr,  "\n**  PEAK AMPLITUDE STATISTICS **" ) ; 
		fprintf( stderr,  "\n*********************************************************************");
		fprintf( stderr, "\n     TIME          PEAKAMP      DECIBELS    (LAST DECIBELS PEAK)" );
		fprintf( stderr,  "\n*********************************************************************");
		first2 = 0 ; 
	} ; 

	outbuffpt = 0 ; 

	while( outbuffpt < I )
	{
//fprintf( stderr, "H1\n" ) ; 
		// TRANSFER VALUES INTO in[]
		while( (numsamps < BLOCKSIZE) && ( outbuffpt < I ) )
		{
			in[ numsamps ] = outbuff[ outbuffpt ] * gain  ; 
			numsamps++; outbuffpt++ ; samps++ ; 
//pri( samps, "samps" ) ; 
		} ; 

		if( (numsamps < BLOCKSIZE) && (flushflag == 0) ){
			// in BUFFER NOT YET FULL AND WE ARE NOT FLUSH/CLOSING
			return(0) ;	 
		}else{
		
			nblockover = 0 ; blockpeakamp = 0 ; blockpeakn = 0 ; 
		
			// EXAMINE EVERY SAMPLE FOR THIS CHANNEL AND GATHER STATS. 
			for( n = 0;  n < numsamps; n++ ){
				temp = (float) fabs( (double) in[ n ] ) ; // ABS VALUE
				if(temp > peakamp[channow]) 
				{
					peakamp[channow] = temp ;
					peakampt[channow] = t ; 
				} ;
				if(temp > blockpeakamp)
				{
					blockpeakamp = temp ; 
					blockpeakt =  t ; 
				} ;
				if(temp > statpeakamp) 
				{
					statpeakamp = temp ; 
				} ;


				// CLIPPING STATS
				if( rescalev == 2 ) 
				{
					if( in[ n ] > 1. ){ in[ n ] = 1. ; nsover[channow]++ ; nblockover++ ; } ; 
					if( in[ n ] < -1. ){ in[ n ] = -1. ; nsover[channow]++ ; nblockover++ ;  } ;  
				} ; 

				// AMP STATS
				if( (t >= nextt) && (quiet != 0) && ( flag == 0 ) )
				{
					temp = (float) (20. * log10( (double) statpeakamp ))  ; 
					fprintf( stderr,  "\n(%6.2f -%6.2f)   ", nextt-ampstatinc, nextt ) ; 
					if( temp < -100 )
					{
						fprintf( stderr, "   *   " ) ;
					}else{			
						fprintf( stderr,  "%7.4f       %7.3f", statpeakamp,  temp ) ; 
					} ; 
					nextt += ampstatinc ; statpeakamp = 0 ;  

					if(peakamp[channow] > lastpeakamp)
					{
						temp = (float) (20. * log10( (double) peakamp[channow] )) ; 
						if( temp > -100 ){
							fprintf( stderr,  "     %7.3f", temp   ) ; 
							lastpeakamp = peakamp[channow] ;
						} ;  
					} else {
						fprintf( stderr, "            " ) ; 
					} ;
//					fprintf( stderr, "  " ) ;	
//					for( i = 0; i < (1 + (int)((96.0 + temp) / 2.0)); i++) fprintf( stderr, "*" ) ; 
			
				} ; 
            	} ; 

 	           // WRITE OUT BLOCK TO OUTPUT CHAN FILE IN /tmp.
            	fwrite( &in, sizeof(float), numsamps, outputTempChanFiles[ outchan ] );

            	blkcount++ ; 
            	numsamps = 0 ; 

            	// PRINT WARNING IF SAMPLES OUT OF RANGE
            	if( ( flag == 0 ) && (nsover[channow] > 0) )
           	{
	        		flag=1; 
	        		fprintf( stderr, "\n*** SAMPLES OUT OF RANGE ***** \n   (Samples out of range will be clipped.)" ) ; 
	        		fprintf( stderr,  "\n*********************************************************************");
	        		fprintf( stderr, "\n     TIME        PEAKAMP     DECIBELS      NUMBER_OF_SAMPLES" );	    
	        		fprintf( stderr,  "\n*********************************************************************");
	    		} ; 
	    		if( nblockover > 0 )
            	{
	        		fprintf( stderr,  "\n-> %6.3f        %7.4f       %7.3f      %d", 
				blockpeakt, blockpeakamp, (float) (20. * log10( (double) blockpeakamp )), nblockover )     ; 
	    		} ; 

        	} ; 

    	} ; 

    	if(flushflag == 0)
    	{
//		fprintf( stderr, "H6\n" ) ; 
        	// NO FLUSH/CLOSE

        	return(1) ; 


    	}else{
//		fprintf( stderr, "H7\n" ) ; 
        	//  CLOSE

        	// PRINT WARNING IF SAMPLES OUT OF RANGE
		if( (blkcount > 0) && (quiet != 0))
        	{
	    		fprintf( stderr, "\n\n============= PEAK AMPLITUDE ========================================" ) ; 
	    		fprintf( stderr, "\nCHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)" );	    
	    		fprintf( stderr,  "\n.....................................................................");

            	fprintf( stderr,  "\n%d          %7.3f          %7.4f     %7.3f", 
		    	channow+1,  
			peakampt[channow], peakamp[channow], (float) (20. * log10( (double) peakamp[channow] )) )     ;
            	if( nsover[channow] > 0 )fprintf( stderr, "     ---> %d <---", nsover[channow] ) ;  

	    		fprintf( stderr,  "\n*********************************************************************");
		} ; 
		fprintf( stderr,  "\n\n" ) ; 


        	if( (channow + 1)  < ochan )
        	{
            	// MULTI-CHANNEL
//	    		prt( "\nDO NEXT CHANNEL" ) ; 
            	// RESET INPUT FILE

		} else {
            	//CLOSE EACH /tmp OUTPUT FILE. 
//            	prt( "\nCLOSE EACH /tmp OUTPUT FILE.\n"); 
            	for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
            	{
	       		fclose( outputTempChanFiles[ thisoutchan ] );
            	} ; 

            	if(quiet != 0)
            	{
                		prline( 69,  "=" ) ;
                		fprintf( stderr, "\n\n                 PEAK AMPLITUDES: ALL CHANNELS" ) ; 
                		prline( 69,  "-" ) ;
                		fprintf( stderr, "\nCHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)" );	    
                		fprintf( stderr,  "\n.....................................................................");

                		for(j = 0;  j < ochan; j++)
                		{
                    		fprintf( stderr,  "\n%d          %7.3f          %7.4f     %7.3f", 
	        	    		j+1,  peakampt[j], peakamp[j], (float) (20. * log10( (double) peakamp[j] )) )     ;
                    		if( nsover[j] > 0 )fprintf( stderr, "     ---> %d <---", nsover[j] ) ;  
                		} ; 
                		prline( 69,  "=" ) ;
            	} ; 
            	fprintf( stderr,  "\n\n" ) ; 

            	// INTERLEAVE /tmp CHANNEL FILES INTO NAMED OUTPUT SOUND FILE.

            	// RE-OPEN /tmp OUTPUT FILES FOR READING AND LATER RESCALING.
//			fprintf( stderr, "beginchan: %d, endchan: %d\n", beginchan, endchan ) ; 
            	for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
            	{
//				fprintf( stderr, "2. scratchString: %s\n", outputTempChanFileNames[ thisoutchan ] ) ;  
                		outputTempChanFiles[ thisoutchan ] = fopen( outputTempChanFileNames[ thisoutchan ], "rb+" ) ; 	
                		if( outputTempChanFiles[ thisoutchan ] == NULL) 
                		{
                   			fprintf( stderr, "1. OUTPUT /tmp FILE %s NOT OPENED.\n", outputTempChanFileNames[ thisoutchan ] ) ; 
                		} ;   
            	} ;  


            	// OPEN OUTPUT SOUND FILE FOR WRITING.
            	// Open the output file.


	outputSFinfo.format = oformat ;

    outputSFinfo.samplerate = osr ; 

   if( channelflag == -2 )
	outputSFinfo.channels = 1 ;
    else
	outputSFinfo.channels = ochan ;

/*

//			prs( ofile, "A3. OPENING SOUND FILE TO WRITE INTERLEAVED DATA." ) ; 
//prt( "H11"); 
            	if (! (outfile = sf_open (ofile, SFM_RDWR, &outputSFinfo )))
            	{   
                		fprintf (stderr, "\n2. Not able to open output file %s.\n", ofile ) ;
               	 	bannero() ;  puts (sf_strerror (NULL)) ;  exit(EXIT_FAILURE) ; // return  1 ;
            	} ;
*/
            	// POSITION AT BEGINNING.
//prt( "H12"); 
            	sf_seek( outfile, 0, SEEK_SET ) ; 

            	// BUFFER IN BUFFERS FROM EACH /tmp FILE; PUT INTO ARRAY.
            	while( (bufferinsamps = 
				fread( &outputBufferByChannels[ beginchan ], sizeof(float), 
					BLOCKSIZE, outputTempChanFiles[ beginchan ] ))	)
            	{


                if( (beginchan + 1) < endchan )
                { 
                    // BUFFER IN THE REST OF THE CHANNELS. 
	            	for(thisoutchan = (beginchan + 1); thisoutchan < endchan; thisoutchan++ )
                    	{
    	                		bufferinsamps = fread( &outputBufferByChannels[ thisoutchan ], sizeof(float), 
						BLOCKSIZE, outputTempChanFiles[ thisoutchan ] ) ; 
                    	} ;  
                } ; 

	        	// RESCALE outputBufferByChannels
                if( rescalev != 2 )
                     rescaleThisBuffer( peakamp, outputBufferByChannels, bufferinsamps ); 	


                // INTERLEAVE AND FLATTEN CHANNEL BUFFERS.
                for( n = 0 ; n < bufferinsamps; n++ )
                {
                    	for(thisoutchan = beginchan, finaloutchan = 0; thisoutchan < endchan; thisoutchan++, finaloutchan++ )
                    	{
                        	interleavedOutputBuffer[ finaloutchan + (n * ochan) ] =  
			          	outputBufferByChannels[ thisoutchan ][ n ] ;      // HERE?
	            	} ; 
                } ; 

                // WRITE FLAT BUFFER TO OUTPUT SOUND FILE. 
//prt( "H13"); 
                sf_write_float ( outfile, interleavedOutputBuffer, bufferinsamps * ochan  ) ; // HERE?


            	} ; 

//            	prt( "\nWRITE AND CLOSE OUTPUT FILE.\n"); 

            	// CLOSE SOUND FILE. 
// pri( outfile, "outfile" ) ;  
//			prs( ofile, "B2. CLOSING SOUND FILE" ) ; 
//prt( "H14"); 
            	sf_close( outfile ) ; 

            	if(quiet != 0){
                		prline( 69,  "=" ) ;
                		fprintf( stderr, "\n\n                 PEAK AMPLITUDES: ALL CHANNELS" ) ; 
                		prline( 69,  "-" ) ;
                		fprintf( stderr, "\nCHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)" );	    
                		fprintf( stderr,  "\n.....................................................................");

                		for(j = 0;  j < ochan; j++)
                		{
                    		fprintf( stderr,  "\n%d          %7.3f          %7.4f     %7.3f", 
                    		j+1,  peakampt[j], peakamp[j], (float) (20. * log10( (double) peakamp[j] )) )     ;
                    		if( nsover[j] > 0 )fprintf( stderr, "     ---> %d <---", nsover[j] ) ;  
                		}
                		prline( 69,  "=" ) ;
            	}
            	fprintf( stderr,  "\n\n" ) ; 

            	prline( 69,  "=" ) ;

		// AUTO-PLAY ***************************************

		autoplay( autoplayreps, formatSwitchflag ) ; 

        } ; // 3

    } ; 
    return(0) ; 
} ; 



void openfiles(){

   int	thisoutchan ; 
// OPEN FILES AFTER SETTNG UP WITH setupfiles.



// ******
    flinflag=0; 

//fprintf( stderr, "outchan: %d, ichan: %d\n", outchan, ichan ) ; 
    // TOTAL SAMPS OUT FOR TPROP COMPUTATION
    ttlsamps = (int) ((float) (end_sample - begin_sample)) * tfactor ; 

//fprintf( stderr, "ttlsamps: %d\n", ttlsamps ) ; 

    // RE-OPEN /tmp INPUT FILES FOR READING.
    for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
    {
//        fprintf( stderr, "thisoutchan: %d\n", thisoutchan ) ; 
//	fprintf( stderr, "REOPEN INPUT FILE: %s\n", inputTempChanFileNames[ thisoutchan ] ) ;
//        fprintf( stderr, "OPENING INPUT CHANNEL FILE %s\n", inputTempChanFileNames[ thisoutchan ] ) ; 
        inputTempChanFiles[ thisoutchan ] = fopen( inputTempChanFileNames[ thisoutchan ], "rb" ) ; 	
        
	if( inputTempChanFiles[ thisoutchan ] == NULL) 
        {
		fprintf( stderr, "0. INPUT /tmp FILE %s NOT OPENED.\n", inputTempChanFileNames[ thisoutchan ] ) ; 
	} ;   
    } ;  

    // RE-OPEN /tmp OUTPUT FILES FOR WRITING.
    if( outputoff != 1 )
    { 
       for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ )
        {
            outputTempChanFiles[ thisoutchan ] = fopen( outputTempChanFileNames[ thisoutchan ], "wb+" ) ; 	
        
	    if( outputTempChanFiles[ thisoutchan ] == NULL) 
            {
	         fprintf( stderr, "2. OUTPUT /tmp FILE %s NOT OPENED.\n", outputTempChanFileNames[ thisoutchan ] ) ; 
	    } ;   
        } ;  
    } ; 


}


// ************************************************************



int bufferin( float *V ) 
{

    static int bufferinsamps=0,  buffinchan=0 ; 
    static float   finbuff[ BLOCKSIZE ] ;

//prt( "100"); 
    if( IO_reset == 1 ){
	// RESET
	bufferinsamps = 0 ; 
	buffinchan = 0 ;
	//sample = 0 ; 
	inbuffn = 0 ; 
	doneflag=0 ;  

	IO_reset = 0 ;  
    } ; 

//prt( "101"); 

    if( buffinchan != channow ){
	// RESET
//	prt( "BUFFERIN: RESETING ...." ) ; 
	inbuffn = 0 ; 
	doneflag = 0 ;
	sample = begin_sample ;
	bufferinsamps = 0 ;
	samps = 0 ;    

	buffinchan = channow ;
    } ; 

//prt( "102"); 

	// INPUT SOUNDFILE BUFFERING 
//pri( inbuffn, "inbuffn"); 
//pri( doneflag, "HERE doneflag"); 
	
    if( inbuffn == 0 ){
	// BUFFER EMPTY
	if( doneflag != 1 ){


	    // READ IN A BLOCK
	    bufferinsamps = fread( &finbuff, sizeof(float), BLOCKSIZE,  inputTempChanFiles[ outchan ] ) ; 

//    fprintf( stderr, "\nAFTER BUFFER IN.... outchan: %d, bufferinsamps: %d\n", outchan, bufferinsamps ) ; 

	    if( bufferinsamps == 0 ){
	        // NO MORE SAMPS
//	        prt( "(A) NO MORE SAMPS!!!!!!!!!" ) ; 
//		fprintf( stderr, ".....CLOSING %s\n", inputTempChanFileNames[ outchan ] ) ; 
                fclose( inputTempChanFiles[ outchan ] ) ; 
		doneflag = 1 ;  
    		return(0); 
	    } ; 	
		// LAST BUFFER
	    if( ((end_sample + ringTimeSamples) - sample) <= bufferinsamps )
	    {
		bufferinsamps = ((end_sample + ringTimeSamples) - sample) ;
		doneflag = 1 ;  
	    } ; 
	
		// ADVANCE BUFFER LAST SAMPLE
	    sample += bufferinsamps ; 
	
	}else{
	    // NO MORE SAMPS
//	    prt( "(B) NO MORE SAMPS!!!!!!!!!" ) ; 
//	    fprintf( stderr, ".....CLOSING %s\n", inputTempChanFileNames[ outchan ] ) ; 
            fclose( inputTempChanFiles[ outchan ] ) ; 
    	    return(0); 
	}
    } ; 

    // PASS BACK A VALUE
//  *V =  ( finbuff[ (inbuffn * ichan) + outchan ]) ;
    *V = ( finbuff[ inbuffn ]) ;


    // TAKE AS PEAK FOR THIS CHANNEL IF SO
    if( *V > ipeakamp[ channow ] ) ipeakamp[ channow ] = *V ; 

    inbuffn++ ; 
    if( inbuffn == bufferinsamps ){
	    // RESET TO ZERO TO TRIGGER NEXT READ
	    inbuffn = 0 ; 
	    bufferinsamps = 0 ; 
    } ; 
    return(1) ; 
	

}



float timenow(float dur ){
    
    
//	t = ((float) samps / (float) ttlsamps ) * dur ; 
	if( outputoff == 1 ) t = (float) samps / (float) isr ; 
	else t = (float) samps / (float) osr   ; 
    return( 1 ) ; 
}



int readffthead( int *N, int *D, int *R, int *chans, int *win_type,  float peakamps[],  struct func *p, int printFlag ){

    float temp, temp2 ; 
    int i ;
    char tempString[ STRING_SIZE ]; 

    // READ IN THE FFT FILE HEADER VALUES

    if( p->n == 0. ){
        if( printFlag  == 1 )fprintf( stderr,  "\n\nYOU MUST PROVIDE A FILTER FILE. BYE.\n" ) ;
        exit(EXIT_FAILURE); 
    } 
    // **************
    // READ IN  HEADER VALUES

    if( fread( &temp, sizeof(float) , 1, p->fp ) == EOF ){
        if( printFlag  == 1 ){fprintf( stderr,  "\n\n1) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;}  exit(EXIT_FAILURE) ;
    }
    *N = (int) temp ; 
    if( fread( &temp, sizeof(float) , 1, p->fp ) == EOF ){
        if( printFlag  == 1 ){fprintf( stderr,  "\n\n2) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;}  exit(EXIT_FAILURE) ;
    }
    *D = (int) temp ; 
    if( fread( &temp, sizeof(float) , 1, p->fp ) == EOF ){
        if( printFlag  == 1 ){fprintf( stderr,  "\n\n3) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;}  exit(EXIT_FAILURE) ;
    }
    *R = (int) temp ; 
    if( fread( &temp, sizeof(float) , 1, p->fp ) == EOF ){
        if( printFlag  == 1 ){fprintf( stderr,  "\n\n4) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;}  exit(EXIT_FAILURE) ;
    }
    *chans = (int) temp ; 


   if( *chans > MAXIMUM_CHANNELS )
   {
	prline( 69, "=" ); 
	prt( "NUMBER OF CHANNELS EXCEEDS CURRENT MAXIMUM SETTING." ); 
	prt( ""); 
	fprintf( stderr, "\n\nIN FILE pvc_lib/pv.h, CHANGE MAXIMUM CHANNEL SETTING OF %d TO %d.\n\n",
			MAXIMUM_CHANNELS, *chans ) ; 
	prt( "THEN RECOMPILE AND REINSTALL." ) ; 
	prline( 69, "=" ); 
	prt( ""); 
	prt( ""); 
	exit(EXIT_FAILURE) ; 
   } ; 






    if( fread( &temp, sizeof(float) , 1, p->fp ) == EOF ){
        if( printFlag  == 1 ){fprintf( stderr,  "\n\n4) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;}  exit(EXIT_FAILURE) ;
    }
    *win_type = (int) temp ; 


    // ************ TEST SIZE TO SEE IF DATA FILE IS POSSIBLY WRONG
    temp = ( ((p->n  - (float) FFT_HEADER_SIZE) / (float) (*N + 2) ) != 0 ) ; 
    temp2 = temp - (float) ( (int) temp ) ; 



    if( (*N <= 0) || (*D <= 0) || (*R <= 0) || 
	    (*chans <= 0) || ( temp2 != 0. ) ){

          if( printFlag == 1 ){
	        prt( "========> YOUR ANALYSIS FILE HAS QUESTIONABLE DATA. <=======\n\n" ) ; 
	        pri( *N,  "INPUT ANALYSIS: FFT SIZE" ) ; 
	        pri( *R,  "INPUT ANALYSIS: SAMPLE RATE" ) ; 
	        pri( *D,  "INPUT ANALYSIS: DECIMATION" ) ; 
	        pri( *chans,  "INPUT ANALYSIS: NUMBER OF CHANNELS" ) ; 
	        pri( p->n,  "INPUT ANALYSIS: FILE SIZE (in 32-bit floats)" ) ; 
	        pri( (p->n * 4),  "INPUT ANALYSIS: FILE SIZE (in bytes)" ) ; 
	        prf( ((float) (p->n * 4 ) /(1024.*1024.)),  "INPUT ANALYSIS: FILE SIZE (in Mbytes)" ) ; 
	        prline( 69,  "-" ) ;
	        prt( "\n" ) ;  
         } ; 	    
		return(-1) ; 
//		exit(EXIT_FAILURE) ; 
    }



    	// PEAKAMPS
    for( i = 0; i <  *chans ; i++){
        if( fread( &temp, sizeof(float) , 1, p->fp ) == EOF ){
    	    if( printFlag  == 1 )fprintf( stderr,  "\n\n4) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;  
			exit(EXIT_FAILURE) ;  
        } ; 
        peakamps[i] = temp ; 
    } ; 


	//  READ SOUND FILE NAME FROM HEADER. 
    fgets( tempString, 500, p->fp ) ; 
//	prs( tempString, "tempString" ) ; 
	i = 0 ; 
	while( ! isspace( tempString[i] ) ){ afile[i] = tempString[i] ; i++ ; } ; 
    if( printFlag  == 1 )fprintf( stderr, "\n\nSOUND FILE ANALYZED: %s\n", afile ) ;  

    return(1) ; 

} 



// FOR USE BY TWARP, CHORDMAPPERPLUS AND DELAYFILTER, I.E. ROUTINES
//  THAT DO NOT HAVE AN AUDIO INPUT FILE TO DETERMINE OUTPUT FILE 
// FORMAT SETTINGS. 

// OPEN OUTPUT IF POSSIBLE; IF NOT, THEN NOTIFY, INSTRUCT, AND EXIT. 
// IF POSSIBLE, TRUNCATE, AND CLOSE IT.
// OPEN /tmp OUTPUT FILES FOR WRITING.  

int outfile_setup(int argc, char **argv )
{
       char *user,
		scratchString[ 128 ]
		;

    int		n,
                thisoutchan		
    ;
	
     
		
    

    SF_INFO      outputSFinfo ; 


    formatSwitchflag = 0 ; 


    end_sample = (int) (outdur * (float) isr ) ; 
    sample = begin_sample =  0 ;
  
     // TOTAL SAMPS OUT FOR TPROP COMPUTATION
    ttlsamps = (int) ((float) (end_sample - begin_sample)) ; 

// ****************************************************************
// *************** SET UP OUTPUT FILE
// ****************************************************************

//    arg_index++ ; 

    if( outputoff == 1 ) {

        if( channelflag == 0 ) ochan = ichan ;
        else ochan = 1 ;
        endchan = beginchan  + ochan ; 
        return(1) ; 

    } ; 

    if( arg_index >= argc  ){ 	// ??? HERE outfile_setup
	// OUTPUT FILE NOT SPECIFIED.
	bannero() ;
	fprintf( stderr, "\n\n...OUTPUT SOUND FILE WITH PRE-SET SOUND FILE HEADER REQUIRED. \n\n" ) ;
	exit(EXIT_FAILURE) ; 
    }else{
	// GET OUTPUT SOUNDFILE NAME
	strcpy( ofile, argv[arg_index] ) ; 
    }	 

    // OPEN OUTPUT SOUND FILE IN READ MODE TO GET FORMAT. 
//	prs( ofile, "A4. OPENING SOUND FILE" ) ; // SFM_RDWR
//prt( "H15"); 
    if (! (outfile = sf_open (ofile, SFM_READ, &outputSFinfo )))
    {   fprintf (stderr, "...OUTPUT SOUND FILE %s NOT FOUND.\n", ofile ) ;
        fprintf( stderr, "\n\n...NAME OF SOUND FILE WITH PRE-SET HEADER FORMAT REQUIRED FOR OUTPUT. \n\n" ) ; 
	    bannero() ;
           puts (sf_strerror (NULL)) ;
           exit(EXIT_FAILURE) ;
    } ;
    // CLOSE IT. 
// pri( outfile, "outfile" ) ;  
//	prs( ofile, "B3. CLOSING SOUND FILE" ) ; 
//prt( "H16"); 
    sf_close( outfile ) ;     

    // SET OUTPUT FORMAT AND RATE TO MATCH OUTPUT FILE.
    oformat = outputSFinfo.format ;

    outputSFinfo.samplerate = osr = isr ;  

   if( channelflag == 0 )
	outputSFinfo.channels = ochan = ichan ;
   else if( channelflag == -1)
	outputSFinfo.channels = ochan ;
   else
	outputSFinfo.channels = ochan = 1 ;

    endchan = beginchan  + ochan ; 



//pri( outputSFinfo.channels,  "BEFORE CREATE OF OUTPUT FILE: NUMBER OF CHANNELS" ) ; 

    // OPEN/CREATE NEW OUTPUT FILE HEADER
//	prs( ofile, "A5. OPENING SOUND FILE" ) ; 
//prt( "H17"); 
    if (! (outfile = sf_open (ofile, SFM_WRITE, &outputSFinfo )))
    {   fprintf (stderr, "...OUTPUT SOUND FILE %s NOT FOUND.\n", ofile ) ;
        fprintf( stderr, "\n\n...NAME OF SOUND FILE WITH PRE-SET HEADER FORMAT REQUIRED FOR OUTPUT. \n\n" ) ; 
	    bannero() ;
           puts (sf_strerror (NULL)) ;
           exit(EXIT_FAILURE) ;
    } ;

//pri( outputSFinfo.channels,  "AFTER CREATE OF OUTPUT FILE: NUMBER OF CHANNELS" ) ; 


//prt( "H18"); 
    if( ! ( sf_format_check (&outputSFinfo) ) )
    {
        fprintf( stderr, "AFTER CREATE OF OUTPUT FILE: INVALID SOUND FILE FORMAT\n" ) ; 
    } ; 

    // POSITION AT BEGINNING AND TRUNCATE.
    sf_count_t frames = 0 ; 
//prt( "H19"); 
    n = sf_command(outfile, SFC_FILE_TRUNCATE, &frames, sizeof (frames)) ; 
 
    // CLOSE
// pri( outfile, "outfile" ) ;  
//    prs( ofile, "B5. CLOSING SOUND FILE" ) ; 
//prt( "H20"); 
//    sf_close( outfile ) ; 

    prbanner( "OUTPUT SOUNDFILE",  69  ) ; 
    prs( ofile,  "OUTPUT FILE: FILENAME " );
    pri( osr,  "OUTPUT FILE: SAMPLE RATE" ) ; 
    pri( ochan,  "OUTPUT FILE: NUMBER OF CHANNELS" ) ; 

   // CREATE /tmp OUTPUT FILES--ONE FOR EACH CHANNEL; LEAVE OPEN FOR WRITING.
   user = pvc_user_tag(); 
   for(thisoutchan = beginchan; thisoutchan < endchan; thisoutchan++ ) {
	sprintf( scratchString, "/tmp/%s.%d.OutputChan.%d", user, (int) getpid(), thisoutchan ) ;
//fprintf( stderr, "\n1. scratchString: %s\n", scratchString ) ;  
       outputTempChanFiles[ thisoutchan ] = fopen( scratchString, "wb" ); 
       strcpy( outputTempChanFileNames[ thisoutchan ], scratchString ) ; 
   } ; 	
   return(1) ; 


}
