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


BaseSettings::BaseSettings()
{
}


BaseSettings::~BaseSettings(void)
{
}

void BaseSettings::loadProperties(void)
{
  updateAllSourceWidgetStates();
  updateFields();
}

int BaseSettings::getGraphLayout(void) const
{
  return SettingFactory().getGraphLayout();
}

qint32 BaseSettings::updateInterval(void) const
{
  return SettingFactory().updateInterval();
}

