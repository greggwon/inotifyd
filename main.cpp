#include "INotifyd.hpp"

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "csvload.hpp"

static void addPerms( WatchList &wl, DirPerms * perms ) {
	Logger _log( "addPerms" );
	wl.add( perms );
	std::string desc = perms->getDescr();
	_log.debug( "Adding permissions: %s", desc.c_str() );
}

void usage( const char *prog, Logger &_log ) {
	fprintf(stderr, "usage: %s [-dst] [-D loglevel] [-c <config>] [-l <log-file>] [-n <log-name>] [--help] <dir:user:group:dir-perm:file-perm>\n", prog );
	_log.error( "usage: %s [-dst] [-D loglevel] [-c <config>] [-l <log-file>] [-n <log-name>] [--help] <dir:user:group:dir-perm:file-perm>", prog );
	exit(2);
}

static struct option options[] = {
	{"help", 0, NULL, 'h'},
	{0}
};

int atoo( char *str ) {
	int v = 0;
	sscanf( str, "%o", &v );
	return v;
}

int
main( int argc, char **argv )
{
	INotifyd isvc;
	WatchList wl;
	Logger _log( "main" );

	//fprintf( stderr, "Running inotifyd\n");
	int ch;
	const char *p = "inotifyd";
	std::string logfile = "inotifyd.log";
	std::string logpath = "/var/log/inotifyd";
	const char *config = NULL;
	int logCount = 10;
	size_t logSize = 20*1024*1024;
	bool noChanges = false;
	bool service = true;
	int idx = 0;
	bool alwaysFprintfStderr = false;

	while( (ch = getopt_long(argc, argv, "S:C:tp:l:D:sc:n:dDh", options, &idx) ) != -1 ) {
		//fprintf(stderr, "Processing -%c option\n", ch );
		switch( ch ) {
			case 'C':
				logCount = atoi( optarg );
				break;
			case 'S':
				logSize = atol( optarg );
				break;
			case 's':
				service = false;
				break;
			case 'c':
				config = strdup(optarg);
				//fprintf( stderr, "loading configuration from: %s\n", config );
				break;
			case 'p':
				logpath = std::string(optarg);
				break;
			case 'l':
				logfile = std::string(optarg);
				break;
			case 'n':
				p = strdup(optarg);
				break;
			case 't':
				alwaysFprintfStderr = true;
				break;
			case 'D':
				Logger::setDefaultLevel(optarg);
				break;
			case 'd':
				noChanges = true;
				break;
			case 'h':
			case '?':
				usage( argv[0], _log );
				break;
			case '-': break;
			default:
				_log.error("%s: usage - unexpected argument %c (%d)\n", argv[0], ch, ch );
				usage( argv[0], _log );
				break;
		}
	}

	Logger::setWithFprintfStderr( isatty(2) || alwaysFprintfStderr );
	Logger::setSystemName(p);
	Logger::setLogfile( logfile );
	{
		struct stat sbuf;
		if( stat( logpath.c_str(), &sbuf ) == 0 ) {
			if( S_ISDIR( sbuf.st_mode) == 0 ) {
				fprintf(stderr, "%s: error: %s is not a directory!\n", argv[0], logpath.c_str() );
				exit(2);
			}
		} else {
			if( mkdir(logpath.c_str(), 0777) == 0 ) {
				Logger::setLogdir( logpath );
			} else {
				fprintf(stderr, "%s: error: cannot created directory %s: %s\n", argv[0], logpath.c_str(), strerror(errno) );
				exit(2);
			}
		}
	}
	Logger::setMaxFileSize( logSize );
	Logger::setMaxLogInst( logCount );
	Logger::setLogdir( logpath );

	// :.,'as/\("[^"]*"\) \("[^"]*"\)[ ]*\(".*\)/\3, \1, \2, 0660

	// Also load from any specified config file
	if( config != NULL ) {
		CSVload ld;

		if( ld.loadFile(config) != 0 ) {
			exit(2);
		}
		std::list<DirPerms*> *perms = ld.getLoaded();
		for( auto i = perms->begin(); i != perms->end(); ++i ) {
			fprintf(stderr, "loaded %s\n", (*i)->getDescr().c_str() );
			addPerms( wl, *i );
		}
	}

	while( optind < argc ) {
		char *t = argv[optind];
		char *p = strdup(strtok( t, ":"));
		char *u = strdup(strtok( NULL, ":"));
		char *g = strdup(strtok( NULL, ":"));
		char *d = strdup(strtok( NULL, ":"));
		char *f = strdup(strtok( NULL, ":"));
		if( p == NULL || u == NULL || g == NULL || d == NULL || f == NULL ) {
			fprintf(stderr, "Error parsing request %s\n", t );
			usage(argv[0], _log);
		}
		if( atoi( f ) == -1 ) {
			addPerms( wl, new DirPerms( p, u, g, atoo(d), -1 ) );
		} else {
			addPerms( wl, new DirPerms( p, u, g, atoo(d), atoo(f) ) );
		}
		_log.info("Adding %s (%s,%s) d=>%o, f=>%o", p, u, g, atoo(d), atoo(f) );

		optind++;
		free( p );
		free( u );
		free( g );
		free( d );
		free( f );
	}
	_log.info("Processing %d entries", wl.size());
	int changes = 0;
	while( true ) {
		_log.info("initializing notify services: %schanges", noChanges ? "without " : "with ");
		if( isvc.init(noChanges) ) {
			_log.info("watching %d directories: %schanges",wl.size(), noChanges ? "without " : "with ");
			if( service ) {
				isvc.watch(wl);
			} else {
				changes = isvc.checkAll(wl);
				break;
			}
		}
		_log.info("Restarting service!", ctime(0) );
	}
	exit( changes );
}
