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
#include <Wt/WTemplate>
#include <Wt/WContainerWidget>
#include <Wt/WButtonGroup>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WIntValidator>
#include <Wt/WApplication>


WebNotificationSettings::WebNotificationSettings(void)
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("notification-settings-form.tpl"))
{
  setMargin(0, Wt::All);
  createFormWidgets();
  bindFormWidgets();
  addEvent();
  updateContents();
}


WebNotificationSettings::~WebNotificationSettings(void)
{
  unbindFormWidgets();
}


void WebNotificationSettings::addEvent(void)
{
  m_notificationSettingsSaveBtn.clicked().connect(this, &WebNotificationSettings::saveChanges);
  m_notificationTypeBox.changed().connect(this, &WebNotificationSettings::updateEmailFieldsEnabledState);
}



void WebNotificationSettings::bindFormWidgets(void)
{
  bindWidget("notification-type", &m_notificationTypeBox);
  bindWidget("notification-settings-save-button", &m_notificationSettingsSaveBtn);
  bindWidget("notification-mail-smtp-server", &m_smtpServerAddrField);
  bindWidget("notification-mail-smtp-port", &m_smtpServerPortField);
  bindWidget("notification-mail-smtp-use-ssl", &m_smtpUseSslField);
  bindWidget("notification-mail-smtp-username", &m_smtpUsernameField);
  bindWidget("notification-mail-smtp-password", &m_smtpPasswordField);
}


void WebNotificationSettings::unbindFormWidgets(void)
{
  takeWidget("notification-type");
  takeWidget("notification-settings-save-button");
  takeWidget("notification-mail-smtp-server");
  takeWidget("notification-mail-smtp-port");
  takeWidget("notification-mail-smtp-use-ssl");
  takeWidget("notification-mail-smtp-username");
  takeWidget("notification-mail-smtp-password");
}


void WebNotificationSettings::createFormWidgets(void)
{
  m_smtpServerAddrField.setEmptyText(Q_TR("smtp.example.com"));
  m_smtpServerPortField.setEmptyText(Q_TR("25"));
  m_smtpUsernameField.setEmptyText(Q_TR("opuser"));
  m_smtpPasswordField.setEchoMode(Wt::WLineEdit::Password);
  m_smtpPasswordField.setEmptyText(Q_TR("*******"));
  m_notificationTypeBox.addItem(Q_TR("No notification"));
  m_notificationTypeBox.addItem(Q_TR("Email"));
  m_notificationSettingsSaveBtn.setText( Q_TR("Save") );
  m_notificationSettingsSaveBtn.setStyleClass("btn btn-info");
}


void WebNotificationSettings::saveChanges(void)
{
  m_settings->setEntry(SettingsHandler::NOTIF_TYPE, QString::number(m_notificationTypeBox.currentIndex()));
  if (m_notificationTypeBox.currentIndex() == EmailNotification) {
    m_settings->setEntry(SettingsHandler::NOTIF_MAIL_SMTP_SERVER_ADRR, m_smtpServerAddrField.text().toUTF8().c_str());
    m_settings->setEntry(SettingsHandler::NOTIF_MAIL_SMTP_SERVER_PORT, m_smtpServerPortField.text().toUTF8().c_str());
    m_settings->setEntry(SettingsHandler::NOTIF_MAIL_SMTP_USERNAME, m_smtpUsernameField.text().toUTF8().c_str());
    m_settings->setEntry(SettingsHandler::NOTIF_MAIL_SMTP_PASSWORD, m_smtpPasswordField.text().toUTF8().c_str());
    m_settings->setEntry(SettingsHandler::NOTIF_MAIL_SMTP_USE_SSL, QString::number(m_smtpUseSslField.checkState()));
    Q_EMIT operationCompleted(ngrt4n::OperationSucceeded, Q_TR("Notification settings updated"));
  }
}


void WebNotificationSettings::updateEmailFieldsEnabledState(void)
{
  bool enable = m_notificationTypeBox.currentIndex() == EmailNotification;
  m_smtpServerAddrField.setEnabled(enable);
  m_smtpServerPortField.setEnabled(enable);
  m_smtpUseSslField.setEnabled(enable);
  m_smtpUsernameField.setEnabled(enable);
  m_smtpPasswordField.setEnabled(enable);
}


void WebNotificationSettings::updateFields(void)
{
  m_notificationTypeBox.setCurrentIndex( getNotificationType() );
  m_smtpServerAddrField.setText( getSmtpServerAddr() );
  m_smtpServerPortField.setText( getSmtpServerPortText() );
  m_smtpUsernameField.setText( getSmtpUsername() );
  m_smtpPasswordField.setEchoMode(Wt::WLineEdit::Password);
  m_smtpPasswordField.setText( getSmtpPassword() );
  m_smtpUseSslField.setCheckState( static_cast<Wt::CheckState>(getSmtpUseSsl()) );
  updateEmailFieldsEnabledState();
}

