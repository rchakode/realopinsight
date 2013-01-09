/*
 * MsgPanel.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
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
  static const QString TAG_HOSTNAME;
  static const QString TAG_HOSTNAME_ZABBIX;
  static const QString TAG_CHECK;
  static const QString TAG_THERESHOLD ;
  static const QString TAG_PLUGIN_OUTPUT;

  MsgConsole(QWidget * parent = 0 );
  virtual ~MsgConsole();
  static const qint16 NUM_COLUMNS;
  void updateNodeMsg(const NodeListT::iterator &);
  void updateNodeMsg(const NodeT &);
  void updateColumnWidths( const QSize& ,  const bool& = false );

public slots:
  inline void acknowledgeMsg(void) { emit acknowledgeChanged(); }
  inline void sortEventConsole(void) { mproxyModel->sort(1); }

signals:
  void acknowledgeChanged(void);

private:
  QStandardItemModel* mmodel;
  MsgConsoleProxyModel* mproxyModel;
  QPoint charSize;
  QSize windowSize;
};

#endif /* MSGCONSOLE_HPP */
