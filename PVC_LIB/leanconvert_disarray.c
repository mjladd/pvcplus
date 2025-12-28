#include "pv.h"

leanconvert1_disarray( float S[], float C[], int N2, int D, int R, int Bins[] )
//float S[], C[]; int N2, D, R, Bins[] ; 
{

 int		real, imag,
		amp, phase;
 float		a, b;
 register int		i;

 for ( i = 0; i <= N2; i++ ) {
   imag = phase = ( real = amp = Bins[i]<<1 ) + 1;
   a = ( Bins[i] == N2 ? S[1] : S[real] );
   b = ( Bins[i] == 0 || Bins[i] == N2 ? 0. : S[imag] );
   C[amp] = hypot( a, b );
   C[phase] = -atan2( b, a );
 }
}

leanconvert2_disarray( float S[], float C[], int N2, int D, int R, int Bins[] )
//float S[], C[]; int N2, D, R, Bins[] ; 
{

 int		real, imag,
		amp, phase;
 float		a, b;
 register int		i;

 for ( i = 0; i <= N2; i++ ) {
   imag = phase = ( real = amp = Bins[i]<<1 ) + 1;
   a = ( Bins[i] == N2 ? S[1] : S[real] );
   b = ( Bins[i] == 0 || Bins[i] == N2 ? 0. : S[imag] );
   C[amp] = hypot( a, b );
   C[phase] = -atan2( b, a );
 }
}

leanconvert3_disarray( float S[], float C[], int N2, int D, int R, int Bins[] )
//float S[], C[]; int N2, D, R, Bins[] ; 
{

 int		real, imag,
		amp, phase;
 float		a, b;
 register int		i;

 for ( i = 0; i <= N2; i++ ) {
   imag = phase = ( real = amp = Bins[i]<<1 ) + 1;
   a = ( Bins[i] == N2 ? S[1] : S[real] );
   b = ( Bins[i] == 0 || Bins[i] == N2 ? 0. : S[imag] );
   C[amp] = hypot( a, b );
   C[phase] = -atan2( b, a );
 }
}

