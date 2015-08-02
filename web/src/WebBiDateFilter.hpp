/*
 * WebBiDateFilter.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 26-07-2015                                                     #
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



#ifndef WEBBIDATEFILTER_HPP
#define WEBBIDATEFILTER_HPP

#include <Wt/WLabel>
#include <Wt/WDatePicker>
#include <Wt/WHBoxLayout>
#include <Wt/WAnchor>
#include <QObject>
#include <QDebug>

class WebBiDateFilter : public QObject, public  Wt::WContainerWidget
{
  Q_OBJECT

public:
  WebBiDateFilter(void);
  ~WebBiDateFilter();
  long epochStartTime(void){ return Wt::WDateTime(m_startDatePicker.date()).toTime_t();}
  long epochEndTime(void) {return Wt::WDateTime(m_endDatePicker.date()).toTime_t();}

public Q_SLOTS:
  void handleApplyButtonSubmitted(void) {Q_EMIT reportPeriodChanged(epochStartTime(), epochEndTime()); }

Q_SIGNALS:
  void reportPeriodChanged(long start, long end);

private:
  Wt::WHBoxLayout* m_layout;
  Wt::WLabel m_promptTextLabel;
  Wt::WLabel m_dateSeparatorLabel;
  Wt::WDatePicker m_startDatePicker;
  Wt::WDatePicker m_endDatePicker;
  Wt::WAnchor m_applyFilterBtn;


  void addEvent(void);
  void bindFormWidgets(void);
  void unbindFormWidgets(void);
  void setupDatePicker(Wt::WDatePicker* datePicker, long defaultEpochTime);
  void setupSubmitButton(void);
};


#endif // WEBBIDATEFILTER_HPP
