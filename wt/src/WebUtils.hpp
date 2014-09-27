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

#include "Settings.hpp"
#include <Wt/WText>
#include <Wt/WLogger>


#define Q_TR(s) QObject::tr(s).toStdString()


class Logger : public Wt::WLogger {
public:
  Logger(const std::string& path);
  void log(const std::string& level, const std::string& msg);
};

#define LOG(level, msg) ngrt4n::log(level, msg)

class QString;
namespace ngrt4n {
  void showMessage(int exitCode, const std::string& errorMsg,
                   const std::string& successMsg, Wt::WText* infoBox);
  std::string tr(const std::string& msg);
  std::string severityCssClass(int severity);
  std::string getPathFromQtResource(const QString& qtPath, const std::string& docRoot="");
  Wt::WWidget* footer(void);
  std::string sqliteDbPath(void);
  std::string timeTextFromQtTimeText(const std::string& qtimeText);
  Wt::WString wHumanTimeText(const std::string& mytime_t);
  Wt::WString wTimeToNow(const std::string& mytime_t);
  Wt::WText* createFontAwesomeTextButton(const std::string& iconClasses, const std::string& tip);
  Wt::WColor severityWColor(const int& _criticity);
  void log(const std::string& level, const std::string& msg);
  bool isValidUri(const QString& addr, const QString& schemePrefix, bool nopath);
  bool isValidHostAddr(const QString& addr);
  std::string md5Hash(const std::string& input);
  void googleAnalyticsLogger(void);
} //Namespace

#endif // WEBUTILS_HPP
