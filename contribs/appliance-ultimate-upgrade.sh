#!/bin/bash

REALOPINSIGHT_WWW_USER=www-data 
REALOPINSIGHT_WWW_GROUP=www-data

TARGET_VERSION=2014b3
PATCH_TARBALL=patch_${TARGET_VERSION}.tar.gz


check_exit_code()
{
  if [ -$? -eq 0 ]; then
    echo "done"
  else
    echo "failed"
    exit 1
  fi
}

echo "DEBUG : Upgrading RealOpInsight Ultimate to version ${TARGET_VERSION}..."

echo -n "DEBUG : Shutting down Apache..."
/etc/init.d/apache2 stop
check_exit_code


echo -n "DEBUG : Updating database..."
su - ${REALOPINSIGHT_WWW_USER} -c'echo "ALTER TABLE user ADD COLUMN authsystem int not null default 0;" \
                | sqlite3 /opt/realopinsight/data/realopinsight.db'

check_exit_code

echo -n "DEBUG : Updating binaries from patch ${PATCH_TARBALL}..."


echo -n "DEBUG : Restarting Apache..."
/etc/init.d/apache2 start
check_exit_code

echo "DEBUG: Upgrade completed."


