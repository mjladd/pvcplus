#include <stdio.h>
#include <math.h>
#include "pv.h"

int make_freqresponse(
    
    float F[],   
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
    int method,
    
    float A_begint, 
    float A_endt 
    
    ){


     


    float nn ; 
    int eof;
    
    int i,  in,  on ;      
    
     

//    pri( method,  "METHOD" ) ; 

    if( method == 0 ){ // AVERAGE
	for( i = 0; i < N; i += 2 ) F[i] = 0. ; 
//	prt( "USING AVERAGE METHOD" ) ; 
    }else{  // PEAK 
	for( i = 0; i < N; i += 2 ) F[i] = -99999999. ; 
//	prt( "USING PEAK METHOD" ) ; 
    }

    for( i = 1; i < N; i += 2 ) F[i] = 0. ; 
	
    nn = 0. ; 
    eof = 0 ; 


//    prt( "BEFORE" ) ;  
//    for( i = 0; i < N; i += 2 ){
//	fprintf( stderr, "\n %f,  %f",  F[ i ],  F[ i + 1 ] ) ; 
//    }
    

    
    // POSITION TO BEGINING OF SEGMENT
 
    // SET TO PROPER POSITION
    end_sample = (int) ((A_endt - endt) * (float) isr ) ; 
    sample = begin_sample =  (int) ((A_begint - endt) * (float) isr ) ;

 // SKIP TO DATA OFFSET + BEGIN
/*
    if( iformat == 3) fseek( ifd, idata_offset + (2 * begin_sample * ichan), SEEK_SET ) ;
    else if( iformat == 6 ) fseek( ifd, idata_offset + (4 * begin_sample * ichan), SEEK_SET ) ;
    else{
	fprintf( stderr,  "\n\nUNKNOWN FORMAT. (openfiles)\n\n" ) ; exit( -1) ; 
    }
*/
    fseek( inputTempChanFiles[ outchan ], begin_sample * sizeof( float ), SEEK_SET ) ; 


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

	if( method == 0 ){

		// FIND AVERAGE SPECTRUM
	    for( i = 0 ; i < N; i += 2 ){
		 F[i + 1] += channel[i + 1] ; // SUM FREQUENCIES
		 F[i] += channel[i] ; // SUM AMPLITUDES
	    }
	    
	}else{

	    // FIND PEAK SPECTRUM
	    for( i = 0 ; i < N; i += 2 ){
		F[i + 1] += channel[i + 1] ;  // SUM FREQUENCIES
		if( channel[i] > F[i] ) F[i] = channel[i] ;  // FIND CURRENT PEAK
	    }
	}	

	nn++ ; 

    }

 

    // AVERAGE
    if( method == 0 ){ // TAKE AVERAGE OF AMPS
	for( i = 0 ; i < N; i += 2 ) F[i] = F[i] / nn ;  
    }
    // TAKE AVERAGE OF FREQ FOR BOTH
    for( i = 1 ; i < N; i += 2 ) F[i] = F[i] / nn ;  

 
    
//prf( begint,	"INPUT FILE: BEGIN TIME" ) ; 
//prf( endt,	"INPUT FILE: END TIME"	 ) ; 
    // SET TO PROPER POSITION
    end_sample = (int) (endt * (float) isr ) ; 
    sample = begin_sample =  (int) (begint * (float) isr ) ;
//pri( begin_sample,	"begin_sample"	 ) ; 
//pri( ichan,	"ichan"	 ) ; 
//pri( channow,	"channow"	 ) ; 

    IO_reset = 1 ; 
    
 // SKIP TO DATA OFFSET + BEGIN
/*
    if( iformat == 3) fseek( ifd, idata_offset + (2 * begin_sample * ichan), SEEK_SET ) ;
    else if( iformat == 6 ) fseek( ifd, idata_offset + (4 * begin_sample * ichan), SEEK_SET ) ;
    else{
	fprintf( stderr,  "\n\nUNKNOWN FORMAT. (openfiles)\n\n" ) ; exit( -1) ; 
    }
*/
   fseek( inputTempChanFiles[ outchan ], 0, SEEK_SET ) ; 





 return( 1 ) ;    
    
}
