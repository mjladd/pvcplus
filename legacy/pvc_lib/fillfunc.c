#include <stdio.h>
#include <math.h>
#include "pv.h"



 
float fillfunc( struct func *p, float F[], int  N  )
{


// FILL THE ARRAY F OF SIZE N 
// WITH THE VALUES IN THE FILE.

    int  nv ;


    if( p->n != N ){
	fprintf( stderr,  "\nFILTER FILE SIZE: \t%d  \nFFT SIZE: \t\t%d ",
			  (int) p->n,  N  );
	fprintf( stderr,  "\n\n*ERROR* FILTER FILE SIZE DOES NOT MATCH FFT." );
	fprintf( stderr,  "\n\nSET  SIZE OF ANALYSIS FFT TO MATCH  THE FFT IN filter." );
	fprintf( stderr,  "\n\n................BYE.\n\n" ) ;
	exit(0) ; 
    }


 
    rewind( p->fp );
    
    nv = 0 ; 
    while( fread( &F[ nv ], sizeof(float), 1, p->fp ) != 0 ){ 
	 nv++ ; 
    }
    fprintf( stderr,  "\nSPECTRUM VALUES READ IN.\n\n" ) ;

//    if( nv != N ){
//	fprintf( stderr,  "\nNUMBER OF AMP/FREQ PAIRS: %d,  NEEDED: %d ",
//			  nv/2,  N/2  );
//	fprintf( stderr,  "\nNUMBER OF PAIRS DOES NOT EQUAL FFT/2. BYE" );
//	exit(0) ; 
//    }
    return( 1 ) ; 
}
