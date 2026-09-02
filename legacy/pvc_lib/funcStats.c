#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

int funcStats( struct func *p, float *low, float *hi, float *avg, int *length, float *median  ) 
{
int	i, k,  pd(), count=0, notdone ;
float	t, *value, temp ;  
FILE	*fopen(),  *fp ;  
 

	*length = p->n ; 

	fvec( value, *length ) ; 
	
	if( p->n <= 1.){
		// ONE VALUE - NO FUNCTION FILE
		*low = *hi = *avg = *median = p->A[ 0 ] ;
		*length = 1 ;  
		return(1) ; 	
	}else{
		// FUNCTION FILE
		// READ IN AND COUNT VALUES, FIND MAXIMA AND AVERAGE AS WELL.
		i = 0 ;  *hi = -999999999. ;  *low = 9999999999. ; *avg = 0. ;

		if(  p->L == 0. ){
	    		// ASCII FILE
	    		while( fscanf( p->fp,  " %f ",  &t ) != 0 ){
                 	value[ count ] = t ; count++ ; 
		    		i += 1 ;
		    		if( t > *hi ) *hi = t ; 
		    		if( t < *low ) *low = t ;
		    		*avg += t ; 
	    		} 

		}else{
			rewind( p->fp ) ;	 

			// FLOAT FILE
			while( fread( &t, sizeof(float), 1, p->fp ) != 0 ){
				value[ count ] = t; count++ ; 
				i += 1 ;
				if( t > *hi ) *hi = t ; 
				if( t < *low ) *low = t ;
				*avg += t ; 
			} 

		}

		// SORT FOR MEDIAN
		notdone = 1 ; 
		while( notdone == 1 ){
			notdone = 0 ; 
	    		for( i = 0 ; i  < *length - 1; i += 2 ){
		 		if( value[ i ] > value[ i + 1 ] ){
             			// SWAP
             			temp = value[ i ] ; value[ i ] = value[ i + 1 ] ; value[ i + 1 ] = temp ; 
					notdone = 1 ; 
              		} ; 
      		} ; 
		}; 

		*median = value[ *length / 2 ] ; 

		*avg = *avg / (float) i ; 

		rewind( p->fp ) ;
    	}

    	return( 1  );
}

