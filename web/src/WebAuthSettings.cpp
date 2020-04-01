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
#include <Wt/WTemplate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WButtonGroup.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WIntValidator.h>
#include <Wt/WApplication.h>


WebAuthSettings::WebAuthSettings(void)
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("auth-settings-form.tpl"))
{
  setMargin(0, Wt::AllSides);

  auto ldapServerURI = std::make_unique<Wt::WLineEdit>();
  m_ldapServerURIFieldRef = ldapServerURI.get();
  ldapServerURI->setValidator(std::make_unique<UriValidator>("ldap", true, this));
  ldapServerURI->setPlaceholderText("ldap://localhost:389");
  bindWidget("ldap-server-uri", std::move(ldapServerURI));

  auto ldapVersion = std::make_unique<Wt::WComboBox>();
  m_ldapVersionFieldRef = ldapVersion.get();
  ldapVersion->addItem(LDAP_VERSION3_LABEL);
  ldapVersion->addItem(LDAP_VERSION2_LABEL);
  ldapVersion->setCurrentIndex(0);
  bindWidget("ldap-version", std::move(ldapVersion));

  auto ldapSslUseCert = std::make_unique<Wt::WCheckBox>();
  m_ldapSslUseCertFieldRef = ldapSslUseCert.get();
  ldapSslUseCert->setText( Q_TR("Use custom SSL certificate") );
  ldapSslUseCert->changed().connect(this, &WebAuthSettings::handleLdapUseSslChanged);
  bindWidget("ldap-ssl-use-custom-settings", std::move(ldapSslUseCert));

  auto ldapSslCertFile = std::make_unique<Wt::WLineEdit>();
  m_ldapSslCertFileFieldRef = ldapSslCertFile.get();
  ldapSslCertFile->setValidator(std::make_unique<FileExistValidator>(this));
  bindWidget("ldap-ssl-cert-file", std::move(ldapSslCertFile));

  auto ldapSslCaFileField = std::make_unique<Wt::WLineEdit>();
  m_ldapSslCaFileFieldRef = ldapSslCaFileField.get();
  ldapSslCaFileField->setValidator(std::make_unique<FileExistValidator>(this));
  bindWidget("ldap-ssl-ca-file", std::move(ldapSslCaFileField));

  auto ldapBindDn = std::make_unique<Wt::WLineEdit>();
  m_ldapBindDnFieldRef = ldapBindDn.get();
  ldapBindDn->setPlaceholderText("cn=Manager,ou=devops,dc=example,dc=com");
  bindWidget("ldap-bind-user-dn", std::move(ldapBindDn));

  auto ldapBindPass = std::make_unique<Wt::WLineEdit>();
  m_ldapBindPassFieldRef = ldapBindPass.get();
  ldapBindPass->setEchoMode(Wt::EchoMode::Password);
  ldapBindPass->setText("mysecretpassword");
  bindWidget("ldap-bind-user-password", std::move(ldapBindPass));

  auto ldapUID = std::make_unique<Wt::WLineEdit>();
  m_ldapUIDFieldRef = ldapUID.get();
  ldapUID->setPlaceholderText("uid");
  bindWidget("ldap-uid-attribute", std::move(ldapUID));

  auto ldapSearchBase = std::make_unique<Wt::WLineEdit>();
  m_ldapSearchBaseFieldRef = ldapSearchBase.get();
  ldapSearchBase->setPlaceholderText("ou=devops,dc=example,dc=com");
  bindWidget("ldap-user-search-base", std::move(ldapSearchBase));

  auto authMode = std::make_unique<Wt::WComboBox>();
  m_authModeFieldRef = authMode.get();
  authMode->addItem(Q_TR("Built-in"));
  authMode->addItem(Q_TR("LDAP"));
  authMode->changed().connect(this, &WebAuthSettings::handleAuthTypeChanged);
  bindWidget("authentication-mode", std::move(authMode));

  auto applyBtn = std::make_unique<Wt::WPushButton>(Q_TR("Save"));
  m_applyBtnRef = applyBtn.get();
  applyBtn->setStyleClass("btn btn-info");
  applyBtn->clicked().connect(this, &WebAuthSettings::saveChanges);
  bindWidget("auth-settings-save-button", std::move(applyBtn));

  updateContents();
}


WebAuthSettings::~WebAuthSettings() {}

