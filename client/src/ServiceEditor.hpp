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
#include <QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QtWidgets>
#else
#   include <QtGui>
#endif


const qint32 MAX_NODE_NAME = 255;
const QString NAME_FIELD = "NAME_FIELD_ID";
const QString TYPE_FIELD = "TYPE_FIELD_ID";
const QString EXTERNAL_SERVICE_NAME = "EXTERNAL_SERVICE_FIELD_ID";
const QString ICON_FIELD = "ICON_FIELD_ID";
const QString DESCRIPTION_FIELD = "DESCRIPTION_FIELD_ID";
const QString ALARM_MSG_FIELD = "ALARM_MSG_ID";
const QString NOTIFICATION_MSG_FIELD = "NOTIFICATION_FIELD_ID";
const QString CHECK_FIELD = "CHECK_FIELD_ID";

class ServiceEditor : public QWidget
{
  Q_OBJECT

public:

  typedef QMap<QString, QWidget*> WidgetMapT;

  ServiceEditor(QWidget* = 0);
  virtual ~ServiceEditor();

  void layoutEditorComponents(void);
  void fillInEditorFields(const NodeListT& nodes, const QString& nodeId);
  void fillInEditorFields(const NodeT& _node);
  bool setNodeFromEditor(NodeT& _node);
  void updateDataPoints(const ChecksT& checks, const QString& srcId);

  WidgetMapT* itemList(void) {return& m_fieldWidgets;}
  QLineEdit* nameField(void){return dynamic_cast<QLineEdit*>(m_fieldWidgets[NAME_FIELD]);}
  QLineEdit* externalServiceField(void){return dynamic_cast<QLineEdit*>(m_fieldWidgets[EXTERNAL_SERVICE_NAME]);}
  QComboBox* typeField(void) const {return dynamic_cast<QComboBox*>(m_fieldWidgets[TYPE_FIELD]);}
  QComboBox* iconField(void) const {return dynamic_cast<QComboBox*>(m_fieldWidgets[ICON_FIELD]);}
  QTextEdit* descriptionField(void) const {return dynamic_cast<QTextEdit*>(m_fieldWidgets[DESCRIPTION_FIELD]);}
  QTextEdit* alarmMsgField(void){return dynamic_cast<QTextEdit*>(m_fieldWidgets[ALARM_MSG_FIELD]);}
  QTextEdit* notificationMsgField(void){return dynamic_cast<QTextEdit*>(m_fieldWidgets[NOTIFICATION_MSG_FIELD]);}
  QListWidget* checkField(void){return dynamic_cast<QListWidget*>(m_fieldWidgets[CHECK_FIELD]);}

public Q_SLOTS:
  void handleSaveClick(void) { Q_EMIT saveClicked(); }
  void handleCloseClick(void){ Q_EMIT closeClicked(); }
  void handleReturnPressed(void) { Q_EMIT returnPressed(); }
  void handleNodeTypeChanged(const QString& typeText);
  void handleNodeTypeActivated(const QString& typeText);
  void handleDataPointFilter(const QString& text);
  void handleDataPointSearch(void) { handleDataPointFilter(m_dataPointSearchField->text()); }
  void handleAddDataPointEntry(void) { addAndSelectDataPointEntry(m_dataPointSearchField->text());}
  void handleDataPointFieldReturnPressed(void);
  void handleUpdateDataPointsList(void);
  void handleAddThreshold(void);
  void handleRemoveThreshold(void);
  void handleThresholdRulesChanged(void);
  void handleCalcRuleChanged(void);

Q_SIGNALS:
  void saveClicked(void);
  void closeClicked(void);
  void returnPressed(void);
  void nodeTypeActivated(qint32);
  void errorOccurred(QString);


private:
  enum WeightType {
    WeightNormalized = 0,
    WeightThreshold = 1
  };

  class WeightBox : public QDoubleSpinBox {
  public:
    WeightBox(qint8 weightType, QWidget* parent = 0) : QDoubleSpinBox(parent) {

      switch (weightType) {
      case WeightThreshold:
        setDecimals(0);
        setRange(0, 100);
        setValue(100);
        setSingleStep(5);
        setSuffix(tr(" %"));
        break;
      case WeightNormalized:
      default:
        setDecimals(1);
        setRange(ngrt4n::WEIGHT_MIN, ngrt4n::WEIGHT_MAX);
        setValue(ngrt4n::WEIGHT_UNIT);
        setSingleStep(ngrt4n::WEIGHT_UNIT);
        break;
      }
    }
  };


  class IconButton : public QPushButton {
  public:
    IconButton(const QString& path, QWidget*parent = 0) : QPushButton(QIcon(path), "", parent) {
      setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
      setParent(parent);
      setFixedSize(QSize(24, 24));
    }
  };

  qint32 m_rows;
  qint16 m_currentRow;
  WidgetMapT m_fieldWidgets;
  QGridLayout* m_mainLayout;
  QDialogButtonBox* m_actionButtonBox;
  QLineEdit* m_externalServiceField;
  QMap<QString, QStringList> m_dataPoints;
  QLineEdit* m_dataPointSearchField;
  QComboBox* m_hostGroupFilterBox;
  QGroupBox* m_checkFieldsGroup;
  QStackedWidget* m_dataPointActionButtons;
  QPushButton* m_searchDataPointButton;
  QPushButton* m_addDataPointButton;
  QComboBox* m_calcRulesBox;
  QComboBox* m_propRulesBox;
  WeightBox* m_weightBox;
  WeightBox* m_thresholdWeightBox;
  QComboBox* m_thresholdInSeverityBox;
  QComboBox* m_thresholdOutSeverityBox;
  QComboBox* m_thresholdRulesBox;
  IconButton* m_addThresholdButton;
  IconButton* m_removeThresholdButton;
  QFrame* m_thresholdFrame;

  void addEvent(void);
  void layoutLabelFields(void);
  void layoutDescriptionFields(void);
  void layoutTypeFields(void);
  void layoutStatusCalcFields(void);
  void layoutStatusPropFields(void);
  void layoutAlarmMsgFields(void);
  void layoutNotificationMsgFields(void);
  void layoutIconFields(void);
  void layoutCheckField(void);
  void layoutButtonBox(void);
  QLabel* createCheckFieldHelpIcon(void);
  void setCheckFieldsStyle(void);
  void addAndSelectDataPointEntry(const QString& text);
  QString thresholdsData(void) const;

  void enableDataPointFields(bool enable);
  void showExternalServiceFields(bool enable);
};

#endif /* SNAVSERVICEEDITOR_H_ */
