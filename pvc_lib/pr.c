#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>





int prp( struct func *p,  char *s ) 
{
int	i, k,  pd() ;
float	t,  hi,  low,  avg ;  
FILE	*fopen(),  *fp ;  
 

	
    if( p->n <= 1.){
	// ONE VALUE - NO FUNCTION FILE
	fprintf( stderr, "\n%s: %8.3f", s, p->A[ 0 ] ) ; 
	
    }else{
	// FUNCTION FILE
	// READ IN AND COUNT VALUES, FIND MAXIMA AND AVERAGE AS WELL.
	i = 0 ;  hi = -999999999. ;  low = 9999999999. ; avg = 0. ;

	if(  p->L == 0. ){
	    // ASCII FILE
	    while( fscanf( p->fp,  " %f ",  &t ) != EOF ){
		    i += 1 ;
		    if( t > hi ) hi = t ; 
		    if( t < low ) low = t ;
		    avg += t ; 
	    } 

	 }else{
	 
	    // FLOAT FILE
	       while( fread( &t, sizeof(float), 1, p->fp ) != 0 ){
		    i += 1 ;
		    if( t > hi ) hi = t ; 
		    if( t < low ) low = t ;
		    avg += t ; 
		} 


	}

	avg = avg / (float) i ; 

	fprintf( stderr, "\n%s (range): %8.3f - %-8.3f\n\t(%d values, average = %-8.3f)", 
		    s,   low,  hi,  (int) p->n,  avg ) ; 

        rewind( p->fp );
    }

    return( 1  );
}

int prbanner( char *r, int w ) 
{
    int i,  k1,  k2  ; 
 
// PRINT A CHARACTER STRING IN A BANNER

    i = strlen( r ) ;
    k1 = w - i - 2 ; 
    k2 =  k1 / 2 ; 
    k1 = k1 - k2 ; 
        	
    fprintf( stderr, "\n\n" ) ; 
    for( i = 0; i < k1; i++)fprintf( stderr, "=" ) ; 
    fprintf( stderr, " %s ",  r  ) ;
    for( i = 0; i < k2; i++)fprintf( stderr, "=" ) ; 
    fprintf( stderr, "\n\n" ) ; 

//    prstar(60) ; 

    return( 1  );
}



int prt( char *r ) 
{

 
// PRINT A CHARACTER STRING
	
	fprintf( stderr, "\n%s", r ) ; 

    return( 1  );
}

int prs( char *r,  char *s ) 
{

 
// PRINT A CHARACTER STRING
	
	fprintf( stderr, "\n%s: %s", s, r ) ; 

    return( 1  );
}

int prf( float f,  char *s ) 
{

 
// PRINT A FLOAT
	
	fprintf( stderr, "\n%s: %f", s, f ) ; 

    return( 1  );
}

int pri( int i,  char *s ) 
{

 
// PRINT AN INTEGER 
	
	fprintf( stderr, "\n%s: %-d", s, i ) ; 

    return( 1  );
}

int prdoubleline( int i ) 
{
int	 pd(),  k ;
 
// PRINT i NUMBER OF STARS ON A NEW LINE

    k = 0 ; 
	fprintf( stderr, "\n" ) ; 
	
    while( k < i ){
    	fprintf( stderr, "=" ) ; k++ ; 
    }
    return( 1  );
}

int prline( int i, char *s  ) 
{
int	 pd(),  k ;
 
// PRINT i NUMBER OF STARS ON A NEW LINE

    k = 0 ; 
	fprintf( stderr, "\n" ) ; 
	
    while( k < i ){
    	fprintf( stderr, "%s",  s ) ; k++ ; 
    }
    return( 1  );
}
