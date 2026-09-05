#include "pv.h"

/* oscillator bank resynthesizer for phase vocoder analyzer
  uses sum of N+1 cosinusoidal table lookup oscillators to 
  compute I (interpolation factor) samples of output O
  from N+1 amplitude and frequency value-pairs in C;
  frequencies are scaled by P */

void noscbank_extra( float C[], int N, int R, int Nw, int I, float P, float O[] )
//float C[], O[], P; int N, Nw, R, I;
{

  static int 	NP,
		L = 8192,
		first = 1;
  static float	Iinv,
		*lastamp,
		*lastfreq,
		*index,
		*table;
  static float	Pinc,
		ffac;
  int 		amp,
		freq,
		n,
		chan;
  

/* first pass: allocate memory to hold previous values
   of amplitude and frequency for each channel, the table
   index for each oscillator, and the table itself; also
   compute constants */

    if ( first ) {
      float 	TWOPIoL = TWOPI/L,
      		tabscale;

	first = 0;
        fprintf(stderr,"\n.....USING OSCILLATOR BANK RESYNTHESIS\n");
	    oscilbankon = 1 ; 
	lastamp = (float *) space( N+1, sizeof(float) );
	lastfreq = (float *) space( N+1, sizeof(float) );
	index = (float *) space( N+1, sizeof(float) );
	table = (float *) space( L, sizeof(float) );

	tabscale =  Nw >= N ? N : 8*N;

	for ( n = 0; n < L; n++ )
	    table[n] = tabscale*cos( TWOPIoL*n );

	Iinv = 1./I;
	Pinc = P*L/R;
	ffac = P*PI/N;

	if ( P > 1. )
	    NP = N/P;
	else
	    NP = N;
    }


    if( frame_count == 0 )
	for(n = 0; n < (N + 1); n++ ){
	    lastamp[n] = lastfreq[n] = index[n] = 0. ; 
	}


/* for each channel, compute I samples using linear
   interpolation on the amplitude and frequency
   control values */

    for ( chan = 0; chan < NP; chan++ ) {

      register float 	a,
			ainc,
			f,
			finc,
			address;

	freq = ( amp = ( chan << 1 ) ) + 1;

	if ( C[amp] < synt ) /* skip the little ones */
	    continue;

	C[freq] *= Pinc;
	finc = ( C[freq] - ( f = lastfreq[chan] ) )*Iinv;
	ainc = ( C[amp] - ( a = lastamp[chan] ) )*Iinv;
	address = index[chan];

/* accumulate the I samples from each oscillator into
   output array O (initially assumed to be zero);
   f is frequency in Hz scaled by oscillator increment
   factor and pitch (Pinc); a is amplitude; */

	for ( n = 0; n < I; n++ ) {
	    O[n] += a*table[ (int) address ];

	    address += f;

	    while ( address >= L )
		address -= L;

	    while ( address < 0 )
		address += L;

	    a += ainc;
	    f += finc;
	} 

/* save current values for next iteration */

	lastfreq[chan] = C[freq];
	lastamp[chan] = C[amp];
	index[chan] = address;
    }
}

