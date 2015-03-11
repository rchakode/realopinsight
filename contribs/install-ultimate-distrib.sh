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

if [ -e "INSTALL.MANIFEST" ]; then
  . INSTALL.MANIFEST  # source path settings. e.g. WWW_USER
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

set_realopinsight_www_config()
{
  if [ -d ${WWW_CONFIG_ROOT}/conf-available ]; then
    REALOPINSIGHT_WWW_CONFIG_FILE="${WWW_CONFIG_ROOT}/conf-available/realopinsight-ultimate.conf"
  else
    REALOPINSIGHT_WWW_CONFIG_FILE="${WWW_CONFIG_ROOT}/conf.d/realopinsight-ultimate.conf"
  fi
}

check_prerequisites()
{
  #TODO
}


install_workstation() {
  echo "==>Installing RealOpInsight Workstation..."  
  check_prerequisites
  echo "DEBUG : Installation directory => ${INSTALL_PREFIX}"
  echo -n "DEBUG : Current component => Manager..."
  install_default
  echo "done"
  
  echo -n "DEBUG : Current component => Editor..."
  install_editor
  echo "done"
  
  echo -n "DEBUG : Current component => Operator Console..."
  install_oc
  echo "done"
  echo "==>Installation completed"
}

install_ultimate_bundle() 
{
  echo "==>Installing RealOpInsight Ultimate..."

  #check that user is root
  check_root_user
  
  # check www user (apache/nginx)
  check_www_user
  
  # check www group (apache/nginx)
  check_www_group
  
  # set www configuration directory
  set_www_config_directory
 
  # check dependencies
  check_prerequisites
 
  if [ ! -z "${REALOPINSIGHT_BUILD_DEBUG}" ]; then
    echo "DEBUG : Building RealOpInsight Standalone..."
    install_webd
    install -m 755 realopinsightd  ${INSTALL_PREFIX}/
    install -m 755 contribs/realopinsightctld  ${INSTALL_PREFIX}/
  else
    echo -n "DEBUG : Building RealOpInsight for FastCGI..."
    install_fcgi
    echo done

    echo -n "DEBUG : Building QoS Data collector..."
    install_reportd
    echo done
  fi

  echo "DEBUG : Installing configuration files..."
  install -d ${INSTALL_PREFIX}
  install -d ${INSTALL_PREFIX}/sbin
  install -d ${INSTALL_PREFIX}/etc
  install -d ${INSTALL_PREFIX}/data
  install -d ${INSTALL_PREFIX}/log
  install -d ${INSTALL_PREFIX}/run   # directory for session info and thumbnails
  install -d ${WWW_INSTALL_PREFIX}
  install -d ${WWW_INSTALL_PREFIX}/run      # directory for thumbnails
  install -d ${WWW_INSTALL_PREFIX}/resources/themes/bootstrap/img  # for missing glyphicons-halflings.png
  install contribs/index.html ${WWW_INSTALL_PREFIX}/
  install -m 755 realopinsight.fcgi ${WWW_INSTALL_PREFIX}/
  install -m 644 favicon.ico ${WWW_INSTALL_PREFIX}/
  install -m 755 realopinsight-reportd ${INSTALL_PREFIX}/sbin
  install -m 755 contribs/init.d/realopinsight-reportd.debian /etc/init.d/realopinsight-reportd
  
  install -m 600 contribs/wt_config.xml ${INSTALL_PREFIX}/etc/
  install resources/themes/img/glyphicons-halflings.png ${WWW_INSTALL_PREFIX}/resources/themes/bootstrap/img
  install -m 600 contribs/apache-realopinsight-ultimate.conf ${REALOPINSIGHT_WWW_CONFIG_FILE}
  cp -r resources ${WWW_INSTALL_PREFIX}/ 
  cp -r images ${WWW_INSTALL_PREFIX}/
  cp -r i18n ${WWW_INSTALL_PREFIX}/
  
  echo "DEBUG : Settings permissions and ..."
  chown -R $WWW_USER:$WWW_GROUP ${INSTALL_PREFIX}/
  chown -R $WWW_USER:$WWW_GROUP ${WWW_INSTALL_PREFIX}/
  
  echo "DEBUG : Settings Apache"
  sed --in-place 's!WWW_INSTALL_PREFIX!'"${WWW_INSTALL_PREFIX}"'!' ${REALOPINSIGHT_WWW_CONFIG_FILE}
  a2enmod fastcgi || true
  a2enconf realopinsight-ultimate || true 
  service apache2 reload
  
  echo "DEBUG : Setting up RealOpInsight QoS data collector"
  update-rc.d realopinsight-reportd defaults
  service realopinsight-reportd restart

  echo "==>Installation completed"
}

prompt_copyright
install_ultimate_bundle

exit 0