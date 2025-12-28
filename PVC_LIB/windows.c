#include <stdio.h>
#include <math.h>
#include "pv.h"



float halfHannWindow( float v ){
    
    
    static float table[ 1024 ] ; 
    static int first=1 ; 
    int i ; 
    float x, twopi ; 
    
    // COSINE CURVE OF PI LENGTH FITTED TO TRAVERSE FROM 1 to 0. 
    // SETUP TABLE FIRST TIME
    
    if( first ){

	twopi = 8.*atan(1.) ;

	for( i = 0 ; i < 1024; i++ ) {
	    x = ((float) i / 1024.) * (twopi * .5) ; 
	    table[ i ] = .5 * ((float) cos( (double) x ) + 1.) ; 
	}
	first = 0 ; 
    }

    i = (int) ((v * 1023.) + .5) ; 
    return( table[ i ] ) ; 
    
    
}



float halfWelchWindow( float v ){
    // v  0	1
    // v  1	0    
    
    
    static float table[ 1024 ] ; 
    static int first=1 ; 
    int i ; 
    float x ; 
    
    
    if( first ){
	
	for( i = 0 ; i < 1024; i++ ) {
	    x = ((float) i / (1024. - 1.))  ; 
	    table[ i ] = 1. - pow( x, 2. ) ; 
	}
	first = 0 ; 
    }
    
    i = (int) ((v * 1023.) + .5) ; 
    return( table[ i ] ) ; 
    
    
}



float halfCosWindow( float v ){
    
    
    static float table[ 1024 ] ; 
    static int first=1 ; 
    int i ; 
    float x, twopi ; 
    
    // COSINE CURVE OF PI/2 LENGTH FITTED TO TRAVERSE FROM 1 to 0. 
    // SETUP TABLE FIRST TIME
    // v  0	1
    // v  1	0    

    if( first ){

	twopi = 8.*atan(1.) ;
	
	for( i = 0 ; i < 1024; i++ ) {
	    x = ((float) i / 1024.) * (twopi * .25) ; 
	    table[ i ] = (float) cos( (double) x ) ; 
	}
	first = 0 ; 
    }
    
    i = (int) ((v * 1023.) + .5) ; 
    return( table[ i ] ) ; 
    
    
}

