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
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------# 

set -e

APP="ngrt4n"
APP_UID="${APP}"
APP_GID="${APP}"
APP_PREFIX="/usr/local"
APP_BIN="${APP_PREFIX}/bin/${APP}"
APP_HOME="/var/lib/${APP}"
APP_DB="db"

mkdir -p ${APP_PREFIX}/bin
rm -rf ${APP_HOME} && mkdir -p ${APP_HOME}

# Print help
#
usage(){
  echo
  echo "Usage: ./install.sh [OPTION]"
  echo 
  echo "       -d <install_dir>
  echo "          Sets the installation directory. Default is /usr/local
  echo "       -h"
  echo "          Shows this help"
}

# First read input 
while true ; do
    [ -z "$1" ] && break ;
    case "$1" in
        -h) usage; exit 0;;
        -d)
            if [ ! -z "$2" ]; then
               case $2 in
                    /*) 
                    	APP_PREFIX="$2" 
                    	APP_BIN="${APP_PREFIX}/bin/${APP}"
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
        *)  echo "ERROR : wrong argument $1" ; usage; exit 1 ;;
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

echo -n "DEGUG : Checking graphviz's dot utility => ${DOT:=`which dot`}... "
[ -z $DOT ] && echo "ERROR : dot not found." && exit 1
echo "done"

echo -n "Compiling ${APP}... "
${QMAKE} -config release && make 1>/dev/null
echo done

echo -n "Installing ${APP}... "  
cp -f --preserve=mode build/usr/bin/${APP} ${APP_BIN}
echo "done"

echo "Setting ${APP}... "
if [ -z `grep "^${APP_GID}" /etc/group` ] ; then
   groupadd ${APP_GID} && echo "DEBUG : groud created => ${APP_GID}"
else  
   echo "DEBUG : the ${APP_GID} group already exists"
fi
if [ -z `grep "^${APP_UID}" /etc/passwd` ]; then
   useradd -g ${APP_GID} ${APP_UID} && echo "DEBUG : user created => ${APP_UID}"
else 
   echo "DEBUG : the ${APP_UID} user already exists"
fi

chown ${APP_UID}:${APP_GID} ${APP_BIN}
chmod 2755 ${APP_BIN}

touch ${APP_HOME}/${APP_DB}
chmod 664 ${APP_HOME}/${APP_DB}
  
usermod -s /bin/false -d ${APP_HOME} ${APP_UID} 
chown -R ${APP_UID}:${APP_GID} ${APP_HOME}



echo "Installation done."

exit 0