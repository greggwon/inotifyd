#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>


int
main( int argc, char **argv )
{
	INotifyd isvc;

	while( true ) {
		if( isvc.init() ) {
			isvc.watch();
		}
		fprintf(stderr, "%s: Restarting service...", ctime(0) );
	}
	exit( 0 );
}
