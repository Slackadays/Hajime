#!/bin/sh
### BEGIN INIT INFO
# Provides: hajime
# Required-Start:
# Required-Stop:
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: Start Hajime at boot time
# Description: Enable Hajime service.
### END INIT INFO
NAME="hajime"
PATH="/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin"
APPDIR={}
APPBIN={}/hajime
APPARGS=""
USER={}
GROUP={}
set -e
. /lib/lsb/init-functions
start() {
    printf "Starting Hajime..."
    start-stop-daemon --start --quiet --background --make-pidfile --pidfile /var/run/$NAME.pid --chuid $USER:$GROUP --exec $APPDIR/$APPBIN -- $APPARGS
    printf "done!"
}
stop() {
    printf "Stopping Hajime..."
    [ -z 'cat /var/run/$NAME.pid 2>/dev/null' ] || \
    kill -9 $(cat /var/run/$NAME.pid)
    [ -z `cat /var/run/$NAME.pid 2>/dev/null` ] || rm /var/run/$NAME.pid
    printf "...done!"
}
status() {
    status_of_proc -p /var/run/$NAME.pid "" $NAME && exit 0 || exit $?
}
case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        stop
        start
        ;;
    status)
        status
        ;;
    *)
        echo "Usage: $NAME (start|stop|restart|status)" >&2
        exit 1
        ;;
esac
exit 0