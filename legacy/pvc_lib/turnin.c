#include <stdio.h>

turnin( A, N, D, fp , valid, SWAP)
    int *valid ; int SWAP ;
    float A[]; int N, D; FILE *fp;
{
    register int 	i;
    short grab;

    if ( *valid < 0 )		/* first time only */
	*valid = N;

    for ( i = 0 ; i < N - D ; i++ )
	A[i] = A[i+D];

    if ( *valid == N ) {
	for (i=(N-D); i < N; i++) {
	    if ( (fread(&grab, sizeof(short), 1, fp)) <= 0 ) {
		*valid = i;
		break;
	    }
	    if( SWAP ){
		grab = ( grab << 8 ) | ( ( grab >> 8 ) & 0xff ) ;
	    }
	    A[i] = ( (float) grab / 32768. );
	}
    }
    if ( *valid < N ) {		/* pad with zeros after 0 */
	for (i=*valid; i < N; i++)
	    A[i] = 0.;
	*valid -= D;
    }
    return( *valid <= 0 );
}

