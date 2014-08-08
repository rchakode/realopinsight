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

./install-sh -d /opt/ -m ultimate
