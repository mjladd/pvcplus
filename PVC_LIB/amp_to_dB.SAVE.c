#include <stdio.h>
#include <math.h>
#include "pv.h"

float amp_to_dB( float amp ){
    
    // CONVERT amp to dB 
	
	float val ; 
	
	if( amp > 0. ) val = 20. * log10( amp ) ; 
 return( val ) ; 

}
