/*
 # ------------------------------------------------------------------------#
# Copyright (c) 2018 Rodrigue Chakode                                      #
# Creation : 17-12-2017                                                    #
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
# ------------------------------------------------------------------------ #
*/

#include "dbo/src/DbSession.hpp"
#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include "K8sHelper.hpp"
#include "WebDataSourceSettings.hpp"
#include <Wt/WSpinBox>
#include <Wt/WApplication>


WebDataSourceSettings::WebDataSourceSettings()
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("data-source-settings-form.tpl")),
    m_operationCompleted(this),
    m_dbSession(nullptr)
{
  createFormWidgets();
  bindFormWidgets();
  setEnabledInputs(false);
  addEvent();
  updateContents();
}


WebDataSourceSettings::~WebDataSourceSettings()
{
  unbindFormWidgets();
}


void WebDataSourceSettings::createFormWidgets(void)
{
  renderSourceIndexSelector();

  m_monitorTypeField.addItem(Q_TR("-- Select a type --"));
  for (const auto& srcid: ngrt4n::MonitorSourceTypes) {
    m_monitorTypeField.addItem(srcid.toStdString());
  }

  m_monitorUrlField.setValidator(new UriValidator("http", false, this));
  m_monitorUrlField.setEmptyText("Set the url to the monitor web interface");

  // set livestatus server
  m_livestatusHostField.setEmptyText("hostname/IP");
  m_livestatusHostField.setValidator(new HostValidator(this));

  // set livestatus port field
  m_livestatusPortField.setWidth(75);
  m_livestatusPortField.setValidator(new PortValidator(this));
  m_livestatusPortField.setEmptyText("port");
  m_livestatusPortField.setMaxLength(5);

  // other fields
  m_dontVerifyCertificateField.setText( Q_TR("Don't verify SSL certificate") );

  // update interval field
  m_updateIntervalField.setMinimum(5);
  m_updateIntervalField.setMaximum(1200);
  m_updateIntervalField.setValue(BaseSettings::updateInterval());

  m_authStringField.setEchoMode(Wt::WLineEdit::Password);
  m_authStringField.setEmptyText( Q_TR("Set the authentication string") );
  m_showAuthStringField.setText( Q_TR("Show in clear") );

  // buttons
  m_applyChangeBtn.setText( Q_TR("Apply changes") );
  m_addAsSourceBtn.setText( Q_TR("Add as new source") );
  m_deleteSourceBtn.setText( Q_TR("Delete source") );
  m_applyChangeBtn.setStyleClass("btn btn-success");
  m_addAsSourceBtn.setStyleClass("btn btn-info");
  m_deleteSourceBtn.setStyleClass("btn btn-danger");
}


void WebDataSourceSettings::bindFormWidgets(void)
{
  bindWidget("show-in-clear", &m_showAuthStringField);
  bindWidget("monitor-auth-string", &m_authStringField);
  bindWidget("monitor-url", &m_monitorUrlField);
  bindWidget("monitor-type", &m_monitorTypeField);
  bindWidget("source-box", &m_sourceSelectionField);
  bindWidget("dont-verify-ssl-certificate", &m_dontVerifyCertificateField);
  bindWidget("update-interval", &m_updateIntervalField);
  bindWidget("livestatus-server", &m_livestatusHostField);
  bindWidget("livestatus-port", &m_livestatusPortField);
  bindWidget("apply-change-button", &m_applyChangeBtn);
  bindWidget("add-as-source-button", &m_addAsSourceBtn);
  bindWidget("delete-button", &m_deleteSourceBtn);
}


void WebDataSourceSettings::unbindFormWidgets(void)
{
  takeWidget("show-in-clear");
  takeWidget("monitor-auth-string");
  takeWidget("monitor-url");
  takeWidget("monitor-type");
  takeWidget("source-box");
  takeWidget("dont-verify-ssl-certificate");
  takeWidget("update-interval");
  takeWidget("livestatus-server");
  takeWidget("livestatus-port");
  takeWidget("use-ngrt4nd");
  takeWidget("apply-change-button");
  takeWidget("add-as-source-button");
  takeWidget("delete-button");
}


