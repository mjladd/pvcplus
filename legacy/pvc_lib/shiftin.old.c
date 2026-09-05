#include "pv.h"

/*
   shift next D samples into righthand end of array A of
   length N, padding with zeros after last sample (A is
   assumed to be initially 0); return 0 when more input
   remains, otherwise return 1 after N-2*D zeros have been
   padded onto end of input
*/

shiftin( A, N, D )
    float A[]; int N, D;
{
    int 	i;
    static int 	valid = -1;

    if ( valid < 0 )		/* first time only */
	valid = N;

    for ( i = 0 ; i < N - D ; i++ )
	A[i] = A[i+D];

    if ( valid == N ) {
	for (i=(N-D); i < N; i++) {
	    if ( (fread(&A[i], sizeof(float), 1, stdin)) == 0 ) {
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
