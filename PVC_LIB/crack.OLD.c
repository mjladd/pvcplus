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

char crack(argc, argv, flags, ign)
    int argc; char **argv; char *flags; int ign;
{
    char *pv, *flgp, *index();
    static int first=1; 

    if( first == 1 ){
	strcpy( routine, argv[0] ) ; 
//	prs( routine, "ROUTINE" ) ; prt("\n\n" ) ; 
	first = 0 ; 
    } ; 

    while ((arg_index) < argc)
	{
	if (pvcon != NULL)
	    pv = pvcon;
	else
	    {
	    if (++arg_index >= argc) return(NULL); 
	    pv = argv[arg_index];
	    if (*pv != '-') 
		return(NULL);
	    }
	pv++;		/* skip '-' or prev. flag */
	if (*pv != NULL) 
	    {
	    if ((flgp=index(flags, *pv)) != NULL)
		{
		pvcon = pv;
		if (*(flgp+1) == '|') { arg_option = pv+1; pvcon = NULL; }
		return(*pv);
		}
	    else
		if (!ign)
		    {
		    fprintf(stderr, "%s: no such flag: %s\n", argv[0], pv);
		    exit(0) ; 
		    //return(0);
		    }
		else
		    pvcon = NULL;
	    }
	pvcon = NULL;
	}
    return(NULL);
    }

