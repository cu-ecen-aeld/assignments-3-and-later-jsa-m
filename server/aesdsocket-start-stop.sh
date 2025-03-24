#In Linux, start-stop-daemon is a utility designed to manage the creation 
#and termination of system-level daemons (background processes). 
#It's commonly used in init scripts (like those in /etc/init.d/) to ensure 
#that a daemon starts, stops, or restarts in a controlled manner.

case "$1" in
    start)
        echo "Starting aesdsocket"
        start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
        #-- -d → Passes -d (likely a "daemon mode" flag) to aesdsocket.
        ;;
    stop)
        echo "Stopping aesdsocket"
        start-stop-daemon -K --signal TERM -n aesdsocket
        ;;
    *)
        echo "Usage: $0 {start,stop}"
        exit 1
esac

exit 0

    