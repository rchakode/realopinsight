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
#include "WebAuthSettings.hpp"
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


WebAuthSettings::WebAuthSettings(void)
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("auth-settings-form.tpl")),
    m_operationCompleted(this),
    m_authSystemChanged(this)
{
  setMargin(0, Wt::All);
  createAuthSettingsFields();
  createLdapSettingsFields();
  createButtons();
  addEvent();
  bindFormWidgets();
  updateContents();
}


WebAuthSettings::~WebAuthSettings()
{
  unbindFormWidgets();
}


void WebAuthSettings::addEvent(void)
{
  m_authSettingsSaveBtn.clicked().connect(this, &WebAuthSettings::saveChanges);
  m_authenticationModeField.changed().connect(this, &WebAuthSettings::handleAuthTypeChanged);
  m_ldapSslUseCertField.changed().connect(this, &WebAuthSettings::handleLdapUseSslChanged);
}


void WebAuthSettings::createLdapSettingsFields(void)
{
  m_ldapServerUriField.setValidator(new UriValidator("ldap", true, this));
  m_ldapServerUriField.setEmptyText("ldap://localhost:389");
  m_ldapVersionField.addItem(LDAP_VERSION3_LABEL);
  m_ldapVersionField.addItem(LDAP_VERSION2_LABEL);
  m_ldapVersionField.setCurrentIndex(0);
  m_ldapSslUseCertField.setText( Q_TR("Use custom SSL certificate") );
  m_ldapSslCertFileField.setValidator(new FileExistValidator(this));
  m_ldapSslCaFileField.setValidator(new FileExistValidator(this));
  m_ldapBindUserDnField.setEmptyText("cn=Manager,ou=devops,dc=example,dc=com");
  m_ldapBindUserPasswordField.setEchoMode(Wt::WLineEdit::Password);
  m_ldapBindUserPasswordField.setText("mysecretpassword");
  m_ldapIdField.setEmptyText("uid");
  m_ldapSearchBaseField.setEmptyText("ou=devops,dc=example,dc=com");
}



void WebAuthSettings::createAuthSettingsFields(void)
{
  m_authenticationModeField.addItem(Q_TR("Built-in"));
  m_authenticationModeField.addItem(Q_TR("LDAP"));

  //FIXME: create m_authStringField and bind it
  m_authStringField.setEchoMode(Wt::WLineEdit::Password);
  m_authStringField.setEmptyText( Q_TR ("Set the authentication string") );
  m_showAuthStringField.setText( Q_TR("Show in clear") );
}



void WebAuthSettings::createButtons(void)
{
  m_authSettingsSaveBtn.setText( Q_TR("Save") );
  m_authSettingsSaveBtn.setStyleClass("btn btn-info");
}



void WebAuthSettings::bindFormWidgets(void)
{
  bindWidget("auth-settings-save-button", &m_authSettingsSaveBtn);
  bindWidget("authentication-mode", &m_authenticationModeField);
  bindWidget("ldap-server-uri", &m_ldapServerUriField);
  bindWidget("ldap-version", &m_ldapVersionField);
  bindWidget("ldap-ssl-use-custom-settings", &m_ldapSslUseCertField);
  bindWidget("ldap-ssl-cert-file", &m_ldapSslCertFileField);
  bindWidget("ldap-ssl-ca-file", &m_ldapSslCaFileField);
  bindWidget("ldap-bind-user-dn", &m_ldapBindUserDnField);
  bindWidget("ldap-bind-user-password", &m_ldapBindUserPasswordField);
  bindWidget("ldap-uid-attribute", &m_ldapIdField);
  bindWidget("ldap-user-search-base", &m_ldapSearchBaseField);
}


void WebAuthSettings::unbindFormWidgets(void)
{
  takeWidget("auth-settings-save-button");
  takeWidget("authentication-mode");
  takeWidget("ldap-server-uri");
  takeWidget("ldap-version");
  takeWidget("ldap-ssl-use-custom-settings");
  takeWidget("ldap-ssl-cert-file");
  takeWidget("ldap-ssl-ca-file");
  takeWidget("ldap-bind-user-dn");
  takeWidget("ldap-bind-user-password");
  takeWidget("ldap-uid-attribute");
  takeWidget("ldap-user-search-base");
}


