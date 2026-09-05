#include <stdio.h>
#include <math.h>
#include "pv.h"


int makeInterpolatedFilterFrame (
	struct func *data,
	float F_lower[], 
	float F_higher[],
	float channel[],
	float iframes_per_sec, 
	int analysis_Nplus2, 
	float timepoint,
	int ainchan, 
	int analysis_chan
)
{
	float filtf ; 
	int i, k, filtflow, filtfhigh, filtfprop ;  
	// channel IS THE MODIFIED OUTPUT ARRAY.



	filtf = iframes_per_sec * timepoint ; // VIRTUAL FRAME 
	filtflow = (int) filtf ; // LOWER FRAME
	filtfhigh = filtflow + 1 ; // HIGHER FRAME
	filtfprop =  filtf - (float) filtflow ; // PROPORTIONAL POSITION BETWEEN FRAMES

	// READ IN THE LOWER AND HIGHER FRAMES
	    
		// POSITION TO LOWER FRAME
		k = ((FFT_HEADER_SIZE + 
		    ((ainchan + (analysis_chan * filtflow) ) 
			    * (analysis_Nplus2) ) ) * sizeof(float)) ; 
		fseek( data->fp,  k,  SEEK_SET ) ; 
		// READ IN LOWER FRAME		
		fread( F_lower, sizeof(float), analysis_Nplus2, data->fp ) ;
    
		// POSITION TO HIGHER FRAME
		k = ((FFT_HEADER_SIZE + 
		    ((ainchan + (analysis_chan * filtfhigh) ) 
			    * (analysis_Nplus2) ) ) * sizeof(float)) ; 
		fseek( data->fp,  k,  SEEK_SET ) ; 
		// READ IN HIGHER FRAME		
		fread( F_higher, sizeof(float) , analysis_Nplus2, data->fp ) ;

	// MAKE INTERPOLATED FRAME
	for( i = 1; i < analysis_Nplus2; i +=2){
	    channel[i - 1] = F_lower[i - 1] + (filtfprop * (F_higher[i - 1] - F_lower[i - 1])) ; // FREQ
	    channel[i] = F_lower[i] + (filtfprop * (F_higher[i] - F_lower[i])) ; // AMP

	} ; 

	return(1); 

} ; 