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

Preferences::Preferences(const QString& settingFile)
  : m_settings(new Settings(settingFile)),
    m_currentSourceIndex(0),
    m_sourceStates(new QBitArray(MAX_SRCS))
{
}
Preferences::Preferences(void)
  : m_settings(new Settings()),
    m_currentSourceIndex(0),
    m_sourceStates(new QBitArray(MAX_SRCS))
{
}


Preferences::~Preferences()
{
  delete m_sourceStates;
}


void Preferences::loadProperties(void)
{
  setSourceStatesFromData(m_settings->value(Settings::SRC_BUCKET_KEY).toString());
  updateAllSourceWidgetStates();
  updateFields();
}


QString Preferences::getSourceStatesSerialized(void)
{
  QString str = "";
  for (int i = 0; i < MAX_SRCS; i++) str += m_sourceStates->at(i)? "1" : "0";
  return str;
}

void Preferences::initSourceStatesFromData(void)
{
  setSourceStatesFromData(m_settings->value(Settings::SRC_BUCKET_KEY).toString());
}

void Preferences::setSourceStatesFromData(const QString& str)
{
  if (str.isEmpty()) {
    for (int i=0; i < MAX_SRCS; ++i) {
      m_sourceStates->setBit(i, false);
    }
  } else {
    for (int i=0; i < MAX_SRCS; ++i) {
      m_sourceStates->setBit(i, str.at(i) == '1');
    }
  }
}


int Preferences::firstSourceSet()
{
  int idx = 0;
  while (idx < MAX_SRCS && ! m_sourceStates->at(idx)) {++idx;}

  return ((idx < MAX_SRCS)? idx : -1);
}
