#include "pv.h"

/*
   shift next D samples into righthand end of array A of
   length N, padding with zeros after last sample (A is
   assumed to be initially 0); return 0 when more input
   remains, otherwise return 1 after N-2*D zeros have been
   padded onto end of input
*/

int shiftinf( A, N, D,  fp )
    float A[]; int N, D; FILE *fp;
{
    int 	i;
    short grab;
    static int 	valid = -1;

    if ( valid < 0 )		/* first time only */
	valid = N;
//fprintf( stderr,  "\nSHIFT: HERE 1" ) ;
//fprintf( stderr, "\nshiftinf: ") ;
//fprintf( stderr,  "\nSHIFT: D = %d,  N = %d",  D,  N  ) ;
    for ( i = 0 ; i < N - D ; i++ )
	A[i] = A[i+D];
//fprintf( stderr,  "\nvalid = %d",  valid ) ;
    if ( valid == N ) {
//fprintf( stderr,  "\nSHIFT: HERE 2: " ) ;
	for (i=(N-D); i < N; i++) {
//	fprintf( stderr,  "\nSHIFT: HERE 3: " ) ;
	    if ( (fread(&grab, sizeof(short), 1, fp)) == 0 ) {
		valid = i; 
//fprintf( stderr, "(s) %d ",  (int) grab ) ;

//fprintf( stderr, "(f)%f ",  A[ i ] ) ;
		break;

	    }
	    A[i] = ( (float) grab / 32768. );
	}
    }
    if ( valid < N ) {		/* pad with zeros after 0 */
	for (i=valid; i < N; i++)
	    A[i] = 0.;
	valid -= D;
    }
    return( valid <= 0 );
}
