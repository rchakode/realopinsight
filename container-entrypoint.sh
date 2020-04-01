#!/bin/bash

set -e
set -u
export LD_LIBRARY_PATH=/app
echo "[INIT] setup database"
/app/realopinsight-setupdb || true

echo "[INIT] starting realopinsight-reportd"
trap "[INIT] echo TRAPed signal" HUP INT QUIT KILL TERM
/app/realopinsight-reportd -t 5 &

echo "[INIT] starting realopinsight-server"
/app/realopinsight-server --docroot /app/www --http-address=0.0.0.0 --http-port 4583 --deploy / &

while sleep 60; do
  ps aux |grep realopinsight-reportd |grep -q -v grep
  REPORTD_STATUS=$?
  ps aux |grep realopinsight-server |grep -q -v grep
  SERVER_STATUS=$?
  # If the greps above find anything, they exit with 0 status
  # If they are not both 0, then something is wrong
  if [ $REPORTD_STATUS -ne 0 -o $SERVER_STATUS -ne 0 ]; then
    echo "One of the processes has already exited (SERVER_STATUS=$SERVER_STATUS, REPORTD_STATUS=$REPORTD_STATUS)."
    exit 1
  fi
done

echo "exited $0"