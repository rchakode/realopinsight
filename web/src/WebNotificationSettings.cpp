/*
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Change: 17-12-2017                                                  #
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
#include "WebNotificationSettings.hpp"
#include "Validators.hpp"
#include <QString>
#include <Wt/WTemplate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WButtonGroup.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WIntValidator.h>
#include <Wt/WApplication.h>


WebNotificationSettings::WebNotificationSettings(void)
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("notification-settings-form.tpl"))
{
  setMargin(0, Wt::AllSides);

  auto notifType = std::make_unique<Wt::WComboBox>();
  m_notificationTypeFieldRef = notifType.get();
  notifType->addItem(Q_TR("No notification"));
  notifType->addItem(Q_TR("Email"));
  notifType->changed().connect(this, &WebNotificationSettings::updateEmailFieldsEnabledState);
  bindWidget("notification-type", std::move(notifType));

  auto applyBtn = std::make_unique<Wt::WPushButton>(Q_TR("Save"));
  m_applyBtnRef = applyBtn.get();
  applyBtn->setStyleClass("btn btn-info");
  applyBtn->clicked().connect(this, &WebNotificationSettings::saveChanges);
  bindWidget("notification-settings-save-button", std::move(applyBtn));

  auto smtpServer = std::make_unique<Wt::WLineEdit>();
  m_smtpServerFieldRef = smtpServer.get();
  smtpServer->setPlaceholderText(Q_TR("smtp.example.com"));
  bindWidget("notification-mail-smtp-server", std::move(smtpServer));

  auto smtpPort = std::make_unique<Wt::WLineEdit>();
  m_smtpPortFieldRef = smtpPort.get();
  smtpPort->setPlaceholderText(Q_TR("25"));
  bindWidget("notification-mail-smtp-port", std::move(smtpPort));

  auto smtpUseSsl = std::make_unique<Wt::WCheckBox>();
  m_smtpUseSslFieldRef = smtpUseSsl.get();
  bindWidget("notification-mail-smtp-use-ssl", std::move(smtpUseSsl));

  auto smtpUser = std::make_unique<Wt::WLineEdit>();
  m_smtpUserFieldRef = smtpUser.get();
  m_smtpUserFieldRef->setPlaceholderText(Q_TR("opuser"));
  bindWidget("notification-mail-smtp-username", std::move(smtpUser));

  auto smtpPass = std::make_unique<Wt::WLineEdit>();
  m_smtpPassFieldRef = smtpPass.get();
  m_smtpPassFieldRef->setEchoMode(Wt::EchoMode::Password);
  m_smtpPassFieldRef->setPlaceholderText(Q_TR("*******"));
  bindWidget("notification-mail-smtp-password", std::move(smtpPass));

  updateContents();
}

WebNotificationSettings::~WebNotificationSettings(void) {}

void WebNotificationSettings::saveChanges(void)
{
  SettingFactory settings;
  settings.setKeyValue(SettingFactory::NOTIF_TYPE, QString::number(m_notificationTypeFieldRef->currentIndex()));
  if (m_notificationTypeFieldRef->currentIndex() == EmailNotification) {
    auto pass = m_smtpPassFieldRef->text().toUTF8();
    auto passEncoded = SettingFactory::base64Encode(reinterpret_cast<const unsigned char *>(pass.c_str()), pass.size());
    settings.setKeyValue(SettingFactory::NOTIF_MAIL_SMTP_PASSWORD, QString::fromStdString(passEncoded));
    settings.setKeyValue(SettingFactory::NOTIF_MAIL_SMTP_USERNAME, m_smtpUserFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::NOTIF_MAIL_SMTP_SERVER_ADRR, m_smtpServerFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::NOTIF_MAIL_SMTP_SERVER_PORT, m_smtpPortFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::NOTIF_MAIL_SMTP_USE_SSL, QString::number(int(m_smtpUseSslFieldRef->checkState())));
  }
  m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("settings updated"));
}


void WebNotificationSettings::updateEmailFieldsEnabledState(void)
{
  bool enable = m_notificationTypeFieldRef->currentIndex() == EmailNotification;
  m_smtpServerFieldRef->setEnabled(enable);
  m_smtpPortFieldRef->setEnabled(enable);
  m_smtpUseSslFieldRef->setEnabled(enable);
  m_smtpUserFieldRef->setEnabled(enable);
  m_smtpPassFieldRef->setEnabled(enable);
}


void WebNotificationSettings::updateFields(void)
{
  m_notificationTypeFieldRef->setCurrentIndex( getNotificationType() );
  m_smtpServerFieldRef->setText( getSmtpServerAddr() );
  m_smtpPortFieldRef->setText( getSmtpServerPortText() );
  m_smtpUserFieldRef->setText( getSmtpUsername() );
  m_smtpPassFieldRef->setEchoMode(Wt::EchoMode::Password);
  m_smtpPassFieldRef->setText( getSmtpPassword() );
  m_smtpUseSslFieldRef->setCheckState( static_cast<Wt::CheckState>(getSmtpUseSsl()) );
  updateEmailFieldsEnabledState();
}

