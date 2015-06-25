/*
 * WebPreferencesBase.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation: 11-04-2015                                                     #
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

#ifndef WEBPREFERENCESBASE_HPP
#define WEBPREFERENCESBASE_HPP

#include "Preferences.hpp"
#include <Wt/WCheckBox>


const std::string LDAP_VERSION2_LABEL = "LDAPv2";
const std::string LDAP_VERSION3_LABEL = "LDAPv3";

class WebPreferencesBase : public Preferences
{
public:
  enum AuthenticationModeT {
    BuiltIn = 0,
    LDAP = 1
  };

  enum NotificationTypeT {
    NoNotification = 0,
    EmailNotification = 1
  };

  WebPreferencesBase(void);
  int getDbState(void) { return keyValue(Settings::GLOBAL_DB_STATE_KEY, "0").toInt();}
  void setDbState(int state) {setKeyValue(Settings::GLOBAL_DB_STATE_KEY, QString::number(state)); sync();}

  std::string getLdapServerUri(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_SERVER_URI).toStdString();}
  std::string getLdapBindUserDn(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_BIND_USER_DN).toStdString();}
  std::string getLdapSearchBase(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_SEARCH_BASE).toStdString();}
  std::string getLdapBindUserPassword(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_BIND_USER_PASSWORD).toStdString();}
  std::string getLdapIdField(void) const;
  int getLdapVersion(void) const;
  int getAuthenticationMode(void) const;
  bool getLdapSslUseMyCert(void) const {return m_settings->keyValue(Settings::AUTH_LDAP_SSL_USE_CERT).toInt() == Wt::Checked;}
  std::string getLdapSslCertFile(void) const {return m_settings->keyValue(Settings::AUTH_LDAP_SSL_CERT_FILE).toStdString();}
  std::string getLdapSslCaFile(void) const {return m_settings->keyValue(Settings::AUTH_LDAP_SSL_CA_FILE).toStdString();}
  static std::string authTypeString(int authSystem) {return (authSystem == LDAP) ? "LDAP" : "Built-in";}
  int getNotificationType(void) const { return m_settings->keyValue(Settings::NOTIF_TYPE).toInt();}
  std::string getSmtpServerAddr(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_SERVER_ADRR).toStdString();}
  std::string getSmtpServerPortText(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_SERVER_PORT).toStdString();}
  int getSmtpServerPort(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_SERVER_PORT).toInt();}
  std::string getSmtpUsername(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_USERNAME).toStdString();}
  std::string getSmtpPassword(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_PASSWORD).toStdString();}
  int getSmtpUseSsl(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_USE_SSL).toInt();}

  QString getLicenseKey(void) {return keyValue(Settings::ACTIVATION_LICENSE_KEY, "");}
  int activeSourceIds(QVector<std::string>& result);


protected :
  virtual void fillFromSource(int srcIndex) {}
  virtual void updateAllSourceWidgetStates(void) {}
  virtual void updateFields(void) {}
  virtual void saveAsSource(const qint32& idx, const QString& type){}

protected Q_SLOTS:
  virtual void applyChanges(void) {}
  virtual void handleCancel(void) {}
  virtual void addAsSource(void) {}
  virtual void deleteSource(void){}
};
#endif // WEBPREFERENCESBASE_HPP
