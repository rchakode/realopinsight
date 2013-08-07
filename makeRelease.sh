#!/bin/bash

set -e 
set -u

qmake /opt/dev/ngrt4n-qt/ngrt4n.pro -r -spec linux-g++ CONFIG+=config-dist 1>/dev/null
make dist 1>/dev/null

git checkout windows
git pull --rebase
git tag $1 master
git tag $1-windows windows
git push --tags
git checkout master
