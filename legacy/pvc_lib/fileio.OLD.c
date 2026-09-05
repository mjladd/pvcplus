#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pv.h"

#define SND_MAGIC ((int)0x2e736e64)
#define SND_FORMAT_UNSPECIFIED          (0)
#define SND_FORMAT_MULAW_8              (1)
#define SND_FORMAT_LINEAR_8             (2)
#define SND_FORMAT_LINEAR_16            (3)
#define SND_FORMAT_LINEAR_24            (4)
#define SND_FORMAT_LINEAR_32            (5)
#define SND_FORMAT_FLOAT                (6)
#define SND_FORMAT_DOUBLE               (7)

#define BLOCKSIZE 1024
#define AMPSTATINC .25
#define OSCILBANKGAIN 1.7782794
/*
*
*/

/* CRACK STUFF */
extern int arg_index;
extern char *arg_option;
extern char *pvcon;
extern char *index();

int bannerflag=0,  banneri(),  bannero() ;  





/************************************************************/
int openfiles(){


//******
    flinflag=0; 

   if( outchan >= ichan ){
      fprintf( stderr, "\n\nTHE AVAILABLE CHANNELS ARE 1-%d, NOT %d. BYE.\n\n",\
	     ichan, outchan+1 ) ;
        exit(0)  ;
   }
    // TOTAL SAMPS OUT FOR TPROP COMPUTATION
    ttlsamps = (int) ((float) (end_sample - begin_sample)) * tfactor ; 


    // OPEN INPUT FILE
    ifd = fopen( ifile,  "r" ) ; 

    // SKIP TO DATA OFFSET + BEGIN
    if( iformat == 3) fseek( ifd, idata_offset + (2 * begin_sample * ichan), SEEK_SET ) ;
    else if( iformat == 6 ) fseek( ifd, idata_offset + (4 * begin_sample * ichan), SEEK_SET ) ;
    else{
	fprintf( stderr,  "\n\nUNKNOWN FORMAT. (openfiles)\n\n" ) ; exit( -1) ; 
    }

    if( outputoff == 0 ){
//fprintf( stderr, "\n\nOPENING OUTPUT FILE....." ) ; 
	// OPEN OUTPUT SOUND FILE
	ofd = open(ofile,O_RDWR,(0644));

	// POSITION TO DATA BEGIN
	lseek(ofd, odata_offset, SEEK_SET);
    }

    return(1) ; 

}

int outfile_setup(int argc, char **argv )
{
       char	ch
		;

    int		n,
		exflag, 
		i,  
		c		
    ;
	
    short int	idata
		; 
		
    float	temp    ;

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

    end_sample = (int) (outdur * (float) isr ) ; 
    sample = begin_sample =  0 ;
  
     // TOTAL SAMPS OUT FOR TPROP COMPUTATION
    ttlsamps = (int) ((float) (end_sample - begin_sample)) ; 

//****************************************************************
//*************** SET UP OUTPUT FILE
//****************************************************************

    if( arg_index >= argc  ){
	prt( "\nWHERE IS YOU OUTPUT FILE NAME?\n\n...................BYE." ) ; 
    }else{
/* GET OUTPUT SOUNDFILE NAME */
	ofile = argv[arg_index] ; 
    }	 

/* OPEN FOR BUSINESS */
    
    ofd = open(ofile,O_RDWR,(0644));
    if(ofd < 0) {


	fprintf( stderr,  "\n\n%s DOES NOT YET EXIST. I'LL MAKE IT.\n", ofile );
	exflag = open(ofile, O_CREAT,(0644)); 
	if( exflag == -1 ){
		
	    bannero() ;
		fprintf( stderr,  "\n\n I COULD NOT MAKE THE FILE:\n\n-----> %s <-------\n\n",  ofile ) ; 
		fprintf( stderr,  "\n\n CHECK THE PATH OF YOUR FILE. ....BYE.\n\n" ) ; 
		exit( 0 ) ; 
	}
	
	ofd = open(ofile, O_RDWR,(0644));
    }else{
	open(ofile, O_TRUNC, (0644));
    }


/* WRITE 1024 dummy bytes for the header */

    idata = 0 ;
    for( i = 0; i < 512 ; i++)
	write(ofd,  &idata, sizeof(short int) ) ;

/* MAKE IT A SOUNDFILE BY WRITING THE HEADER */
/* Now,create new header  and slap it at the front of this soundfile*/

    newheader.magic = SND_MAGIC;
    odata_offset = 1024 ;
    newheader.dataLocation = odata_offset;
   // oformat = SND_FORMAT_LINEAR_16 ;
    if( outputformat == 0 ) oformat = iformat ;
    else if( outputformat == 1 )oformat = SND_FORMAT_LINEAR_16 ;
    else if( outputformat == 2 )oformat = SND_FORMAT_FLOAT ; 
    else {
	fprintf( stderr,  "\n\nUNKNOWN OUTPUT FORMAT. (outfile_setup)\n\n" ) ; exit( -1) ; 

    }
    newheader.dataFormat = oformat ;
    osr = isr ; 
    newheader.samplingRate = osr ;
    if( channelflag == 0 ){
	// SET OUTPUT NUMBER TO INPUT
	ochan = ichan ; 
    }else if( channelflag == -1 ){
	// PRESELECTED
	ochan = ochan ; 
    }else if( channelflag == -2 ){
	// PRESELECTED
	ochan = ichan ; 
    }else{
	// JUST ONE
	ochan = 1 ; 
    }

    if( channelflag == -2 )
	newheader.channelCount = 1 ;
    else
	newheader.channelCount = ochan ;

prbanner( "OUTPUT SOUNDFILE",  69  ) ; 
    prs( ofile,  "OUTPUT FILE: FILENAME " );
    pri( osr,  "OUTPUT FILE: SAMPLE RATE" ) ; 
    pri( newheader.channelCount,  "OUTPUT FILE: NUMBER OF CHANNELS" ) ; 
    if( oformat == SND_FORMAT_LINEAR_16 )
	prt( "OUTPUT FILE FORMAT: 16-BIT INTEGER" ) ; 
    else if( oformat == SND_FORMAT_FLOAT ) 
	prt( "OUTPUT FILE FORMAT: 32-BIT FLOAT" ) ; 
    else{
       	fprintf( stderr,  "\n\nUNKNOWN OUTPUT FORMAT. (openfiles)\n\n" ) ; exit( -1) ; 
    }
    
        
    if(stat(ofile,&st))  {
                fprintf(stderr, "\n\nputlength:  Couldn't stat file\n");
                exit(1);
    }
    newheader.dataSize = (int)st.st_size - odata_offset ;

    // this captures the whole size of the file

    lseek(ofd, 0, SEEK_SET);
    if((write( ofd, &newheader, sizeof(newheader) ) == -1))
                        printf("(A) couldn\'t write header\n");

    // POSITION TO DATA BEGIN
    lseek(ofd, odata_offset, SEEK_SET);


    return(1) ; 


}



