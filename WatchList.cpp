#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "logger.hpp"

/**
 *  Construct an instance with only a directory name
 */
DirPerms::DirPerms(const char *path, bool changes=true) : _log(path) {
	std::ifstream g_file( "/proc/sys/kernel/random/uuid", std::ifstream::in );
	makeChanges = changes;
	g_file >> _guid;
	g_file.close();
}
/**
 *  Construct an instance with directory name and user/group and perms settings
 *
 *  path:  path to directory
 *  user:  owner of directory and contained files
 *  group: group with access to directory and files
 *  dirPerms: permissions on directory and subdirectories? (no subdirs yet)
 *  filePerms: permissions on files
 *  changes: true, by default, to cause fixes to be made to user/group and perms.
 */
DirPerms::DirPerms( const char *path,  const char *user,  const char *group, int dirPerms, int filePerms, bool changes=true) : DirPerms(path) {
	makeChanges = changes;
	_dperms = dirPerms;
	_perms = filePerms;
	_path = path;
	_user = user;
	_group = group;
}

std::string DirPerms::formatPerms( int mask, bool canSetId=false ) {
	std::string t = "";
	if( mask & 04 ) {
		t += "r";
	} else {
		t += "-";
	}
	if( mask & 02 ) {
		t += "w";
	} else {
		t += "-";
	}
	if( mask & 01 ) {
		t += canSetId ? "s" : "x";
	} else {
		t += "-";
	}
	return t;
}

/**
 *  Provide virtual destructor for any potential subclass
 */
DirPerms::~DirPerms(){}

std::string DirPerms::getDescr() {
	return "key="+getKey()
		+", path=\""+getPath()
		+"\", user="+getUser()
		+", group="+getGroup()
		+", dir-perms=d"
		+formatPerms((getRootPerms()>>6)&07, getRootPerms() & S_ISUID)
		+formatPerms((getRootPerms()>>3)&07, getRootPerms() & S_ISGID)
		+formatPerms((getRootPerms())&07)
		+", file-perms="
		+formatPerms((getPerms()>>6)&07, getPerms() & S_ISUID)
		+formatPerms((getPerms()>>3)&07, getPerms() & S_ISGID)
		+formatPerms((getPerms())&07)
		;
}

/**
 *  Check owner, group and perms for this directory and it's contained files
 */
int DirPerms::checkPerms(bool debugging) {
	DIR *d;
	struct dirent *de;

	d = opendir(_path.c_str());
	if (d == NULL) {
		_log.perror(_path.c_str());
		return -1;
	}
	changed = 0;

	// Check directory details forcing directory processing
	if( checkFile( _path.c_str(), _user.c_str(), _group.c_str(), _dperms, debugging, false ) != 0 ) {
		_log.error("cannot check permissions and ownership of directory: %s", _path.c_str() );
	}
	
	// Check if should change file details, -1 perms says ignore files and just check the directory as we've done above
	if( _perms != -1 ) {
		for (de = readdir(d); de != NULL; de = readdir(d)) {
			std::string p = _path + "/" + std::string(de->d_name);
			// Check file in directory avoiding subdirectory perms
			if( checkFile( p.c_str(), _user.c_str(), _group.c_str(), _perms, debugging, true ) != 0 ) {
				_log.error("cannot check permissions and ownership of file: %s", p.c_str() );
			}
		}
	}
	closedir(d);
	return changed;
}

