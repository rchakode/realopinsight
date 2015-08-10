/*
 * JsonHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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
#include <QDebug>
#include "JsonHelper.hpp"

JsonHelper::JsonHelper(const QString& _data) : QScriptEngine()
{
  setData(_data);
}

void JsonHelper::setData(const std::string& data)
{
  m_data = evaluate("(" + QString::fromStdString(data) + ")");
  m_isGood = ! hasUncaughtException();
  if (! m_isGood) {
    m_lastError = tr("Unexpected data: %1").arg(data.c_str());
  }
}

void JsonHelper::setData(const QString& data)
{
  setData(data.toStdString());
}

QScriptValue JsonHelper::getProperty(const std::string& key)
{
  return m_data.property(QString::fromStdString(key)) ;
}
