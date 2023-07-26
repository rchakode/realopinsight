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

#include <any>
#include "dbo/src/DbSession.hpp"
#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include "K8sHelper.hpp"
#include "WebDataSourceSettings.hpp"
#include <Wt/WSpinBox.h>
#include <Wt/WApplication.h>


WebDataSourceSettings::WebDataSourceSettings()
  : WebBaseSettings(),
    Wt::WTemplate(Wt::WString::tr("data-source-settings-form.tpl")),
    m_dbSession(nullptr)
{

  m_sourceSelectionFieldRef = bindNew<Wt::WComboBox>("source-box");
  m_sourceSelectionFieldRef->changed().connect(this, &WebDataSourceSettings::handleSourceBoxChanged);

  m_sourceDataModel = std::make_shared<Wt::WStringListModel>();
  m_sourceSelectionFieldRef->setModel(m_sourceDataModel);

  m_sourceIndexSelector.rejectWhenEscapePressed();
  m_sourceIndexSelector.setStyleClass("Wt-dialog");
  m_sourceIndexSelector.titleBar()->setStyleClass("titlebar");
  m_sourceIndexSelector.setWindowTitle(Q_TR("Select the source index"));
  m_sourceIndexFieldRef = m_sourceIndexSelector.contents()->addNew<Wt::WComboBox>();
  for (const auto& src : ngrt4n::DataSourceIndices) { m_sourceIndexFieldRef->addItem(src.toStdString()); }

  auto sourceIndexValidationBtn = std::make_unique<Wt::WPushButton>("OK");
  sourceIndexValidationBtn->setDefault(true);
  sourceIndexValidationBtn->clicked().connect(this, &WebDataSourceSettings::handleAddAsSource);
  m_sourceIndexSelector.footer()->addWidget(std::move(sourceIndexValidationBtn));

  auto sourceIndexCancellationBtn = std::make_unique<Wt::WPushButton>("Cancel");
  sourceIndexCancellationBtn->clicked().connect(&m_sourceIndexSelector, &Wt::WDialog::reject);
  m_sourceIndexSelector.footer()->addWidget(std::move(sourceIndexCancellationBtn));

  m_monitorTypeFieldRef = bindNew<Wt::WComboBox>("monitor-type");
  m_monitorTypeFieldRef->addItem(Q_TR("-- Select a type --"));
  for (const auto& srcid: ngrt4n::MonitorSourceTypes) { m_monitorTypeFieldRef->addItem(srcid.toStdString()); }
  m_monitorTypeFieldRef->activated().connect(this, &WebDataSourceSettings::updateComponentsVisibiliy);

  m_monitorUrlFieldRef = bindNew<Wt::WLineEdit>("monitor-url");
  m_monitorUrlFieldRef->setValidator(std::make_unique<UriValidator>("http", false, this));
  m_monitorUrlFieldRef->setPlaceholderText("Set the url to the monitor web interface");

  m_livestatusHostFieldRef = bindNew<Wt::WLineEdit>("livestatus-server");
  m_livestatusHostFieldRef->setPlaceholderText("hostname/IP");
  m_livestatusHostFieldRef->setValidator(std::make_unique<HostValidator>(this));

  m_livestatusPortFieldRef = bindNew<Wt::WLineEdit>("livestatus-port");
  m_livestatusPortFieldRef->setWidth(75);
  m_livestatusPortFieldRef->setValidator(std::make_unique<PortValidator>());
  m_livestatusPortFieldRef->setPlaceholderText("port");
  m_livestatusPortFieldRef->setMaxLength(5);

  m_dontVerifyCertificateFieldRef = bindNew<Wt::WCheckBox>("dont-verify-ssl-certificate");
  m_dontVerifyCertificateFieldRef->setText(Q_TR("Don't verify SSL certificate") );

  m_updateIntervalFieldRef = bindNew<Wt::WSpinBox>("update-interval");
  m_updateIntervalFieldRef->setMinimum(5);
  m_updateIntervalFieldRef->setMaximum(1200);
  m_updateIntervalFieldRef->setValue(BaseSettings::updateInterval());

  m_authStringFieldRef = bindNew<Wt::WLineEdit>("monitor-auth-string");
  m_authStringFieldRef->setEchoMode(Wt::EchoMode::Password);
  m_authStringFieldRef->setPlaceholderText( Q_TR("Set the authentication string") );

  m_showAuthStringFieldRef = bindNew<Wt::WCheckBox>("show-in-clear");
  m_showAuthStringFieldRef->setText( Q_TR("Show in clear") );
  m_showAuthStringFieldRef->changed().connect(this, &WebDataSourceSettings::handleShowAuthStringChanged);

  m_applyBtnRef = bindNew<Wt::WPushButton>("apply-change-button");
  m_applyBtnRef->setText(Q_TR("Apply changes"));
  m_applyBtnRef->setStyleClass("btn btn-success");
  m_applyBtnRef->clicked().connect(this, &WebDataSourceSettings::applyChanges);

  m_addAsNewBtnRef = bindNew<Wt::WPushButton>("add-as-source-button");
  m_addAsNewBtnRef->setText(Q_TR("Add as new source"));
  m_addAsNewBtnRef->setStyleClass("btn btn-info");
  m_addAsNewBtnRef->clicked().connect(this, &WebDataSourceSettings::addAsSource);

  m_deleteBtnRef = bindNew<Wt::WPushButton>("delete-button");
  m_deleteBtnRef->setText(Q_TR("Delete source"));
  m_deleteBtnRef->setStyleClass("btn btn-danger");
  m_deleteBtnRef->clicked().connect(this, &WebDataSourceSettings::deleteSource);

  setEnabledInputs(false);
  updateContents();
}


