#include "pv.h"

/* oscillator bank resynthesizer for phase vocoder analyzer
  uses sum of N+1 cosinusoidal table lookup oscillators to 
  compute I (interpolation factor) samples of output O
  from N+1 amplitude and frequency value-pairs in C;
  frequencies are scaled by P */

void noscbank3( float C1[], int N1, int R, int Nw, int I, float P, float O[], float C2[],  int N2, float C3[], int N3 )
//float C1[], O[], C2[], C3[], P; int N1, Nw, R, I, N2, N3 ;
{

  static int 	NP,
		L = 8192,
		first = 1;
  static float	Iinv,
		*lastamp1,
		*lastfreq1,
		*index1,
		*lastamp2,
		*lastfreq2,
		*index2,
		*lastamp3,
		*lastfreq3,
		*index3,
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
        fprintf(stderr,"\n....USING OSCILLATOR BANK RESYNTHESIS\n");
	    oscilbankon = 1 ;
	lastamp1 = (float *) space( N1+1, sizeof(float) );
	lastfreq1 = (float *) space( N1+1, sizeof(float) );
	index1 = (float *) space( N1+1, sizeof(float) );

	lastamp2 = (float *) space( N2+1, sizeof(float) );
	lastfreq2 = (float *) space( N2+1, sizeof(float) );
	index2 = (float *) space( N2+1, sizeof(float) );


	lastamp3 = (float *) space( N3+1, sizeof(float) );
	lastfreq3 = (float *) space( N3+1, sizeof(float) );
	index3 = (float *) space( N3+1, sizeof(float) );

	table = (float *) space( L, sizeof(float) );
	tabscale =  Nw >= N1 ? N1 : 8*N1;

	for ( n = 0; n < L; n++ )
	    table[n] = tabscale*cos( TWOPIoL*n );

	Iinv = 1./I;
	Pinc = P*L/R;
	ffac = P*PI/N1;

	if ( P > 1. )
	    NP = N1/P;
	else
	    NP = N1;
    }

    // FIRST ARRAY
    if( frame_count == 0 ){

	for(n = 0; n < (N1 + 1); n++ ){
	    lastamp1[n] = lastfreq1[n] = index1[n] = 0. ; 
	} ; 

	for(n = 0; n < (N2+1); n++ ){
	    lastamp2[n] = lastfreq2[n] = index2[n] = 0. ; 
	}

	for(n = 0; n < (N3+1); n++ ){
	    lastamp3[n] = lastfreq3[n] = index3[n] = 0. ; 
	}
    }; 

    // EXTRA ARRAY
// OLD BEFORE John Gibson JG FIX	for(n = 0; n < (N2+1 + 1); n++ ){ 


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

	if ( C1[amp] < synt ) /* skip the little ones */
	    continue;

	C1[freq] *= Pinc;
	finc = ( C1[freq] - ( f = lastfreq1[chan] ) )*Iinv;
	ainc = ( C1[amp] - ( a = lastamp1[chan] ) )*Iinv;
	address = index1[chan];

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

	lastfreq1[chan] = C1[freq];
	lastamp1[chan] = C1[amp];
	index1[chan] = address;
    }

/**********************************/

/*
 * THE SECOND ARRAY
 */
/* for each channel, compute I samples using linear
   interpolation on the amplitude and frequency
   control values */
//fprintf( stderr,  "\nHERE 1");
    for ( chan = 0; chan < N2; chan++ ) {

      register float 	a,
			ainc,
			f,
			finc,
			address;


	freq = ( amp = ( chan << 1 ) ) + 1;

	if ( C2[amp] < synt ) /* skip the little ones */
	    continue;

	C2[freq] *= Pinc;
	finc = ( C2[freq] - ( f = lastfreq2[chan] ) )*Iinv;
	ainc = ( C2[amp] - ( a = lastamp2[chan] ) )*Iinv;
	address = index2[chan];

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

	lastfreq2[chan] = C2[freq];
	lastamp2[chan] = C2[amp];
	index2[chan] = address;
    }
/**********************************/

/**********************************/

/*
 * THE THIRD ARRAY
 */
/* for each channel, compute I samples using linear
   interpolation on the amplitude and frequency
   control values */
//fprintf( stderr,  "\nHERE 1");
    for ( chan = 0; chan < N3; chan++ ) {

      register float 	a,
			ainc,
			f,
			finc,
			address;


	freq = ( amp = ( chan << 1 ) ) + 1;

	if ( C3[amp] < synt ) /* skip the little ones */
	    continue;

	C3[freq] *= Pinc;
	finc = ( C3[freq] - ( f = lastfreq3[chan] ) )*Iinv;
	ainc = ( C3[amp] - ( a = lastamp3[chan] ) )*Iinv;
	address = index3[chan];

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

	lastfreq3[chan] = C3[freq];
	lastamp3[chan] = C3[amp];
	index3[chan] = address;
    }
/**********************************/



}

