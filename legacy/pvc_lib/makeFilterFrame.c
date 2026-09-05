#include <stdio.h>
#include <math.h>
#include "pv.h"


int readFilterFrame (
	struct func *data,
	float channel[],
	int frame, 
	int analysis_N, 
	int ainchan, 
	int analysis_chan
)
{
	int k ;  

	// READ IN THE LOWER AND HIGHER FRAMES
	    
		// POSITION TO LOWER FRAME
		k = ((FFT_HEADER_SIZE + 
		    ((ainchan + (analysis_chan * frame) )  * (analysis_N + 2) ) ) * sizeof(float)) ; 

		fseek( data->fp,  k,  SEEK_SET ) ; 
		// READ IN LOWER FRAME		
		fread( channel, sizeof(float), analysis_N + 2, data->fp ) ;
    

	return(1); 

} ; 
