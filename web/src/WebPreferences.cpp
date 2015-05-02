/*
 * WebSettingUI.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 23-03-2014                                                  #
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
#include "WebPreferences.hpp"
#include "WebLicenseActivation.hpp"
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


WebPreferences::WebPreferences(void)
  : WebPreferencesBase(),
    Wt::WContainerWidget(),
    m_operationCompleted(this),
    m_authSystemChanged(this)
{
  setMargin(0, Wt::All);
  createAuthSettingsFields();
  createSourceSettingsFields();
  createLdapSettingsFields();
  createNotificationSettingsFields();
  createButtons();
  addEvent();
  bindFormWidget();
  loadProperties();
}


WebPreferences::~WebPreferences()
{
}


void WebPreferences::addEvent(void)
{
  m_applyChangeBtn->clicked().connect(this, &WebPreferences::applyChanges);
  m_addAsSourceBtn->clicked().connect(this, &WebPreferences::addAsSource);
  m_deleteSourceBtn->clicked().connect(this, &WebPreferences::deleteSource);
  m_monitorTypeField->activated().connect(this, &WebPreferences::showLivestatusSettings);
  m_authSettingsSaveBtn->clicked().connect(this, &WebPreferences::saveAuthSettings);
  m_notificationSettingsSaveBtn->clicked().connect(this, &WebPreferences::saveNotificationSettings);
  m_notificationTypeBox->changed().connect(this, &WebPreferences::updateEmailFieldsEnabledState);
  m_sourceSelectionBox->changed().connect(this, &WebPreferences::handleSourceBoxChanged);
  m_authenticationModeField->changed().connect(this, &WebPreferences::handleAuthTypeChanged);
  m_showAuthStringField->changed().connect(this, &WebPreferences::handleShowAuthStringChanged);
  m_ldapSslUseCertField->changed().connect(this, &WebPreferences::handleLdapUseSslChanged);
}


void WebPreferences::createLdapSettingsFields(void)
{
  m_ldapServerUriField.reset(new Wt::WLineEdit(this));
  m_ldapServerUriField->setValidator(new UriValidator("ldap", true, this));
  m_ldapServerUriField->setEmptyText("ldap://localhost:389");

  m_ldapVersionField.reset(new Wt::WComboBox(this));
  m_ldapVersionField->addItem(LDAP_VERSION3_LABEL);
  m_ldapVersionField->addItem(LDAP_VERSION2_LABEL);
  m_ldapVersionField->setCurrentIndex(0);

  m_ldapSslUseCertField.reset(new Wt::WCheckBox(QObject::tr("Use custom SSL certificate").toStdString(), this));
  m_ldapSslCertFileField.reset(new Wt::WLineEdit(this));
  m_ldapSslCertFileField->setValidator(new FileExistValidator(this));

  m_ldapSslCaFileField.reset(new Wt::WLineEdit(this));
  m_ldapSslCaFileField->setValidator(new FileExistValidator(this));

  m_ldapBindUserDnField.reset(new Wt::WLineEdit(this));
  m_ldapBindUserDnField->setEmptyText("cn=Manager,ou=devops,dc=example,dc=com");

  m_ldapBindUserPasswordField.reset(new Wt::WLineEdit(this));
  m_ldapBindUserPasswordField->setEchoMode(Wt::WLineEdit::Password);
  m_ldapBindUserPasswordField->setText("mysecretpassword");

  m_ldapIdField.reset(new Wt::WLineEdit(this));
  m_ldapIdField->setEmptyText("uid");

  m_ldapSearchBaseField.reset(new Wt::WLineEdit(this));
  m_ldapSearchBaseField->setEmptyText("ou=devops,dc=example,dc=com");
}


void WebPreferences::createAuthSettingsFields(void)
{
  m_authenticationModeField.reset(new Wt::WComboBox(this));
  m_authenticationModeField->addItem(Q_TR("Built-in"));
  m_authenticationModeField->addItem(Q_TR("LDAP"));

  m_authStringField.reset(new Wt::WLineEdit(this));
  m_authStringField->setEchoMode(Wt::WLineEdit::Password);
  m_authStringField->setEmptyText("Set the authentication string");
  m_showAuthStringField.reset(new Wt::WCheckBox(QObject::tr("Show in clear").toStdString(), this));
}


void WebPreferences::createSourceSettingsFields(void)
{
  m_sourceIndexSelector.reset(createSourceIndexSelector());

  m_sourceSelectionBox.reset(new Wt::WComboBox(this));
  m_sourceBoxModel.reset(new Wt::WStringListModel(m_sourceSelectionBox.get()));
  m_sourceSelectionBox->setModel(m_sourceBoxModel.get());


  m_monitorTypeField.reset(new Wt::WComboBox(this));
  m_monitorTypeField->addItem(ngrt4n::tr("-- Select a type --"));
  for (const auto& srcid: ngrt4n::sourceTypes()) {
    m_monitorTypeField->addItem(srcid.toStdString());
  }

  m_monitorUrlField.reset(new Wt::WLineEdit(this));
  m_monitorUrlField->setValidator(new UriValidator("http", false, this));
  m_monitorUrlField->setEmptyText("Set the url to the monitor web interface");

  // set livestatus server
  m_livestatusHostField.reset(new Wt::WLineEdit(this));
  m_livestatusHostField->setEmptyText("hostname/IP");
  m_livestatusHostField->setValidator(new HostValidator(this));

  // set livestatus port field
  m_livestatusPortField.reset(new Wt::WLineEdit(this));
  m_livestatusPortField->setWidth(50);
  m_livestatusPortField->setValidator(new PortValidator(this));
  m_livestatusPortField->setEmptyText("port");
  m_livestatusPortField->setMaxLength(5);

  // other fields
  m_useNgrt4ndField.reset(new Wt::WCheckBox(QObject::tr("Use ngrt4nd").toStdString(), this));
  m_dontVerifyCertificateField.reset(new Wt::WCheckBox(QObject::tr("Don't verify SSL certificate").toStdString(),this));

  // update interval field
  m_updateIntervalField.reset(new Wt::WSpinBox(this));
  m_updateIntervalField->setMinimum(5);
  m_updateIntervalField->setMaximum(1200);
  m_updateIntervalField->setValue(Preferences::updateInterval());
}


void WebPreferences::createButtons(void)
{
  m_applyChangeBtn.reset(new Wt::WPushButton(QObject::tr("Apply changes").toStdString(), this));
  m_addAsSourceBtn.reset(new Wt::WPushButton(QObject::tr("Add as source").toStdString(), this));
  m_deleteSourceBtn.reset(new Wt::WPushButton(QObject::tr("Delete source").toStdString(), this));
  m_authSettingsSaveBtn.reset(new Wt::WPushButton(QObject::tr("Save").toStdString(), this));

  m_applyChangeBtn->setStyleClass("btn btn-success");
  m_addAsSourceBtn->setStyleClass("btn btn-info");
  m_deleteSourceBtn->setStyleClass("btn btn-danger");
  m_authSettingsSaveBtn->setStyleClass("btn btn-info");

  m_applyChangeBtn->setDisabled(true);
  m_addAsSourceBtn->setDisabled(true);
  m_deleteSourceBtn->setDisabled(true);
}



void WebPreferences::bindFormWidget(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("setting-page.tpl"), this);
  tpl->bindWidget("show-in-clear", m_showAuthStringField.get());
  tpl->bindWidget("monitor-auth-string", m_authStringField.get());
  tpl->bindWidget("monitor-url", m_monitorUrlField.get());
  tpl->bindWidget("monitor-type", m_monitorTypeField.get());
  tpl->bindWidget("source-box", m_sourceSelectionBox.get());
  tpl->bindWidget("dont-verify-ssl-certificate", m_dontVerifyCertificateField.get());
  tpl->bindWidget("update-interval", m_updateIntervalField.get());
  tpl->bindWidget("livestatus-server", m_livestatusHostField.get());
  tpl->bindWidget("livestatus-port", m_livestatusPortField.get());
  tpl->bindWidget("use-ngrt4nd", m_useNgrt4ndField.get());

  tpl->bindWidget("apply-change-button", m_applyChangeBtn.get());
  tpl->bindWidget("add-as-source-button", m_addAsSourceBtn.get());
  tpl->bindWidget("delete-button", m_deleteSourceBtn.get());
  tpl->bindWidget("auth-settings-save-button", m_authSettingsSaveBtn.get());

  tpl->bindWidget("authentication-mode", m_authenticationModeField.get());
  tpl->bindWidget("ldap-server-uri", m_ldapServerUriField.get());
  tpl->bindWidget("ldap-version", m_ldapVersionField.get());
  tpl->bindWidget("ldap-ssl-use-custom-settings", m_ldapSslUseCertField.get());
  tpl->bindWidget("ldap-ssl-cert-file", m_ldapSslCertFileField.get());
  tpl->bindWidget("ldap-ssl-ca-file", m_ldapSslCaFileField.get());
  tpl->bindWidget("ldap-bind-user-dn", m_ldapBindUserDnField.get());
  tpl->bindWidget("ldap-bind-user-password", m_ldapBindUserPasswordField.get());
  tpl->bindWidget("ldap-uid-attribute", m_ldapIdField.get());
  tpl->bindWidget("ldap-user-search-base", m_ldapSearchBaseField.get());

  tpl->bindWidget("notification-type", m_notificationTypeBox.get());
  tpl->bindWidget("notification-settings-save-button", m_notificationSettingsSaveBtn.get());
  tpl->bindWidget("notification-mail-smtp-server", m_smtpServerAddrField.get());
  tpl->bindWidget("notification-mail-smtp-port", m_smtpServerPortField.get());
  tpl->bindWidget("notification-mail-smtp-use-ssl", m_smtpUseSslField.get());
  tpl->bindWidget("notification-mail-smtp-username", m_smtpUsernameField.get());
  tpl->bindWidget("notification-mail-smtp-password", m_smtpPasswordField.get());
}


void WebPreferences::applyChanges(void)
{
  if (! validateSourceSettingsFields())
    return ;

  if ( m_monitorTypeField->currentIndex() <= 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Invalid monitor type").toStdString());
    return;
  }

  if (currentSourceIndex() < 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Invalid data source index (%1)").arg(currentSourceIndex()).toStdString());
    return;
  }

  saveAsSource(currentSourceIndex(), m_monitorTypeField->currentText().toUTF8().c_str());
}


void WebPreferences::deleteSource(void)
{
  int curIndex = currentSourceIndex();
  if (curIndex >= 0 && curIndex < MAX_SRCS) {
    m_sourceBoxModel->removeRow(currentSourceIndex());
    setSourceState(currentSourceIndex(), false);
    setKeyValue(Settings::GLOBAL_SRC_BUCKET_KEY, sourceStatesSerialized());
    sync();
    updateFields();
  }
}


void WebPreferences::fillFromSource(int _index)
{
  if (_index >= 0 && _index < MAX_SRCS) {
    SourceT src;
    loadSource(_index, src);

    m_sourceSelectionBox->setValueText(ngrt4n::sourceId(_index).toStdString());
    m_monitorUrlField->setText(src.mon_url.toStdString());
    m_livestatusHostField->setText(src.ls_addr.toStdString());
    m_livestatusPortField->setText(QString::number(src.ls_port).toStdString());
    m_authStringField->setText(src.auth.toStdString());
    m_monitorTypeField->setCurrentIndex(src.mon_type+1);
    m_useNgrt4ndField->setCheckState(static_cast<Wt::CheckState>(src.use_ngrt4nd));
    m_dontVerifyCertificateField->setCheckState(src.verify_ssl_peer? Wt::Checked : Wt::Unchecked);
    m_updateIntervalField->setValue(updateInterval());

    showLivestatusSettings(m_monitorTypeField->currentIndex());

    // this triggers a signal
    setCurrentSourceIndex(_index);
  }
}


void WebPreferences::updateAllSourceWidgetStates(void)
{
  //FIXME: ensure the model is clear or remove duplication
  for (int index=0; index< MAX_SRCS; ++index) {
    if (isSetSource(index)) {
      m_sourceBoxModel->addString(ngrt4n::sourceId(index).toStdString());
      m_sourceBoxModel->setData(m_sourceBoxModel->rowCount() - 1, 0, index, Wt::UserRole);
    }
  }
  m_sourceBoxModel->sort(0);
}


void WebPreferences::updateFields(void)
{
  setCurrentSourceIndex(firstSourceSet());
  int curIndex = currentSourceIndex();
  if (curIndex >= 0) {
    m_sourceSelectionBox->setCurrentIndex(curIndex);
    fillFromSource(curIndex);
    m_applyChangeBtn->setDisabled(false);
    m_addAsSourceBtn->setDisabled(false);
    m_deleteSourceBtn->setDisabled(false);
  }
  fillInAuthSettings();
  fillInNotificationSettings();
}


void WebPreferences::saveAsSource(const qint32& index, const QString& type)
{
  // global settings
  setKeyValue(Settings::GLOBAL_UPDATE_INTERVAL_KEY, m_updateIntervalField->text().toUTF8().c_str());

  // source-specific settings
  SourceT src;
  src.id = ngrt4n::sourceId(index);
  src.mon_type = ngrt4n::convertToSourceType(type);
  src.mon_url = m_monitorUrlField->text().toUTF8().c_str();
  src.ls_addr = m_livestatusHostField->text().toUTF8().c_str();
  src.ls_port = QString(m_livestatusPortField->text().toUTF8().c_str()).toInt();
  src.auth = m_authStringField->text().toUTF8().c_str();
  src.use_ngrt4nd = m_useNgrt4ndField->checkState();
  src.verify_ssl_peer = (m_dontVerifyCertificateField->checkState() == Wt::Checked);
  setKeyValue(ngrt4n::sourceKey(index), ngrt4n::sourceData2Json(src));
  setSourceState(index, true);
  setKeyValue(Settings::GLOBAL_SRC_BUCKET_KEY, sourceStatesSerialized());

  // save changes
  sync();

  // emit signal a finilize
  emitTimerIntervalChanged(1000 * QString(m_updateIntervalField->text().toUTF8().c_str()).toInt());
  addToSourceBox(index);
  m_sourceSelectionBox->setCurrentIndex(findSourceIndexInBox(index));
}


Wt::WDialog* WebPreferences::createSourceIndexSelector(void)
{
  Wt::WDialog* inputDialog = new Wt::WDialog();
  inputDialog->setStyleClass("Wt-dialog");
  inputDialog->titleBar()->setStyleClass("titlebar");
  inputDialog->setWindowTitle(Q_TR("Select the source index"));

  Wt::WComboBox* inputField = new Wt::WComboBox(inputDialog->contents());
  for (const auto& src : ngrt4n::sourceIndexes()) inputField->addItem(src.toStdString());

  Wt::WPushButton *ok = new Wt::WPushButton("OK", inputDialog->footer());
  ok->clicked().connect(std::bind(&WebPreferences::handleAddAsSourceOkAction, this, inputField));
  ok->setDefault(true);

  Wt::WPushButton *cancel = new Wt::WPushButton("Cancel", inputDialog->footer());
  cancel->clicked().connect(inputDialog, &Wt::WDialog::reject);

  inputDialog->rejectWhenEscapePressed();
  return inputDialog;
}


void WebPreferences::handleAddAsSourceOkAction(Wt::WComboBox* inputBox)
{
  m_sourceIndexSelector->accept();
  int maxAllowedViews = LicenseActivationBase(PKG_VERSION).maxAllowedSources();
  qDebug() << activeSourcesCount() << maxAllowedViews;
  if (activeSourcesCount() < maxAllowedViews) {
    bool isValidIndex;
    int index = QString::fromStdString( inputBox->currentText().toUTF8() ).toInt(&isValidIndex);
    if (isValidIndex) {
      setCurrentSourceIndex(index);
    } else {
      setCurrentSourceIndex(-1);
    }
    applyChanges();
  } else {
    m_operationCompleted.emit(ngrt4n::OperationFailed,
                              QObject::tr("Can't add new data source."
                                          " Your license allows a maximum of %1 data source(s)")
                              .arg(QString::number(maxAllowedViews)).toStdString());
  }
}


void WebPreferences::addAsSource(void)
{
  if (validateSourceSettingsFields())
    m_sourceIndexSelector->show();
}


void WebPreferences::setEnabledInputs(bool enable)
{
  m_monitorUrlField->setEnabled(enable);
  m_authStringField->setEnabled(enable);
  m_livestatusHostField->setEnabled(enable);
  m_livestatusPortField->setEnabled(enable);
  m_monitorTypeField->setEnabled(enable);
  m_showAuthStringField->setEnabled(enable);
  m_useNgrt4ndField->setEnabled(enable);
  m_dontVerifyCertificateField->setEnabled(enable);
  m_updateIntervalField->setEnabled(enable);
  m_applyChangeBtn->setEnabled(enable);
  m_addAsSourceBtn->setEnabled(enable);
  m_deleteSourceBtn->setEnabled(enable);
}


int WebPreferences::getSourceGlobalIndex(int sourceBoxIndex)
{
  return boost::any_cast<int>(static_cast<Wt::WAbstractItemModel*>(m_sourceBoxModel.get())->data(sourceBoxIndex, 0, Wt::UserRole));
}


int WebPreferences::findSourceIndexInBox(int sourceGlobalIndex)
{
  int index = m_sourceBoxModel->rowCount() - 1;
  while (index >= 0 && (getSourceGlobalIndex(index) != sourceGlobalIndex)) --index;
  return index;
}


void WebPreferences::addToSourceBox(int sourceGlobalIndex)
{
  int index = findSourceIndexInBox(sourceGlobalIndex);
  if (index < 0) {
    m_sourceBoxModel->addString(ngrt4n::sourceId(sourceGlobalIndex).toStdString());
    m_sourceBoxModel->setData(m_sourceBoxModel->rowCount() - 1, 0, sourceGlobalIndex, Wt::UserRole);
  }
  m_sourceBoxModel->sort(0);
}


void WebPreferences::showAuthSettings(void)
{
  fillInAuthSettings();
  showAuthSettingsWidgets(true);
  showSourcesSettingsWidgets(false);
  showNotificationSettingsWidgets(false);
}


void WebPreferences::showAuthSettingsWidgets(bool display)
{
  std::string v = display? "true" : "false";
  wApp->doJavaScript(Wt::WString("$('#auth-section').toggle({1});"
                                 "$('#auth-setting-save-button').toggle({1});").arg(v).toUTF8());
  switch (m_settings->keyValue(Settings::AUTH_MODE_KEY).toInt()) {
    case LDAP:
      wApp->doJavaScript("$('#ldap-auth-setting-section').show();");
      break;
    case BuiltIn:
    default:
      wApp->doJavaScript("$('#ldap-auth-setting-section').hide();");
      break;
  }
}

void WebPreferences::saveAuthSettings(void)
{
  // validate fields
  if (validateAuthSettingsFields()) {
    setKeyValue(Settings::AUTH_MODE_KEY, QString::number(m_authenticationModeField->currentIndex()));
    if (getAuthenticationMode() == LDAP) {
      setKeyValue(Settings::AUTH_LDAP_SERVER_URI, m_ldapServerUriField->text().toUTF8().c_str());
      setKeyValue(Settings::AUTH_LDAP_VERSION, m_ldapVersionField->currentText().toUTF8().c_str());
      setKeyValue(Settings::AUTH_LDAP_SEARCH_BASE, m_ldapSearchBaseField->text().toUTF8().c_str());
      setKeyValue(Settings::AUTH_LDAP_BIND_USER_DN, m_ldapBindUserDnField->text().toUTF8().c_str());
      setKeyValue(Settings::AUTH_LDAP_BIND_USER_PASSWORD, m_ldapBindUserPasswordField->text().toUTF8().c_str());
      setKeyValue(Settings::AUTH_LDAP_ID_FIELD, m_ldapIdField->text().toUTF8().c_str());

      int useCert = m_ldapSslUseCertField->checkState();
      setKeyValue(Settings::AUTH_LDAP_SSL_USE_CERT, QString::number(useCert));
      if (useCert == Wt::Checked) {
        setKeyValue(Settings::AUTH_LDAP_SSL_CERT_FILE, m_ldapSslCertFileField->text().toUTF8().c_str());
        setKeyValue(Settings::AUTH_LDAP_SSL_CA_FILE, m_ldapSslCaFileField->text().toUTF8().c_str());
      }
    }
    m_authSystemChanged.emit(getAuthenticationMode());
  }
}


void WebPreferences::fillInAuthSettings(void)
{
  m_authenticationModeField->setCurrentIndex(getAuthenticationMode());
  m_ldapServerUriField->setText(getLdapServerUri());
  m_ldapVersionField->setValueText(m_settings->keyValue(Settings::AUTH_LDAP_VERSION).toStdString());
  m_ldapSearchBaseField->setText(getLdapSearchBase());
  m_ldapBindUserDnField->setText(getLdapBindUserDn());
  m_ldapBindUserPasswordField->setText(getLdapBindUserPassword());
  m_ldapIdField->setText(getLdapIdField());

  // optional settings related if secure LDAP
  Wt::CheckState useMySslCert = static_cast<Wt::CheckState>(m_settings->keyValue(Settings::AUTH_LDAP_SSL_USE_CERT).toInt());
  m_ldapSslUseCertField->setCheckState(useMySslCert);
  m_ldapSslCertFileField->setText(getLdapSslCertFile());
  m_ldapSslCaFileField->setText(getLdapSslCaFile());
  showLdapSslSettings(useMySslCert == Wt::Checked);
}



void WebPreferences::showSourceSettings(void)
{
  fillFromSource(firstSourceSet());
  showSourcesSettingsWidgets(true);
  showAuthSettingsWidgets(false);
  showNotificationSettingsWidgets(false);
}


void WebPreferences::showSourcesSettingsWidgets(bool display)
{
  std::string v = display? "true" : "false";
  wApp->doJavaScript(Wt::WString("$('#monitoring-settings-section').toggle({1});"
                                 "$('#apply-source-settings-button').toggle({1});"
                                 "$('#add-as-source-settings-button').toggle({1});"
                                 "$('#delete-source-settings-button').toggle({1});").arg(v).toUTF8());
}


void WebPreferences::showLdapSslSettings(bool display)
{
  std::string v = display ? "true" : "false";
  wApp->doJavaScript(Wt::WString("$('#ldap-custom-ssl-settings').toggle({1});").arg(v).toUTF8());
}


void WebPreferences::showLivestatusSettings(int monitorTypeIndex)
{
  switch (monitorTypeIndex) {
    case 1:
      wApp->doJavaScript("$('#livetstatus-section').show();");
      break;
    default:
      wApp->doJavaScript("$('#livetstatus-section').hide();");
      break;
  }
}


bool WebPreferences::validateSourceSettingsFields(void)
{
  if (m_monitorTypeField->currentIndex() == 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Monitor type not set").toStdString());
    return false;
  }

  if (m_monitorTypeField->currentIndex() > 1
      && m_monitorUrlField->validate() != Wt::WValidator::Valid
      ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Please fix field(s) in red").toStdString());
    return false;

  }

  return true;
}


bool WebPreferences::validateAuthSettingsFields(void)
{
  if (m_authenticationModeField->currentIndex() == BuiltIn)
    return true;

  if (m_authenticationModeField->validate() == Wt::WValidator::Valid
      && m_ldapServerUriField->validate() == Wt::WValidator::Valid
      && m_ldapSearchBaseField->validate() == Wt::WValidator::Valid
      && m_ldapBindUserDnField->validate() == Wt::WValidator::Valid
      && m_ldapBindUserPasswordField->validate() == Wt::WValidator::Valid
      && m_ldapIdField->validate() == Wt::WValidator::Valid)
    return true;

  m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Please fix field(s) in red").toStdString());
  return false;
}


void WebPreferences::handleAuthTypeChanged(void)
{
  switch (m_authenticationModeField->currentIndex()) {
    case LDAP:
      wApp->doJavaScript("$('#ldap-auth-setting-section').show();");
      break;
    case BuiltIn: // BUILT-IN
    default:
      wApp->doJavaScript("$('#ldap-auth-setting-section').hide();");
      break;
  }
}


void WebPreferences::handleShowAuthStringChanged(void)
{
  if (m_showAuthStringField->isChecked()) {
    m_authStringField->setEchoMode(Wt::WLineEdit::Normal);
  } else {
    m_authStringField->setEchoMode(Wt::WLineEdit::Password);
  }
}


void WebPreferences::handleLdapUseSslChanged(void)
{
  if (m_ldapSslUseCertField->checkState() == Wt::Checked) {
    wApp->doJavaScript("$('#ldap-custom-ssl-settings').show();");
  } else {
    wApp->doJavaScript("$('#ldap-custom-ssl-settings').hide();");
  }
}


void WebPreferences::createNotificationSettingsFields(void)
{
  m_smtpServerAddrField.reset(new Wt::WLineEdit(this));
  m_smtpServerAddrField->setEmptyText(Q_TR("smtp.example.com"));

  m_smtpServerPortField.reset(new Wt::WLineEdit(this));
  m_smtpServerPortField->setEmptyText(Q_TR("25"));

  m_smtpUseSslField.reset(new Wt::WCheckBox(this));

  m_smtpUsernameField.reset(new Wt::WLineEdit(this));
  m_smtpUsernameField->setEmptyText(Q_TR("opuser"));

  m_smtpPasswordField.reset(new Wt::WLineEdit(this));
  m_smtpPasswordField->setEchoMode(Wt::WLineEdit::Password);
  m_smtpPasswordField->setEmptyText(Q_TR("*******"));

  m_notificationTypeBox.reset(new Wt::WComboBox(this));
  m_notificationTypeBox->addItem(Q_TR("No notification"));
  m_notificationTypeBox->addItem(Q_TR("Email"));

  m_notificationSettingsSaveBtn.reset(new Wt::WPushButton(QObject::tr("Save").toStdString(), this));
  m_notificationSettingsSaveBtn->setStyleClass("btn btn-info");
}


void WebPreferences::saveNotificationSettings(void)
{
  m_settings->setEntry(Settings::NOTIF_TYPE, QString::number(m_notificationTypeBox->currentIndex()));
  if (m_notificationTypeBox->currentIndex() == EmailNotification) {
    m_settings->setEntry(Settings::NOTIF_MAIL_SMTP_SERVER_ADRR, m_smtpServerAddrField->text().toUTF8().c_str());
    m_settings->setEntry(Settings::NOTIF_MAIL_SMTP_SERVER_PORT, m_smtpServerPortField->text().toUTF8().c_str());
    m_settings->setEntry(Settings::NOTIF_MAIL_SMTP_USERNAME, m_smtpUsernameField->text().toUTF8().c_str());
    m_settings->setEntry(Settings::NOTIF_MAIL_SMTP_PASSWORD, m_smtpPasswordField->text().toUTF8().c_str());
    m_settings->setEntry(Settings::NOTIF_MAIL_SMTP_USE_SSL, QString::number(m_smtpUseSslField->checkState()));
  }
}


void WebPreferences::fillInNotificationSettings(void)
{
  m_notificationTypeBox->setCurrentIndex( getNotificationType() );
  m_smtpServerAddrField->setText( getSmtpServerAddr() );
  m_smtpServerPortField->setText( getSmtpServerPortText() );
  m_smtpUsernameField->setText( getSmtpUsername() );
  m_smtpPasswordField->setEchoMode(Wt::WLineEdit::Password);
  m_smtpPasswordField->setText( getSmtpPassword() );
  m_smtpUseSslField->setCheckState( static_cast<Wt::CheckState>(getSmtpUseSsl()) );
  updateEmailFieldsEnabledState();
}


void WebPreferences::showNotificationSettings(void)
{
  showAuthSettingsWidgets(false);
  showSourcesSettingsWidgets(false);
  showNotificationSettingsWidgets(true);
  fillInNotificationSettings();
}


void WebPreferences::showNotificationSettingsWidgets(bool display)
{
  std::string v = display? "true" : "false";
  wApp->doJavaScript(Wt::WString("$('#notification-section').toggle({1});"
                                 "$('#notification-setting-save-button').toggle({1});").arg(v).toUTF8());
}


void WebPreferences::updateEmailFieldsEnabledState(void)
{
  bool enable = m_notificationTypeBox->currentIndex() == EmailNotification;
  m_smtpServerAddrField->setEnabled(enable);
  m_smtpServerPortField->setEnabled(enable);
  m_smtpUseSslField->setEnabled(enable);
  m_smtpUsernameField->setEnabled(enable);
  m_smtpPasswordField->setEnabled(enable);
}
