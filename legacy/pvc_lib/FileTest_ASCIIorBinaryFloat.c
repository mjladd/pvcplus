#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <ctype.h>



int FileTest_ASCIIorBinaryFloat( char *filename, int printFlag )
{
	FILE *fp ; 
	int last, num, bad, dpc ; 
	char c ; 

	fp = fopen( filename, "r" ) ; 

	// ******************
	// FIND INPUT DATA TYPE: ASCII OR FLOAT?
	// TEST FOR DATA TYPE AND SET DATA TYPE FLAG
	num = 0 ; bad = 0 ; dpc = 0 ; 
	last = 0 ; 
	c = getc( fp ) ;    

	while(  !feof( fp ) ){
		num++ ; 
        	if( isdigit(c) || isspace(c) || (c == '.') || (c == '-') || (c == '+' )){
            		// GOOD NUMBER CHARACTER

			if( isspace(c) ){
				// SET POINT COUNT TO 0
				dpc = 0 ; 
			}
		    
			// CHECK LEADING  + OR -
			if( (c == '-') || (c == '+' ) ){			
				if( last != 0 ) bad++ ; // SHOULD FOLLOW SPACE
			}
			// CHECK FOR EXTRA .
			if( c == '.' ){
				dpc++ ; // INCREMENT NUMBER OF POINTS IN THIS WORD
				if( dpc > 1 ) bad++ ; 
			}

			if( isspace(c) ) last = 0 ; else last = 1 ; 

		}else{
			// BAD
			bad++ ; 
		}
		
		c = getc( fp ) ;    
	}
    
	fclose( fp ) ; 

	if( bad ){
		if( printFlag )fprintf( stderr, "INPUT FILE IS BINARY\n" ) ; 
		return( 0 ) ;   // SET DATA TYPE TO DEFAULT FLOAT VALUES
	}else{
		if( printFlag )fprintf( stderr, "INPUT FILE IS ASCII\n" ) ; 
		return( 1 ) ;   // SET DATA TYPE TO ASCII VALUES
	}

}

