#!/bin/bash
# ------------------------------------------------------------------------ #
# File: appliance-ultimate-makepatch.sh                                    #
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

if [ $# -ne 1 ]; then
  echo "`basename $0` <version>"
  exit 1
fi
MAKE_PATCH_SCRIPT=contribs/appliance-ultimate-apply-patch.sh
VERSION_TEMPLATE=X.Y.Z
REAlOPINSIGHT_TARGET_VERSION=$1
REAlOPINSIGHT_PATCH_TARBALL=patch_${REAlOPINSIGHT_TARGET_VERSION}-x64_86.tar.gz
REALOPINSIGHT_PREFIX=/opt
REALOPINSIGHT_WWW=/var/www
REALOPINSIGHT_WWW_USER=www-data 
REALOPINSIGHT_WWW_GROUP=www-data
RELEASE_TARBALL_BASENAME=realopinsight-ultimate-patch-${REAlOPINSIGHT_TARGET_VERSION}-x64_86

mkdir ${RELEASE_TARBALL_BASENAME}
tar --same-owner \
    --exclude ${REALOPINSIGHT_WWW}/realopinsight/run \
    -zcf ${RELEASE_TARBALL_BASENAME}/${REAlOPINSIGHT_PATCH_TARBALL} ${REALOPINSIGHT_WWW}/realopinsight
	
sed "s/$VERSION_TEMPLATE/$REAlOPINSIGHT_TARGET_VERSION/g" $MAKE_PATCH_SCRIPT \
     > ${RELEASE_TARBALL_BASENAME}/`basename $MAKE_PATCH_SCRIPT`
cp contribs/README_UPDATE ${RELEASE_TARBALL_BASENAME}

chmod +x ${RELEASE_TARBALL_BASENAME}/*.sh

tar zcf ${RELEASE_TARBALL_BASENAME}.tar.gz ${RELEASE_TARBALL_BASENAME}
