/*
 * WebSettingUI.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 29-12-2013                                                 #
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

#include "utilsClient.hpp"
#include "WebUtils.hpp"
#include "WebPreferences.hpp"
#include <QString>
#include <Wt/WTemplate>
#include <Wt/WContainerWidget>
#include <Wt/WButtonGroup>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WIntValidator>


#define VALIDATE_FIELDS()   if (m_monitorTypeField->validate() != Wt::WValidator::Valid \
  || m_livestatusPortField->validate() != Wt::WValidator::Valid \
  || m_monitorTypeField->currentIndex() <= 0) { \
  m_errorOccurred.emit(QObject::tr("Please verify the monitor type and fix field(s) in red").toStdString());\
  return; \
  }
WebPreferences::WebPreferences(void)
  : Preferences("/var/lib/realopinsight/config/realopinsight.conf"),
    m_errorOccurred(this)

{
  m_dialog = std::make_shared<Wt::WDialog>(ngrt4n::tr("Preferences - " +APP_NAME.toStdString()));
  m_dialog->setStyleClass("Wt-dialog");
  m_dialog->titleBar()->setStyleClass("titlebar");

  Wt::WContainerWidget* mainContainer = m_dialog->contents();
  mainContainer->setMargin(0, Wt::All);

  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("setting-page.tpl"), mainContainer);

  m_sourceBox = std::make_shared<Wt::WComboBox>(mainContainer);
  m_sourceBoxModel = std::make_shared<Wt::WStringListModel>(m_sourceBox.get());
  m_sourceBox->setModel(m_sourceBoxModel.get());

  m_sourceBox->changed().connect(std::bind([=]() {
    fillFromSource(getSourceGlobalIndex(m_sourceBox->currentIndex()));
  }));
  tpl->bindWidget("source-box", m_sourceBox.get());

  m_monitorTypeField = std::make_shared<Wt::WComboBox>(mainContainer);
  m_monitorTypeField->addItem(ngrt4n::tr("-- Select a type --"));
  for (const auto& srcid: ngrt4n::sourceTypes()) {
    m_monitorTypeField->addItem(srcid.toStdString());
  }

  m_monitorTypeField->changed().connect(std::bind([=]() {
    if (m_sourceBox->currentIndex() != 1) {
      m_livestatusHostField->setEnabled(false);
      m_livestatusPortField->setEnabled(false);
    } else {
      m_livestatusHostField->setEnabled(true);
      m_livestatusPortField->setEnabled(true);
    }
  }));
  tpl->bindWidget("monitor-type", m_monitorTypeField.get());

  m_monitorUrlField = std::make_shared<Wt::WLineEdit>(mainContainer);
  m_monitorUrlField->setValidator(createTextValidator());
  m_monitorUrlField->setEmptyText("e.g. http://server.example.com/monitor");
  tpl->bindWidget("monitor-url", m_monitorUrlField.get());

  m_authStringField = std::make_shared<Wt::WLineEdit>(mainContainer);
  tpl->bindWidget("auth-string", m_authStringField.get());
  m_authStringField->setEchoMode(Wt::WLineEdit::Password);

  m_showAuthStringField = std::make_shared<Wt::WCheckBox>(QObject::tr("Show in clear").toStdString(), mainContainer);
  tpl->bindWidget("show-in-clear", m_showAuthStringField.get());

  m_showAuthStringField->changed().connect(std::bind([=](){
    if (m_showAuthStringField->isChecked()) {
      m_authStringField->setEchoMode(Wt::WLineEdit::Normal);
    } else {
      m_authStringField->setEchoMode(Wt::WLineEdit::Password);
    }
  }));

  m_livestatusHostField = std::make_shared<Wt::WLineEdit>(mainContainer);
  m_livestatusHostField->setEmptyText("hostname/IP");
  m_livestatusHostField->setValidator(createTextValidator());
  tpl->bindWidget("livestatus-server", m_livestatusHostField.get());

  m_livestatusPortField = std::make_shared<Wt::WLineEdit>(mainContainer);
  m_livestatusPortField->setWidth(50);
  m_livestatusPortField->setValidator(createPortValidator());
  m_livestatusPortField->setEmptyText("port");
  m_livestatusPortField->setMaxLength(5);
  tpl->bindWidget("livestatus-port", m_livestatusPortField.get());

  m_useNgrt4ndField = std::make_shared<Wt::WCheckBox>(QObject::tr("Use ngrt4nd").toStdString(), mainContainer);
  tpl->bindWidget("use-ngrt4nd", m_useNgrt4ndField.get());

  m_dontVerifyCertificateField = std::make_shared<Wt::WCheckBox>(QObject::tr("Don't verify SSL certificate").toStdString(),mainContainer);
  tpl->bindWidget("dont-verify-ssl-certificate", m_dontVerifyCertificateField.get());

  m_updateIntervalField = std::make_shared<Wt::WSpinBox>(mainContainer);
  tpl->bindWidget("update-interval", m_updateIntervalField.get());

  m_applyChangeBtn = std::make_shared<Wt::WPushButton>(QObject::tr("Apply changes").toStdString(), mainContainer);
  tpl->bindWidget("apply-change-button", m_applyChangeBtn.get());

  m_addAsSourceBtn = std::make_shared<Wt::WPushButton>(QObject::tr("Add as source").toStdString(), mainContainer);
  tpl->bindWidget("add-as-source-button", m_addAsSourceBtn.get());

  m_deleteSourceBtn = std::make_shared<Wt::WPushButton>(QObject::tr("Delete source").toStdString(), mainContainer);
  tpl->bindWidget("delete-button", m_deleteSourceBtn.get());

  m_applyChangeBtn->setStyleClass("btn btn-success");
  m_addAsSourceBtn->setStyleClass("btn btn-info");
  m_deleteSourceBtn->setStyleClass("btn btn-danger");

  m_applyChangeBtn->clicked().connect(this, &WebPreferences::applyChanges);
  m_addAsSourceBtn->clicked().connect(this, &WebPreferences::addAsSource);
  m_deleteSourceBtn->clicked().connect(this, &WebPreferences::deleteSource);

  m_updateIntervalField->setMinimum(5);
  m_updateIntervalField->setMaximum(1200);
  m_updateIntervalField->setValue(Preferences::updateInterval());

  m_applyChangeBtn->setDisabled(true);
  m_addAsSourceBtn->setDisabled(true);
  m_deleteSourceBtn->setDisabled(true);

  loadProperties();
}


WebPreferences::~WebPreferences()
{
}

void WebPreferences::applyChanges(void)
{
  VALIDATE_FIELDS();

  if ( m_monitorTypeField->currentIndex() <= 0) {
    m_errorOccurred.emit(QObject::tr("Bad monitor type").toStdString());
    return;
  }
  if (m_currentSourceIndex < 0) {
    m_errorOccurred.emit(QObject::tr("Bad index for source (%1)").arg(m_currentSourceIndex).toStdString());
    return;
  }
  saveAsSource(m_currentSourceIndex, m_monitorTypeField->currentText().toUTF8().c_str());
}

void WebPreferences::deleteSource(void)
{
  if (m_currentSourceIndex>=0 && m_currentSourceIndex < MAX_SRCS) {
    m_sourceBoxModel->removeRow(m_currentSourceIndex);
    m_sourceStates->setBit(m_currentSourceIndex, false);
    m_settings->setEntry(Settings::SRC_BUCKET_KEY, getSourceStatesSerialized());
    m_settings->sync();
    updateFields();
  }
}


void WebPreferences::fillFromSource(int _sidx)
{
  SourceT src;
  m_settings->loadSource(_sidx, src);
  m_monitorUrlField->setText(src.mon_url.toStdString());
  m_livestatusHostField->setText(src.ls_addr.toStdString());
  m_livestatusPortField->setText(QString::number(src.ls_port).toStdString());
  m_authStringField->setText(src.auth.toStdString());
  m_monitorTypeField->setCurrentIndex(src.mon_type+1);
  m_useNgrt4ndField->setCheckState(static_cast<Wt::CheckState>(src.use_ngrt4nd));
  m_dontVerifyCertificateField->setCheckState(src.verify_ssl_peer? Wt::Checked : Wt::Unchecked);
  m_updateIntervalField->setValue(m_settings->updateInterval());
  m_currentSourceIndex = _sidx;
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
  m_currentSourceIndex = firstSourceSet();
  if (m_currentSourceIndex >= 0) {
    m_sourceBox->setCurrentIndex(m_currentSourceIndex);
    fillFromSource(m_currentSourceIndex);
    m_applyChangeBtn->setDisabled(false);
    m_addAsSourceBtn->setDisabled(false);
    m_deleteSourceBtn->setDisabled(false);
  }
}

void WebPreferences::saveAsSource(const qint32& index, const QString& type)
{
  SourceT src;
  src.id = ngrt4n::sourceId(index);
  src.mon_type = ngrt4n::convert2ApiType(type);
  src.mon_url = m_monitorUrlField->text().toUTF8().c_str();
  src.ls_addr = m_livestatusHostField->text().toUTF8().c_str();
  src.ls_port = QString(m_livestatusPortField->text().toUTF8().c_str()).toInt();
  src.auth = m_authStringField->text().toUTF8().c_str();
  src.use_ngrt4nd = m_useNgrt4ndField->checkState();
  src.verify_ssl_peer = (m_dontVerifyCertificateField->checkState() == Wt::Checked);
  m_settings->setEntry(ngrt4n::sourceKey(index), ngrt4n::sourceData2Json(src));
  m_settings->setEntry(Settings::UPDATE_INTERVAL_KEY, m_updateIntervalField->text().toUTF8().c_str());
  m_sourceStates->setBit(index, true);
  m_settings->setEntry(Settings::SRC_BUCKET_KEY, getSourceStatesSerialized());
  m_settings->sync();
  m_settings->emitTimerIntervalChanged(1000 * QString(m_updateIntervalField->text().toUTF8().c_str()).toInt());
  addToSourceBox(index);
  m_sourceBox->setCurrentIndex(findSourceIndexInBox(index));
}


void WebPreferences::promptUser(int inputType)
{

  Wt::WDialog* inputDialog = new Wt::WDialog();
  inputDialog->setStyleClass("Wt-dialog");
  inputDialog->titleBar()->setStyleClass("titlebar");

  Wt::WComboBox *inputField = new Wt::WComboBox(inputDialog->contents());
  std::string dialogTitle;
  switch (inputType){
  case SourceTypeInput:
    dialogTitle = QObject::tr("Select source type").toStdString();
    for (const auto& src : ngrt4n::sourceTypes())
      inputField->addItem(src.toStdString());
    break;
  case SourceIndexInput:
    dialogTitle = QObject::tr("Select the source index").toStdString();
    for (const auto& src : ngrt4n::sourceIndexes())
      inputField->addItem(src.toStdString());
    break;
  default:
    break;
  }
  inputDialog->setWindowTitle(dialogTitle);
  Wt::WPushButton *ok = new Wt::WPushButton("OK", inputDialog->footer());
  ok->setDefault(true);

  Wt::WPushButton *cancel = new Wt::WPushButton("Cancel", inputDialog->footer());
  cancel->clicked().connect(inputDialog, &Wt::WDialog::reject);
  inputDialog->rejectWhenEscapePressed();

  ok->clicked().connect(std::bind([=] () {
    if (inputField->validate()) {
      inputDialog->accept();
      handleInput(inputField->currentText().toUTF8(), inputType);
    }
  }));

  inputDialog->show();
}


void WebPreferences::handleInput(const std::string& input, int inputType)
{
  switch(inputType) {
  case SourceIndexInput:
    m_currentSourceIndex = input[0]-48;
    applyChanges();
    break;
  default:
    //Do nothing
    break;
  }
}

void WebPreferences::addAsSource(void)
{
  VALIDATE_FIELDS();
  promptUser(SourceIndexInput);
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


Wt::WIntValidator* WebPreferences::createPortValidator(void)
{
  Wt::WIntValidator* validator = new Wt::WIntValidator();
  validator->setRange(1, 65535);
  return validator;
}


Wt::WLengthValidator* WebPreferences::createTextValidator(void)
{
  Wt::WLengthValidator* validator = new Wt::WLengthValidator();
  validator->setMinimumLength(1);
  return validator;
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
