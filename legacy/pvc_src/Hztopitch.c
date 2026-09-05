#include "globals.h"

int main(argc,argv)
   int argc;
   char *argv[]; 
   {
   


    char crack()
    ;

    float octpch,  Hz,  temp1,  temp2,  log_two, reference ; 
     
    

//#include "underflow.h"

 
/*GIVE USAGE AND EXIT IF NO ARGUMENTS */
   if(argc == 1) {
	fprintf( stderr,"%s",
	"USAGE:\n\n   Hztopitch frequency value(s)\n\n" 
	"   This routine prints out the octave.pitch class value(s)\n"
	"    for the specified frequencies.\n\n"
	 );
	exit(0);
    }

	log_two=flog10(2.) ;  
	reference = 220. * (pow(2.,3./12.)) ;  

    arg_index++ ; 
    fprintf( stdout, "\n\n" ) ; 


    while(arg_index < argc){ 
  
      /* GET FREQUENCY VALUE */
       Hz = atof(argv[arg_index]) ;

	// TRANSFORM 
	temp2   = 8. + ( flog10( Hz / reference ) / log_two  ) ; 
	temp1 = (float) ( (int) temp2 ) ; 
	octpch = temp1 + ( .12 * (temp2 - temp1) ) ; //OCT.PCLASS

       
       fprintf( stdout, "\nFREQUENCY:\t%-6.2f Hz\tOCT.PCH:\t%-f",  Hz,  octpch ) ; 

	arg_index++ ; 
    }

    fprintf( stdout, "\n\n" ) ; 
    exit( 1 ) ; 
}

