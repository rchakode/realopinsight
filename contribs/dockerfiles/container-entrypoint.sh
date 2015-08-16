#!/bin/bash

set -e

trap "echo TRAPed signal" HUP INT QUIT KILL TERM


# start realopinsight-reportd service
service realopinsight-reportd start

# start a bash session
/bin/bash


echo "stopping realopinsight-reportd service"
service realopinsight-reportd stop


echo "exited $0"
