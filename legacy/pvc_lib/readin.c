#include <stdio.h>
readin(fp, array)
FILE *fp; float **array;
{
float value;
char *malloc(), *realloc();
int count;

for( count = 0; ( (fread(&value, sizeof(float), 1, fp)) != NULL); count++ )
   {
   if( count == 0 )
      {
      *array = (float *) malloc( sizeof(float) );
      **array = value;
      }
   else
      {
      *array = (float *) realloc( *array, (count+1)*sizeof(float));
      *(*array + count) = value;
      }
   }
   return(count);
}
readerr( fname ) char fname[];
{
  fprintf(stderr,"could not open %s\n",fname); 
}

