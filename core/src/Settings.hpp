/*
 * Settings.hpp
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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#include <QString>
#include <QSettings>
#include "Base.hpp"
#include <ctime>


class Settings : public QSettings
{
  Q_OBJECT
public:
  Settings();
  Settings(const QString& path);
  virtual ~Settings(void);

  void init(void);
  void setKeyValue(const QString & _key, const QString & _value);
  qint32 updateInterval() const;
  void setEntry(const QString& key, const QString& value);
  QString setEntry(const QString& key) const {return QSettings::value(key).toString();}
  bool loadSource(qint32 _idx, SourceT& _src);
  bool loadSource(const QString& _id, SourceT& _src);
  bool setSource(const QString& _info, SourceT& _src);
  void emitTimerIntervalChanged(qint32 _interval) {Q_EMIT timerIntervalChanged(_interval);}

  static const QString UPDATE_INTERVAL_KEY;
  static const QString ADM_UNSERNAME_KEY;
  static const QString OP_UNSERNAME_KEY;
  static const QString ADM_PASSWD_KEY;
  static const QString OP_PASSWD_KEY;
  static const QString SRC_BUCKET_KEY;

Q_SIGNALS:
  void timerIntervalChanged(qint32);
};


#endif // SETTINGS_HPP
