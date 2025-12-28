h20808
s 00012/00000/00000
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

double expn(rfun, delta)
	double (*rfun)();
	double delta;
{
	register double rtn; 

	rtn = -log((*rfun)(0.0, 1.0)) / delta;

	return(rtn);
	}
E 1