void WebDataSourceSettings::addEvent(void)
{
  m_applyChangeBtn.clicked().connect(this, &WebDataSourceSettings::applyChanges);
  m_addAsSourceBtn.clicked().connect(this, &WebDataSourceSettings::addAsSource);
  m_deleteSourceBtn.clicked().connect(this, &WebDataSourceSettings::deleteSource);
  m_monitorTypeField.activated().connect(this, &WebDataSourceSettings::updateComponentsVisibiliy);
  m_sourceSelectionField.changed().connect(this, &WebDataSourceSettings::handleSourceBoxChanged);
  m_showAuthStringField.changed().connect(this, &WebDataSourceSettings::handleShowAuthStringChanged);
}



bool WebDataSourceSettings::validateSourceSettingsFields(void)
{
  bool monitorTypeIsSet = ngrt4n::MonitorSourceTypes.contains( QString::fromStdString(m_monitorTypeField.currentText().toUTF8()));
  if (! monitorTypeIsSet ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No monitor type selected"));
    return false;
  }

  SourceT sinfo = extractSourceSettingsGivenIndex( m_sourceSelectionField.currentIndex() );
  bool isValidMonitorUrl = (m_monitorUrlField.validate() == Wt::WValidator::Valid || sinfo.mon_type == MonitorT::Nagios);
  if (! isValidMonitorUrl) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Please fix field(s) in red"));
    return false;
  }

  return true;
}


void WebDataSourceSettings::applyChanges(void)
{
  setKeyValue(SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY, m_updateIntervalField.text().toUTF8().c_str());
  applyChangesGivenSourceId(m_sourceSelectionField.currentIndex());
}

void WebDataSourceSettings::applyChangesGivenSourceId(int index)
{
  if (! validateSourceSettingsFields()) {
    return ;
  }

  if (! ngrt4n::DataSourceIndices.contains(QString::number(index)) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Invalid monitor source index"));
    return;
  }

  SourceT sinfo = extractSourceSettingsGivenIndex(index);
  QHash<QString, bool> monitoredGroups;
  if (sinfo.mon_type == MonitorT::Kubernetes) {
    K8sHelper k8sHelper(sinfo.mon_url, sinfo.verify_ssl_peer);
    auto outListNamespaces = k8sHelper.listNamespaces();
    if (outListNamespaces.second != ngrt4n::RcSuccess) {
      m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("failed connecting to source (%1)").arg(outListNamespaces.first.at(0)).toStdString());
      return ;
    }
    for (const auto& mgroup: outListNamespaces.first) {
      monitoredGroups[mgroup] = true;
    }
  } else {
    ChecksT checks;
    auto loadDataItemsOut = ngrt4n::loadDataItems(sinfo, "", checks);
    if (loadDataItemsOut.first != ngrt4n::RcSuccess) {
      m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("failed connecting to source (%1)").arg(loadDataItemsOut.second).toStdString());
      return ;
    }
    for (const auto& check: checks) {
      auto groups = QString::fromStdString(check.host_groups).split(ngrt4n::CHILD_Q_SEP);
      for (const auto& group: groups) {
        monitoredGroups[group] = true;
      }
    }
  }

  saveSourceInDatabase(sinfo);

  WebBaseSettings settings;
  DbSession dbSession(settings.getDbType(), settings.getDbConnectionString());
  for (const QString& mgroup: monitoredGroups.keys()) {
    NodeT gNode;
    gNode.type = NodeType::K8sClusterService;
    gNode.id = sinfo.id;
    gNode.name = mgroup;
    gNode.child_nodes = "";
    gNode.sev_prule = PropRules::Unchanged;
    gNode.sev_crule = CalcRules::Worst;
    gNode.weight = ngrt4n::WEIGHT_UNIT;
    gNode.icon = ngrt4n::K8S_NS;
    gNode.description = QString("Namespace %1").arg(mgroup);

    CoreDataT cdata;
    cdata.monitor = sinfo.mon_type;
    cdata.bpnodes.insert(gNode.id, gNode);

    auto destPath = QString("%1/mgroup_%2_%3.ms.ngrt4n.xml").arg(qgetenv("REALOPINSIGHT_CONFIG_DIR"),
                                                                 QString(mgroup).replace(" ", "_").replace("/", "_").replace("\\", "_"),
                                                                 ngrt4n::generateId());
    std::pair<int, QString> outSaveView = ngrt4n::saveViewDataToPath(cdata, destPath);

    if (outSaveView.first != ngrt4n::RcSuccess) {
      CORE_LOG("error", outSaveView.second.toStdString());
    } else {
      DboView vinfo;
      vinfo.name = QString("%1:%2").arg(gNode.id, gNode.name).toStdString();
      vinfo.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
      vinfo.path = destPath.toStdString();
      auto addViewOut = dbSession.addView(vinfo);
      if (addViewOut.first != ngrt4n::RcSuccess) {
        CORE_LOG("error", addViewOut.second.toStdString());
      }
    }
  }

  updateSourceDataModel(index);
  m_sourceSelectionField.setCurrentIndex(findFormSourceIndex(index));
  m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Settings saved"));
}


