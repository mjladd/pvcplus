h29240
s 00020/00000/00000
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

double Pi;

double cauchy(rfun, tau,iopt)
	double (*rfun)();
	double tau, iopt;
{
	static int first=1;
	double u, rtn;

	if (first) { first=0; Pi = 4.0 * atan(1.0); }

	u = (*rfun)(0.0, 1.0);
	if (iopt == 1.0) u /= 2.0;
	u *= Pi;
	rtn = tau * tan(u);

	return(rtn);
	}
E 1