int setupfiles(int argc, char **argv)
{
       char	ch
		;

    int		n,
		exflag, 
		i,  
		c		
    ;
	
    short int	idata
		; 
		
    float	temp    ;

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

// ZERO PEAK AMPS ON INPUT FILE
    for( i = 0; i < 4; i++ )ipeakamp[ i ] = 0. ; 

// GET SOUNDFILE NAME AND HEADER INFO

   ifile = argv[arg_index] ;

        exflag = getsfstats(ifile, &isr, &ichan, &idur, \
                &idata_offset, &iformat);
	

	if ( exflag == -2 ) {
	    banneri() ;
	    fprintf( stderr, "\n\n\t....FIX THE FORMAT OF YOUR INPUT FILE. BYE.\n\n" ) ;
	    exit(0) ; 
	}

	if( exflag == -1 ) {
	    banneri() ;
	    fprintf( stderr, "\n\n\t....FIX YOUR INPUT FILE (PATH)NAME. BYE.\n\n" ) ;
	    exit(0) ; 
	}


if( endt <= 0. ) endt = idur ; 
if( begint <= 0. ) begint = 0. ; 

prbanner( "INPUT SOUNDFILE",  69 ) ; 
prs( ifile,  "INPUT FILE: FILENAME " );
pri( isr,  "INPUT FILE: SAMPLE RATE" ) ; 
pri( ichan,  "INPUT FILE: NUMBER OF CHANNELS" ) ; 
prf( idur,  "INPUT FILE: DURATION" ) ; 
prf( begint,	"INPUT FILE: BEGIN TIME" ) ; 
prf( endt,	"INPUT FILE: END TIME"	 ) ; 
if( iformat == 3 ) prt( "INPUT FILE FORMAT: 16-BIT INTEGER" ) ; 
else if( iformat == 6 ) prt( "INPUT FILE FORMAT: 32-BIT FLOAT" ) ; 
else{     fprintf( stderr,  "\n\n========>  YOUR INPUT FILE FORMAT IS NOT SUPPORTED \n\n" ) ; 
    exit( -1 ) ; 
}	
    if( (endt != -1.) && (endt <= begint)){
	    banneri() ;
	fprintf( stderr,
	     "\n\nBEGIN TIME = %f\nEND TIME = %f\n\n",  begint,  endt  ) ;

	fprintf( stderr,
	     "\n\nYOUR END TIME IS BEFORE OR EQUAL TO YOUR BEGIN TIME! BYE.\n\n" ) ;
	exit(0) ; 
    }
	
    if( endt > idur ){
	    banneri() ;
	    fprintf( stderr, "%s%f%s%f%s", "\nYOUR END TIME OF  ", 
		endt, 
		"  SECONDS \nEXCEEDS THE FILE DURATION OF  ", 
		idur, 
		" SECONDS.\nYOUR END TIME WILL BE RESET TO THE DURATION OF THE FILE.\n\n" 
		 ) ;
	   	
	    endt = idur ; 
    }



    end_sample = (int) (endt * (float) isr ) ; 
    sample = begin_sample =  (int) (begint * (float) isr ) ;
  
    if( outputoff == 1 ) {
	// NO AUDIO  FILE OUTPUT: SET ochan and exit
fprintf( stderr, "\n\n...................NO AUDIO OUTPUT FILE\n" ) ; 
        if( (channelflag == 0) || (channelflag == -2) ){
	    ochan = ichan ; 
	}else{
	    ochan = 1 ; 
        }

	return(1) ;
    }     


//****************************************************************
//*************** SET UP OUTPUT FILE
//****************************************************************

    arg_index++ ; 

    if( arg_index >= argc  ){
	bannero() ;
	ofile =  "pv.out.snd" ; 
	fprintf( stderr, "\n\n......USING DEFAULT OUTPUT FILENAME........\n\n" ) ;
    }else{
/* GET OUTPUT SOUNDFILE NAME */
	ofile = argv[arg_index] ; 
    }	 

/* OPEN FOR BUSINESS */
    
    ofd = open(ofile,O_RDWR,(0644));
    if(ofd < 0) {


	fprintf( stderr,  "\n\n%s DOES NOT YET EXIST. I'LL MAKE IT.\n", ofile );
	exflag = open(ofile, O_CREAT,(0644)); 
	if( exflag == -1 ){
		
	    bannero() ;
		fprintf( stderr,  "\n\n I COULD NOT MAKE THE FILE:\n\n-----> %s <-------\n\n",  ofile ) ; 
		fprintf( stderr,  "\n\n CHECK THE PATH OF YOUR FILE. ....BYE.\n\n" ) ; 
		exit( 0 ) ; 
	}
	
	ofd = open(ofile, O_RDWR,(0644));
    }else{
	open(ofile, O_TRUNC, (0644));
    }


/* WRITE 1024 dummy bytes for the header */

    idata = 0 ;
    for( i = 0; i < 512 ; i++)
	write(ofd,  &idata, sizeof(short int) ) ;

/* MAKE IT A SOUNDFILE BY WRITING THE HEADER */
/* Now,create new header  and slap it at the front of this soundfile*/

    newheader.magic = SND_MAGIC;
    odata_offset = 1024 ;
    newheader.dataLocation = odata_offset;
    if( outputformat == 0 ) oformat = iformat ;
    else if( outputformat == 1 )oformat = SND_FORMAT_LINEAR_16 ;
    else if( outputformat == 2 )oformat = SND_FORMAT_FLOAT ; 
    else {
	fprintf( stderr,  "\n\nUNKNOWN OUTPUT FORMAT. (outfile_setup)\n\n" ) ; exit( -1) ; 

    }


    newheader.dataFormat = oformat ;
    osr = isr ; 
    newheader.samplingRate = osr ;
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

    if( channelflag == -2 )
	newheader.channelCount = 1 ;
    else
	newheader.channelCount = ochan ;

prbanner( "OUTPUT SOUNDFILE",  69  ) ; 
    prs( ofile,  "OUTPUT FILE: FILENAME " );
    pri( osr,  "OUTPUT FILE: SAMPLE RATE" ) ; 
    pri( newheader.channelCount,  "OUTPUT FILE: NUMBER OF CHANNELS" ) ; 
    if( oformat == SND_FORMAT_LINEAR_16 )
	prt( "OUTPUT FILE FORMAT: 16-BIT INTEGER" ) ; 
    else if( oformat == SND_FORMAT_FLOAT ) 
	prt( "OUTPUT FILE FORMAT: 32-BIT FLOAT" ) ; 
    else{
       	fprintf( stderr,  "\n\nUNKNOWN OUTPUT FORMAT. (openfiles)\n\n" ) ; exit( -1) ; 
    }
    
        
    if(stat(ofile,&st))  {
                fprintf(stderr, "\n\nputlength:  Couldn't stat file\n");
                exit(1);
    }
    newheader.dataSize = (int)st.st_size - odata_offset ;

    // this captures the whole size of the file

    lseek( ofd, 0, SEEK_SET);
    if((write( ofd, &newheader, sizeof(newheader) ) == -1))
                        printf("(A) couldn\'t write header\n");


    close(ofd) ; 

    return(1) ; 

}

