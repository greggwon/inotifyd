#include <string>
#include "logger.hpp"
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

bool Logger::dbg = false;
FILE *Logger::logf = NULL;
std::string Logger::logfile = "logging.log";
std::string Logger::logdir = ".";
int Logger::revCnt = 10;
size_t Logger::maxSize = 20*1024*1024;

std::string levelName( int lvl ) {
	switch(lvl) {
		case LOG_ERR: return "ERROR";
		case LOG_EMERG: return "EMERG";
		case LOG_CRIT: return "CRIT";
		case LOG_ALERT: return "ALERT";
		case LOG_WARNING: return "WARNING";
		case LOG_NOTICE: return "NOTICE";
		case LOG_INFO: return "INFO";
		case LOG_DEBUG: return "DEBUG";
	}
	return "UNK";
}

struct LogLevel {
	const char *nm;
	int lvl;
};

static struct LogLevel levels[] = {
	{"ERR     ", LOG_ERR},
	{"EMERG   ", LOG_EMERG},
	{"CRIT    ", LOG_CRIT},
	{"ALERT   ", LOG_ALERT},
	{"WARNING ", LOG_WARNING},
	{"NOTICE  ", LOG_NOTICE},
	{"INFO    ", LOG_INFO},
	{"DEBUG   ", LOG_DEBUG},
	{NULL      , 0}
};

void Logger::setMaxFileSize( size_t size ) {
	maxSize = size;
}

void Logger::setMaxLogInst( int cnt ) {
	revCnt = cnt;
}

void Logger::setLogdir( std::string &nm ) {
	logdir = nm;
	if( logf != NULL ) {
		fclose(logf);
		logf = NULL;
	}
}

void Logger::setDefaultLevel( const char *name ) {
	for( int i = 0; levels[i].nm != NULL; ++i ) {
		if( strncmp( levels[i].nm, name, strlen(name) ) == 0 ) {
			defLevel = levels[i].lvl;
			return;
		}
	}
	fprintf(stderr, "unknown log level not set: %s\n", name );
}

void Logger::setLogfile( std::string &nm ) {
	logfile = nm;
	if( logf != NULL ) {
		fclose(logf);
		logf = NULL;
	}
}

void Logger::shuffleDown() {
	if(dbg) fprintf(stderr, "Shuffling files down");
	if( logf != NULL ) {
		if(dbg) fprintf(stderr, "closing log");
		fclose(logf);
		logf = NULL;
	}
	for( int i = revCnt-1; i > 0; --i ) {
		if(dbg) fprintf( stderr, "unlinking %s\n", logPath(logfile, i).c_str());
		unlink(logPath(logfile, i).c_str());
		if( i < revCnt ) {
			if(dbg) fprintf( stderr, "renaming %s to %s\n", logPath(logfile, i-1).c_str(), logPath(logfile, i).c_str());
			int st =rename( logPath(logfile,i-1).c_str(), logPath(logfile,i).c_str() );
			if(dbg) fprintf( stderr, "renamed %s to %s: %d\n", logPath(logfile, i-1).c_str(), logPath(logfile, i).c_str(), st);
		}
	}

	chkOpenLog();
}

void Logger::chkOpenLog() {
	if( logf != NULL ) {
		fclose(logf);
		logf = NULL;
	}
	if(dbg) fprintf(stderr, "using logging path %s and file %s to log into: %s\n", logdir.c_str(), logfile.c_str(), logPath(logfile).c_str());
	//fflush(stderr);
	logf = fopen( logPath(logfile).c_str(), "a" );
}

std::string Logger::logPath( std::string &file, int inst) {
	char buf[10];
	snprintf(buf,sizeof(buf),"%d", inst);
	return logdir + "/" + file + "."+buf;
}

