h19852
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

double lin(rfun, g)
	double (*rfun)();
	double g;
{
	register double rtn; 

	rtn = g * (1.0 - sqrt((*rfun)(0.0, 1.0)));

	return(rtn);
	}
E 1
