#!/bin/bash
# ------------------------------------------------------------------------ #
# File: get_service_account_token.sh                                       #
# Copyright (c) 2020 Rodrigue Chakode and contributors                     #
# Creation: 13-09-2020                                                     #
#                                                                          #
# This Software is part of RealOpInsight (http://realopinsight.com)        #
#                                                                          #
# This is a free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty or           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with the Software.  If not, see <http://www.gnu.org/licenses/>.	   #
#--------------------------------------------------------------------------#


SERVICE_ACCOUNT_NAME=realopinsight
CONTEXT=$(kubectl config current-context)
NAMESPACE=${1:-default}

SECRET_NAME=$(kubectl get serviceaccount ${SERVICE_ACCOUNT_NAME} \
  --context ${CONTEXT} \
  --namespace ${NAMESPACE} \
  -o jsonpath='{.secrets[0].name}')
TOKEN_DATA=$(kubectl get secret ${SECRET_NAME} \
  --context ${CONTEXT} \
  --namespace ${NAMESPACE} \
  -o jsonpath='{.data.token}')

TOKEN=$(echo ${TOKEN_DATA} | base64 -d)
echo $TOKEN
