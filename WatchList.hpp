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

#ifndef __WATCHLIST_HPP__
#define __WATCHLIST_HPP__

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
	DirPerms(const char *path, bool changes=true);
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
	DirPerms( const char *path,  const char *user,  const char *group, int dirPerms, int filePerms, bool changes=true);

	std::string formatPerms( int mask, bool canSetId=false );

	/**
	 *  Provide virtual destructor for any potential subclass
	 */
	virtual ~DirPerms();

	std::string getDescr();

	/**
	 *  Check owner, group and perms for this directory and it's contained files
 	 */
	int checkPerms(bool debugging);

	int checkFile( const char *file, const char *user, const char *grp, int perms, bool debugging = true, bool onlyFiles= true );

	std::string getKey();
	std::string getPath();
	std::string getUser();
	std::string getGroup();
	int getPerms();
	int getRootPerms();
};

/**
 * This class is a vector of DirPerms which is the
 * list of all entries we are watching over.  Entries
 * can be managed with some other mechanism, such as
 * watching a config file for changes.
 */
class WatchList : public virtual std::vector<DirPerms*> {
public:
	WatchList();
	WatchList(std::vector<DirPerms*> &perms);
	virtual ~WatchList();
	void add( DirPerms * perms);
	void addPerms( std::vector<DirPerms *> & perms);
	void removePerms( std::vector<DirPerms*> & perms);
};

#endif
