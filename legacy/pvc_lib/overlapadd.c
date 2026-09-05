#include "pv.h"
/*
 * input I is a folded spectrum of length N; output O and
 * synthesis window W are of length Nw--overlap-add windowed,
 * unrotated, unfolded input data into output O
 */
void overlapadd( float I[], int N, float W[], float O[], int Nw, int n )
//float I[], W[], O[] ; int N, Nw, n ; 
{
 int i ;
  static int first = 1;	

    if( frame_count == 0 ) first = 1;


  if ( first ) {
  	first = 0;
        fprintf(stderr,"\n.....USING OVERLAP/ADD RESYNTHESIS\n");
    }
    while ( n < 0 )
	n += N ;
    n %= N ;
    for ( i = 0 ; i < Nw ; i++ ) {
	O[i] += I[n]*W[i] ;
	if ( ++n == N )
	    n = 0 ;
    }
}
