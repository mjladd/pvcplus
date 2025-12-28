norman( float *arr, float omin, float omax, int len ) 
//float *arr, omin, omax; int len;
{
int i;
float mapp();
float imin=9999999999., imax=-9999999999.;
   for(i = 0; i < len ; i++){
	if( imin > arr[i] ) 
	   imin = arr[i];
	if( imax < arr[i] ) 
	   imax = arr[i];
	}
   for(i = 0; i < len; i++ )
	arr[i] = mapp(arr[i], imin, imax, omin, omax);
}
