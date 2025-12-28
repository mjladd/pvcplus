#include "pv.h"

/*
   shift next D samples into righthand end of array A of
   length N, padding with zeros after last sample (A is
   assumed to be initially 0); return 0 when more input
   remains, otherwise return 1 after N-2*D zeros have been
   padded onto end of input
*/

int shiftin( float A[],int N,int D )
//    float A[]; int N, D;
{
    int 	i;
    static int 	valid = -1, sinchan=0 ;

//pri( channow, "SHIFTIN: channow" ) ; 
//pri( IO_reset, "SHIFTIN: IO_reset" ) ; 
    if( IO_reset == 1 ){
	// RESET
	valid = -1 ; 
	sinchan = 0 ; 
    }
    if( sinchan != channow ){
	// RESET
//	prt( "SHIFTIN: RESETING ...." ) ; 
	valid = -1 ; sinchan = channow ;
    }

//pri( sinchan, "sinchan" ) ; 
//pri( valid,  "SHIFTIN: valid" ) ; 
//pri( N,  "SHIFTIN: N" ) ; 
//pri( D,  "SHIFTIN: D" ) ; 

    if ( valid < 0 )		/* first time only */
	valid = N;


    for ( i = 0 ; i < N - D ; i++ )
	A[i] = A[i+D];

//pri( valid,  "SHIFTIN: valid" ) ; 

    if ( valid == N ) {
	for (i=(N-D); i < N; i++) {

	    if ( ( bufferin( &A[i] ) ) == 0 ) {
//prt( "SHIFTIN: breaking" ) ; 
		valid = i;
		break;
	    }
	}
    }


    if ( valid < N ) {		/* pad with zeros after 0 */
	for (i=valid; i < N; i++)
	    A[i] = 0.;
	valid -= D;
    }

    return( valid <= 0 );
}
