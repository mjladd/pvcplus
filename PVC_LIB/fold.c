/*
 * multiply current input I by window W (both of length Nw);
 * using modulus arithmetic, fold and rotate windowed input
 * into output array O of (FFT) length N according to current
 * input time n
 */
void fold( float I[], float W[], int Nw, float O[], int N, int n )
//    float I[], W[], O[] ; int Nw, N, n ;
{
 
    int i;

    for ( i = 0; i < N; i++ )
	O[i] = 0.;

    while ( n < 0 )
	n += N;
    n %= N;
    for ( i = 0; i < Nw; i++ ) {
	O[n] += I[i]*W[i];
	if ( ++n == N )
	    n = 0;
    }
}

/*
void fold( float input[], Wanal[], buffer[], int Nw, N, in )
//    float input[], Wanal[], buffer[] ; int Nw, N, in ;
{
 
    int i;

    for ( i = 0; i < N; i++ )
	buffer[i] = 0.;

    while ( in < 0 ) in += N;

    in %= N;
    for ( i = 0; i < Nw; i++ ) {
	buffer[in] += input[i] * Wanal[i];
	if ( ++in == N ) in = 0;
    }
}


*/