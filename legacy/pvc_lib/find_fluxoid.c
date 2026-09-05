#include <stdio.h>
#include <math.h>
#include "pv.h"

float find_fluxoid(

    float A[], 
    float old_A[], 
    int N, 
    float lowf, 
    float hif, 
    float fundamental, 
    float *old_value, 
    int flux_weight_flag

 ){
    
    int i,  i1,  i2 ; 
    float sum,  temp2,  ampsum ; 
    
    // MAKE THE FLUXOID

    // TURN FREQ BOUNDS INTO INDECES


		i1 = 1 +  ( 2 * (int) ( lowf / fundamental ) ) ; // LOW FREQ
		i2 = 1 +  ( 2 * (int) ( hif / fundamental ) ) ; // HI FREQ
		if( i2 >= N ) i2 = N - 1 ; 
		
		if( i2 == i1 ) i2 = i1 + 2 ; 

		sum = 0. ; temp2 = 0. ; ampsum = 0. ;  
		if( frame_count == 0 ) *old_value = 0. ; 

		for( i = i1; i <= i2; i+= 2){ 

		    ampsum += A[i - 1] ; 

		    if(flux_weight_flag == 1)
			    sum += (   A[i - 1] * fabs( A[i]  - old_A[i] ) ) ;
		    else
			    sum += ( fabs( A[i]  - old_A[i] ) ) ;
		}

	
    return( sum ) ; 


}
