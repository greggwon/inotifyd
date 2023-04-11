#include <string>
#include "logger.hpp"

static bool dbg = false;

void Logger::logas( int level, const char *fmt, va_list ap )
{
	int sz = strlen(fmt) + 5 + strlen(logName);
	char *buf = (char *)malloc( sz );

	snprintf( buf, sz, "%s: %s", logName, fmt );
	vsyslog( LOG_USER|level, buf, ap );
}

bool Logger::opened = false;
std::string Logger::facility = getenv("USER");

void Logger::setSystemName( const char *facility ) {
	Logger::facility = facility;
	fprintf(stderr, "set facility to %s\n", Logger::facility.c_str());
	if( Logger::opened ) {
		closelog();
		fprintf( stderr, "change openlog to %s facility\n", Logger::facility.c_str() );
		openlog( Logger::facility.c_str(), LOG_CONS|LOG_PID, LOG_USER );
	}
}

Logger::Logger(const char *name) {
	logName = name;
	if( !Logger::opened ) {	
		if(dbg) fprintf( stderr, "openlog with: %s facility\n", Logger::facility.c_str() );
		openlog( Logger::facility.c_str(), LOG_CONS|LOG_PID, LOG_USER );
		Logger::opened = true;
	}
}

Logger::~Logger()
{
	closelog();
}

void Logger::log( int level, const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(level, fmt, ap );
}

void Logger::error( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_ERR, fmt, ap );
}


void Logger::emerg( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_EMERG, fmt, ap );
}

void Logger::critical( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_CRIT, fmt, ap );
}

void Logger::alert( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_ALERT, fmt, ap );
}

void Logger::warn( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_WARNING, fmt, ap );
}

void Logger::notice( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_NOTICE, fmt, ap );
}

void Logger::info( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_INFO, fmt, ap );
}

void Logger::debug( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_DEBUG, fmt, ap );
}

void Logger::perror( const char *msg )
{
	perror( msg, errno );
}

void Logger::perror( const char *msg, int errorno )
{
	alert( "%s: %s", strerror(errorno), msg );
}
