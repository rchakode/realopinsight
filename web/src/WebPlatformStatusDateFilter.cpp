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



#include "WebPlatformStatusDateFilter.hpp"
#include "WebUtils.hpp"
#include <Wt/WTemplate.h>


#define LAST_30_DAYS time(NULL) - 30 * 24 * 3600

WebPlatformStatusDateFilter::WebPlatformStatusDateFilter(void)
  : m_startDatePickerRef(nullptr),
    m_endDatePickerRef(nullptr)
{

  auto startDatePicker= std::make_unique<Wt::WDatePicker>();
  m_startDatePickerRef = startDatePicker.get();
  m_startDatePickerRef->setFormat("dd-MM-yyyy");
  Wt::WDateTime defaultStartDate;
  defaultStartDate.setTime_t(LAST_30_DAYS);
  m_startDatePickerRef->setDate(defaultStartDate.date());


  auto endDatePicker= std::make_unique<Wt::WDatePicker>();
  m_endDatePickerRef = endDatePicker.get();
  Wt::WDateTime defaultEndDate;
  defaultEndDate.setTime_t(time(NULL)); // epoch
  m_endDatePickerRef->setDate(defaultEndDate.date());

  auto applyBtn = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), Q_TR("Apply"));
  applyBtn->clicked().connect(this, std::bind([=]{m_reportPeriodChanged.emit(this->epochStartTime(), this->epochEndTime());}));

  auto widget = std::make_unique<Wt::WTemplate>(
        "<div class=\"row\">"
        "  <div class=\"col-sm-1\">${start-date-label}</div>"
        "  <div class=\"col-sm-1\">${start-date-field}</div>"
        "  <div class=\"col-sm-1\">${end-date-label}</div>"
        "  <div class=\"col-sm-1\">${end-date-field}</div>"
        "  <div class=\"col-sm-1\">${apply-btn}</div>"
        "</div>");
  widget->bindString("start-date-label", Q_TR("Start date"));
  widget->bindWidget("start-date-field", std::move(startDatePicker));
  widget->bindWidget("end-date-field", std::move(endDatePicker));
  widget->bindString("end-date-label", Q_TR("End date"));
  widget->bindWidget("apply-btn", std::move(applyBtn));

  addWidget(std::move(widget));
}


WebPlatformStatusDateFilter::~WebPlatformStatusDateFilter()
{
}



