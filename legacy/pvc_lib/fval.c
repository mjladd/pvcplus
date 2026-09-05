#include <stdio.h>
#include <math.h>
#include "pv.h"



 
float fval( struct func *p, float dur, float  T )
{
   
    float z,   v ;
    int i, ivlow,  ivhi;
     
 
if( p->n <= 1.){
// ONE VALUE, SKIP OUT
     return( p->A[ 0 ] ) ; 
}else{
   // MULTIPLE VALUES


  if( p->L == 0. ){
      // ASCII FILE

	fseek( p->fp, 0, SEEK_SET ) ;

	if( T < 0. ){
	    // BEGINING
	    v = 0. ; // VIRTUAL FUNCTION INDEX
	    ivlow = (int) v ; // LOWER FUNCTION VALUE
	    ivhi = ivlow + 1 ; // UPPER FUNCTION VALUE
	    z = 0. ; // FRACTIONAL POINT BETWEEN

	}else if( T > dur ){
	    // END
	    v = (float) (p->n - 2) ; // VIRTUAL FUNCTION INDEX
	    ivlow = (int) v ; // LOWER FUNCTION VALUE
	    ivhi = ivlow + 1 ; // UPPER FUNCTION VALUE
	    z = 1. ; // FRACTIONAL POINT BETWEEN

	}else{
	    // MIDDLE
	    v = (T / dur) * (float) (p->n - 1) ; // VIRTUAL FUNCTION INDEX
	    ivlow = (int) v ; // LOWER FUNCTION VALUE
	    ivhi = ivlow + 1 ; // UPPER FUNCTION VALUE
	    z = v - (float) ivlow ; // FRACTIONAL POINT BETWEEN
	    
	}
	

	// READ UNTIL YOU FIND THE LOWER  VALUE
	for( i = 0; i < ivhi ; i++ ) fscanf( p->fp,  " %f ",  &p->A[ 1 ] ) ; 
	// READ IN THE HIGHER VALUE
	fscanf( p->fp,  " %f ",  &p->A[ 2 ] ) ; 
	
	p->A[ 0 ] = p->A[ 1 ] + z * (p->A[ 2 ] - p->A[ 1 ] ) ; // INTERPOLATED VALUE BETWEEN



  }else{

      // FLOAT FILE
      
	if( T <= 0. ){
	    // BEGINING
	    fseek( p->fp, 0, SEEK_SET ) ;
	    fread( &p->A[ 0 ], sizeof(float), 1, p->fp ) ;
	}else if( T >= dur ){
	    // END
//	    fprintf( stderr,  "\nHERE" ) ; 
	    fseek( p->fp, (p->n - 1) * sizeof(float), SEEK_SET ) ;
	    fread( &p->A[ 0 ], sizeof(float), 1, p->fp ) ;
	}else{
	    // MIDDLE
	    v = (T / dur) * (float) (p->n - 1) ; // VIRTUAL FUNCTION INDEX
	    ivlow = (int) v ; // LOWER FUNCTION VALUE
	    ivhi = ivlow + 1 ; // UPPER FUNCTION VALUE
	    z = v - (float) ivlow ; // FRACTIONAL POINT BETWEEN

	    fseek( p->fp,ivlow * sizeof(float), SEEK_SET ) ;
	    fread( &p->A[ 1 ], sizeof(float), 2, p->fp ) ; // LOWER  AND UPPER VALUES
	
	    p->A[ 0 ] = p->A[ 1 ] + z * (p->A[ 2 ] - p->A[ 1 ] ) ; // INTERPOLATED VALUE BETWEEN
	}
    
  }

}

    return( p->A[ 0 ] ) ;	


}


