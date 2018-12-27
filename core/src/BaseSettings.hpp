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


#ifndef BASESETTINGS_HPP_
#define BASESETTINGS_HPP_

#include "SettingFactory.hpp"
#include "utilsCore.hpp"
#include "Base.hpp"


class BaseSettings: public QObject
{
  Q_OBJECT

public:
  enum FormTypeT {
    ChangePassword,
    ForceChangePassword,
    ChangeMonitoringSettings,
    ShowHelp,
    ShowAbout,
    BasicLoginForm,
    NoForm
  };

  BaseSettings(void);
  BaseSettings(const QString& settingFile);
  ~BaseSettings(void);
  int getGraphLayout(void) const;


Q_SIGNALS:
  void urlChanged(QString);
  void sourcesChanged (QList<qint8>);
  void errorOccurred(QString msg);
  void timerIntervalChanged(qint32);

public Q_SLOTS:
  qint32 updateInterval(void) const;

protected :
  virtual void fillFromSource(int sourceIndex) = 0;
  virtual void updateAllSourceWidgetStates(void) = 0;
  virtual void loadProperties(void);
  virtual void updateFields(void) = 0;
  virtual void saveAsSource(qint32 index) = 0;
  void sync(void);
  QString keyValue(const QString& key, const QString& defaultValue);
  void setKeyValue(const QString & _key, const QString & _value);

protected Q_SLOTS:
  virtual void applyChanges(void) = 0;
  virtual void handleCancel(void) = 0;
  virtual void addAsSource(void) = 0;
  virtual void deleteSource(void) = 0;

protected:
  SettingFactory* m_settingFactory;
};


#endif /* BASESETTINGS_HPP_ */
