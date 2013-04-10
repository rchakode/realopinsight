/*
 * JsonHelper.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 15-08-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#ifndef JSHELPER_HPP
#define JSHELPER_HPP

#include <string>
#include <QString>
#include <QtScript/QScriptEngine>

class JsonHelper : public QScriptEngine
{
public:
  JsonHelper(const std::string& _data = "");
  JsonHelper(const QString& _data = "");
  void setData(const std::string& _data);
  void setData(const QString& _data);
  QScriptValue getProperty(const std::string& key);
private :
  QScriptValue mdata;
};

#endif // JSHELPER_HPP
