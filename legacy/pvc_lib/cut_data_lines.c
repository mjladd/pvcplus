#include <stdio.h>
#include <math.h>
#include "pv.h"

#include <ctype.h>


int cut_data_lines( 
    char *datafile,  
    char *new_datafile,  
    int data_size

){
 
 
 
FILE *dfile,  *ndfile,  *tfile ; 
int data_end,  solo_on,  baddata=0 ;
char c,  last_c='\n', *name , namet[  STRING_SIZE  ], tstring[ STRING_SIZE ] ;
int i, parenopen,  sendout ; 
   

    data_end = 0 ;  parenopen = 0 ; sendout = 1 ; 
 // MAKE TEMP NAME
  
 //    cuserid( user ) ;   
 	name = pvc_user_tag(); 


    sprintf( namet, "/tmp/%s.%d.temp_data_file2", name, (int) getpid() ) ;

    // OPEN TEMP FILE

    if( (tfile = fopen( namet, "w" )) == NULL ){
	fprintf( stderr, "\n%s <-- PROBLEMS. BYE.\n",  namet ) ; 
	exit(EXIT_FAILURE); 
    }

// OPEN DATA FILE
  	if( (dfile = fopen( datafile, "r")) == NULL ){
	    fprintf( stderr, "\n%s <-- NO SUCH FILE. BYE.\n",  datafile ) ; 
	    exit(EXIT_FAILURE); 
	}

// CUT {} COMMENTS

 
    while( data_end != 1 ){
	last_c = c ; c = getc( dfile ) ;

	if( feof( dfile ) == 0 ) {
	    // DATA
	    if( c == '{' ){
		// COMMENT BEGIN
		if( parenopen ){
		    // BRACE ALREADY OPEN
		    prt( "\n\n\tDATA FILE:  TWO OPEN BRACES IN A ROW!\n" ) ; exit(EXIT_FAILURE) ; 
		}else{
		    // NEW OPEN BRACE: TURN OFF OUTPUT
		    sendout = 0 ; parenopen = 1 ; 
//prt("OPEN CURLY BRACE" ) ; 
		}
		
	    }else if( c == '}' ){
		// COMMENT END
		if( parenopen ){
		    // CLOSE OF CURLY: TURN OUTPUT ON
		    sendout = 1 ;  parenopen = 0 ; 
		}else{
		    // BRACE NOT OPEN! 
		    prt( "\n\n\tDATA FILE:  BRACE NOT OPEN!\n" ) ; exit(EXIT_FAILURE) ; 
		}
	    }else{
		// DATA
		if( sendout ) fprintf( tfile,  "%c",  c ) ; 
	    }
	}else{
	    // END OF DATA
	   data_end = 1 ;  
	}
 
    }


    if( parenopen ){
	// OPEN PAREN
	prt("\n\n\tDATA FILE: OPEN CURLY BRACE!\n") ; exit(EXIT_FAILURE) ; 
    }


    fclose( tfile ) ;


//****************

 // MAKE TEMP NAME 

 //   cuserid( name ) ; 
    name = pvc_user_tag(); 

   
    sprintf( new_datafile, "/tmp/%s.%d.temp_data_file", name, (int) getpid() ) ;
   

// CREATE THE FILE

// OPEN FILE
    if( (tfile = fopen( namet, "r" )) == NULL ){
	fprintf( stderr, "\n%s <-- PROBLEMS. BYE.\n",  namet ) ; 
	exit(EXIT_FAILURE); 
    }

    if( (ndfile = fopen( new_datafile, "w" )) == NULL ){
	fprintf( stderr, "\n%s <-- PROBLEMS. BYE.\n",  new_datafile ) ; 
	exit(EXIT_FAILURE); 
    }


// CHECK FOR SOLO LINES
    data_end = 0 ; 
    solo_on = 0 ; 
    c = ' ' ;  
    fprintf( stderr, "\n" ) ; 

    while( data_end != 1 ){
	last_c = c ; c = getc( tfile ) ;

	if( feof( tfile ) == 0 ) {
	    // DATA
	    if( (c == '!') && ( !isalpha( last_c ) ) && ( last_c != '/' )  ){
		// SOLO MODE
		solo_on = 1 ; 
	    }

	}else{
	    // END OF DATA
	   data_end = 1 ;  
	}

    }

    rewind( tfile ) ; 
    data_end = 0 ;  
    c = ' ' ;  
    fprintf( stderr, "\n" ) ; 

    if( solo_on ){
	// SOLO MODE
     
     
	while( data_end != 1 ){
	    last_c = c ; c = getc( tfile ) ;
    
	    if( feof( tfile ) == 0 ) {
		// DATA

		if( (c == '!') && ( !isalpha( last_c ) ) && ( last_c != '/' )  ){
//prt("TRANSFERRING SOLO LINE\n" ) ; 
		// SOLO'ED LINE
			fprintf( stderr, "\n" ) ; 
		    for( i = 0 ; i < data_size; i++ ){
			fscanf( tfile, "%s", tstring ) ; 
			fprintf( stderr, "%s ", tstring  ) ; 
			fprintf( ndfile, "%s ", tstring ) ;
		    }
			fprintf( stderr, "\n" ) ; 
	       }
	    }else{
		// END OF DATA
	       data_end = 1 ;  
	    }
     
	}

    }else{
	// MUTE MODE

     // GET CHARACTERS FROM OLD, IF NOT"*" or "m" THEN PASS,
     // ELSE SKIP data_size NUMBERS PAST THE * or m     
     // THEN CONTINUE
     
     
	while( data_end != 1 ){
	    last_c = c ; c = getc( tfile ) ;
    
	    if( feof( tfile ) == 0 ) {
		// DATA
		if( ((c == 'm')) && ( !isalpha( last_c ) ) && ( last_c != '/' )  ){
		// COMMENTED/MUTED LINE
//prt("REMOVING LINE" ) ; 
		    for( i = 0 ; i < data_size; i++ ) fscanf( tfile, "%s", tstring ) ;
		}else{
		    fprintf( ndfile,  "%c",  c ) ;  fprintf( stderr,  "%c",  c ) ; 
	         }
	    }else{
		// END OF DATA
	       data_end = 1 ;  
	    }
     
	}    
    
    
    }

    fclose( dfile ) ; fclose( tfile ) ; fclose( ndfile ) ; 

    if( (ndfile = fopen( new_datafile, "r" )) == NULL ){
	fprintf( stderr, "\n%s <-- PROBLEMS. BYE.\n",  new_datafile ) ; 
	exit(EXIT_FAILURE); 
    }



    // CHECK FILE FOR ALPHABETIC CHARACTERS (ERRORS); IF THERE ARE ABORT!
    
    rewind( ndfile ) ; 
    data_end = 0 ;  
 
  
	while( data_end != 1 ){
	    last_c = c ; c = getc( ndfile ) ;

	    //    
	    if( feof( ndfile ) == 0 ) {
	
/*
		if( isalpha(c) ){
		   // ALPHA! ABORT
		   baddata = 1 ; 
		  if( first ){
		      fprintf( stderr, "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
		      fprintf( stderr, "\n\tYOUR DATA FILE HAS NON-NUMERIC CHARACTERS IN IT. EXITING!\n\n" ) ;  
		      fprintf( stderr, "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n" );
		      
			    first = 0 ; 
		   }
		}
		if( baddata ) fprintf( stderr, "%c",  c ) ; 
*/
		
	    }else{
		// END OF DATA
	       data_end = 1 ;  
	    }
     
	}     

    if( baddata ) {
	fprintf( stderr, "\n\n\n" ) ; exit(EXIT_FAILURE) ; 
    }
    fclose( ndfile ) ; 



return( 1 ) ; 

}
