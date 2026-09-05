#include <stdio.h>
#include <math.h>
#include "pv.h"
 
int shapespec( float *F, int N2, float octaves,  int R,  float A )
{


    int  i, j, k,  l, m,   lowbin,  hibin,  first=1 ;
    float pma, t,  pmb, *T,  fund, norm,  temp,  temp2 ;

    if(first){
	fvec(T, N2) ; // TEMP SPACE
	first = 0 ; 
    }

  if( octaves <= 0.){
      return(0) ; 
  }else{
    // AVERAGE THE SPECTRUM VALUES WITH WINDOWS THE SIZE OF
    // octaves
    
    fund = (float) R / (float) (N2 * 2) ; 

    // OCTAVES TO MULTIPLIER
    t = octaves / 2. ; 
    pma = pow( (double) 2.,  (double) t ) ; 
    pmb = pow( (double) 2.,  (double) (-t) ) ; 
    
	// LOOP FOR BINS
    for(i = 1,  j = 0; j < N2; i += 2,  j++ ){

	// FIND THE LOW AND HIGH BIN INDECES
	lowbin = (int) (.5 + (pmb * j)) ;
	hibin = (int) (.5 + (pma * j)) ;
	if(lowbin < 0)lowbin = 0;
	if(hibin > N2 )hibin = N2;
	if( hibin == lowbin )hibin = lowbin + 1 ; 

	// FIND THE AVERAGE
	T[j] = 0; temp = 0 ; 
	for(k = lowbin,  m = 0; k < hibin; k++,  m++){
	    T[j] +=  F[ k*2 ] ; 
	
	}	
	?? = T[j] / (float) m ; 

    }
    
    // REPLACE VALUES IN F
    for(j = 0,  i = 0; j < N2; j++,  i += 2) F[i] = T[j] ; 


    return( 1 ) ; 
  }
}
