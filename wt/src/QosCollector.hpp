/*
 * ReportCollector.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 06-10-2014                                                  #
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

#ifndef REPORTCOLLECTOR_HPP
#define REPORTCOLLECTOR_HPP
#include "DashboardBase.hpp"
#include "DbObjects.hpp"
#include "ChartBase.hpp"
#include "DbSession.hpp"

class QosCollector : public DashboardBase
{
public:
  QosCollector(const QString& descriptionFile, DbSession* dbSession);

protected:
  virtual void updateChart(void);
  virtual void buildMap(void) {}
  virtual void updateMap(const NodeT& _node, const QString& _tip) {}
  virtual void buildTree(void) {}
  virtual void updateTree(const NodeT& _node, const QString& _tip){}
  virtual void updateMsgConsole(const NodeT& _node) {}
  virtual void updateEventFeeds(const NodeT& node) {}

private:
  ChartBase m_chartBase;
  DbSession* m_dbSession;
};

#endif // REPORTCOLLECTOR_HPP
