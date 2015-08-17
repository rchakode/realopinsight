# ------------------------------------------------------------------------ #
# File: make-ultimate-distrib.sh                                        #
# Copyright (c) 2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation : 10-03-2015                                                    #
#                                                                          #
# This Software is part of RealOpInsight Ultimate                          #
#                                                                          #
# Read legal notice & use terms: http://legal.realopinsight.com            #
#--------------------------------------------------------------------------#

#!/bin/bash

set -u
set -e

REALOPINSIGHT_USER=realopinsight
REALOPINSIGHT_GROUP=realopinsight

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
  case $path in
    /*) 
      ;;
    *) path=$PWD/$path
      ;;
  esac
  echo "$path"
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
      install -m 755 $f ${DISTRIB_PKG_NAME}/lib/
    fi
  done
}

create_bundle_fs_tree()
{ 
  if [ -d $DISTRIB_PKG_NAME ]; then
     echo "Directory already exist: ${DISTRIB_PKG_NAME}"
     exit 1
  fi
  install -d $DISTRIB_PKG_NAME
  install -d $DISTRIB_PKG_NAME/bin
  install -d $DISTRIB_PKG_NAME/sbin
  install -d $DISTRIB_PKG_NAME/lib
  install -d $DISTRIB_PKG_NAME/etc
  install -d $DISTRIB_PKG_NAME/var
  install -d $DISTRIB_PKG_NAME/www
  install -d $DISTRIB_PKG_NAME/sql
  install -d $DISTRIB_PKG_NAME/scripts
  install -d $DISTRIB_PKG_NAME/scripts/init.d
}


extract_binary_file()
{
  path=$1
  dest=$2
  extract_binary_deplibs $path
  install -m 755 $path ${DISTRIB_PKG_NAME}/$dest
}


extract_sbin_files()
{
  for f in $SBIN_FILES; do
    extract_binary_deplibs $f
  done 
}

copy_www_files()
{
  cp --recursive $REALOPINSIGHT_INSTALL_PREFIX/www/* ${DISTRIB_PKG_NAME}/www
}


extract_scripts()
{
  install -m 755 ${REALOPINSIGHT_INSTALL_PREFIX}/bin/realopinsight-backup ${DISTRIB_PKG_NAME}/bin
  install -m 755 ${REALOPINSIGHT_INSTALL_PREFIX}/bin/realopinsight-restore ${DISTRIB_PKG_NAME}/bin
  install -m 755 $HTTP_SERVER_INIT_SCRIPT ${DISTRIB_PKG_NAME}/scripts/init.d/`basename $HTTP_SERVER_INIT_SCRIPT`
  install -m 755 $REPORTD_INIT_SCRIPT ${DISTRIB_PKG_NAME}/scripts/init.d/`basename $REPORTD_INIT_SCRIPT`
}


copy_config_files()
{
  install -m 644 ${REALOPINSIGHT_INSTALL_PREFIX}/etc/wt_config.xml ${DISTRIB_PKG_NAME}/etc/
}

get_user_group()
{
  USER=$1
  GROUP="UNSET"
  if [ ! -z "$1" ]; then
    GROUP=$(id $USER | cut -d' ' -f2 | sed 's/[=(| )]/ /g' | awk '{print $3}')
    echo $GROUP
  fi
}


move_to_working_dir()
{
  cd /tmp
}


copy_readme_files()
{
  cp ${CONTRIBS_DIR}/README_INSTALL ${DISTRIB_PKG_NAME}/
}


copy_sql_patch_files()
{
  cp -r ${CONTRIBS_DIR}/sql/*.sql ${DISTRIB_PKG_NAME}/sql/
}



# check usage
check_usage $@

# set variables
REALOPINSIGHT_VERSION=$1
REALOPINSIGHT_INSTALL_PREFIX=$(get_absolute_path $2)
REALOPINSIGHT_WWW_HOME=$(get_absolute_path $3)
OS_NAME=$(echo `lsb_release -s -i` | sed 's/ //g' | tr '[:upper:]' '[:lower:]')
OS_VERSION=$(echo `lsb_release -s -r` | awk '{sub("\\.", "", $0);sub("-", "", $0); sub(" ", "", $0);print $1}')
DISTRIB_PKG_NAME="realopinsight-ultimate-${REALOPINSIGHT_VERSION}.${OS_NAME}${OS_VERSION}.`uname -m`"
INSTALL_MANIFEST="${DISTRIB_PKG_NAME}/INSTALL.MANIFEST"
INSTALLATION_FILE="${CONTRIBS_DIR}/distrib/install-ultimate-distrib.sh"
HTTP_SERVER_BINARY="${REALOPINSIGHT_INSTALL_PREFIX}/sbin/realopinsight-server"
REPORTD_BINARY="${REALOPINSIGHT_INSTALL_PREFIX}/sbin/realopinsight-reportd"

HTTP_SERVER_INIT_SCRIPT="/etc/init.d/realopinsight-server"
REPORTD_INIT_SCRIPT="/etc/init.d/realopinsight-reportd"
SQLITE3=$(which sqlite3 || echo "")

if [ -z "$SQLITE3" ]; then
  echo "sqlite3: file not found"
  exit 1
fi


# start processing
check_file $HTTP_SERVER_BINARY
check_file $REPORTD_BINARY
check_file $HTTP_SERVER_INIT_SCRIPT
check_file $REPORTD_INIT_SCRIPT
check_file $INSTALLATION_FILE
move_to_working_dir
create_bundle_fs_tree
copy_config_files
copy_www_files
extract_binary_file $HTTP_SERVER_BINARY sbin
extract_binary_file $REPORTD_BINARY sbin
extract_binary_file ${SQLITE3} bin

extract_scripts
copy_sql_patch_files
copy_readme_files

echo "REALOPINSIGHT_VERSION=$REALOPINSIGHT_VERSION" >> $INSTALL_MANIFEST
echo "REALOPINSIGHT_INSTALL_PREFIX=${REALOPINSIGHT_INSTALL_PREFIX}" >> $INSTALL_MANIFEST
echo "REALOPINSIGHT_USER=$REALOPINSIGHT_USER" >> $INSTALL_MANIFEST
echo "REALOPINSIGHT_GROUP=$REALOPINSIGHT_GROUP" >> $INSTALL_MANIFEST

# make tarball
install -m 755 $INSTALLATION_FILE ${DISTRIB_PKG_NAME}/
tar zcf ${DISTRIB_PKG_NAME}.tar.gz ${DISTRIB_PKG_NAME}

exit 0
