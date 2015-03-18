# ------------------------------------------------------------------------ #
# File: install-ultimate-distrib.sh                                        #
# Copyright (c) 2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation : 10-03-2015                                                    #
#                                                                          #
# This Software is part of RealOpInsight Ultimate                          #
#                                                                          #
# Read legal notice & use terms: http://legal.realopinsight.com            #
#--------------------------------------------------------------------------#

#!/bin/bash


set -e
set -u

if [ -e ./INSTALL.MANIFEST ]; then
  . ./INSTALL.MANIFEST  # source path settings. e.g. WWW_USER
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


start_services()
{
  service realopinsight-reportd start
  service apache2 reload
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
    echo "To install it manually:"
    echo " $ sudo apt-get install libapache2-mod-fastcgi"
    echo " $ a2enmod fastcgi"
    echo "**NOTE**: On Ubuntu this package can be provided by the following repository:"
    echo "          => http://us.archive.ubuntu.com/ubuntu/"
    exit 1
  fi
}


install_ultimate_distrib() 
{
  echo "==>Installing RealOpInsight Ultimate..."

  #check that user is root
  check_root_user
  
  # check www user (apache/nginx)
  check_www_user
  
  # check www group (apache/nginx)
  check_www_group
  
  # check dependencies
  check_prerequisites
   
  echo "DEBUG: Creating destination file system..."
  install -d ${REALOPINSIGHT_HOME}
  install -d ${REALOPINSIGHT_HOME}/lib
  install -d ${REALOPINSIGHT_HOME}/sbin
  install -d ${REALOPINSIGHT_HOME}/etc
  install -d ${REALOPINSIGHT_HOME}/data
  install -d ${REALOPINSIGHT_HOME}/log
  install -d ${REALOPINSIGHT_HOME}/run   # directory for session info and thumbnails
  install -d ${REALOPINSIGHT_WWW_HOME}
  install -d ${REALOPINSIGHT_WWW_HOME}/run      # directory for thumbnails


  echo "DEBUG: Copying core distribution files..."
  cp -r www/* ${REALOPINSIGHT_WWW_HOME}/
  install -D -m 755 lib/* ${REALOPINSIGHT_HOME}/lib
  install -m 755 sbin/realopinsight-reportd ${REALOPINSIGHT_HOME}/sbin
  chmod 755 ${REALOPINSIGHT_WWW_HOME}/realopinsight.fcgi
  
  echo "DEBUG: Copying configuration files..."
  install -m 644 etc/wt_config.xml ${REALOPINSIGHT_HOME}/etc/
  install -m 644 etc/realopinsight-ultimate.conf $REALOPINSIGHT_WWW_CONFIG_PATH
  
  
  echo "DEBUG: Setting file permissions..."
  chown -R $WWW_USER:$WWW_GROUP ${REALOPINSIGHT_HOME}/{data,log,run}
  chown -R $WWW_USER:$WWW_GROUP ${REALOPINSIGHT_WWW_HOME}/run
  
  echo "DEBUG: Activating Apache's Specific Settings..."
  a2enconf realopinsight-ultimate
  
  echo "DEBUG: Setting Up Init Scripts and Starting Services.."
  install_initd_scripts
  start_services
  echo "==>Installation completed"
}

prompt_copyright
install_ultimate_distrib

exit 0