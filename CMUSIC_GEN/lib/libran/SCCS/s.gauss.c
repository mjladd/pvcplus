h25925
s 00016/00000/00000
d D 1.1 94/11/10 12:47:46 loomis 1 0
c date and time created 94/11/10 12:47:46 by loomis
e
u
U
f e 0
t
T
I 1
#include <math.h>

double gauss(rfun, sigma, xmu)
	double (*rfun)();
	double xmu, sigma;
{
	register int i;
	register double s = 0.0, rtn; 

	for (i = 0; i < 12; i++) 
		s += (*rfun)(0.0, 1.0);
	rtn = sigma*(s - 6.0) + xmu;

	return(rtn);
	}

E 1
