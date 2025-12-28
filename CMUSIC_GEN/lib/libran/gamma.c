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
