h27711
s 00020/00000/00000
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

double beta(rfun, a, b)
	double (*rfun)();
	double a, b;
{
	double ea=1.0/a, eb=1.0/b, y1, y2, s;
	register double rtn; 

	do
		{
		y1 = pow((*rfun)(0.0, 1.0), ea);
		y2 = pow((*rfun)(0.0, 1.0), eb);
		s = y1 + y2;
		}
	while (s > 1.0);
	rtn = y1/s;

	return(rtn);
	}
E 1
