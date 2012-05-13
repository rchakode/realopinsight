# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).						   #
#																		   #
# NGRT4N is free software: you can redistribute it and/or modify		   #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.									   #
#																		   #
# Foobar is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------# 

set -e

APP="ngrt4n"
APP_HOME_ENV="NGRT4N_HOME"
APP_USER="${APP}"
APP_HOME="/opt/${APP}"
APP_BIN="${APP_HOME}/bin/${APP}"
APP_CONFIG="/etc/${APP}.cfg"

# Print help
#
usage(){
  echo
  echo "Usage: ./install.sh [OPTION]"
  echo 
  echo "       -d INSTALL_PREFIX[=/opt/${APP}]"
  echo "          Sets the installation directory"
  echo "       -h"
  echo "          Shows this help"
}

while true ; do
    [ -z "$1" ] && break ;
    case "$1" in
        -h) usage; exit 0;;
        -d)
            if [ ! -z "$2" ]; then
               case $2 in
                    /*) 
                    	APP_HOME="$2" 
                    	APP_BIN="${APP_HOME}/bin/${APP}"
                    ;;
                    *) 
                    	echo "ERROR: The installation directory must be an absolute path" 
                    ;;
               esac
            else
               echo "ERROR: The option -d required an argument"
 	       usage
            fi
            shift 2 
        ;;
        *)  usage; exit 1 ;;
    esac
done

# Cheking prerequises
[ `id -u` -ne 0 ] && echo "ERROR : the installation must be carried out as root" && exit 1 

# Installation
#
echo -n "DEGUG : Checking qmake => ${QMAKE:=`which qmake`}... "
[ -z $QMAKE ] && echo "ERROR : qmake not found." && exit 1
echo "done"

echo -n "DEGUG : Checking make => ${MAKE:=`which make`}... "
[ -z $MAKE ] && echo "ERROR : make not found." && exit 1
echo "done"

echo -n "DEGUG : Checking c++ compiler => ${CXX:=`which g++`}... "
[ -z $CXX ] && echo "ERROR : g++ not found." && exit 1
echo "done"

echo -n "Compiling ${APP}... "
${QMAKE} -config release && make 1>/dev/null
echo done

echo -n "Installing ${APP}... "  
mkdir -p ${APP_HOME}/bin
cp -f --preserve=mode build/bin/${APP}.bin ${APP_HOME}/bin
cp -f --preserve=mode scripts/${APP} ${APP_HOME}/bin
cp -r examples ${APP_HOME}
echo "done"

echo "Setting ${APP}... "
if [ -z `grep "^${APP_USER}" /etc/passwd` ]; then
   useradd ${APP_USER} && echo "user ${APP_USER} created"
else 
   echo "DEBUG : the ${APP_USER} user already exists"
fi
if [ -z `grep "^${APP_USER}" /etc/group` ] ; then
   groupadd ${APP_USER}
else  
   echo "DEBUG : the ${APP_USER} group already exists"
fi
 
usermod -s /bin/false -d ${APP_HOME} ${APP_USER} 
chown -R ${APP_USER}:${APP_USER} ${APP_HOME}
chmod 2755 ${APP_BIN}.bin

touch ${APP_CONFIG} 
chown ${APP_USER}:${APP_USER} ${APP_CONFIG} 
chmod 664 ${APP_CONFIG}

echo "Installation done."

exit 0