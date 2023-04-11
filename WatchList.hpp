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

class DirPerms {
private:
	std::string _path;	// Path to directory
	std::string _user;	// username who should be owner for directory and files in directory
	std::string _group;	// groupname for directory and files in directory
	int _perms,		// permissions on files in directory
		_dperms;	// permissions on directory
	std::string _guid;	// GUID key for this entry
	Logger _log;		// log instance access
	bool makeChanges;	// Should we make owner/group and permission changes
	int changed;

public:
	/**
	 *  Construct an instance with only a directory name
	 */
	DirPerms(const char *path, bool changes=true) : _log(path) {
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
	DirPerms( const char *path,  const char *user,  const char *group, int dirPerms, int filePerms, bool changes=true) : DirPerms(path) {
		makeChanges = changes;
		_dperms = dirPerms;
		_perms = filePerms;
		_path = path;
		_user = user;
		_group = group;
	}

	std::string formatPerms( int mask, bool canSetId=false ) {
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
	virtual ~DirPerms(){}

	std::string getDescr() {
		return getKey()
			+", path="+getPath()
			+", user="+getUser()
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
	int checkPerms(bool debugging) {
		DIR *d;
		struct dirent *de;

		d = opendir(_path.c_str());
		if (d == NULL) {
			perror(_path.c_str());
			return -1;
		}
		changed = 0;

		// Check directory details forcing directory processing
		if( checkFile( _path.c_str(), _user.c_str(), _group.c_str(), _dperms, debugging, false ) != 0 ) {
			_log.error("cannot check permissions and ownership of directory: %s", _path.c_str() );
		}
		
		for (de = readdir(d); de != NULL; de = readdir(d)) {
			std::string p = _path + "/" + std::string(de->d_name);
			// Check file in directory avoiding subdirectory perms
			if( checkFile( p.c_str(), _user.c_str(), _group.c_str(), _perms, debugging ) != 0 ) {
				_log.error("cannot check permissions and ownership of file: %s", p.c_str() );
			}
		}
		closedir(d);
		return changed;
	}

	int checkFile( const char *file, const char *user, const char *grp, int perms, bool debugging = true, bool onlyFiles= true ) {
		struct stat sbuf;
		int exists;
		debugging = debugging || !makeChanges;
		exists = stat( file, &sbuf);
		if (exists < 0) {
			_log.perror(file);
			return -1;
		} else {
			if( !onlyFiles || ( sbuf.st_mode & S_IFDIR ) == 0 ) {
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
						_log.info("User %s Not found\n", user);
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
					_log.info( "Group %s not found\n", grp);
					return -1;
				}
				// Get the ID to use
				gid_t dgid = gp->gr_gid;

				int st = 0;
				// Check for owner or group id mismatch
				if( sbuf.st_uid != uid || sbuf.st_gid != dgid ) {
					_log.info("Wrong ownership:  user=%d, grp=%d changing to user=%d, grp=%d\n", 
						sbuf.st_uid, sbuf.st_gid, uid, dgid );
					// If changes are enabled, fix user and/or group discrepency.
					if( !debugging && (st = chown( file, uid, dgid ))  != 0 ) {
						_log.perror( ("chown of "+std::string(file)).c_str() );
					} else if( debugging && st == 0 ) {
						_log.info( ("would chown of %s: from %d:%d to %d:%d",
							std::string(file)).c_str(),
							sbuf.st_uid, sbuf.st_gid, uid, dgid );
					}
					changed += (st != -1 && !debugging);
				}
				st = 0;
				// Check user,group and others permissions masked mode matches perms
				// setuid/setgid is not managed here, but could be added
				if( ( (sbuf.st_mode & (S_IRWXU| S_IRWXG| S_IRWXO)) & _perms) != _perms ) {
					_log.info( "Wrong perms for %s: %o: setting to %o\n", file, sbuf.st_mode & 07777, _perms );
					// If changes allowed, change the mode of the file by adding missing bits
					// We don't remove existing bits to avoid breaking temporary changes needed
					if( !debugging && (st = chmod( file, (sbuf.st_mode | _perms) & 07777 )) != 0 ) {
						_log.perror( ("chmod of "+std::string(file)).c_str() );
					} else if( debugging && st == 0 ) {
						_log.info( ("would chmod of %s: from %012o to %012o",
							std::string(file)).c_str(),
							sbuf.st_mode, _perms );
					}
					changed += (st != -1 && !debugging);
				}
			}
		}
		return 0;
	}

	std::string getKey()   { return _guid;   }
	std::string getPath()  { return _path;   }
	std::string getUser()  { return _user;   }
	std::string getGroup() { return _group;  }
	int getPerms()         { return _perms;  }
	int getRootPerms()     { return _dperms; }
};

/**
 * This class is a vector of DirPerms which is the
 * list of all entries we are watching over.  Entries
 * can be managed with some other mechanism, such as
 * watching a config file for changes.
 */
class WatchList : public virtual std::vector<DirPerms> {
public:
	WatchList() {
	}
	WatchList(std::vector<DirPerms> &perms) : std::vector<DirPerms>(perms) {
	}
	virtual ~WatchList() {}
	void add( DirPerms * perms) {
		push_back( *perms );
	}
	void addPerms( std::vector<DirPerms> & perms) {
		for( auto v = perms.begin(); v != perms.end(); ++v ) {
			push_back( *v );
		}
	}
	void removePerms( std::vector<DirPerms> & perms) {
		for( auto v = perms.begin(); v != perms.end(); ++v ) {
			for( auto m = begin(); m != end(); ++m ) {
				if( v->getKey() == m->getKey() ) {
					erase( m );
					break;
				}
			}
		}
	}
};