int DirPerms::checkFile( const char *file, const char *user, const char *grp, int perms, bool debugging = true, bool onlyFiles= true ) {
	struct stat sbuf;
	int exists;
	debugging = debugging || !makeChanges;
	exists = stat( file, &sbuf);
	_log.debug( "Looking at %s (%s,%s), perms=%o", file, user, grp, perms);
	if (exists < 0) {
		perror(file);
		_log.perror(file);
		return -1;
	} else {
		if( !onlyFiles || (onlyFiles && ( sbuf.st_mode & S_IFDIR ) == 0 ) ) {
			_log.debug( "Checking \"%s\" (%s,%s), perms=%o", file, user, grp, perms);
			struct passwd pwd;
			struct passwd *result;
			char *buf;
			size_t bufsize;

			int s;

			// Find size of password data struture
			bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
			if (bufsize == -1)		/* Value was indeterminate */
				bufsize = 16384;	/* Should be more than enough */

			// Get a buffer to hold the data
			buf = (char *)malloc(bufsize);
			if (buf == NULL) {
				_log.perror("malloc");
				return -1;
			}
			// Get the entry for the user
			s = getpwnam_r(user, &pwd, buf, bufsize, &result);
			if (result == NULL) {
				// No such user or something bad happened report
				// error and move on
				if (s == 0) {
					_log.error("User %s Not found", user);
				} else {
					errno = s;
					_log.perror("getpwnam_r");
				}
				// free buffer before returning error
				free(buf);
				return -1;
			}
			// Get uid and gid int values for the user
			uid_t uid = result->pw_uid;
			gid_t gid = result->pw_gid;

			// free buffer no longer needed
			free(buf);
			result = NULL;

			// Get the group entry
			struct group *gp = getgrnam(grp);
			if( gp == NULL ) {
				// No such group or other problem, report it
				_log.perror(("getgrnam: '"+std::string(grp)+"'").c_str());
				_log.info( "Group %s not found", grp);
				return -1;
			}
			// Get the ID to use
			gid_t dgid = gp->gr_gid;

			int st = 0;
			// Check for owner or group id mismatch
			if( sbuf.st_uid != uid || sbuf.st_gid != dgid ) {
				_log.debug("Wrong ownership:  user=%d, grp=%d changing to user=%d, grp=%d", 
					sbuf.st_uid, sbuf.st_gid, uid, dgid );
				// If changes are enabled, fix user and/or group discrepency.
				if( !debugging && ((st = chown( file, uid, dgid )) != 0) ) {
					_log.perror( "chown of \"%s\"",file);
				} else if( debugging && st == 0 ) {
					_log.debug( "would chown of \"%s\": from %d:%d to %d:%d",
						file, sbuf.st_uid, sbuf.st_gid, uid, dgid );
				} else if( !debugging && st == 0 ) {
					_log.info( "did chown \"%s\": from %d:%d to %d:%d",
						file, sbuf.st_uid, sbuf.st_gid, uid, dgid );
				}
				changed += (st != -1 && !debugging);
			}
			st = 0;
			// Check user,group and others permissions masked mode matches perms
			// setuid/setgid is not managed here, but could be added
			if( (sbuf.st_mode & (S_IRWXU| S_IRWXG| S_IRWXO)) != perms ) {
				_log.debug( "Wrong perms for %s: %o: setting to %o", file, sbuf.st_mode & 07777, perms );
				// If changes allowed, change the mode of the file by adding missing bits
				// We don't remove existing bits to avoid breaking temporary changes needed
				if( !debugging && ((st = chmod( file, perms & 0777 )) != 0) ) {
					_log.perror( "chmod of \"%s\"", file );
				} else if( debugging && st == 0 ) {
					_log.debug( "would chmod of \"%s\": from %012o to %012o",
						file, sbuf.st_mode, perms );
				} else if( !debugging && st == 0 ) {
					_log.info( "did chmod \"%s\": from %012o to %012o",
						file, sbuf.st_mode, perms );
				}
				changed += (st != -1 && !debugging);
			}
		}
	}
	return 0;
}

std::string DirPerms::getKey()   { return _guid;   }
std::string DirPerms::getPath()  { return _path;   }
std::string DirPerms::getUser()  { return _user;   }
std::string DirPerms::getGroup() { return _group;  }
int DirPerms::getPerms()         { return _perms;  }
int DirPerms::getRootPerms()     { return _dperms; }

/**
 * This class is a vector of DirPerms which is the
 * list of all entries we are watching over.  Entries
 * can be managed with some other mechanism, such as
 * watching a config file for changes.
 */

WatchList::WatchList() {
}

WatchList::WatchList(std::vector<DirPerms*> &perms) : std::vector<DirPerms*>(perms) {
}

WatchList::~WatchList() {}

void WatchList::add( DirPerms * perms) {
	push_back( perms );
}

void WatchList::addPerms( std::vector<DirPerms *> & perms) {
	for( auto v = perms.begin(); v != perms.end(); ++v ) {
		push_back( *v );
	}
}

void WatchList::removePerms( std::vector<DirPerms*> & perms) {
	for( auto v = perms.begin(); v != perms.end(); ++v ) {
		for( auto m = begin(); m != end(); ++m ) {
			if( (*v)->getKey() == (*m)->getKey() ) {
				erase( m );
				break;
			}
		}
	}
}
