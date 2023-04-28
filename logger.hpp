#ifndef __LOGGER__HPP__
#define __LOGGER__HPP__

#include <syslog.h>
#include <string.h>
#include <stdarg.h>

class Logger {
	char *logName;
	static bool opened;
	static bool dbg;
	static std::string facility;
	static void openLog();
	static void closeLog();
	static int defLevel;
	static std::string logfile;
	static std::string logdir;
	static size_t maxSize;
	static int revCnt;
	int level;
	static bool withFprintfStderr;
	static void shuffleDown();
	static void checkSizeAndLog( const char *d );
	static std::string logPath( std::string &file, int inst = 0);
	static void chkOpenLog();
	static FILE *logf;

	void logas( int level, const char *fmt, va_list ap );
public:
	static void setSystemName( const char *facility );
	static void setDefaultLevel( const char *nm );
	static void setMaxFileSize( size_t size );
	static void setMaxLogInst( int cnt );
	static void setLogfile( std::string &nm );
	static void setLogdir( std::string &nm );
	static void setWithFprintfStderr( bool how );

	Logger(const char *name);
	virtual ~Logger();
	void setLevel( int lvl );
	int getLevel();

	void log( int level, const char *fmt, ... );
	void error( const char *fmt, ... );
	void emerg( const char *fmt, ... );
	void critical( const char *fmt, ... );
	void alert( const char *fmt, ... );
	void warn( const char *fmt, ... );
	void notice( const char *fmt, ... );
	void info( const char *fmt, ... );
	void debug( const char *fmt, ... );
	void perror( const char *msg, ... );
	void perror( int errorno, const char *msg, ... );
};

#endif
