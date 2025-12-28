#include <stdio.h>
#include <math.h>
#include "pv.h"

float OPPC_to_Hz( 
    float OctavePointPitchClass
)
{
    float temp, integer, freq, fraction ; 
    static float midC ; 
    static int first=1 ; 
    if( first == 1 ){
	midC = (220.*pow(2., (3./12.))) ; 
	first = 0 ; 
    } ; 


    integer = (float) ((int) OctavePointPitchClass ) ; // INTEGER PART
    fraction =  OctavePointPitchClass - integer ; // FRACTION
    if( fraction > .12 ){
        fprintf( stderr, "\n\n--------------> INVALID PITCHCLASS IN OCTAVE.PITCHCLASS: %f\n\n", OctavePointPitchClass ); 
        exit(0) ; 
    } ; 
    temp =  (( 12. * (integer - 8.)) + (100. * fraction) ) / 12. ;
    freq = midC * pow( 2., (double) temp ) ; 
    
    return( freq ) ; 
} 