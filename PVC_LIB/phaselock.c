#include "pv.h"

void phaselock( float channel[], int N2 )
//float channel[] ; int N2 ;
{
	static int first=1, *peakMarkers ;  

	int ampIndex, bin, j ; 


	if( first ){
		ivec( peakMarkers, N2 ) ; 
		first = 0 ; 
	} ; 
	for( bin = 0; bin < N2; bin++ ) peakMarkers[bin] = 0 ; 


	// MARK PEAKS (BINS WHOSE AMP IS GREATER THAN ITS NEIGHBORS) AND NEIGHBORS.
	for( bin = 1, ampIndex = 2; bin < (N2 - 1); bin++, ampIndex += 2 ){		
		if( (channel[(bin - 1) * 2] < channel[ampIndex]) && (channel[(bin + 1) * 2] < channel[ampIndex]) ){
			peakMarkers[ bin ] = 2 ; peakMarkers[ bin - 1 ] = 1 ; peakMarkers[ bin + 1 ] = 1 ; 
		}else{
			peakMarkers[ bin ] = 0 ; 
		} ; 
	} ; 

	// IDENTIFY (BEST) ALLEGIANCE OF NEIGHBORS
	for( bin = 0; bin < N2; bin++ ){
		if( peakMarkers[bin] == 1 ){
			if( peakMarkers[bin - 1] == 2 ){
				// PEAK BELOW
				if( peakMarkers[bin + 1] == 2 ){
					// PEAK ALSO ABOVE -- FIND CLOSEST IN AMP TO VALLEY
					if( (channel[ (bin * 2) + 2 ] - channel[ bin * 2 ]) > (channel[ (bin * 2) - 2 ] - channel[ bin * 2 ]) ){
						// USE ABOVE
						channel[(bin * 2) + 1] = channel[ (bin * 2) + 3 ] ;
					}else{
						// USE BELOW
						channel[(bin * 2) + 1] = channel[ (bin * 2) - 1 ] ;
					} ; 
				}else{
					// PEAK ONLY BELOW
					channel[(bin * 2) + 1] = channel[ (bin * 2) - 1 ] ; 
				} ; 
			}else{
				// PEAK IS ABOVE
					channel[(bin * 2) + 1] = channel[ (bin * 2) + 3 ] ; 
			} ; 
		}  ; 
	} ; 


} ; 