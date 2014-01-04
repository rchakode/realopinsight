/*
 * WebUtils.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 06-12-2013                                                 #
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

#include "WebUtils.hpp"
#include "MonitorBroker.hpp"
#include <Wt/WTemplate>
#include <QObject>
#include <QString>


void utils::showMessage(int exitCode,
                        const std::string& errorMsg,
                        const std::string& successMsg, Wt::WText* infoBox)
{
  Wt::WTemplate* tpl = NULL;
  if (exitCode != 0){
    tpl = new Wt::WTemplate(Wt::WString::tr("error-msg-div-tpl"));
    tpl->bindString("msg", errorMsg);
  } else {
    tpl = new Wt::WTemplate(Wt::WString::tr("success-msg-div-tpl"));
    tpl->bindString("msg", successMsg);
  }

  if (tpl) {
    std::ostringstream oss;
    tpl->renderTemplate(oss);
    infoBox->setText(oss.str());
    delete tpl;
  }
}


std::string utils::tr(const std::string& msg)
{
  return QObject::tr(msg.c_str()).toStdString();
}

std::string utils::severityCssClass(int severity)
{
  std::string cssClass = "";
  switch(severity) {
    case MonitorBroker::Normal:
      cssClass.append("severity-normal");
      break;
    case MonitorBroker::Minor:
      cssClass.append("severity-minor");
      break;
    case MonitorBroker::Major:
      cssClass.append("severity-major");
      break;
    case MonitorBroker::Critical:
      cssClass.append("severity-critical");
      break;
    case MonitorBroker::Unknown:
    default:
      cssClass.append("severity-unknown");
      break;
  }
  return cssClass;
}

std::string utils::getPathFromQtResource(const QString& qtPath, const std::string& docRoot)
{
  return docRoot+qtPath.mid(1, -1).toStdString();
}


