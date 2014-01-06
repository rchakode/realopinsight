/*
 * WebUtils.cpp
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

#ifndef WEBUTILS_HPP
#define WEBUTILS_HPP

#include <Wt/WText>

#define LOG(level, msg) \
Wt::log(level)<< "[realopinsight]" << Wt::WLogger::sep << msg;

class QString;
namespace utils {
  void showMessage(int exitCode, const std::string& errorMsg,
                   const std::string& successMsg, Wt::WText* infoBox);
  std::string tr(const std::string& msg);
  std::string severityCssClass(int severity);
  std::string getPathFromQtResource(const QString& qtPath, const std::string& docRoot="");
  Wt::WWidget* footer(void);

} //Namespace

#endif // WEBUTILS_HPP
