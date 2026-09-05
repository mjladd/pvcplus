#include "globals.h"




int main(argc,argv)
   int argc; char *argv[]; 
   {
   


    char ch, 
		crack()
    ;

    float norm=1.,  amplitude,  dB ; 
    
 
/*GIVE USAGE AND EXIT IF NO ARGUMENTS */
   if(argc == 1) {
	fprintf( stderr,"%s",
	"USAGE:\n\n   amptodB [ flags ] amplitude(s)\n" 
	"\tflags:\n"
	"	(Values in brackets denote defaults.)\n"
	"   :n	    normalization amplitude [1.]\n\n"
	"   Converts amplitude value(s) to decibels.\n"
	"   Specification of the -n flag (i.e. -n32000. ) causes the amplitude to first \n"
	"   be normalized to the -n range ( i.e. amplitude / 3200. ). \n"
	 );
	exit(0);
    }

    while((ch = crack(argc,argv,"n|",0)) != CRACK_DONE_FLAG)
{
        switch(ch) {
            case 'n': norm = atof(arg_option) ;
			fprintf( stderr, "\nNORMALIZATION AMPLITUDE = %f",  norm ) ;                 
		 break;	
        }
    }

   if(arg_index >= argc) {
       // ABORT
       fprintf( stderr, "\n\nWHERE IS THE AMPLITUDE VALUE??\n\n" ) ;
       exit( 0 ) ; 
   }
    fprintf( stdout, "\n\n" ) ; 
    
    while(arg_index < argc){ 
  
      /* GET AMPLITUDE VALUE */
       amplitude = atof(argv[arg_index]) ;
       amplitude = amplitude / norm ; 
       dB = 20. * log10( (double) amplitude ) ; 

	if( norm != 1. )
	    fprintf( stdout, "\nNORMALIZED AMPLITUDE:\t%-f\tDECIBELS:\t%-6.2f dB",  amplitude,  dB ) ; 
	else
	    fprintf( stdout, "\nAMPLITUDE:\t%-f\tDECIBELS:\t%-6.2f dB",  amplitude,  dB ) ; 
	
	// GO TO NEXT
	arg_index++ ; 

    }
    fprintf( stdout, "\n\n" ) ; 

    exit( 1 ) ; 
}

