#include "globals.h"

int main(argc,argv)
   int argc;
   char *argv[]; 
   {
   


    char	*filename, 
		ch, 
		crack()
    ;

    float norm=0.,  amplitude,  dB ; 
    int n = 0 ; 
    

//#include "underflow.h"

 
/*GIVE USAGE AND EXIT IF NO ARGUMENTS */
   if(argc == 1) {
	fprintf( stderr,"%s",
	"USAGE:\n\n   dBtoamp decibel value(s)\n\n" 
	"   This routine prints out the amplitude for the decibel level(s).\n"
	"   ( amplitude = 10^( decibel / 20.) )\n\n"
	 );
	exit(0);
    }

    arg_index++ ; 
   if(arg_index >= argc) {
       // ABORT
       fprintf( stderr, "\n\nWHERE IS THE DECIBEL VALUE??\n\n" ) ;
       exit( 0 ) ; 
   }
    fprintf( stdout, "\n\n" ) ; 


    while(arg_index < argc){ 
  
      /* GET DECIBEL VALUE */
       dB = atof(argv[arg_index]) ;
       //dB = dB + norm ; 
       amplitude = pow( 10., (double) (dB / 20.) ) ; 
       
       fprintf( stdout, "\nDECIBEL LEVEL:\t%-6.2f dB\tAMPLITUDE:\t%-f",  dB,  amplitude ) ; 

	arg_index++ ; 
    }

    fprintf( stdout, "\n\n" ) ; 
    exit( 1 ) ; 
}

