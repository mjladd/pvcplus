#include <stdio.h>
#include <math.h>
#include "pv.h"

float find_peak_amp(
    
    float Hwin[],   
    float Wanal[],   
    float Wsyn[],   
    int N,   
    int Nw,  
    int I,
    int D,
    int N2, 
    int R,    
    int obank,   

    float buffer[],   
    float channel[],   
    float input[],   

    int beginchan,  
    int endchan
    
    ){




    float peaksumamp ; 
    float sumamp ; 
    int eof;
    
    int i,  j,  in,  on ;      
    
    peaksumamp = -9999. ; 

//*********************************************
// LOOP FOR CHANNELS
//*********************************************



for(outchan = beginchan,  channow = 0; outchan < endchan; outchan++,  channow++ ){

    //*****   REINITS
    eof = 0 ; 


    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank ) ;
    in = -Nw ;
    if ( D )
	on = (in*I)/D ;
    else
	on = in ;

	
//*********************************************
// LOOP FOR FRAMES
//*********************************************

    while ( !eof ) {
	in += D ;
	on += I ;

	eof = shiftin( input, Nw, D ) ;
	fold( input, Wanal, Nw, buffer, N, in ) ;
	rfft( buffer, N2, FORWARD ) ;
	convert( buffer, channel, N2, D, R ) ;

	// FIND SUM OF AMPS
	sumamp = 0. ; 
	for( i = 0 ; i < N; i += 2 ){
	    sumamp += channel[ i ] ; 
	}
	if( sumamp > peaksumamp ) peaksumamp = sumamp ;
	



    }
    
    
}
     // SKIP TO DATA OFFSET + BEGIN
    if( iformat == 3) fseek( ifd, idata_offset + (2 * begin_sample * ichan), SEEK_SET ) ;
    else if( iformat == 6 ) fseek( ifd, idata_offset + (4 * begin_sample * ichan), SEEK_SET ) ;
    else{
	fprintf( stderr,  "\n\nUNKNOWN FORMAT. (openfiles)\n\n" ) ; exit( -1) ; 
    }

 return( peaksumamp ) ;    
    
}
