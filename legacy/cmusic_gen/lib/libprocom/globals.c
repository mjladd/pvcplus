/* globals.c	1.4	(CARL)	28 Aug 1995	18:39:51 */

/*
 * Note: with the advent of 4.3BSD, the size of the file descriptor table
 * became variable, and should be determined dynamically by getdtablesize(2).
 * However, rather than perform the moderate upgrade this would require
 * to this code, I have opted for the cheap way out, and derive this
 * directly from /sys/h/param.h.  Anyone ambitious enough to fix this to
 * do it right is welcome to it.	=dgl
 */

#include <sys/param.h>
#include <carl/procom.h>

struct fltbuf fb[NOFILE];
short _samplesize = sizeof(float);
