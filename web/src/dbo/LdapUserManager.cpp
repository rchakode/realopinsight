/*
# LdapUserManager.cpp
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
#include "WebAuthSettings.hpp"
#include "WebUtils.hpp"
#include "LdapUserManager.hpp"
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>

/**
 * @brief LdapUserTable::LdapUserTable
 * @param parent
 */
namespace {
  const int TABLE_COLUMN_COUNT = 5;
}
LdapUserManager::LdapUserManager(DbSession* dbSession, Wt::WContainerWidget* parent)
  : Wt::WTableView(parent),
    m_userEnableStatusChanged(this),
    m_model(new Wt::WStandardItemModel(0, TABLE_COLUMN_COUNT, this)),
    m_dbSession(dbSession)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);
  setAlternatingRowColors(true);

  setModelHeader();
  setModel(m_model);
  addEvent();
}

/**
 * @brief Add signa/slot event handling
 */
void LdapUserManager::addEvent()
{
  m_model->itemChanged().connect(this, &LdapUserManager::handleImportationAction);
}

/**
 * @brief Set the table view header
 */
void LdapUserManager::setModelHeader(void)
{
  m_model->insertColumns(0, TABLE_COLUMN_COUNT);
  m_model->setHeaderData(0, Q_TR("DN"));
  m_model->setHeaderData(1, Q_TR("Full Name"));
  m_model->setHeaderData(2, Q_TR("UID"));
  m_model->setHeaderData(3, Q_TR("Email"));
  m_model->setHeaderData(4, Q_TR("Enable Auth"));
}

/**
 * @brief LdapUserTable::updateLdapUsers
 * @return
 */
int LdapUserManager::updateUserList(void)
{
  setDisabled(true);
  WebAuthSettings preferences;
  m_ldapUidField = preferences.getLdapIdField();
  LdapHelper ldapHelper(preferences.getLdapServerUri(),
                        preferences.getLdapVersion(),
                        preferences.getLdapSslUseMyCert(),
                        preferences.getLdapSslCertFile(),
                        preferences.getLdapSslCaFile());

  std::string filter = "(objectClass=person)";
  m_users.clear();
  int count = ldapHelper.listUsers(preferences.getLdapSearchBase(),
                                   preferences.getLdapBindUserDn(),
                                   preferences.getLdapBindUserPassword(),
                                   filter,
                                   m_users);
  m_model->clear();
  setModelHeader();
  if (count <= 0) {
    m_lastError = ldapHelper.lastError();
  } else {
    for (const auto& userInfo : m_users) {
      DboUserT dbUserInfo;
      bool imported = m_dbSession->findUser(userInfo[m_ldapUidField], dbUserInfo);
      addUserRow(userInfo, imported);
    }
  }
  setDisabled(false);
  return count;
}


void LdapUserManager::addUserRow(const LdapUserAttrsT& userInfo, bool imported)
{
  int row = m_model->rowCount();
  std::string dn = userInfo["dn"];
  m_model->setItem(row, 0, ngrt4n::createStandardItem(dn, dn));
  m_model->setItem(row, 1, ngrt4n::createStandardItem(userInfo["cn"], dn));
  m_model->setItem(row, 2, ngrt4n::createStandardItem(userInfo[m_ldapUidField], dn));
  m_model->setItem(row, 3, ngrt4n::createStandardItem(userInfo["mail"], dn));
  m_model->setItem(row, 4, ngrt4n::createCheckableStandardItem(dn, imported));
}


void LdapUserManager::handleImportationAction(Wt::WStandardItem* item)
{
  if (item->isCheckable()) {
    std::string ldapDn = ngrt4n::getItemData(item);
    LdapUserMapT::ConstIterator userInfo =  m_users.find(ldapDn);
    if (userInfo != m_users.end()) {
      std::string username = (*userInfo)[m_ldapUidField];
      if (item->checkState() == Wt::Checked) { // enable LDAP authentication
        if (insertIntoDatabase(*userInfo) != 0) {
          updateUserList();
        }
      } else { // disable LDAP authentication
        if (m_dbSession->deleteUser(username) == 0) {
          m_userEnableStatusChanged.emit(DisableAuthSuccess, username);
        } else {
          m_userEnableStatusChanged.emit(GenericError, m_dbSession->lastError());
        }
      }
    } else {
      m_userEnableStatusChanged.emit(GenericError, Q_TR("User DN not found in the directory: ")+ldapDn);
    }
  }
}


int LdapUserManager::insertIntoDatabase(const LdapUserAttrsT& userInfo)
{
  int retCode = -1;
  DboUserT dbUser;
  dbUser.username = userInfo[m_ldapUidField];

  if (dbUser.username.empty()) {
    m_userEnableStatusChanged.emit(GenericError, Q_TR("The ID attribute is empty: ")+m_ldapUidField);
    return retCode;
  }

  dbUser.password = userInfo["userpassword"];
  dbUser.email = userInfo["mail"];
  dbUser.firstname = userInfo["gn"];
  dbUser.lastname = userInfo["sn"];
  dbUser.role = DboUser::OpRole;
  dbUser.authsystem = WebBaseSettings::LDAP;

  if (m_dbSession->addUser(dbUser) == 0) {
    m_userEnableStatusChanged.emit(EnableAuthSuccess, dbUser.username);
    retCode = 0;
  } else {
    m_userEnableStatusChanged.emit(GenericError, m_dbSession->lastError());
  }

  return retCode;
}
