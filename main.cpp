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
	fprintf(stderr, "usage: %s [-dsit] [-D loglevel] [-c <config>] [-l <log-file>] [-n <log-name>] [--help] <dir:user:group:dir-perm:file-perm>\n", prog );
	_log.error( "usage: %s [-dsit] [-D loglevel] [-c <config>] [-l <log-file>] [-n <log-name>] [--help] <dir:user:group:dir-perm:file-perm>", prog );
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
	bool init = true;
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

	while( (ch = getopt_long(argc, argv, "S:C:tp:l:D:sc:in:dDh", options, &idx) ) != -1 ) {
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
				if( init == true ) {
					_log.error("%s: usage - cannot use -c <config-file> and -i (init static) at the same time",argv[0]);
					exit(1);
				}
				config = strdup(optarg);
				//fprintf( stderr, "loading configuration from: %s\n", config );
				break;
			case 'p':
				logpath = std::string(optarg);
				break;
			case 'l':
				logfile = std::string(optarg);
				break;
			case 'i':
				if( config != NULL ) {
					_log.error("%s: usage - cannot use -c <config-file> and -i (init static) at the same time",argv[0]);
					exit(1);
				}
				init = false;
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

	if( init ) {
		_log.info("using static initialization set");
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/allthisjazz", "radio", "allthisjazz", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/audio-backup", "rivendell", "kwgs", 0770, -1 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/BSI KEYS-Trg", "radio", "radio", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/event log archive", "radio", "producers", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/folksalad", "radio", "folksalad", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Max", "radio", "kwgs", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/rhythmatlas", "radio", "rhythmatlas", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/rivendell-varsnd", "rivendell", "rivendell", 0775, 0664 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/scripts", "radio", "radio", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/swing", "radio", "swing", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/TuPublicScholars", "radio", "tupublicscholars", 0770, 0660 ) );

		  // !'aawk '{print "	addPerms( wl, new DirPerms( \"/kwgs-pool/newOperations/"$9" "$10" "$11" "$12"\", \""$3"\", \""$4"\", 0770, 0660 ) );  // " $1 }'
		
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Opera", "radio", "kwgs", 0770, 0660 ) );               // drwxrwxr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Announcements", "radio", "kwgs", 0775, 0664 ) );       // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/audio", "radio", "kwgs", 0770, 0660 ) );               // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/classical tulsa", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/contentdepot", "radio", "kwgs", 0775, 0664 ) );        // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/copyLogs", "radio", "kwgs", 0770, 0660 ) );            // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/database-backup", "radio", "kwgs", 0777, 0660 ) );     // drwxrwsrwx+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/EAS", "radio", "kwgs", 0775, 0664 ) );                 // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Elizabeth", "radio", "kwgs", 0770, 0660 ) );           // drwxrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Engineer Test Audio", "radio", "kwgs", 0770, 0660 ) ); // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/evergreens", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/fill music folder", "radio", "kwgs", 0775, 0664 ) );   // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/ftp", "radio", "kwgs", 0775, 0664 ) );                 // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Fund Drive", "radio", "kwgs", 0775, 0664 ) );          // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/fund drive folder", "radio", "kwgs", 0770, 0660 ) );   // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Hurricane Sports", "radio", "kwgs", 0775, 0664 ) );    // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/IDs", "radio", "kwgs", 0775, 0664 ) );                 // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Julianne", "radio", "kwgs", 0770, 0660 ) );            // drwxrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1", "radio", "kwgs", 0770, 0660 ) );                // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 carts", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 event logs", "radio", "kwgs", 0775, 0664 ) );     // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 recordings", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 records", "radio", "kwgs", 0770, 0660 ) );        // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2", "radio", "kwgs", 0770, 0660 ) );                // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2 carts", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2 event logs", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2 recordings", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3", "radio", "kwgs", 0770, 0660 ) );                // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3 carts", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3 event logs", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3 recordings", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/KWGS Live Broadcast", "radio", "kwgs", 0775, 0664 ) ); // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1", "radio", "kwgs", 0770, 0660 ) );                // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1 carts", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1 event logs", "radio", "kwgs", 0775, 0664 ) );     // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1 recordings", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2", "radio", "kwgs", 0770, 0660 ) );                // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 carts", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 christmas music", "radio", "kwgs", 0775, 0664 ) );// drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 event logs", "radio", "kwgs", 0775, 0664 ) );     // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 music", "radio", "kwgs", 0775, 0664 ) );          // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 recordings", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3", "radio", "kwgs", 0770, 0660 ) );                // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3 carts", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3 event logs", "radio", "kwgs", 0775, 0664 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3 recordings", "radio", "kwgs", 0770, 0660 ) );     // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/KWTU Live Broadcast", "radio", "kwgs", 0775, 0664 ) ); // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Music T2 Duplicate", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/NaturalLog", "radio", "kwgs", 0770, 0660 ) );          // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/news", "radio", "kwgs", 0770, 0660 ) );                // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Newscasts", "radio", "kwgs", 0770, 0660 ) );           // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/news cuts", "radio", "kwgs", 0770, 0660 ) );           // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/News Old Stories", "radio", "kwgs", 0770, 0660 ) );    // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/normnews", "radio", "kwgs", 0770, 0660 ) );            // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/programs", "radio", "kwgs", 0775, 0660 ) );            // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Promos", "radio", "kwgs", 0775, 0664 ) );              // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/PSAs", "radio", "kwgs", 0775, 0664 ) );                // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Rivendell-Reports", "radio", "kwgs", 0775, 0664 ) );   // drwxrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/shared carts", "radio", "kwgs", 0770, 0660 ) );        // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Stingers", "radio", "kwgs", 0775, 0664 ) );            // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/studiotulsa", "radio", "kwgs", 0770, 0660 ) );         // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Test", "radio", "kwgs", 0775, 0664 ) );                // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/transmitter logs", "radio", "kwgs", 0775, 0664 ) );    // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/TSO", "radio", "kwgs", 0770, 0660 ) );                 // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Under Writing", "radio", "kwgs", 0775, 0664 ) );       // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Weather", "radio", "kwgs", 0775, 0664 ) );             // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations", "radio", "kwgs", 0775, -1 ) );
	} else if( config != NULL ) {
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
