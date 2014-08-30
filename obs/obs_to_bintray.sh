#!/bin/bash
#

#set -e 
set -x
set -u
shopt -s globstar

# You should define BINTRAY_ACCOUNT and BINTRAY_APIKEY here or from the outside
# BINTRAY_ACCOUNT is you Bintray account and BINTRAY_APIKEY, API Key generated under your Bintray profile
 
# Redefine following variables to match your own usage


VERBOSE=""
BINTRAY_ACCOUNT=realopinsight
BINTRAY_USER=realopinsight
BINTRAY_APIKEY=83e88c775a0951eb64f271a88b1ae1a06161ad88

WEBSITE_URL=http://realopinsight.com
RELEASE_NOTES_URL=http://docs.realopinsight.com/latest/00_intro/release-notes.html
ISSUE_TRACKER_URL=https://github.com/RealOpInsightLabs/realopinsight-workstation/issues
VCS_URL=https://github.com/RealOpInsightLabs/realopinsight-workstation
BASE_DESC=https://raw.githubusercontent.com/RealOpInsightLabs/realopinsight-workstation/master/README.md

OBS_DOWNLOAD_DIR=$PWD'/download.opensuse.org/repositories/home\:/ngrt4n'
OBS_DOWNLOAD_URL="http://download.opensuse.org/repositories/home:/"

PACKAGES="realopinsight-workstation ngrt4n-d4n"
PACKAGE_TYPES="deb rpm"

DISTRIBS="Debian_7.0 Fedora_17 Fedora_18 Fedora_19  Fedora_20 openSUSE_12.1 openSUSE_12.2 openSUSE_12.3 openSUSE_Factory"

DESCRIPTION="RealOpInsight is an open source business service monitoring
dashboard toolkit that enables IT operations staff to deal with monitoring
with focus on business. It supports Nagios, Zenoss, Zabbix, Shinken, Centreon, 
Naemon, Icinga, op5 and more."


usage()
{
  echo "usage: `basename $0` [options]"
  echo ""
  echo "OPTIONS"
  echo " -d <version>"
  echo "    Download files of the given version from OBS"
  echo " -u <version>"
  echo "    Upload files of the given version to Bintray"
  echo " -v verbose"
  echo "    Set verbose options"
  echo " -s"
  echo "   Setup packages and versions on Bintray repository"
  echo " -p"
  echo "   Publish all Bintray uploaded files"
  echo " -h"
  echo "   Print this help"
}


get_file_extention()
{
  echo "$1" | awk -F . '{print $NF}'
}

get_package_name()
{
  FILE=$1
  PKG_TYPE=`get_file_extention $FILE`
  
  result=""
  case $PKG_TYPE in
    rpm) 
      result=`rpm --queryformat "%{NAME}" -qp $FILE`
    ;;
    deb) 
      result=`dpkg-deb --show  --showformat='${Package}' $FILE`
    ;;  
    ?) echo "unknown package type"; 
    ;;    
  esac
  
  echo "$result"
}

get_package_version()
{
  FILE=$1
  PKG_TYPE=`echo "$FILE" | awk -F . '{print $NF}'`
  
  result=""
  case $PKG_TYPE in
    rpm) 
      result=`rpm --queryformat "%{VERSION}" -qp $FILE`
    ;;
    deb) 
      result=`dpkg-deb --show  --showformat='${Version}' $FILE`
    ;;  
    ?) echo "unknown package type"; 
    ;;    
  esac
  
  echo "$result"
}


get_package_release()
{
  FILE=$1
  PKG_TYPE=`echo "$FILE" | awk -F . '{print $NF}'`
  
  result=""
  case $PKG_TYPE in
    rpm) 
      result=`rpm --queryformat "%{RELEASE}" -qp $FILE`
    ;;
    deb) 
      result=""
    ;;  
    ?) echo "unknown package type"; 
    ;;    
  esac
  
  echo "$result"
}


get_package_arch()
{
  FILE=$1
  PKG_TYPE=`echo "$FILE" | awk -F . '{print $NF}'`
  
  result=""
  case $PKG_TYPE in
    rpm) 
      result=`rpm --queryformat "%{ARCH}" -qp $FILE`
    ;;
    deb) 
      result=`dpkg-deb --show  --showformat='${Architecture}' $FILE`
    ;;  
    ?) echo "unknown package type"; 
    ;;    
  esac
  
  echo "$result"
}

download_obs_packages()
{
  OBS_PACKAGE_VERSION="$1"
  OBS_PACKAGE_NAME="*${OBS_PACKAGE_VERSION}*"
  wget -I /repositories/home:/ngrt4n \
     -A "$OBS_PACKAGE_NAME.rpm" -A "$OBS_PACKAGE_NAME.deb" \
     -R "*-debuginfo*" -R "ngrt4n-d4n*" \
     -c -r $OBS_DOWNLOAD_URL
}

