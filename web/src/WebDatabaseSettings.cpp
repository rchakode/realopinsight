/*
 * WebPreferencesBase.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 21-06-2015                                                     #
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

#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include "WebDatabaseSettings.hpp"
#include "Validators.hpp"
#include "dbo/DbSession.hpp"
#include <QString>
#include <Wt/WTemplate>
#include <Wt/WContainerWidget>
#include <Wt/WButtonGroup>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WIntValidator>
#include <Wt/WApplication>


WebDatabaseSettings::WebDatabaseSettings(void)
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("database-settings-form.tpl"))
{
  setMargin(0, Wt::All);
  createFormWidgets();
  bindFormWidgets();
  addEvent();
  updateContents();
}


WebDatabaseSettings::~WebDatabaseSettings(void)
{
  unbindFormWidgets();
}


void WebDatabaseSettings::addEvent(void)
{
  m_saveSettingsBtn.clicked().connect(this, &WebDatabaseSettings::applySettings);
  m_dbTypeBox.changed().connect(this, &WebDatabaseSettings::updateFieldEnabledState);
}



void WebDatabaseSettings::bindFormWidgets(void)
{
  bindWidget("database-type", &m_dbTypeBox);
  bindWidget("database-save-settings-btn", &m_saveSettingsBtn);
  bindWidget("database-server-addr", &m_dbServerAddrField);
  bindWidget("database-server-port", &m_dbServerPortField);
  bindWidget("database-name", &m_dbNameField);
  bindWidget("database-empty-state", &m_dbEmptyState);
  bindWidget("database-user", &m_dbUserField);
  bindWidget("database-password", &m_dbPasswordField);

  // Initialize fields when applicable
  m_dbEmptyState.setChecked(true);
}


void WebDatabaseSettings::unbindFormWidgets(void)
{
  takeWidget("database-type");
  takeWidget("database-save-settings-btn");
  takeWidget("database-server-addr");
  takeWidget("database-server-port");
  takeWidget("database-name");
  takeWidget("database-empty-state");
  takeWidget("database-user");
  takeWidget("database-password");
}


void WebDatabaseSettings::createFormWidgets(void)
{
  m_dbTypeBox.addItem(Q_TR("Sqlite3"));
  m_dbTypeBox.addItem(Q_TR("PostgreSQL"));
  m_dbPasswordField.setEchoMode(Wt::WLineEdit::Password);
  m_saveSettingsBtn.setText(Q_TR("Save"));
  m_saveSettingsBtn.setToolTip(Q_TR("Changes will be saved. Then you should disconnect manually to have the changes take effect"));
  m_saveSettingsBtn.setStyleClass("btn btn-info");
}


void WebDatabaseSettings::applySettings(void)
{
  m_settingFactory->setEntry(SettingFactory::DB_TYPE, QString::number(m_dbTypeBox.currentIndex()));

  // save PostgreSQL settings if applicable
  if (m_dbTypeBox.currentIndex() == PostgresqlDb) {
    m_settingFactory->setEntry(SettingFactory::DB_SERVER_ADDR, m_dbServerAddrField.text().toUTF8().c_str());
    m_settingFactory->setEntry(SettingFactory::DB_SERVER_PORT, m_dbServerPortField.text().toUTF8().c_str());
    m_settingFactory->setEntry(SettingFactory::DB_USER, m_dbUserField.text().toUTF8().c_str());
    m_settingFactory->setEntry(SettingFactory::DB_PASSWORD, m_dbPasswordField.text().toUTF8().c_str());
    m_settingFactory->setEntry(SettingFactory::DB_NAME, m_dbNameField.text().toUTF8().c_str());
  }

  // test connection and initialize the database if needed
  if (m_dbEmptyState.checkState() == Wt::Checked) {
    DbSession dbSession(getDbType(), getDbConnectionString());
    if (dbSession.isConnected() && dbSession.initDb() == 0) {
      m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Changes applied: Please *disconnect* and *reconnect* to have the changes take effect"));
    } else {
      m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Failed to initialize the database. Check logs for more details"));
    }
  } else {
    m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Database settings saved"));
  }

  CORE_LOG("info", Q_TR("Database settings updated"));
}


void WebDatabaseSettings::updateFieldEnabledState(void)
{
  bool enable = m_dbTypeBox.currentIndex() == PostgresqlDb;
  m_dbServerAddrField.setEnabled(enable);
  m_dbServerPortField.setEnabled(enable);
  m_dbNameField.setEnabled(enable);
  m_dbUserField.setEnabled(enable);
  m_dbPasswordField.setEnabled(enable);
  m_dbEmptyState.setEnabled(enable);
}


void WebDatabaseSettings::updateFields(void)
{
  m_dbTypeBox.setCurrentIndex( getDbType() );
  m_dbServerAddrField.setText( getDbServerAddr() );
  m_dbServerPortField.setText( QString::number(getDbServerPort()).toStdString().c_str() );
  m_dbUserField.setText( getDbUser() );
  m_dbPasswordField.setEchoMode(Wt::WLineEdit::Password);
  m_dbPasswordField.setText( getDbPassword() );
  m_dbNameField.setText( getDbName() );
  updateFieldEnabledState();
}


void WebDatabaseSettings::updateContents() { updateFields(); }
