#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>




FILE *crackstring_bin_only(char  s[], struct func *p ) 
{
int	i, j, k,  pd();
float	t ;  
FILE	*fopen(),  *fp ;  
struct       stat st;
char c ; 
FILE *home ;
char ch, homeDirectory[ STRING_SIZE ];


 
    // OTHERWISE TREAT s AS A FILENAME. COUNT THE VALUES IN THE FILE
    //  AND PUT IN N. 



    if( ! isalpha(s[ 0 ]) && (s[ 0 ] != '/')  ){

// CONSTANT
	 sscanf( s,  "%f",  &p->A[ 0 ] ) ;
	 p->n = 1. ; 

    }else{


//***********************************************
// FUNCTION FILE: 
//***********************************************
     

    

	// GET FILE NAME
	sscanf( s,  "%s",  p->fname ) ;

	// OPEN AND COUNT VALUES
	if( (p->fp = fopen( p->fname, "r")) == NULL ){
	    // NULL FILE
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  p->fname ) ; 
	    exit(EXIT_FAILURE); 

    	}else{
	    
	    // TEST FOR DATA TYPE AND SET DATA TYPE FLAG
	    rewind( p->fp ) ; 
	    clearerr( p->fp ) ; 
	        

		p->L = 1.;   // SET DATA TYPE TO DEFAULT FLOAT VALUES

	    rewind( p->fp ) ;     




	    // SO COUNT THEM
	    
		// STAT THE FILE AND TRANSLATE AS FLOATS TO MAKE n


		if(stat(p->fname, &st))  {
		    fprintf(stderr, "\nputlength: (#1)  Couldn't stat file\n\n");
		    exit(EXIT_FAILURE);
		}
		p->n = (int) st.st_size / 4 ;
	
		fprintf( stderr,  "\nThe 32-bit FLOAT data file %s has %d values.\n\n\n",  p->fname,  (int) p->n ) ; 
		if( (int) p->n <= 0 ){
		    prt( "\n\n\tERROR: EMPTY FUNCTION FILE. BYE.\n\n" ) ; exit(EXIT_FAILURE) ; 
		}

	}
	rewind( p->fp );
    }
    return( p->fp  );
}


