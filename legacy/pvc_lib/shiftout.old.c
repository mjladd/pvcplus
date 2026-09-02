#include "pv.h"

/*
   if output time n >= 0, output first I samples in
   array A of length N, then shift A left by I samples,
   padding with zeros after last sample
*/

shiftout( A, N, I, n )
    float A[]; int N, I, n;
{
 int i;
    if ( n >= 0 ) 
	fwrite( A, sizeof(float), I, stdout );
 
    for ( i = 0; i < N - I; i++ )
	A[i] = A[i+I];
    for ( i = N - I; i < N; i++ )
	A[i] = 0.;
}
