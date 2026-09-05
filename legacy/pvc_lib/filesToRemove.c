#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pv.h"

// Registers scratch files created under /tmp for later removal, and removes
// them either at an explicit flush call (removeFlag != 0) or, as a safety
// net, whenever the process exits (including via an early exit(EXIT_FAILURE)
// on an error path that never reaches its own flush call). Previously this
// built up one long "rm file1 file2 ..." string and ran it through system(),
// which only ever ran on the explicit flush call and shelled out for what is
// just a series of unlink()s.

static char **pendingFiles = NULL ;
static int pendingFileCount = 0 ;
static int pendingFileCapacity = 0 ;

static void removePendingFiles( void )
{
	int i ;
	for( i = 0 ; i < pendingFileCount ; i++ ){
		unlink( pendingFiles[ i ] ) ;
		free( pendingFiles[ i ] ) ;
	}
	pendingFileCount = 0 ;
}

void filesToRemove(
	char *file,
	int removeFlag
)
{
	static int atexitRegistered = 0 ;

	if( ! atexitRegistered ){
		atexit( removePendingFiles ) ;
		atexitRegistered = 1 ;
	}

	if( removeFlag == 0 ){
		if( pendingFileCount == pendingFileCapacity ){
			pendingFileCapacity = pendingFileCapacity ? (pendingFileCapacity * 2) : 16 ;
			pendingFiles = realloc( pendingFiles, pendingFileCapacity * sizeof( char * ) ) ;
		}
		pendingFiles[ pendingFileCount++ ] = strdup( file ) ;
	}else{
		removePendingFiles() ;
	}

} ;
