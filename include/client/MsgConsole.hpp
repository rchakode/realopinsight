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
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);
    if (leftData.type() == QVariant::DateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
      }
    //return leftData.toString() < rightData.toString();
    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
  }
};

class MsgConsole : public QTableWidget
{
  Q_OBJECT

public:
  static const QString HOSTNAME_META_MSG_PATERN;
  static const QString SERVICE_META_MSG_PATERN;
  static const QString THERESHOLD_META_MSG_PATERN ;
  static const QString PLUGIN_OUTPUT_META_MSG_PATERN;

  MsgConsole(QWidget * parent = 0 );
  virtual ~MsgConsole() {}
  static const qint16 NUM_COLUMNS;
  void addMsg(const NodeListT::iterator &);
  void addMsg(const NodeT &);
  void resizeFields( const QSize& ,  const bool& = false );
  inline void sort(const int& column) {
    mmsgConsoleProxy->setSourceModel(model());
    mmsgConsoleProxy->sort(column, Qt::AscendingOrder);
  }

public slots:
  inline void acknowledgeMsg(void) { emit acknowledgeChanged();}
  inline void sortEventConsole(void) {sortItems(1, Qt::DescendingOrder); }

signals:
  void acknowledgeChanged(void);

private:
  QPoint charSize;
  QSize windowSize;
  MsgConsoleProxyModel* mmsgConsoleProxy;

  static const QStringList HeaderLabels;
  inline QCheckBox* msgItem(const qint32& _row, const qint32& _column){
    return dynamic_cast<QCheckBox*>(cellWidget(_row, _column) );
  }
};

#endif /* MSGCONSOLE_HPP */
