#include <stdio.h>
#include <math.h>
#include "pv.h"

int smoothfreqs( 
    float A[], 
    float old_A[], 
    int N, 
    float coef,
    float minuscoef
)
{    

    int i ; 

	//*********ATTACK/RELEASE:

  if( coef != 0.)
	for( i = 1; i < N; i+= 2 )
	    A[i] =  (coef * old_A[i] ) + ( minuscoef * A[i] ) ; 
  
  for( i = 1; i < N; i+= 2 ) old_A[i] = A[i] ;

  return( 1 ) ; 

}
