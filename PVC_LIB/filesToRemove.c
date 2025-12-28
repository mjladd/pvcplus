#include <stdio.h>
#include <math.h>
#include "pv.h"



void filesToRemove(
	char *file,
	int removeFlag
)
{
	static int first=1; 
	static char *filesToRemove ; 

	
	if(first == 1){
		filesToRemove = realloc( NULL, 10 ); 
		sprintf( filesToRemove, "rm " ) ; 
		first = 0 ; 
	}; 
	
	if( removeFlag == 0 ){
		filesToRemove  = realloc( filesToRemove, strlen( filesToRemove ) + strlen( file ) + 10 ) ; 
		strcat( filesToRemove, " " ) ; strcat( filesToRemove, file ) ;
		// fprintf( stderr, "\nCURRENT FILE REMOVAL COMMAND: %s\n", filesToRemove ) ; 
	}else{
		if( strcmp( filesToRemove, "rm " ) != 0 ){
			// fprintf( stderr, "\nREMOVING FILES WITH: %s\n", filesToRemove ) ;
 			system( filesToRemove ); 
		} ; 
	} ; 

}; 
