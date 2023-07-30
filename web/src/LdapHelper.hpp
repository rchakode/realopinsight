/*
# LdapHelper.hpp
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

#ifndef LDAPHELPER_HPP
#define LDAPHELPER_HPP
#include "Base.hpp"
#include <QString>
#include <QVector>
#include <ldap.h>



typedef QMap<std::string, std::string> LdapUserAttrsT;
typedef QMap<std::string, LdapUserAttrsT> LdapUserMapT;

class LdapHelper
{
public:
  LdapHelper(const std::string& serverUri,
             int version = LDAP_VERSION3,
             bool sslUseMyCert = true,
             const std::string& sslCertFile = "",
             const std::string& sslCaFile = "");
  virtual ~LdapHelper();
  void reset(void);
  bool loginWithDistinguishName(const std::string& dn, const std::string& password);
  int listUsers(const std::string& searchBase,
                const std::string& bindUser,
                const std::string& bindPass,
                const std::string& filter,
                LdapUserMapT& users);
  QString lastError(void) const {return m_lastError;}

private:
  LDAP* m_handler;
  QString m_lastError;
  std::string m_serverUri;
  int m_version;
  bool m_sslUseMyCert;
  std::string m_sslCertFile;
  std::string m_sslCaFile;


  int setupHandler(void);
  void cleanupHandler(void);
  void setSslSettings(void);
  std::string getObjectDistinguishName(LDAPMessage* objectData);
  void parseObjectAttr(LDAPMessage* objectData, LdapUserAttrsT& userInfo);
};

#endif // LDAPHELPER_HPP
