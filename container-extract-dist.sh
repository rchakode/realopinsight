#!/bin/bash
# -------------------------------------------------------------------------------------- #
# File: container-extract-distrib.sh                                                     #
# Copyright (c) 2020 Rodrigue Chakode (rodrigue.chakode@gmail.com)                       #
# Creation : 13-04-2020                                                                  #
#                                                                                        #
# This Software is part of RealOpInsight                                                 #
#                                                                                        #
# RealOpInsight is licensed under the Apache License 2.0 (the "License");                #
# you may not use this file except in compliance with the License. You may obtain        #
# a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0                   #
#                                                                                        #
# Unless required by applicable law or agreed to in writing, software distributed        #
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR            #
# CONDITIONS OF ANY KIND, either express or implied. See the License for the             #
# specific language governing permissions and limitations under the License.             #
#--------------------------------------------------------------------------#


set -u
set -e

GREEN_COLOR="\\033[1;32m"
DEFAULT_COLOR="\\033[0;39m"
RED_COLOR="\\033[1;31m"

get_absolute_path()
{
  path=$1
  case $path in
    /*)
      ;;
    *) path=$PWD/$path
      ;;
  esac
  echo "$path"
}


check_file()
{
  path=$1
  if [ ! -e $path ]; then
    echo "File not found: $path"
    exit 1
  fi
}


extract_binary_deplibs()
{
  binary=$1
  dest_folder=$2

  index=$(echo "0" | bc)
  declare -a dependencies
  for l in  $(ldd $binary | awk '{print $1}'); do
    dependencies[$index]="$l"
    index=$(echo "$index + 1" | bc)
  done

  # now copy lib
  index=$(echo "0" | bc)
  for f in $(ldd $binary | awk '{print $3}'); do
    if [ -e "$f" ]; then
      install -m 755 "$f" ${dest_folder}/
    else
      echo "====================>>>>>>>>>>>>>>>> [WARNING] Library not found => ${dependencies[$index]}"
    fi
    index=$(echo "$index + 1" | bc)
  done
}

extract_binary_file()
{
  path=$1
  dest_folder=$2

  check_file $path

  extract_binary_deplibs $path $dest_folder/
  install -m 755 $path $dest_folder/
}

if [ -d ./dist ]; then
    rm -rf ./dist/*
else
    mkdir -p ./dist
fi

mkdir ./dist/etc
mkdir -p ./dist/www/resources/themes/bootstrap/img # required path for missing glyphicons

extract_binary_file "realopinsight-server" ./dist/
extract_binary_file "realopinsight-reportd" ./dist/
extract_binary_file "realopinsight-setupdb" ./dist/

cp ./Dockerfile ./dist/
cp ./LICENSE ./dist/
cp -r resources i18n images ./dist/www
cp ./resources/themes/img/glyphicons-halflings.png ./dist/www/resources/themes/bootstrap/img

install -m 644 ./favicon.ico ./dist/www
install -m 644 ./contribs/etc/wt_config.xml ./dist/etc/
install -m 755 ./container-entrypoint.sh ./dist/
