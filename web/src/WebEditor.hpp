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
#include <QMap>


class WebEditor : public Wt::WContainerWidget
{
public:
  WebEditor(void);
  virtual ~WebEditor();
  void reload(void);
  Wt::Signal<int, std::string>& operationCompleted(void) {return m_operationCompleted;}

private:
  enum {
    MENU_ADD_SUBSERVICE = 1,
    MENU_DELETE_SUBSERVICE
  };
  const static QMap<int, std::string> MENU_TEXTS;

  Wt::Signal<int, std::string> m_operationCompleted;

  CoreDataT m_cdata;
  WebTree m_tree;

  Wt::WModelIndex m_currentTreeItemIndex;
  QString m_formerSelectedNodeId;

  Wt::WHBoxLayout* m_mainLayout;
  Wt::WTemplate m_fieldEditionPane;
  Wt::WLineEdit m_nameField;
  Wt::WComboBox m_typeField;
  Wt::WComboBox m_iconBox;
  Wt::WComboBox m_calcRuleBox;
  Wt::WComboBox m_propRuleBox;
  Wt::WTextArea m_descField;
  Wt::WLineEdit m_dataPointField;
  Wt::WImage m_newServiceViewBtn;
  Wt::WImage m_openServiceViewBtn;
  Wt::WImage m_renameCurrentViewBtn;
  Wt::WImage m_saveCurrentViewBtn;


  Wt::WPopupMenu m_editionContextMenu;
  Wt::WMenuItem* m_menuAddSubService;
  Wt::WMenuItem* m_menuDeleteService;

  QMap<QString, int> m_iconIndexMap;


  void openServiceView(void);
  void bindMainPanes(void);
  void unbindWidgets(void);
  void addEvents(void);
  void buildTree(void);
  void bindFormWidgets(void);
  void newView(void);


  void activateTreeEditionFeatures(void);
  void showTreeContextMenu(Wt::WModelIndex, Wt::WMouseEvent);
  void addNewSubService(const Wt::WModelIndex& currentTreeItemIndex);
  void fillInEditorFromCurrentSelection(void);
  void updateNodeDataFromEditor(const QString& nodeId);



  void handleTreeContextMenu(Wt::WMenuItem*);
  void handleKeyPressed(Wt::WKeyEvent event);
  void handleTreeItemSelectionChanged(void);
  void handleNodeLabelChanged(void);
};


#endif /* WEBEDITOR_HPP */
