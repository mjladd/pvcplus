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

#define BLOCKSIZE 1024
#define AMPSTATINC .5
/*
* THIS ROUTINE READS FLOATS FROM THE STANDARD IN AND 
* WRITES THEM AS SHORTS TO THE STANDARD OUT
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
   

    char	*filename, 
		ch, 
		crack()
    ;


    int		n,
		quiet=0, 
		flag=0, 
		nsover=0,
		nblockover=0,
		blockpeakn,    
		numsamps, 
		exflag, 
		sr,
		chan,
		data_offset, 
		format, 
		fd,
		i,  
		outchan, 
		c, 
		Size
		
    ;
	
    short int	idata, 
		out[ BLOCKSIZE ]
    ; 
		
    float	dur,
		nn,
		blockpeakt,
		peakampt,   
		peakamp=0,
		lastpeakamp=0, 
		ampstatinc=AMPSTATINC, 
		nextt=AMPSTATINC,
		t=0,  
		blockpeakamp=0,
		statpeakamp=0, 
		temp,    
		in[ BLOCKSIZE ] 
    ;
    FILE *data_file;

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
	fprintf( stderr,"%s",
	"\n\nUSAGE:\n\n   flin  [ NeXT/Sun format ouput soundfile (\"name\".snd) ]" 
	"\n\n\tflags:"
	"\n\t-p amplitude reports print mode: 0 = off (default), 1 = on" 
	"\n\t-i time interval between amplitude reports : (default = .5)" 
	"\n\n\tThis routine reads the standard in as floats"
	"\n\tand writes it out as shorts to the new or" 
	"\n\texisting NeXT/Sun format output soundfile.\n\n"
	);
	exit(0);
    }
    
    while((ch = crack(argc,argv,"p|i|",0)) != 0)
{
        switch(ch) {
            case 'p': quiet = atoi(arg_option) ; break;
            case 'i': ampstatinc = atof(arg_option) ; break;

        }
    }
 
 nextt = ampstatinc ; 
 
    if( argc < 2  ){
	banner() ; 
	fprintf( stderr, "\n\nWHERE IS THE OUTPUT FILE?  BYE.\n") ;
	exit(0);  	
    }else{

/* GET SOUNDFILE NAME */
/*
   filename = argv[1] ;
*/
   filename = argv[arg_index] ; 
    }	 




/* OPEN FOR BUSINESS */

    fd = open(filename, O_RDWR,(0644));
    
    fd = open(filename,O_RDWR,(0644));
    if(fd < 0) {


	fprintf( stderr,  "%s DOES NOT YET EXIST. I'LL MAKE IT.\n", filename );
	exflag = open(filename, O_CREAT,(0644)); 
	if( exflag == -1 ){
		
	    banner() ;
		fprintf( stderr,  "\n\n I COULD NOT MAKE THE FILE:\n\n-----> %s <-------\n\n",  filename ) ; 
		fprintf( stderr,  "\n\n CHECK THE PATH OF YOUR FILE. ....BYE.\n\n" ) ; 
		exit( 0 ) ; 
	}
	
	fd = open(filename, O_RDWR,(0644));
    }else{
	open(filename, O_TRUNC, (0644));
    }




/* WRITE 1024 dummy bytes for the header */
    idata = 0 ;
    for( i = 0; i < 512 ; i++)
	write(fd,  &idata, sizeof(short int) ) ;
/* MAKE IT A SOUNDFILE BY WRITING THE HEADER */

        /* Now,create new header  and slap it at the front of this soundfile*/

        newheader.magic = SND_MAGIC;
        data_offset = 1024 ;
        newheader.dataLocation = data_offset;
        newheader.dataFormat = SND_FORMAT_LINEAR_16 ;
        newheader.samplingRate = 44100 ;
        newheader.channelCount = 1 ;


        if(stat(filename,&st))  {
                fprintf(stderr, "putlength:  Couldn't stat file\n");
                exit(1);
        }
        newheader.dataSize = (int)st.st_size - data_offset ;

	 /* this captures the whole size of the file */

    	lseek(fd, 0, SEEK_SET);
        if((write( fd, &newheader, sizeof(newheader) ) == -1))
                        printf("couldn\'t write header\n");

    