/************************************************************/

int bufferin( float *V ) 
{

    static int bufferinsamps=0,  buffinchan=0 ; 
    static short int   inbuff[ BLOCKSIZE * 4 ] ;
    static float   finbuff[ BLOCKSIZE * 4 ] ;

    if( IO_reset == 1 ){
	// RESET
	bufferinsamps = 0 ; 
	buffinchan = 0 ;
	//sample = 0 ; 
	inbuffn = 0 ; 
	doneflag=0 ;  

	IO_reset = 0 ;  
    }



    if( buffinchan != channow ){
	// RESET
//	prt( "BUFFERIN: RESETING ...." ) ; 
	inbuffn = 0 ; 
	doneflag = 0 ;
	sample = begin_sample ;
	bufferinsamps = 0 ;
	samps = 0 ;    
    // SKIP TO DATA OFFSET + BEGIN
//	pri( (idata_offset + (2 * begin_sample * ichan)), "BUFFERIN: REQUESTED BYTE POSITION" ) ; 

    if( iformat == 3) fseek( ifd, idata_offset + (2 * begin_sample * ichan), SEEK_SET ) ;
    else if( iformat == 6 ) fseek( ifd, idata_offset + (4 * begin_sample * ichan), SEEK_SET ) ;
    else{
	fprintf( stderr,  "\n\nUNKNOWN FORMAT. (openfiles)\n\n" ) ; exit( -1) ; 
    }



//	pri( ( ftell( ifd ) ),  "INPUT FILE POSITION" ) ;     


	buffinchan = channow ;
    }

	// INPUT SOUNDFILE BUFFERING 
	
    if( inbuffn == 0 ){
	// BUFFER EMPTY
	if( doneflag != 1 ){

	// READ IN A BLOCK

	if( iformat == 3) 
	    bufferinsamps = fread( &inbuff, sizeof(short int), BLOCKSIZE * ichan,  ifd ) ; 
	else if( iformat == 6 ) 
	    bufferinsamps = fread( &finbuff, sizeof(float), BLOCKSIZE * ichan,  ifd ) ; 
	else{
	    fprintf( stderr, "\n\nUNKNOWN FORMAT (bufferin)\n\n" ) ; exit( -1 ) ; 
	}
	// COMPUTE THE NUMBER OF SAMPLES
	bufferinsamps = bufferinsamps / ichan ; 
//	pri( bufferinsamps,  "BUFFERIN: bufferinsamps" ) ; 

	if( bufferinsamps == 0 ){
	    // NO MORE SAMPS
//	    prt( "(A) NO MORE SAMPS!!!!!!!!!" ) ; 
		doneflag = 1 ;  
    		return(0); 
	}	
		// LAST BUFFER
	if( (end_sample - sample) < bufferinsamps ){
		bufferinsamps = (end_sample - sample) ;
		doneflag = 1 ;  
	}
	
	// ADVANCE BUFFER LAST SAMPLE
	sample += bufferinsamps ; 
	
	}else{
	    // NO MORE SAMPS
//	    prt( "(B) NO MORE SAMPS!!!!!!!!!" ) ; 
    		return(0); 
	}
    }

    // PASS BACK A VALUE
    if( iformat == 3 ) *V =  ((float) inbuff[ (inbuffn * ichan) + outchan ]) / 32768.0 ; 
    else  *V =  ( finbuff[ (inbuffn * ichan) + outchan ]) ;

    // TAKE AS PEAK FOR THIS CHANNEL IF SO
    if( *V > ipeakamp[ outchan ] ) ipeakamp[ outchan ] = *V ; 

    inbuffn++ ; 
    if( inbuffn == bufferinsamps ){
	    // RESET TO ZERO TO TRIGGER NEXT READ
	    inbuffn = 0 ; 
	    bufferinsamps = 0 ; 
    }
    return(1) ; 
	

}
/************************************************************/

