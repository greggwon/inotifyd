#include "csvload.hpp"

CSVload::CSVload() : loaded() {
}

CSVload::~CSVload() {
}

static int permsParse( const char *str ) {
	int p;
	if( sscanf( str, "%o", &p ) == 1 )
		return p;

	return -2;
}

static char *nextToken() {
	char *p = strtok(NULL,",:");
	if( p != NULL )
		return strdup(p);
	return NULL;
}

std::list<DirPerms*> *CSVload::getLoaded() {
	return &loaded;
}

int CSVload::loadFile( const char * inputfile ) {
	int rc = 0;
	FILE *fp = fopen(inputfile,"r");
	if( fp == NULL ) {
		perror( inputfile );
		return 1;
	}

	char buf[4096];

	while( fgets( buf, sizeof(buf), fp ) != NULL ) {
		if( buf[0] == '#' ) {
			continue;
		}
		if( buf[strlen(buf)-1] == '\n' ) {
			buf[strlen(buf)-1] = '\0';
		}
		char *line = strdup(buf);
		char *t = strtok( buf, ",:" );
		if( t == NULL ) {
			fprintf(stderr, "Ignoring line: \"%s\"\n", line );
			free( line );
			continue;
		}
		char* dir = strdup(t);
		char* user = nextToken();
		char* group = nextToken();
		char* dirp = nextToken();
		char* file = nextToken();
		if( dir != NULL && user != NULL && group != NULL && dirp != NULL && file != NULL ) {
			int dirPerms = permsParse(dirp);
			int filePerms = permsParse(file);
			if( dirPerms == -2 ) {
				fprintf(stderr, "Bad directory perms for %s: %s\n", dir, dirp );
				rc++;
			}
			if( filePerms == -2 ) {
				fprintf(stderr, "Bad file perms for %s: %s\n", dir, file );
				rc++;
			}
				
			DirPerms *d = new DirPerms( dir, user, group, dirPerms, filePerms );
			loaded.push_back( d );
		} else {
			fprintf(stderr, "Ignoring malformated line: \"%s\"\n", line );
			rc++;
		}
		if( dir   != NULL ) free( dir );
		if( user  != NULL ) free( user );
		if( group != NULL ) free( group );
		if( dirp  != NULL ) free( dirp );
		if( file  != NULL ) free( file );
		free( line );
	}
	fclose(fp);
	return rc;
}
