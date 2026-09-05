#include "globals.h"

void usage(); 
void pd( int i ) ;

int main(argc, argv)
    int argc; char *argv[];
{
    int 	R=44100,
		N=1024,
		N2,
		Nw = 4096,
		Nw2, 
		D = 1024, 
		I = 1024,
		i, j, k, l,  
		printflag=0, 
		in,
		exflag,
		specprintflag=0,  
		eof = 0;
    float 	peakamps[MAXIMUM_CHANNELS], 
		temp,
		fundamental, 
		DR,  
		P = 0.,
		len,
		tincr,
		tpos,
		*Hwin,
		*Wanal,
		*Wsyn,
		*input,
		*winput,
		*buffer,
		*channel,
		*output, 
		*SP
		;
    char	scratch[ 500 ], 
		ch,
		*dbuf;
int   obank = 0,  sflag = 0,  channelout=0 ;
int   print_flag=0 ; 
int Qflag ;
int qseccount=0,  seccount=0;
float nyquist,  frametprop,  tempt=0.,  dur ;
 
 
 // SHELF EQ
float  dBlow=0, dBhi=0,  freqlow=200, freqhi=2000  ; 

 
float warpshape=0. ; 

float normdB=0., dBgain=0.,  ampgain=1.,  peakamp=0., peakampnow=0., avgpeakamp=0.,  peakdB,  avgpeakdB   ; 


if( argc < 2 )usage() ; 


    while( (ch= crack( argc, argv, "R|w|N|M|D|q|Z|c|C|P|b|e|H|m|p|A|X|c|W|h", 0  )) != CRACK_DONE_FLAG ) {
	switch(ch) {
	    case 'N':   N = atoi(arg_option);
			break;
	    case 'M':   Nw = atoi(arg_option);
			break;
	    case 'w':   window_type = atoi(arg_option) ;
			break;
	    case 'D':   frames_per_sec = atof(arg_option);
			break;

	    case 'b':   begint = atof(arg_option) ;
			break;
	    case 'e':   endt = atof(arg_option) ;
			break;

	    case 'C':   channelout = atoi(arg_option) ;
			break;

	    case 'H':   dBlow = atof(arg_option) ;
			break;
	    case 'X':   dBhi = atof(arg_option) ;
			break;
	    case 'm':   freqlow = atof(arg_option) ;
			break;
	    case 'R':   freqhi = atof(arg_option) ;
			break;

	    case 'A':   dBgain = atof(arg_option);
			break;
	    case 'p':   printflag = atoi(arg_option) ;
			break;
	    case 'P':   specprintflag = atoi(arg_option) ;
			break;
			
	    case 'W':   warpshape = atof(arg_option) ;
			break;
	}
    }

prline( 69,  "/" ) ; 
prline( 69,  "-" ) ; 
prbanner( "PVANALYSIS", 69 ) ; 
prline( 69,  "-" ) ; 


    if(channelout == 0){
	channelflag = 0 ; 
	beginchan = 0 ;
    } else{
	channelflag = 1 ; 
	beginchan = channelout - 1 ; 
    }
 
    // SET NO OUTPUT FLAG
    outputoff=1;

// GET INPUT HEADER INFO
    setupfiles(argc, argv) ; 


    endchan = beginchan + ochan ; 

// GET NAME OF OUTPUT FILE
    arg_index++ ; 

    if( arg_index >= argc  ){
	bannero() ;
	sprintf( ofile, "pvanalysis.out" ) ; 
	fprintf( stderr, "\n\n......USING DEFAULT OUTPUT FILENAME........\n\n" ) ;
    }else{
/* GET OUTPUT SOUNDFILE NAME */
	strcpy( ofile, argv[arg_index] ) ; 
    }	 
    /* OPEN FOR BUSINESS */

    /* OPEN FOR BUSINESS */
    if( (ofd = fopen( ofile, "w+" )) == NULL ){
        printf( "\n\n****** CANNOT OPEN FILE NAMED: %s\n\n", ofile ) ; exit(EXIT_FAILURE) ; 
    } ; 
    
// **** SET UPS *****
    R = isr ; // SAMPLE RATE EQUALS INPUT FILE
    if( frames_per_sec < 32.){
	fprintf( stderr, "\n\n----> YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND. <-----" ) ; 
	fprintf( stderr, "\n.............RESETING TO DEFAULT OF 200.\n\n" ) ; 
	frames_per_sec = 200 ; 
    }
    D = (int) ((float) R / frames_per_sec) ; 
//*****
//******  WINDOW SETUP/ADJUSTMENT ***************************
// MAKE WINDOW SIZE TWICE FFT IF SET TO 0
    if( Nw <= 0 ) Nw = 2 * N ;
//*********************************


    PI = 4.*atan(1.) ;
    TWOPI = 8.*atan(1.) ;
    nyquist = R/2.0;
    fundamental =  ((float) R / (float) N) ; 
    obank = P != 0. ;
    if( P == 0.0 ) {P = 1.0;}
    N2 = N>>1 ;
    Nw2 = Nw>>1 ;
    DR = (float) D / (float) R ; 
    // COMPUTE THE DURATION
    dur = (endt - begint) ; 

    frametprop = (float) D / (float) R ; 
    ampgain = dB_to_amp( dBgain ) ; 
    
//***************** PRINT VALUES
prbanner( "OUTPUT ANALYSIS FILE",  69 ) ; 
prs( ofile, "OUTPUT FILE NAME" ) ; 
prf( dur, "OUTPUT FILE: DURATION" ) ; 
pri( ochan, "OUTPUT FILE NUMBER OF CHANNELS" ) ; 
pri( R, "OUTPUT FILE SAMPLE RATE" ) ; 
prline( 69,  "." ) ; 
prbanner( "ANALYSIS PARAMETERS",  69 ) ; 
pri( N,  "FFT SIZE" ) ; 
prline( 1,  "*" ) ; 
prf( fundamental, "      FUNDAMENTAL ANALYSIS FREQUENCY" ) ; 
prline( 1,  "*" ) ; 
pri( Nw,  "WINDOW SIZE" ) ; 
pri( R,  "SAMPLE RATE" ) ; 

pri( frames_per_sec,  "FRAMES/SECOND" ) ; 
prline( 1,  "*" ) ; 
pri( D,  "      DECIMATION SAMPLES (samples between analysis frames)" ) ; 
prline( 1,  "*" ) ; 
prt( "*............. LOW/HIGH SHELF EQ............*" ) ;  
prf( freqlow, "LOW SHELF FREQUENCY" ) ; 
prf( dBlow, ".......... LOW SHELF DECIBELS" ) ; 
prf( freqhi, "HIGH SHELF FREQUENCY" ) ; 
prf( dBhi, ".......... HIGH SHELF DECIBELS" ) ; 
prline( 1,  "*" ) ; 
prf( warpshape, "SPECTRUM WARPSHAPE INDEX" ) ; 

// *******

    if (Nw == 0)
	Nw = N;

    if (I == 0)
	I = D;


// *******

    fvec( Wanal, Nw ) ;		/* analysis window */
    fvec( Wsyn, Nw ) ;		/* synthesis window */
    fvec( input, Nw ) ;		/* input buffer */
    fvec( Hwin, Nw ) ;		/* plain Hamming window */
    fvec( winput, Nw ) ;	/* windowed input buffer */
    fvec( buffer, N ) ;		/* FFT buffer */
    fvec( channel, N+2 ) ;	/* analysis channels */
    fvec( output, Nw ) ;	/* output buffer */
    
    if( specprintflag )fvec( SP, N+2 ) ;	/* average spectrum array */


// OPEN INPUT  AND OUTPUT FILES
    openfiles() ; 

//*********************************************
// WRITE OUT FFT SIZE, DECIMATION, RATE, AND NUMBER OF CHANNELS AS A HEADER
    temp = (float) N; 
    fwrite(&temp, sizeof(float), 1, ofd ) ;    
    temp = (float) D; 
    fwrite(&temp, sizeof(float), 1, ofd ) ;    
    temp = (float) R; 
    fwrite(&temp, sizeof(float), 1, ofd ) ;    
    temp = (float) ochan ; 
    fwrite(&temp, sizeof(float), 1, ofd ) ;    
    temp = (float) window_type ; 
    fwrite(&temp, sizeof(float), 1, ofd ) ;    


    // ADD 27 MORE SPACES FOR STUFF (32 TOTAL)
    temp = 0. ; 
    fwrite( &temp, sizeof(float), (FFT_HEADER_SIZE - 5), ofd ) ;    


//*********************************************

//*********************************************
// LOOP FOR CHANNELS
//*********************************************

for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

prline( 69,   "=" ) ; 
pri( (outchan+1), "ANALYSIS: CHANNEL" ) ; 

    //*****   REINITS
    frame_count = 0 ;    eof = 0 ;  t = 0 ; samps = 0 ; 
    tempt = 0. ; qseccount=0 ;  seccount=0 ; 
    peakamps[channow] = 0 ; 
    
    fseek(ofd,  sizeof(float) * FFT_HEADER_SIZE, SEEK_SET );



    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank );


    in = -Nw;
    
