#!/bin/bash
# ------------------------------------------------------------------------ #
# File: install-ultimate-distrib.sh                                        #
# Copyright (c) 2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation : 10-03-2015                                                    #
#                                                                          #
# This Software is part of RealOpInsight Ultimate                          #
#                                                                          #
# Read legal notice & use terms: http://legal.realopinsight.com            #
#--------------------------------------------------------------------------#


set -u
set -e

GREEN_COLOR="\\033[1;32m"
DEFAULT_COLOR="\\033[0;39m"
RED_COLOR="\\033[1;31m"


REALOPINSIGHT_USER=realopinsight
REALOPINSIGHT_GROUP=$REALOPINSIGHT_USER


# Source the installation manifest and set misc variables
if [ ! -e ./INSTALL.MANIFEST ]; then
  echo "INSTALL.MANIFEST not found"
  exit 1
else
  . ./INSTALL.MANIFEST  # source path settings. e.g. WWW_USER
  SQLITE3="LD_LIBRARY_PATH=$REALOPINSIGHT_INSTALL_PREFIX/lib $REALOPINSIGHT_INSTALL_PREFIX/bin/sqlite3"
  REALOPINSIGHT_BACKUP_FILE=backup_`date +%Y-%M-%d_%H-%M-%S`.tar.gz
fi


# Print help
#
print_usage()
{
  echo
  echo "Usage: `basename $0`"
}


prompt_copyright()
{ 
  echo
  echo "Thanks for choosing RealOpInsight Ultimate, version $REALOPINSIGHT_VERSION."
  echo
  echo "By installing this software, you accept its terms of use."
  echo
  echo "  Type 'y' to accept this license offer."
  echo "  Type 'n' to decline this license offer."
  echo "  Type 'r' to read the license offer."
  echo
  while true; do
    read -p "Do you accept the terms of the License ? y/N/r " rep
    case $rep in
	  r) echo " => Please read the RealOpInsight Ultimate terms of use at the following URL: http://legal.realopinsight.com" ; echo ;;
	  y) break ;;
	  *) exit 0 ;; 
    esac
  done
}


check_realopinsight_user()
{
  id $REALOPINSIGHT_USER || useradd $REALOPINSIGHT_USER
}


check_root_user()
{
  if [ `id -u` -ne 0 ]; then 
    echo "ERROR: root privileges are required"
    exit 1	
  fi
}




install_initd_scripts()
{
  echo "DEBUG: Setting Up Init Scripts and Starting Services.."
  install -m 755 scripts/init.d/realopinsight-server /etc/init.d/
  install -m 755 scripts/init.d/realopinsight-reportd /etc/init.d/
  update-rc.d realopinsight-server defaults
  update-rc.d realopinsight-reportd defaults
}

stop_services()
{
  service realopinsight-server stop || true
  service realopinsight-reportd stop || true
}


start_services()
{
  service realopinsight-reportd start
  service realopinsight-server start
}

check_graphviz()
{
  echo "DEBUG: Checking Graphviz => ${DOT:=`which dot`}... "
  if [ -z "$DOT" ]; then
    echo "ERROR : Graphviz not found. To install it: sudo apt-get install graphviz"
    exit 1
  fi
}


check_prerequisites()
{
  echo "DEBUG: checking prerequisites..."
  check_graphviz
}

prompt_to_get_current_db_version()
{
  echo
  echo "Which version of RealOpInsight Ultimate is currently installed ?"
  echo
  echo "    1) 2014b7"
  echo "    2) 2014b8"
  echo "    3) 2015r1/2015r2"
  echo "    4) 2015r3.b1/2015r3"
  echo "    q|Q) Quit"
  echo

  while true; do
    read -p "Type a response: " rep
    case $rep in
         1) INSTALLED_VERSION="2014b7"
             break
         ;;
         2) INSTALLED_VERSION="2014b8"
             break
         ;;
         3) INSTALLED_VERSION="2015r1_XOR_2015r2"
             break
         ;;
         4) INSTALLED_VERSION="2015r3b1_XOR_2015r3"
             break
         ;;
         q|Q) exit 0
         ;;
         *) echo -n "Invalid input! ";;
    esac
  done
}


update_db_2014b8()
{
  echo -n "DEBUG: Updating database to 2014b8..."
  su - ${WWW_USER} -c "${SQLITE3} /opt/realopinsight/data/realopinsight.db < $PWD/sql/update_2014b8.sql"
}


update_db_2015r1()
{
  echo -n "DEBUG: Updating database to 2015r1..."
  su - ${WWW_USER} -c "${SQLITE3} /opt/realopinsight/data/realopinsight.db < $PWD/sql/update_2015r1.sql"
}


