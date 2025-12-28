#include <stdio.h>
#include <carl/defaults.h>
#include <carl/carl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

double (*randfunc)() = frand;

/*

#include <carl/frm.h>

*/

/*

#include <carl/libran.h>

*/


/* from crack() */
extern int arg_index;
extern char *arg_option;

/* from corrand() */
extern double cor_factor;

#define FDLOSR	22050.0	/* float slow sampling rate */

struct dist 
	{
	char *distname;
	double  (*distfun)();
	int nargs;
	float arg1, arg2;
	char *usage;
	} 
	distab[] = 
	{
/*	 name,     function  args  a1   a2      arg names, defaults */
	{"arcsin", arcsin, 	0, 0.0, 0.0, 	"no args"},
	{"beta", beta, 		2, 0.25, 0.25, 	"a (0.25) b (0.25)"},
	{"cauchy", cauchy, 	2, 0.01272, 0.0, "tau (0.01272) iopt (0.0)"}, 
	{"exponential", expn, 	1, 2.0, 0.0, 	"delta (2.0)"},
	{"gamma", tgamma, 	1, 2.0, 0.0, 	"nu (2.0)"},
	{"gauss", gauss, 	2, 1.0, 0.0, 	"sigma (1.0) xmu (0.0)"},
	{"hyperbolic", hyper, 	2, 0.48732, 0.0, "tau (0.48732), xmu (0.0"},
	{"linear", lin, 	1, 1.0, 0.0, 	"g 1.0"},
	{"logistic", logist, 	2, 2.2021, 0.0, "alpha (2.2021) beta (0.0)"},
	{"laplace", plapla, 	2, 0.50723, 0.0, "tau (0.50723) xmu (0.0)"},
	{"urand", frand, 	-1, 0.0, 1.0, 	"lb (0.0) ub (1.0)"},
	{"frand", onefrand, 	-1, 0.0, 1.0, 	"lb (0.0) ub (1.0)"},
	{"crand", corrand, 	-1, 0.0, 1.0, 	"lb (0.0) ub (1.0)"},
	{"randfi", randfi, 	2, FDLOSR, 440.0,"slow sRate (FDLOSR)  freq (440.0)"},
	{"randfc",randfc, 	2, FDLOSR, 440.0,"slow sRate (FDLOSR) freq (440.0)"},
	{NULL, NULL, 		0, 0.0, 0.0, NULL}
	};

void cannonfodder() ;
double getf() ;
int lookup(char *name) ;


int main(argc, argv)
	int argc; char **argv;
{
	char crack(), ch;
	int i, j, N=1024, otty = isatty(1);
	double vals[6];
	char *dname;
	int dind;
	double (*dfun)();
	float output;
	double atof(); 

	if (argc == 1)
		cannonfodder();

	while ((ch = crack(argc, argv, "s|S|L|IFochC|")) != CRACK_DONE_FLAG) 
		{
		switch(ch) 
			{
			case 'o':
			case 'c':	
				break;
			case 'I':
				randfunc = getf;
				break;
			case 'S':  	
				if (strlen(arg_option))
//					srandom((int)atoi(arg_option)); 
					srand((int)atoi(arg_option)); 
				else
//					srandom(time(0));
					srand(time(0));
				break;
			case 'L':	
				N = atoi(arg_option); break;
			case 'F':
				randfunc = onefrand;
				break;
			case 'C':
				randfunc = corrand;
				cor_factor = atof(arg_option);
				break;
			case 's':
				f1init(atoi(arg_option));
				break;
			case 'h':
				cannonfodder();
			default: 	
				exit(-1);
			}
		}

	if (arg_index >= argc)
		{
		fprintf(stderr, "cannon: missing function name\n");
		exit(-1);
		}
	else
		{
		dname = argv[arg_index++];
		dind = lookup(dname);
		if (dind < 0)
			{
			fprintf(stderr, "cannon: function not found: %s\n",
				dname);
			exit(-1);
			}
		dfun = distab[dind].distfun;
		vals[0] = distab[dind].arg1;
		vals[1] = distab[dind].arg2;
		}


	for (i = 0, j = arg_index; j < argc; i++, j++)
		{
		vals[i] = atof(argv[j]);
//		if (exprerr) 
//			{
//			fprintf(stderr, "cannon: ill-formed expression: %s\n",
//				argv[j]);
//			exit(1);
//			}
		}



	for (i = 0; i < N; i++)
		{
		switch (distab[dind].nargs) {
			case 0: output = (*dfun)(randfunc); 
				break;
			case 1: output = (*dfun)(randfunc, vals[0]); 
				break;
			case 2: output = (*dfun)(randfunc, vals[0], vals[1]);
				break;
			case -1: 
				output = (*dfun)(vals[0], vals[1]);
			}
		if (otty)
			printf("%f\n",output);
		else
			putfloat(&output);
		}
	if (!otty) flushfloat();
	exit(0);
	}

int lookup(char *name)
//	char *name;
{
	register int i;

	for (i = 0; distab[i].distname != NULL; i++)
		{
		if (!strcmp(name, distab[i].distname))
			return(i);
		}
	return(-1);
}

double getf()
{
	float input;
	if (getfloat(&input) <= 0)
		{
			fprintf(stderr, "cannon: ran out of standard input\n");
			exit(-1);
		}
	return((double)input);
}

void cannonfodder()
{
int i;

fprintf(stderr, "%s%s%s%s%s%s%s%s%s%-12s%-12s",
"usage: cannon [flags] function args...\n",
"flags:\n",
" -LN\t set N output floatsams (1024)\n",
" -S[N]\t set seed for random number generator to N\n",
" \t\t (or to time of day if no N)\n",
" -I\tuse standard input instead of internally generated noise\n",
" -F\tuse 1/f noise instead of white noise\n",
" -CN\tuse correlated noise; N=correlation factor: 0=white, -> 1=brownian\n",
" -sN\tset sequence length for 1/f noise to N\n",
"functions:",
"args:\n");

for (i = 0; distab[i].distname != NULL; i++)
	fprintf(stderr, "\t%-12s%-12s\n", distab[i].distname, distab[i].usage);
}
