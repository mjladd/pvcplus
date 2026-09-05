#include "globals.h"


void usage(); 
void pd( int i ) ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j, k, l,  m, nnn=0 ;
float nyquist,  fundamental ;
int R=44100, N=1024, N2, Nw = 2048, Nw2, D = 220, I = 220, in, on;
int   eof = 0, obank = 0,  sflag = 0,  channelout=0,  write_ascii=0 ;
float P = 1.0;
FILE *fopen() ;
char ch,  tempstring[ STRING_SIZE ],  
    scratch[ STRING_SIZE ],  scratch2[ STRING_SIZE ],  *user ;
float  dur ;
float  gain, f ;
float  temp, temp1,  temp2,  pm,  IR  ;  
int showme=0, numFramesOut=0 ; 
int numberOfOutChannels=1, chan, outputChan, inputChan, fileSizeInBytes, numFrames ; 
float outputChanPeakAmpSum[2]={-99999999.,-999999999.}, thisOutputChanAmpSum[2],
	PeakAmp ; 

int numSampsBufferedIn, numFramesLeft, middleChanFlag, middleChanNumber ; 
int numFramesBufferedIn, blockFrame ; 

float tempBlock[ BLOCKSIZE ], *allChanInputBlock, *allChanOutputBlock ; 

SF_INFO      outputSFinfo ; 

endt = -1.0 ; 

