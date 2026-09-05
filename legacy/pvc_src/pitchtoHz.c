#include "globals.h"


int main(argc,argv)
   int argc;
   char *argv[]; 
   {
   


    char crack()
    ;

    float midC,   octpch,  Hz,  temp1,  log_two, reference ; 
     
    

//#include "underflow.h"

 
/*GIVE USAGE AND EXIT IF NO ARGUMENTS */
   if(argc == 1) {
	fprintf( stderr,"%s",
	"USAGE:\n\n   pitchtoHz octave.pitch class value(s)\n\n" 
	"   This routine prints out the frequency\n"
	"    for the specified octave.pitch class value(s).\n\n"
	 );
	exit(0);
    }

	log_two=flog10(2.) ;  
	reference = 220. * (pow(2.,3./12.)) ;  
    midC = (220.*pow(2., (3./12.))) ; 

    arg_index++ ; 
    fprintf( stdout, "\n\n" ) ; 


    while(arg_index < argc){ 
  
      /* GET OCTAVE.PITCH CLASS VALUE */
       octpch = atof(argv[arg_index]) ;

	// TRANSFORM 
	temp1 = (float) ((int) octpch ) ; // INTEGER PART
	temp1 =  (( 12. * (temp1 - 8.)) + 
	    (100. * (octpch - temp1)) ) / 12. ;
	Hz = midC * pow( 2., (double) temp1 ) ; 

       
       fprintf( stdout, "\nOCT.PCH:\t%-6.2f\tFREQUENCY:\t%-f Hz", octpch,  Hz ) ; 

	arg_index++ ; 
    }

    fprintf( stdout, "\n\n" ) ; 
    exit( 1 ) ; 
}

