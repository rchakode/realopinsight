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



#include "WebBiDateFilter.hpp"
#include "WebUtils.hpp"


#define LAST_30_DAYS time(NULL) - 30 * 24 * 3600

WebBiDateFilter::WebBiDateFilter(void)
  : m_layout(new Wt::WHBoxLayout(this))
{
  m_promptTextLabel.setText("Start date");
  m_dateSeparatorLabel.setText(Q_TR("End date"));
  setupDatePicker(&m_startDatePicker, LAST_30_DAYS);
  setupDatePicker(&m_endDatePicker, time(NULL));
  setupSubmitButton();
  addEvent();
  bindFormWidgets();
}


WebBiDateFilter::~WebBiDateFilter()
{
  unbindFormWidgets();
}

void WebBiDateFilter::addEvent(void)
{
  m_applyFilterBtn.clicked().connect(this, &WebBiDateFilter::handleApplyButtonSubmitted);
}


void WebBiDateFilter::bindFormWidgets(void)
{
  m_layout->addWidget(&m_promptTextLabel,    1);
  m_layout->addWidget(&m_startDatePicker,    1);
  m_layout->addWidget(&m_dateSeparatorLabel, 1);
  m_layout->addWidget(&m_endDatePicker,      1);
  m_layout->addWidget(&m_applyFilterBtn,     1);
}


void WebBiDateFilter::unbindFormWidgets(void)
{
  m_layout->removeWidget(&m_promptTextLabel);
  m_layout->removeWidget(&m_startDatePicker);
  m_layout->removeWidget(&m_dateSeparatorLabel);
  m_layout->removeWidget(&m_endDatePicker);
  m_layout->removeWidget(&m_applyFilterBtn);
}


void WebBiDateFilter::setupDatePicker(Wt::WDatePicker* datePicker, long defaultEpochTime)
{
  if (datePicker) {
    Wt::WDateTime dt;
    dt.setTime_t(defaultEpochTime);
    datePicker->setFormat("dd-MM-yyyy");
    datePicker->setDate(dt.date());
    datePicker->setStyleClass("inline");
  }
}

void WebBiDateFilter::setupSubmitButton(void)
{
  m_applyFilterBtn.setLink(Wt::WLink("#"));
  m_applyFilterBtn.setText(Q_TR("Apply"));
}


