/*
 * WebEditor.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)        #
# Last Update : 03-01-2018                                                 #
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

#ifndef WEBEDITOR_HPP
#define WEBEDITOR_HPP

#include "DashboardBase.hpp"
#include "WebTree.hpp"
#include "WebUtils.hpp"
#include <Wt/WGridLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WMenu>
#include <Wt/WMenuItem>
#include <Wt/WStackedWidget>
#include "WebMap.hpp"
#include <Wt/WLocalizedStrings>
#include <Wt/WLabel>
#include <QHash>
#include <Wt/WApplication>
#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WTextArea>
#include <Wt/WComboBox>




class WebEditor : public Wt::WContainerWidget
{
public:
  WebEditor(void);
  virtual ~WebEditor();

private:
  WebTree m_tree;
  Wt::WHBoxLayout* m_mainLayout;
  Wt::WTemplate m_fieldEditionPane;

  Wt::WLineEdit m_nameField;
  Wt::WComboBox m_typeField;
  Wt::WComboBox m_iconBox;
  Wt::WComboBox m_calcRuleBox;
  Wt::WComboBox m_propRuleBox;
  Wt::WTextArea m_descField;
  Wt::WComboBox m_checkItemField;
  Wt::WPushButton m_saveBtn;


  void openViewFile(const QString& path);
  void bindFormWidgets(void);
  void unbindWidgets(void);
  void addEvents(void);
  void buildTree(void);
  void bindEditionForm(void);
};


#endif /* WEBEDITOR_HPP */
