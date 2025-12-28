#include "pv.h"
#include <math.h>

/*
 * PRINT THE INPUT ARRAY OF FREQUENCIES AND MAGNITUDES
 * WITH A PLOT ON THE SIDE -- OLD WORLD STYLE
 */
int tprintspec_groupdelay( float *F, int N, float fundamental, int freqcutoff ){

    int i, j, k, ii, jj,  flag,  zeroflag,  itemp ; 
    float temp,  temp2, temp3,  delayrange,  delaydiv ; 
 
	
	zeroflag = 0 ; 
	temp2 = 99999999999. ; flag = 0 ; temp3 = -99999999999. ; 
	for( i = 1; i < N; i += 2){ 
	    if( F[i] >= 0.){
		temp = F[ i ] ; 
		if( temp < temp2 ) {temp2 = temp ; flag = 1;}
		if( temp > temp3 ) temp3 = temp ;
	    }
	}
	if( !flag )prt( "\nALL ZERO DELAYS!\n" ) ; 
	prf( temp2, "SHORTEST DELAY" ) ; 
	prf( temp3, "LONGEST DELAY" ) ; 
	
	delayrange = temp3 - 0. ; 
	delaydiv = delayrange / 32. ; 
	//if(delaydiv < 1.)delaydiv = 1. ; 

    prbanner(  "DATA (TIME DELAY/AMPLITUDES)",  69 ) ; 
 
    prline( 69,  "=" ) ;
    fprintf( stderr,"\n| BIN |BINFREQ|DELAY  |AMP     |DB  | TIME DELAY GRAPH (0 - %-5.3f)", temp3  ) ; 
    prline( 69,  "=" ) ;

 //   for(i = 1; i < N; i += 2 ) prf( F[ i ],  "time delay" ) ; 

	temp2 *= -1. ; 
	    for( i = (N-1),  j = ((N/2)-1); i >= 0 ; i -= 2,  j--){ 
		itemp = (int) ((float) j) * fundamental ; 
	      if( 
		    ((itemp < (float) freqcutoff) || (freqcutoff == 1)) ){

		fprintf( stderr,  "\n%5d| %-7.1f|%-7.1f|%-8.6f|",
			j, ((float) j) * fundamental,   F[ i ],  F[ i - 1 ] ) ;

	       if( flag ){
		 if( F[i - 1] > 0.){
		     temp = (20. * log10( (double) F[ i - 1 ] )) ; 
		     if( temp != 0.)fprintf( stderr,  "%-4.f|",  temp ) ; 
		     else fprintf( stderr,  "-0  |" ) ;

		    // GRAPH
		    k = (int) (.5 + (F[i] / delaydiv)) ;  
		    for(jj = 0; jj < k; jj++)fprintf( stderr, "-" ) ;
			fprintf( stderr, "+" ) ;
		}else{
		    // O AMP
		    fprintf( stderr,  "    |o" ) ; 		    
		}

	      }
		
	    }

	    }
    prline( 69,  "=" ) ;
    fprintf( stderr,"\n| BIN |BINFREQ|DELAY  |AMP     |DB  | TIME DELAY GRAPH (0 - %-5.3f)", temp3  ) ; 
    prline( 69,  "=" ) ;
    return(1); 

}
