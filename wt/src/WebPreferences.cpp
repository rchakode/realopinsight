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

WebPreferences::WebPreferences(int _userRole)
  : Preferences(),
    m_dialog(new Wt::WDialog(ngrt4n::tr("Preferences - " +APP_NAME.toStdString())))
{
  m_dialog->setStyleClass("Wt-dialog");
  m_dialog->titleBar()->setStyleClass("titlebar");
  Wt::WContainerWidget* container = m_dialog->contents();
  container->setMargin(0, Wt::All);

  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("setting-page.tpl"), container);

  tpl->bindWidget("info-box", m_infoBox = new Wt::WText("", container));

  m_srcBtnGroup = new Wt::WButtonGroup(container);
  for (int i=0; i< 10; ++i) {
    std::string srcId = QString(i+48).toStdString();
    Wt::WRadioButton* button;
    tpl->bindWidget(QString("s%1").arg(srcId.c_str()).toStdString(), button = new Wt::WRadioButton(srcId));
    m_srcBtnGroup->addButton(button, i);
  }

  m_srcBtnGroup->checkedChanged().connect(std::bind([=](){
    int selectedIndex = m_srcBtnGroup->checkedId();
    fillFromSource(selectedIndex);
  }));

  tpl->bindWidget("monitor-type", m_monitorTypeField = new Wt::WComboBox(container));
  m_monitorTypeField->addItem(ngrt4n::tr("Select a monitor type"));
  for (const auto& srcid: ngrt4n::sourceTypes())
    m_monitorTypeField->addItem(srcid.toStdString());
  m_monitorTypeField->activated().connect(std::bind([=](int index){
    bool enable = index>0;
    m_applyChangeBtn->setDisabled(! enable);
    m_addAsSourceBtn->setDisabled(! enable);
  }, std::placeholders::_1));

  tpl->bindWidget("monitor-url", m_monitorUrlField = new Wt::WLineEdit(container));
  m_monitorUrlField->setEmptyText("http://server-address/monitor");

  tpl->bindWidget("auth-string", m_authStringField = new Wt::WLineEdit(container));
  m_authStringField->setEchoMode(Wt::WLineEdit::Password);

  tpl->bindWidget("livestatus-server", m_livestatusHostField = new Wt::WLineEdit(container));
  tpl->bindWidget("livestatus-port", m_livestatusPortField = new Wt::WLineEdit(container));
  m_livestatusPortField->setWidth(50);
  tpl->bindWidget("use-ngrt4nd", m_useNgrt4ndField = new Wt::WCheckBox(ngrt4n::tr("Use ngrt4nd"), container));

  m_livestatusHostField->setEmptyText("server-address");
  m_livestatusPortField->setEmptyText("1983");
  m_livestatusPortField->setValidator(new Wt::WIntValidator());
  m_livestatusPortField->setMaxLength(5);

  tpl->bindWidget("dont-verify-ssl-certificate", m_dontVerifyCertificateField = new Wt::WCheckBox(ngrt4n::tr("Don't verify SSL certificate"), container));
  tpl->bindWidget("show-in-clear", m_clearAuthStringField = new Wt::WCheckBox(ngrt4n::tr("Show in clear"), container));
  tpl->bindWidget("update-interval", m_updateIntervalField = new Wt::WSpinBox(container));

  tpl->bindWidget("apply-change-button", m_applyChangeBtn = new Wt::WPushButton(ngrt4n::tr("Apply changes"), container));
  tpl->bindWidget("add-as-source-button", m_addAsSourceBtn = new Wt::WPushButton(ngrt4n::tr("Add as source"), container));
  tpl->bindWidget("delete-button", m_deleteSourceBtn = new Wt::WPushButton(ngrt4n::tr("Delete source"), container));

  m_applyChangeBtn->setStyleClass("btn btn-success");
  m_addAsSourceBtn->setStyleClass("btn btn-info");
  m_deleteSourceBtn->setStyleClass("btn btn-danger");

  m_applyChangeBtn->clicked().connect(this, &WebPreferences::applyChanges);
  m_addAsSourceBtn->clicked().connect(this, &WebPreferences::addAsSource);
  m_deleteSourceBtn->clicked().connect(this, &WebPreferences::deleteSource);

  m_clearAuthStringField->changed().connect(std::bind([=](){
    if (m_clearAuthStringField->isChecked()) {
      m_authStringField->setEchoMode(Wt::WLineEdit::Normal);
    } else {
      m_authStringField->setEchoMode(Wt::WLineEdit::Password);
    }
  }));

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
  delete m_dialog;
}

