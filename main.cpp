#include "INotifyd.hpp"

#include <unistd.h>
#include <getopt.h>

void addPerms( WatchList &wl, DirPerms * perms ) {
	Logger _log( "addPerms" );
	wl.add( perms );
	_log.debug( "Adding permissions: %s", perms->getDescr().c_str() );
}

void usage( const char *prog, Logger &_log ) {
	fprintf(stderr, "usage: %s [-c <config>] [-i] [-n <log-name>] [-d] [-D] [-s] --help\n", prog );
	_log.info( "usage: %s [-c <config>] [-i] [-n <log-name>] [-d] [-D] [-s] --belp", prog );
	exit(2);
}

static struct option options[] = {
	{"help", 0, NULL, 'h'},
	{0}
};

int
main( int argc, char **argv )
{
	INotifyd isvc;
	WatchList wl;
	Logger _log( "main" );

	int ch;
	bool init = false;
	const char *p = "inotifyd";
	const char *config = NULL;
	bool debugging = false;
	bool noChanges = false;
	bool service = false;
	int idx = 0;
	while( (ch = getopt_long(argc, argv, "sc:i:n:dDh", options, &idx) ) != -1 ) {
		switch( ch ) {
			case 's':
				service = true;
				break;
			case 'c':
				if( init == true ) {
					_log.error("%s: usage - cannot use -c <config-file> and -i (init static) at the same time",argv[0]);
					exit(1);
				}
				config = strdup(optarg);
				break;
			case 'i':
				if( config != NULL ) {
					_log.error("%s: usage - cannot use -c <config-file> and -i (init static) at the same time",argv[0]);
					exit(1);
				}
				init = true;
				break;
			case 'n':
				p = strdup(optarg);
				break;
			case 'D':
				debugging = true;
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
				fprintf( stderr, "%s: usage - unexpected argument %c (%d)\n", argv[0], ch, ch );
				usage( argv[0], _log );
				break;
		}
	}

	Logger::setSystemName(p);

	// :.,'as/\("[^"]*"\) \("[^"]*"\)[ ]*\(".*\)/\3, \1, \2, 0660

	if( init ) {
		_log.info("using static initialization set");
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/allthisjazz", "radio", "allthisjazz", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/audio-backup", "rivendell", "kwgs", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/BSI KEYS-Trg", "radio", "radio", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/event log archive", "radio", "producers", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/folksalad", "radio", "folksalad", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Max", "radio", "producers", 0770, 0740 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/rhythmatlas", "radio", "rhythmatlas", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/rivendell-varsnd", "radio", "rivendell", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/scripts", "radio", "radio", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/swing", "radio", "swing", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/TuPublicScholars", "radio", "tupublicscholars", 0770, 0660 ) );

		// !'aawk '{print "	addPerms( wl, new DirPerms( \"/kwgs-pool/newOperations/"$9" "$10" "$11" "$12"\", \""$3"\", \""$4"\", 0770, 0660 ) );  // " $1 }'
		
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Opera", "radio", "kwgs", 0770, 0660 ) );
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Announcements", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/audio", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/classical tulsa", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/contentdepot", "radio", "kwgs", 0775, 0660 ) );  // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/copyLogs", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/database-backup", "radio", "kwgs", 0777, 0660 ) );  // drwxrwsrwx
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/EAS", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Elizabeth", "radio", "kwgs", 0770, 0660 ) );  // drwxrws---
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Engineer Test Audio", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/evergreens", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/fill music folder", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/ftp", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Fund Drive", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/fund drive folder", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Hurricane Sports", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/IDs", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Julianne", "radio", "kwgs", 0770, 0660 ) );  // drwxrws---
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 carts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 event logs", "radio", "kwgs", 0775, 0660 ) );  // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 recordings", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg1 records", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2 carts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2 event logs", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg2 recordings", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3 carts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3 event logs", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwg3 recordings", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/KWGS Live Broadcast", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1 carts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1 event logs", "radio", "kwgs", 0775, 0775 ) );  // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt1 recordings", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 carts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 christmas music", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 event logs", "radio", "kwgs", 0775, 0660 ) );  // drwsr-sr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 music", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt2 recordings", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3 carts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3 event logs", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/kwt3 recordings", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/KWTU Live Broadcast", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Music T2 Duplicate", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/NaturalLog", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/news", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Newscasts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/news cuts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/News Old Stories", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/normnews", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/programs", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Promos", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/PSAs", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Rivendell-Reports", "radio", "kwgs", 0775, 0660 ) );  // drwxrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/shared carts", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Stingers", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/studiotulsa", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Test", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/transmitter logs", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/TSO", "radio", "kwgs", 0770, 0660 ) );  // drwsrws---+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Under Writing", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations/Weather", "radio", "kwgs", 0775, 0660 ) );  // drwsrwsr-x+
		addPerms( wl, new DirPerms( "/kwgs-pool/newOperations", "radio", "kwgs", 0775, 0660 ) );
	}
	if( debugging ) {
		_log.info("Debugging: monitoring testdir/");
		system("mkdir -p testdir");
		addPerms( wl, new DirPerms( "testdir", "radio", "kwgs", 0775, 0660 ) );
	}
	_log.info("starting up");
	int changes = 0;
	while( true ) {
		_log.info("initializing notify services");
		if( isvc.init(noChanges) ) {
			_log.info("watching %d directories",wl.size());
			if( service ) {
				isvc.watch(wl);
			} else {
				changes = isvc.checkAll(wl);
				break;
			}
		}
		fprintf(stderr, "%s: Restarting service...\n", ctime(0) );
	}
	exit( changes );
}
