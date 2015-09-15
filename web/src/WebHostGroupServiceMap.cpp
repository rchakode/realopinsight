/*
 * WebImportHostGroupMap.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 25-06-2015                                                     #
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

#include "WebHostGroupServiceMap.hpp"
#include "WebUtils.hpp"
#include "utilsCore.hpp"

WebHostGroupServiceMap::WebHostGroupServiceMap()
  : WebPreferencesBase(),
    Wt::WTemplate(Wt::WString::tr("import-hostgroup-form.tpl")),
    m_operationCompleted(this),
    m_hostgroupSubmitted(this)
{
  updateFormWidgets();
  bindFormWidgets();
  addEvent();
}


WebHostGroupServiceMap::~WebHostGroupServiceMap()
{
  unbindFormWidgets();
}


void WebHostGroupServiceMap::addEvent(void)
{
  m_submitButton.clicked().connect(this, &WebHostGroupServiceMap::handleSubmitTriggerred);
}


void WebHostGroupServiceMap::updateDataSourceList(void)
{
  QVector<std::string> sourceIds;
  if (activeSourceIds(sourceIds) > 0) {
    m_sourceListBox.clear();
    for(auto sid : sourceIds) m_sourceListBox.addItem(sid);
  }
}

void WebHostGroupServiceMap::updateFormWidgets(void)
{
  m_submitButton.setText(Q_TR("Submit"));
  m_submitButton.setStyleClass("btn btn-info");
  updateDataSourceList();
}


void WebHostGroupServiceMap::bindFormWidgets(void)
{
  bindWidget("source-list-field", &m_sourceListBox);
  bindWidget("host-group-filter", &m_hostGroupFilterField);
  bindWidget("submit-button", &m_submitButton);
}


void WebHostGroupServiceMap::unbindFormWidgets(void)
{
  takeWidget("source-list-field");
  takeWidget("host-group-filter");
  takeWidget("submit-button");
}



void WebHostGroupServiceMap::handleSubmitTriggerred(void)
{
  QMap<QString, SourceT> sourceList = fetchSourceList(MonitorT::Auto);
  QString srcId = QString::fromStdString( m_sourceListBox.currentText().toUTF8() );
  SourceT srcInfo = sourceList[srcId];
  QString hostgroup = QString::fromStdString( m_hostGroupFilterField.text().toUTF8() );
  m_hostgroupSubmitted.emit(srcInfo, hostgroup);
}
