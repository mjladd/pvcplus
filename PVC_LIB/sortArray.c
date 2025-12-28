#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

void sortArray( 
	float	array[],
	int 	numberOfValues
) 
{
	int	i, ii, k,  notdone ;

	// SORT
	notdone = 1 ;
	while( notdone ){ 
		notdone = 0 ; 

		for(ii = 0; ii < 2; ii++){
			for( i = ii ; i  < (numberOfValues - 1); i += 2 ){
		    		if( array[i] > array[i + 1] ){
					// SWITCH
					k = array[i] ; array[i] = array[i + 1] ; array[i + 1] = k ; 
					notdone = 1 ; 
				} ; 
				if( ii == 0 ) ii = 1 ; else ii = 0 ; 
			}
		}

	}

}
