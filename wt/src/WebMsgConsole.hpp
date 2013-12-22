/*
 * WebMsgConsole.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 19-09-2013                                                 #
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

#ifndef NGRT4NMSGPANEL_HPP_
#define NGRT4NMSGPANEL_HPP_
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WSortFilterProxyModel>
#include <boost/any.hpp>
#include "Base.hpp"

class SortingProxyModel : public Wt::WSortFilterProxyModel {
public:
  SortingProxyModel(Wt::WObject* _parent=0)
    : Wt::WSortFilterProxyModel(_parent) { }
  virtual ~SortingProxyModel() {}

protected:
  virtual bool lessThan( const Wt::WModelIndex&lhs, const Wt::WModelIndex& rhs) const {
    if(lhs.data(Wt::UserRole).empty() || rhs.data(Wt::UserRole).empty())
      return lhs < rhs;
    return atol(boost::any_cast<std::string>(lhs.data(Wt::UserRole)).c_str())< atol(boost::any_cast<std::string>(rhs.data(Wt::UserRole)).c_str());
  }
};

class WebMsgConsole : public Wt::WTableView
{
  //  Q_OBJECT

public:
  WebMsgConsole();
  virtual
  ~WebMsgConsole();

  Wt::WStandardItemModel* getRenderingModel(void) const {return m_model;}
  void addMsg(const NodeT&  _node);
  void updateNodeMsgs(const NodeListT& _cnodes);
  void updateNodeMsg(const NodeT& _node);
  Wt::WStandardItem* createStatusItem(const NodeT& _node);
  Wt::WStandardItem* createDateTimeItem(const std::string& _lastcheck);
  Wt::WStandardItem* createDateTimeItem(time_t _time);

  //signals:
  //  void sizeChanged(int, int);

protected:
  void layoutSizeChanged (int width, int height);

private:
  Wt::WStandardItemModel* m_model;
  int m_rowCount;

  int findServiceRow(const std::string& _id);
  void setSeverityItem(Wt::WStandardItem* item, int severity);
  void setModel(void);
};

#endif /* NGRT4NMSGPANEL_HPP_ */
