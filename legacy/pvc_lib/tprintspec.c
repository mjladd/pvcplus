#include "pv.h"
#include <math.h>

#define DECIBELS_GRAPH_WIDTH_IN_CHARACTERS 32.0
#define LINE_WIDTH 69
/*
 * PRINT THE INPUT ARRAY OF FREQUENCIES AND MAGNITUDES
 * WITH A PLOT ON THE SIDE -- OLD WORLD STYLE
 */
int tprintspec( float *F, int Nplus2, float fundamental, int freqcutoff ){

    int i, j, k, jj,  flag,  zeroflag,  itemp ; 
    float dBnow,  basedB,  peakdB,  dBrange,  dBdiv ; 
 
    float A[10], AdB[10], freq ; 
    int lowbin,  highbin,  n; 

    // OCTAVE AVERAGE PRINT
    freq = 31.25 ; n = 0 ; 

    for( i = 0; i < 10; i++) {
	A[ i ] = 0. ;
	lowbin =  (int) (freq / fundamental) ; 
	highbin = 1 + (int) (2. * freq / fundamental) ; 
	if( highbin > (Nplus2 / 2) ) highbin = Nplus2 / 2 ; 
	n = 0 ; 
	for( j = lowbin; j < highbin; j++){
	    A[ i ] += F[ j * 2 ] ; n++ ; 
	}
	if( n != 0 ) A[ i ] = A[ i ] / (float) n ; 
	if( A[ i ] > 0. ) AdB[ i ] = 20. * log10( A[ i ] ) ; 
	freq *= 2. ; 
    }
    


	zeroflag = 0 ; 
	basedB = 99999999999. ; flag = 0 ; peakdB = -99999999999. ; 



	for( i = 1; i < Nplus2; i += 2){ 
	    if( F[i - 1] > 0.){
		dBnow = (20. * log10( (double) F[ i - 1 ] )) ; 
		if( dBnow < basedB ) {basedB = dBnow ; flag = 1;}
		if( dBnow > peakdB ) {
		     peakdB = dBnow ;
		}
		
	    }
	}
	if( flag == 0 )prt( "\nALL ZERO AMPLITUDES!\n" ) ; 
	
	dBrange = peakdB - basedB ; 
	dBdiv = dBrange / DECIBELS_GRAPH_WIDTH_IN_CHARACTERS ; 
	if(dBdiv < 1.)dBdiv = 1. ; 

    prbanner(  "FFT DATA (FREQUENCY/AMPLITUDES)",  LINE_WIDTH ) ; 
 
    prline( LINE_WIDTH,  "=" ) ;
    prt( "| BIN |BINFREQ|FREQ   |AMP     |DB  | DECIBELS GRAPH                                                |" ) ; 
    prline( LINE_WIDTH,  "=" ) ;




	basedB *= -1. ; 
	    for( i = (Nplus2-1),  j = ((Nplus2/2)-1); i >= 0 ; i -= 2,  j--){ 
		itemp = (int) ((float) j) * fundamental ; 
	      if( 
		    ((F[ i - 1 ] > 0.) || (zeroflag)) && 
		    ((itemp < (float) freqcutoff) || (freqcutoff == 1)) ){
		zeroflag = 1 ; 
		fprintf( stderr,  "\n%5d| %-7.1f|%-7.1f|%-8.6f|",
			j, ((float) j) * fundamental,   F[ i ],  F[ i - 1 ] ) ;

	       if( flag ){
		 if( F[i - 1] > 0.){
		     dBnow = (20. * log10( (double) F[ i - 1 ] )) ; 
		     if( dBnow != 0.)fprintf( stderr,  "%-4.f|",  dBnow ) ; 
		     else fprintf( stderr,  "-0  |" ) ;

		    // GRAPH
		    k = (int) (.5 + ((basedB + dBnow) / dBdiv)) ;  
		    if( dBnow >= (peakdB - 0.5) ){
			fprintf( stderr, "-> PEAK <" ) ;
			k -= 9 ; 
			for(jj = 0; jj < k; jj++)fprintf( stderr, "-" ) ;
			    fprintf( stderr, "+" ) ;
		    }else{
			for(jj = 0; jj < k; jj++)fprintf( stderr, "-" ) ;
			    fprintf( stderr, "+" ) ;
		    }
		}else{
		    // O AMP
		    fprintf( stderr,  "    |o" ) ; 		    
		}

	      }
		
	    }

	    }
    prline( LINE_WIDTH,  "=" ) ;
    prt( "| BIN |BINFREQ|FREQ   |AMP     |DB  | DECIBELS GRAPH                |" ) ; 
    prline( LINE_WIDTH,  "=" ) ;

    prline( LINE_WIDTH,  "=" ) ;
    fprintf( stderr, "\nAVERAGE RESPONSE BY OCTAVE" ) ; 
    fprintf( stderr, "\n\ndB " ) ; 
    for( k = 0 ; k < 10; k++ ){ 
	fprintf( stderr, "| " ) ;   
	if( A[ k ] > 0. )
	    fprintf( stderr, "%-4.f ",   AdB[ k ] ) ;  
	else
	    fprintf( stderr, "  -o- " ) ; 

    }
    fprintf( stderr, "| " ) ;   
    prline( LINE_WIDTH,  "-" ) ;

    fprintf( stderr, "\nHz:31     62    125    250    500   1000   2000   4000   8000   16000  22050" ) ; 
    
    prline( LINE_WIDTH,  "=" ) ;

    return(1); 

}