int bufferout(float *outbuff,  int I, int flushflag)
   {
   

 
    static int	numsamps=0, 
		blkcount=0,
		n, j, m, noldread, 
		exflag, 
		flag=0, 
		nblockover=0,
		blockpeakn, 
		outbuffpt,     
		first=1,
		first2=1,  
		buffoutchan=0
		 ; 
    
    static short int	idata, 
			out[ BLOCKSIZE  * 4]
		    ; 

    static float    in[ BLOCKSIZE ], 
		    fout[ BLOCKSIZE  * 4] 
		    ; 
    		
    static float dur,
		nn,
		blockpeakt,
		peakampt[4],   
		peakamp[4],
		lastpeakamp=0, 
		nextt,
		blockpeakamp=0,
		statpeakamp=0, 
		temp, 
		gain    
    ;

    int data_offset=1024 ; 


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

        /* OUTPUT SOUNDFILE HEADER */

    newheader.magic = SND_MAGIC;
    newheader.dataLocation = odata_offset;
    newheader.dataFormat = oformat ;
    newheader.samplingRate = osr ;
    newheader.channelCount = ochan ;



     if( (buffoutchan != channow) || (first) ){
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
	    }
	    else gain = 1. ; 
	}

	if((quiet != 0) && (first2)){
	    fprintf( stderr,  "\n*********************************************************************");
	    fprintf( stderr,  "\n**  PEAK AMPLITUDE STATISTICS **" ) ; 
	    fprintf( stderr,  "\n*********************************************************************");
	    fprintf( stderr, "\n     TIME          PEAKAMP      DECIBELS    (LAST DECIBELS PEAK)" );	    
	    fprintf( stderr,  "\n*********************************************************************");
	    first2 = 0 ; 
	}



     outbuffpt = 0 ; 

