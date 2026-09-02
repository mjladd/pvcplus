#include <stdio.h>
#include <math.h>
#include <carl/carl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>


/* cc gen4.c  -lm -o gen4 */

/*
 *  #include <carl/carl.h>

 */

# define SIZE 1024

int main(argc, argv) int
	argc;
	char           *argv[]; {
	float           tcoef[SIZE], vcoef[SIZE], alpha[SIZE], factor, *f, *ff, scale = 0.0;
	float           expr();
	extern int      exprerr;
	int             i, j, seglen, length, nc = 0, closed = 1;
	double		atof() ; 

	if (argc < 7 || *argv[1] != '-' || *(argv[1] + 1) != 'L') {
		fprintf(stderr, "Usage: gen4 -Llength t1 v1 a1 ... tN vN\n");
		exit(-1);
	}
	length = atoi(argv[1] + 2);
	if (length < 1) {
		fprintf(stderr, "GEN4: Illegal length expression\n");
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
		tcoef[nc] = atof(argv[j++]);
//		if (exprerr) {
//			fprintf(stderr, "GEN4: Illegal expression\n");
//			exit(-1);
//		}
		if (nc > 0) {
			for (i = 0; i < nc; i++)
				if (tcoef[nc] <= tcoef[i]) {
					fprintf(stderr, 
					    "GEN4: Non-increasing T values\n");
					exit(-1);
				}
		}
		vcoef[nc] = atof(argv[j++]);
		if (j < argc)
			alpha[nc] = atof(argv[j++]);
//		if (exprerr) {
//			fprintf(stderr, "GEN4: Illegal expression\n");
//			exit(-1);
//		}
		if (fabs(vcoef[nc]) > scale)
			scale = fabs(vcoef[nc]);
	}

	for (i = 0; i < nc; i++)
		tcoef[i] *= (float) (length - closed) / tcoef[nc - 1];
//	if (scale == 0.0) {
//		fprintf(stderr, "GEN4: all-zero function\n");
//		exit(-1);
//	}
	for (
	     ff = f = (float *) malloc(length * sizeof(float)), i = 0;
	     i < nc - 1;
	     f += seglen - 1, i++) {
		seglen = floor(tcoef[i + 1] +.5) - floor(tcoef[i] +.5) + 1;
		trans(vcoef[i], alpha[i], vcoef[i + 1], seglen, f);
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
