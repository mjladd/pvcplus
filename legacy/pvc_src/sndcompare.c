#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SND_MAGIC ((int)0x2e736e64)
#define SND_FORMAT_UNSPECIFIED          (0)
#define SND_FORMAT_MULAW_8              (1)
#define SND_FORMAT_LINEAR_8             (2)
#define SND_FORMAT_LINEAR_16            (3)
#define SND_FORMAT_LINEAR_24            (4)
#define SND_FORMAT_LINEAR_32            (5)
#define SND_FORMAT_FLOAT                (6)
#define SND_FORMAT_DOUBLE               (7)


#define BLOCKSIZE 8192
/*
* THIS ROUTINE READS THE SHORTS OF THE SPECIFIED FILE AND 
* CHANNEL AND WRITES THEM AS FLOATS TO THE STANDARD OUT
*
*/

/* CRACK STUFF */
int arg_index = 0;
char *arg_option;
char *pvcon = NULL;
char *index();

int bannerflag=0,  banner();  

/************************************************************/

int main(argc,argv)
   int argc;
   char *argv[]; 
   {
   

    char	
		*firstfilename, 
		*secondfilename, 
		*outputfilename, 
		ch, 
		crack()
    ;

    
    int		verbose=0, 
		exflag=0, 
		exflagl=0,
		exflagr=0, 
		numreadr, 
		numreadl,
		sampcount,  
		n,nn, 
		i,
		j,
		m, 
		mm,  
		urate=0,  
		lsr, rsr, osr,  
		lchan, rchan, ochan, 
		ldata_offset, rdata_offset, odata_offset, 
		lformat, rformat, oformat, 
		fdfirst, fdsecond, fdoutput, 
		ki, 
		nsamples, 
		numerrors
    ;
	
    short int	idata
    ; 

    short int	firstdata[ BLOCKSIZE ],  seconddata[ BLOCKSIZE ],  output_array[ BLOCKSIZE * 2 ]
    ;		
    float	ldur, rdur, 
		out, 
		bwt=0., 
		ewt=0., 
		sum=0., 
		oldsum=0., 
		begint=0., 
		outputdur=-1., 
		
		ampsum, 
		sumerrors, 
		avgerrordB, 
		avgerror, 
		temp, 
		temp2, 
		temp3
    ;
    int		begints,
		filesamp, 
		outputdurs,  
		bwts,
		peakamp,  
		ewts, 
		ttlsamps, 
		diffoffset=0., 
		ic=0, 
		itemp
    ;

char scratch[ STRING_SIZE ] ; 

int getsfstats(char *filename, int *sr, int *chan, float *dur, int *data_offset, int *format) ; 

typedef struct {
        int magic;          /* must be equal to SND_MAGIC */
        int dataLocation;   /* Offset or pointer to the raw data */
        int dataSize;       /* Number of bytes of data in the raw data */
        int dataFormat;     /* The data format code */
        int samplingRate;   /* The sampling rate */
        int channelCount;   /* The number of channels */
        char info[4];       /* Textual information relating to the sound. */
} 
SNDSoundStruct;

   SNDSoundStruct newheader;
   struct       stat st;


/*GIVE USAGE AND EXIT IF NO ARGUMENTS */
   if(argc == 1) {
	fprintf( stderr,"%s%s%s%s%s%s%s", 
	"\n\nUSAGE:\n\n   sndcompare [flags] (NeXT/Sun format files:) <first.snd> <second.snd> <output.snd>\n\n", 
	"\tThe first and second stereo (NeXT/Sun format) input sound files ", 
	"\n\tare subtracted from each other to create an ouput (NeXT/Sun format)  sound file.", 
	"\n\tThe correlation window is used to find the probable phase offest of the two files.\n\n", 
	 "\n\t\t[defaults in brackets]", 
	"\n\t\t-b:\tbegin correlation window time (in seconds) [0.]", 
	"\n\t\t-e:\tend correlation window time (in seconds) [0.]"
	"\n\t\t-B:\tbegin output time (in seconds) [0.]"
	"\n\t\t-D:\tduration of output following -B (in seconds) [complete file]"
	"\n\t\t-v:\tverbose printout: print deviant samples to stderr, 1 = on,  0 = off [0]\n\n"
	 ) ;
	exit(0);
    }
    

    while( (ch= crack( argc, argv,
    "|b|e|B|D|v|", 0  )) != NULL ) {
	switch(ch) {
            case 'b': bwt = atof(arg_option) ;                  break;
            case 'e': ewt = atof(arg_option) ;                  break;
            case 'B': begint = atof(arg_option) ;                  break;
            case 'D': outputdur = atof(arg_option) ;                  break;
            case 'v': verbose = atoi(arg_option) ;                  break;
	    
        }
    }




/* GET SOUNDFILE NAME AND OPEN FOR BUSINESS */
   firstfilename = argv[arg_index] ;
   secondfilename = argv[arg_index + 1] ;
   if( (arg_index + 2) >= argc ) {
	outputfilename = "difference.snd" ;
	banner() ;
	fprintf( stderr,  "\nDEFAULT OUTPUT FILE = difference.snd\n" ) ;
    } else
	outputfilename = argv[arg_index + 2] ;



        exflagl = getsfstats(firstfilename, &lsr, &lchan, &ldur, &ldata_offset, &lformat) ; 
		
		
        exflagr = getsfstats(secondfilename, &rsr, &rchan, &rdur, &rdata_offset, &rformat) ; 


        fprintf(stderr,  "\n\nFILE 1: %s\n\tsample rate = %d\n\tchannels = %d\n\tduration=%f\n", \
                 firstfilename, lsr, lchan, ldur );

    sprintf( scratch, "ls -l %s\0", firstfilename ) ;
    system( scratch ) ; 

        fprintf(stderr,  "\n\nFILE 2: %s\n\tsample rate = %d\n\tchannels = %d\n\tduration = %f\n", \
                 secondfilename, rsr, rchan, rdur );

    sprintf( scratch, "ls -l %s\0", secondfilename ) ;
    system( scratch ) ; 

	fprintf( stderr, "\n\nOUTPUT FILE (DIFFERENCE OF FILE 1 AND 2): %s\n", outputfilename ) ; 


/* CHECK TO SEE IF ANY OF THE FILES ARE THE SAME */

    if( ! strcmp( firstfilename, secondfilename ) ){
	fprintf( stderr, "\n\nYOUR INPUT FILES ( %s and %s ) \n\n\tARE THE SAME! BYE.\n\n", 
	    firstfilename,  secondfilename ) ; 
	exit(0) ; 
    }
    if( ! strcmp( firstfilename, outputfilename ) ){
	fprintf( stderr, "\n\nYOUR OUTPUT FILE IS THE SAME AS YOUR FIRST FILE! BYE.\n\n" ) ; 
	exit(0) ; 
    }

    if( ! strcmp( secondfilename, outputfilename ) ){
	fprintf( stderr, "\n\nYOUR OUTPUT FILE IS THE SAME AS YOUR SECOND FILE! BYE.\n\n" ) ; 
	exit(0) ; 
    }
	
	if( ( exflagl == -2 ) || ( exflagr == -2 ) ) {
	    banner() ;
	    fprintf( stderr, "\n\n\t....FIX THE FORMAT OF YOUR FILE(S). BYE.\n\n" ) ;
	    exit(0) ; 
	}
	if( ( exflagl == -1 ) || ( exflagr == -1 ) ) {
	    banner() ;
	    fprintf( stderr, "\n\n\t....FIX YOUR FILE (PATH)NAME(S). BYE.\n\n" ) ;
	    exit(0) ; 
	}
		 
/* CHECK FOR MONO FILES */
	if( lchan != 2 ) {
	    banner() ;
	    fprintf(stderr,  "\nYOUR FIRST FILE:\n\n------> %s <-------\n\n IS NOT STEREO.\n\n", firstfilename );
	}
	if( rchan != 2 ) {
	    banner() ;
	    fprintf(stderr,  "\nYOUR SECOND FILE:\n\n------> %s <-------\n\n IS NOT MONO.\n\n", secondfilename );
	}
	if( rchan != 2 || lchan != 2 ) {
	    banner() ;
	    fprintf(stderr,  "\n.........MAKE SURE YOUR LEFT AND RIGHT  FILES ARE STEREO. BYE.\n\n");
	    exit(0) ; 
	}


/*
/* CHECK FOR STUPIDITY */
    if( lformat != 3 ){
	    banner() ;
        fprintf( stderr, "\n\nYOUR FIRST FILE IS NOT A NEXT FORMAT FILE.\n\n" ) ;
    }
    if( rformat != 3 ){
	    banner() ;
        fprintf( stderr, "\n\nYOUR SECOND FILE IS NOT A NEXT FORMAT FILE.\n\n" ) ;
    }
    if( lformat != 3  ||  rformat != 3 ){
	    banner() ;
	 fprintf( stderr, "\nUSE nexts OR nextd TO CHANGE THE FORMAT OF YOUR FILES.\n\t...................BYE.\n\n" ) ;
	 exit(0) ; 
    }


   if( (lsr != rsr) ){
	    banner() ;
        fprintf( stderr, "\n\nYOUR FILES HAVE DIFFERENT SAMPLE RATES.BYE\n\n" ) ;exit(0) ; 
    }



    fprintf( stderr, "\n\nBEGIN WINDOW TIME: %f",  bwt ) ; 
    fprintf( stderr, "\nEND WINDOW TIME: %f",  ewt ) ; 
    fprintf( stderr, "\nBEGIN OUTPUT TIME: %f",  begint ) ; 
    if( outputdur != -1. ) fprintf( stderr, "\nOUTPUT DURATION: %f",  outputdur ) ; 
    else fprintf( stderr, "\nOUTPUT DURATION: ALL OF FILE OR TO END (AS ALLOWED BY ANY SHIFT)" ) ; 
    if( argc  < (arg_index + 2)  ){
	banner() ; 
	fprintf( stderr, "\nWHERE ARE THE TWO INPUT FILES? BYE.\n\n") ;
        exit(0)  ;	
    }



/* OPEN FILES */
    fdfirst = open( firstfilename, O_RDWR,(0644)); 
    fdsecond = open( secondfilename, O_RDWR,(0644)); 

/* OPEN OUPUT FILE */
    fdoutput = open( outputfilename,O_RDWR,(0644));
    if(fdoutput < 0) {
	fprintf( stderr,  "%s DOES NOT YET EXIST. I'LL MAKE IT.\n", 
	    outputfilename );
	exflag = open(outputfilename, O_CREAT,(0644)); 
	if( exflag == -1 ){
		
	    banner() ;
		fprintf( stderr,  "\n\n I COULD NOT MAKE THE FILE:\n\n-----> %s <-------\n\n",  outputfilename ) ; 
		fprintf( stderr,  "\n\n CHECK THE PATH OF YOUR FILE. ....BYE.\n\n" ) ; 
		exit( 0 ) ; 
	}
	
	fdoutput = open(outputfilename, O_RDWR,(0644));
    }else{
	open(outputfilename, O_TRUNC, (0644));
    }

/* WRITE 1024 dummy bytes for the header */
    idata = 0 ;
    for( i = 0; i < 512 ; i++)
	write( fdoutput,  &idata, sizeof(short int) ) ;

/* MAKE IT A SOUNDFILE BY WRITING THE HEADER */

        /* Now,create new header  and slap it at the front of this soundfile*/

        newheader.magic = SND_MAGIC;
        odata_offset = 1024 ;
        newheader.dataLocation = odata_offset;
        newheader.dataFormat = SND_FORMAT_LINEAR_16 ;
        newheader.samplingRate = lsr ;
        newheader.channelCount = 2 ;


        if(stat(outputfilename,&st))  {
                fprintf(stderr, "\nputlength: (#1)  Couldn't stat file\n\n");
                exit(1);
        }
        newheader.dataSize = (int)st.st_size - odata_offset ;
/*
fprintf( stderr,  "\n dataSize = %d",  (int)newheader.dataSize ) ; 
*/
	 /* this captures the whole size of the file */

    	lseek(fdoutput, 0, SEEK_SET);
        if((write( fdoutput, &newheader, sizeof(newheader) ) == -1))
                        fprintf( stderr, "\ncouldn\'t write header\n\n");

	lseek( fdoutput, odata_offset, SEEK_SET ) ;
    
    
//**************************************************************    
// FIND THE CORRELATION POINT
    // TRANSLATE THE BEGIN AND END TIMES TO SAMPLES
	if( ewt < bwt ){
	    fprintf( stderr, "\n\nYOUR WINDOW TIMES ARE DECREASING. BYE\n\n" ) ;
	    exit(0); 
	}
	bwts = (int) (bwt * (float) lsr) ; 
	ewts = (int) (ewt * (float) rsr ) ; 
	ttlsamps = ewts - bwts ; 
	diffoffset = 0. ; 

//fprintf( stderr, "\nttlsamps: %d",  ttlsamps ) ; 


	if( ttlsamps > 0 ){
	    // FIRST SKIPT TO THE BEGIN POINTS    
    


	     oldsum = 0 ; ic = 0 ; 
	    // LOOP FOR THE CORRELATION SHIFTS
	    for( i = 0 ; i < ttlsamps; i++ ){

		//PASSIFIER PRINT
		ic++ ; 
		if( ic == 4410 ){
		    ic = 0 ; fprintf( stderr, "*" ) ;
		}
		
		// 
		lseek( fdfirst, ldata_offset + (bwts * sizeof(short) * 2), SEEK_SET ) ;
		lseek( fdsecond, rdata_offset + ((bwts + i ) * sizeof(short) * 2), SEEK_SET ) ;

		// LOOP FOR THE SUM ACCUMULATION
		sum = 0. ; ki = 0 ; 
		for( j = 0 ; j < ttlsamps; j++ ){
		    /* READ IN THE VALUES */
		    if( ki == 0 ){
			numreadl = read( fdfirst,   &firstdata,  BLOCKSIZE * sizeof(short)  ) ; 
			numreadr = read( fdsecond,  &seconddata, BLOCKSIZE * sizeof(short) ) ; 
		    }

	
		    sum += fabs( (float) (firstdata[ki] - seconddata[ki]) ) + 
			fabs( (float) (firstdata[ki + 1] - seconddata[ki + 1]) ) ; 
		    
		    ki += 2 ;
		    if( ki == BLOCKSIZE )ki = ki - BLOCKSIZE ; 
		}
		
		
		if( sum < 0. ) sum = sum * -1. ; 


		if( i == 0 ){
		    // FIRST
		    oldsum = sum ; diffoffset = i ; 
		}else{
		    // COMPARE AND TAKE SMALLEST
		    if( sum < oldsum ){
			// CLOSER
			oldsum = sum ; 
			diffoffset = i ;
		    }
		    //fprintf( stderr, "\nsum(1): %f,  diffoffset %d,  i: %d",  sum,  diffoffset,  i ) ; 
		}
		
	    }
	    
	    //** NOW THE OTHER SHIFTS
	    	    // LOOP FOR THE CORRELATION SHIFTS
	    ic = 0 ; 
	    for( i = 0 ; i < ttlsamps; i++ ){
 
 		//PASSIFIER PRINT
		ic++ ; 
		if( ic == 4410 ){
		    ic = 0 ; fprintf( stderr, "*" ) ;
		}

		lseek( fdfirst, ldata_offset + ((bwts + i) * sizeof(short) * 2), SEEK_SET ) ;
		lseek( fdsecond, rdata_offset + ((bwts) * sizeof(short) * 2), SEEK_SET ) ;

		// LOOP FOR THE SUM ACCUMULATION
		sum = 0. ; ki = 0 ;
		for( j = 0 ; j < ttlsamps; j++ ){
		    /* READ IN THE VALUES */
		    if( ki == 0 ){
			numreadl = read( fdfirst,   &firstdata,  BLOCKSIZE * sizeof(short)  ) ; 
			numreadr = read( fdsecond,  &seconddata, BLOCKSIZE * sizeof(short) ) ; 
		    }

		    sum += fabs( (float) (firstdata[ki] - seconddata[ki]) ) + 
			fabs( (float) (firstdata[ki + 1] - seconddata[ki + 1]) ) ; 
		    
		    ki += 2 ;
		    if( ki == BLOCKSIZE )ki = ki - BLOCKSIZE ; 



		}
		
		if( sum < 0. ) sum = sum * -1. ; 


		    // COMPARE AND TAKE SMALLEST
		    if( sum < oldsum ){
			// CLOSER
			oldsum = sum ; 
			diffoffset = -1 * i ;
		    }
		    //fprintf( stderr, "\nsum(2): %f,  diffoffset %d,  i: %d",  sum,  diffoffset,  i ) ; 
		
	    }

	    if( oldsum != 0. ){
		fprintf( stderr, "\n\nWARNING: YOUR WINDOW  DID NOT FIND A PERFECT CORRELATION POINT." ) ; 
		fprintf( stderr, "\nCONSIDER USING A LARGER WINDOW.\n" ) ; 		
		
	    }else{
		fprintf( stderr, "\n\nYOUR WINDOW HAS PRODUCED A PERFECT CORRELATION.\n" ) ; 
	    }

	    if( diffoffset == 0 ){
		fprintf( stderr, "\n\nSHIFT: 0 samples\n" ) ; 
	    }else if(diffoffset > 0){
		fprintf( stderr, "\n\nSECOND FILE SHIFT: %d samples (%f seconds)\n", 
		     abs(diffoffset), (float) diffoffset / (float) lsr ) ; 
	    }else{
		fprintf( stderr, "\n\nFIRST FILE SHIFT: %d samples (%f seconds)\n", 
		     abs(diffoffset), (float) diffoffset / (float) lsr ) ; 
	    }


	}else{
	    // NO CORRELATION SEARCH
	    fprintf( stderr, "\nO WINDOW SIZE; NO CORRELATION SEARCH.\n\n" ) ;
	    diffoffset = 0 ; 
	}
  
  
  
    
//**************************************************************    

/* SKIP TO DATA OFFSET OF INPUT FILES PLUS THE CORRELATION SHIFT + BEGIN */
	
	    begints = begint * lsr ; // BEGIN TIME IN SAMPLES
	    if( outputdur == -1. )outputdurs = (ldur - begint) * lsr ; 
	    else outputdurs = outputdur * lsr ; 

	    if( diffoffset == 0 ){
		// NO SHIFT
		lseek( fdfirst, ldata_offset + (2 * sizeof( short ) * begints), SEEK_SET ) ;
		lseek( fdsecond, rdata_offset+ (2 * sizeof( short ) * begints), SEEK_SET ) ;
	    }else if(diffoffset > 0){
		// SECOND FILE SHIFT
		lseek( fdfirst, ldata_offset + (2 * sizeof( short ) * begints), SEEK_SET ) ;
		lseek( fdsecond, rdata_offset + ((abs(diffoffset) + begints) * 2 * sizeof( short )), SEEK_SET ) ;
	    }else{
		// FIRST FILE SHIFT
		lseek( fdfirst, ldata_offset + ((abs(diffoffset) + begints) * 2 * sizeof( short) ), SEEK_SET ) ;
		lseek( fdsecond, rdata_offset + (2 * sizeof( short ) * begints), SEEK_SET ) ;
	    }



/* READ IN THE VALUES */
    numreadl = read( fdfirst,   &firstdata,  BLOCKSIZE * sizeof(short)  ) ; 
    numreadr = read( fdsecond,  &seconddata, BLOCKSIZE * sizeof(short) ) ; 
    
    
	 sampcount = 0 ;numerrors = 0 ; sumerrors = 0. ; peakamp = -9999999 ; 
	 filesamp = begints ; 
    
    while( (numreadl  > 0 ) && (numreadr > 0 ) && (sampcount < outputdurs) ) {


	// TAKE THE DIFFERENCE 
     
	// USE THE SMALLER LENGTH
	if( numreadl < numreadr ) numreadr = numreadl ; 
	
	nsamples = numreadl / 4 ; // NUMBER OF SAMPLES (2 CHANNELS, 2 BYTES/CHANNEL
	
	
	if( (outputdurs - sampcount) < nsamples ) nsamples = (outputdurs - sampcount) ; 
	
	for( n = 0,  m = 0,  mm = 1; n < nsamples ; n++, m += 2, mm += 2   ){

	    // LEFT CHANNEL
	    output_array[ m ] = firstdata[ m ] -  seconddata[ m ]; 

	    if( output_array[ m ] != 0 ) {
		numerrors++ ; 
		itemp = abs( (long) output_array[ m ]) ; 
		if( itemp > peakamp ) peakamp = itemp ; 
		sumerrors += ((float) itemp / 32768.0) ; 
		
		temp = (float) (filesamp) /  (float) lsr ; // SECONDS
		temp2 = (int) (temp / 60.) ; // MINUTES
		temp = temp - (float) temp2 * 60. ; // SECS
		if( verbose )fprintf( stderr, "\n(%d min. %f sec): %d   ",
		    (int) temp2,  temp, itemp ) ;  

	    }

	    // RIGHT CHANNEL
	    output_array[ mm ] = firstdata[ mm ] -  seconddata[ mm ]; 
	    if( output_array[ mm ] != 0 ) {
		numerrors++ ; 
		itemp = abs( (long) output_array[ mm ]) ; 
		if( itemp > peakamp ) peakamp = itemp ; 
		sumerrors += ((float) itemp / 32768.0) ; 

		temp = (float) (filesamp) /  (float) lsr ; // SECONDS
		temp2 = (int) (temp / 60.) ; // MINUTES
		temp = temp - (float) temp2 * 60. ; // SECS
		if( verbose )fprintf( stderr, "\n(%d min. %f sec): %d   ",
		    (int) temp2,  temp, itemp ) ;  



	    }

	    filesamp++ ; 

	}
	sampcount += nsamples ; 

	write( fdoutput,  &output_array, sizeof(short) * nsamples * 2 ) ;

	/* READ IN THE VALUES */
	numreadl = read( fdfirst,   &firstdata,  BLOCKSIZE * sizeof(short)  ) ; 
	numreadr = read( fdsecond,  &seconddata, BLOCKSIZE * sizeof(short) ) ; 

	}




    
     

/**********/

/* PUT THE HEADER ON THE FILE AND CLOSE IT */

        /* Now,create new header  and slap it at the front of this soundfile*/

        newheader.magic = SND_MAGIC;
        odata_offset = 1024 ;
        newheader.dataLocation = odata_offset;
        newheader.dataFormat = SND_FORMAT_LINEAR_16 ;
        newheader.samplingRate = lsr ;
        newheader.channelCount = 2 ;


        if(stat(outputfilename,&st))  {
                fprintf(stderr, "putlength: (#2) Couldn't stat file\n");

                exit(1);

        }
        newheader.dataSize = (int)st.st_size - odata_offset ;
	 /* this captures the whole size of the file */
/*
fprintf( stderr,  "\nDATA SIZE: newheader.dataSize = %d,  %d",  (int) newheader.dataSize, (int)st.st_size  ) ; 
*/

    	lseek(fdoutput, 0, SEEK_SET);
        if((write( fdoutput, &newheader, sizeof(newheader) ) == -1))
                        fprintf(stderr,  "\ncouldn\'t write header\n\n");
        close( fdoutput ) ;
        close( fdfirst ) ;        
	close( fdsecond ) ;


    avgerror = sumerrors / (float) numerrors ; 
    avgerrordB = (float) (20. * log10((double)  avgerror )) ; 

    fprintf( stderr, "\nTOTAL SAMPLES COMPARED (INCLUDES BOTH CHANNELS): %d\n",  
	sampcount*2 ) ; 
    fprintf( stderr, "\n\nTOTAL NUMBER OF DEVIANT SAMPLES (INCLUDES BOTH CHANNELS): %d, (%f PER CENT)\n",  
	numerrors,  100. * (float) numerrors / (float) (sampcount*2)  ) ; 

    if( numerrors > 0 ){
	fprintf( stderr, "\nPEAK ERROR VALUE: %f ( %f dB )",  
	    ((float) peakamp / 32768.0),  
		(float) (20. * log10((double)  ((float) peakamp / 32768.0) )) ) ; 
	fprintf( stderr, "\nAVERAGE ERROR VALUE: %f ( %f dB )\n\n",  avgerror,  avgerrordB ) ; 
    }
exit(0) ; 




}