while( outbuffpt < I ){


// TRANSFER VALUES INTO in[]
    while( (numsamps < BLOCKSIZE) && ( outbuffpt < I ) ){
	in[ numsamps ] = outbuff[ outbuffpt ] * gain  ; 
	numsamps++; outbuffpt++ ; samps++ ; 
    }



	if( (numsamps < BLOCKSIZE) && (flushflag == 0) ){
	    // in BUFFER NOT YET FULL AND WE ARE NOT FLUSH/CLOSING
	    return(0) ;     
    
	}else{
	    
	    nblockover = 0 ; blockpeakamp = 0 ; blockpeakn = 0 ; 
	    
	    // IF MULTI-CHANNEL OUTPUT, AND NOT FIRST CHANNEL
	    // READ IN A BLOCK INTO THE OUT ARRAY



	    if( (ochan > 1) && (channow > 0) ){
		// SGI WAY    j = tell( ofd ) ;
		j = lseek( ofd, 0, SEEK_CUR ) ;
 
		if( oformat == SND_FORMAT_LINEAR_16 ) noldread = 
		    read( ofd,  &out, sizeof(short int) * numsamps * ochan ) ;
		else if( oformat == SND_FORMAT_FLOAT ) noldread = 
		    read( ofd,  &fout, sizeof(float) * numsamps * ochan ) ;
		else{
		    fprintf( stderr, "\n\nUNKNOWN OUTPUT FORMAT\n\n" ) ; exit( -1 ) ; 
		}

//		pri( noldread,  "noldread" ) ; 
		// RESET THE FILE POSITION TO CONTINUE WRITING 
		lseek( ofd, j,  SEEK_SET ) ; 
		
	    }

	    for( n = 0,  m = channow ;  n < numsamps; n++,  m += ochan ){

		 temp = (float) fabs( (double) in[ n ] ) ; 
		 if(temp > peakamp[channow]) {
			peakamp[channow] = temp ;
			peakampt[channow] = t ; 
		 } 
		 if(temp > blockpeakamp) {
			blockpeakamp = temp ; 
			blockpeakt =  t ; 
		 }
		 if(temp > statpeakamp) {
			statpeakamp = temp ; 
		 }


		// TRANSFER TO FLOAT OUT AS SHORTS
		fout[ m ] =  in[ n ]  ; 


		if( oformat == SND_FORMAT_LINEAR_16 ){
		    // CLIPPING  FOR INTEGERS
		    if( in[ n ] > 1. ){ in[ n ] = 1. ; nsover[channow]++ ; nblockover++ ; } 

		    if( in[ n ] < -1. ){ in[ n ] = -1. ; nsover[channow]++ ; nblockover++ ;  } 
		    // TRANSFER TO OUT AS SHORTS
		    out[ m ] =  (short int) ( in[ n ] * 32767. ) ; 

		}
/* AMP STATS */
//		t = (((float) blkcount * (float) BLOCKSIZE) + (float) n) / (float) osr ;
		if( (t >= nextt) && (quiet != 0) && ( flag == 0 ) ){
		    temp = (float) (20. * log10( (double) statpeakamp ))  ; 
		    fprintf( stderr,  "\n(%6.2f -%6.2f)   ", 
			    nextt-ampstatinc, nextt )     ; 
		    if( temp < -100 ){
			fprintf( stderr, "   *   " ) ;
		    }else{		    
			fprintf( stderr,  "%7.4f       %7.3f", 
			    statpeakamp,  temp ) ; 
		    }
		    nextt += ampstatinc ;		
		    statpeakamp = 0 ;  

		    if(peakamp[channow] > lastpeakamp){
			temp = (float) (20. * log10( (double) peakamp[channow] )) ; 
			if( temp > -100 ){
			    fprintf( stderr,  "     %7.3f", temp   ) ; 
			    lastpeakamp = peakamp[channow] ;
			} 
		    }  

		}
/**/


	    }

/**/  
	    if( oformat == SND_FORMAT_LINEAR_16 ) write( ofd,  &out, sizeof(short int) * numsamps * ochan ) ;
	    else if( oformat == SND_FORMAT_FLOAT ) write( ofd,  &fout, sizeof(float) * numsamps * ochan ) ;
	    else{
		fprintf( stderr, "\n\nUNKNOWN OUTPUT FORMAT\n\n" ) ; exit( -1 ) ; 
	    }
	    blkcount++ ; 
	    numsamps = 0 ; 

/* PRINT WARNING IF SAMPLES OUT OF RANGE */
	if( ( flag == 0 ) && (nsover[channow] > 0) ){
	    flag=1; 
	    fprintf( stderr, "\n*** SAMPLES OUT OF RANGE ***** \n   (Samples out of range will be clipped.)" ) ; 
	    fprintf( stderr,  "\n*********************************************************************");
	    fprintf( stderr, "\n     TIME        PEAKAMP     DECIBELS      NUMBER_OF_SAMPLES" );	    
	    fprintf( stderr,  "\n*********************************************************************");
	}
	if( nblockover > 0 ){
	    fprintf( stderr,  "\n-> %6.3f        %7.4f       %7.3f      %d", 
		blockpeakt, blockpeakamp, (float) (20. * log10( (double) blockpeakamp )), nblockover )     ; 
	}

	

/*
 UPDATE THE FILE HEADER WITH THE DURATION/SIZE CHANGE AFTER EVERY
20 blocks 
*/	    

	    if( ((blkcount%20) == 0) && (channow == 0) ){
		if(stat(ofile,&st))  {
		    fprintf(stderr, "\n\nputlength:  Couldn't stat file\n");
		    exit(1);
		}
    /* this captures the whole size of the file */
	    newheader.dataSize = (int)st.st_size - odata_offset ;

// SAVE THE CURRENT POSITION
	    // SGI WAY     j = tell( ofd ) ; 
	    j = lseek( ofd, 0, SEEK_CUR ) ;


    	    lseek(ofd, 0, SEEK_SET);
	    if((write( ofd, &newheader, sizeof(newheader) ) == -1))
                        printf("(B) couldn\'t write header\n");
 	    
/* RESET THE FILE POSITION TO CONTINUE WRITING */
	    lseek( ofd, j,  SEEK_SET ) ; 

	    }

	}

}

