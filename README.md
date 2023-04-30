#### Inotify
This program can be used as a command line tool or as a systemctl service to keep user/group and permission settings sane on specified directory structures.

### Building INotifyd
make(1) is used to build inotify.  The following targets are defined.

## inotify
Compile the source code for the application to create the inotify executable.

## install
Build the inotify executable and copy to the users ${HOME}/bin directory.

## service
Build the inotify executable and copy it to /usr/local/bin.  Copy a service definition to /etc/systemd and enable the service.

## clean
Remove compiler generated output files from the local directory tree