WebDataSourceSettings::~WebDataSourceSettings(){}


bool WebDataSourceSettings::validateSourceSettingsFields(void)
{
  bool monitorTypeIsSet = ngrt4n::MonitorSourceTypes.contains( QString::fromStdString(m_monitorTypeFieldRef->currentText().toUTF8()));
  if (! monitorTypeIsSet ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No monitor type selected"));
    return false;
  }

  SourceT sinfo = getSourceInfoByIndex( m_sourceSelectionFieldRef->currentIndex() );
  bool isValidMonitorUrl = (m_monitorUrlFieldRef->validate() == Wt::ValidationState::Valid || sinfo.mon_type == MonitorT::Nagios);
  if (! isValidMonitorUrl) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Please fix field(s) in red"));
    return false;
  }

  return true;
}


void WebDataSourceSettings::applyChanges(void)
{
  SettingFactory().setKeyValue(SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY, m_updateIntervalFieldRef->text().toUTF8().c_str());
  applySourceChanges(m_sourceSelectionFieldRef->currentIndex());
}

void WebDataSourceSettings::applySourceChanges(int index)
{
  if (! validateSourceSettingsFields()) {
    return ;
  }

  if (! ngrt4n::DataSourceIndices.contains(QString::number(index)) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Invalid monitor source index"));
    return;
  }

  SourceT sinfo = getSourceInfoByIndex(index);
  QHash<QString, bool> monitoredGroups;
  if (sinfo.mon_type == MonitorT::Kubernetes) {
    K8sHelper k8sHelper(sinfo.mon_url, sinfo.verify_ssl_peer, sinfo.auth);
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

  DbSession dbSession;
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
    auto destPath = QString("%1/mgroup_%2_%3.roi.xml").arg(SettingFactory::coreDataDir(),
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
  m_sourceSelectionFieldRef->setCurrentIndex(findFormSourceIndex(index));
  m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("settings updated"));
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

  auto currentIndex = m_sourceSelectionFieldRef->currentIndex();
  if ( ! ngrt4n::DataSourceIndices.contains(QString::number(currentIndex)) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("cannot delete source (no valid source id)"));
    return;
  }

  auto deleteSourceOut = m_dbSession->deleteSource( ngrt4n::sourceId(currentIndex) );
  if (deleteSourceOut.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, deleteSourceOut.second.toStdString());
    return ;
  }

  m_sourceDataModel->removeRow(currentIndex);
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

  m_sourceDataModel->setStringList( activeSourceLabels );
  for (size_t i = 0; i < activeSourceLabels.size(); ++i) {
    m_sourceDataModel->setData(static_cast<int>(i), 0, activeSourceIndices[i], Wt::ItemDataRole::User);
  }

  m_sourceDataModel->sort(0);
}


void WebDataSourceSettings::updateFields(void)
{
  int curIndex = m_sourceSelectionFieldRef->currentIndex();
  if ( ngrt4n::DataSourceIndices.contains(QString::number(curIndex)) ) {
    m_sourceSelectionFieldRef->setCurrentIndex(curIndex);
    fillInFormGivenSourceId(curIndex);
  }
}

void WebDataSourceSettings::saveSourceInDatabase(const SourceT& sinfo)
{
  if (! m_dbSession) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("no valid db session provided"));
    return;
  }

  auto dbResp = m_dbSession->addSource(sinfo);
  if (dbResp.first == ngrt4n::RcDbDuplicationError) {
    dbResp = m_dbSession->updateSource(sinfo);
  }

  if (dbResp.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, dbResp.second.toStdString());
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

  auto sourceLoaded = m_dbSession->findSourceById(ngrt4n::sourceId(sid));
  if (! sourceLoaded.first) {
    return ;
  }

  m_sourceSelectionFieldRef->setValueText(ngrt4n::sourceId(sid).toStdString());
  m_monitorUrlFieldRef->setText(sourceLoaded.second.mon_url.toStdString());
  m_livestatusHostFieldRef->setText(sourceLoaded.second.ls_addr.toStdString());
  m_livestatusPortFieldRef->setText(QString::number(sourceLoaded.second.ls_port).toStdString());
  m_authStringFieldRef->setText(sourceLoaded.second.auth.toStdString());
  m_monitorTypeFieldRef->setCurrentIndex( m_monitorTypeFieldRef->findText( MonitorT::toString(sourceLoaded.second.mon_type).toStdString() ) );
  m_dontVerifyCertificateFieldRef->setCheckState(sourceLoaded.second.verify_ssl_peer? Wt::CheckState::Unchecked : Wt::CheckState::Checked);
  m_updateIntervalFieldRef->setValue(updateInterval());

  updateComponentsVisibiliy(m_monitorTypeFieldRef->currentIndex());

  m_sourceSelectionFieldRef->setCurrentIndex(sid);
}