if(flushflag == 0){
    // NO FLUSH/CLOSE

    return(1) ; 


}else{
    //  CLOSE

/* PRINT WARNING IF SAMPLES OUT OF RANGE */
//	if(nsover[channow] > 0 ){ 
//	    fprintf( stderr,  "\nTOTAL SAMPLES OUT OF RANGE:    %d",  nsover[channow] ) ; 
//	}
	if( (blkcount > 0) && (quiet != 0)){
	    fprintf( stderr, "\n\n============= PEAK AMPLITUDE ========================================" ) ; 
	    fprintf( stderr, "\nCHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)" );	    
	    fprintf( stderr,  "\n.....................................................................");

		fprintf( stderr,  "\n%d          %7.3f          %7.4f     %7.3f", 
		    channow+1,  peakampt[channow], peakamp[channow], (float) (20. * log10( (double) peakamp[channow] )) )     ;
		if( nsover[channow] > 0 )fprintf( stderr, "     ---> %d <---", nsover[channow] ) ;  

	    fprintf( stderr,  "\n*********************************************************************");
	}
	fprintf( stderr,  "\n\n" ) ; 



    if( (channow + 1)  < ochan ){
    // MULTI-CHANNEL
//	prt( "DO NEXT CHANNEL" ) ; 
    // RESET INPUT FILE
	 ; 

    // RESET OUTPUT FILE    
	lseek(ofd, odata_offset, SEEK_SET);


    // ZERO FOR RESTART




    }else{
    // CLOSE

/* PUT THE HEADER ON THE FILE */



        if(stat(ofile,&st))  {
                fprintf(stderr, "\n\nputlength:  Couldn't stat file\n");
                exit(1);
        }
        newheader.dataSize = (int)st.st_size - odata_offset ;

	 /* this captures the whole size of the file */

    	lseek(ofd, 0, SEEK_SET);
        if((write( ofd, &newheader, sizeof(newheader) ) == -1))
                        printf("(C2) couldn\'t write header\n");


// RESCALE IT IF IT IS A FLOAT 

    if( oformat == SND_FORMAT_FLOAT ){
		if(quiet != 0){
	    prline( 69,  "=" ) ;
	    fprintf( stderr, "\n\n                 PEAK AMPLITUDES (BEFORE RESCALE): ALL CHANNELS" ) ; 
	    prline( 69,  "-" ) ;
	    fprintf( stderr, "\nCHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)" );	    
	    fprintf( stderr,  "\n.....................................................................");

	    for(j = 0;  j < ochan; j++){
		fprintf( stderr,  "\n%d          %7.3f          %7.4f     %7.3f", 
		    j+1,  peakampt[j], peakamp[j], (float) (20. * log10( (double) peakamp[j] )) )     ;
		if( nsover[j] > 0 )fprintf( stderr, "     ---> %d <---", nsover[j] ) ;  
	    }
	    prline( 69,  "=" ) ;
	}
	fprintf( stderr,  "\n\n" ) ; 


    }


    rescalefloatfile( peakamp ); 


// CLOSE IT

        close( ofd ) ;

	if(quiet != 0){
	    prline( 69,  "=" ) ;
	    fprintf( stderr, "\n\n                 PEAK AMPLITUDES: ALL CHANNELS" ) ; 
	    prline( 69,  "-" ) ;
	    fprintf( stderr, "\nCHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)" );	    
	    fprintf( stderr,  "\n.....................................................................");

	    for(j = 0;  j < ochan; j++){
		fprintf( stderr,  "\n%d          %7.3f          %7.4f     %7.3f", 
		    j+1,  peakampt[j], peakamp[j], (float) (20. * log10( (double) peakamp[j] )) )     ;
		if( nsover[j] > 0 )fprintf( stderr, "     ---> %d <---", nsover[j] ) ;  
	    }
	    prline( 69,  "=" ) ;
	}
	fprintf( stderr,  "\n\n" ) ; 

    }
}
return(0) ; 
}


float timenow(float dur ){
    
    float v;
    
    t = ((float) samps / (float) ttlsamps ) * dur ; 
    return( 1 ) ; 
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
			close( fd );
			return(-2);
		}
	}
	else return(-1);
	close( fd );
	return(0);
}



int getsfstats(char *filename, int *sr, int *chan, float *dur, int *data_offset, int *format)
{
SNDSoundStruct *ss;
int mySNDReadSoundfile(char *,SNDSoundStruct **),k_err;

	k_err = mySNDReadSoundfile(filename,&ss);
		if(k_err == -2){
			banneri(); 
			fprintf(stderr,  "\n\n********\nYOUR SOUNDFILE:\n\n------>  %s <------\n********",filename);
			fprintf(stderr,  "\nIS NOT A NEXT (.snd) FORMAT FILE.\n" ) ;
			*chan=0; *sr=0;
			return(-2);
		}else if(k_err == -1) {
			banneri(); 
			fprintf(stderr,  "\n\n********\nCANNOT OPEN SOUNDFILE:\n\n------>  %s <------\n********",filename);
			*chan=0; *sr=0;
			return(-1);
		}else{
		    *chan=ss->channelCount;
		    *sr=ss->samplingRate;
		    *dur = (double)ss->dataSize/(double)(ss->channelCount * ss->samplingRate * 2);
		    *data_offset = ss->dataLocation;
		    *format = ss->dataFormat;
		    // FIX DURATION IF FILE IS FLOAT
		    if( *format == 6 ){
			// FLOAT
			*dur = *dur / 2. ; 
		    }
		    return(1);
		}
}

