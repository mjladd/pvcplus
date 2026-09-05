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
   


    char	*filename,
		*outfilename, 
		*ofile,
		ch, 
		crack()
    ;

    int		loc,
		pflag=0, 
		outputswitch=1, 
		i,
		j,
		k, 
		n, 
		doneflag=0, 
		numsamps,
		numbytes,  
		exflag, 
		sr,
		chan,
		data_offset, 
		format, 
		fd,
		end_sample, 
		begin_sample, 
		sample, 
		outchan=0, 
		new_blocksize=BLOCKSIZE, 
		pt
    ;

    FILE *ifd, *ofd ; 

    short int	idata[ BLOCKSIZE ],
	nit
    ; 
		
    float	outarray[ BLOCKSIZE * 4 ],
		peakamp=0.,
		peakdB,
		rms=0., 
		rmsdB,   
		c,
		avgamp=0.,
		avgdB,   
		dur, 
		out, 
		begin, 
		end,
		temp
    ;

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

	FILE *data_file ;

/*GIVE USAGE AND EXIT IF NO ARGUMENTS */
   if(argc == 1) {
	fprintf( stderr,"%s",
	"USAGE:\n\n   short_to_float [Sun/NeXT  shorts input soundfile] [Sun/NeXT 32-bit floating-point output file]\n\n" 
	 );
	exit(0);
    }
   
   begin = 0. ; end = -1. ; 

    if( argc < 3 ){
	    banner() ;
	fprintf( stderr, "\n\nSPECIFY AN OUTPUT FILE. BYE.\n\n") ;
        exit(0)  ;	
    }


/* GET SOUNDFILE NAME AND OPEN FOR BUSINESS */
   filename = argv[arg_index + 1] ;

        exflag = getsfstats(filename, &sr, &chan, &dur, \
                &data_offset, &format);
	
 /*
      fprintf(stderr,  "%s has sr=%d, chan=%d, dur=%f, data_offset=%d, format=%d\n", \
                 filename, sr, chan, dur, data_offset, \
                 format);
*/
if(stat(filename,&st))  {
                fprintf(stderr, "\n\nputlength:  Couldn't stat file\n");
                exit(1);
    }
    dur = (double) ((int)st.st_size - data_offset) / (double)(chan * sr * 2 )  ;



	if ( exflag == -2 ) {
	    banner() ;
	    fprintf( stderr, "\n\n\t....FIX THE FORMAT OF YOUR FILE. BYE.\n\n" ) ;
	    exit(0) ; 
	}
	if( exflag == -1 ) {
	    banner() ;
	    fprintf( stderr, "\n\n\t....FIX YOUR FILE (PATH)NAME. BYE.\n\n" ) ;
	    exit(0) ; 
	}


	begin_sample = begin * (float) sr ; 

		 
 
/* CHECK FOR STUPIDITY */
    if( format != 3 ){
	    banner() ;
        fprintf( stderr, "\n\nYOUR INPUT FILE IS NOT A Sun/NeXT 16-bit SHORT INTEGER FILE.\n\n" ) ;
        exit(0)  ;
    }

     end = dur ; 
    end_sample = (int) (end * (float) sr ) ; 

  //      fprintf(stderr,  "%s has sr=%d, chan=%d, dur=%f, data_offset=%d, format=%d\n\n", \
  //               filename, sr, chan, dur, data_offset, \
  //               format);

 
     data_file = fopen( filename,  "r" ) ; 

/* SKIP TO DATA OFFSET + BEGIN */
	
	fseek( data_file, data_offset + (2 * begin_sample * chan), SEEK_SET ) ;
	

// *********************   SET UP OUTPUT FILE

 ofile = argv[arg_index + 2] ; 
 
//fprintf( stderr, "\n OUTPUT FILE: %s\n", ofile ) ; 


/* OPEN FOR BUSINESS */
    
    fd = open(ofile,O_RDWR,(0644));
    if(fd < 0) {


	//fprintf( stderr,  "\n\n%s DOES NOT YET EXIST. I'LL MAKE IT.\n", ofile );
	exflag = open(ofile, O_CREAT,(0644)); 
	if( exflag == -1 ){
		
	    bannero() ;
		fprintf( stderr,  "\n\n I COULD NOT MAKE THE FILE:\n\n-----> %s <-------\n\n",  ofile ) ; 
		fprintf( stderr,  "\n\n CHECK THE PATH OF YOUR FILE. ....BYE.\n\n" ) ; 
		exit( 0 ) ; 
	}
	
	fd = open(ofile, O_RDWR,(0644));
    }else{
	open(ofile, O_TRUNC, (0644));
    }


