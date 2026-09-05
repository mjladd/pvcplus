#include <stdio.h>

void stripheader(fp, SIZ0_HEADER)
int SIZ0_HEADER ;
FILE *fp;
{
  register int i;
  for ( i=0; i < SIZ0_HEADER; i++ )
    fgetc(fp);
}
