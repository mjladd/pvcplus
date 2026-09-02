h25975
s 00019/00000/00000
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

double plapla(rfun, tau, xmu)
	double (*rfun)();
	double tau, xmu;
{
	double u, rtn;

	u = (*rfun)(0.0, 1.0) * 2.0;
	if (u > 1.0)
		{
		u = 2.0 - u;
		rtn = (-tau * log(u)) + xmu;
		}
	else
		rtn = (tau * log(u)) + xmu;

	return(rtn);
	}
E 1