void Logger::checkSizeAndLog( const char *d ) {
	struct stat sbuf;
	time_t now;
	if( stat( logPath(logfile).c_str(), &sbuf  ) == 0 ) {
		if(dbg) fprintf(stderr, "found %s to be %ld of %ld\n", logPath(logfile).c_str(), sbuf.st_size, maxSize );
		if( sbuf.st_size > maxSize ) {
			shuffleDown();
		}
	}
	chkOpenLog();
	time(&now);
	fprintf(logf,"%.24s: ",ctime(&now));
	fputs( d, logf );
	fputc( '\n', logf );
	fflush(logf);
}

void Logger::logas( int level, const char *fmt, va_list ap )
{
	if( level > this->level ) {
		return;
	}
	int sz = strlen(fmt) + 25+ 115 + strlen(logName);
	char *buf = (char *)malloc( sz );
	snprintf( buf, sz, "%s: [%s] %s", logName, levelName(level).c_str(), fmt );
	char d[4096];
	vsnprintf( d, sizeof(d), buf, ap );
	syslog( LOG_USER|level, d );
	checkSizeAndLog( d );
	if( withFprintfStderr ) {
		time_t now;
		time(&now);
		fprintf(stderr,"%.24s: ",ctime(&now));
		fputs(d, stderr);
		fputc('\n', stderr);
	}
	free(buf);
}

bool Logger::opened = false;
std::string Logger::facility = getenv("USER") ? getenv("USER") : "radio";
int Logger::defLevel = LOG_INFO;
bool Logger::withFprintfStderr = false;

void Logger::setSystemName( const char *name ) {
	Logger::facility = name;
	if(dbg) fprintf(stderr, "set facility to %s\n", Logger::facility.c_str());
	if( Logger::opened ) {
		Logger::closeLog();
		Logger::openLog();
	}
}

Logger::Logger(const char *name) {
	logName = strdup(name);
	Logger::openLog();
	level = Logger::defLevel;
}

void Logger::closeLog() {
	closelog();
	opened = false;
}

void Logger::openLog() {
	if( Logger::opened == false ) {
		if(dbg) fprintf( stderr, "openlog with: %s facility\n", Logger::facility.c_str() );
		openlog( Logger::facility.c_str(), LOG_CONS|LOG_PID, LOG_USER );
		opened = true;
	}
}

void Logger::setLevel( int lvl ) {
	level = lvl;
}

int Logger::getLevel() {
	return level;
}

Logger::~Logger()
{
	free(logName);
}

void Logger::setWithFprintfStderr( bool how ) {
	withFprintfStderr = how;
}

void Logger::log( int level, const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(level, fmt, ap );
	va_end(ap);
}

void Logger::error( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_ERR, fmt, ap );
	va_end(ap);
}


void Logger::emerg( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_EMERG, fmt, ap );
	va_end(ap);
}

void Logger::critical( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_CRIT, fmt, ap );
	va_end(ap);
}

void Logger::alert( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_ALERT, fmt, ap );
	va_end(ap);
}

void Logger::warn( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_WARNING, fmt, ap );
	va_end(ap);
}

void Logger::notice( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_NOTICE, fmt, ap );
	va_end(ap);
}

void Logger::info( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_INFO, fmt, ap );
	va_end(ap);
}

void Logger::debug( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	logas(LOG_DEBUG, fmt, ap );
	va_end(ap);
}

void Logger::perror( const char *fmt, ... )
{
	char buf[4096];
	char *err = strerror(errno);
	va_list ap;
	va_start( ap, fmt );
	snprintf( buf, sizeof(buf), "%s: %s", fmt, err );
	logas( LOG_ALERT, buf, ap );
	va_end(ap);
}

void Logger::perror( int errorno,  const char *fmt, ...)
{
	char buf[4096];
	char *err = strerror(errorno);
	va_list ap;
	va_start( ap, fmt );
	snprintf( buf, sizeof(buf), "%s: %s", fmt, err );
	logas( LOG_ALERT, buf, ap );
	va_end(ap);
}
