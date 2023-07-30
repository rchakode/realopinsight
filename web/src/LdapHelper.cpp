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

#include "dbo/src/DbObjects.hpp"
#include "LdapHelper.hpp"
#include "SettingFactory.hpp"
#include "WebUtils.hpp"
#include <ctime>

namespace {
  const int REQUEST_TIMEOUT = 60;
}


LdapHelper::LdapHelper(const std::string& serverUri,
                       int version,
                       bool sslUseMyCert,
                       const std::string& sslCertFile,
                       const std::string& sslCaFile)
  : m_handler(nullptr),
    m_serverUri(serverUri),
    m_version(version),
    m_sslUseMyCert(sslUseMyCert),
    m_sslCertFile(sslCertFile),
    m_sslCaFile(sslCaFile)
{
  setupHandler();
}

LdapHelper::~LdapHelper()
{
  cleanupHandler();
}

int LdapHelper::setupHandler(void)
{
  setSslSettings();
  if (ldap_initialize(& m_handler, m_serverUri.c_str()) != 0) {
    m_lastError = QObject::tr("Failed initializing annuary handler");
    return -1;
  }
  return 0;
}

void LdapHelper::cleanupHandler(void)
{
  if (m_handler)
    ldap_unbind_ext(m_handler, nullptr, nullptr);
}

void LdapHelper::reset(void)
{
  cleanupHandler();
  setupHandler();
}

void LdapHelper::setSslSettings(void)
{
  if ( ldap_is_ldaps_url(m_serverUri.c_str()) ) {
    int sslRequireCert = LDAP_OPT_X_TLS_DEMAND;
    ldap_set_option (nullptr, LDAP_OPT_X_TLS_REQUIRE_CERT, &sslRequireCert);

    if (m_sslUseMyCert) {
      ldap_set_option(nullptr, LDAP_OPT_X_TLS_CACERTFILE, m_sslCaFile.c_str());
      ldap_set_option(nullptr, LDAP_OPT_X_TLS_CERTFILE, m_sslCertFile.c_str());
    }
  }
}

bool LdapHelper::loginWithDistinguishName(const std::string& dn, const std::string& password)
{
  // Initialize a connection handler
  if (! ldap_is_ldap_url(m_serverUri.c_str()) ) {
    m_lastError = QObject::tr("Invalid LDAP URL: %1").arg(m_serverUri.c_str());
    return false;
  }

  if (! m_handler) {
    m_lastError = QObject::tr("Uninitialized handler");
    return false;
  }

  // Set LDAP protocol version
  ldap_set_option(m_handler, LDAP_OPT_PROTOCOL_VERSION, &m_version);

  // Prepare credentials
  struct berval cred;
  cred.bv_val = const_cast<char*>(password.c_str());
  cred.bv_len = password.length();

  // User authentication (bind)
  bool resultStatus = false;
  int rc = ldap_sasl_bind_s(m_handler, dn.c_str(), LDAP_SASL_SIMPLE, &cred, nullptr,nullptr,nullptr);
  if (rc != LDAP_SUCCESS) {
    m_lastError = QString("LDAP: %1").arg(ldap_err2string(rc));
  } else {
    resultStatus = true;
    m_lastError = QObject::tr("Authentication successful");
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
    m_lastError = QObject::tr("LDAP: Uninitialized handler");
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
                              nullptr,
                              0,
                              nullptr,
                              nullptr,
                              &timeout,
                              0,
                              &searchResult);
  if (ret != LDAP_SUCCESS) {
    m_lastError = QObject::tr("LDAP: Search failed: %1").arg(ldap_err2string(ret));
    return -1;
  }

  // parse result
  for (LDAPMessage* currentEntry = ldap_first_entry(m_handler, searchResult);
       currentEntry != nullptr;
       currentEntry = ldap_next_entry(m_handler, currentEntry)) {
    std::string dn = getObjectDistinguishName(currentEntry);
    userMap[dn].insert("dn", dn);
    parseObjectAttr(currentEntry, userMap[dn]);
  }

  if (searchResult)
    ldap_msgfree(searchResult);

  return userMap.size();
}


std::string LdapHelper::getObjectDistinguishName(LDAPMessage* objectData)
{
  char* buffer;
  std::string result = "";
  if ((buffer = ldap_get_dn(m_handler, objectData)) != nullptr) {
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
       curAttr != nullptr; curAttr = ldap_next_attribute(m_handler, objectData, ber) ) {
    if ((values = ldap_get_values_len(m_handler, objectData, curAttr)) != nullptr ) {
      for (int attrIndex = 0; values[attrIndex] != nullptr; attrIndex++ ) {
        userInfo.insert(QString(curAttr).toLower().toStdString(), values[attrIndex]->bv_val);
      }
      ldap_value_free_len(values);
    }
    ldap_memfree(curAttr);
  }
}

