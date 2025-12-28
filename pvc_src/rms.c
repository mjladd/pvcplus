#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

/*

#include <sys/fpu.h>

*/


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
   


    char	*filename, 
		ch, 
		crack()
    ;

    int		pflag=0, 
		outputswitch=1, 
		i, 
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
		peak_samp_time[16]
    ;
    int channow,  beginchan,  endchan,  exitflag ; 	    
	
    short int	idata[ BLOCKSIZE * 4 ] 
    ; 
    float	fdata[ BLOCKSIZE * 4 ] 
    ; 
		
    float	sampnow,
		peakamp[16],
		peakdB[16],
		rms[16], 
		rmsdB[16],
		DC[16],    
		c,
		avgamp[16],
		avgdB[16],   
		dur, 
		out, 
		begin, 
		end,
		endsave=-1., 
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

/*

//#include "underflow.h"

*/

/*GIVE USAGE AND EXIT IF NO ARGUMENTS */
   if(argc == 1) {
	fprintf( stderr,"%s",
	"USAGE:\n\n   rms [ flags ] [ NeXT/Sun format input soundfiles (integer or floats)]\n" 
	"\tflags:\n"
	"	(values in brackets denote defaults)\n"
	"   :c	    channel: 1 to ?,  0 = all [0]\n"
	"   :b	    start time in seconds [0.]\n"
	"   :e	    end time [end of file]\n\n"
	"   For each of the NeXT/Sun format integer or float files,\n"
	"   this routine reads the specified channel(s) for the values \n"
	"   between begin and end, printing peak amplitude, average  \n"
	"    peak amplitude, RMS amplitude and average DC amplitude.\n\n" 
	 );
	exit(0);
    }
   
begin = 0. ; outchan = 0 ; 

    while((ch = crack(argc,argv,"c|b|e|po",0)) != 0)
{
        switch(ch) {
            case 'c': outchan = atoi(arg_option) ;                  break;
            case 'b': begin = atof(arg_option) ;                  break;
            case 'e': endsave = atof(arg_option) ;                  break;

        }
    }

    if( arg_index >= argc ){
	    //banner() ;
	fprintf( stderr, "\n\nWHERE IS THE FILE? BYE.\n\n") ;
        exit(0)  ;	
    }


