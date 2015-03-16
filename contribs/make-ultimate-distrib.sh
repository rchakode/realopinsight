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
      install -m 755 $f $DISTRIB_PKG_NAME/lib/
    fi
  done
}

create_bundle_fs_tree()
{ 
  if [ -d $DISTRIB_PKG_NAME ]; then
     echo "Directory already exist: $DISTRIB_PKG_NAME"
     exit 1
  fi
  mkdir -p $DISTRIB_PKG_NAME/{etc,lib,sbin,var,www,scripts/init.d}
}


extract_binary_file()
{
  path=$1
  dest=$2
  extract_binary_deplibs $path
  install -m 755 $path $DISTRIB_PKG_NAME/$dest
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
  cp --recursive $WWW_DIR/* $DISTRIB_PKG_NAME/www
}


copy_apache_config()
{
  if [ -d ${APACHE_CONFIG_DIR}/conf-available ]; then
    WWW_CONFIG_DIR="${APACHE_CONFIG_DIR}/conf-available"
  else
    WWW_CONFIG_DIR="${APACHE_CONFIG_DIR}/conf.d"
  fi
  REALOPINSIGHT_WWW_CONFIG_PATH=${WWW_CONFIG_DIR}/realopinsight-ultimate.conf
  install -m 644 $REALOPINSIGHT_WWW_CONFIG_PATH $DISTRIB_PKG_NAME/etc/
}


extract_init_scripts()
{
  install -m 755 $REPORTD_INIT_SCRIPT $DISTRIB_PKG_NAME/scripts/init.d/$REPORTD_INIT_SCRIPT
}

copy_wt_config()
{
  install -m 644 $REALOPINSIGHT_HOME/etc/wt_config.xml $DISTRIB_PKG_NAME/etc/
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


move_to_working_dir()
{
  cd /tmp
}

# check usage
check_usage $@

# set variables
VERSION=$1
REALOPINSIGHT_HOME=$(get_absolute_path $2)
WWW_DIR=$(get_absolute_path $3)
REALOPINSIGHT_HOME="/opt/realopinsight"
DISTRIB_PKG_NAME="realopinsight-ultimate-distrib-${VERSION}-`uname -m`"
INSTALL_MANIFEST="$DISTRIB_PKG_NAME/INSTALL.MANIFEST"
INSTALLATION_FILE="/tmp/install-ultimate-distrib.sh"
APACHE_CONFIG_DIR="/etc/apache2"
FCGI_FILE="${WWW_DIR}/realopinsight.fcgi"
REPORTD_FILE="$REALOPINSIGHT_HOME/sbin/realopinsight-reportd"
REPORTD_INIT_SCRIPT="/etc/init.d/realopinsight-reportd"
WWW_USER=$(get_www_user)
WWW_GROUP=$(get_user_group $WWW_USER)



# start processing
check_file $FCGI_FILE
check_file $REPORTD_FILE
check_file $REPORTD_INIT_SCRIPT
check_file $INSTALLATION_FILE
move_to_working_dir
create_bundle_fs_tree
copy_config_files
copy_www_files
extract_binary_file $REPORTD_FILE sbin
extract_init_scripts

echo "REALOPINSIGHT_HOME=$REALOPINSIGHT_HOME" >> $INSTALL_MANIFEST
echo "REALOPINSIGHT_WWW_HOME=$WWW_DIR" >> $INSTALL_MANIFEST
echo "REALOPINSIGHT_WWW_CONFIG_PATH=$REALOPINSIGHT_WWW_CONFIG_PATH" >> $INSTALL_MANIFEST
echo "WWW_USER=$WWW_USER" >> $INSTALL_MANIFEST
echo "WWW_GROUP=$WWW_GROUP" >> $INSTALL_MANIFEST

# make tarball
install -m 755 $INSTALLATION_FILE $DISTRIB_PKG_NAME/
tar zcf $DISTRIB_PKG_NAME.tar.gz $DISTRIB_PKG_NAME

exit 0