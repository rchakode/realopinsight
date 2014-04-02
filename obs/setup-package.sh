#!/bin/bash
# This script


VERSION=$1
DEFAULT_VERSION=X.Y.Z
OBS_PROJECT="home:ngrt4n"
OBS_PROJECT_DIR=/opt/dev/ngrt4n-qt/obs/$OBS_PROJECT
PACKAGE_NAME=RealOpInsight-$VERSION
DEFAULT_PACKAGE=RealOpInsight-$DEFAULT_VERSION
DOWNLOAD_URL="file:///opt/dev/ngrt4n-qt/$PACKAGE_NAME.tar.gz"
WORKING_DIR=/tmp/$PACKAGE_NAME

function usage
{
 echo "`basename $0` <version>"
}

if [ -z $VERSION ]; then
  echo "no version set"
  exit 1
fi

# Create the package directory from the default directory
cp -r $DEFAULT_PACKAGE $WORKING_DIR

cd $WORKING_DIR
FILES_TO_ALTER=$(grep -r "$DEFAULT_VERSION" . | awk -F : '{print $1}')

for file in $FILES_TO_ALTER; do
  sed --in-place "s/$DEFAULT_VERSION/$VERSION/g" $file
done

# Download RealOpInsight
curl -O $DOWNLOAD_URL

if [ $? -ne 0 ]; then
  echo "Can not download the tarball at $DOWNLOAD_URL"
#  exit 1
fi

# Create OBS package
if [ -d $OBS_PROJECT_DIR ]; then
  cd $OBS_PROJECT_DIR
  osc up
else
  cd /tmp/
  osc co $OBS_PROJECT
  cd $OBS_PROJECT
fi

osc mkpac $PACKAGE_NAME
cp -r $WORKING_DIR/* $PACKAGE_NAME
osc add $PACKAGE_NAME/*
osc commit -m "add package for $PACKAGE_NAME"

