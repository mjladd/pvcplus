#include <stdio.h>
#include <math.h>
#include <carl/carl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>


#define TWOPI (8.*atan(1.))

/* cc gen2.c -lfrm -lm -o gen2 */

# define SIZE 1024

int main(argc, argv) int 
	argc;
	char           *argv[]; {
	float           coef[SIZE], factor, *f, *ff, scale = 0.0;
	float           expr();
	extern int      exprerr;
	int             i, j, seglen, length, na, nc, closed = 0;
	double 		atof(); 

	if (argc < 4 || *argv[1] != '-' || *(argv[1] + 1) != 'L') {
		fprintf(stderr,
			"Usage: gen2 -Llength [-o (default) or -c] a1 ... aN b0 ... bM N\n");
		exit(-1);
	}
	length = atoi(argv[1] + 2);
	if (length < 1) {
		fprintf(stderr, "GEN2: Illegal length expression\n");
		exit(-1);
	}
	i = 2;
	if (!strcmp(argv[2], "-c")) {
		closed = 1;
		i = 3;
	}
	if (*argv[2] == '-' && !(index("0123456789.", *(argv[2] + 1))))
		i = 3;
	factor = TWOPI / (length - closed);
	for (nc = 0; i < argc - 1; i++, nc++)
		coef[nc] = atof(argv[i]);
	na = atof(argv[argc - 1]);
	ff = f = (float *) calloc(length, sizeof(float));
	for (i = 0; i < na; i++)
		for (f = ff, j = 0; j < length; j++, f++)
			*f += coef[i] * sin(factor * j * (i + 1));

	for (i = na; i < nc; i++)
		for (f = ff, j = 0; j < length; j++, f++)
			*f += coef[i] * cos(factor * j * (i - na));

	for (scale = 0.0, f = ff, j = 0; j < length; j++, f++)
		if (fabs(*f) > scale)
			scale = fabs(*f);
	if (scale == 0.0) {
		fprintf(stderr, "GEN2: all-zero function\n");
		exit(-1);
	}
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