char crack(argc, argv, flags, ign)
    int argc; char **argv; char *flags; int ign;
{
    char *pv, *flgp, *index();
    while ((arg_index) < argc)
	{
	if (pvcon != NULL)
	    pv = pvcon;
	else
	    {
	    if (++arg_index >= argc) return(NULL); 
	    pv = argv[arg_index];
	    if (*pv != '-') 
		return(NULL);
	    }
	pv++;		/* skip '-' or prev. flag */
	if (*pv != NULL) 
	    {
	    if ((flgp=index(flags, *pv)) != NULL)
		{
		pvcon = pv;
		if (*(flgp+1) == '|') { arg_option = pv+1; pvcon = NULL; }
		return(*pv);
		}
	    else
		if (!ign)
		    {
		    fprintf(stderr, "%s: no such flag: %s\n", argv[0], pv);
		    return(0);
		    }
		else
		    pvcon = NULL;
	    }
	pvcon = NULL;
	}
    return(NULL);
    }


/* NEW HEADER STUFF */

typedef struct {
	int magic;          /* must be equal to SND_MAGIC */
	int dataLocation;   /* Offset or pointer to the raw data */
	int dataSize;       /* Number of bytes of data in the raw data */
	int dataFormat;     /* The data format code */
	int samplingRate;   /* The sampling rate */
	int channelCount;   /* The number of channels */
	char info[4];       /* Textual information relating to the sound. */
} 
SNDSoundStruct;

