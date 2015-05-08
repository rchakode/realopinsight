#!/bin/bash
# ------------------------------------------------------------------------ #
# File: realopinsight-backup.sh                                            #
# Copyright (c) 2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation : 08-05-2015                                                    #
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
  echo "`basename $0`"
}

if [ $# -ne 0 ]; then
  print_usage
  exit 1
fi

REALOPINSIGHT_DB="/opt/realopinsight/data/realopinsight.db"
REALOPINSIGHT_CONFIG_DIR="/opt/realopinsight/etc"
REALOPINSIGHT_EXPORT_TARBALL=realopinsight-ultimate-export-$(date +%F).tar.gz

tar --same-owner -zcf ${REALOPINSIGHT_EXPORT_TARBALL} \
    $REALOPINSIGHT_DB \
    ${REALOPINSIGHT_CONFIG_DIR}
    
echo "Backup File: ${REALOPINSIGHT_EXPORT_TARBALL}"
