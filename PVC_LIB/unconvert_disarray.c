#include "pv.h"

unconvert1_disarray( C, S, N2, I, R, Bins, fundamental )
float C[], S[]; int N2, I, R, Bins[]; float fundamental ; 
{
  static int 	first = 1;
  static float 	*lastphase,
//		fundamental,
		factor;
  int 		i,
		real,
		imag,
		amp,
		freq;
  float 	mag,
		phase;
  char		*space();

/* first pass: allocate memory and compute constants */

    if ( first ) {
	first = 0;
	lastphase = (float *) space( N2+1, sizeof(float) );
//	fundamental = (float) R/(N2<<1);
        factor = TWOPI*I/R;
    } 

    if( frame_count == 0 )
	for(i = 0; i < (N2 + 1); i++ )lastphase[i] = 0 ; 
	

/* subtract out frequencies associated with each channel,
   compute phases in terms of radians per I samples, and
   convert to complex form */

    for ( i = 0; i <= N2; i++ ) {
	imag = freq = ( real = amp = Bins[i]<<1 ) + 1;
	if ( Bins[i] == N2 )
	    real = 1;
	mag = C[amp];
	lastphase[i] += C[freq] - Bins[i]*fundamental;
	phase = lastphase[i]*factor;
	S[real] = mag*cos( phase );
	if ( Bins[i] != N2 )
	    S[imag] = -mag*sin( phase );
    }
}

unconvert2_disarray( C, S, N2, I, R, Bins, fundamental )
float C[], S[]; int N2, I, R, Bins[]; float fundamental ; 
{
  static int 	first = 1;
  static float 	*lastphase,
//		fundamental,
		factor;
  int 		i,
		real,
		imag,
		amp,
		freq;
  float 	mag,
		phase;
  char		*space();

/* first pass: allocate memory and compute constants */

    if ( first ) {
	first = 0;
	lastphase = (float *) space( N2+1, sizeof(float) );
//	fundamental = (float) R/(N2<<1);
        factor = TWOPI*I/R;
    } 

    if( frame_count == 0 )
	for(i = 0; i < (N2 + 1); i++ )lastphase[i] = 0 ; 
	

/* subtract out frequencies associated with each channel,
   compute phases in terms of radians per I samples, and
   convert to complex form */

    for ( i = 0; i <= N2; i++ ) {
	imag = freq = ( real = amp = Bins[i]<<1 ) + 1;
	if ( Bins[i] == N2 )
	    real = 1;
	mag = C[amp];
	lastphase[i] += C[freq] - Bins[i]*fundamental;
	phase = lastphase[i]*factor;
	S[real] = mag*cos( phase );
	if ( Bins[i] != N2 )
	    S[imag] = -mag*sin( phase );
    }
}

unconvert3_disarray( C, S, N2, I, R, Bins, fundamental )
float C[], S[]; int N2, I, R, Bins[]; float fundamental ; 
{
  static int 	first = 1;
  static float 	*lastphase,
//		fundamental,
		factor;
  int 		i,
		real,
		imag,
		amp,
		freq;
  float 	mag,
		phase;
  char		*space();

/* first pass: allocate memory and compute constants */

    if ( first ) {
	first = 0;
	lastphase = (float *) space( N2+1, sizeof(float) );
//	fundamental = (float) R/(N2<<1);
        factor = TWOPI*I/R;
    } 

    if( frame_count == 0 )
	for(i = 0; i < (N2 + 1); i++ )lastphase[i] = 0 ; 
	

/* subtract out frequencies associated with each channel,
   compute phases in terms of radians per I samples, and
   convert to complex form */

    for ( i = 0; i <= N2; i++ ) {
	imag = freq = ( real = amp = Bins[i]<<1 ) + 1;
	if ( Bins[i] == N2 )
	    real = 1;
	mag = C[amp];
	lastphase[i] += C[freq] - Bins[i]*fundamental;
	phase = lastphase[i]*factor;
	S[real] = mag*cos( phase );
	if ( Bins[i] != N2 )
	    S[imag] = -mag*sin( phase );
    }
}
