/*
# LdapUserManager.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 07-12-2014                                                  #
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


#ifndef LDAPUSERMANAGER_HPP
#define LDAPUSERMANAGER_HPP


#include "LdapHelper.hpp"
#include "dbo/src/DbObjects.hpp"
#include "dbo/src/DbSession.hpp"
#include <Wt/WTableView>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>
#include <QString>

class LdapUserManager : public Wt::WTableView
{
public:
  enum EnableOperationT {
    EnableAuthSuccess,
    DisableAuthSuccess,
    GenericError
  };

  LdapUserManager(DbSession* dbSession, Wt::WContainerWidget* parent = 0);
  int updateUserList(void);
  std::string lastError(void) const {return m_lastError.toStdString();}

  Wt::Signal<int, std::string>& userEnableStatusChanged(void) {return m_userEnableStatusChanged;}

private:
  /** Signals **/
  Wt::Signal<int, std::string> m_userEnableStatusChanged;

  /** other members **/
  QString m_lastError;
  Wt::WStandardItemModel* m_model;
  DbSession* m_dbSession;
  LdapUserMapT m_users;
  std::string m_ldapUidField;

  void addEvent(void);
  void setModelHeader(void);
  void addUserRow(const LdapUserAttrsT& userInfo, bool imported);
  void handleImportationAction(Wt::WStandardItem* item);
  int insertIntoDatabase(const LdapUserAttrsT& userInfo);
};

#endif // LDAPUSERMANAGER_HPP
