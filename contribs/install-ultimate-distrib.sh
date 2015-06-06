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


set -e
set -u

if [ -e ./INSTALL.MANIFEST ]; then
  . ./INSTALL.MANIFEST  # source path settings. e.g. WWW_USER
  SQLITE3="LD_LIBRARY_PATH=$REALOPINSIGHT_INSTALL_PREFIX/lib $REALOPINSIGHT_INSTALL_PREFIX/bin/sqlite3"
  REALOPINSIGHT_BACKUP_FILE=backup_`date +%Y-%M-%d_%H-%M-%S`.tar.gz
else
  echo "INSTALL.MANIFEST not found"
  exit 1
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
  echo "Thanks for choosing RealOpInsight Ultimate."
  echo
  echo "You are licensed to use this software under the terms of following licence: http://legal.realopinsight.com/"
  echo
  echo "Type 'y' to accept this license offer."
  echo "Type 'n' to decline this license offer."
  echo "Type 'r' to view this license offer."
  echo
  while true; do
    read -p "Do you accept the terms of the License ? y/N/r " rep
    case $rep in
	  r) w3m http://legal.realopinsight.com ; echo ;;
	  y) break ;;
	  *) exit 0 ;; 
    esac
  done
}


check_www_user()
{
  if [ -z "$WWW_USER" ]; then
    echo "ERROR: WWW_USER not set"
	exit 1
  fi
}

check_www_group()
{
  if [ -z "$WWW_GROUP" ]; then
    echo "ERROR: WWW_GROUP not set"
	exit 1
  fi  
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
  install -m 755 scripts/init.d/realopinsight-reportd /etc/init.d/
  update-rc.d realopinsight-reportd defaults
}

stop_services()
{
  service realopinsight-reportd stop || true
  service apache2 stop || true
}


start_services()
{
  service realopinsight-reportd restart
  service apache2 start
}


check_prerequisites()
{
  APACHECTL=$(which apachectl) || true
  if [ -z "APACHECTL" ]; then
    echo "ERROR: Apache seems to be not installed. If yes, set your PATH variable suitably"
    echo "To install it manually:"
    echo " $ sudo apt-get install apache2"
    exit 1
  fi
  
  MOD_FASTCGI=$($APACHECTL -M | grep "fastcgi\_module") || true
  if [ -z "$MOD_FASTCGI" ]; then
    echo "ERROR: Apache Module mod_fastcgi is not enabled."
    echo ""
    echo "  To install it manually:"
    echo "    $ sudo apt-get install libapache2-mod-fastcgi"
    echo "    $ a2enmod fastcgi"
    echo "  **NOTE**: On Ubuntu this package can be provided by the following repository:"
    echo "          => http://us.archive.ubuntu.com/ubuntu/"
    exit 1
  fi
}


prompt_to_get_current_db_version()
{
  echo
  echo "Which version of RealOpInsight Ultimate is currently installed ?"
  echo
  echo "    1) 2014b7"
  echo "    2) 2014b8"
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
         q|Q) exit 0
         ;;
         *) echo -n "Invalid input! ";;
    esac
  done
}


update_db_2014b8()
{
  echo -n "DEBUG : Updating database to 2014b8..."
  su - ${WWW_USER} -c "${SQLITE3} /opt/realopinsight/data/realopinsight.db < $PWD/sql/update_2014b8.sql"
  restore_backup_on_error
}



update_db_2015r1()
{
  echo -n "DEBUG : Updating database to 2015r1..."
  su - ${WWW_USER} -c "${SQLITE3} /opt/realopinsight/data/realopinsight.db < $PWD/sql/update_2015r1.sql"
  restore_backup_on_error
}



upgrade_database()
{
  if [ "${INSTALLED_VERSION}" == "2014b7" ]; then
    update_db_2014b8
    update_db_2015r1
  elif [ "${INSTALLED_VERSION}" == "2014b8" ]; then
    update_db_2015r1
  else
    echo "Unknown version ${INSTALLED_VERSION}"
    exit 1
  fi
}


make_backup()
{
  echo -n "DEBUG : Backup current installation to ${REALOPINSIGHT_BACKUP_FILE}..."
  tar --same-owner \
      --exclude ${REALOPINSIGHT_INSTALL_PREFIX}/run \
      -zcf \
      ${REALOPINSIGHT_BACKUP_FILE} \
      ${REALOPINSIGHT_WWW_HOME} \
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
  echo -n "DEBUG : Restoring system from ${REALOPINSIGHT_BACKUP_FILE} ..."
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
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/run   # directory for session info and thumbnails
  install -d ${REALOPINSIGHT_WWW_HOME}
  install -d ${REALOPINSIGHT_WWW_HOME}/run      # directory for thumbnails
}


copy_distribution_files()
{
  create_destination_fs_tree
  echo "DEBUG: Copying core distribution files..."
  cp -rf www/* ${REALOPINSIGHT_WWW_HOME}/
  install -D -m 755 lib/* ${REALOPINSIGHT_INSTALL_PREFIX}/lib
  install -D -m 755 bin/* ${REALOPINSIGHT_INSTALL_PREFIX}/bin
  install -D -m 755 sbin/* ${REALOPINSIGHT_INSTALL_PREFIX}/sbin
  chmod 755 ${REALOPINSIGHT_WWW_HOME}/realopinsight.fcgi
  echo "DEBUG: Copying configuration files..."
  install -m 644 etc/wt_config.xml ${REALOPINSIGHT_INSTALL_PREFIX}/etc/
  install -m 644 etc/realopinsight-ultimate.conf $REALOPINSIGHT_WWW_CONFIG_PATH
}


install_ultimate_distrib() 
{
  echo "DEBUG: Starting the installation of RealOpInsight Ultimate $REALOPINSIGHT_VERSION..."
  stop_services
  check_root_user
  check_www_user
  check_www_group
  check_prerequisites
  copy_distribution_files
  echo "DEBUG: Setting file permissions..."
  chown -R $WWW_USER:$WWW_GROUP ${REALOPINSIGHT_INSTALL_PREFIX}/{data,log,run} \
                                ${REALOPINSIGHT_WWW_HOME}/run
  
  echo "DEBUG: Activating Apache's Specific Settings..."
  a2enconf realopinsight-ultimate
  
  echo "DEBUG: Setting Up Init Scripts and Starting Services.."
  install_initd_scripts
  start_services
  echo "==>Installation completed"
}


upgrade_ultimate_distrib()
{
  echo "DEBUG : Starting upgrade to RealOpInsight  Ultimate version ${REALOPINSIGHT_VERSION}..."
  prompt_to_get_current_db_version
  stop_services
  make_backup
  copy_distribution_files
  upgrade_database
  install_initd_scripts
  start_services
  echo "DEBUG: Upgrade completed. Backup file: ${REALOPINSIGHT_BACKUP_FILE}"
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
    *) echo -n "Invalid input! ";;
    esac
done
exit 0
