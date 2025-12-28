#include <stdio.h>
#include <math.h>
#include "pv.h"

float Hz_to_OPPC( 
    float Hz
)
{

    float log_two, reference, temp1, temp2, octpch ; 

    log_two=flog10(2.) ;  
    reference = 220. * (pow(2.,3./12.)) ;  
  
    // TRANSFORM 
    temp2   = 8. + ( flog10( Hz / reference ) / log_two  ) ; 
    temp1 = (float) ( (int) temp2 ) ; 
    octpch = temp1 + ( .12 * (temp2 - temp1) ) ; //OCT.PCLASS

    return( octpch ) ; 


} 