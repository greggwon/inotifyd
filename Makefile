CFLAGS=-std=c99

OFILES=\
	INotifyd.o \
	main.o

LDFLAGS=-g

inotify: ${OFILES}
	${CXX} -o $@ $(OFILES) $(LDFLAGS)

clean:
	rm -f inotify

install: ${HOME}/bin/inotify

${HOME}/bin/inotify: inotify ${HOME}/bin
	cp inotify ${HOME}/bin/inotify

${HOME}/bin:
	mkdir -p ${HOME}/bin || exit 1
