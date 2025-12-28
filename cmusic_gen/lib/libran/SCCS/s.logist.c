h23197
s 00012/00000/00000
d D 1.1 94/11/10 12:47:47 loomis 1 0
c date and time created 94/11/10 12:47:47 by loomis
e
u
U
f e 0
t
T
I 1
#include <math.h>

double logist(rfun, alpha, beta)
	double (*rfun)();
	double alpha, beta;
{
	register double rtn, x; 

	rtn = (-beta - log(1.0/(*rfun)(0.0,1.0) - 1.0)) / alpha;

	return(rtn);
	}
E 1
