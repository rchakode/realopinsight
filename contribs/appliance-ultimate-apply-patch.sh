#!/bin/bash
# ------------------------------------------------------------------------ #
# File: install-sh                                                         #
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


export REALOPINSIGHT_WWW_USER=www-data 
export REALOPINSIGHT_WWW_GROUP=www-data

export TARGET_VERSION=2014b3
export PATCH_TARBALL=patch_${TARGET_VERSION}.tar.gz


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
    exit 1
  fi
}


prompt_copyright

echo "DEBUG : Upgrading RealOpInsight Ultimate to version ${TARGET_VERSION}..."

echo -n "DEBUG : Shutting down Apache..."
/etc/init.d/apache2 stop
check_exit_code

echo -n "DEBUG : Installing sqlite3 CLI..."
apt-get install -y sqlite3
check_exit_code

echo -n "DEBUG : Updating database..."
su - ${REALOPINSIGHT_WWW_USER} -c'echo "ALTER TABLE user ADD COLUMN authsystem int not null default 0;" \
                | sqlite3 /opt/realopinsight/data/realopinsight.db'

check_exit_code

echo -n "DEBUG : Upplying update from ${PATCH_TARBALL}..."
tar --same-owner zxf ${PATCH_TARBALL}

echo -n "DEBUG : Restarting Apache..."
/etc/init.d/apache2 start
check_exit_code

echo "DEBUG: Upgrade completed."


