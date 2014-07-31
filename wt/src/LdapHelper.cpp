/*
# LdapHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 25-07-2014                                                  #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */

#include "LdapHelper.hpp"
#include "Settings.hpp"
#include "WebUtils.hpp"
#include "DbObjects.hpp"
#include <ctime>

namespace {
  const int REQUEST_TIMEOUT = 60;
}


LdapHelper::LdapHelper(const std::string& serverUri, int version)
  : m_handler(NULL),
    m_serverUri(serverUri),
    m_version(version)
{
  setupHandler();
}

LdapHelper::~LdapHelper()
{
  cleanupHandler();
}

int LdapHelper::setupHandler(void)
{
  if (ldap_initialize(& m_handler, m_serverUri.c_str()) != 0) {
    m_lastError = QObject::tr("Failed initializing annuary handler");
    return -1;
  }
  return 0;
}

void LdapHelper::cleanupHandler(void)
{
  if (m_handler)
    ldap_unbind_ext(m_handler, NULL, NULL);
}


bool LdapHelper::loginWithDistinguishName(const std::string& dn, const std::string& password)
{
  // Intialize a connection handler
  if (! ngrt4n::isValidUri(m_serverUri.c_str(), "ldap", true)) {
    m_lastError = QObject::tr("Invalid LDAP address: %1").arg(m_serverUri.c_str());
    return false;
  }

  if (! m_handler) {
    m_lastError = QObject::tr("Unitialized handler");
    return false;
  }

  // Set protocol
  ldap_set_option(m_handler, LDAP_OPT_PROTOCOL_VERSION, &m_version);

  // Prepare credentials
  struct berval cred;
  cred.bv_val = const_cast<char*>(password.c_str());
  cred.bv_len = password.length();

  // User authentication (bind)
  bool resultStatus = false;
  int rc = ldap_sasl_bind_s(m_handler, dn.c_str(), LDAP_SASL_SIMPLE, &cred, NULL,NULL,NULL);
  if (rc == LDAP_SUCCESS) {
    resultStatus = true;
    m_lastError = QObject::tr("Authentication successful");
  } else {
    m_lastError = QObject::tr("LDAP: %1").arg(ldap_err2string(rc));
  }

  return resultStatus;
}




int LdapHelper::listUsers(const std::string& searchBase,
                          const std::string& bindUser,
                          const std::string& bindPass,
                          const std::string& filter,
                          LdapUserMapT& userMap)
{
  if (! m_handler) {
    m_lastError = QObject::tr("LDAP: Unitialized handler");
    return -1;
  }

  LDAPMessage* searchResult;
  struct timeval timeout;
  timeout.tv_sec = REQUEST_TIMEOUT;

  if (! loginWithDistinguishName(bindUser, bindPass)) {
    return -1;
  }

  // make request
  int ret = ldap_search_ext_s(m_handler,
                              searchBase.c_str(),
                              LDAP_SCOPE_SUBTREE,
                              filter.c_str(),
                              NULL,
                              0,
                              NULL,
                              NULL,
                              &timeout,
                              0,
                              &searchResult);
  if (ret != LDAP_SUCCESS) {
    m_lastError = QObject::tr("LDAP: Search failed: %1").arg(ldap_err2string(ret));
    return -1;
  }

  // parse result
  for (LDAPMessage* currentEntry = ldap_first_entry(m_handler, searchResult);
       currentEntry != NULL;
       currentEntry = ldap_next_entry(m_handler, currentEntry)) {
    std::string dn = getObjectDistingisghName(currentEntry);
    userMap[dn].insert("dn", dn);
    parseObjectAttr(currentEntry, userMap[dn]);
  }

  if (searchResult)
    ldap_msgfree(searchResult);

  return userMap.size();
}


std::string LdapHelper::getObjectDistingisghName(LDAPMessage* objectData)
{
  char* buffer;
  std::string result = "";
  if ((buffer = ldap_get_dn(m_handler, objectData)) != NULL) {
    result = std::string(buffer);
    ldap_memfree(buffer);
  }
  return result;
}

void LdapHelper::parseObjectAttr(LDAPMessage* objectData, LdapUserAttrsT& userInfo)
{
  BerElement* ber;
  struct berval** values;
  for (char* curAttr = ldap_first_attribute(m_handler, objectData, &ber);
       curAttr != NULL; curAttr = ldap_next_attribute(m_handler, objectData, ber) ) {
    if ((values = ldap_get_values_len(m_handler, objectData, curAttr)) != NULL ) {
      for (int attrIndex = 0; values[attrIndex] != NULL; attrIndex++ ) {
        userInfo.insertMulti(QString(curAttr).toLower().toStdString(), values[attrIndex]->bv_val);
      }
      ldap_value_free_len(values);
    }
    ldap_memfree(curAttr);
  }
}