void WebAuthSettings::saveChanges(void)
{
  // validate fields
  if (validateAuthSettingsFields()) {
    setKeyValue(SettingFactory::AUTH_MODE_KEY, QString::number(m_authenticationModeField.currentIndex()));
    if (getAuthenticationMode() == LDAP) {
      setKeyValue(SettingFactory::AUTH_LDAP_SERVER_URI, m_ldapServerUriField.text().toUTF8().c_str());
      setKeyValue(SettingFactory::AUTH_LDAP_VERSION, m_ldapVersionField.currentText().toUTF8().c_str());
      setKeyValue(SettingFactory::AUTH_LDAP_SEARCH_BASE, m_ldapSearchBaseField.text().toUTF8().c_str());
      setKeyValue(SettingFactory::AUTH_LDAP_BIND_USER_DN, m_ldapBindUserDnField.text().toUTF8().c_str());
      setKeyValue(SettingFactory::AUTH_LDAP_BIND_USER_PASSWORD, m_ldapBindUserPasswordField.text().toUTF8().c_str());
      setKeyValue(SettingFactory::AUTH_LDAP_ID_FIELD, m_ldapIdField.text().toUTF8().c_str());

      int useCert = m_ldapSslUseCertField.checkState();
      setKeyValue(SettingFactory::AUTH_LDAP_SSL_USE_CERT, QString::number(useCert));
      if (useCert == Wt::Checked) {
        setKeyValue(SettingFactory::AUTH_LDAP_SSL_CERT_FILE, m_ldapSslCertFileField.text().toUTF8().c_str());
        setKeyValue(SettingFactory::AUTH_LDAP_SSL_CA_FILE, m_ldapSslCaFileField.text().toUTF8().c_str());
      }
    }
    m_authSystemChanged.emit(getAuthenticationMode());
  }
}




void WebAuthSettings::showLdapSslSettings(bool display)
{
  std::string v = display ? "true" : "false";
  wApp->doJavaScript(Wt::WString("$('#ldap-custom-ssl-settings').toggle({1});").arg(v).toUTF8());
}



bool WebAuthSettings::validateAuthSettingsFields(void)
{
  if (m_authenticationModeField.currentIndex() == BuiltIn)
    return true;

  if (m_authenticationModeField.validate() == Wt::WValidator::Valid
      && m_ldapServerUriField.validate() == Wt::WValidator::Valid
      && m_ldapSearchBaseField.validate() == Wt::WValidator::Valid
      && m_ldapBindUserDnField.validate() == Wt::WValidator::Valid
      && m_ldapBindUserPasswordField.validate() == Wt::WValidator::Valid
      && m_ldapIdField.validate() == Wt::WValidator::Valid)
    return true;

  m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Please fix field(s) in red").toStdString());
  return false;
}


void WebAuthSettings::handleAuthTypeChanged(void)
{
  switch (m_authenticationModeField.currentIndex()) {
    case LDAP:
      wApp->doJavaScript("$('#ldap-auth-setting-section').show();");
      break;
    case BuiltIn: // BUILT-IN
    default:
      wApp->doJavaScript("$('#ldap-auth-setting-section').hide();");
      break;
  }
}


void WebAuthSettings::handleShowAuthStringChanged(void)
{
  if (m_showAuthStringField.isChecked()) {
    m_authStringField.setEchoMode(Wt::WLineEdit::Normal);
  } else {
    m_authStringField.setEchoMode(Wt::WLineEdit::Password);
  }
}


void WebAuthSettings::handleLdapUseSslChanged(void)
{
  if (m_ldapSslUseCertField.checkState() == Wt::Checked) {
    wApp->doJavaScript("$('#ldap-custom-ssl-settings').show();");
  } else {
    wApp->doJavaScript("$('#ldap-custom-ssl-settings').hide();");
  }
}


void WebAuthSettings::updateFields(void)
{
  m_authenticationModeField.setCurrentIndex(getAuthenticationMode());
  m_ldapServerUriField.setText(getLdapServerUri());
  m_ldapVersionField.setValueText(m_settingFactory->keyValue(SettingFactory::AUTH_LDAP_VERSION).toStdString());
  m_ldapSearchBaseField.setText(getLdapSearchBase());
  m_ldapBindUserDnField.setText(getLdapBindUserDn());
  m_ldapBindUserPasswordField.setText(getLdapBindUserPassword());
  m_ldapIdField.setText(getLdapIdField());

  // optional settings related if secure LDAP
  Wt::CheckState useMySslCert = static_cast<Wt::CheckState>(m_settingFactory->keyValue(SettingFactory::AUTH_LDAP_SSL_USE_CERT).toInt());
  m_ldapSslUseCertField.setCheckState(useMySslCert);
  m_ldapSslCertFileField.setText(getLdapSslCertFile());
  m_ldapSslCaFileField.setText(getLdapSslCaFile());
  handleAuthTypeChanged();
}

