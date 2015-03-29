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

#include "web/src/utils/Logger.hpp"
#include "Base.hpp"
#include "WebUtils.hpp"
#include <Wt/WTemplate>
#include <Wt/WDateTime>
#include <QObject>
#include <QString>
#include <QDateTime>
#include <Wt/WColor>
#include <Wt/WApplication>
#include <Wt/WStandardItem>
#include <memory>


Logger coreLogger(Logger::CoreLogger, "/opt/realopinsight/log/");
Logger reportdLogger(Logger::ReportdLogger, "/opt/realopinsight/log/");

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

std::string ngrt4n::severityHtmlColor(int severity)
{
  std::string color = "#000000";
  switch (static_cast<ngrt4n::SeverityT>(severity)) {
    case ngrt4n::Normal:
      color = "#00ff00";
      break;
    case ngrt4n::Minor:
      color ="#ffff00";
      break;
    case ngrt4n::Major:
      color = "#ffa500";
      break;
    case ngrt4n::Critical:
      color = "#ff0000";
      break;
    case ngrt4n::Unknown:
    default:
      color = "#c0c0c0";
      break;
  }
  return color;
}


Wt::WColor ngrt4n::severityWColor(int severity)
{
  return Wt::WColor(severityHtmlColor(severity));
}

std::string ngrt4n::thumbnailCssClass(int severity)
{
  std::string cssClass = "";
  switch(severity) {
    case ngrt4n::Normal:
      cssClass.append("btn btn-normal");
      break;
    case ngrt4n::Minor:
      cssClass.append("btn btn-minor");
      break;
    case ngrt4n::Major:
      cssClass.append("btn btn-major");
      break;
    case ngrt4n::Critical:
      cssClass.append("btn btn-critical");
      break;
    case ngrt4n::Unknown:
      cssClass.append("btn btn-unknown");
      break;
    default:
      cssClass.append("btn btn-unknown");
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



Wt::WString ngrt4n::wHumanTimeText(long mytime_t)
{
  Wt::WDateTime t;
  t.setTime_t(mytime_t);
  return t.toString();
}


Wt::WString ngrt4n::wHumanTimeText(const std::string& mytime_t)
{
  return wHumanTimeText(QString(mytime_t.c_str()).toUInt() );
}

Wt::WString ngrt4n::wTimeToNow(const std::string& myStrTime_t)
{
  return wTimeToNow(QString(myStrTime_t.c_str()).toUInt());
}

Wt::WString ngrt4n::wTimeToNow(long mytime_t)
{
  Wt::WDateTime dt;
  dt.setTime_t(mytime_t);
  return dt.timeTo(Wt::WDateTime::currentDateTime());
}

Wt::WString ngrt4n::timet2String(long mytime_t, const std::string& format)
{
  Wt::WDateTime dt;
  dt.setTime_t(mytime_t);
  return dt.toString(format);
}

Wt::WText* ngrt4n::createFontAwesomeTextButton(const std::string& iconClasses, const std::string& tip)
{
  Wt::WText* link = new Wt::WText(QObject::tr("<span class=\"btn\">"
                                              " <i class=\"%1\"></i>"
                                              "</span>").arg(iconClasses.c_str()).toStdString(),
                                  Wt::XHTMLText);
  link->setToolTip(tip);
  return link;
}

void ngrt4n::log(const std::string& level, const std::string& msg)
{
  coreLogger.log(level, msg);
}

void ngrt4n::logReportd(const std::string& level, const std::string& msg)
{
  reportdLogger.log(level, msg);
}

void ngrt4n::logReportd(const std::string& level, const QString& msg)
{
  logReportd(level, msg.toStdString());
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


Wt::WStandardItem* ngrt4n::createStandardItem(const std::string& text, const std::string& data)
{
  Wt::WStandardItem* item = new Wt::WStandardItem(text);
  item->setData(data, Wt::UserRole);
  return item;
}

Wt::WStandardItem* ngrt4n::createCheckableStandardItem(const std::string& data, bool checked)
{
  Wt::WStandardItem* item = createStandardItem("", data);
  item->setCheckable(true);
  item->setChecked(checked);
  return item;
}

Wt::WStandardItem* ngrt4n::createSeverityStandardItem(const NodeT& _node)
{
  Wt::WStandardItem* item = new Wt::WStandardItem();
  item->setData(QString::number(_node.sev).toStdString(), Wt::UserRole);
  item->setText(Severity(_node.sev).toString().toStdString());
  updateSeverityItem(item, _node.sev);
  return item;
}

void ngrt4n::updateSeverityItem(Wt::WStandardItem* item, int severity)
{
  item->setText(Severity(severity).toString().toStdString());
  item->setStyleClass(ngrt4n::severityCssClass(severity));
}

std::string ngrt4n::getItemData(Wt::WStandardItem* item)
{
  std::string data;
  try {
    data = boost::any_cast<std::string>(item->data(Wt::UserRole));
  } catch(...) {
    data = "";
  }

  return data;
}