create_packages()
{
  for PKG_TYPE in $PACKAGE_TYPES; do
    for PKG in $PACKAGES; do 
      echo "@@@@@@@@@@@@@@@@@@@@@@@@@@"
      echo "@@@ create package $PKG @@"
      echo "@@@@@@@@@@@@@@@@@@@@@@@@@@"
      curl $VERBOSE -u$BINTRAY_USER:$BINTRAY_APIKEY \
               -H "Content-Type: application/json" \
               -X POST https://api.bintray.com/packages/$BINTRAY_ACCOUNT/$PKG_TYPE/ \
               --data "{ \"name\": \"$PKG\", 
                         \"desc\": \"$DESCRIPTION\", 
                         \"desc_url\": \"$BASE_DESC\",
                         \"vcs_url\":\"$VCS_URL\",
                         \"issue_tracker_url\":\"$ISSUE_TRACKER_URL\",
                         \"website_url\":\"$WEBSITE_URL\",
                         \"labels\": [\"Nagios\",\"Zabbix\",\"Zenoss\",\"Centreon\",\"op5\",\"Icinga\"], 
                         \"licenses\": [\"GPL-3.0\"] }"  
      echo ""
    done
  done
}

create_package_versions()
{
  for DIST in $DISTRIBS; do 
  
    PKG_DIR="$OBS_DOWNLOAD_DIR/$DIST/*"
    FILES=$(ls $PKG_DIR/*.{rpm,deb} 2>/dev/null)
    
    for FILE in $FILES; do 

      PKG_TYPE=`get_file_extention $FILE`
      PKG_NAME=`get_package_name $FILE`             #`rpm --queryformat "%{NAME}" -qp $FILE`
      PKG_VERSION=`get_package_version $FILE`          # `rpm --queryformat "%{VERSION}" -qp $FILE`
      PKG_RELEASE=`get_package_release $FILE`          # `rpm --queryformat "%{RELEASE}" -qp $FILE`
      PKG_ARCH=`get_package_arch $FILE`             #`rpm --queryformat "%{ARCH}" -qp $FILE`
 
      echo "PKG_NAME=$PKG_NAME, PKG_VERSION=$PKG_VERSION, PKG_RELEASE=$PKG_RELEASE, PKG_ARCH=$PKG_ARCH"
      sleep 2

      echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
      echo "@@@ create version $PKG_VERSION   "
      echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
      curl $VERBOSE -u$BINTRAY_USER:$BINTRAY_APIKEY \
               -H "Content-Type: application/json" \
               -X POST https://api.bintray.com/packages/$BINTRAY_ACCOUNT/$PKG_TYPE/$PKG_NAME/versions/ \
               --data "{ \"name\": \"$DIST\", 
                      \"desc\": \"$RELEASE_NOTES_URL\", 
                      \"release_notes\": \"$RELEASE_NOTES_URL\",
                      \"vcs_tag\": \"$PKG_VERSION\"}"
      echo ""
    done
  done
}


upload_packages()
{
  for DIST in $DISTRIBS; do   
    
    PKG_DIR="$OBS_DOWNLOAD_DIR/$DIST/*"
    FILES=$(ls $PKG_DIR/*.{rpm,deb} 2>/dev/null)
    
    for FILE in $FILES; do 
      PKG_TYPE=`get_file_extention $FILE`
      PKG_NAME=`get_package_name $FILE`             #`rpm --queryformat "%{NAME}" -qp $FILE`
      PKG_VERSION=`get_package_version $FILE`          # `rpm --queryformat "%{VERSION}" -qp $FILE`
      PKG_RELEASE=`get_package_release $FILE`          # `rpm --queryformat "%{RELEASE}" -qp $FILE`
      PKG_ARCH=`get_package_arch $FILE`             #`rpm --queryformat "%{ARCH}" -qp $FILE`

      echo "PKG_NAME=$PKG_NAME, PKG_VERSION=$PKG_VERSION, PKG_RELEASE=$PKG_RELEASE, PKG_ARCH=$PKG_ARCH"
      
      echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@"
      echo "@@@ uploading $FILE...   "
      echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@"
      #  curl $VERBOSEf -T $FILE -u$BINTRAY_USER:$BINTRAY_APIKEY \
      #       -H "X-Bintray-Package:$PKG_NAME" \
      #       -H "X-Bintray-Version:$DIST" \
      #        https://api.bintray.com/content/$BINTRAY_ACCOUNT/$PKG_TYPE/$PKG_NAME-$PKG_VERSION-$PKG_RELEASE.$PKG_ARCH.rpm
      curl $VERBOSE -T $FILE -u$BINTRAY_USER:$BINTRAY_APIKEY \
           -H "X-Bintray-Package:$PKG_NAME" \
           -H "X-Bintray-Version:$DIST" \
           -X PUT https://api.bintray.com/content/$BINTRAY_ACCOUNT/$PKG_TYPE/
      echo ""
    done
  done
}


publish_packages()
{
  for PKG_TYPE in $PACKAGE_TYPES; do
    for PKG in $PACKAGES; do 
      for DIST in PKG_DISTS; do 
      
        echo "@@@@@@@@@@@@@@@@@@@@@@@"
        echo "@@@ publish content @@@"
        echo "@@@@@@@@@@@@@@@@@@@@@@@"
        curl $VERBOSE -u$BINTRAY_USER:$BINTRAY_APIKEY \
             -H "Content-Type: application/json" \
             -X POST https://api.bintray.com/content/$BINTRAY_ACCOUNT/$PKG_TYPE/$PKG/$DIST/publish \
             --data '{ "discard": "false" }'
        echo ""
      done
    done
  done
}

while getopts d:u:v:sph name ; do
  case $name in
    d) download_obs_packages $OPTARG
    ;;
    s) create_packages; create_package_versions
    ;;    
    u) upload_packages $OPTARG
    ;;
    p) publish_packages
    ;;
    v) VERBOSE="-v$OPTARG"
    ;;
    h) usage;
    ;;
    ?) usage; exit 1
    ;;
  esac
done

