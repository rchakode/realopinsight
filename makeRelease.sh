#!/bin/bash

set -e 
set -u

version=$1

lrelease ngrt4n.pro
qmake ngrt4n.pro -r  CONFIG+=gui-base CONFIG+=dist 1>/dev/null
make dist 1>/dev/null

git pull --rebase
<<<<<<< HEAD
git tag $version master
git push github-upstream $version
=======
<<<<<<< HEAD
git tag --force $1 master
git push --force --tags
=======
git tag $version master
git push github-upstream $version
>>>>>>> a209c06... updated release script
>>>>>>> b777554... fixed conflict after merging release script