void WebDataSourceSettings::addAsSource(void)
{
  if (! validateSourceSettingsFields()) {
    return;
  }

  m_sourceIndexSelector.show(); // accept action from this form triggers the method handleAddAsSourceOkAction
}



void WebDataSourceSettings::deleteSource(void)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session provided"));
    return;
  }

  auto currentIndex = m_sourceSelectionField.currentIndex();
  if ( ! ngrt4n::DataSourceIndices.contains(QString::number(currentIndex)) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("cannot delete source (no valid source id)"));
    return;
  }

  auto deleteSourceOut = m_dbSession->deleteSource( ngrt4n::sourceId(currentIndex) );
  if (deleteSourceOut.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, deleteSourceOut.second.toStdString());
    return ;
  }

  m_sourceDataModel.removeRow(currentIndex);
  updateFields();
}


void WebDataSourceSettings::updateAllSourceWidgetStates(void)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session"));
    return;
  }

  std::vector< Wt::WString > activeSourceLabels;
  std::vector< int > activeSourceIndices;
  for (const auto& src: m_dbSession->listSources(MonitorT::Any)) {
    activeSourceLabels.push_back(src.id.toStdString());
    activeSourceIndices.push_back(QString(src.id.at(src.id.size() - 1)).toInt());
  }

  m_sourceDataModel.setStringList( activeSourceLabels );
  for (size_t i = 0; i < activeSourceLabels.size(); ++i) {
    m_sourceDataModel.setData(static_cast<int>(i), 0, activeSourceIndices[i], Wt::UserRole);
  }

  m_sourceDataModel.sort(0);
}


void WebDataSourceSettings::updateFields(void)
{
  int curIndex = m_sourceSelectionField.currentIndex();
  if ( ngrt4n::DataSourceIndices.contains(QString::number(curIndex)) ) {
    m_sourceSelectionField.setCurrentIndex(curIndex);
    fillInFormGivenSourceId(curIndex);
  }
}

void WebDataSourceSettings::saveSourceInDatabase(const SourceT& sinfo)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session provided"));
    return;
  }

  auto addSourceOut = m_dbSession->addSource(sinfo);
  if (addSourceOut.first == ngrt4n::RcDbDuplicationError) {
    addSourceOut = m_dbSession->updateSource(sinfo);
  }

  if (addSourceOut.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, addSourceOut.second.toStdString());
    return;
  }

}


