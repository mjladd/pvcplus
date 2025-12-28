float getthresh( arr, Nplus2, tgen ) float *arr, tgen; int Nplus2;
{
int i;
float max = 0.;
   for( i = 2; i < Nplus2 ; i += 2 )
      if( max < arr[i] )
         max = arr[i] ;
   return( max * tgen );
}
