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
#include <Wt/WTemplate>
#include <Wt/WContainerWidget>
#include <Wt/WButtonGroup>
#include <QString>

WebPreferences::WebPreferences(int _userRole)
  : Preferences(_userRole, Preferences::WebForm),
    m_dialog(new Wt::WDialog(utils::tr("Preferences - " +APP_NAME.toStdString())))
{
  m_dialog->setStyleClass("ngrt4n-gradient Wt-dialog");
  m_dialog->titleBar()->setStyleClass("titlebar");
  Wt::WContainerWidget* container = m_dialog->contents();
  container->setMargin(0, Wt::All);
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("setting-page.tpl"), container);

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
  m_monitorTypeField->addItem(utils::tr("Select a monitor type"));
  for (const auto& srcid: utils::sourceTypes()) m_monitorTypeField->addItem(srcid.toStdString());

  tpl->bindWidget("monitor-url", m_monitorUrlField = new Wt::WLineEdit(container));
  m_monitorUrlField->setEmptyText("http://server-address/monitor");

  tpl->bindWidget("auth-string", m_authStringField = new Wt::WLineEdit(container));
  m_authStringField->setEchoMode(Wt::WLineEdit::Password);

  tpl->bindWidget("livestatus-server", m_livestatusAddressField = new Wt::WLineEdit(container));
  tpl->bindWidget("use-ngrt4nd", m_useNgrt4ndField = new Wt::WCheckBox(utils::tr("Use ngrt4nd"), container));
  m_livestatusAddressField->setEmptyText("server-address:port");

  tpl->bindWidget("dont-verify-ssl-certificate", m_dontVerifyCertificateField = new Wt::WCheckBox(utils::tr("Don't verify SSL certificate"), container));
  tpl->bindWidget("show-in-clear", m_clearAuthStringField = new Wt::WCheckBox(utils::tr("Show in clear"), container));
  tpl->bindWidget("update-interval", m_updateIntervalField = new Wt::WSpinBox(container));

  tpl->bindWidget("close-button", m_cancelBtn = new Wt::WPushButton(utils::tr("Close"), container));
  tpl->bindWidget("apply-change-button", m_applyChangeBtn = new Wt::WPushButton(utils::tr("Apply changes"), container));
  tpl->bindWidget("add-as-source-button", m_addAsSourceBtn = new Wt::WPushButton(utils::tr("Add as source"), container));
  tpl->bindWidget("delete-button", m_deleteSourceBtn = new Wt::WPushButton(utils::tr("Delete source"), container));


  m_cancelBtn->setStyleClass("btn");
  m_applyChangeBtn->setStyleClass("btn btn-success");
  m_addAsSourceBtn->setStyleClass("btn btn-info");
  m_deleteSourceBtn->setStyleClass("btn btn-danger");

  m_applyChangeBtn->clicked().connect(this, &WebPreferences::applyChanges);
  //m_addAsSourceBtn->clicked().connect(this, &WebPreferences::addAsSource);
  m_addAsSourceBtn->clicked().connect(std::bind([=](){
  }));
  m_deleteSourceBtn->clicked().connect(this, &WebPreferences::deleteSource);
  m_cancelBtn->clicked().connect(this, &WebPreferences::handleClose);

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

  loadProperties();
}


WebPreferences::~WebPreferences()
{
  delete m_dialog;
}

void WebPreferences::handleCancel(void)
{

}

void WebPreferences::applyChanges(void)
{
  saveAsSource(m_currentSourceIndex, letUserSelectType());
}

QString WebPreferences::letUserSelectType(void)
{
  return "";
}


void WebPreferences::addAsSource(void)
{

}

void WebPreferences::deleteSource(void)
{

}


void WebPreferences::fillFromSource(int _sidx)
{
  SourceT src;
  m_settings->loadSource(_sidx, src);
  m_monitorUrlField->setText(src.mon_url.toStdString());
  m_livestatusAddressField->setText(QString("%1:%2").arg(src.ls_addr, src.ls_port).toStdString());
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
  }
}

void WebPreferences::saveAsSource(const qint32& _idx, const QString& _stype)
{
  SourceT src;
  src.id = utils::sourceId(_idx);
  src.mon_type = utils::convert2ApiType(_stype);
  src.mon_url = m_monitorUrlField->text().toUTF8().c_str();
  src.ls_addr = m_livestatusAddressField->text().toUTF8().c_str();
  src.ls_port = QString(m_livestatusAddressField->text().toUTF8().c_str()).toInt();
  src.auth = m_authStringField->text().toUTF8().c_str();
  src.use_ngrt4nd = m_useNgrt4ndField->checkState();
  src.verify_ssl_peer = (m_dontVerifyCertificateField->checkState() == Wt::Checked);
  m_settings->setEntry(utils::sourceKey(_idx), utils::source2Str(src));
  m_settings->setEntry(Settings::UPDATE_INTERVAL_KEY, m_updateIntervalField->text().toUTF8().c_str());
  m_sourceStates->setBit(_idx, true);
  m_settings->setEntry(Settings::SRC_BUCKET_KEY, getSourceStatesSerialized());
  m_settings->sync();
  m_settings->emitTimerIntervalChanged(1000 * QString(m_updateIntervalField->text().toUTF8().c_str()).toInt());


  if (! m_srcBtnGroup->button(_idx)->isEnabled()) {
    //FIXME: consider only if source is used in the loaded service view?
    m_srcBtnGroup->button(_idx)->setEnabled(true);
    m_srcBtnGroup->setSelectedButtonIndex(_idx);
  }

  m_currentSourceIndex = _idx;
  updateSourceBtnState();
}
