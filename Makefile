CPPFLAGS=-std=gnu++11 -g

OFILES=\
	INotifyd.o \
	logger.o \
	main.o

LDFLAGS=-g

inotify: ${OFILES}
	${CXX} -o $@ $(OFILES) $(LDFLAGS)

INotifyd.o: INotifyd.hpp WatchList.hpp logger.hpp
main.o: INotifyd.hpp WatchList.hpp logger.hpp

install: ${HOME}/bin/inotify

${HOME}/bin/inotify: inotify ${HOME}/bin
	cp inotify ${HOME}/bin/inotify

${HOME}/bin:
	mkdir -p ${HOME}/bin || exit 1

clean: 
	rm -f ${OFILES} inotify
