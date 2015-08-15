/*
 * WebDataSourcePreferences.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 21-06-2015                                                     #
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

#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include "WebDataSourcePreferences.hpp"
#include "WebLicenseManager.hpp"
#include <Wt/WSpinBox>
#include <Wt/WApplication>


WebDataSourcePreferences::WebDataSourcePreferences()
  : WebPreferencesBase(),
    Wt::WTemplate(Wt::WString::tr("data-source-settings-form.tpl")),
    m_operationCompleted(this)
{
  createFormWidgets();
  bindFormWidgets();
  setEnabledInputs(false);
  addEvent();
  updateContents();
}


WebDataSourcePreferences::~WebDataSourcePreferences()
{
  unbindFormWidgets();
}


void WebDataSourcePreferences::createFormWidgets(void)
{
  renderSourceIndexSelector();

  m_monitorTypeField.addItem(Q_TR("-- Select a type --"));
  for (const auto& srcid: ngrt4n::sourceTypes()) {
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
  m_useNgrt4ndField.setText( Q_TR("Use ngrt4nd") );
  m_dontVerifyCertificateField.setText( Q_TR("Don't verify SSL certificate") );

  // update interval field
  m_updateIntervalField.setMinimum(5);
  m_updateIntervalField.setMaximum(1200);
  m_updateIntervalField.setValue(Preferences::updateInterval());

  m_authStringField.setEchoMode(Wt::WLineEdit::Password);
  m_authStringField.setEmptyText( Q_TR("Set the authentication string") );
  m_showAuthStringField.setText( Q_TR("Show in clear") );


  // buttons
  m_applyChangeBtn.setText( Q_TR("Apply changes") );
  m_addAsSourceBtn.setText( Q_TR("Add as source") );
  m_deleteSourceBtn.setText( Q_TR("Delete source") );
  m_applyChangeBtn.setStyleClass("btn btn-success");
  m_addAsSourceBtn.setStyleClass("btn btn-info");
  m_deleteSourceBtn.setStyleClass("btn btn-danger");
}


void WebDataSourcePreferences::bindFormWidgets(void)
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
  bindWidget("use-ngrt4nd", &m_useNgrt4ndField);
  bindWidget("apply-change-button", &m_applyChangeBtn);
  bindWidget("add-as-source-button", &m_addAsSourceBtn);
  bindWidget("delete-button", &m_deleteSourceBtn);
}


void WebDataSourcePreferences::unbindFormWidgets(void)
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


void WebDataSourcePreferences::addEvent(void)
{
  m_applyChangeBtn.clicked().connect(this, &WebDataSourcePreferences::applyChanges);
  m_addAsSourceBtn.clicked().connect(this, &WebDataSourcePreferences::addAsSource);
  m_deleteSourceBtn.clicked().connect(this, &WebDataSourcePreferences::deleteSource);
  m_monitorTypeField.activated().connect(this, &WebDataSourcePreferences::showLivestatusSettings);
  m_sourceSelectionBox.changed().connect(this, &WebDataSourcePreferences::handleSourceBoxChanged);
  m_showAuthStringField.changed().connect(this, &WebDataSourcePreferences::handleShowAuthStringChanged);
}



bool WebDataSourcePreferences::validateSourceSettingsFields(void)
{
  if (m_monitorTypeField.currentIndex() == 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Monitor type not set"));
    return false;
  }
  if (m_monitorTypeField.currentIndex() > 1 && m_monitorUrlField.validate() != Wt::WValidator::Valid) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Please fix field(s) in red"));
    return false;
  }
  return true;
}


void WebDataSourcePreferences::applyChanges(void)
{
  if (! validateSourceSettingsFields())
    return ;

  if ( m_monitorTypeField.currentIndex() <= 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Invalid monitor type"));
    return;
  }

  if (currentSourceIndex() < 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Invalid data source index (%1)").arg(currentSourceIndex()).toStdString());
    return;
  }

  saveAsSource(currentSourceIndex(), m_monitorTypeField.currentText().toUTF8().c_str());
}



void WebDataSourcePreferences::addAsSource(void)
{
  if (validateSourceSettingsFields())
    m_sourceIndexSelector.show();
}



void WebDataSourcePreferences::deleteSource(void)
{
  int curIndex = currentSourceIndex();
  if (curIndex >= 0 && curIndex < MAX_SRCS) {
    m_sourceBoxModel.removeRow(currentSourceIndex());
    setSourceState(currentSourceIndex(), false);
    setKeyValue(Settings::GLOBAL_SRC_BUCKET_KEY, sourceStatesSerialized());
    sync();
    updateFields();
  }
}


void WebDataSourcePreferences::updateAllSourceWidgetStates(void)
{
  std::vector< Wt::WString > activeSourceLabels;
  std::vector< int > activeSourceIndexes;
  for (int index = 0; index< MAX_SRCS; ++index) {
    if ( isSetSource(index) ) {
      activeSourceLabels.push_back( ngrt4n::sourceId(index).toStdString() );
      activeSourceIndexes.push_back(index);
    }
  }

  m_sourceBoxModel.setStringList( activeSourceLabels );
  for (size_t i = 0; i < activeSourceLabels.size(); ++i) {
    m_sourceBoxModel.setData(i, 0, activeSourceIndexes[i], Wt::UserRole);
  }

  m_sourceBoxModel.sort(0);
}


void WebDataSourcePreferences::updateFields(void)
{
  setCurrentSourceIndex(firstSourceSet());
  int curIndex = currentSourceIndex();
  if (curIndex >= 0) {
    m_sourceSelectionBox.setCurrentIndex(curIndex);
    fillFromSource(curIndex);
  }
}


void WebDataSourcePreferences::saveAsSource(const qint32& index, const QString& type)
{
  // global settings
  setKeyValue(Settings::GLOBAL_UPDATE_INTERVAL_KEY, m_updateIntervalField.text().toUTF8().c_str());

  // source-specific settings
  SourceT src;
  src.id = ngrt4n::sourceId(index);
  src.mon_type = ngrt4n::convertToSourceType(type);
  src.mon_url = m_monitorUrlField.text().toUTF8().c_str();
  src.ls_addr = m_livestatusHostField.text().toUTF8().c_str();
  src.ls_port = QString( m_livestatusPortField.text().toUTF8().c_str() ).toInt();
  src.auth = QString( m_authStringField.text().toUTF8().c_str() );
  src.use_ngrt4nd = m_useNgrt4ndField.checkState();
  src.verify_ssl_peer = (m_dontVerifyCertificateField.checkState() == Wt::Checked);
  setKeyValue(ngrt4n::sourceKey(index), ngrt4n::sourceData2Json(src));
  setSourceState(index, true);
  setKeyValue(Settings::GLOBAL_SRC_BUCKET_KEY, sourceStatesSerialized());

  // save changes
  sync();

  // emit signal a finilize
  emitTimerIntervalChanged(1000 * QString(m_updateIntervalField.text().toUTF8().c_str()).toInt());
  addToSourceBox(index);
  m_sourceSelectionBox.setCurrentIndex(findSourceIndexInBox(index));
}


void WebDataSourcePreferences::fillFromSource(int _index)
{
  if (_index >= 0 && _index < MAX_SRCS) {
    SourceT src;
    loadSource(_index, src);

    m_sourceSelectionBox.setValueText(ngrt4n::sourceId(_index).toStdString());
    m_monitorUrlField.setText(src.mon_url.toStdString());
    m_livestatusHostField.setText(src.ls_addr.toStdString());
    m_livestatusPortField.setText(QString::number(src.ls_port).toStdString());
    m_authStringField.setText(src.auth.toStdString());
    m_monitorTypeField.setCurrentIndex(src.mon_type+1);
    m_useNgrt4ndField.setCheckState(static_cast<Wt::CheckState>(src.use_ngrt4nd));
    m_dontVerifyCertificateField.setCheckState(src.verify_ssl_peer? Wt::Checked : Wt::Unchecked);
    m_updateIntervalField.setValue(updateInterval());

    showLivestatusSettings(m_monitorTypeField.currentIndex());

    // this triggers a signal
    setCurrentSourceIndex(_index);
  }
}


void WebDataSourcePreferences::renderSourceIndexSelector(void)
{
  m_sourceSelectionBox.setModel(&m_sourceBoxModel);
  m_sourceIndexSelector.rejectWhenEscapePressed();

  m_sourceIndexSelector.setStyleClass("Wt-dialog");
  m_sourceIndexSelector.titleBar()->setStyleClass("titlebar");
  m_sourceIndexSelector.setWindowTitle(Q_TR("Select the source index"));

  //FIXME: memory leak ?
  Wt::WComboBox* inputField = new Wt::WComboBox(m_sourceIndexSelector.contents());
  for (const auto& src : ngrt4n::sourceIndexes()) inputField->addItem(src.toStdString());

  //FIXME: memory leak ?
  Wt::WPushButton *ok = new Wt::WPushButton("OK", m_sourceIndexSelector.footer());
  ok->clicked().connect(std::bind(&WebDataSourcePreferences::handleAddAsSourceOkAction, this, inputField));
  ok->setDefault(true);

  //FIXME: memory leak ?
  Wt::WPushButton *cancel = new Wt::WPushButton("Cancel", m_sourceIndexSelector.footer());
  cancel->clicked().connect(&m_sourceIndexSelector, &Wt::WDialog::reject);
}



void WebDataSourcePreferences::handleAddAsSourceOkAction(Wt::WComboBox* inputBox)
{
  m_sourceIndexSelector.accept();
  int maxAllowedViews = LicenseActivationBase(PKG_VERSION).maxAllowedSources();
  if (activeSourcesCount() < maxAllowedViews) {
    bool isValidIndex;
    int index = QString::fromStdString( inputBox->currentText().toUTF8() ).toInt(&isValidIndex);
    if (isValidIndex) {
      setCurrentSourceIndex(index);
    } else {
      setCurrentSourceIndex(-1);
    }
    applyChanges();
  } else {
    m_operationCompleted.emit(ngrt4n::OperationFailed,
                              QObject::tr("Can't add new data source. Your license allows a maximum of %1 data source(s)")
                              .arg(QString::number(maxAllowedViews)).toStdString());
  }
}



int WebDataSourcePreferences::getSourceGlobalIndex(int sourceBoxIndex)
{
  boost::any value =
      static_cast<Wt::WAbstractItemModel*>(
        &m_sourceBoxModel)->data(sourceBoxIndex, 0, Wt::UserRole);

  return boost::any_cast<int>(value);
}


int WebDataSourcePreferences::findSourceIndexInBox(int sourceGlobalIndex)
{
  int index = m_sourceBoxModel.rowCount() - 1;
  while (index >= 0 && (getSourceGlobalIndex(index) != sourceGlobalIndex)) --index;
  return index;
}


void WebDataSourcePreferences::addToSourceBox(int sourceGlobalIndex)
{
  int index = findSourceIndexInBox(sourceGlobalIndex);
  if (index < 0) {
    m_sourceBoxModel.addString(ngrt4n::sourceId(sourceGlobalIndex).toStdString());
    m_sourceBoxModel.setData(m_sourceBoxModel.rowCount() - 1, 0, sourceGlobalIndex, Wt::UserRole);
  }
  m_sourceBoxModel.sort(0);
}


void WebDataSourcePreferences::setEnabledInputs(bool enable)
{
  m_monitorUrlField.setEnabled(enable);
  m_authStringField.setEnabled(enable);
  m_livestatusHostField.setEnabled(enable);
  m_livestatusPortField.setEnabled(enable);
  m_monitorTypeField.setEnabled(enable);
  m_showAuthStringField.setEnabled(enable);
  m_useNgrt4ndField.setEnabled(enable);
  m_dontVerifyCertificateField.setEnabled(enable);
  m_updateIntervalField.setEnabled(enable);
  m_applyChangeBtn.setEnabled(enable);
  m_addAsSourceBtn.setEnabled(enable);
  m_deleteSourceBtn.setEnabled(enable);
}



void WebDataSourcePreferences::handleSourceBoxChanged(void)
{
  fillFromSource(getSourceGlobalIndex(m_sourceSelectionBox.currentIndex()));
}


void WebDataSourcePreferences::showLivestatusSettings(int monitorTypeIndex)
{
  switch (monitorTypeIndex) {
    case 1:
      wApp->doJavaScript("$('#livetstatus-section').show();");
      break;
    default:
      wApp->doJavaScript("$('#livetstatus-section').hide();");
      break;
  }
}


void WebDataSourcePreferences::handleShowAuthStringChanged(void)
{
  if (m_showAuthStringField.isChecked()) {
    m_authStringField.setEchoMode(Wt::WLineEdit::Normal);
  } else {
    m_authStringField.setEchoMode(Wt::WLineEdit::Password);
  }
}
