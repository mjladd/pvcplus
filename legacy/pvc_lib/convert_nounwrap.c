#include "pv.h"

/* S is a spectrum in rfft format, i.e., it contains N real values
   arranged as real followed by imaginary values, except for first
   two values, which are real parts of 0 and Nyquist frequencies;
   convert first changes these into N/2+1 PAIRS of magnitude and
   phase values to be stored in output array C; the phases are then
   unwrapped and successive phase differences are used to compute
   estimates of the instantaneous frequencies for each phase vocoder
   analysis channel; decimation rate D and sampling rate R are used
   to render these frequency values directly in Hz. */

void convert_nounwrap( float S[], float C[], int N2, int D, int R)
//float S[], C[]; int N2, D, R;
{
  static int 	first = 1;
  static float 	*lastphase,
		fundamental,
		factor;
  float 	phase,
		phasediff;
  int 		real,
		imag,
		amp,
		freq;
  float 	a,
		b;
  int 		i;

/* first pass: allocate zeroed space for previous phase
   values for each channel and compute constants */

    if ( first ) {
      first = 0;
//      lastphase = (float *) space( N2+1, sizeof(float) );
      fvec( lastphase, N2 + 1 ) ; 
      fundamental = (float) R/(N2<<1);
      factor = R/(D*TWOPI);
    } 

    if( frame_count == 0 )
	for(i = 0; i < (N2 + 1); i++ )lastphase[i] = 0. ; 
	
/* unravel rfft-format spectrum: note that N2+1 pairs of
   values are produced */

    for ( i = 0; i <= N2; i++ ) {
      imag = freq = ( real = amp = i<<1 ) + 1;
      a = ( i == N2 ? S[1] : S[real] );
      b = ( i == 0 || i == N2 ? 0. : S[imag] );

/* compute magnitude value from real and imaginary parts */

      C[amp] = hypot( a, b );

/* compute phase value from real and imaginary parts and take
   difference between this and previous value for each channel */

      if ( C[amp] == 0. )
	phasediff = 0.;
      else {
	phasediff = ( phase = -atan2( b, a ) ) - lastphase[i];
	lastphase[i] = phase;
	
/* unwrap phase differences */

//	while ( phasediff > PI )
//	  phasediff -= TWOPI;
//	while ( phasediff < -PI )
//	  phasediff += TWOPI;
      }

/* convert each phase difference to Hz */

      C[freq] = phasediff*factor + i*fundamental;
    }
}

