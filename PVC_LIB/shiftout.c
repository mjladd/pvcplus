#include "pv.h"

/*
   if output time n >= 0, output first I samples in
   array A of length N, then shift A left by I samples,
   padding with zeros after last sample
*/

void shiftout( float A[], int N, int I, int n, int flushflag )
//    float A[]; int N, I, n, flushflag ;
{
 int i;

if( flushflag == 0){
//pri( n,  "SHIFTOUT: n" ) ; 
    if ( n >= 0 ) 
	bufferout(A, I, 0)  ; 
    for ( i = 0; i < N - I; i++ )
	A[i] = A[i+I];
    for ( i = N - I; i < N; i++ )
	A[i] = 0.;

}else{
    // FLUSH
//pri( flushflag,  "SHIFTOUT: flushflag" ) ; 
	bufferout(A, I, 1)  ; 
    
}

}
