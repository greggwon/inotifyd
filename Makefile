CPPFLAGS=-std=gnu++11 -g

OFILES=\
	INotifyd.o \
	logger.o \
	WatchList.o \
	main.o

LDFLAGS=-g
MANDIR=/usr/local/share/man/man8/

EXE=/usr/local/bin
SYSTEMD=/etc/systemd/user

inotify: ${OFILES}
	${CXX} -o $@ $(OFILES) $(LDFLAGS)

INotifyd.o: INotifyd.hpp WatchList.hpp logger.hpp
main.o: INotifyd.hpp WatchList.hpp logger.hpp
WatchList.o: INotifyd.hpp WatchList.hpp logger.hpp

install: ${HOME}/bin/inotify

service: ${EXE}/inotifyd ${SYSTEMD}/inotifyd.service ${MANDIR}/inotifyd.8
	systemctl enable inotifyd

${MANDIR}/inotifyd.8: inotifyd.8
	cp $< $@

${SYSTEMD}/inotifyd.service: inotifyd.service
	cp inotifyd.service ${SYSTEMD}/inotifyd.service

${EXE}/inotifyd: inotify
	cp inotify ${EXE}/inotifyd

${HOME}/bin/inotify: inotify ${HOME}/bin
	cp inotify ${HOME}/bin/inotify

${HOME}/bin:
	mkdir -p ${HOME}/bin || exit 1

clean: 
	rm -f ${OFILES} inotify
