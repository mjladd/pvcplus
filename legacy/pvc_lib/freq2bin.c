#include <stdio.h>
#include "pv.h"


freq2bin( freq, N, R ) float freq; int N,R ;
{
static int first = 1;
static float *freqs;
float funda, matchfreq;
int i;
int bin;
static int N2  ;
float closest2();
if( first){
  N2 = N>>1;
  first = 0;
//  freqs = (float *) space( N2, sizeof(float));
  fvec( freqs, N2 ) ; 
  funda = (float)(R/N);
  for( i = 0; i< N2; i++ ){
     freqs[i] = funda * (float)i ;
  }
}
matchfreq =  closest2( freq, freqs, N2) ;
fprintf(stderr,"%f matched by %f\n",freq,matchfreq);
for( i = 0; i < N2; i++ ){
  if( freqs[i] == matchfreq ){
     bin = i;
     break;
  }
}
return( bin );
}

float closest2( val, arr, len ) float val, *arr; int len;
{
  int i;
  float min = 10000000000.;
  float diff, outval;
  for( i = 0; i < len; i++ ){
     diff = fabs( val - arr[i] );
     if( min > diff ){
        min = diff;
         outval = arr[i];
     }
  }
  return( outval );
}

