/*
 * WebSettingUI.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 29-12-2013                                                 #
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
#include "WebPreferences.hpp"
#include <Wt/WTemplate>
#include <Wt/WContainerWidget>

WebPreferences::WebPreferences(int _userRole)
  : Preferences(_userRole, Preferences::WebForm),
    m_dialog(new Wt::WDialog())
{
  Wt::WContainerWidget* container = m_dialog->contents();
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("setting-page.tpl"), container);
}


WebPreferences::~WebPreferences()
{
  delete m_dialog;
}


QString WebPreferences::selectSourceType(void)
{

  return "";
}
