/*
 * WebBiDateFilter.cpp
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



#include "WebQoSDateFilter.hpp"
#include "WebUtils.hpp"


#define LAST_30_DAYS time(NULL) - 30 * 24 * 3600

WebQoSDateFilter::WebQoSDateFilter(void)
  : m_startDatePickerRef(nullptr),
    m_endDatePickerRef(nullptr)
{
  auto layout = std::make_unique<Wt::WHBoxLayout>();

  layout->addWidget(std::make_unique<Wt::WLabel>("Start date"), 1);
  auto startDatePicker= std::make_unique<Wt::WDatePicker>();
  m_startDatePickerRef = startDatePicker.get();
  setupDatePicker(m_startDatePickerRef, LAST_30_DAYS);
  layout->addWidget(std::move(startDatePicker), 1);

  layout->addWidget(std::make_unique<Wt::WLabel>(Q_TR("End date")), 1);
  auto endDatePicker= std::make_unique<Wt::WDatePicker>();
  m_endDatePickerRef = endDatePicker.get();
  setupDatePicker(m_endDatePickerRef, time(NULL));
  layout->addWidget(std::move(endDatePicker), 1);

  auto applyBtn = std::make_unique<Wt::WAnchor>(Q_TR("Apply"));
  applyBtn->setLink(Wt::WLink("#"));
  applyBtn->clicked().connect(this, std::bind([=]{m_reportPeriodChanged.emit(this->epochStartTime(), this->epochEndTime());}));
  layout->addWidget(std::move(applyBtn), 1);

  setLayout(std::move(layout));
}


WebQoSDateFilter::~WebQoSDateFilter()
{
}


void WebQoSDateFilter::setupDatePicker(Wt::WDatePicker* datePicker, long defaultEpochTime)
{
  if (datePicker) {
    Wt::WDateTime dt;
    dt.setTime_t(defaultEpochTime);
    datePicker->setFormat("dd-MM-yyyy");
    datePicker->setDate(dt.date());
    datePicker->setStyleClass("inline");
  }
}



