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


struct UserInfoT
{
  std::string dn;
  std::string sn;
  std::string cn;
  std::string uid;
  std::string password;
  std::string username;
  std::string email;
};

typedef QMap<std::string, UserInfoT> UserInfoListT;

class LdapHelper
{
public:
  LdapHelper(const QString& serverUriconst, const QString& baseDnSearchFormat, int version=LDAP_VERSION3);
  virtual ~LdapHelper();
  bool loginWithUsername(const std::string& username, const std::string& password);
  bool loginWithDistinguishName(const std::string& dn, const std::string& password);
  int listUsers(const std::string& baseDn,
                const std::string& bindUsername,
                const std::string& bindPassword,
                UserInfoListT& users);
  QString lastError(void) const {return m_lastError;}

private:
  LDAP* m_handler;
  QString m_lastError;
  QString m_serverUri;
  QString m_baseDnSearchFormat;
  int m_version;

  int setupHandler(void);
  void cleanupHandler(void);
  std::string getObjectDistingisghName(LDAPMessage* objectData);
  void parseObjectAttr(LDAPMessage* objectData, StringMapT& attrs);
  void fillUserInfo(const StringMapT& attrs, UserInfoT& userInfo);
};

#endif // LDAPHELPER_HPP