pri( ochan, "NUMBER OF OUTPUT CHANNELS" ) ; 
	
//*********************************************
// LOOP FOR FRAMES
//*********************************************

    while ( !eof ) {
	in += D;
	timenow( dur ) ;

	    eof = shiftin( input, Nw, D );
	    fold( input, Wanal, Nw, buffer, N, in );
	    rfft( buffer, N2, FORWARD );
	    convert( buffer, channel, N2, D, R );

//*************EQ
    eq( channel,  (N + 2),  dBlow,  dBhi, freqlow,  freqhi,  fundamental, 1, 0, 0 ) ; 
//*************WARP
    spectmagwarp( channel,  (N + 2), warpshape, 0 ) ;

// ADJUST BY dBgain AND FIND PEAKAMP
	peakampnow=0; 
	for( i = 1; i < N; i+= 2){
	    channel[i - 1] = channel[i - 1] * ampgain ;
		// FIND PEAK CHANNEL AMP FOR THIS FRAME
	    if( channel[i - 1] > peakampnow) peakampnow = channel[i - 1] ; 
	}
	avgpeakamp += peakampnow ;
	if( peakampnow > peakamps[channow] )peakamps[channow]  =  peakampnow ; 
	
	if( peakampnow > peakamp) peakamp = peakampnow ; 

//  *****  AVERAGE SPECTRUM ACCUMULATOR
	if( specprintflag )for( i = 1; i < N; i+= 2){
	    SP[ i - 1 ] += channel[ i - 1 ] ; 
	    SP[ i ] += channel[ i ] ; 
	}

    // *** WRITE FFT TO OUTPUT FILE
    for( k = 0; k < ochan; k++){
	if( k == channow ){
	    // ** OUTPUT THE FFT
	    fwrite(channel, sizeof(float), (N + 2), ofd ) ;
	}else{
	    // SKIP BY A BUFFER (OR WRITE A BLANK BUFFER IF FIRST PASS)
	    if( channow == 0 ){
		// ** FIRST PASS  -- WRITE IT
		fwrite( channel, sizeof(float), (N+2), ofd ) ;
	    }else{
		// ** NOT FIRST PASS -- SKIP
		fseek(ofd,  sizeof(float) * (N+2), SEEK_CUR);
	    }
	}
    }

// *** PASSIFIER PRINT
    if(printflag != 0){
	if(!frame_count)fprintf( stderr,  "\n\nELAPSED TIME (in secs): 0 " ) ; 

	tempt = tempt + frametprop ; 
	if(tempt > .5){
	    // PRINT PASSIFIER
	    qseccount++ ; 
	    while( tempt > .5 ) tempt -= .5 ; 
	    if( qseccount == 2){
		// SECOND
		seccount++ ;  qseccount = 0 ; 
		fprintf( stderr, " %d ",  seccount ) ; 
	    }else{
		// QUARTER SECOND
		fprintf( stderr,  "*" ) ; 
	    }
	    
	}
    }
// *** 
	frame_count++ ; 

// FRAMES LOOP END
    }

    
