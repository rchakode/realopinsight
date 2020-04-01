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
#include "dbo/src/DbSession.hpp"
#include <QString>
#include <Wt/WTemplate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WButtonGroup.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WIntValidator.h>
#include <Wt/WApplication.h>


WebDatabaseSettings::WebDatabaseSettings(void)
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("database-settings-form.tpl"))
{
  setMargin(0, Wt::AllSides);

  auto dbType = std::make_unique<Wt::WComboBox>();
  m_dbTypeBoxRef = dbType.get();
  dbType->addItem(Q_TR("Sqlite3"));
  dbType->addItem(Q_TR("PostgreSQL"));
  dbType->changed().connect(this, &WebDatabaseSettings::updateFieldEnabledState);
  bindWidget("database-type", std::move(dbType));

  auto applyBtn = std::make_unique<Wt::WPushButton>();
  m_applyBtnRef = applyBtn.get();
  applyBtn->setText(Q_TR("Save"));
  applyBtn->setToolTip(Q_TR("Changes will be saved. Then you should disconnect manually to have the changes take effect"));
  applyBtn->setStyleClass("btn btn-info");
  applyBtn->clicked().connect(this, &WebDatabaseSettings::applySettings);
  bindWidget("database-save-settings-btn", std::move(applyBtn));

  auto dbServer = std::make_unique<Wt::WLineEdit>();
  m_dbServerFieldRef = dbServer.get();
  m_dbServerFieldRef->setPlaceholderText("hostname or IP address");
  bindWidget("database-server-addr", std::move(dbServer));

  auto dbPort = std::make_unique<Wt::WLineEdit>();
  m_dbPortFieldRef = dbPort.get();
  m_dbPortFieldRef->setPlaceholderText("5432");
  bindWidget("database-server-port", std::move(dbPort));

  auto dbName = std::make_unique<Wt::WLineEdit>();
  m_dbNameFieldRef = dbName.get();
  bindWidget("database-name", std::move(dbName));

  auto dbEmptyState = std::make_unique<Wt::WCheckBox>();
  m_dbEmptyStateRef = dbEmptyState.get();
  dbEmptyState->setChecked(true);
  bindWidget("database-empty-state", std::move(dbEmptyState));

  auto dbUser = std::make_unique<Wt::WLineEdit>();
  m_dbUserFieldRef = dbUser.get();
  bindWidget("database-user", std::move(dbUser));

  auto dbPass = std::make_unique<Wt::WLineEdit>();
  m_dbPassFieldRef = dbPass.get();
  m_dbPassFieldRef->setEchoMode(Wt::EchoMode::Password);
  bindWidget("database-password", std::move(dbPass));

  updateContents();
}


WebDatabaseSettings::~WebDatabaseSettings(void){ }

void WebDatabaseSettings::applySettings(void)
{
  SettingFactory settings;
  settings.setKeyValue(SettingFactory::DB_TYPE, QString::number(m_dbTypeBoxRef->currentIndex()));
  // PostgreSQL-specific settings
  if (m_dbTypeBoxRef->currentIndex() == PostgresqlDb) {
    auto pass =  m_dbPassFieldRef->text().toUTF8();
    auto passEncoded = SettingFactory::base64Encode(reinterpret_cast<const unsigned char *>(pass.c_str()), pass.size());
    settings.setKeyValue(SettingFactory::DB_PASSWORD, QString::fromStdString(passEncoded));
    settings.setKeyValue(SettingFactory::DB_USER, m_dbUserFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::DB_SERVER_ADDR, m_dbServerFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::DB_SERVER_PORT, m_dbPortFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::DB_NAME, m_dbNameFieldRef->text().toUTF8().c_str());
  }
  // test connection and initialize the database if needed
  if (m_dbEmptyStateRef->checkState() == Wt::CheckState::Checked) {
    DbSession dbSession;
    if (dbSession.isReady() && dbSession.initDb() == ngrt4n::RcSuccess) {
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
  m_dbEmptyStateRef->setEnabled(true);
  bool enable = m_dbTypeBoxRef->currentIndex() == PostgresqlDb;
  m_dbServerFieldRef->setEnabled(enable);
  m_dbPortFieldRef->setEnabled(enable);
  m_dbNameFieldRef->setEnabled(enable);
  m_dbUserFieldRef->setEnabled(enable);
  m_dbPassFieldRef->setEnabled(enable);
}


void WebDatabaseSettings::updateFields(void)
{
  m_dbTypeBoxRef->setCurrentIndex( getDbType() );
  m_dbServerFieldRef->setText( getDbServerAddr() );
  m_dbPortFieldRef->setText( QString::number(getDbServerPort()).toStdString().c_str() );
  m_dbUserFieldRef->setText( getDbUser() );
  m_dbPassFieldRef->setEchoMode(Wt::EchoMode::Password);
  m_dbPassFieldRef->setText( getDbPassword() );
  m_dbNameFieldRef->setText( getDbName() );
  updateFieldEnabledState();
}


void WebDatabaseSettings::updateContents() { updateFields(); }
