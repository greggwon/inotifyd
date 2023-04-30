#include <list>
#include "WatchList.hpp"

class CSVload {

private:
	std::list<DirPerms*> loaded;
public:
	CSVload();
	virtual ~CSVload();
	std::list<DirPerms*> *getLoaded();
	int loadFile( const char*file );
};
