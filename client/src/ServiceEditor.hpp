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
const QString CHECK_FIELD = "CheckField";
const QString CHECK_FILTER_FIELD = "Search...";
const QString CHECK_HOST_GROUP_FIELD = "Select Host Group";
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
  bool updateNodeInfo(NodeT& _node);
  void updateDataPoints(const ChecksT& checks, const QString& srcId);
  void setEnableFields(const bool& enable);

  inline WidgetMapT* itemList(void) {return& m_fieldWidgets;}
  inline QLineEdit* nameField(void){return dynamic_cast<QLineEdit*>(m_fieldWidgets[NAME_FIELD]);}
  inline QComboBox* typeField(void) const {return dynamic_cast<QComboBox*>(m_fieldWidgets[TYPE_FIELD]);}
  inline QComboBox* statusCalcRuleField(void) const {return dynamic_cast<QComboBox*>(m_fieldWidgets[STATUS_CALC_RULE_FIELD]);}
  inline QComboBox* statusPropRuleField(void) const {return dynamic_cast<QComboBox*>(m_fieldWidgets[STATUS_PROP_RULE_FIELD]);}
  inline QComboBox* iconField(void) const {return dynamic_cast<QComboBox*>(m_fieldWidgets[ICON_FIELD]);}
  inline QTextEdit* descriptionField(void) const {return dynamic_cast<QTextEdit*>(m_fieldWidgets[DESCRIPTION_FIELD]);}
  inline QTextEdit* alarmMsgField(void){return dynamic_cast<QTextEdit*>(m_fieldWidgets[ALARM_MSG_FIELD]);}
  inline QTextEdit* notificationMsgField(void){return dynamic_cast<QTextEdit*>(m_fieldWidgets[NOTIFICATION_MSG_FIELD]);}
  inline QListWidget* checkField(void){return dynamic_cast<QListWidget*>(m_fieldWidgets[CHECK_FIELD]);}

public Q_SLOTS:
  inline void handleSaveClick(void) { Q_EMIT saveClicked(); }
  void handleCloseClick(void){ Q_EMIT closeClicked(); }
  inline void handleReturnPressed(void) { Q_EMIT returnPressed(); }
  void handleNodeTypeChanged(const QString&);
  void handleNodeTypeActivated(const QString& text);
  void handleDataPointFilter(const QString& text);
  void handleDataPointSearch(void) { handleDataPointFilter(m_dataPointSearchField->text()); }
  void handleAddDataPointEntry(void) { addAndSelectDataPointEntry(m_dataPointSearchField->text());}

Q_SIGNALS:
  void saveClicked(void);
  void closeClicked(void);
  void returnPressed(void);
  void nodeTypeActivated(qint32);


private:
  qint32 m_rows;
  qint16 m_currentRow;
  WidgetMapT m_fieldWidgets;
  QGridLayout* m_mainLayout;
  QDialogButtonBox* m_actionButtonBox;
  QStringList m_dataPoints;
  QLineEdit* m_dataPointSearchField;
  QComboBox* m_hostGroupFilterBox;
  QGroupBox* m_checkFieldsGroup;
  QStackedWidget* m_dataPointActionButtons;
  QPushButton* m_searchDataPointButton;
  QPushButton* m_addDataPointButton;

  void addEvent(void);
  void layoutLabelFields(void);
  void layoutDescriptionFields(void);
  void layoutTypeFields(void);
  void layoutStatusHandlingFields(void);
  void layoutAlarmMsgFields(void);
  void layoutNotificationMsgFields(void);
  void layoutIconFields(void);
  void layoutCheckField(void);
  void layoutButtonBox(void);
  QLabel* createCheckFieldHelpIcon(void);
  void setCheckFieldsStyle(void);
  void addAndSelectDataPointEntry(const QString& text);
};

#endif /* SNAVSERVICEEDITOR_H_ */