void WebAuthSettings::saveChanges(void)
{
  if (! validateAuthSettingsFields()) {
    return;
  }
  SettingFactory settings;
  settings.setKeyValue(SettingFactory::AUTH_MODE_KEY, QString::number(m_authModeFieldRef->currentIndex()));
  if (getAuthenticationMode() == LDAP) {
    auto ldapPass = m_ldapBindPassFieldRef->text().toUTF8();
    auto encodedPass = SettingFactory::base64Encode(reinterpret_cast<const unsigned char *>(ldapPass.c_str()), ldapPass.size());
    settings.setKeyValue(SettingFactory::AUTH_LDAP_BIND_USER_PASSWORD, QString::fromStdString(encodedPass));
    settings.setKeyValue(SettingFactory::AUTH_LDAP_SERVER_URI, m_ldapServerURIFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::AUTH_LDAP_VERSION, m_ldapVersionFieldRef->currentText().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::AUTH_LDAP_SEARCH_BASE, m_ldapSearchBaseFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::AUTH_LDAP_BIND_USER_DN, m_ldapBindDnFieldRef->text().toUTF8().c_str());
    settings.setKeyValue(SettingFactory::AUTH_LDAP_ID_FIELD, m_ldapUIDFieldRef->text().toUTF8().c_str());
    int useCert = static_cast<int>(m_ldapSslUseCertFieldRef->checkState());
    settings.setKeyValue(SettingFactory::AUTH_LDAP_SSL_USE_CERT, QString::number(useCert));
    if (useCert == static_cast<int>(Wt::CheckState::Checked)) {
      settings.setKeyValue(SettingFactory::AUTH_LDAP_SSL_CERT_FILE, m_ldapSslCertFileFieldRef->text().toUTF8().c_str());
      settings.setKeyValue(SettingFactory::AUTH_LDAP_SSL_CA_FILE, m_ldapSslCaFileFieldRef->text().toUTF8().c_str());
    }
  }
  m_authSystemChanged.emit(getAuthenticationMode());
}


bool WebAuthSettings::validateAuthSettingsFields(void)
{
  if (m_authModeFieldRef->currentIndex() == BuiltIn) {
    return true;
  }

  if (m_authModeFieldRef->validate() == Wt::ValidationState::Valid
      && m_ldapServerURIFieldRef->validate() == Wt::ValidationState::Valid
      && m_ldapSearchBaseFieldRef->validate() == Wt::ValidationState::Valid
      && m_ldapBindDnFieldRef->validate() == Wt::ValidationState::Valid
      && m_ldapBindPassFieldRef->validate() == Wt::ValidationState::Valid
      && m_ldapUIDFieldRef->validate() == Wt::ValidationState::Valid)
  {
    return true;
  }

  m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Please fix field(s) in red").toStdString());

  return false;
}


void WebAuthSettings::handleAuthTypeChanged(void)
{
  switch (m_authModeFieldRef->currentIndex()) {
  case LDAP:
    wApp->doJavaScript("$('#ldap-auth-setting-section').show();");
    break;
  case BuiltIn: // BUILT-IN
  default:
    wApp->doJavaScript("$('#ldap-auth-setting-section').hide();");
    break;
  }
}


void WebAuthSettings::handleLdapUseSslChanged(void)
{
  if (m_ldapSslUseCertFieldRef->checkState() == Wt::CheckState::Checked) {
    wApp->doJavaScript("$('#ldap-custom-ssl-settings').show();");
  } else {
    wApp->doJavaScript("$('#ldap-custom-ssl-settings').hide();");
  }
}


void WebAuthSettings::updateFields(void)
{
  SettingFactory settings;
  m_authModeFieldRef->setCurrentIndex(getAuthenticationMode());
  m_ldapServerURIFieldRef->setText(getLdapServerUri());
  m_ldapVersionFieldRef->setValueText(settings.keyValue(SettingFactory::AUTH_LDAP_VERSION).toStdString());
  m_ldapSearchBaseFieldRef->setText(getLdapSearchBase());
  m_ldapBindDnFieldRef->setText(getLdapBindUserDn());
  m_ldapBindPassFieldRef->setText(getLdapBindUserPassword());
  m_ldapUIDFieldRef->setText(getLdapIdField());

  // optional settings related if secure LDAP
  Wt::CheckState useMySslCert = static_cast<Wt::CheckState>(settings.keyValue(SettingFactory::AUTH_LDAP_SSL_USE_CERT).toInt());
  m_ldapSslUseCertFieldRef->setCheckState(useMySslCert);
  m_ldapSslCertFileFieldRef->setText(getLdapSslCertFile());
  m_ldapSslCaFileFieldRef->setText(getLdapSslCaFile());
  handleAuthTypeChanged();
}

