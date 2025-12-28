#include "pv.h"

/* unconvert essentially undoes what convert does, i.e., it
  turns N2+1 PAIRS of amplitude and frequency values in
  C into N2 PAIR of complex spectrum data (in rfft format)
  in output array S; sampling rate R and interpolation factor
  I are used to recompute phase values from frequencies */

leanunconvert1_disarray( float C[], float S[], int N2, int I, int R, float Bins[] )
//float C[], S[]; int N2, I, R, Bins[] ; 
{

  int		real, imag,
		amp, phase;
  float		a, b;
  register int		i;
  
  for ( i = 0; i <= N2; i++ ) {
    imag = phase = ( real = amp = Bins[i]<<1 ) + 1;

// NEXT ADDED 2 LINES ARE JG FIX
	if ( Bins[i] == N2 )
	    real = 1;



    S[real] = *(C+amp) * cos( *(C+phase) );
    if ( Bins[i] != N2 )
      S[imag] = -*(C+amp) * sin( *(C+phase) );
  }
}


leanunconvert2_disarray( float C[], float S[], int N2, int I, int R, float Bins[] )
//float C[], S[]; int N2, I, R, Bins[] ; 
{

  int		real, imag,
		amp, phase;
  float		a, b;
  register int		i;
  
  for ( i = 0; i <= N2; i++ ) {
    imag = phase = ( real = amp = Bins[i]<<1 ) + 1;

// NEXT ADDED 2 LINES ARE JG FIX
	if ( Bins[i] == N2 )
	    real = 1;



    S[real] = *(C+amp) * cos( *(C+phase) );
    if ( Bins[i] != N2 )
      S[imag] = -*(C+amp) * sin( *(C+phase) );
  }
}


leanunconvert3_disarray( float C[], float S[], int N2, int I, int R, float Bins[] )
//float C[], S[]; int N2, I, R, Bins[] ; 
{

  int		real, imag,
		amp, phase;
  float		a, b;
  register int		i;
  
  for ( i = 0; i <= N2; i++ ) {
    imag = phase = ( real = amp = Bins[i]<<1 ) + 1;

// NEXT ADDED 2 LINES ARE JG FIX
	if ( Bins[i] == N2 )
	    real = 1;



    S[real] = *(C+amp) * cos( *(C+phase) );
    if ( Bins[i] != N2 )
      S[imag] = -*(C+amp) * sin( *(C+phase) );
  }
}


