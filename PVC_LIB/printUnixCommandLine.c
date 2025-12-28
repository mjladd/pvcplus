#include <stdio.h>
#include <math.h>
#include "pv.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

void printUnixCommandLine(
	int argc, 
	char **argv
)
{
	int i; 

	prt("\nUNIX COMMAND:\n"); 
	for(i = 0; i < argc; i++) fprintf( stderr,"%s ", argv[ i ] ); 
	prt( "\n\n" ); 

} ; 