/*
 * Preferences.hpp
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


#ifndef SNAVPREFERENCES_HPP_
#define SNAVPREFERENCES_HPP_

#include "global.hpp"
#include "Settings.hpp"
#include "Base.hpp"


class Preferences: public QObject
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

  Preferences(const QString& settingFile);
  Preferences(void);
  virtual ~Preferences();
  QBitArray* getSourceStates() const { return m_sourceStates; }
  bool isSetSource(int idx) {return (idx < MAX_SRCS && m_sourceStates)? m_sourceStates->at(idx) : false; }
  void setSourceState(int index, int value) {m_sourceStates->setBit(index, value);}
  virtual void initSourceStatesFromData();

Q_SIGNALS:
  void urlChanged(QString);
  void sourcesChanged (QList<qint8>);
  void errorOccurred(QString msg);

public Q_SLOTS:
  qint32 updateInterval(void) const {return m_settings->updateInterval();}
  bool loadSource(qint32 _id, SourceT& _src) {return m_settings->loadSource(_id, _src);}
  bool loadSource(const QString& _id, SourceT& _src) {return m_settings->loadSource(_id, _src);}

protected :
  virtual void fillFromSource(int _index) = 0;
  virtual void updateAllSourceWidgetStates(void) = 0;
  virtual void loadProperties(void);
  virtual void updateFields(void) = 0;
  virtual void saveAsSource(const qint32& idx, const QString& type) = 0;
  virtual int firstSourceSet(void);
  void setSourceStatesFromData(const QString& str);
  QString getSourceStatesSerialized(void);
  int currentSourceIndex(void) const {return m_currentSourceIndex;}
  void setCurrentSourceIndex(int value) {m_currentSourceIndex = value;}
  void sync(void) {m_settings->sync();}
  void setEntry(const QString& key, const QString& value) {m_settings->setEntry(key, value);}
  QString value(const QString& key, const QString& defaultValue) {return m_settings->value(key, defaultValue).toString(); }
  void setKeyValue(const QString & _key, const QString & _value) {m_settings->setKeyValue(_key, _value);}
  bool getSourceState(int index) {return m_sourceStates->at(index);}

protected Q_SLOTS:
  virtual void applyChanges(void) = 0;
  virtual void handleCancel(void) = 0;
  virtual void addAsSource(void) = 0;
  virtual void deleteSource(void) = 0;
  void emitTimerIntervalChanged(qint32 _interval) {m_settings->emitTimerIntervalChanged(_interval);}

protected:
  Settings* m_settings;
  int m_currentSourceIndex;
  QBitArray* m_sourceStates;

};


#endif /* SNAVPREFERENCES_HPP_ */
