#!/bin/bash

set -u
set -e



print_usage()
{
  echo "`basename $0` <version> <install_prefix> <www_dir>"
}


check_usage()
{
  if [ $# -ne 3 ]; then
    print_usage
    exit 1
  fi
}



get_absolute_path()
{
  path=$1
  if [ "${path:0:1}" = "/" ]; then
    echo "$path"
  else
    echo "$PWD/$path"
  fi
}


check_file()
{
  path=$1
  if [ ! -e $path ]; then
    echo "File not found: $path"
    exit 1
  fi
}


extract_binary_deplibs()
{
  BIN_FILE=$1
  for f in $(ldd $BIN_FILE | awk '{print $3}'); do
    if [ -e $f ]; then
      install -m 755 $f $TARGET_DIR/lib/
    fi
  done
}

create_bundle_fs_tree()
{ 
  if [ -d $TARGET_DIR ]; then
     echo "Directory already exist: $TARGET_DIR"
     exit 1
  fi
  mkdir -p $TARGET_DIR/{etc,lib,sbin,www}
}


extract_binary_file()
{
  path=$1
  dest=$2
  extract_binary_deplibs $path
  install -m 755 $path $TARGET_DIR/$dest
}
extract_sbin_files()
{
  for f in $SBIN_FILES; do
    extract_binary_deplibs $f
  done 
}


extract_fcgi_deps()
{
  FCGI_FILE="${WWW_DIR}/realopinsight.fcgi"
  if [ ! -e $FCGI_FILE ]; then
    echo "File not found: $FCGI_FILE"
    exit 1
  fi
  extract_binary_deplibs $FCGI_FILE
}


copy_www_files()
{
  extract_fcgi_deps
  cp --recursive $WWW_DIR/* $TARGET_DIR/www
}

copy_apache_config()
{
  if [ -d ${APACHE_CONFIG_DIR}/conf-available ]; then
    WWW_CONFIG_DIR="${APACHE_CONFIG_DIR}/conf-available"
  else
    WWW_CONFIG_DIR="${APACHE_CONFIG_DIR}/conf.d"
  fi
  install -m 644 $WWW_CONFIG_DIR/realopinsight-ultimate.conf $TARGET_DIR/etc/
}

copy_wt_config()
{
  install -m 644 $INSTALL_PREFIX/etc/wt_config.xml $TARGET_DIR/etc/
}


copy_config_files()
{
  copy_apache_config
  copy_wt_config
}


get_www_user()
{
  WWW_USER=$(ps aux | egrep '(apache|httpd)' | egrep -v grep | grep -v root | tail -1 | cut -d' ' -f1)
  if [ -z $WWW_USER ]; then
    echo "Not www user found"
    exit 1
  fi
  echo $WWW_USER
}

get_user_group()
{
  if [ -z "$1" ]; then
    echo "Not user given"
    exit 1
  fi

  USER=$1
  GROUP=$(id $USER | cut -d' ' -f2 | awk -F "(" '{sub(")", "", $2); print $2}')
  echo $GROUP
}


check_usage $@

VERSION=$1
INSTALL_PREFIX=$(get_absolute_path $2)
WWW_DIR=$(get_absolute_path $3)

INSTALL_PREFIX="/opt/realopinsight"
BUNDLE_NAME="realopinsight-ultimate-bundle-${VERSION}-`uname -m`"
TARGET_DIR="/tmp/$BUNDLE_NAME/"
APACHE_CONFIG_DIR="/etc/apache2"
INSTALL_MANIFEST="$TARGET_DIR/INSTALL.MANIFEST"
FCGI_FILE="${WWW_DIR}/realopinsight.fcgi"
REPORTD_FILE="$INSTALL_PREFIX/sbin/realopinsight-reportd"

WWW_USER=$(get_www_user)
WWW_GROUP=$(get_user_group $WWW_USER)

check_file $FCGI_FILE
check_file $REPORTD_FILE
create_bundle_fs_tree
copy_config_files
copy_www_files
extract_binary_file $REPORTD_FILE sbin


echo "INSTALL_PREFIX=$INSTALL_PREFIX" >> $INSTALL_MANIFEST
echo "WWW_INSTALL_PREFIX=$WWW_DIR" >> $INSTALL_MANIFEST
echo "WWW_CONFIG_ROOT=$WWW_CONFIG_DIR" >> $INSTALL_MANIFEST
echo "WWW_USER=$WWW_USER" >> $INSTALL_MANIFEST
echo "WWW_GROUP=$WWW_GROUP" >> $INSTALL_MANIFEST


exit 0