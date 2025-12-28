h42269
s 00001/00001/00017
d D 1.2 86/09/17 23:21:14 root 2 1
c switched to random().
e
s 00018/00000/00000
d D 1.1 84/08/11 11:55:52 root 1 0
c original distributed version
e
u
U
f i 
t
T
I 1
/* %M%	%I%	(CARL)	%G%	%U% */

#include <math.h>

/* 
 * frand - return a double precision floating point random number scaled
 * to be in the range [lb,ub].
 */

double divisor;

double frand(lb, ub)
	double lb, ub;
{
	if (divisor == 0) 
		divisor = pow(2.0, (double) ((sizeof(int) * 8) - 1)) - 1.0;
D 2
	return((ub - lb) * (rand() / divisor) + lb);
E 2
I 2
	return((ub - lb) * (random() / divisor) + lb);
E 2
	}
E 1
