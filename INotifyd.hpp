#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <string>
#include "WatchList.hpp"
#include "logger.hpp"

class INotifyd {
public:
	INotifyd();
	virtual ~INotifyd();
	bool init(bool debug = true);
	bool watch(WatchList & list);
	bool checkAll(WatchList & list);
	bool debugging;


private:
	std::string describeEvent(int mask, char name[], int len );
	int fd;
	Logger _log;

};
