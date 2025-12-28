#include "pv.h"

/* Lyon Hack of Penrose Hack of FRM oscillator bank */

voscbank( C, N, R, Nw, I, P, O )
float C[],  O[], P; int N, Nw,  R, I;
{
  static int 	NP,
		L = 8192,
		first = 1;
  static float 	Iinv,
		*lastamp,
		*lastfreq,
		*index,
		*table;
  static float 	Pinc,
		ffac;
  int 		amp,
		freq,
		n,
		chan;
  float 	a0;
  char		*space();


    if ( first ) {
      
      float TWOPIoL = TWOPI/L, tabscale;
	
      first = 0;
      lastamp = (float *) space( N+1, sizeof(float) );
      lastfreq = (float *) space( N+1, sizeof(float) );
      index = (float *) space( N+1, sizeof(float) );
      table = (float *) space( L, sizeof(float) );
      tabscale =  ( Nw >= N ? N : 8*N );
     
      for ( n = 0; n < L; n++ )
	table[n] = tabscale*cos( TWOPIoL*n );
      
      Iinv = 1./I;

    }


    Pinc = P*L/R;
    ffac = P*PI/N;
    if ( P > 1. )
      NP = N/P;
    else
	NP = N;

    for ( chan =  0; chan < NP; chan++ ) {
      
      register float a, ainc, f, finc, address;
	
      freq = ( amp = ( chan << 1 ) ) + 1;
      if ( C[amp] < synt ) /* skip the little ones */
	continue;
/* if( chan == 9 )
   fprintf(stderr,"fOLD %f ",C[freq]); */
      C[freq] *= Pinc;
 /* if( chan == 9 )
    fprintf(stderr,"fNEW %f\n",C[freq]); */
      finc = ( C[freq] - ( f = lastfreq[chan] ) )*Iinv;
      ainc = ( C[amp] - ( a = lastamp[chan] ) )*Iinv;
      address = index[chan];


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
      lastfreq[chan] = C[freq];
      lastamp[chan] = C[amp];
      index[chan] = address;
    }
}
