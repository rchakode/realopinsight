#!/bin/bash

set -e

trap "echo TRAPed signal" HUP INT QUIT KILL TERM

# start Apache service in background
/usr/sbin/apachectl start

# start realopinsight-reportd service
service realopinsight-reportd start

# start a bash session
/bin/bash

#echo "[Type 'quit' + Enter to exit] or run 'docker stop <container>'"
#while [ 1 ]; do
#  read rep
#  if [ "$rep" = "quit" ]; then
#    break
#  fi
#done

echo "stopping services"
/usr/sbin/apachectl stop

echo "stopping realopinsight-reportd service"
service realopinsight-reportd stop


echo "exited $0"
