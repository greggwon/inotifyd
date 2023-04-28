#include "INotifyd.hpp"
#include <stdarg.h>


std::string
INotifyd::describeEvent(int mask, char name[], int len )
{
	std::string desc;
	for( int i = 0; i < 32; ++i ) {
		int s = ( mask & (1<<i) );
		if( !s )
			continue;

		switch( mask & (1<<i) ) {
			case IN_ACCESS: desc.append("ACCESS"); break;
			case IN_ATTRIB: desc.append("ATTRIB"); break;
			case IN_CLOSE_WRITE: desc.append("CLOSE_WRITE"); break;
			case IN_CLOSE_NOWRITE: desc.append("CLOSE_NOWRITE"); break;
			case IN_CREATE: desc.append("CREATE"); break;
			case IN_DELETE: desc.append("DELETE"); break;
			case IN_DELETE_SELF: desc.append("DELETE_SELF"); break;
			case IN_MODIFY: desc.append("MODIFY"); break;
			case IN_MOVE_SELF: desc.append("MOVE_SELF"); break;
			case IN_MOVED_FROM: desc.append("MOVED_FROM"); break;
			case IN_MOVED_TO: desc.append("MOVED_TO"); break;
			case IN_OPEN: desc.append("OPEN"); break;
		}
	}

	if( len > 0 ) {
		char *buf = (char*)malloc( len +1 );
		desc.append(" | \"");
		strncpy( buf, name, len );
		buf[len] = '\0';
		desc.append( buf );
		desc.append("\"");
	}
	return desc;
}

INotifyd::INotifyd() : _log("INotifyd" )
{
}

INotifyd::~INotifyd()
{
}

bool INotifyd::init(bool debugOnly)
{
	fd = inotify_init();
	debugging = debugOnly;
	if(fd == -1) {
		_log.perror("inotify_init");
	}
	return( fd != -1 );
}

bool INotifyd::checkAll(WatchList & list)
{
	int changed = 0;
	for( int i = 0; i < list.size(); ++i ) {
		_log.info("checking [%d] (%s)", i, list[i]->getPath().c_str() );
		if( list[i]->checkPerms(debugging) ) {
			changed++;
		}
	}
	return changed;
}

bool INotifyd::watch(WatchList & list)
{
	struct inotify_event *ev;
	char buf[10000];
	int st[list.size()];

	for( int i = 0; i < list.size(); ++i ) {
		st[i] = inotify_add_watch( fd, list[i]->getPath().c_str(), IN_ALL_EVENTS & ~(IN_CLOSE_NOWRITE|IN_DELETE|IN_MOVED_FROM|IN_OPEN|IN_MODIFY|IN_ACCESS) );
	}
	int cnt;
	while( (cnt = read( fd, buf, sizeof(buf) )) > 0 ) {
		_log.debug( "read notify returns %d, sizeof=%d", cnt, sizeof(*ev) );
		ev = (struct inotify_event *)buf;
		while( cnt > sizeof(struct inotify_event) ) {
			_log.debug( "processing ev at %p: %d vs %d (%d)", ev, cnt, sizeof(struct inotify_event), ev->len );
			for( int i = 0; i < list.size(); ++i ) {
				_log.debug("checking [%d] (%s) [len=%d], %.*s", i, list[i]->getPath().c_str(), ev->len, ev->len, ev->name );
				_log.debug("Check \"%s\" vs \"%.*s\"", list[i]->getPath().c_str(), ev->len, ev->name );
				if( st[i] == ev->wd ) {
					time_t now;
					time(&now);
					_log.info( "%.24s: event(%d:%d): %s: %s", ctime(&now), cnt, ev->len, list[i]->getPath().c_str(), describeEvent(ev->mask, ev->name, ev->len ).c_str() );
				}
				list[i]->checkPerms(debugging);
			}
			int l = sizeof(*ev)+ev->len;
			ev = (struct inotify_event *)(((char *)ev) + l);
			cnt -= l;
			_log.debug("len=%d, ev=%p, cnt=%d", l, ev, cnt );
		}
	}
	if( cnt < 0 ) {
		_log.critical("Error processing inotify_event: %s", strerror(errno) );
		_log.perror("read inotify_event");
	}
	return cnt > 0;
}