while( arg_index < argc ){
   end = endsave ; exitflag = 0 ;  doneflag = 0 ; 


/* GET SOUNDFILE NAME AND OPEN FOR BUSINESS */
   filename = argv[arg_index] ;

        exflag = getsfstats(filename, &sr, &chan, &dur, \
                &data_offset, &format);
	
	fprintf( stderr, "\n------------------------------------------------------------" ) ; 
	fprintf( stderr, "\nFILENAME:\t%s",  filename ) ; 
	fprintf( stderr, "\n%d CHANNELS",  chan ) ; 
	fprintf( stderr, "\n============================================================" ) ; 

	if ( exflag == -2 ) {
	    //banner() ;
	    fprintf( stderr, "\n\n\t....FIX THE FORMAT OF YOUR FILE. BYE.\n\n" ) ;
	    exitflag = 1 ;  
	}
	if( exitflag == 0 ) if( exflag == -1 ) {
	    //banner() ;
	    fprintf( stderr, "\n\n\t....FIX YOUR FILE (PATH)NAME. BYE.\n\n" ) ;
	    exitflag = 1 ;  
	}


	begin_sample = begin * (float) sr ; 

		 

	if( exitflag == 0 ) if( exflag  == -1 ) 

/* CHECK FOR STUPIDITY */
	    if( (format != 3) || (format != 6)  ){
		//banner() ;
		fprintf( stderr, "\n\nYOUR FILE IS NOT A FLOAT OR INTEGER FORMAT NEXT FILE.\n\n" ) ;
		exitflag = 1 ;  
	    }

    if( exitflag == 0 ) if( begin < 0. ){
	fprintf( stderr,
	     "\n\nBEGIN TIME (%f) is < 0. (GET REAL!)\nWILL RESET TO 0.\n\n",  begin  ) ;
	     begin = 0. ; 
    }

	
    if( exitflag == 0 ) if( (end != -1.) && (end <= begin)){


	    //banner() ;
	fprintf( stderr,
	     "\n\nBEGIN TIME = %f\nEND TIME = %f\n\n",  begin,  end  ) ;

	fprintf( stderr,
	     "\n\nYOUR END TIME IS BEFORE OR EQUAL TO YOUR BEGIN TIME! BYE.\n\n" ) ;
	exit(0) ; 
    }
	
    if( exitflag == 0 ) if( end > dur ){
	    //banner() ;
	    fprintf( stderr, "%s%f%s%f%s", "\nYOUR END TIME OF  ", 
		end, 
		"  SECONDS \nEXCEEDS THE FILE DURATION OF  ", 
		dur, 
		" SECONDS.\nWILL RESET TO THE DURATION OF THE FILE.\n\n" 
		 ) ;
	    end = dur ; 	   	

    }

/* ZERO STUFF */
for( i = 0; i < 16; i++){
    peakamp[i] = 0.; avgamp[i] = 0.; rms[i] = 0 ; DC[i] = 0. ; 
}
  
/* NO END TIME SPECIFIED, SET TO dur */
    if( end <= 0. ) end = dur ; 
    end_sample = (int) (end * (float) sr ) ; 
 
 
 
    if( exitflag == 0 ) if( outchan >= chan ){
	//banner(); 
        fprintf( stderr, "\n\nTHE AVAILABLE CHANNELS ARE 0-%d, NOT %d. BYE.\n\n",\
	     (chan-1), outchan ) ;
	    exitflag = 1 ;  
   }

    /* SETUP CHANNEL STUFF */
    if( outchan == 0 ){
	beginchan = 0; endchan = chan - 1 ; 
    }else{
	beginchan = outchan - 1; endchan = beginchan ; 
    }


if( exitflag == 0 ){ 


    data_file = fopen( filename,  "r" ) ; 

/* SKIP TO DATA OFFSET + BEGIN */
	
	fseek( data_file, data_offset + (2 * begin_sample * chan), SEEK_SET ) ;
	
/* READ IN THE VALUES */


	c = 0 ;  sample = begin_sample ; 

	if( format == SND_FORMAT_LINEAR_16 )
	    numsamps = fread( &idata, sizeof(short int), BLOCKSIZE,  data_file ) ; 
	else
	    numsamps = fread( &fdata, sizeof(float), BLOCKSIZE,  data_file ) ; 
		
	numsamps = numsamps / chan ; 
	
	while( (doneflag != 1 ) && ( numsamps > 0 ) ) {

	    if( (end_sample - sample) < numsamps ){
		numsamps = (end_sample - sample) ;
		doneflag = 1 ;  
	    }

	    for( i = 0; i < numsamps; i++ ){


		for( channow = beginchan; channow <= endchan; channow++){

		    if( format == SND_FORMAT_LINEAR_16 )
			sampnow = ((float) idata[ (i * chan) + channow ]) / 32768.0 ; 
		    else
			sampnow = fdata[ (i * chan) + channow ] ; 
		    
		    
		    
		    
		    // PEAK AMP
		    temp = fabs( sampnow ) ; 
		    if( temp > peakamp[ channow ] ){
			peakamp[ channow ] = temp ; peak_samp_time[ channow ]  = c ; 
		    } 

		    // AVERAGE AMP
		    avgamp[ channow ] += temp ;
		
		    // RMS
		    rms[ channow ] = rms[ channow ] + (sampnow * sampnow) ; 

		    // DC AVERAGE
		    DC[ channow ] += sampnow ; 
		    

		}

		c += 1. ; 
	    }

	    sample += numsamps ; 

	if( format == SND_FORMAT_LINEAR_16 )
	    numsamps = fread( &idata, sizeof(short int), BLOCKSIZE,  data_file ) ; 
	else
	    numsamps = fread( &fdata, sizeof(float), BLOCKSIZE,  data_file ) ; 



	numsamps = numsamps / chan ; 
	
	}

	fclose( data_file ) ; 

	fprintf( stderr, "\nFILE DURATION:\t\t%f",  dur ) ; 
	fprintf( stderr, "\nSEGMENT DURATION:\t%f (%f to %f)",  end - begin,  begin,  end ) ; 

	for( channow = beginchan; channow <= endchan; channow++){


	fprintf( stderr, "\n------------------------ CHANNEL %d -------------------------",   channow + 1 ) ; 

	    DC[ channow ] =  DC[ channow ] / c ; 
	    temp =  (float) (20. * log10( (double) fabs( DC[ channow ] )) ) ; 
	    fprintf( stderr, "\nAVG. DC AMP:\t%8.5f\t   (%6.2f dB)", DC[ channow ],  temp ) ;

	fprintf( stderr, "\n                                                TIME" ) ; 

	    temp = (float) (20. * log10( (double) peakamp[ channow ] )) ; 
	    fprintf( stderr, "\nPEAKAMP:\t%8.5f\t   (%6.2f dB)\t%f sec",
		 peakamp[ channow ],  temp,  begin + ((float) peak_samp_time[ channow ] / (float) sr ) ) ;

	    avgamp[ channow ] = avgamp[ channow ] /  c ;
	    temp = (float) (20. * log10( (double) avgamp[ channow ] )) ; 
    	    fprintf( stderr, "\nAVG. PEAK AMP:\t%8.5f\t   (%6.2f dB)",  avgamp[ channow ],  temp ) ;


	    rms[ channow ] = (float) pow( (double) (rms[ channow ] / c),  .5 ) ; 
	    temp = (float) (20. * log10( (double) rms[ channow ] )) ; 
    	    fprintf( stderr, "\nRMS AMP:\t%8.5f\t   (%6.2f dB)",  rms[ channow ],  temp ) ;
	}

	fprintf( stderr, "\n============================================================\n" ) ; 




/*

*/

}
    arg_index++ ; 
}
    exit( 1 ) ; 
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
			//banner(); 
			fprintf(stderr,  "\n\n********\nYOUR SOUNDFILE:\n\n------>  %s <------\n********",filename);
			fprintf(stderr,  "\nIS NOT A NEXT (.snd) FORMAT FILE.\n" ) ;
			*chan=0; *sr=0;
			return(-2);
		}else if(k_err == -1) {
			//banner(); 
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
    fprintf( stderr,  "\n\n*************************************\n*********     RMS     **********\n*************************************\n" ) ;
			 bannerflag = 1 ; 
}	
