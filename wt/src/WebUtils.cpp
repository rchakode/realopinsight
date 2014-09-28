/*
 * WebUtils.hpp
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

#include "Base.hpp"
#include "WebUtils.hpp"
#include <Wt/WTemplate>
#include <Wt/WDateTime>
#include <QObject>
#include <QString>
#include <QDateTime>
#include <Wt/WColor>
#include <Wt/WApplication>
#include <memory>



Logger::Logger(const std::string& path)
{
  addField("datetime", false);
  addField("session", false);
  addField("type", false);
  addField("message", true);
  setFile(path);
}
void Logger::log(const std::string& level, const std::string& msg)
{
  Wt::WLogEntry logEntry = Wt::WLogger::entry(level);
  logEntry << Wt::WLogger::timestamp << Wt::WLogger::sep
           << '[' << wApp->sessionId() << ']' << Wt::WLogger::sep
           << '[' << level << ']' << Wt::WLogger::sep
           << msg;
}


Logger logger("/opt/realopinsight/log/realopinsight.log");

void ngrt4n::showMessage(int exitCode,
                         const std::string& errorMsg,
                         const std::string& successMsg, Wt::WText* infoBox)
{
  std::unique_ptr<Wt::WTemplate> tpl;
  if (exitCode != 0){
    tpl.reset(new Wt::WTemplate(Wt::WString::tr("error-msg-div-tpl")));
    tpl->bindString("msg", errorMsg);
  } else {
    tpl.reset(new Wt::WTemplate(Wt::WString::tr("success-msg-div-tpl")));
    tpl->bindString("msg", successMsg);
  }

  std::ostringstream oss;
  tpl->renderTemplate(oss);
  infoBox->setText(oss.str());
}


std::string ngrt4n::tr(const std::string& msg)
{
  return QObject::tr(msg.c_str()).toStdString();
}

std::string ngrt4n::severityCssClass(int severity)
{
  std::string cssClass = "";
  switch(severity) {
  case ngrt4n::Normal:
    cssClass.append("severity-normal");
    break;
  case ngrt4n::Minor:
    cssClass.append("severity-minor");
    break;
  case ngrt4n::Major:
    cssClass.append("severity-major");
    break;
  case ngrt4n::Critical:
    cssClass.append("severity-critical");
    break;
  case ngrt4n::Unknown:
    cssClass.append("severity-unknown");
    break;
  default:
    cssClass.append("default-item-background");
    break;
  }
  return cssClass;
}

std::string ngrt4n::getPathFromQtResource(const QString& qtPath, const std::string& docRoot)
{
  return docRoot+qtPath.mid(2, -1).toStdString();
}


Wt::WWidget* ngrt4n::footer(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("copyright-footer.tpl"));
  tpl->bindString("software", APP_NAME.toStdString());
  tpl->bindString("version", PKG_VERSION.toStdString());
  tpl->bindString("package-url", PKG_URL.toStdString());
  tpl->bindString("release-year", REL_YEAR.toStdString());
  return tpl;
}



std::string ngrt4n::sqliteDbPath(void)
{
  return "/opt/realopinsight/data/realopinsight.db";
}


std::string ngrt4n::timeTextFromQtTimeText(const std::string& qtimeText)
{
  Wt::WDateTime mytime;

  mytime.setTime_t(QDateTime::fromString(QString(qtimeText.c_str())).toTime_t());

  return mytime.toString().toUTF8();
}


Wt::WString ngrt4n::wHumanTimeText(const std::string& mytime_t)
{
  Wt::WDateTime t;
  t.setTime_t(QString(mytime_t.c_str()).toUInt());
  return t.toString();
}

Wt::WString ngrt4n::wTimeToNow(const std::string& mytime_t)
{
  Wt::WDateTime t;
  t.setTime_t(QString(mytime_t.c_str()).toUInt());

  return t.timeTo(Wt::WDateTime::currentDateTime());
}


Wt::WText* ngrt4n::createFontAwesomeTextButton(const std::string& iconClasses, const std::string& tip)
{
  Wt::WText* link = new Wt::WText(QObject::tr("<span class=\"btn\">"
                                              " <i class=\"%1\"></i>"
                                              "</span>")
                                  .arg(iconClasses.c_str()).toStdString(),
                                  Wt::XHTMLText);
  link->setToolTip(tip);
  return link;
}


Wt::WColor ngrt4n::severityWColor(const int& _criticity)
{
  Wt::WColor color;
  switch (static_cast<ngrt4n::SeverityT>(_criticity)) {
  case ngrt4n::Normal:
    color = Wt::WColor("#00ff00");
    break;
  case ngrt4n::Minor:
    color = Wt::WColor("#ffff00");
    break;
  case ngrt4n::Major:
    color = Wt::WColor("#ffa500");
    break;
  case ngrt4n::Critical:
    color = Wt::WColor("#ff0000");
    break;
  case ngrt4n::Unknown:
  default:
    color = Wt::WColor(" #c0c0c0");
    break;
  }
  return color;
}

void ngrt4n::log(const std::string& level, const std::string& msg)
{
  logger.log(level, msg);
}


bool ngrt4n::isValidUri(const QString& addr, const QString& schemePrefix, bool nopath)
{
  QUrl qurl(addr);
  QString scheme = qurl.scheme();
  QString secureSchemePrefix = schemePrefix+"s";
  if ( ! addr.isEmpty()
       && ! scheme.isEmpty()
       && qurl.isValid()
       && qurl.path().isEmpty() == nopath
       && (scheme == schemePrefix || scheme == secureSchemePrefix) )
    return true;

  return false;
}

bool ngrt4n::isValidHostAddr(const QString& addr)
{
  QUrl qurl(addr);
  if ( ! addr.isEmpty()
       && qurl.scheme().isEmpty()
       && ! qurl.host().isEmpty()
       && qurl.port(-1) == -1
       && qurl.path().isEmpty() )
    return true;

  return false;
}

std::string ngrt4n::md5Hash(const std::string& input)
{
  return std::string(QCryptographicHash::hash(input.c_str(), QCryptographicHash::Md5).data());
}

void ngrt4n::googleAnalyticsLogger(void)
{
  wApp->doJavaScript("ga('create', 'UA-8836482-12', 'auto'); ga('send', 'pageview');");
}
