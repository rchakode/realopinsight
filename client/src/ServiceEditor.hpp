/*
 * SNAVServiceEditor.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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


#ifndef SNAVSERVICEEDITOR_H_
#define SNAVSERVICEEDITOR_H_

#include "Base.hpp"
#include "Parser.hpp"
#include "Settings.hpp"

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QtWidgets>
#else
#   include <QtGui>
#endif


const qint32 MAX_NODE_NAME = 255;
const QString NAME_FIELD = "name";
const QString TYPE_FIELD = "type";
const QString STATUS_CALC_RULE_FIELD = "StatusCalcRules";
const QString STATUS_PROP_RULE_FIELD = "StatusPropRules";
const QString ICON_FIELD = "icon";
const QString DESCRIPTION_FIELD = "description";
const QString ALARM_MSG_FIELD = "alarmMsg";
const QString NOTIFICATION_MSG_FIELD = "notificationMsg";
const QString CHECK_LIST_FIELD = "ChecksField";
const QString CHECK_FIELD = "CheckField";
const QString HIGH_CRITICITY_CALC_RULE_FIELD = "Higher Severity";
const QString WEIGHTED_CALC_RULE_FIELD = "Equal-weighted Severity";

class ServiceEditor : public QWidget
{
  Q_OBJECT

public:

  typedef QMap<QString, QWidget*> WidgetMapT;

  ServiceEditor(QWidget* = 0);
  virtual ~ServiceEditor();

  void layoutEditorComponents(void);
  void fillFormWithNodeContent(const NodeListT& nodes, const QString& nodeId);
  void fillFormWithNodeContent(const NodeT& _node);
  void fillFormWithNodeContent(NodeListT::const_iterator nodeIt) {fillFormWithNodeContent(*nodeIt);}
  bool updateNodeContent(NodeListT& _nodes, const QString& _nodeId);
  bool updateNodeContent(NodeListT::iterator& _node);
  void loadChecks(const ChecksT& checks);
  void setEnableFields(const bool& enable);
  void setLowLevelAlarmComponentEnabled(bool enable){ mitems[CHECK_LIST_FIELD]->setEnabled(enable); }

  inline WidgetMapT* itemList(void) {return& mitems;}
  inline QLineEdit* nameField(void){return dynamic_cast<QLineEdit*>(mitems[NAME_FIELD]);}
  inline QComboBox* typeField(void) const {return dynamic_cast<QComboBox*>(mitems[TYPE_FIELD]);}
  inline QComboBox* statusCalcRuleField(void) const {return dynamic_cast<QComboBox*>(mitems[STATUS_CALC_RULE_FIELD]);}
  inline QComboBox* statusPropRuleField(void) const {return dynamic_cast<QComboBox*>(mitems[STATUS_PROP_RULE_FIELD]);}
  inline QComboBox* iconField(void) const {return dynamic_cast<QComboBox*>(mitems[ICON_FIELD]);}
  inline QTextEdit* descriptionField(void) const {return dynamic_cast<QTextEdit*>(mitems[DESCRIPTION_FIELD]);}
  inline QTextEdit* alarmMsgField(void){return dynamic_cast<QTextEdit*>(mitems[ALARM_MSG_FIELD]);}
  inline QTextEdit* notificationMsgField(void){return dynamic_cast<QTextEdit*>(mitems[NOTIFICATION_MSG_FIELD]);}
  inline QComboBox* checkField(void){return dynamic_cast<QComboBox*>(mitems[CHECK_FIELD]);}
  inline QListWidget* checkListField(void){return dynamic_cast<QListWidget*>(mitems[CHECK_LIST_FIELD]);}

public Q_SLOTS:
  inline void handleSaveClick(void) { Q_EMIT saveClicked(); }
  void handleCloseClick(void){ Q_EMIT closeClicked(); }
  inline void handleReturnPressed(void) { Q_EMIT returnPressed(); }
  void handleNodeTypeChanged(const QString&);
  void handleNodeTypeActivated(const QString& _text);

Q_SIGNALS:
  void saveClicked(void);
  void closeClicked(void);
  void returnPressed(void);
  void nodeTypeActivated(qint32);

private:
  Settings* msettings;
  qint32 editorLayoutRowCount;
  qint32 mlayoutColumnCount;
  qint16 mlayoutRowIndex;
  WidgetMapT mitems;
  QGridLayout* mlayout;
  QDialogButtonBox* buttonBox;

  void addEvent(void);
  void loadLabelFields(void);
  void loadDescriptionFields(void);
  void loadTypeFields(void);
  void loadStatusHandlingFields(void);
  void loadAlarmMsgFields(void);
  void loadNotificationMsgFields(void);
  void loadIconFields(void);
  void loadCheckField(void);
  void loadButtonBox(void);
};

#endif /* SNAVSERVICEEDITOR_H_ */