upgrade_database()
{
  if [ "${INSTALLED_VERSION}" == "2014b7" ]; then
    update_db_2014b8
    update_db_2015r1
  elif [ "${INSTALLED_VERSION}" == "2014b8" ]; then
    update_db_2015r1
  else
    echo "DEBUG: No database updates"
  fi
}


make_backup()
{
  echo -n "DEBUG: Backup current installation to ${REALOPINSIGHT_BACKUP_FILE}..."
  tar --same-owner \
      --exclude ${REALOPINSIGHT_INSTALL_PREFIX}/run \
      --exclude ${REALOPINSIGHT_INSTALL_PREFIX}/www/run \
      -zcf \
      ${REALOPINSIGHT_BACKUP_FILE} \
      ${REALOPINSIGHT_INSTALL_PREFIX}

  if [ $? -eq 0 ]; then
    echo "done"
  else
    echo "failed"
    exit 1
  fi
}

make_restore()
{
  echo -n "DEBUG: Restoring system from ${REALOPINSIGHT_BACKUP_FILE} ..."
  tar --same-owner -zxf ${REALOPINSIGHT_BACKUP_FILE} -C /
  if [ $? -eq 0 ]; then
    echo "done"
  else
    echo "failed"
    exit 1
  fi
}


restore_backup_on_error()
{
  if [ -$? -eq 0 ]; then
    echo "done"
  else
    echo "failed"
    make_restore
    exit 1
  fi
}


create_destination_fs_tree()
{
  echo "DEBUG: Creating destination fs tree..."
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/lib
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/bin
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/sbin
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/etc
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/data
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/log
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/www   # directory for storing www files
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/www/run   # directory for storing thumbnails
}


copy_distribution_files()
{
  create_destination_fs_tree
  echo "DEBUG: Copying core distribution files..."
  cp -rf www/* ${REALOPINSIGHT_INSTALL_PREFIX}/www/
  install -D -m 755 lib/* ${REALOPINSIGHT_INSTALL_PREFIX}/lib/
  install -D -m 755 bin/* ${REALOPINSIGHT_INSTALL_PREFIX}/bin/
  install -D -m 755 sbin/* ${REALOPINSIGHT_INSTALL_PREFIX}/sbin/
  install -m 644 etc/wt_config.xml ${REALOPINSIGHT_INSTALL_PREFIX}/etc/
}



apply_permissions()
{
  echo "DEBUG: Applying permissions..."
  check_realopinsight_user
  chown -R $REALOPINSIGHT_USER:$REALOPINSIGHT_GROUP ${REALOPINSIGHT_INSTALL_PREFIX}
}


install_ultimate_distrib() 
{
  echo "DEBUG: Starting the installation of RealOpInsight Ultimate $REALOPINSIGHT_VERSION..."
  stop_services
  check_root_user
  check_prerequisites
  copy_distribution_files
  apply_permissions
  install_initd_scripts
  start_services
  echo -e " $GREEN_COLOR ==>Installation completed $DEFAULT_COLOR"
}

clear_deprecated_settings()
{
  if [ "${INSTALLED_VERSION}" == "2015r1_XOR_2015r2" ]; then
    # clear old Apache2/FastCGI settings
    a2disconf realopinsight-ultimate
    service apache2 restart
    rm -rf $REALOPINSIGHT_INSTALL_PREFIX/run # old fcgi session data
  fi

  # clear old thumbnails : shall exist later to version 2015r3.b1
  rm -rf ${REALOPINSIGHT_INSTALL_PREFIX}/www/run/*
}

upgrade_ultimate_distrib()
{
  echo "DEBUG: Starting upgrade to RealOpInsight  Ultimate version ${REALOPINSIGHT_VERSION}..."
  check_prerequisites
  prompt_to_get_current_db_version
  stop_services
  make_backup
  copy_distribution_files
  upgrade_database || upgrade_database
  apply_permissions
  install_initd_scripts
  clear_deprecated_settings
  start_services
  echo -e " $GREEN_COLOR DEBUG: Upgrade completed. Backup file: ${REALOPINSIGHT_BACKUP_FILE} $DEFAULT_COLOR"
}


prompt_copyright


echo
echo "What do you want to do?"
echo
echo "    n) New installation"
echo "    u) Upgrade"
echo "    q|Q) Quit"
echo

while true; do
  read -p "Type a response: " rep
  case $rep in
    n) install_ultimate_distrib
       break;;
    u) upgrade_ultimate_distrib
       break;;
    q|Q) break;;
    *) echo -n "Invalid input! "
       exit 1;;
    esac
done

echo -e " $RED_COLOR Please reboot the server before the first use: sudo reboot $DEFAULT_COLOR                         "
echo -e " $GREEN_COLOR Web Access: http://$HOSTNAME:4583/realopinsight$DEFAULT_COLOR "
echo -e "+==========================================================================+"

exit 0
