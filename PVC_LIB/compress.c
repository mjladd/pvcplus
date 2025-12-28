#include <stdio.h>
#include <math.h>
#include "pv.h"

int compress( 
    float SP[],  
    int Nplus2,  
    float filtcompthreshamp, 
    float  filtcompamp, 
    float filtcompnormamp 
    )
{    

    int i ;
    int flag;  
    
// IF A BIN'S AMPLITUDE IS GREATER THAN THE THRESHOLD, 
// REDUCE THE AMOUNT LYING ABOVE THE TRHESHOLD BY THE 
// COMPRESSION AMPLITUDE

	flag = 0 ; 

	for( i = 1; i < Nplus2 ; i+= 2){

//fprintf( stderr,  "\nBEFORE: SP[i - 1] = %f,  flag = %d",  SP[i - 1], flag) ; 
	    if( SP[i - 1] >  filtcompthreshamp ){
// COMPRESS
		SP[i - 1] =  filtcompnormamp * 
		    (filtcompthreshamp + (filtcompamp * (SP[i - 1] - filtcompthreshamp))) ; 
		flag = 1. ; 
	    }else{
		SP[i - 1] =  filtcompnormamp * SP[i - 1] ; 
	    }
	} 

	
	return( flag ) ;

  
}