void WebDataSourceSettings::fillInFormGivenSourceId(int sid)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session"));
    return;
  }

  if (sid < 0 || sid >= MAX_SRCS) {
    return;
  }

  auto findSourceOut = m_dbSession->findSourceById(ngrt4n::sourceId(sid));
  if (! findSourceOut.first) {
    return ;
  }

  m_sourceSelectionField.setValueText(ngrt4n::sourceId(sid).toStdString());
  m_monitorUrlField.setText(findSourceOut.second.mon_url.toStdString());
  m_livestatusHostField.setText(findSourceOut.second.ls_addr.toStdString());
  m_livestatusPortField.setText(QString::number(findSourceOut.second.ls_port).toStdString());
  m_authStringField.setText(findSourceOut.second.auth.toStdString());
  m_monitorTypeField.setCurrentIndex( m_monitorTypeField.findText( MonitorT::toString(findSourceOut.second.mon_type).toStdString() ) );
  m_dontVerifyCertificateField.setCheckState(findSourceOut.second.verify_ssl_peer? Wt::Checked : Wt::Unchecked);
  m_updateIntervalField.setValue(updateInterval());

  updateComponentsVisibiliy(m_monitorTypeField.currentIndex());

  m_sourceSelectionField.setCurrentIndex(sid);
}


void WebDataSourceSettings::renderSourceIndexSelector(void)
{
  m_sourceSelectionField.setModel(&m_sourceDataModel);
  m_sourceIndexSelector.rejectWhenEscapePressed();

  m_sourceIndexSelector.setStyleClass("Wt-dialog");
  m_sourceIndexSelector.titleBar()->setStyleClass("titlebar");
  m_sourceIndexSelector.setWindowTitle(Q_TR("Select the source index"));

  // m_sourceIndexSelector.contents() take ownership on the pointer and will free up the object
  Wt::WComboBox* sourceIndexField = new Wt::WComboBox(m_sourceIndexSelector.contents());
  for (const auto& src : ngrt4n::DataSourceIndices) {
    sourceIndexField->addItem(src.toStdString());
  }

  // m_sourceIndexSelector.contents() take ownership on the pointer and will free up the oject
  Wt::WPushButton *sourceIndexValidationBtn = new Wt::WPushButton("OK", m_sourceIndexSelector.footer());
  sourceIndexValidationBtn->clicked().connect(std::bind(&WebDataSourceSettings::handleAddAsSourceOkAction, this, sourceIndexField));
  sourceIndexValidationBtn->setDefault(true);

  // m_sourceIndexSelector.contents() take ownership on the pointer and will free up the oject
  Wt::WPushButton *sourceIndexCancellationBtn = new Wt::WPushButton("Cancel", m_sourceIndexSelector.footer());
  sourceIndexCancellationBtn->clicked().connect(&m_sourceIndexSelector, &Wt::WDialog::reject);
}



void WebDataSourceSettings::handleAddAsSourceOkAction(Wt::WComboBox* sourceIndexSectionField)
{
  m_sourceIndexSelector.accept();
  bool isValidIndex;
  int index = QString::fromStdString( sourceIndexSectionField->currentText().toUTF8() ).toInt(&isValidIndex);
  if (isValidIndex) {
    applyChangesGivenSourceId(index);
  }
}



int WebDataSourceSettings::getSourceId(int sid)
{
  boost::any value = static_cast<Wt::WAbstractItemModel*>(&m_sourceDataModel)->data(sid, 0, Wt::UserRole);
  return boost::any_cast<int>(value);
}


int WebDataSourceSettings::findFormSourceIndex(int sourceGlobalIndex)
{
  int index = m_sourceDataModel.rowCount() - 1;
  while (index >= 0 && (getSourceId(index) != sourceGlobalIndex)) --index;
  return index;
}


void WebDataSourceSettings::updateSourceDataModel(int sid)
{
  int index = findFormSourceIndex(sid);
  if (index < 0) {
    m_sourceDataModel.addString(ngrt4n::sourceId(sid).toStdString());
    m_sourceDataModel.setData(m_sourceDataModel.rowCount() - 1, 0, sid, Wt::UserRole);
  }
  m_sourceDataModel.sort(0);
}


