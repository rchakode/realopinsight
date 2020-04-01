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
#include <memory>
#include <any>
#include <fcntl.h>
#include <QObject>
#include <QString>
#include <Wt/WTemplate.h>
#include <Wt/WDateTime.h>
#include <QDateTime>
#include <Wt/WColor.h>
#include <Wt/WApplication.h>
#include <Wt/WStandardItem.h>


namespace {
  Logger* coreLogger = nullptr;
  Logger* reportdLogger = nullptr;
}

void ngrt4n::initCoreLogger(void)
{
  coreLogger = new Logger(Logger::CoreLogger);
}

void ngrt4n::freeCoreLogger(void)
{
  delete coreLogger;
}


void ngrt4n::initReportdLogger(void)
{
  reportdLogger = new Logger(Logger::ReportdLogger);
}

void ngrt4n::freeReportdLogger(void)
{
  delete reportdLogger;
}

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
  std::string color = "#ffffff";
  switch (static_cast<ngrt4n::SeverityT>(severity)) {
  case ngrt4n::Normal:
    color = "#4b7";
    break;
  case ngrt4n::Minor:
    color ="#fa4";
    break;
  case ngrt4n::Major:
    color = "#ffa500";
    break;
  case ngrt4n::Critical:
    color = "#f56";
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

std::string ngrt4n::thumbCss(int severity)
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
  default:
    cssClass.append("btn btn-unknown");
    break;
  }
  return cssClass;
}


std::unique_ptr<Wt::WWidget> ngrt4n::footer(void)
{
  auto page = std::make_unique<Wt::WTemplate>(Wt::WString::tr("copyright-footer.tpl"));
  page->bindString("software-name", APP_NAME.toStdString());
  page->bindString("version", PKG_VERSION.toStdString());
  page->bindString("package-url", PKG_URL.toStdString());
  page->bindString("release-year", REL_YEAR.toStdString());
  return std::move(page);
}



std::string ngrt4n::sqliteDbPath(void)
{
  auto cdir = SettingFactory::coreDataDir();
  if (cdir.endsWith("/")) {
    return cdir.append("realopinsight.db").toStdString();
  }
  return cdir.append("/realopinsight.db").toStdString();
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

std::unique_ptr<Wt::WText>  ngrt4n::createFontAwesomeTextButton(const std::string& iconClasses, const std::string& tip)
{
  auto link = std::make_unique<Wt::WText>(QObject::tr("<span class=\"btn\"><i class=\"%1\"></i></span>").arg(iconClasses.c_str()).toStdString(), Wt::TextFormat::XHTML);
  link->setToolTip(tip);
  return std::move(link);
}

void ngrt4n::logCore(const std::string& level, const std::string& msg)
{
  if (coreLogger) {
    coreLogger->log(level, msg);
  } else {
    std::cerr << "DEBUG [" << level.c_str() << "] " << msg << std::endl;
  }
}

void ngrt4n::logReportd(const std::string& level, const std::string& msg)
{
  if (reportdLogger) {
    reportdLogger->log(level, msg);
  } else {
    std::cerr << "DEBUG [" << level.c_str() << "] " << msg << std::endl;
  }
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


std::unique_ptr<Wt::WStandardItem> ngrt4n::createStandardItem(const std::string& text, const std::string& data)
{
  auto item = std::make_unique<Wt::WStandardItem>(text);
  item.get()->setData(data, Wt::ItemDataRole::User);
  return item;
}

std::unique_ptr<Wt::WStandardItem> ngrt4n::createCheckableStandardItem(const std::string& data, bool checked)
{
  auto item = std::make_unique<Wt::WStandardItem>("", data);
  item.get()->setCheckable(true);
  item.get()->setChecked(checked);
  return item;
}

std::unique_ptr<Wt::WStandardItem> ngrt4n::createSeverityStandardItem(const NodeT& _node)
{
  auto item = std::make_unique<Wt::WStandardItem>();
  auto itemRef = item.get();
  itemRef->setData(QString::number(_node.sev).toStdString(), Wt::ItemDataRole::User);
  itemRef->setText(Severity(_node.sev).toString().toStdString());
  updateSeverityItem(itemRef, _node.sev);
  return std::move(item);
}

void ngrt4n::updateSeverityItem(Wt::WStandardItem* itemRef, int severity)
{
  itemRef->setText(Severity(severity).toString().toStdString());
  itemRef->setStyleClass(ngrt4n::severityCssClass(severity));
}

std::string ngrt4n::getItemData(Wt::WStandardItem* itemRef)
{
  std::string data;
  try {
    data = Wt::cpp17::any_cast<std::string>(itemRef->data(Wt::ItemDataRole::User));
  } catch(...) {
    data = "";
  }

  return data;
}


sem_t* ngrt4n::createSemaphoreOrDie(const std::string& sem_name)
{
  sem_t* my_sem = sem_open(sem_name.c_str(), O_CREAT, S_IRUSR|S_IWUSR, 1);
  if (my_sem == SEM_FAILED) {
    std::string errorMsg = Q_TR("Failed while initializing semaphore: ");
    switch (errno) {
    case EACCES:
      errorMsg += QObject::tr("permission denied to access to the semaphore %1")
          .arg(sem_name.c_str())
          .toStdString();
      break;
    default:
      errorMsg += QObject::tr("sem_open returned errno %1")
          .arg(QString::number(errno))
          .toStdString();
      break;
    }
    CORE_LOG("fatal", errorMsg);
    qFatal("%s", errorMsg.c_str());
    exit(1);
  }
  return my_sem;
}


void ngrt4n::releaseSemaphore(sem_t* my_sem)
{
  sem_close(my_sem);
}


Wt::WFont ngrt4n::chartTitleFont(void)
{
  Wt::WFont tf;
  return tf;
}

