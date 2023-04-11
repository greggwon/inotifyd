#ifndef __LOGGER__HPP__
#define __LOGGER__HPP__

#include <syslog.h>
#include <string.h>
#include <stdarg.h>

class Logger {
	const char *logName;
	static bool opened;
	static std::string facility;

	void logas( int level, const char *fmt, va_list ap );
public:
	static void setSystemName( const char *facility );
	Logger(const char *name);
	virtual ~Logger();

	void log( int level, const char *fmt, ... );
	void error( const char *fmt, ... );
	void emerg( const char *fmt, ... );
	void critical( const char *fmt, ... );
	void alert( const char *fmt, ... );
	void warn( const char *fmt, ... );
	void notice( const char *fmt, ... );
	void info( const char *fmt, ... );
	void debug( const char *fmt, ... );
	void perror( const char *msg );
	void perror( const char *msg, int errorno );
};

#endif
