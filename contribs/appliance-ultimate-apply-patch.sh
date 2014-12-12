#!/bin/bash
# ------------------------------------------------------------------------ #
# File: appliance-ultimate-apply-patch.sh                                  #
# Copyright (c) 2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation : 08-08-2014                                                    #
#                                                                          #
# This Software is part of RealOpInsight Ultimate.                         #
#                                                                          #
# See the Terms of Use: <http://legal.realopinsight.com>                   #
#                                                                          #
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES #
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         #
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  #
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   #
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    #
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  #
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           #
#                                                                          #
#--------------------------------------------------------------------------#

set -u

export REAlOPINSIGHT_TARGET_VERSION=X.Y.Z
export REAlOPINSIGHT_PATCH_TARBALL=patch_${REAlOPINSIGHT_TARGET_VERSION}-x64_86.tar.gz
export REALOPINSIGHT_APP_DIR=/opt/realopinsight
export REALOPINSIGHT_WWW_DIR=/var/www/realopinsight
export REALOPINSIGHT_WWW_USER=www-data 
export REALOPINSIGHT_WWW_GROUP=www-data
export REAlOPINSIGHT_BACKUP_FILE=backup_`date +%Y-%M-%d_%H-%M-%S`.tar.gz

make_backup()
{
  echo -n "DEBUG : Backup current installation to ${REAlOPINSIGHT_BACKUP_FILE}..."
  tar --same-owner \
      --exclude ${REALOPINSIGHT_APP_DIR}/run \
      -zcf ${REAlOPINSIGHT_BACKUP_FILE} ${REALOPINSIGHT_WWW_DIR} ${REALOPINSIGHT_APP_DIR}
  if [ $? -eq 0 ]; then
    echo done
  else  
    echo failed
	exit 1
  fi
}

make_restore()
{
  echo -n "DEBUG : Restoring system from ${REAlOPINSIGHT_BACKUP_FILE} ..."
  tar --same-owner -zxf ${REAlOPINSIGHT_BACKUP_FILE} -C /
  if [ $? -eq 0 ]; then
    echo done
  else  
    echo failed
	exit 1
  fi
}

prompt_copyright()
{ 
  echo
  echo "Thanks for choosing RealOpInsight Ultimate."
  echo
  echo "By installing the software you accept the Terms of Use"
  echo "which a copy is available at the following address:"
  echo "            <http://legal.realopinsight.com/>"
  echo
  echo "Type 'y' to accept this license offer."
  echo "Type 'n' to decline this license offer."
  echo "Type 'r' to view this license offer."
  echo
  while true; do
    read -p "Do you accept the terms of the License ? y/N/r " rep
    case $rep in
	  r) w3m http://legal.realopinsight.com/ ; echo ;;
	  y) break ;;
	  *) exit 0 ;; 
    esac
  done
}

check_exit_code()
{
  if [ -$? -eq 0 ]; then
    echo "done"
  else
    echo "failed"
	make_restore
    exit 1
  fi
}

update_db_2014b3()
{
  echo -n "DEBUG : Updating database to 2014b3..."
  su - ${REALOPINSIGHT_WWW_USER} -c "sqlite3 /opt/realopinsight/data/realopinsight.db < $PWD/sql/update_2014b3.sql"
  check_exit_code
  echo "database upgraded to 2014b3"
}

update_db_2014b7()
{
  echo -n "DEBUG : Updating database to 2014b7..."
  su - ${REALOPINSIGHT_WWW_USER} -c "sqlite3 /opt/realopinsight/data/realopinsight.db < $PWD/sql/update_2014b7.sql"
  check_exit_code
  su - ${REALOPINSIGHT_WWW_USER} -c 'printf "\n[%%General]\nDbState=1\n" >> /opt/realopinsight/etc/realopinsight.conf'
  echo "database upgraded to 2014b7"
}

update_db_2014b7()
{
  echo -n "DEBUG : Updating database to 2014b8..."
  su - ${REALOPINSIGHT_WWW_USER} -c "sqlite3 /opt/realopinsight/data/realopinsight.db < $PWD/sql/db_2014b8.sql"
  check_exit_code
}

get_target_version_from_user()
{
  echo
  echo "Which version of the software is current installed ?"
  echo
  echo "1) 3.0.0b1"
  echo "2) 3.0.0b2"
  echo "3) 2014b3 | 2014b4 | 2014b5 | 2014b6"
  echo "4) Other"
  echo "q) Quit"
  echo
  while true; do
    read -p "Type response " rep
    case $rep in
          1) INSTALLED_VERSION="3.0.0b1"
	      break
		  ;;
          2) INSTALLED_VERSION="3.0.0b2"
	     break
		 ;;
          3) INSTALLED_VERSION="2014b3-2014b6"
             break
                 ;;
          4) INSTALLED_VERSION="2014b7"
             break
                 ;;                 
          5) INSTALLED_VERSION="Other";
	      break
		  ;;
	  q) exit 0 
	      ;; 
	  *) echo -n "Invalid input. ";; 
    esac
  done  
}

restarting_apache()
{
  echo -n "DEBUG : Restarting Apache..."
  service apache2 restart
  check_exit_code
}


setting_up_reportd()
{
  echo "DEBUG : Setting up RealOpInsight QoS data collector"
  install -m 755 init.d/realopinsight-reportd.debian /etc/init.d/realopinsight-reportd
  update-rc.d realopinsight-reportd defaults
  service realopinsight-reportd restart
}


# start
prompt_copyright

# Get installed version
get_target_version_from_user

# Make backup
make_backup

echo "DEBUG : Upgrading RealOpInsight Ultimate to version ${REAlOPINSIGHT_TARGET_VERSION}..."

echo -n "DEBUG : Shutting down Apache..."
/etc/init.d/apache2 stop
check_exit_code

echo -n "DEBUG : Installing sqlite3 CLI..."
apt-get install -y sqlite3
check_exit_code


echo -n "DEBUG : Updating database..."
if [ "${INSTALLED_VERSION}" == "3.0.0b1" ] || [ "${INSTALLED_VERSION}" == "3.0.0b2" ]; then
  update_db_2014b3
  update_db_2014b7
  update_db_2014b8
fi

if [ "${INSTALLED_VERSION}" == "2014b3-2014b6" ]; then
  update_db_2014b7
  update_db_2014b8
fi

if [ "${INSTALLED_VERSION}" == "2014b7" ]; then
  update_db_2014b8
fi


echo -n "DEBUG : Applying update from ${REAlOPINSIGHT_PATCH_TARBALL}..."
tar --same-owner -zxf ${REAlOPINSIGHT_PATCH_TARBALL} -C /
check_exit_code


setting_up_reportd

restarting_apache


echo "DEBUG: Upgrade completed. Backup file: ${REAlOPINSIGHT_BACKUP_FILE}"