int banneri(){
    if( bannerflag == 0 )
    fprintf( stderr,  "\n\n*************************************\n*********  INPUT SOUNDFILE  **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}

int bannero(){
    if( bannerflag == 0 )
    fprintf( stderr,  "\n\n*************************************\n*********  OUTPUT SOUNDFILE  **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}

int readffthead( int *N, int *D, int *R, int *chans, int *win_type,  float peakamps[],  struct func *p ){

    float temp ; 
    int i ; 

    // READ IN THE FFT FILE HEADER VALUES

if( p->n == 0. ){
    fprintf( stderr,  "\n\nYOU MUST PROVIDE A FILTER FILE. BYE.\n" ) ;
    exit(0); 
} 
//**************
// READ IN  HEADER VALUES

		if( fread( &temp, sizeof(float) , 1, p->fp ) == NULL ){
		    fprintf( stderr,  "\n\n1) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;  exit(0) ;  
		}
		*N = (int) temp ; 
		if( fread( &temp, sizeof(float) , 1, p->fp ) == NULL ){
		    fprintf( stderr,  "\n\n2) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;  exit(0) ;  
		}
		*D = (int) temp ; 
		if( fread( &temp, sizeof(float) , 1, p->fp ) == NULL ){
		    fprintf( stderr,  "\n\n3) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;  exit(0) ;  
		}
		*R = (int) temp ; 
		if( fread( &temp, sizeof(float) , 1, p->fp ) == NULL ){
		    fprintf( stderr,  "\n\n4) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;  exit(0) ;  
		}
		*chans = (int) temp ; 
		if( fread( &temp, sizeof(float) , 1, p->fp ) == NULL ){
		    fprintf( stderr,  "\n\n4) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;  exit(0) ;  
		}
		*win_type = (int) temp ; 
		// PEAKAMPS
		for( i = 0; i <  *chans ; i++){
		    if( fread( &temp, sizeof(float) , 1, p->fp ) == NULL ){
			fprintf( stderr,  "\n\n4) YOUR INPUT ANALYSIS FILE HAS NO VALUES. BYE.\n\n" ) ;  exit(0) ;  
		    }
		    peakamps[i] = temp ; 
		}


    return(1) ; 
}

/************************************************************/

int closeoutsound()
   {
   

 
    static int	numsamps=0, 
		blkcount=0,
		n, j, k,  m, noldread, 
		exflag, 
		flag=0, 
		nsover[4],
		nblockover=0,
		blockpeakn, 
		outbuffpt,     
		first=1, 
		buffoutchan=0
		 ; 
    
    static short int	idata, 
			out[ BLOCKSIZE  * 4]
    ; 

    static float in[ BLOCKSIZE ] ; 
    		
    static float dur,
		nn,
		blockpeakt,
		peakampt[4],   
		peakamp[4],
		lastpeakamp=0, 
		nextt,
		blockpeakamp=0,
		statpeakamp=0, 
		temp, 
		gain    
    ;

    int data_offset=1024 ; 


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

        /* OUTPUT SOUNDFILE HEADER */

    newheader.magic = SND_MAGIC;
    newheader.dataLocation = odata_offset;
    newheader.dataFormat = oformat ;
    newheader.samplingRate = osr ;
    newheader.channelCount = ochan ;


{
    //  CLOSE

/* PUT THE HEADER ON THE FILE AND CLOSE IT */



        if(stat(ofile,&st))  {
                fprintf(stderr, "\n\nputlength:  Couldn't stat file\n");
                exit(1);
        }
        newheader.dataSize = (int)st.st_size - odata_offset ;

	 /* this captures the whole size of the file */

    	lseek(ofd, 0, SEEK_SET);
        if((write( ofd, &newheader, sizeof(newheader) ) == -1))
                        printf("(C1) couldn\'t write header\n");
        close( ofd ) ;

	if(quiet != 0){
	    prline( 69,  "=" ) ;
	    fprintf( stderr, "\n\n                 PEAK AMPLITUDES: ALL CHANNELS" ) ; 
	    prline( 69,  "-" ) ;
	    fprintf( stderr, "\nCHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)" );	    
	    fprintf( stderr,  "\n.....................................................................");

	    for(j = 0;  j < ochan; j++){
		fprintf( stderr,  "\n%d          %7.3f          %7.4f     %7.3f", 
		    j+1,  peakampt[j], peakamp[j], (float) (20. * log10( (double) peakamp[j] )) )     ;
		if( nsover[j] > 0 )fprintf( stderr, "     ---> %d <---", nsover[j] ) ;  
	    }
	    prline( 69,  "=" ) ;
	}
	fprintf( stderr,  "\n\n" ) ; 

    }

return(0) ; 
}


int rescalefloatfile( float peakamp[] ){

    float ampval, peakofout,  peakifout,  tempy ;
    int j,jc,  k, k2,  m, nsamps,  bytesread, byteswritten,   nsampsw ; 
    
    float finbuff[ 4 * BLOCKSIZE ] ; 
    
    // IF THIS IS A FLOAT FILE, RESCALE IT
    
    if( oformat == SND_FORMAT_FLOAT ){




	// FLOAT FILE RESCALE
	    prline( 69,  "-" ) ;

	// FIND GREATEST PEAK AMONG CHANNELS OF OUTPUT FILE
	   // for(j = 0;  j < ochan; j++)prf( peakamp[j], " PEAKAMPS" )  ; 

	    peakofout = -999999. ; 
	    for(j = 0;  j < ochan; j++)
		if( peakamp[j] >  peakofout ) peakofout =  peakamp[j] ; 

	    if( peakofout <= 0. ) {
		fprintf( stderr, "\n\n PEAK OUTPUT AMP IS 0. RESCALE ABORTED.\n\n" ) ; 
		return( 0 ) ; 
	    }
	    
	// FIND GREATEST PEAK AMONG CHANNELS OF INPUT FILE

	    peakifout = -999999. ; 
	    for(j = 0;  j < ochan; j++)if( ipeakamp[j] >  peakifout ) peakifout =  ipeakamp[j] ; 

	    if( peakifout <= 0. ) {
		fprintf( stderr, "\n\n PEAK INPUT AMP IS 0. RESCALE ABORTED.\n\n" ) ; 
		return( 0 ) ; 
	    }
	    
	// MAKE AMP RESCALE VALUE

	if( rescalev > 0. ){
	    // RESCALE TO PEAKAMP OF INPUT FILE 
	    ampval = peakifout  /  peakofout  ; 
	    prf( peakifout, "FLOAT FILE AMPLITUDE RESCALE LEVEL " ) ;  

	}else{
	    // USE INPUT RESCALE VALUE: CONVERT TO AMP
	    ampval = dB_to_amp( rescalev ) / peakofout   ; 

	    tempy = dB_to_amp( rescalev ) ; 
	    prf( tempy, "FLOAT FILE AMPLITUDE RESCALE LEVEL " ) ;  
	}

	//********* NOW RESCALE FILE
	
	// RESET PEAKAMP LEVELS  AND SAMPLES OVER FOR NEW DETECTION
	for( j = 0; j < ochan; j++ )peakamp[ j ] = -999999. ; 
	for( j = 0; j < ochan; j++ )nsover[ j ] = 0. ; 
	
	// RESET FILE TO DATA OFFSET
	lseek(ofd, odata_offset, SEEK_SET);

	// SAVE DATA POSITION
	// SGI WAY   k = tell( ofd ) ; 
	k = lseek( ofd, 0, SEEK_CUR ) ;

	// GET SOME DATA
	bytesread = read( ofd,  &finbuff, sizeof(float) * BLOCKSIZE * ochan ) ; 

	// LOOP FOR RESCALE
	while(  bytesread > 0 ){
	    
	    
	nsamps = ( bytesread  / sizeof(float) ) / ochan ; 
	

	    // SCALE VALUES
	    for( j = 0; j < nsamps; j++ ) {
		// CHANNELS
		for( jc = 0; jc < ochan; jc++ ){
		    k2 = (j * ochan) + jc ; 
		    finbuff[ k2 ] = finbuff[ k2 ] * ampval ; 
		    // GET NEW STATS ON OUTPUT FILE
		    if ( fabs( finbuff[ k2 ] )  > peakamp[ jc ] )
			    peakamp[ jc ] = fabs( finbuff[ k2 ] ) ; 
		    // CHECK FOR OVERFLOW AND CLIP IT AND COUNT IT
		    if( fabs( finbuff[ k2 ])  > 1.0 ){
			// CLIP
			if( finbuff[ k2 ] > 1.0 ) finbuff[ k2 ] = .9999999 ; 
			if( finbuff[ k2 ] < -1.0 ) finbuff[ k2 ] = -.999999 ; 
			// COUNT IT
			nsover[ jc ]++ ; 
		    }
		}

	    }

	    // RESET TO SAVED DATA POSITION
	    lseek( ofd, k,  SEEK_SET ) ; 
	    
	    // WRITE BUFFER BACK OUT 	
	   byteswritten =  write( ofd,  &finbuff, sizeof(float) * nsamps * ochan ) ; 

	    //lseek( ofd, k2,  SEEK_SET ) ; 

//fprintf( stderr, "\nbyteswritten = %d,  k2 = %d",  byteswritten, k2 ) ; 


	    // SAVE DATA POSITION
	    // SGI WAY    k = tell( ofd ) ; 
	    k = lseek( ofd, 0, SEEK_CUR ) ;


	    // GET SOME DATA
	    bytesread = read( ofd,  &finbuff, sizeof(float) * BLOCKSIZE * ochan ) ; 





	} 


    }else{
	// NOT A FLOAT FILE
	return( 0 ) ; 
    }
    
    
    
    return( 1 ) ; 
}
