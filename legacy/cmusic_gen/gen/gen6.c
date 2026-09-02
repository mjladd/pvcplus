#include <stdio.h>
#include <carl/carl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>


/* cc gen6.c -lfrm -lm -o gen6 */

int main(argc, argv) int 
	argc;
	char           *argv[]; {
	float          *f, *ff;
	int             i, length;

	if (argc < 2 || *argv[1] != '-' || *(argv[1] + 1) != 'L') {
		fprintf(stderr, "Usage: gen6 -Llength\n");
		exit(-1);
	}
	length = atoi(argv[1] + 2);
	if (length < 1) {
		fprintf(stderr, "GEN6: Illegal length expression\n");
		exit(-1);
	}
	for (ff = f = (float *) malloc(length * sizeof(float)), 
		i = 0; i < length; i++)
		    *f++ = 2.0 * ((float) rand() / (float) 0x7fffffff) - 1.0;
	if (isatty(1)) {
		for (i = 0; i < length; i++)
			printf("%f\n", *(ff + i));
	} else {
		for (i = 0; i < length; i++)
			putfloat((ff + i));
	}
	flushfloat();
	exit(0);		/* Return status of 0 if all went OK */
}
