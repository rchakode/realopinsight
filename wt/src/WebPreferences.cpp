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

  Wt::WButtonGroup* srcBtnGroup = new Wt::WButtonGroup(container);
  for (int i=0; i< 10; ++i) {
    std::string srcId = QString(i+48).toStdString();
    Wt::WRadioButton* button;
    tpl->bindWidget(QString("s%1").arg(srcId.c_str()).toStdString(), button = new Wt::WRadioButton(srcId));
    srcBtnGroup->addButton(button, i);
    m_sourceButtons.insert(srcId, button);
  }

  tpl->bindWidget("monitor-type", m_monitorTypeField = new Wt::WComboBox(container));
  m_monitorTypeField->addItem(utils::tr("Select a monitor type"));
  for (const auto& srcid: utils::sourceTypes()) m_monitorTypeField->addItem(srcid.toStdString());

  tpl->bindWidget("monitor-url", m_monitorUrlField = new Wt::WLineEdit(container));
  m_monitorUrlField->setEmptyText("http://server-address/monitor");

  tpl->bindWidget("auth-string", m_authStringField = new Wt::WLineEdit(container));
  m_authStringField->setEchoMode(Wt::WLineEdit::Password);

  tpl->bindWidget("livestatus-server", m_livestatusServerField = new Wt::WLineEdit(container));
  m_livestatusServerField->setEmptyText("server-address:port");

  tpl->bindWidget("dont-verify-ssl-certificate", m_dontVeriftSslCertificateField = new Wt::WCheckBox(utils::tr("Don't verify SSL certificate"), container));
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

  m_cancelBtn->clicked().connect(this, &WebPreferences::handleClose);
  //m_applyChangeBtn->clicked().connect(this, &WebPreferences::applySettings);
  //m_addAsSourceBtn->clicked().connect(this, &WebPreferences::addAsSource);
  //m_deleteSourceBtn->clicked().connect(this, &WebPreferences::deleteSource);

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
}


WebPreferences::~WebPreferences()
{
  delete m_dialog;
}


QString WebPreferences::selectSourceType(void)
{

  return "";
}