void WebDataSourceSettings::handleAddAsSource(Wt::WMouseEvent)
{
  m_sourceIndexSelector.accept();
  bool isValidIndex;
  int index = QString::fromStdString( m_sourceIndexFieldRef->currentText().toUTF8() ).toInt(&isValidIndex);
  if (isValidIndex) {
    applySourceChanges(index);
  }
}


int WebDataSourceSettings::getSourceId(int sid)
{
  Wt::cpp17::any value = static_cast<Wt::WAbstractItemModel*>(m_sourceDataModel.get())->data(sid, 0, Wt::ItemDataRole::User);
  return Wt::cpp17::any_cast<int>(value);
}


int WebDataSourceSettings::findFormSourceIndex(int sourceGlobalIndex)
{
  int index = m_sourceDataModel->rowCount() - 1;
  while (index >= 0 && (getSourceId(index) != sourceGlobalIndex)) --index;
  return index;
}


void WebDataSourceSettings::updateSourceDataModel(int sid)
{
  int index = findFormSourceIndex(sid);
  if (index < 0) {
    m_sourceDataModel->addString(ngrt4n::sourceId(sid).toStdString());
    m_sourceDataModel->setData(m_sourceDataModel->rowCount() - 1, 0, sid, Wt::ItemDataRole::User);
  }
  m_sourceDataModel->sort(0);
}


void WebDataSourceSettings::setEnabledInputs(bool enable)
{
  m_monitorUrlFieldRef->setEnabled(enable);
  m_authStringFieldRef->setEnabled(enable);
  m_livestatusHostFieldRef->setEnabled(enable);
  m_livestatusPortFieldRef->setEnabled(enable);
  m_monitorTypeFieldRef->setEnabled(enable);
  m_showAuthStringFieldRef->setEnabled(enable);
  m_dontVerifyCertificateFieldRef->setEnabled(enable);
  m_updateIntervalFieldRef->setEnabled(enable);
  m_applyBtnRef->setEnabled(enable);
  m_addAsNewBtnRef->setEnabled(enable);
  m_deleteBtnRef->setEnabled(enable);
}



void WebDataSourceSettings::handleSourceBoxChanged(void)
{
  fillInFormGivenSourceId(getSourceId(m_sourceSelectionFieldRef->currentIndex()));
}


void WebDataSourceSettings::updateComponentsVisibiliy(int monitorTypeCurrentIndex)
{
  const auto monitorType = QString::fromStdString(m_monitorTypeFieldRef->itemText(monitorTypeCurrentIndex).toUTF8());

  const auto NagiosSourceSelected = monitorType.contains("Nagios");
  if (NagiosSourceSelected) {
    m_livestatusHostFieldRef->setFocus();
    wApp->doJavaScript("$('#livetstatus-settings').show();");
    wApp->doJavaScript("$('#source-api-settings').hide();");
  } else {
    m_monitorUrlFieldRef->setFocus();
    m_showAuthStringFieldRef->setCheckState(Wt::CheckState::Unchecked);
    handleShowAuthStringChanged();
    wApp->doJavaScript("$('#livetstatus-settings').hide();");
    wApp->doJavaScript("$('#source-api-settings').show();");
  }
}


void WebDataSourceSettings::handleShowAuthStringChanged(void)
{
  if (m_showAuthStringFieldRef->isChecked()) {
    m_authStringFieldRef->setEchoMode(Wt::EchoMode::Normal);
  } else {
    m_authStringFieldRef->setEchoMode(Wt::EchoMode::Password);
  }
}


SourceT WebDataSourceSettings::getSourceInfoByIndex(int sourceIndex)
{
  SourceT sinfo;

  auto sourceName = QString( m_monitorTypeFieldRef->currentText().toUTF8().c_str() );
  if (sourceName.contains("Nagios")) {
    sinfo.mon_type = MonitorT::Nagios;
  } else if (sourceName.contains("Zabbix")) {
    sinfo.mon_type = MonitorT::Zabbix;
  } else if (sourceName.contains("Kubernetes")) {
    sinfo.mon_type = MonitorT::Kubernetes;
  } else {
    sinfo.mon_type = MonitorT::Any;
  }

  sinfo.id = ngrt4n::sourceId(sourceIndex);
  sinfo.auth = QString( m_authStringFieldRef->text().toUTF8().c_str() );

  if (sinfo.mon_type == MonitorT::Nagios) {
    sinfo.ls_addr = m_livestatusHostFieldRef->text().toUTF8().c_str();
    sinfo.ls_port = QString( m_livestatusPortFieldRef->text().toUTF8().c_str() ).toInt();
  } else {
    sinfo.mon_url = m_monitorUrlFieldRef->text().toUTF8().c_str();
    sinfo.verify_ssl_peer = (m_dontVerifyCertificateFieldRef->checkState() != Wt::CheckState::Checked);
  }

  return sinfo;
}
