/*
 * WebImportHostGroupMap.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 25-06-2015                                                     #
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

#ifndef WEBMANAGEHOSTGROUPMAPS_HPP
#define WEBMANAGEHOSTGROUPMAPS_HPP

#include "WebPreferencesBase.hpp"
#include <Wt/WTemplate>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WComboBox>

class WebHostGroupServiceMap : public WebPreferencesBase, public Wt::WTemplate
{
public:
  WebHostGroupServiceMap();
  ~WebHostGroupServiceMap();
  void updateContents(void) {}
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }


private:
  /** Signal **/
  Wt::Signal<int, std::string> m_operationCompleted;

  /** other members **/
  Wt::WComboBox m_sourceListBox;
  Wt::WLineEdit m_hostGroupFilterField;
  Wt::WPushButton m_submitButton;

  void addEvent(void);
  void updateFormWidgets(void);
  void bindFormWidgets(void);
  void unbindFormWidgets(void);
  void handleImportationSubmitted(void);
};

#endif // WEBMANAGEHOSTGROUPMAPS_HPP
