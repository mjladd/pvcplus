#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>



float crackfloat( char  s[], char ch ){
    float outVal ; 
    char name[ STRING_SIZE ], command[ STRING_SIZE ], tempFile[ STRING_SIZE ] ; 

   if( ! isalpha(s[ 0 ]) && (s[ 0 ] != '/')  ){

	// CONSTANT
	 sscanf( s,  "%f",  &outVal ) ;
	 return( outVal ) ; 

    }else{
	// GET FILE NAME
	sscanf( s,  "%s",  name ) ;
      fprintf( stderr, "\n\nERROR: Function file  -------->    %s    <---------\n", name ) ; 
      fprintf( stderr, "\nnot allowed for %s parameter flag %c: \n\n", routine, ch ) ;
	snprintf( tempFile, sizeof(tempFile), "/tmp/%s_command", routine ) ;
	snprintf( command, sizeof(command), "%s >& %s", routine, tempFile ) ;
 	system( command ) ;
	snprintf( command, sizeof(command), "grep \"%c:\" %s", ch, tempFile ) ;
 	system( command ) ; 
	prt( "Constants only." ) ; 
	prt( "\n. . . . . BYE.\n\n" ) ;        
      exit(EXIT_FAILURE) ; 

    } ; 

} ;   
