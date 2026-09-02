#include <stdlib.h>
#include <stdio.h>

#define allc_min 8192

// OLD allc_min 8192

char *space( int sz, int obsz )
//int sz, obsz;
{
    char	*p;

    if ( (sz * obsz) < allc_min ) {
	if ( (p = (char *) malloc( allc_min )) == NULL ) {
	    fprintf(stderr,"No Memory!\n");
	    exit(-1);
	}
    }
    else {
	if ( (p = (char *) malloc( sz * obsz )) == NULL ) {
	    fprintf(stderr,"No Memory!\n");
	    exit(-1);
	}
    }
    return p;
}
