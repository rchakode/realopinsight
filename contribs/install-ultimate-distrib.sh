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
    echo "ERROR : WWW_USER not set"
	exit 1
  fi
}

check_www_group()
{
  if [ -z "$WWW_GROUP" ]; then
    echo "ERROR : WWW_GROUP not set"
	exit 1
  fi  
}


check_root_user()
{
  if [ `id -u` -ne 0 ]; then 
    echo "ERROR : root privileges are required"
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
  service apache2 stop
}

start_services()
{
  service realopinsight-reportd start
  service apache2 start
}

check_prerequisites()
{
  echo "#TODO: check prerequisistes"
}


install_ultimate_distrib() 
{
  echo "==>Installing RealOpInsight Ultimate..."

  #check that user is root
  check_root_user
  
  # stop dependent services
  stop_services || true
  
  # check www user (apache/nginx)
  check_www_user
  
  # check www group (apache/nginx)
  check_www_group
  
  # check dependencies
  check_prerequisites
   

  echo "DEBUG : Creating destination file system..."
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/sbin
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/etc
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/data
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/log
  install -d ${REALOPINSIGHT_INSTALL_PREFIX}/run   # directory for session info and thumbnails
  install -d ${WWW_INSTALL_PREFIX}
  install -d ${WWW_INSTALL_PREFIX}/run      # directory for thumbnails

  echo "DEBUG : Setting file permissions..."
  chown -R $WWW_USER:$WWW_GROUP ${REALOPINSIGHT_INSTALL_PREFIX}/
  chown -R $WWW_USER:$WWW_GROUP ${WWW_INSTALL_PREFIX}/

  echo "DEBUG : Copying core distribution files..."
  cp -r www/* ${WWW_INSTALL_PREFIX}/
  chmod 755 ${WWW_INSTALL_PREFIX}/realopinsight.fcgi
  install -m 755 sbin/realopinsight-reportd ${REALOPINSIGHT_INSTALL_PREFIX}/sbin
  
  echo "Copying configuration files..."
  install -m 600 etc/wt_config.xml ${REALOPINSIGHT_INSTALL_PREFIX}/etc/
  install -m 600 etc/realopinsight-ultimate.conf $REALOPINSIGHT_WWW_CONFIG_PATH
  
  echo "DEBUG : Activating Apache's Specific Settings..."
  a2enmod fastcgi || true
  a2enconf realopinsight-ultimate || true 
  
  echo "Setting Up Init Scripts and Starting Services.."
  install_initd_scripts
  start_services
  echo "==>Installation completed"
}

prompt_copyright
install_ultimate_distrib

exit 0
