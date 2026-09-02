#include "pv.h"

void convert1_disarray( float S[], float C[], int N2, int D, int R, int Bins[], float fundamental )
//float S[], C[]; int N2, D, R, Bins[]; float fundamental ;  
{
  static int 	first = 1;
  static float 	*lastphase,
//		fundamental,
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
//      fundamental = (float) R/(N2<<1);
      factor = R/(D*TWOPI);
    } 

    if( frame_count == 0 )
	for(i = 0; i < (N2 + 1); i++ )lastphase[i] = 0. ; 
	
/* unravel rfft-format spectrum: note that N2+1 pairs of
   values are produced */

    for ( i = 0; i <= N2; i++ ) {
      imag = freq = ( real = amp = Bins[i]<<1 ) + 1;
      a = ( Bins[i] == N2 ? S[1] : S[real] );
      b = ( Bins[i] == 0 || Bins[i] == N2 ? 0. : S[imag] );

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

	while ( phasediff > PI )
	  phasediff -= TWOPI;
	while ( phasediff < -PI )
	  phasediff += TWOPI;
      }

/* convert each phase difference to Hz */

      C[freq] = phasediff*factor + Bins[i]*fundamental;
    }
}


void convert2_disarray( float S[], float C[], int N2, int D, int R, int Bins[], float fundamental )
//float S[], C[]; int N2, D, R, Bins[];  float fundamental ;  
{
  static int 	first = 1;
  static float 	*lastphase,
//		fundamental,
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
//      fundamental = (float) R/(N2<<1);
      factor = R/(D*TWOPI);
    } 

    if( frame_count == 0 )
	for(i = 0; i < (N2 + 1); i++ )lastphase[i] = 0. ; 
	
/* unravel rfft-format spectrum: note that N2+1 pairs of
   values are produced */

    for ( i = 0; i <= N2; i++ ) {
      imag = freq = ( real = amp = Bins[i]<<1 ) + 1;
      a = ( Bins[i] == N2 ? S[1] : S[real] );
      b = ( Bins[i] == 0 || Bins[i] == N2 ? 0. : S[imag] );

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

	while ( phasediff > PI )
	  phasediff -= TWOPI;
	while ( phasediff < -PI )
	  phasediff += TWOPI;
      }

/* convert each phase difference to Hz */

      C[freq] = phasediff*factor + Bins[i]*fundamental;
    }
}



void convert3_disarray(  float S[], float C[], int N2, int D, int R, int Bins[], float fundamental )
//float S[], C[]; int N2, D, R, Bins[]; float fundamental ;  
{
  static int 	first = 1;
  static float 	*lastphase,
//		fundamental,
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
//      fundamental = (float) R/(N2<<1);
      factor = R/(D*TWOPI);
    } 

    if( frame_count == 0 )
	for(i = 0; i < (N2 + 1); i++ )lastphase[i] = 0. ; 
	
/* unravel rfft-format spectrum: note that N2+1 pairs of
   values are produced */

    for ( i = 0; i <= N2; i++ ) {
      imag = freq = ( real = amp = Bins[i]<<1 ) + 1;
      a = ( Bins[i] == N2 ? S[1] : S[real] );
      b = ( Bins[i] == 0 || Bins[i] == N2 ? 0. : S[imag] );

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

	while ( phasediff > PI )
	  phasediff -= TWOPI;
	while ( phasediff < -PI )
	  phasediff += TWOPI;
      }

/* convert each phase difference to Hz */

      C[freq] = phasediff*factor + Bins[i]*fundamental;
    }
}

