#!/bin/bash
# This script allows to add packages to Debian repositories defines in the current directory
# Author : Rodrigue Chakode <rodrigue.chakode@realopinsight.com>
# Creation : 10/01/2015

# REQUIREMENTS
#
# ** Your Debian packages MUST be available a the given directory on your local machine (deb, dsc, src, diff)
# ** The directory must contains a sub directory for any considered distribution. Currently: wheezy, squeeze
# ** You MUST install reprepro tool
# ** You MUST run the script from the current directory
# ** The script assume that 

set -e 

if [ $# -ne 1 ] ; then
  echo "`basename $0` <version>"
  exit 1
fi

VERSION=$1
DESC_FILE_TPL=$PWD/realopinsight-workstation.dsc.tpl

if [ $# -ne 1 ]; then
  echo "basename $0 <package_bin_dir>"
  exit 1
fi

mkdir -p obs-packages
cd obs-packages

# Download packages for debian wheezy
dist="wheezy"
archs="i386 amd64"

mkdir -p $dist
cd $dist

wget -N http://download.opensuse.org/repositories/home:/ngrt4n/Debian_7.0/realopinsight-workstation-${VERSION}.tar.gz_3.2.1.diff.gz
wget -N http://download.opensuse.org/repositories/home:/ngrt4n/Debian_7.0/realopinsight-workstation-${VERSION}.tar.gz_${VERSION}.dsc 
wget -N http://download.opensuse.org/repositories/home:/ngrt4n/Debian_7.0/realopinsight-workstation-${VERSION}.tar.gz_${VERSION}.orig.tar.gz

for arch in $archs; do 
  wget -N http://download.opensuse.org/repositories/home:/ngrt4n/Debian_7.0/${arch}/realopinsight-workstation_${VERSION}_${arch}.deb 
done 

TGZ=realopinsight-workstation-${VERSION}.tar.gz_${VERSION}.orig.tar.gz
DIFF_TGZ=realopinsight-workstation-${VERSION}.tar.gz_3.2.1.diff.gz
DSC_FILE=realopinsight-workstation-${VERSION}.tar.gz_${VERSION}.dsc
DEFAULT_VERSION=X.Y.Z

cp ${DESC_FILE_TPL} ${DSC_FILE}

# alter version settings
sed --in-place "s/$DEFAULT_VERSION/$VERSION/g" ${DSC_FILE}

# alter orig tgz file settings
tgz_md5sum=`md5sum ${TGZ} | awk '{print $1}'`
tgz_size=`stat --printf "%s" ${TGZ}`
sed --in-place "s/<TGZ_MD5SUM>/$tgz_md5sum/g" ${DSC_FILE}
sed --in-place "s/<TGZ_SIZE>/$tgz_size/g" ${DSC_FILE}

# alter diff tgz file settings
diff_tgz_md5sum=`md5sum ${DIFF_TGZ} | awk '{print $1}'`
diff_tgz_size=`stat --printf "%s" ${DIFF_TGZ}`
sed --in-place "s/<DIFF_TGZ_MD5SUM>/$diff_tgz_md5sum/g" ${DSC_FILE}
sed --in-place "s/<DIFF_TGZ_SIZE>/$diff_tgz_size/g" ${DSC_FILE}

# Make repository

cd ../..

echo $PWD
reprepro -Vb . includedeb $dist $PWD/obs-packages/$dist/*.deb
reprepro -Vb . includedsc $dist $PWD/obs-packages/$dist/*.dsc

# Copy to OVH
#scp -r conf/ dists/ db/ pool/ root@91.121.160.100:/var/lib/lxc/sysfera/rootfs/var/www/web/download/realopinsight/repos/debs