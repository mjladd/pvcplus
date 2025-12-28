#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pv.h"
#include <sndfile.h>
#include <string.h>

#include <unistd.h>

#ifndef CRACK
#define CRACK
#define DEFNM "test"	/* default file name */

char crack();

extern int arg_index;
extern char *arg_option;
extern char *pvcon;

extern float sfexpr();
extern long summation();
#endif


int arg_index = 0;
char *arg_option;
char *pvcon = NULL;

char crack(int argc, char **argv, char *flags, int ign)
//    int argc; char **argv; char *flags; int ign;
{
    char *pv, *flgp, *index();
    static int first=1; 

    if( first == 1 ){
		strcpy( routine, argv[0] ) ; 
		prs( routine, "ROUTINE" ) ; prt("\n\n" ) ; 
		printUnixCommandLine( argc, argv ); 
		first = 0 ; 
    } ; 

    while ((arg_index) < argc)
	{
	if (pvcon != NULL)
	    pv = pvcon;
	else
	    {
	    if (++arg_index >= argc) return(CRACK_DONE_FLAG); 
	    pv = argv[arg_index];
	    if (*pv != '-') 
		return(CRACK_DONE_FLAG);
	    }
	pv++;		/* skip '-' or prev. flag */
	if (*pv != '\0') 
	    {
	    if ((flgp=index(flags, *pv)) != NULL_CHAR)
		{
		pvcon = pv;
		if (*(flgp+1) == '|') { arg_option = pv+1; pvcon = NULL; }
		return(*pv);
		}
	    else
		if (!ign)
		    {
		    fprintf(stderr, "ERROR --> %s: UNRECOGNIZED FLAG: %s\n . . . BYE.\n\n", argv[0], pv);
		    exit(EXIT_FAILURE) ; 
		    //return(0);
		    }
		else
		    pvcon = NULL;
	    }
	pvcon = NULL;
	}
    return(CRACK_DONE_FLAG);
    }

