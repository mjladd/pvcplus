#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

int findFuncMinMaxAvg( 
    struct func *p,
    float *minval,
    float *maxval,
    float *avgval	  
) 
{
int	i, k,  pd() ;
float	t,  hi,  low,  avg ;  
FILE	*fopen(),  *fp ;  
 

	
    if( p->n <= 1.){
	// ONE VALUE - NO FUNCTION FILE
	*minval = *maxval = *avgval = p->A[ 0 ] ; 
    }else{
	// FUNCTION FILE
	// READ IN AND COUNT VALUES, FIND MAXIMA AND AVERAGE AS WELL.
	i = 0 ;  hi = -999999999. ;  low = 9999999999. ; avg = 0. ;

	if(  p->L == 0. ){
	    // ASCII FILE
	    while( fscanf( p->fp,  " %f ",  &t ) != 0 ){
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

	*minval = low ; 
	*maxval = hi ;
	*avgval = avg ; 

        rewind( p->fp );
    }

    return( 1  );
}

