#include <stdio.h>
#include <math.h>
#include <carl/carl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>


/* cc gen3.c -lfrm -lm -o gen3 */

# define SIZE 1024

int main(argc, argv) int 
	argc;
	char           *argv[]; {
	float           tcoef[SIZE], vcoef[SIZE], factor, *f, *ff, scale = 0.0;
	float           expr();
	extern int      exprerr;
	int             i, j, seglen, length, nc, closed = 1;
	double		atof(); 

	if (argc < 4 || *argv[1] != '-' || *(argv[1] + 1) != 'L') {
		fprintf(stderr, "Usage: gen3 -Llength v1 v2 ... vN\n");
		exit(-1);
	}
	length = atoi(argv[1] + 2);
	if (length < 1) {
		fprintf(stderr, "GEN3: Illegal length expression\n");
		exit(-1);
	}
	j = 2;
	if (!strcmp(argv[2], "-o")) {
		closed = 0;
		j = 3;
	}
	if (*argv[2] == '-' && !(index("0123456789.", *(argv[2] + 1))))
		j = 3;

	for (nc = 0; j < argc; nc++) {
		vcoef[nc] = atof(argv[j++]);
//		if (nc > 0) {
//			fprintf(stderr, "gen3: expression error: %s\n", argv[j - 1]);
//			exit(1);
//		}
		if (fabs(vcoef[nc]) > scale)
			scale = fabs(vcoef[nc]);
	}
	for (i = 0; i < nc; i++)
		tcoef[i] = (float) i *(length - closed) / (nc - 1);

	for (
	     ff = f = (float *) malloc(length * sizeof(float)), i = 0;
	     i < nc - 1;
	     f += seglen - 1, i++) {
		seglen = floor(tcoef[i + 1] +.5) - floor(tcoef[i] +.5) + 1;
		trans(vcoef[i], 0., vcoef[i + 1], seglen, f);
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
