#include <stdio.h>
#include <math.h>
#include "pv.h"



 
float fval( struct func *p, float dur, float  T )
{
   
    float z ;
    int i, nv ;
     
// IF CONSTANT, SKIP OUT
//fprintf(stderr,"\nN = %f,  T = %f,  dur = %f\n", N,  T, dur );
 
if( p->n <= 1.) return( p->A[ 0 ] ) ; 
      
// IF FIRST CALL, SETUP
    if( T <= 0. ){
fprintf(stderr,"\n********SETUP\n" ) ; 	
	fread( &p->A[ 2 ], sizeof(float), 1, p->fp ) ;
	fread( &p->A[ 3 ], sizeof(float), 1, p->fp ) ;

	if( p->n >= 3. ){
	    fread( &p->A[ 4 ], sizeof(float), 1, p->fp ) ;
	}else{
// USING THE SLOPE OF p->A[3] to p->A[2], INVENT p->A[ 4 ] 
	    p->A[ 4 ] = p->A[ 3 ] + (p->A[ 3 ] - p->A[ 2 ]) ;

	 }

// USING THE SLOPE OF p->A[3] to p->A[2], INVENT p->A[ 1 ] 
	  p->A[ 1 ] = p->A[ 2 ] - (p->A[ 3 ] - p->A[ 2 ]) ;

// SET FUNCTION TIME TO 0
	  p->A[ 5 ] = 0. ; 
    }
    
// GENERATE SOME VALUES
	if( T > dur ) T = dur ; 
	p->A[ 6 ] = p->L * (p->n - 1.) * ( T / dur ) ; 
	z = p->A[ 6 ] - p->A[ 5 ] ; 
	nv = floor( (double) z )  ;
	p->A[ 5 ] = p->A[ 5 ]  + (float) nv ; 
	z = p->A[ 6 ] - (float) p->A[ 5 ] ;
	fprintf( stderr,  "\nz = %f",  z ) ;   
// SHIFT VALUES IN UNTIL WE ARE IN THE CORRECT PLACE
	while( nv > 0 ){
	    for( i = 1 ; i < 4; i++) p->A[ i ] = p->A[ i + 1 ] ; 
	    if( fread( &p->A[ 4 ], sizeof(float), 1, p->fp ) == 0 ){
		rewind( p->fp ); 
		fread( &p->A[ 4 ], sizeof(float), 1, p->fp ) ; 
		//p->A[ 4 ] = p->A[ 3 ] + (p->A[ 3 ] - p->A[ 2 ]) ;
	    }
	    nv-- ; 
	}

fprintf( stderr,  "\n" ) ;
for( i = 0; i < 7; i++){
    fprintf( stderr,  " %f",  p->A[ i ] ) ; 
}	
fprintf( stderr,  "\n" ) ;
	
// OUTPUT THE VALUE WHICH IS THE CROSSFADE-INTERPOLATED VALUE
// LYING AT THE z FRACTIONAL POINT

       p->A[ 0 ] = ((p->A[2] + (p->A[2] - p->A[1]) * z) * (1. - z) ) +
		((p->A[3] - (p->A[4] - p->A[3]) * (1. - z)) * z ) ;



   	
fprintf( stderr,  "\nA[ 0 ] = %f",  p->A[ 0 ] ) ;    
    return( p->A[ 0 ] ) ;	
}


