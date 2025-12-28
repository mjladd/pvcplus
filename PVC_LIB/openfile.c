#include <stdio.h>
FILE * openfile( fname, mode ) const char *fname, *mode ; 
{
FILE *fp, *fopen();
if( (fp = fopen(fname, mode)) == NULL) {
  fprintf(stderr,"could not open %s\n",fname);
  exit(-1);
}
return( fp );
}

scoremsg(fname) char fname[];
{
   fprintf(stderr,"SCORE WRITTEN TO %s\n",fname);
}
