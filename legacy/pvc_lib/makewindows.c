#include "pv.h"

/*
 * make balanced pair of analysis (A) and synthesis (S) windows;
 * window lengths are Nw, FFT length is N, synthesis interpolation
 * factor is I, and osc is true (1) if oscillator bank resynthesis 
 * is specified
 */
void makewindows( float H[], float A[], float S[], int Nw, int N, int I, int osc )
//    float H[], A[], S[] ; int Nw, N, I, osc ;
{
 int i ;
 float sum,  besselalpha ;
 float alpha ; 

float besselfunc( float x ) ; 

int n1,  n2 ; 
// TEST FFT SIZE FOR POWER OF TWO

n1 = 2,  n2 = n1 * 2 ; 


while( (N > n2) || (N < 2 ) ){
    n1 = n2 ; n2 = n1 * 2 ; 
}

if( (N != n1) && (N != n2) ){
    // PROBLEMS
prline( 69,   "!" ) ; 
    fprintf( stderr, "\n\n\t******* YOUR FFT SIZE OF %d IS NOT A POWER OF TWO. BYE. ******\n\n",  N ) ; 
prline( 69,   "!" ) ; 
    fprintf( stderr, "\n\n" ) ; 
    exit(EXIT_FAILURE); 
}



/*
 * basic  windows
 */

if( window_type == 0 ){
    // HAMMING WINDOW
    prt( "..............USING HAMMING WINDOW" ) ; 
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 0.54 - 0.46*cos( TWOPI*i/(Nw - 1) ) ;

}else if( window_type == 1 ){
    prt( "..............USING RECTANGULAR WINDOW" ) ; 
    // RECTANGULAR WINDOW
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 1.0 ;
    
}else if( window_type == 2 ){
    prt( "..............USING BLACKMAN WINDOW" ) ; 
    // BLACKMAN WINDOW
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 
	    .42 - .5 * cos((TWOPI / (Nw - 1)) * i) + 0.08 * 
		cos((2. * TWOPI / (Nw - 1)) * i ) ;
    
}else if( window_type == 3 ){
    prt( "..............USING BARTLET TRIANGULAR WINDOW" ) ; 
    // TRIANGULAR WINDOW
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 
	    1. - (1./ (float) (Nw+1)) * 
		fabs(2.* (float) i - (float) Nw + 1.) ;     

}else if( (window_type >= 4) && (window_type <= 12) ){
    prt( "..............USING KAISER WINDOW" ) ; 
    // KAISER WINDOW
	alpha =  (float) window_type ; 
	prf( alpha,  "ALPHA" ) ;
	prf( (-7.5 * alpha),  "SIDELOBE DECIBEL LEVEL" ) ;   
    
     besselalpha = besselfunc( alpha ) ; 
    for( i = 0 ; i < Nw ; i++ ){

	H[i] = fabs( (float) (2.* i) - (float) Nw + 1.) ; 
	H[i] = ( 1. / (float) (Nw - 1)) * H[i] ; 
	H[i] = H[i] * H[i] ;
	H[i] = (float) sqrt( 1. - (double) H[ i ] ) ; 
	H[i] = alpha * H[i] ; 
	H[i] = A[i] = S[i] =
	    besselfunc( H[i] ) / besselfunc( alpha ) ; 
//prf( H[ i ],  "KAISER FUNCTION" ) ; 


     }

}else if( window_type == 13 ){
    prt( "..............USING BLACKMAN-HARRIS WINDOW" ) ; 
    // BLACKMAN-HARRIS WINDOW
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 
	    .35875 
		- .48829 * cos((TWOPI / (Nw - 1)) * i) 
			+ 0.14128 * cos((2. * TWOPI / (Nw - 1)) * i )
				- .01168 * cos(( (3. / 2.) * TWOPI / (Nw - 1)) * i) ;
    
}else if( window_type == 14 ){
    prt( "..............USING NUTTAL WINDOW" ) ; 
    // NUTTAL WINDOW
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 
	    .355768 
		- .487396 * cos((TWOPI / (Nw - 1)) * i) 
			+ 0.144232 * cos((2. * TWOPI / (Nw - 1)) * i )
				- .012604 * cos(( (3. / 2.) * TWOPI / (Nw - 1)) * i) ;

}else if( window_type == 15 ){
    prt( "..............USING BLACKMAN-NUTTAL WINDOW" ) ; 
    // NUTTAL BLACKMAN-WINDOW
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 
	    .3635819 
		- .4891775 * cos((TWOPI / (Nw - 1)) * i) 
			+ 0.1365995 * cos((2. * TWOPI / (Nw - 1)) * i )
				- .0106411 * cos(( (3. / 2.) * TWOPI / (Nw - 1)) * i) ;
    
}else if( window_type == 16 ){
    prt( ".............USING FLAT-TOP WINDOW" ) ; 
    // FLAT TOP WINDOW
    for ( i = 0 ; i < Nw ; i++ )
	H[i] = A[i] = S[i] = 
	( 1.0 - (1.93 * cos( (TWOPI * i)/(Nw - 1) ))
        + (1.29 * cos( (2. * TWOPI * i)/(Nw - 1) ))
        - (0.388 * cos( (3. * TWOPI * i)/(Nw - 1) ))
        + (0.032 * cos( (4. * TWOPI * i)/(Nw - 1) ))
        ) / 5.0 ;  


    
}else{
    
    prt( "NOT A LEGAL WINDOW TYPE. BYE." ) ; 
    exit(EXIT_FAILURE) ; 
}


/*
 * when Nw > N, also apply interpolating (sinc) windows to
 * ensure that window are 0 at increments of N (the FFT length)
 * away from the center of the analysis window and of I away
 * from the center of the synthesis window
 */
    if ( Nw > N ) {
     float x ;
     float PI = 4.*atan(1.) ;
/*
 * take care to create symmetrical windows
 */
	x = -(Nw - 1)/2. ;
	for ( i = 0 ; i < Nw ; i++, x += 1. )
	    if ( x != 0. ) {
		A[i] *= N*sin( PI*x/N )/(PI*x) ;
		if ( I )
		    S[i] *= I*sin( PI*x/I )/(PI*x) ;
	    }
    }
/*
 * normalize windows for unity gain across unmodified
 * analysis-synthesis procedure
 */
    for ( sum = i = 0 ; i < Nw ; i++ )
	sum += A[i] ;

    for ( i = 0 ; i < Nw ; i++ ) {
     float afac = 2./sum ;
     float sfac = Nw > N ? 1./afac : afac ;
	A[i] *= afac ;
	S[i] *= sfac ;
    }

    if ( Nw <= N && I ) {
	for ( sum = i = 0 ; i < Nw ; i += I )
	    sum += S[i]*S[i] ;
	for ( sum = 1./sum, i = 0 ; i < Nw ; i++ )
	    S[i] *= sum ;
    }



}

  float besselfunc( float x ){
  
  
  int k,  i ; 
  float V,  D=.0000001,  S,  factsum,  Ssum ; 
  
  S = 1. ; Ssum = 0. ; 
  k = 1 ; 
  
  while( S > D ){
	
      /* FACTORIAL */
      factsum = 1. ; 
      for( i = 2; i <= k; i++ ) factsum = factsum * (float) i ;  

    S = pow( (double) x,  (double) k ) / ( pow( 2.,  (double) k  ) * factsum ) ; 
    S = S * S ; 
    Ssum += S ; 
    k++ ; 

  }

    return( Ssum ) ; 
  
  }  
