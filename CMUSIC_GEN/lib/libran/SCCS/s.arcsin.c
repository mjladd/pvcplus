h25166
s 00016/00000/00000
d D 1.1 94/11/10 12:47:45 loomis 1 0
c date and time created 94/11/10 12:47:45 by loomis
e
u
U
f e 0
t
T
I 1
#include <math.h>
double Pi2;

double arcsin(rfunc)
	double (*rfunc)();
{
	static int first=1;
	double rtn;

	if (first) { first=0; Pi2 = 2.0 * atan(1.0); }

	rtn = sin(Pi2 * (*rfunc)(0.0, 1.0));
	rtn *= rtn;

	return(rtn);
	}
E 1
