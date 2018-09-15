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
#include "dbo/src/DbObjects.hpp"
#include "dbo/src/DbSession.hpp"
#include "ChartBase.hpp"

class QosCollector : public DashboardBase
{
public:
  QosCollector(void);
  QosDataT qosInfo(void) const {return m_qosInfo;}

protected:
  virtual void updateChart(void);
  virtual void buildMap(void) {}
  virtual void updateMap(const NodeT&, const QString&) {}
  virtual void buildTree(void) {}
  virtual void updateTree(const NodeT&, const QString&){}
  virtual void updateMsgConsole(const NodeT&) {}
  virtual void updateEventFeeds(const NodeT&) {}

private:
  ChartBase m_chartBase;
  QosDataT m_qosInfo;
};

#endif // REPORTCOLLECTOR_HPP
