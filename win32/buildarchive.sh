#!/bin/bash

set -e

VERSION=1.0
RELEASE=1
APP=ngrt4n
TARGET_DIR=${APP}-${VERSION}
TARBALL=${APP}-${VERSION}.${RELEASE}.tar.gz
ABS_TARGET_DIR=/tmp/${TARGET_DIR}
DOC_DIR=${ABS_TARGET_DIR}/doc

rm -rf ${ABS_TARGET_DIR}
mkdir ${ABS_TARGET_DIR}
mkdir ${DOC_DIR}

cp -r doc/html ${DOC_DIR}
cp -r include \
	src \
	examples \
	images \
	${APP}.pro \
	${APP}.qrc \
	LICENSE \
	ChangeLog \
	README \
	install.sh ${ABS_TARGET_DIR}

cd ${ABS_TARGET_DIR}/.. 
tar czf ${TARBALL} ${TARGET_DIR}
echo "Archived created => /tmp/${TARBALL}" 