void WebPreferences::applyChanges(void)
{
  if ( m_monitorTypeField->currentIndex() <= 0) {
    ngrt4n::showMessage(-1, ngrt4n::tr("Bad monitor type"), "", m_infoBox);
  } else {
    saveAsSource(m_currentSourceIndex, m_monitorTypeField->currentText().toUTF8().c_str());
  }
}

void WebPreferences::deleteSource(void)
{
  if (m_currentSourceIndex>=0 && m_currentSourceIndex < MAX_SRCS) {
    m_srcBtnGroup->button(m_currentSourceIndex)->setEnabled(false);
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
  m_dontVerifyCertificateField->setCheckState(src.verify_ssl_peer? Wt::Unchecked : Wt::Checked);
  m_updateIntervalField->setValue(m_settings->updateInterval());
  m_currentSourceIndex = _sidx;
}


void WebPreferences::updateSourceBtnState(void)
{
  int size = m_srcBtnGroup->count();
  for (int i=0; i < size; ++i) {
    m_srcBtnGroup->button(i)->setEnabled(m_sourceStates->at(i));
  }
}


void WebPreferences::updateFields(void)
{
  m_currentSourceIndex = firstSourceSet();
  if (m_currentSourceIndex >= 0) {
    m_srcBtnGroup->setCheckedButton(m_srcBtnGroup->button(m_currentSourceIndex));
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
  m_settings->setEntry(ngrt4n::sourceKey(index), ngrt4n::source2Str(src));
  m_settings->setEntry(Settings::UPDATE_INTERVAL_KEY, m_updateIntervalField->text().toUTF8().c_str());
  m_sourceStates->setBit(index, true);
  m_settings->setEntry(Settings::SRC_BUCKET_KEY, getSourceStatesSerialized());
  m_settings->sync();
  m_settings->emitTimerIntervalChanged(1000 * QString(m_updateIntervalField->text().toUTF8().c_str()).toInt());


  if (! m_srcBtnGroup->button(index)->isEnabled()) {
    //FIXME: consider only if source is used in the loaded service view?
    m_srcBtnGroup->button(index)->setEnabled(true);
    m_srcBtnGroup->setSelectedButtonIndex(index);
  }

  m_currentSourceIndex = index;
  updateSourceBtnState();
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
    std::cout << "source <<" <<m_currentSourceIndex <<"\n";
    applyChanges();
    break;
  default:
    //Do nothing
    break;
  }
}

void WebPreferences::addAsSource(void)
{
  promptUser(SourceIndexInput);
}

void WebPreferences::setEnabledInputs(bool enable)
{
 m_monitorUrlField->setEnabled(enable);
 m_authStringField->setEnabled(enable);
 m_livestatusHostField->setEnabled(enable);
 m_livestatusPortField->setEnabled(enable);
 m_monitorTypeField->setEnabled(enable);
 m_clearAuthStringField->setEnabled(enable);
 m_useNgrt4ndField->setEnabled(enable);
 m_dontVerifyCertificateField->setEnabled(enable);
 m_updateIntervalField->setEnabled(enable);
 m_applyChangeBtn->setEnabled(enable);
 m_addAsSourceBtn->setEnabled(enable);
 m_deleteSourceBtn->setEnabled(enable);
}
