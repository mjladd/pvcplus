#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pv.h"
#include <sndfile.h>
#include <string.h>

#include <unistd.h>

int autoplay(
	int autoplayreps, 
	int formatSwitchflag
){
	static char numberLetters[4] = {'1','2','3','4' } ;  
	char playfiles[3][ STRING_SIZE ] ; 

	int	
		n, j, m, i, k, l ; 
    
	char	commandString[ STRING_SIZE ],
                string[10], 
		promptString[ STRING_SIZE ],
                ch
	;



            if( (autoplayreps == -2) || (autoplayreps == -1) || (autoplayreps >= 1) ){ // 2
                fprintf( stderr, "\nOUTPUT FILE: %s", ofile ) ; 
			fprintf( stderr, "\n" ); 

			sprintf( commandString, "sndfile-info %s | head -n14 | tail -n9", ofile ) ; 
			system( commandString  ) ; 

			if( formatSwitchflag == 1 ){
               		fprintf (stderr, "\n\nFORMAT-SUPPLYING OUTPUT SOUND FILE %s  WAS NOT FOUND.\n", ofile ) ;
 				fprintf( stderr, "FILE FORMAT WAS COPIED FROM INPUT SOUND FILE INSTEAD. \n" ) ; 
				fprintf( stderr, "\n==> NOTE: FILE NAME EXTENSION MAY NOT MATCH HEADER FORMAT. <==\n\n" ) ; 
			} ; 
                prline( 69,  "=" ) ;
			fprintf( stderr, "\n" ); 

                sprintf( commandString, "sndfile-play %s", ofile ) ; 

                if( autoplayreps >= 1){ // 1
                   // AUTO-REPEAT
                   fprintf( stderr, " AUTO-PLAY MODE, OUTPUT FILE REPETITIONS = %d\n", autoplayreps ) ; 
                   for( n = 0; n < autoplayreps ; n++ )
                   { 
                       fprintf( stderr, "%d. ", n + 1 ) ; 
                       system( commandString ) ; 
                   } ; 
                }else{ // 1
//				fprintf( stderr, "ifile: %s, afile: %s, ofile: %s", ifile, afile, ofile ) ; 
		    		// IDENTIFY FILES 
		    		i = 0 ; 
                    	if( strlen( ifile ) > 0  ){
					strcpy( playfiles[ i ], ifile ) ; i++ ;
		    		}; 
                    	if( strlen( afile ) > 0  ){
					strcpy( playfiles[ i ], afile ) ; i++ ;
		    		}; 
                    	if( strlen( ofile ) > 0  ){
					strcpy( playfiles[ i ], ofile ) ; i++ ;
		    		}; 

		      	// INTERACTIVE
                    	if( autoplayreps == -2 ){
                        	fprintf( stderr, 
						" INTERACTIVE MODE (Play output file once, then prompt for more.)\n" ) ; 
                        	ch = numberLetters[i - 1] ; 
		        		fprintf( stderr, "Play File:\n" ) ; 
		        		for( j = 0; j < i; j++ ){
			    			fprintf( stderr, "\t(%d) %s\n", j + 1, playfiles[j] ) ;
		        		} ; 
		        		fprintf( stderr, "\t   or\n\t(q) quit                 (Ctrl-c ends current play.)\n" );
                    	}else{
                        	fprintf( stderr, " INTERACTIVE MODE (Prompt for each play of file.)\n" ) ; 
		        		fprintf( stderr, "Play File:\n" ) ; 
		        		for( j = 0; j < i; j++ ){
			    			fprintf( stderr, "\t(%d) %s\n", j + 1, playfiles[j] ) ;  
		        		} ; 
		        		fprintf( stderr, "\t   or\n\t(q) quit\n" );
					fprintf( stderr, "Play file (1-%d) or quit(q)?           (Ctrl-c ends current play.)\n", i ) ;
                        
					scanf("%s", string ) ; 
                        	ch = string[0] ;  
                    } ; 
                   

                    for( ; ; ){
//					fprintf( stderr, "ch: %c\n", ch ) ; 
					switch( ch ){
			    			case '1':
							sprintf( commandString, "sndfile-play %s", playfiles[0] ) ; break; 
			    			case '2':
							sprintf( commandString, "sndfile-play %s", playfiles[1] ) ; break;  
			    			case '3':
							sprintf( commandString, "sndfile-play %s", playfiles[2] ) ; break; 
			    			default:
							exit(EXIT_FAILURE) ; 
 					} ; 
//					fprintf( stderr, "COMMAND: %s", commandString ) ; 

                        	system( commandString ) ;
                        	fprintf( stderr, "Play file (1-%d) or quit(q)?           (Ctrl-c ends current play.)\n", i ) ;
                        	scanf("%s", string ) ; 
                        	ch = string[0] ;  
                    } ; 
                } ;  // 1

            } ; // 2

	return(1); 

} ;

