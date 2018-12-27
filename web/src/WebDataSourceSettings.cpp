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
  m_livestatusPortField.setWidth(50);
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
  bindWidget("source-box", &m_sourceSelectionBox);
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
  m_sourceSelectionBox.changed().connect(this, &WebDataSourceSettings::handleSourceBoxChanged);
  m_showAuthStringField.changed().connect(this, &WebDataSourceSettings::handleShowAuthStringChanged);
}



bool WebDataSourceSettings::validateSourceSettingsFields(void)
{
  bool monitorSourceIsSet = ngrt4n::MonitorSourceTypes.contains( QString::fromStdString(m_monitorTypeField.currentText().toUTF8()));
  if (! monitorSourceIsSet ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No monitor source type set"));
    return false;
  }

  if (monitorSourceIsSet && m_monitorUrlField.validate() != Wt::WValidator::Valid) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Please fix field(s) in red"));
    return false;
  }

  return true;
}


void WebDataSourceSettings::applyChanges(void)
{
  applyChangesByIndex(m_sourceSelectionBox.currentIndex());
}

void WebDataSourceSettings::applyChangesByIndex(int index)
{
  if (! validateSourceSettingsFields()) {
    return ;
  }

  if (! ngrt4n::DataSourceIndices.contains(QString::number(index)) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Invalid monitor source index"));
    return;
  }

  auto&& monitorLabel = QString::fromStdString(m_monitorTypeField.currentText().toUTF8());
  QString validationErrorMsg = "";
  if (monitorLabel.contains(MonitorT::toString(MonitorT::Kubernetes))) {
    saveK8sDataSource(index);
  } else {
    saveAsSource(index);
  }
}



void WebDataSourceSettings::saveK8sDataSource(int sourceIndex)
{
  auto&& k8sProxyUrl = QString::fromStdString(m_monitorUrlField.text().toUTF8());
  auto&& verifySslPeer = m_dontVerifyCertificateField.checkState() == Wt::Checked;

  K8sHelper k8s(k8sProxyUrl, verifySslPeer);
  auto outListNamespaces = k8s.listNamespaces();
  if (outListNamespaces.second != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("failed connecting to Kubernetes API (%1)").arg(outListNamespaces.first.at(0)).toStdString());
    return ;
  }

  saveAsSource(sourceIndex);

  WebBaseSettings settings;
  DbSession dbSession(settings.getDbType(), settings.getDbConnectionString());

  for (auto&& ns: outListNamespaces.first) {
    NodeT nsNode;
    nsNode.type = NodeType::K8sClusterService;
    nsNode.id = QString("Source%1").arg(sourceIndex);
    nsNode.name = ns;
    nsNode.child_nodes = "";
    nsNode.sev_prule = PropRules::Unchanged;
    nsNode.sev_crule = CalcRules::Worst;
    nsNode.weight = ngrt4n::WEIGHT_UNIT;
    nsNode.icon = ngrt4n::K8S_NS;
    nsNode.description = QString("Namespace %1").arg(ns);

    CoreDataT cdata;
    cdata.monitor = MonitorT::Kubernetes;
    cdata.bpnodes.insert(nsNode.id, nsNode);

    auto destPath = QString("%1/k8s_ns_%2_%3.ms.ngrt4n.xml").arg(qgetenv("REALOPINSIGHT_CONFIG_DIR"), ns, ngrt4n::generateId());
    std::pair<int, QString> outSaveView = ngrt4n::saveViewDataToPath(cdata, destPath);

    if (outSaveView.first != ngrt4n::RcSuccess) {
      CORE_LOG("error", outSaveView.second.toStdString());
    } else {
      DboView vinfo;
      vinfo.name = QString("%1:%2").arg(nsNode.id, nsNode.name).toStdString();
      vinfo.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
      vinfo.path = destPath.toStdString();
      auto addViewOut = dbSession.addView(vinfo);
      if (addViewOut.first != ngrt4n::RcSuccess) {
        CORE_LOG("error", addViewOut.second.toStdString());
      }
    }
  }
}