/* WRITE 1024 dummy bytes for the header */

    outarray[ 0 ] = 0 ;
    for( i = 0; i < 512 ; i++)
	write( fd,  &outarray, sizeof(short int) ) ;

/* MAKE IT A SOUNDFILE BY WRITING THE HEADER */
/* Now,create new header  and slap it at the front of this soundfile*/

    newheader.magic = SND_MAGIC;
    data_offset = 1024 ;
    newheader.dataLocation = data_offset;
    format = SND_FORMAT_FLOAT ;
 
 if(stat(ofile,&st))  {
                fprintf(stderr, "\n\nputlength:  Couldn't stat file\n");
                exit(1);
    }
    newheader.dataSize = (int)st.st_size - data_offset ;

    // this captures the whole size of the file

    lseek( fd, 0, SEEK_SET);
    if((write( fd, &newheader, sizeof(newheader) ) == -1))
                        printf("(A) couldn\'t write header\n");

    close(fd) ; 


//exit( 0 ) ; 

//**************

	// OPEN OUTPUT SOUND FILE
	ofd = fopen(ofile,"w");

	// POSITION TO DATA BEGIN
	fseek(ofd, data_offset, SEEK_SET);


//******* NOW WRITE


// END NEW SETUP



 
/* READ IN THE VALUES */


	c = 0 ;  sample = begin_sample ; 

	new_blocksize = chan * ((int) (new_blocksize / chan)) ; 
	
	/* fprintf( stderr, "\n new_blocksize = %d",  new_blocksize ) ; */ 

	numsamps = fread( &idata, sizeof(short int), new_blocksize,  data_file ) ; 
	numsamps = numsamps / chan ; 
 	
	while( (doneflag != 1 ) && ( numsamps > 0 ) ) {

	    if( (end_sample - sample) < numsamps ){
		numsamps = (end_sample - sample) ;
		doneflag = 1 ;  
	    }

 	    
	    for( i = 0; i < numsamps; i++ ){
		k = i * chan ; 
		for( outchan = 0; outchan < chan; outchan++ ){
 
			outarray[ k + outchan ] = (float) (idata[ k + outchan ] / 32768.0 ) ; 
 	
		}

	    }
	    
	    fwrite( &outarray, sizeof(float), numsamps * chan, ofd ) ;
	    sample += numsamps ;


	    numsamps = fread( &idata, sizeof(short int), new_blocksize,  data_file ) ; 
	    numsamps = numsamps / chan ; 
	
	}

	fclose( data_file ) ; 

 


/* PUT THE HEADER ON THE OUTPUT FILE AND CLOSE IT */

        /* Now,create new header  and slap it at the front of this soundfile*/

        newheader.magic = SND_MAGIC;
        data_offset = 1024 ;
        newheader.dataLocation = data_offset;
        newheader.dataFormat = SND_FORMAT_FLOAT ;
        newheader.samplingRate = 44100 ;
        newheader.channelCount = chan ;


        if(stat(filename,&st))  {
                fprintf(stderr, "putlength:  Couldn't stat file\n");
                exit(1);
        }
        newheader.dataSize = (int)st.st_size - data_offset ;
/*
fprintf( stderr,  "\n dataSize = %d",  (int)newheader.dataSize ) ; 
*/
	 /* this captures the whole size of the file */

    	fseek(ofd, 0, SEEK_SET);
        if( (fwrite( &newheader, sizeof(newheader), 1,  ofd ) == -1))
                        printf("couldn\'t write header(3)\n");
        fclose( ofd ) ;




	exit( 0 ) ; 


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
		    fprintf(stderr, "\n\n%s: no such flag: %s\n", argv[0], pv);
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
int banner(){
    if( bannerflag == 0 )
    fprintf( stderr,  "\n\n*************************************\n*********     FLOUT     **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}	

int pd( i ) int i ; { fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }
