/*
 * WebUtils.cpp
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

#ifndef WEBUTILS_HPP
#define WEBUTILS_HPP

#include "semaphore.h"
#include "SettingsHandler.hpp"
#include <QString>
#include <Wt/WText>


#define Q_TR(s) QObject::tr(s).toStdString()
#define CORE_LOG(level, msg) ngrt4n::logCore(level, msg)
#define REPORTD_LOG(level, msg) ngrt4n::logReportd(level, msg)


namespace ngrt4n {

  enum OperationStatusT {
    OperationSucceeded,
    OperationFailed,
    DatabaseInitializationRequired
  };

  void showMessage(int exitCode, const std::string& errorMsg,
                   const std::string& successMsg, Wt::WText* infoBox);
  std::string tr(const std::string& msg);
  std::string severityCssClass(int severity);
  std::string severityHtmlColor(int severity);
  std::string thumbnailCssClass(int severity);
  Wt::WColor severityWColor(int severity);
  std::string getPathFromQtResource(const QString& qtPath, const std::string& docRoot="");
  Wt::WWidget* footer(void);
  std::string sqliteDbPath(void);
  std::string timeTextFromQtTimeText(const std::string& qtimeText);
  Wt::WString timet2String(long mytime_t, const std::string& format = "dd/MM hh:mm:ss");
  Wt::WString wHumanTimeText(long mytime_t);
  Wt::WString wHumanTimeText(const std::string& mytime_t);
  Wt::WString wTimeToNow(const std::string& myStrTime_t);
  Wt::WString wTimeToNow(long mytime_t);
  Wt::WText* createFontAwesomeTextButton(const std::string& iconClasses, const std::string& tip);
  void initCoreLogger(void);
  void initReportdLogger(void);
  void freeCoreLogger(void);
  void freeReportdLogger(void);
  void logCore(const std::string& level, const std::string& msg);
  void logReportd(const std::string& level, const std::string& msg);
  void logReportd(const std::string& level, const QString& msg);
  bool isValidUri(const QString& addr, const QString& schemePrefix, bool nopath);
  bool isValidHostAddr(const QString& addr);
  std::string md5Hash(const std::string& input);
  void googleAnalyticsLogger(void);

  Wt::WStandardItem* createStandardItem(const std::string& text, const std::string& data);
  Wt::WStandardItem* createCheckableStandardItem(const std::string& data, bool checked);
  Wt::WStandardItem* createSeverityStandardItem(const NodeT& _node);
  void updateSeverityItem(Wt::WStandardItem* item, int severity);
  std::string getItemData(Wt::WStandardItem* item);
  sem_t* createSemaphoreOrDie(const std::string& sem_name);
  void releaseSemaphore(sem_t* my_sem);
  Wt::WFont chartTitleFont(void);
} //Namespace


#endif // WEBUTILS_HPP