#define SND_MAGIC ((int)0x2e736e64)
#define SND_FORMAT_UNSPECIFIED          (0)
#define SND_FORMAT_MULAW_8              (1)
#define SND_FORMAT_LINEAR_8             (2)
#define SND_FORMAT_LINEAR_16            (3)
#define SND_FORMAT_LINEAR_24            (4)
#define SND_FORMAT_LINEAR_32            (5)
#define SND_FORMAT_FLOAT                (6)
#define SND_FORMAT_DOUBLE               (7)
#define SND_FORMAT_INDIRECT             (8)
#define SND_FORMAT_NESTED               (9)
#define SND_FORMAT_DSP_CORE             (10)
#define SND_FORMAT_DSP_DATA_8           (11)
#define SND_FORMAT_DSP_DATA_16          (12)
#define SND_FORMAT_DSP_DATA_24          (13)
#define SND_FORMAT_DSP_DATA_32          (14)
#define SND_FORMAT_DISPLAY              (16)
#define SND_FORMAT_MULAW_SQUELCH        (17)
#define SND_FORMAT_EMPHASIZED           (18)
#define SND_FORMAT_COMPRESSED           (19)
#define SND_FORMAT_COMPRESSED_EMPHASIZED (20)
#define SND_FORMAT_DSP_COMMANDS         (21)
#define SND_FORMAT_DSP_COMMANDS_SAMPLES (22)
#define SND_FORMAT_ADPCM_G721           (23)
#define SND_FORMAT_ADPCM_G722           (24)
#define SND_FORMAT_ADPCM_G723_3         (25)
#define SND_FORMAT_ADPCM_G723_5         (26)
#define SND_FORMAT_ALAW_8               (27)
#define SND_FORMAT_AES                  (28)
#define SND_FORMAT_DELTA_MULAW_8        (29)