void WebDataSourceSettings::addAsSource(void)
{
  if (! validateSourceSettingsFields()) {
    return;
  }
  m_sourceIndexSelector.show();
}



void WebDataSourceSettings::deleteSource(void)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session provided"));
    return;
  }

  auto currentIndex = m_sourceSelectionBox.currentIndex();
  if ( ! ngrt4n::DataSourceIndices.contains(QString::number(currentIndex)) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("cannot delete source (no valid source id)"));
    return;
  }

  auto deleteSourceOut = m_dbSession->deleteSource( ngrt4n::sourceId(currentIndex) );
  if (deleteSourceOut.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, deleteSourceOut.second.toStdString());
    return ;
  }

  m_sourceBoxModel.removeRow(currentIndex);
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

  m_sourceBoxModel.setStringList( activeSourceLabels );
  for (size_t i = 0; i < activeSourceLabels.size(); ++i) {
    m_sourceBoxModel.setData(static_cast<int>(i), 0, activeSourceIndices[i], Wt::UserRole);
  }

  m_sourceBoxModel.sort(0);
}


void WebDataSourceSettings::updateFields(void)
{
  int curIndex = m_sourceSelectionBox.currentIndex();
  if ( ngrt4n::DataSourceIndices.contains(QString::number(curIndex)) ) {
    m_sourceSelectionBox.setCurrentIndex(curIndex);
    fillFromSource(curIndex);
  }
}


void WebDataSourceSettings::saveAsSource(qint32 index)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session provided"));
    return;
  }

  // set global settings
  setKeyValue(SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY, m_updateIntervalField.text().toUTF8().c_str());

  // extract settings from  UI
  SourceT srcInfo;
  srcInfo.id = ngrt4n::sourceId(index);
  srcInfo.mon_url = m_monitorUrlField.text().toUTF8().c_str();
  srcInfo.ls_addr = m_livestatusHostField.text().toUTF8().c_str();
  srcInfo.ls_port = QString( m_livestatusPortField.text().toUTF8().c_str() ).toInt();
  srcInfo.auth = QString( m_authStringField.text().toUTF8().c_str() );
  srcInfo.verify_ssl_peer = (m_dontVerifyCertificateField.checkState() == Wt::Checked);

  auto sourceName = QString( m_monitorTypeField.currentText().toUTF8().c_str() );
  if (sourceName.contains("Nagios")) {
    srcInfo.mon_type = MonitorT::Nagios;
  } else if (sourceName.contains("Zabbix")) {
    srcInfo.mon_type = MonitorT::Zabbix;
  } else if (sourceName.contains("Zenoss")) {
    srcInfo.mon_type = MonitorT::Zenoss;
  } else if (sourceName.contains("OpManager")) {
    srcInfo.mon_type = MonitorT::OpManager;
  } else if (sourceName.contains("Kubernetes")) {
    srcInfo.mon_type = MonitorT::Kubernetes;
  } else {
    srcInfo.mon_type = MonitorT::Any;
  }

  auto addSourceOut = m_dbSession->addSource(srcInfo);
  if (addSourceOut.first == ngrt4n::RcDbDuplicationError) {
    addSourceOut = m_dbSession->updateSource(srcInfo);
  }

  if (addSourceOut.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, addSourceOut.second.toStdString());
    return;
  }

  // signal the change to other components
  Q_EMIT timerIntervalChanged(1000 * QString(m_updateIntervalField.text().toUTF8().c_str()).toInt());

  addToSourceBox(index);
  m_sourceSelectionBox.setCurrentIndex(findSourceIndexInBox(index));
  m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Settings saved"));
}


