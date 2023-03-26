#include "INotifyd.hpp"

std::string INotifyd::describeEvent(int mask )
{
	char desc[1000];
	*desc = 0;
	for( int i = 0; i < 32; ++i ) {
		int s = ( mask & (1<<i) );
		if( !s )
			continue;
		if( *desc )
			strcat( desc, " | ");
		switch( mask & (1<<i) ) {
			case IN_ACCESS: strcat( desc, "ACCESS"); break;
			case IN_ATTRIB: strcat( desc, "ATTRIB"); break;
			case IN_CLOSE_WRITE: strcat( desc, "CLOSE_WRITE"); break;
			case IN_CLOSE_NOWRITE: strcat( desc, "CLOSE_NOWRITE"); break;
			case IN_CREATE: strcat( desc, "CREATE"); break;
			case IN_DELETE: strcat( desc, "DELETE"); break;
			case IN_DELETE_SELF: strcat( desc, "DELETE_SELF"); break;
			case IN_MODIFY: strcat( desc, "MODIFY"); break;
			case IN_MOVE_SELF: strcat( desc, "MOVE_SELF"); break;
			case IN_MOVED_FROM: strcat( desc, "MOVED_FROM"); break;
			case IN_MOVED_TO: strcat( desc, "MOVED_TO"); break;
			case IN_OPEN: strcat( desc, "OPEN"); break;
		}
	}
	return std::string(desc);
}

INotifyd::INotifyd() {
}

bool INotifyd::init() {
	fd = inotify_init();
	if(fd == -1) {
		perror("inotify_init");
	}
	return( fd != -1 );
}

bool INotifyd::watch()
{
	struct inotify_event *ev;
	char buf[10000];
	int st[argc];

	for( int i = 1; i < argc; ++i ) {
		st[i-1] = inotify_add_watch( fd, argv[i], IN_ALL_EVENTS );
	}
	int cnt;
	while( (cnt = read( fd, buf, sizeof(buf) )) > 0 ) {
		// fprintf( stderr, "read notify returns %d, sizeof=%d\n", cnt, sizeof(*ev) );
		while( cnt > sizeof(struct inotify_event) ) {
			ev = (struct inotify_event *)buf;
			// fprintf(stderr, "processing ev at %p\n", ev );
			for( int i = 1; i < argc; ++i ) {
				// fprintf(stderr, "checking [%d] (%s) [len=%d], %.*s\n", i, argv[i], ev->len, ev->len, ev->name );
				// fprintf( stderr, "Check \"%s\" vs \"%.*s\"\n", argv[i], ev->len, ev->name );
				if( st[i-1] == ev->wd ) {
					time_t now;
					time(&now);
					fprintf( stderr, "%.24s: event: %s (%.*s): %s\n", ctime(&now), argv[i], ev->len, ev->name, describeEvent(ev->mask ) );
				}
			}
			int l = sizeof(*ev) + ev->len;
			ev = (struct inotify_event *)((char *)ev) + l;
			cnt -= l;
		}
	}
	if( cnt < 0 ) {
		perror("read inotify_event");
	}
	return cnt > 0;
}
