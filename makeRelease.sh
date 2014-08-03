#!/bin/bash

set -e 
set -u

version=$1

lrelease ngrt4n.pro
qmake ngrt4n.pro -r  CONFIG+=gui-base CONFIG+=dist 1>/dev/null
make dist 1>/dev/null

git pull --rebase
git tag $version master
git push github-upstream $version
