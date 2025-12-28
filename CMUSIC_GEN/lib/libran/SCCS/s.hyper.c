h28112
s 00018/00000/00000
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

double Pi2;

double hyper(rfun, tau, xmu)
	double (*rfun)();
	double tau, xmu;
{
	static int first=1;
	double u, rtn;

	if (first) { first=0; Pi2 = 2.0 * atan(1.0); }

	u = (*rfun)(0.0, 1.0) * Pi2;
	rtn = tau * log(tan(u)) + xmu;

	return(rtn);
	}
E 1