if( argc < 2 )usage() ; 


    while( (ch = crack( argc, argv,
    "b|e|c|C|_|", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {


	    case 'c':   numberOfOutChannels = (int) crackfloat( arg_option, ch ) ;
			break;

	    case 'b':   begint = crackfloat( arg_option, ch ) ;
			break;
	    case 'e':   endt = crackfloat( arg_option, ch ) ;
			break;
           case '_':	autoplayreps = (int) crackfloat( arg_option, ch ) ; break;


    } 
}



prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "CHANNELMIX", 69 ) ; 
prline( 69,  "-" ) ; 

beginchan = 0 ;

// SET NO OUTPUT FLAG
outputoff=1;


// GET INPUT HEADER INFO
    setupfiles(argc, argv) ; 

    endchan = beginchan + ochan ; 

    // GET NAME OF USER
 	user = getlogin(); 
  
// **** SET UPS *****
    R = isr ; // SAMPLE RATE EQUALS INPUT FILE

    nyquist = R/2.0;

    // COMPUTE THE DURATION
//prf( begint, "INPUT SOUND FILE BEGIN TIME" ) ; 
//prf( endt, "INPUT SOUND FILE END TIME" ) ; 
dur = (endt - begint) * (float) I / (float) D ; 
//prf( dur, "INPUT/OUTPUT FILE: DURATION" ) ; 



    
//***************** PRINT VALUES

if( numberOfOutChannels > 2 ){
	prt( "\n\n---------------> ERROR:" ) ; 
	pri( numberOfOutChannels, "NUMBER OF OUTPUT CHANNELS" ) ; 
	prt( "MONO(1) OR STEREO(2) OUTPUT ONLY.\n\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
};  
/*
if( ichan == numberOfOutChannels ){
	prt( "\n\n---------------> ERROR:" ) ; 
	pri( ichan, "NUMEBR OF INPUT CHANNELS" ) ; 
	pri( numberOfOutChannels, "NUMBER OF OUTPUT CHANNELS" ) ; 
	prt( "INPUT AND OUTPUT CHANNELS ARE THE SAME.\n\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
} ; 
*/

/*
if( ichan == 1 ){
	prt( "\n\n---------------> ERROR:" ) ; 
	pri( ichan, "NUMEBR OF INPUT CHANNELS" ) ; 
	prt( "ONLY ONE CHANNEL IN INPUT FILE.\n\n\n" ) ; 
	exit(EXIT_FAILURE) ; 
}; 
*/

// 

// ************

	    arg_index++ ; 

		if( arg_index >= argc  ){ 	// ??? HERE outfile_setup
			// OUTPUT FILE NOT SPECIFIED.
			bannero() ;
			fprintf( stderr, "\n\n...OUTPUT SOUND FILE WITH PRE-SET SOUND FILE HEADER REQUIRED. \n\n" ) ;
			exit(EXIT_FAILURE) ; 
    		}else{
			// GET OUTPUT SOUNDFILE NAME
			strcpy( ofile, argv[arg_index] ) ; 
    		} ; 	 




		// OPEN OUTPUT SOUND FILE IN READ MODE TO GET FORMAT. 
    		if (! (outfile = sf_open (ofile, SFM_READ, &outputSFinfo )))
    		{   
			fprintf (stderr, "...OUTPUT SOUND FILE %s NOT FOUND.\n", ofile ) ;
			fprintf( stderr, "FILE FORMAT WILL BE COPIED FROM INPUT SOUND FILE INSTEAD. \n" ) ; 
			fprintf( stderr, "\n==> NOTE: FILE NAME EXTENSION MAY NOT MATCH HEADER FORMAT. <==\n\n" ) ; 
	           bannero() ;
                // SET OUTPUT FORMAT AND RATE TO MATCH INPUT FILE.
                oformat = iformat ; outputSFinfo.format = oformat ;

		}else{

		    	// PRE-EXISTING FILE FOUND: SET OUTPUT FORMAT AND RATE TO MATCH OUTPUT FILE.
                	oformat = outputSFinfo.format ;


   		} ;




    		// CLOSE IT. 
    		sf_close( outfile ) ;     

    		outputSFinfo.samplerate = osr = isr ;  
		outputSFinfo.channels = numberOfOutChannels ;

    		// OPEN/CREATE NEW OUTPUT FILE HEADER
    		if (! (outfile = sf_open (ofile, SFM_WRITE, &outputSFinfo )))
    		{   
			fprintf (stderr, "...OUTPUT SOUND FILE %s NOT FOUND.\n", ofile ) ;
        		fprintf( stderr, "\n\n...NAME OF SOUND FILE WITH PRE-SET HEADER FORMAT REQUIRED FOR OUTPUT. \n\n" ) ; 
	    		bannero() ;
           		puts (sf_strerror (NULL)) ;
           		exit(EXIT_FAILURE) ;
    		} ;

    		if( ! ( sf_format_check (&outputSFinfo) ) ){
        		fprintf( stderr, "AFTER CREATE OF OUTPUT FILE: INVALID SOUND FILE FORMAT\n" ) ; 
    		} ; 

    		// POSITION AT BEGINNING AND TRUNCATE.
    		sf_count_t frames = 0 ; 
    		k = sf_command(outfile, SFC_FILE_TRUNCATE, &frames, sizeof (frames)) ; 

 
// ************





// OPEN TEMP FILES
for(chan = 0; chan < ichan; chan++ ){
//	pri( chan, "CHANNEL" ) ; 
	inputTempChanFiles[ chan ] = fopen( inputTempChanFileNames[ chan ], "rb" );
	fseek( inputTempChanFiles[ chan ], 0, SEEK_END);
	i = ftell(inputTempChanFiles[ chan ]);
	if( chan == 0 ) fileSizeInBytes = i ;
	if( i != fileSizeInBytes ){
		prt( "\n\n---------------> ERROR:" ) ; 
		pri( fileSizeInBytes, "PREVIOUS CHANNEL FILE SIZE IN BYTES" ) ; 
		pri( i, "THIS CHANNEL FILE SIZE IN BYTES" ) ; 
		exit(EXIT_FAILURE) ; 
	} ; 
	
} ; 	

//pri( fileSizeInBytes, "FILE SIZE IN BYTES" ) ; 

// MAKE SPACE
fvec( allChanInputBlock, BLOCKSIZE * ichan ) ; 
fvec( allChanOutputBlock, BLOCKSIZE * numberOfOutChannels ) ; 


// COMPUTE NUMBER OF FRAMES
numFrames = fileSizeInBytes / sizeof(float) ; 

pri( numFrames, "TOTAL NUMBER OF FRAMES" ) ; 

prline( 1,  "*" ) ;
//prs( ofile, "OUTPUT SOUND FILE NAME" ) ; 


// FIND PEAK SUM FOR EACH OUTPUT CHANNEL'S  INPUT CHANNELS. 

if( numberOfOutChannels > 1 ){ 
	if( ( ((float)  ichan / 2.) - (float) ( (int) (ichan / 2.) )) > 0. ){
		middleChanFlag = 1 ; middleChanNumber = (int)((float) ichan / 2.) ; // pri( middleChanNumber, "MIDDLE CHANNEL" ) ;  
	}else{
		middleChanFlag = 0 ; //                      prt( "NO MIDDLE CHANNEL" ) ; 
	} ; 
} ; 

for(l = 0; l < 2; l++ ){


	// REWIND ALL INPUT FILES.
	for(chan = 0; chan < ichan; chan++ )fseek( inputTempChanFiles[ chan ], 0, SEEK_SET );

	// SET FRAMES LEFT TO TOTAL FRAMES.
	numFramesLeft = numFrames ; 

//	pri( numFramesLeft, "NUMBER OF FRAMES LEFT TO READ IN FOR PEAK AMP SEARCH" ) ; 

	prt( "\nWRITING MIX TO SOUND FILE . . .\n" ) ; 
	while( numFramesLeft > 0 ){
		// FRAMES LOOP
		
		for( chan = 0; chan < ichan; chan++ ){ // LOOP FOR INPUT CHANNELS
			// READ IN AN INPUT CHANNEL BLOCK
	    		numFramesBufferedIn = fread( &tempBlock, sizeof(float), BLOCKSIZE,  inputTempChanFiles[ chan ] ) ; 

//			pri( numFramesBufferedIn, "NUMBER OF FRAMES BUFFERED IN" ); 

			// TRANFSFER CHANNEL BLOCK TO FLAT INPUT BLOCKS SAVE [chan0 * BLOCKSIZE, chan1 * BLOCKSIZE, etc. ]
			for(i = 0, k = (chan * BLOCKSIZE); i < numFramesBufferedIn; i++, k++ )
				allChanInputBlock[k] = tempBlock[i] ; 
		};  

		// ZERO OUTPUT BLOCK.
		for(i = 0; i < BLOCKSIZE * numberOfOutChannels; i++) allChanOutputBlock[i] = 0. ; 


		// STEP THROUGH BLOCK FRAMES	
		for(blockFrame = 0; blockFrame < numFramesBufferedIn; blockFrame++ ){

			if( numberOfOutChannels == 1 ){
				// MONO
				thisOutputChanAmpSum[0] = 0. ; // SUM FOR THIS FRAME
				for( chan = 0, k = blockFrame; chan < ichan ; chan++, k += BLOCKSIZE ){ 
						// LOOP FOR ALL INPUT CHANNELS FOR SUMMING THIS FRAME
					temp = allChanInputBlock[k] ; if(l == 0) temp = fabs(temp) ; thisOutputChanAmpSum[0] += temp ; 	 		
				} ; 

				// SAVE IF GREATER THAN PREVIOUS SUM.
				if(l == 0) if( thisOutputChanAmpSum[0] > outputChanPeakAmpSum[0] ) 
						outputChanPeakAmpSum[0] = thisOutputChanAmpSum[0] ; 
				if(l == 1){
					// WRITE thisOutputChanAmpSum INTO INTERLEAVED BLOCK
					allChanOutputBlock[blockFrame] = thisOutputChanAmpSum[0] * rescalev ; 
				} ; 
				 

			}else{
				// STEREO
				// LEFT
				thisOutputChanAmpSum[0] = 0. ;
				for(chan = 0, k = blockFrame; chan < (int)((float) ichan / 2.); chan++, k += BLOCKSIZE){ 
					temp = allChanInputBlock[k] ; if(l == 0) temp = fabs(temp) ; thisOutputChanAmpSum[0] += temp ; 	 		
				} ; 
				if( middleChanFlag ){
					// ADD HALF OF MIDDLE
					temp = 0.5 * allChanInputBlock[ (middleChanNumber * BLOCKSIZE) + blockFrame ] ; 
					if(l == 0) temp = fabs(temp) ; thisOutputChanAmpSum[0] += temp ;
				} ;
				if(l == 0) if( thisOutputChanAmpSum[0] > outputChanPeakAmpSum[0] ) 
									outputChanPeakAmpSum[0] = thisOutputChanAmpSum[0] ; 

				// RIGHT
				thisOutputChanAmpSum[1] = 0. ;
				for(chan = 0, k = ((ichan - 1) * BLOCKSIZE) + blockFrame; 
						chan < (int)((float) ichan / 2.); chan ++, k -= BLOCKSIZE)
				{ 
					temp = allChanInputBlock[k] ; if(l == 0) temp = fabs(temp) ; thisOutputChanAmpSum[1] += temp ; 	 		
				} ; 
				if( middleChanFlag ){
					// ADD HALF OF MIDDLE
					temp = 0.5 * allChanInputBlock[ (middleChanNumber * BLOCKSIZE) + blockFrame ] ; 
					if(l == 0) temp = fabs(temp) ; thisOutputChanAmpSum[1] += temp ;
				} ;
				if(l == 0) if( thisOutputChanAmpSum[1] > outputChanPeakAmpSum[1] ) 
									outputChanPeakAmpSum[1] = thisOutputChanAmpSum[1] ; 



				if(l == 1){
					// WRITE thisOutputChanAmpSum, LEFT AND RIGHT, INTO INTERLEAVED BLOCK
					allChanOutputBlock[blockFrame * 2] = thisOutputChanAmpSum[0] * rescalev ; 
					allChanOutputBlock[(blockFrame * 2) + 1] = thisOutputChanAmpSum[1] * rescalev ; 
				} ; 

			} ; 



		} ; 
		if(l == 1){
			// WRITE BUFFER OUT TO SOUNDFILE 
			k = numFramesBufferedIn * numberOfOutChannels ;
			numFramesOut += k ; 
			if( numFramesOut > (osr * numberOfOutChannels) ){
				fprintf( stderr, "*" ) ; numFramesOut -= (osr * numberOfOutChannels) ;  
			} ; 
                sf_write_float ( outfile, allChanOutputBlock, k  ) ; // HERE?
//			pri( k, "WROTE BLOCK: SAMPS" ) ; 
		} ; 



		numFramesLeft -= numFramesBufferedIn ; 
//		pri( numFramesLeft, "NUMBER OF FRAMES LEFT TO READ IN FOR PEAK AMP SEARCH" ) ; 

	} ; 

	if(l == 1) prt( "\n" ) ; 

	if(l == 0){
		if(numberOfOutChannels == 1 ) PeakAmp = outputChanPeakAmpSum[0] ; 
		else {
			PeakAmp = outputChanPeakAmpSum[0] > outputChanPeakAmpSum[1] ? outputChanPeakAmpSum[0] : outputChanPeakAmpSum[1] ; 
		} ; 

/*
		for( chan = 0; chan < numberOfOutChannels; chan++ ){
			pri( chan, "CHANNEL" ) ; 
			prf( amp_to_dB( outputChanPeakAmpSum[chan] ), "PEAK OUTPUT CHANNEL AMP (in dB)"  ) ; 
		}; 
*/
		prf( amp_to_dB( PeakAmp ), "PEAK AMP OF ALL OUTPUT CHANNELS (in dB)" ) ; 

		if( PeakAmp > 1. ){ 
			rescalev = 0.99999 / PeakAmp ; prt("RESCALING TO AVOID CLIPPING." ) ; 
		}else rescalev = 1.  ; 
	} ; 

} ; 


    // *******

    // CLOSE  INPUT FILE
if(ifd)fclose(ifd);  

for( chan = 0; chan < ichan; chan++ ) fclose( inputTempChanFiles[ chan ] ); 

// CLOSE SOUND FILE. 
sf_close( outfile ) ; 


autoplay( autoplayreps, 0 ); 


fprintf(stderr,"\nCHANNEL MIX COMPLETED\n");
exit(EXIT_SUCCESS) ;

}

void usage()
{
    fprintf(stderr, "%s",
	"channelmix:    \n"
	"channelmix   [flags] [input file] [output file]\n"
	"	    Most formats accepted. Output format copied from input file.\n"
	"	    (Values in brackets denote defaults.)\n"

	"	b:	"BEGIN_TIME		// begint
	"	e:	"END_TIME			// endt
	"	c:	number of output channels [1]\n"
	"	_:	 "AUTO_PLAY		// autoplayreps


	);
    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }