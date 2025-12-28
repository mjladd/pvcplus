#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>





FILE *crackstring(char  s[], struct func *p ) 
{
int	i, k,  pd(),  num,  bad,  last,  dpc ;
float	t ;  
FILE	*fopen(),  *fp ;  
struct       stat st;
char c ; 
 
    // TEST s FOR CHARACTERS. IF NOT CHARACTERS. TREAT AS A  
    // NUMBER CONSTANT, PUTTING IT IN A[ 0 ], AND SETTING N TO 1.
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
	sscanf( s,  "%s",  &p->fname ) ;

	


	// OPEN AND COUNT VALUES
	if( (p->fp = fopen( p->fname, "r")) == NULL ){
	    // NULL FILE
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  p->fname ) ; 
	    exit(0); 

    	}else{
	    


	    // TEST FOR DATA TYPE AND SET DATA TYPE FLAG
	    num = 0 ; bad = 0 ; dpc = 0 ; 
	    rewind( p->fp ) ; 
	    clearerr( p->fp ) ; 
	    
	    last = 0 ; 
	    c = getc( p->fp ) ;    
	    
	    while(  !feof( p->fp ) ){
		    num++ ; 



		if( isdigit(c) || isspace(c) || (c == '.') || (c == '-') || (c == '+' )){

			// GOOD NUMBER CHARACTER

		    if( isspace(c) ){
			// SET POINT COUNT TO 0
			dpc = 0 ; 
		    }
		    
			// CHECK LEADING  + OR -
		    if( (c == '-') || (c == '+' ) ){			
			if( last != 0 ) bad++ ; // SHOULD FOLLOW SPACE
		    }
			// CHECK FOR EXTRA .
		    if( c == '.' ){
			dpc++ ; // INCREMENT NUMBER OF POINTS IN THIS WORD
			if( dpc > 1 ) bad++ ; 
		    }

		    if( isspace(c) ) last = 0 ; else last = 1 ; 

		}else{
			// BAD
			bad++ ; 
		}
		
		c = getc( p->fp ) ;    
	    }
    

	    if( bad ){
		p->L = 1.;   // SET DATA TYPE TO DEFAULT FLOAT VALUES
	    }else{
		p->L = 0.;   // SET DATA TYPE TO ASCII VALUES
	    }

	    rewind( p->fp ) ;     




	    // SO COUNT THEM
	    // BRANCH TO FILE DATA TYPE
	    if( p->L == 0. ){

		// ASCII
		rewind( p->fp );
		p->n  = 0 ; 
		// COUNT VALUES IN FILE
		while( fscanf( p->fp,  " %f ",  &t ) != 0 ) p->n++ ;
		fprintf( stderr,  "\nThe ASCII data file %s has %d values.\n\n\n",  p->fname,  (int) p->n ) ; 


	    }else{
	    
		// STAT THE FILE AND TRANSLATE AS FLOATS TO MAKE n


		if(stat(p->fname, &st))  {
		    fprintf(stderr, "\nputlength: (#1)  Couldn't stat file\n\n");
		    exit(1);
		}
		p->n = (int) st.st_size / 4 ;
	
		fprintf( stderr,  "\nThe 32-bit FLOAT data file %s has %d values.\n\n\n",  p->fname,  (int) p->n ) ; 
		if( (int) p->n <= 0 ){
		    prt( "\n\n\tERROR: EMPTY FUNCTION FILE. BYE.\n\n" ) ; exit( 0 ) ; 
		}
	    }

	}
	rewind( p->fp );
    }
    return( p->fp  );
}


