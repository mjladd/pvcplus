h46267
s 00027/00000/00000
d D 1.1 94/11/10 12:47:48 loomis 1 0
c date and time created 94/11/10 12:47:48 by loomis
e
u
U
f e 0
t
T
I 1
/*
 * randfi - linearly interpolating random function
 * 
 * Produced at frequency freq of sampling rate sRate.
 */

#include <math.h>

double randfi(rfun, sRate, freq)
	double (*rfun)();
	double sRate, freq;
{
	register double rtn, frac;
	static long cnt;
	static double oldVal, diff;

	frac = cnt/(sRate/freq);
	if (frac >= 1.0)
		cnt = frac = 0;
	if (cnt == 0) {
		oldVal = oldVal + diff;
		diff = (*rfun)(-1.0, 1.0) - oldVal;
	}
	cnt++;
	rtn = oldVal + diff * frac;
	return(rtn);
}
E 1
