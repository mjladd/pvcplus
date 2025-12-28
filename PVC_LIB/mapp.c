#include <stdio.h>
#include "pv.h"

float mapp( in, imin, imax, omin, omax ) float in, imin, imax, omin, omax;
{
  if( imax == 0.0 )
  {
     fprintf(stderr,"mapp: maxvalue of 0.0 not allowed, exiting\n");
     exit(-1);
  }
  return( omin+((omax-omin)*((in-imin)/(imax-imin))) );
}

/* RETURNS A ZERO FLOOR */
float lmapp( in, imin, imax, omax ) float in, imin, imax, omax;
{
float omin = 0.0;
  if( imax == 0.0 )
  {
     fprintf(stderr,"mapp: maxvalue of 0.0 not allowed, exiting\n");
     exit(-1);
  }
   return( (omax*((in-imin)/(imax-imin))) );
}

/* RETURNS BETWEEN 1.0 AND omax ,
   REQUIRES in BETWEEN 0.0-1.0
*/
float zmapp( in, omax ) float in, omax;
{
   if( in < 0.0 || in > 1.0 ){
      fprintf(stderr,"zmapp: warning map value out of expected 0-1 range\n");
   }
   return( 1. + ((omax-1.0) * in) );
}