int 
mySNDGetDataPointer(SNDSoundStruct *sndfile,char **address,int *length,int *width)
{
	*length = (sndfile->dataSize)/2;
	*address = (char *)(sndfile +(sndfile->dataLocation));
	return(0);
}

int 
mySNDAlloc(SNDSoundStruct **ss,int datasize,int format,int srate,
	int channelcount,int infosize)
{
	*ss =(SNDSoundStruct *) malloc(infosize);
	(*ss)->dataLocation = infosize;
	(*ss)->dataSize = datasize;
	(*ss)->dataFormat = SND_FORMAT_LINEAR_16;
	(*ss)->channelCount = channelcount;
	(*ss)->samplingRate = srate;
	return(0);
}

int 
mySNDReadSoundfile(char *sndfile,SNDSoundStruct **snd)
{
	int fd;
	fd = open(sndfile,O_RDONLY,0);
	*snd = (SNDSoundStruct *)malloc(28);
	if (fd != -1)
	{
		lseek(fd,0L,0);
		read(fd,*snd,24);
		if((*snd)->magic != SND_MAGIC) {
			close(fd);
			return(-2);
		}
	}
	else return(-1);
	close(fd);
	return(0);
}


int getsfstats(char *filename, int *sr, int *chan, float *dur, int *data_offset, int *format)
{
SNDSoundStruct *ss;
int mySNDReadSoundfile(char *,SNDSoundStruct **),k_err;

	k_err = mySNDReadSoundfile(filename,&ss);
		if(k_err == -2){
			banner(); 
			fprintf(stderr,  "\n\n********\nYOUR SOUNDFILE:\n\n------>  %s <------\n********",filename);
			fprintf(stderr,  "\nIS NOT A NEXT (.snd) FORMAT FILE.\n" ) ;
			*chan=0; *sr=0;
			return(-2);
		}else if(k_err == -1) {
			banner(); 
			fprintf(stderr,  "\n\n********\nCANNOT OPEN SOUNDFILE:\n\n------>  %s <------\n********",filename);
			*chan=0; *sr=0;
			return(-1);
		}else{
		    *chan=ss->channelCount;
		    *sr=ss->samplingRate;
		    *dur = (double)ss->dataSize/(double)(ss->channelCount * ss->samplingRate * 2);
		    *data_offset = ss->dataLocation;
		    *format = ss->dataFormat;
		    return(1);
		}
}

int banner(){
    if( bannerflag == 0 )
    fprintf( stderr,  "\n\n*************************************\n*********   MONOTOSTEREO   **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}

int pd( i ) int i ; { fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
	
