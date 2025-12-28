#include <stdio.h>
#include <math.h>
#include "pv.h"

float find_centroid(

	float *freq, 
	float *amp,
    float A[], 
    int Nplus2, 
    float lowf, 
    float hif, 
    float fundamental, 
    float *old_value

 ){
    
    int i,  i1,  i2 ; 
    float sum,  a,  temp2,  value ; 
    
    // MAKE THE CENTROID

    // TURN FREQ BOUNDS INTO INDECES


		i1 = 1 +  ( 2 * (int) ( lowf / fundamental ) ) ; 
		i2 = 1 +  ( 2 * (int) ( hif / fundamental ) ) ; 
		if( i2 >= Nplus2 ) i2 = Nplus2 - 1 ; 
		


		if( i2 == i1 ) i2 = i1 + 2 ; 

		sum = 0. ; temp2 = 0. ; 
		if( frame_count == 0 ) *old_value = ((hif - lowf) * .5) + lowf ; 

		for( i = i1; i <= i2; i+= 2){ 
		    a = A[i] ;


		    sum += A[i - 1] * A[i - 1] ;
		    temp2 += ( a * A[i - 1] * A[i - 1] ) ; 
		}


		if( sum > 0. ) value = temp2 / sum ; 
		else value = *old_value ; 

		// LIMIT TO BOUNDS
		if( value > hif ) value =  hif ; 
		if( value < lowf ) value =  lowf ; 


    *freq = value  ; *amp = sum ;  

	return(1.0) ; 

}