void WebDataSourceSettings::setEnabledInputs(bool enable)
{
  m_monitorUrlField.setEnabled(enable);
  m_authStringField.setEnabled(enable);
  m_livestatusHostField.setEnabled(enable);
  m_livestatusPortField.setEnabled(enable);
  m_monitorTypeField.setEnabled(enable);
  m_showAuthStringField.setEnabled(enable);
  m_dontVerifyCertificateField.setEnabled(enable);
  m_updateIntervalField.setEnabled(enable);
  m_applyChangeBtn.setEnabled(enable);
  m_addAsSourceBtn.setEnabled(enable);
  m_deleteSourceBtn.setEnabled(enable);
}



void WebDataSourceSettings::handleSourceBoxChanged(void)
{
  fillInFormGivenSourceId(getSourceId(m_sourceSelectionField.currentIndex()));
}


void WebDataSourceSettings::updateComponentsVisibiliy(int monitorTypeCurrentIndex)
{
  const auto monitorType = QString::fromStdString(m_monitorTypeField.itemText(monitorTypeCurrentIndex).toUTF8());

  const auto NagiosSourceSelected = monitorType.contains("Nagios");
  if (NagiosSourceSelected) {
    m_livestatusHostField.setFocus();
    wApp->doJavaScript("$('#livetstatus-settings').show();");
    wApp->doJavaScript("$('#source-api-settings').hide();");
  } else {
    m_monitorUrlField.setFocus();
    m_showAuthStringField.setCheckState(Wt::Unchecked);
    handleShowAuthStringChanged();
    wApp->doJavaScript("$('#livetstatus-settings').hide();");
    wApp->doJavaScript("$('#source-api-settings').show();");
  }

  const auto K8sSourceSelected = ! monitorType.contains("Kubernetes");
  m_authStringField.setEnabled(K8sSourceSelected);
}


void WebDataSourceSettings::handleShowAuthStringChanged(void)
{
  if (m_showAuthStringField.isChecked()) {
    m_authStringField.setEchoMode(Wt::WLineEdit::Normal);
  } else {
    m_authStringField.setEchoMode(Wt::WLineEdit::Password);
  }
}


SourceT WebDataSourceSettings::extractSourceSettingsGivenIndex(int sourceIndex)
{
  SourceT sinfo;

  auto sourceName = QString( m_monitorTypeField.currentText().toUTF8().c_str() );
  if (sourceName.contains("Nagios")) {
    sinfo.mon_type = MonitorT::Nagios;
  } else if (sourceName.contains("Zabbix")) {
    sinfo.mon_type = MonitorT::Zabbix;
  } else if (sourceName.contains("Zenoss")) {
    sinfo.mon_type = MonitorT::Zenoss;
  } else if (sourceName.contains("OpManager")) {
    sinfo.mon_type = MonitorT::OpManager;
  } else if (sourceName.contains("Pandora FMS")) {
    sinfo.mon_type = MonitorT::Pandora;
  } else if (sourceName.contains("Kubernetes")) {
    sinfo.mon_type = MonitorT::Kubernetes;
  } else {
    sinfo.mon_type = MonitorT::Any;
  }

  sinfo.id = ngrt4n::sourceId(sourceIndex);
  sinfo.auth = QString( m_authStringField.text().toUTF8().c_str() );

  if (sinfo.mon_type == MonitorT::Nagios) {
    sinfo.ls_addr = m_livestatusHostField.text().toUTF8().c_str();
    sinfo.ls_port = QString( m_livestatusPortField.text().toUTF8().c_str() ).toInt();
  } else {
    sinfo.mon_url = m_monitorUrlField.text().toUTF8().c_str();
    sinfo.verify_ssl_peer = (m_dontVerifyCertificateField.checkState() == Wt::Checked);
  }

  return sinfo;
}
