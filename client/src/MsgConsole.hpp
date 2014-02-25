/*
 * MsgPanel.hpp
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

#ifndef MSGCONSOLE_HPP
#define MSGCONSOLE_HPP
#include "Base.hpp"

class MsgConsoleProxyModel: public QSortFilterProxyModel
{
  Q_OBJECT
public:
  MsgConsoleProxyModel(QObject *parent = 0): QSortFilterProxyModel(parent){}

protected:
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const
  {
    QVariant leftData = sourceModel()->data(left, Qt::UserRole);
    QVariant rightData = sourceModel()->data(right, Qt::UserRole);
    if (leftData.type() == QVariant::DateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
      } else if (leftData.type() == QVariant::Int) {
        return leftData.toInt() < rightData.toInt();
      }
    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
  }
};

class MsgConsole : public QTableView  //TODO see style for QtableView
{
  Q_OBJECT

public:
  static const qint16 NUM_COLUMNS;
  static const qint16 ROW_MARGIN;

  MsgConsole(QWidget * parent = 0 );
  virtual ~MsgConsole();
  void updateNodeMsg(const NodeListT::iterator &);
  void updateNodeMsg(const NodeT &);
  void updateEntriesSize(bool _resizeWindow);
  void clearMsg(const NodeT &);
  void clearNormalMsg(void);
  qint32 getRowCount() const {return model()->rowCount();}
  void useLargeFont(bool _toggled);
  void setConsoleSize(const QSize& size) {m_consoleSize = size;}
  QSize getConsoleSize(void) const {return m_consoleSize;}

public Q_SLOTS:
  void sortEventConsole(void) {mproxyModel->sort(1);}

Q_SIGNALS:
  void acknowledgeChanged(void);

private:
  QStandardItemModel* mmodel;
  MsgConsoleProxyModel* mproxyModel;
  QSize m_consoleSize;
};

#endif /* MSGCONSOLE_HPP */