// CHANNELS LOOP END
} 
// WRITE PEAKAMPS TO HEADER
    fseek(ofd,  sizeof(float) * 5, SEEK_SET );
    for(i = 0 ; i < ochan ; i++){
	 fwrite( &peakamps[i], sizeof(float), 1, ofd ) ;    
    } ; 

// WRITE SOUND FILE NAME TO HEADER
    fseek(ofd,  sizeof(float) * (5 + ochan), SEEK_SET );

    strcpy( scratch, ifile ) ; strcat( scratch, "\n" ) ; 
    fputs( scratch, ofd ) ; 
	
	// AVERAGE SPECTRUM AVERAGER
		temp = (float) ( ochan * frame_count) ; 
	if( specprintflag ){
	    for( i = 1; i < N; i+= 2){
		SP[ i - 1 ] = SP[ i - 1 ] / temp ; 
		SP[ i ] = SP[ i ] / temp ; 
	    }
	//*** PRINT TO TERMINAL ******
	        if( specprintflag )tprintspec( SP, (N + 2), fundamental,  specprintflag) ;

	}


    // CLOSE  INPUT  AND OUTPUT FILES
    if(ifd)fclose(ifd);  
//    close(ofd) ; 
    if(ofd)fclose(ofd); 




    fprintf(stderr,"\nPVANALYSIS: WROTE %d FRAMES\n",frame_count);

    peakdB =  (float) (20. * log10( (double)peakamp));
    avgpeakdB =  (float) (20. * log10( (double)(avgpeakamp/ (float) frame_count)));

    fprintf(stderr,"\nPEAK BIN LEVEL WAS %f dB\n",peakdB );    
    fprintf(stderr,"\nAVERAGE PEAK BIN LEVEL WAS %f dB\n",avgpeakdB );    

    sprintf( scratch, "readheader %s", ofile ) ; 
    system( scratch ) ; 

    exit(EXIT_SUCCESS);
}

