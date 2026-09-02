#include <stdio.h>

float RIfindpeak(float  buffer[], int N )
{
    float v ;
    int i, flag ;  
    // RIfindpeak FINDS THE PEAK VALUE IN THE buffer ARRAY
    // CONTAINING REAL AND IMAGINARY VALUES
    v = -999999999 ; 
    flag = 0 ;     
    
    for(i = 0; i < N; i++){
	if(buffer[i] > v){
	    v = buffer[i] ;
	    flag = 1 ; 
	} 
    }
 
    if( flag )
	return(v) ;        
    else
	return( 0. ) ; 
}

