h24961
s 00015/00000/00000
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

double gamma(rfun, nu)
	double (*rfun)();
	double nu;
{
	register int x, n;
	register double sum = 1.0, rtn; 

	for (x = 0, n = nu; x < n; x++)
		sum *= (*rfun)(0.0, 1.0);
	rtn = -log(sum);

	return(rtn);
	}
E 1
