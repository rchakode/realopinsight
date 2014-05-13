#!/bin/bash

set -e 
set -u

lrelease ngrt4n.pro
qmake ngrt4n.pro -r  CONFIG+=gui-base CONFIG+=dist 1>/dev/null
make dist 1>/dev/null

git pull --rebase
git tag $1 master
git push --tags
