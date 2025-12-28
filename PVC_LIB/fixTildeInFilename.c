#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <ctype.h>


int fixTildeInFilename( char *tempstring ) 
{
int i, j, len ; 
char *homeDirectory  ; 

	if( tempstring[0] == '~'){ 
		if( (homeDirectory = getenv("HOME")) == NULL ){ 
			// HOME NOT SET
			fprintf( stderr, "\n\n----> ERROR: UNABLE TO RESOLVE ~ IN %s", tempstring ) ; 
			fprintf( stderr, "\n\n----> \"HOME\" SHELL VARIABLE NOT SET." ) ; 
			exit( EXIT_FAILURE ) ;  
		} else {
			len = strlen( tempstring ) ; 
			for(i = 1, j = 0; i < len; i++, j++ ) tempstring[j] = tempstring[i] ; 
			tempstring[ len - 1 ] = '\0' ; 

			strncat( homeDirectory, tempstring, strlen( tempstring ) ) ; 
			strcpy( tempstring, homeDirectory ) ; 

		}; 
	} ; 


	return(1); 
} ; 