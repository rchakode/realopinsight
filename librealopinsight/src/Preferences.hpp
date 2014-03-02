/*
 * Preferences.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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


class LIBREALOPINSIGHTSHARED_EXPORT Preferences: public QObject
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
    WebForm
  };

  Preferences(void);
  virtual ~Preferences();
  QBitArray* getSourceStates() const { return m_sourceStates; }
  bool isSetSource(int idx) {return (idx < MAX_SRCS && m_sourceStates)? m_sourceStates->at(idx) : false; }

Q_SIGNALS:
  void urlChanged(QString);
  void sourcesChanged (QList<qint8>);
  void errorOccurred(QString msg);

protected :
  virtual void fillFromSource(int _sidx) = 0;
  virtual void updateSourceBtnState(void) = 0;
  virtual void loadProperties(void);
  virtual void updateFields(void) = 0;
  virtual void saveAsSource(const qint32& idx, const QString& type) = 0;
  virtual int firstSourceSet(void);
  virtual void initSourceStates();
  QString getSourceStatesSerialized(void);

protected Q_SLOTS:
  virtual void applyChanges(void) = 0;
  virtual void handleCancel(void) = 0;
  virtual void addAsSource(void) = 0;
  virtual void deleteSource(void) = 0;
  qint32 updateInterval(void) const {return m_settings->updateInterval();}

protected:
  Settings* m_settings;
  int m_currentSourceIndex;
  QBitArray* m_sourceStates;

  void initSourceStates(const QString& str);
};


#endif /* SNAVPREFERENCES_HPP_ */
