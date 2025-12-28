#include <stdio.h>
#include <math.h>
#include "pv.h"

float Hz_to_MIDI( 
    float Hz
)
{

    float log_two, reference, temp1, temp2, midi ; 

    log_two=flog10(2.) ;  
    reference = 220. * (pow(2.,3./12.)) ;  
  
    // TRANSFORM 
    midi   = 60. + (12. * ( flog10( Hz / reference ) / log_two  ) ) ; 

    return( midi ) ; 


} 

float MIDI_to_Hz( 
    float MIDI
)
{

    float log_two, reference, temp1, temp2, Hz ; 

    reference = 220. * (pow(2.,3./12.)) ;  

	Hz = reference * (pow(2., (MIDI - 60.) /  12.) ) ; 

    return( Hz ) ; 


} 