void WebDataSourceSettings::fillFromSource(int srcIndex)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session"));
    return;
  }

  if (srcIndex < 0 || srcIndex >= MAX_SRCS) {
    return;
  }

  auto findSourceOut = m_dbSession->findSourceById(ngrt4n::sourceId(srcIndex));
  if (! findSourceOut.first) {
    return ;
  }

  m_sourceSelectionBox.setValueText(ngrt4n::sourceId(srcIndex).toStdString());
  m_monitorUrlField.setText(findSourceOut.second.mon_url.toStdString());
  m_livestatusHostField.setText(findSourceOut.second.ls_addr.toStdString());
  m_livestatusPortField.setText(QString::number(findSourceOut.second.ls_port).toStdString());
  m_authStringField.setText(findSourceOut.second.auth.toStdString());
  m_monitorTypeField.setCurrentIndex( m_monitorTypeField.findText( MonitorT::toString(findSourceOut.second.mon_type).toStdString() ) );
  m_dontVerifyCertificateField.setCheckState(findSourceOut.second.verify_ssl_peer? Wt::Checked : Wt::Unchecked);
  m_updateIntervalField.setValue(updateInterval());

  updateComponentsVisibiliy(m_monitorTypeField.currentIndex());

  m_sourceSelectionBox.setCurrentIndex(srcIndex);
}


void WebDataSourceSettings::renderSourceIndexSelector(void)
{
  m_sourceSelectionBox.setModel(&m_sourceBoxModel);
  m_sourceIndexSelector.rejectWhenEscapePressed();

  m_sourceIndexSelector.setStyleClass("Wt-dialog");
  m_sourceIndexSelector.titleBar()->setStyleClass("titlebar");
  m_sourceIndexSelector.setWindowTitle(Q_TR("Select the source index"));

  // m_sourceIndexSelector.contents() take ownership on the pointer and will free up the oject
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



void WebDataSourceSettings::handleAddAsSourceOkAction(Wt::WComboBox* inputBox)
{
  m_sourceIndexSelector.accept();
  bool isValidIndex;
  int index = QString::fromStdString( inputBox->currentText().toUTF8() ).toInt(&isValidIndex);
  if (isValidIndex) {
    applyChangesByIndex(index);
  }
}



int WebDataSourceSettings::getSourceGlobalIndex(int sourceBoxIndex)
{
  boost::any value = static_cast<Wt::WAbstractItemModel*>(&m_sourceBoxModel)->data(sourceBoxIndex, 0, Wt::UserRole);
  return boost::any_cast<int>(value);
}


int WebDataSourceSettings::findSourceIndexInBox(int sourceGlobalIndex)
{
  int index = m_sourceBoxModel.rowCount() - 1;
  while (index >= 0 && (getSourceGlobalIndex(index) != sourceGlobalIndex)) --index;
  return index;
}


void WebDataSourceSettings::addToSourceBox(int sourceGlobalIndex)
{
  int index = findSourceIndexInBox(sourceGlobalIndex);
  if (index < 0) {
    m_sourceBoxModel.addString(ngrt4n::sourceId(sourceGlobalIndex).toStdString());
    m_sourceBoxModel.setData(m_sourceBoxModel.rowCount() - 1, 0, sourceGlobalIndex, Wt::UserRole);
  }
  m_sourceBoxModel.sort(0);
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
  fillFromSource(getSourceGlobalIndex(m_sourceSelectionBox.currentIndex()));
}


void WebDataSourceSettings::updateComponentsVisibiliy(int monitorTypeCurrentIndex)
{
  const auto monitorText = QString::fromStdString(m_monitorTypeField.itemText(monitorTypeCurrentIndex).toUTF8());

  auto NagiosSelected = monitorText.contains("Nagios");
  if (NagiosSelected) {
    m_livestatusHostField.setHidden(false);
    m_livestatusPortField.setHidden(false);
    wApp->doJavaScript("$('#livetstatus-section').show();");
  } else {
    m_livestatusHostField.setHidden(true);
    m_livestatusPortField.setHidden(true);
    wApp->doJavaScript("$('#livetstatus-section').hide();");
  }

  auto KubernetesNotSelected = ! monitorText.contains("Kubernetes");
  m_authStringField.setEnabled(KubernetesNotSelected);
}


void WebDataSourceSettings::handleShowAuthStringChanged(void)
{
  if (m_showAuthStringField.isChecked()) {
    m_authStringField.setEchoMode(Wt::WLineEdit::Normal);
  } else {
    m_authStringField.setEchoMode(Wt::WLineEdit::Password);
  }
}
