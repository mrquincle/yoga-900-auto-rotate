#!/bin/bash

case "$1" in 
start)
   auto-rotate &
   echo $!>/var/run/auto-rotate.pid
   ;;
stop)
   kill `cat /var/run/auto-rotate.pid`
   rm /var/run/auto-rotate.pid
   ;;
restart)
   $0 stop
   $0 start
   ;;
status)
   if [ -e /var/run/auto-rotate.pid ]; then
      echo auto-rotate is running, pid=`cat /var/run/auto-rotate.pid`
   else
      echo auto-rotate is NOT running
      exit 1
   fi
   ;;
*)
   echo "Usage: $0 {start|stop|status|restart}"
esac

exit 0 
