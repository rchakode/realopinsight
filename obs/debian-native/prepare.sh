#!bin/sh
BASE_NAME=$1
SOURCE_TARBALL=${BASE_NAME}.orig.tar.gz
tar zxf ${SOURCE_TARBALL}

dh_make --email bugs@ngrt4n.com --copyright=GLPv3
#--file ${SOURCE_TARBALL}
