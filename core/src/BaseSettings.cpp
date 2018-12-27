/*
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Change: 17-12-2017                                                  #
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


#include "BaseSettings.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include <sstream>
#include <QIntValidator>
#include <QRegExpValidator>


BaseSettings::BaseSettings(void)
  : m_settingFactory(new SettingFactory())
{
}

BaseSettings::BaseSettings(const QString& settingFile)
  : m_settingFactory(new SettingFactory(settingFile))
{
}


BaseSettings::~BaseSettings(void)
{
  delete m_settingFactory;
}

void BaseSettings::loadProperties(void)
{
  updateAllSourceWidgetStates();
  updateFields();
}

int BaseSettings::getGraphLayout(void) const
{
  return m_settingFactory->getGraphLayout();
}

qint32 BaseSettings::updateInterval(void) const
{
  return m_settingFactory->updateInterval();
}

void BaseSettings::sync(void)
{
  m_settingFactory->sync();
}

QString BaseSettings::keyValue(const QString& key, const QString& defaultValue)
{
  return m_settingFactory->value(key, defaultValue).toString();
}

void BaseSettings::setKeyValue(const QString & _key, const QString & _value)
{
  m_settingFactory->setKeyValue(_key, _value); m_settingFactory->sync();
}


