#!/bin/bash
# ------------------------------------------------------------------------ #
# File: appliance-ultimate-makepatch.sh                                    #
# Copyright (c) 2014-2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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
set -e

print_usage()
{
  echo "`basename $0` <version> <install_prefix> <www_dir>"
}

get_absolute_path()
{
  path=$1
  if [ "${path:0:1}" = "/" ]; then
    echo "$path"
  else
    echo "$PWD/$path"
  fi
}

if [ $# -ne 3 ]; then
  print_usage
  exit 1
fi

MAKE_PATCH_SCRIPT=contribs/appliance-ultimate-apply-patch.sh
VERSION_TEMPLATE=X.Y.Z
REAlOPINSIGHT_TARGET_VERSION=$1
REALOPINSIGHT_HOME=$(get_absolute_path $2)
REALOPINSIGHT_WWW_HOME=$(get_absolute_path $3)

REAlOPINSIGHT_PATCH_TARBALL=patch_${REAlOPINSIGHT_TARGET_VERSION}-x64_86.tar.gz
RELEASE_TARBALL_BASENAME=realopinsight-ultimate-patch-${REAlOPINSIGHT_TARGET_VERSION}-x64_86

mkdir ${RELEASE_TARBALL_BASENAME}
tar --same-owner \
    --exclude ${REALOPINSIGHT_WWW_HOME}/realopinsight/run \
    -zcf ${RELEASE_TARBALL_BASENAME}/${REAlOPINSIGHT_PATCH_TARBALL} \
    ${REALOPINSIGHT_WWW_HOME}/ \
    ${REALOPINSIGHT_HOME}/sbin/ \
    ${REALOPINSIGHT_HOME}/etc/wt_config.xml
	
sed "s/$VERSION_TEMPLATE/$REAlOPINSIGHT_TARGET_VERSION/g" $MAKE_PATCH_SCRIPT \
     > ${RELEASE_TARBALL_BASENAME}/`basename $MAKE_PATCH_SCRIPT`


# copy the README FILE
cp contribs/README_UPDATE ${RELEASE_TARBALL_BASENAME}/

# copy database patch scripts
mkdir ${RELEASE_TARBALL_BASENAME}/sql
cp -r contribs/sql/*.sql ${RELEASE_TARBALL_BASENAME}/sql/

# copy rc scripts
mkdir ${RELEASE_TARBALL_BASENAME}/init.d
cp contribs/init.d/realopinsight-reportd.debian ${RELEASE_TARBALL_BASENAME}/init.d/

chmod +x ${RELEASE_TARBALL_BASENAME}/*.sh
tar zcf ${RELEASE_TARBALL_BASENAME}.tar.gz ${RELEASE_TARBALL_BASENAME}