void pd( int i ){ fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

void usage()
{
    fprintf(stderr, "%s",
	"pvanalysis:  phase vocoder analysis\n"
	"pvanalysis [flags] [input file     ] [output analysis file (optional)]\n"
	"                    (default: pvanalysis.out)\n"
	"	    (values in brackets denote defaults)\n"
	"	N:	FFT length (must be a power of 2) [1024]\n"
	"	M:	window size in samples (must be a power of 2) [2*FFT]\n"
	"		    (0 will automatically set window to 2*FFT size or larger)\n"
	"	w:	window type: 0 = hamming,  1 = rectangular  \n"
	"		    2 = Blackman,  3 = Bartlett triangular [0.]\n"
	"	D:	analysis frames per second [200]\n"

	"	b:	begin time in seconds  [0.] \n"
	"	e:	end time in seconds (0. = end of file) [0.] \n"
	"	C:	resynthesis channel (1 -> ?) (0 = all) [0] \n"

	"	A:	gain in decibels [0.] \n"
	"	p:	print elapsed time (1 = on,  0 = off) [0]\n"
	"	P:	print average spectrum to stderr (1 = on,  0 = off) [0]\n"
	"		    ( values > 1 become hi cutoff frequency for printing\n"
	"	W:	warp index for reshaping frequency response (func) [0.] \n"
	"		    values > 0 expand the dynamic range, \n"
	"		    values < 0 compress the dynamic range \n"

	"	H:	SHELF EQ: Low shelf gain in dB [0.] \n"
	"	X:	SHELF EQ: High shelf gain in dB [0.] \n"
	"	m:	SHELF EQ: Low shelf frequency in Hz [200.] \n"
	"	R:	SHELF EQ: High shelf frequency in Hz [2000.] \n"


	);
    exit(EXIT_SUCCESS);

/*

*/

}

