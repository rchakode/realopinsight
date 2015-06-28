/*
 * Preferences.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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


#include "Preferences.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include <sstream>
#include <QIntValidator>
#include <QRegExpValidator>


Preferences::Preferences(void)
  : m_settings(new Settings()),
    m_currentSourceIndex(0)
{
  updateSourceStates();
}

Preferences::Preferences(const QString& settingFile)
  : m_settings(new Settings(settingFile)),
    m_currentSourceIndex(0)
{
  updateSourceStates();
}


Preferences::~Preferences(void)
{
  delete m_settings;
}

void Preferences::loadProperties(void)
{
  updateAllSourceWidgetStates();
  updateFields();
}


QString Preferences::sourceStatesSerialized(void)
{
  QString str = "";
  for (int i = 0; i < MAX_SRCS; i++) str += m_sourceStates.at(i)? "1" : "0";
  return str;
}



void Preferences::resetSourceStates(void)
{
  m_sourceStates.clear();
  m_sourceStates.resize(MAX_SRCS);
  for (int i=0; i < MAX_SRCS; ++i) {
    m_sourceStates.setBit(i, false);
  }
}


void Preferences::updateSourceStates(void)
{
  resetSourceStates();
  QString content = m_settings->value(Settings::GLOBAL_SRC_BUCKET_KEY).toString();
  if (! content.isEmpty()) {
    for (int i=0; i < MAX_SRCS; ++i) {
      m_sourceStates.setBit(i, content.at(i) == '1');
    }
  }
}


int Preferences::firstSourceSet()
{
  int idx = 0;
  while (idx < MAX_SRCS && ! m_sourceStates.at(idx)) {++idx;}
  return ((idx < MAX_SRCS)? idx : -1);
}


int Preferences::activeSourcesCount(void)
{
  updateSourceStates();
  return m_sourceStates.count(true);
}


QMap<QString, SourceT>
Preferences::fetchSourceList(int type)
{
  QMap<QString, SourceT> sourceList;
  SourceT srcInfo;

  updateSourceStates();
  for (int i = 0; i< MAX_SRCS; ++i) {
    if (loadSource(i, srcInfo)) {
      if (srcInfo.mon_type == type || type == MonitorT::Auto) {
        sourceList.insert(srcInfo.id, srcInfo);
      }
    }
  }

  return sourceList;
}
