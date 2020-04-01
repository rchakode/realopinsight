/*
 * WebMsgConsole.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 19-09-2014                                                 #
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

#ifndef WEBEVENTCONSOLE_HPP
#define WEBEVENTCONSOLE_HPP
#include "Base.hpp"
#include <boost/any.hpp>
#include <Wt/WTableView.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WSortFilterProxyModel.h>

class SortingProxyModel : public Wt::WSortFilterProxyModel {
public:
  SortingProxyModel()
    : Wt::WSortFilterProxyModel() { }
  virtual ~SortingProxyModel() {}

protected:
  virtual bool lessThan( const Wt::WModelIndex&lhs, const Wt::WModelIndex& rhs) const {
    if (lhs.data(Wt::ItemDataRole::User).empty() || rhs.data(Wt::ItemDataRole::User).empty()) {
      return lhs < rhs;
    }
    auto v1 = atol(Wt::cpp17::any_cast<std::string>(lhs.data(Wt::ItemDataRole::User)).c_str());
    auto v2 = atol(Wt::cpp17::any_cast<std::string>(rhs.data(Wt::ItemDataRole::User)).c_str());
    return v1 < v2;
  }
};

class WebMsgConsole : public Wt::WTableView
{
public:
  WebMsgConsole();
  virtual ~WebMsgConsole();
  void clearAll(void);


  Wt::WStandardItemModel* getRenderingModel(void) const {return m_modelRef;}
  void updateNodeMsg(const NodeT& _node);
  std::unique_ptr<Wt::WStandardItem> createItem(const Wt::WString& text, int row);
  std::unique_ptr<Wt::WStandardItem> createDateItem(const std::string& _lastcheck, int row);


protected:
  void layoutSizeChanged (int width, int height);
  void setModelHeaders();

private:
  Wt::WStandardItemModel* m_modelRef;

  int findServiceRow(const std::string& _id);
};

#endif /* WEBEVENTCONSOLE_HPP */