/* READ IN THE VALUES  AND WRITE THEM OUT AS SHORTS */
    	lseek(fd, 1024, SEEK_SET);
	i = 0 ; 

	numsamps = fread( &in, sizeof(float), BLOCKSIZE, stdin ) ;


	if( quiet != 0 ){
	    fprintf( stderr,  "\n*********************************************************************");
	    fprintf( stderr,  "\n** FLIN: PEAK AMPLITUDE STATISTICS **" ) ; 
	    fprintf( stderr,  "\n*********************************************************************");
	    fprintf( stderr, "\n     TIME          PEAKAMP      DECIBELS    (LAST DECIBELS PEAK)" );	    
	    fprintf( stderr,  "\n*********************************************************************");
	}



	while( numsamps > 0 ) {
	    
	    nblockover = 0 ; blockpeakamp = 0 ; blockpeakn = 0 ; 
	    for( n = 0; n < numsamps; n++ ){

		 temp = (float) fabs( (double) in[ n ] ) ; 
		 if(temp > peakamp) {
			peakamp = temp ;
			peakampt = 
			 (((float) i * (float) BLOCKSIZE) + (float) n) / (float) newheader.samplingRate ; 
		 } 
		 if(temp > blockpeakamp) {
			blockpeakamp = temp ; 
			blockpeakt =  
			 (((float) i * (float) BLOCKSIZE) + (float) n) / (float) newheader.samplingRate ; 
		 }
		 if(temp > statpeakamp) {
			statpeakamp = temp ; 
		 }

		if( in[ n ] > 1. ){
		    in[ n ] = 1. ; nsover++ ; nblockover++ ; 
		} 

		if( in[ n ] < -1. ){
		    in[ n ] = -1. ; nsover++ ; nblockover++ ;  
		} 

		out[ n ] =  (short int) ( in[ n ] * 32767.0 ) ; 

/* AMP STATS */
	t = (((float) i * (float) BLOCKSIZE) + (float) n) / (float) newheader.samplingRate ;
	

	if( (t >= nextt) && (quiet != 0) && ( flag == 0 ) ){
	    fprintf( stderr,  "\n(%6.2f -%6.2f)   %7.4f       %7.3f", 
		nextt-ampstatinc, nextt,  statpeakamp, (float) (20. * log10( (double) statpeakamp )) )     ; 
		nextt += ampstatinc ;		
		statpeakamp = 0 ;  

	    if(peakamp > lastpeakamp){
		fprintf( stderr,  "     %7.3f",
			 (float) (20. * log10( (double) peakamp ))    ) ; 
		lastpeakamp = peakamp ; 
	    }  

	}
/**/


	    }

/**/  
	    write( fd,  &out, sizeof(short int) * numsamps ) ;
	    i++ ; 


/* PRINT WARNING IF SAMPLES OUT OF RANGE */
	if( ( flag == 0 ) && (nsover > 0) ){
	    flag=1; 
	    fprintf( stderr, "\n*** FLIN: SAMPLES OUT OF RANGE ***** \n   (Samples out of range will be clipped.)" ) ; 
	    fprintf( stderr,  "\n*********************************************************************");
	    fprintf( stderr, "\n     TIME        PEAKAMP      DECIBELS      NUMBER_OF_SAMPLES" );	    
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

	    if((i%20) == 0 ){
		if(stat(filename,&st))  {
		    fprintf(stderr, "putlength:  Couldn't stat file\n");
		    exit(1);
	    }
	    newheader.dataSize = (int)st.st_size - data_offset ;

/* this captures the whole size of the file */

    	    lseek(fd, 0, SEEK_SET);
	    if((write( fd, &newheader, sizeof(newheader) ) == -1))
                        printf("couldn\'t write header\n");
 	    
/* RESET THE FILE POSITION TO CONTINUE WRITING */

    	    lseek(fd, (data_offset+(i * sizeof(short int) * numsamps)), SEEK_SET);	    

	    }

	    numsamps = fread( &in, sizeof(float), BLOCKSIZE, stdin ) ;

	}

/* PUT THE HEADER ON THE FILE AND CLOSE IT */

        /* Now,create new header  and slap it at the front of this soundfile*/

        newheader.magic = SND_MAGIC;
        data_offset = 1024 ;
        newheader.dataLocation = data_offset;
        newheader.dataFormat = SND_FORMAT_LINEAR_16 ;
        newheader.samplingRate = 44100 ;
        newheader.channelCount = 1 ;


        if(stat(filename,&st))  {
                fprintf(stderr, "putlength:  Couldn't stat file\n");
                exit(1);
        }
        newheader.dataSize = (int)st.st_size - data_offset ;
/*
fprintf( stderr,  "\n dataSize = %d",  (int)newheader.dataSize ) ; 
*/
	 /* this captures the whole size of the file */

    	lseek(fd, 0, SEEK_SET);
        if((write( fd, &newheader, sizeof(newheader) ) == -1))
                        printf("couldn\'t write header\n");
        close( fd ) ;

/* PRINT WARNING IF SAMPLES OUT OF RANGE */
	if(nsover > 0 ){ 
	    fprintf( stderr,  "\nTOTAL SAMPLES OUT OF RANGE:    %d",  nsover ) ; 
}
if( (i > 0) && (quiet != 0)){
	    fprintf( stderr,  "\n*********************************************************************");
	    fprintf( stderr, "\n\n======= FLIN: PEAK AMPLITUDE ========================================" ) ; 
	    fprintf( stderr, "\n     TIME          PEAKAMP      DECIBELS" );	    

	    fprintf( stderr,  "\n  %7.3f          %7.4f       %7.3f", 
		peakampt, peakamp, (float) (20. * log10( (double) peakamp )) )     ; 
	    fprintf( stderr,  "\n*********************************************************************");
}
	    fprintf( stderr,  "\n\n" ) ; 




exit(0) ; 


   if( (data_file = fopen( filename, "r" )) == NULL ) {
        fprintf( stderr, "\n\n%s IS NOT A FILE. BYE.\n", filename ) ;
        exit(0)  ;
   }
   else {
        if(getsfstats(filename, &sr, &chan, &dur, \
                &data_offset, &format) == 0 ) exit(-1) ;
        printf("%s has sr=%d, chan=%d, dur=%f, data_offset=%d, format=%d\n", \
                 filename, sr, chan, dur, data_offset, \
                 format);
    }
 
        fclose( data_file ) ;




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
    fprintf( stderr,  "\n\n*************************************\n*********     FLIN     **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}	
