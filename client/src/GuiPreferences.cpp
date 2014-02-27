
#include "GuiPreferences.hpp"
#include "utilsClient.hpp"

GuiPreferences::GuiPreferences(qint32 _userRole, qint32 _formType)
  : Preferences(),
    m_dialog(new QDialog()),
    m_mainLayout(new QGridLayout(m_dialog)),
    m_userRole(_userRole),
    m_formType(_formType),
    m_cancelBtn(new QPushButton(tr("&Close"))),
    m_cancelled(false)
{
  switch (_formType)
  {
  case Preferences::ChangeMonitoringSettings:
    createPreferenceWindow();
    break;
  case Preferences::ChangePassword:
  case Preferences::ForceChangePassword:
    createChangePasswordForm();
    break;
  case Preferences::ShowAbout:
    organizeAbortWindow();
    break;
  case Preferences::BasicLoginForm:
    loadBasicLoginForm();
    break;
  case WebForm:
    break;
  default:
    break;
  }
  addEvents();
}


GuiPreferences::~GuiPreferences()
{
  switch(m_formType) {
  case ChangeMonitoringSettings:
    delete m_sourceStates;
    delete m_monitorTypeField;
    delete m_monitorUrlField;
    delete m_updateIntervalField;
    delete m_applySettingBtn;
    delete m_addAsSourceBtn;
    delete m_deleteSourceBtn;
    delete m_sockAddrField;
    delete m_sockPortField;
    delete m_serverPassField;
    delete m_showAuthInfoChkbx;
    delete m_useNgrt4ndChkbx;
    delete m_verifySslPeerChkBx;
    break;
  case ChangePassword:
    delete m_oldPwdField;
    delete m_pwdField;
    delete m_rePwdField;
    delete m_changePwdBtn;
    break;
  case BasicLoginForm:
    delete m_realmLoginField;
    delete m_realmPasswdField;
    break;
  case ShowAbout:
    delete m_donateBtn;
    break;
  default:
    break;
  }
  delete m_cancelBtn;
  delete m_mainLayout;
  delete m_dialog;
}



void GuiPreferences::show(void)
{
  switch(m_formType) {
  case ChangePassword:
    m_oldPwdField->setText("");
    m_pwdField->setText("");
    m_rePwdField->setText("");
    break;
  case ChangeMonitoringSettings:
    break;
  case BasicLoginForm:
    break;
  case ShowAbout:
    break;
  default:
    break;
  }
  m_dialog->show();
}


QString GuiPreferences::letUserSelectType(void)
{
  if (m_monitorTypeField->currentIndex() > 0) {
    return m_monitorTypeField->currentText();
  }

  bool ok = false;
  QString srcType = QInputDialog::getItem(m_dialog,
                                          tr("Select the source type | %1").arg(APP_NAME),
                                          tr("Please select the source type (Remote API)"),
                                          utils::sourceTypes(),
                                          0,
                                          false,
                                          &ok);
  if (!ok || srcType.isEmpty()) {
    srcType.clear();
  }

  return srcType;
}



void GuiPreferences::updateSourceBtnState(void)
{
  int size = m_sourceBtns.size();
  for (int i=0; i < size; ++i) {
    m_sourceBtns.at(i)->setEnabled(m_sourceStates->at(i));
  }
}



void GuiPreferences::handleCancel(void)
{
  switch(m_formType) {
  case ChangeMonitoringSettings:
    Q_EMIT sourcesChanged(m_updatedSources);
    break;
  case BasicLoginForm:
    break;
  default:
    break;
  }
  m_cancelled = true;
  m_dialog->done(0);
}

void GuiPreferences::createPreferenceWindow(void)
{
  m_dialog->setWindowTitle(tr("Monitoring Settings | %1").arg(APP_NAME));

  m_monitorUrlField = new QLineEdit();
  m_monitorTypeField = new QComboBox();
  m_updateIntervalField = new QSpinBox();
  m_sockAddrField = new QLineEdit();
  m_sockPortField = new QLineEdit();
  m_serverPassField = new QLineEdit();
  m_applySettingBtn = new QPushButton(tr("&Apply settings"));
  m_addAsSourceBtn = new QPushButton(tr("Add a&s Source"));
  m_deleteSourceBtn = new QPushButton(tr("&Delete Source"));
  m_showAuthInfoChkbx = new QCheckBox(tr("&Show in clear"));
  m_useNgrt4ndChkbx = new QCheckBox(tr("Use &Ngrt4nd"));
  m_verifySslPeerChkBx = new QCheckBox(tr("Don't verify SSL peer (https)"));
  m_serverPassField->setEchoMode(QLineEdit::Password);
  m_sockPortField->setValidator(new QIntValidator(1, 65535, m_sockPortField));

  qint32 line = 0;
  m_mainLayout->addWidget(createCommonGrp(), line, 0, 1, 3);
  m_mainLayout->addWidget(createScktGrp(), ++line, 0, 1, 3);
  m_mainLayout->addWidget(m_cancelBtn, ++line, 0, Qt::AlignLeft),
      m_mainLayout->addWidget(createUpdateBtnsGrp(), line, 1, 1, 2, Qt::AlignRight);
  m_mainLayout->addWidget(new QLabel(tr("(*) Required for Zabbix and Zenoss.")), ++line, 0, 1, 3);

  m_mainLayout->setColumnStretch(0, 0);
  m_mainLayout->setColumnStretch(1, 6);
  m_mainLayout->setColumnStretch(2, 0);

  loadProperties();

  disableFieldIfRequired();
}

void GuiPreferences::createChangePasswordForm(void)
{
  m_oldPwdField = new QLineEdit();
  m_pwdField = new QLineEdit();
  m_rePwdField = new QLineEdit();
  m_changePwdBtn = new QPushButton(tr("C&hange password"));
  m_oldPwdField->setEchoMode(QLineEdit::Password);
  m_pwdField->setEchoMode(QLineEdit::Password);
  m_rePwdField->setEchoMode(QLineEdit::Password);

  m_dialog->setWindowTitle(tr("Change Password | %1").arg(APP_NAME));

  int line = -1;

  ++line,
      m_mainLayout->addWidget(new QLabel(tr("Current Password")), line, 0),
      m_mainLayout->addWidget(m_oldPwdField, line, 1, 1, 2);
  ++line,
      m_mainLayout->addWidget(new QLabel(tr("New password")), line, 0),
      m_mainLayout->addWidget(m_pwdField, line, 1, 1, 2);
  ++line,
      m_mainLayout->addWidget(new QLabel(tr("Retype new password")), line, 0),
      m_mainLayout->addWidget(m_rePwdField, line, 1, 1, 2);
  ++line,
      m_mainLayout->addWidget(m_cancelBtn, line, 1),
      m_mainLayout->addWidget(m_changePwdBtn, line, 2);

  if(m_formType == Preferences::ForceChangePassword) {
    m_cancelBtn->setEnabled(false);
  }
}


void GuiPreferences::organizeAbortWindow(void)
{
  m_dialog->setWindowTitle(tr("About %1").arg(APP_NAME));
  m_donateBtn = new ImageButton(":images/built-in/donate.png");
  QString about = QObject::tr("\n%1 %2 (codename: %3)\n"
                              "\nRelease ID: %4\n"
                              "\nCopyright (c) 2010-%5 NGRT4N Project. All rights reserved"
                              "\nVisit %6 for more information\n"
                              "\nReport Bugs: bugs@ngrt4n.com\n").arg(APP_NAME, PKG_VERSION, REL_NAME, REL_INFO, REL_YEAR, PKG_URL);
  int line = 0;
  m_mainLayout->addWidget(new QLabel(about), line++, 0, 1, 2);
  m_mainLayout->addWidget(m_donateBtn, line, 0, 1, 1, Qt::AlignLeft);
  m_mainLayout->addWidget(m_cancelBtn, line, 1, 1, 1, Qt::AlignRight);
}


void GuiPreferences::disableFieldIfRequired(void)
{
  if(m_userRole == ngrt4n::OpUserRole) {
    m_monitorUrlField->setEnabled(false);
    m_monitorTypeField->setEnabled(false);
    m_updateIntervalField->setEnabled(false);
    m_applySettingBtn->setEnabled(false);
    m_addAsSourceBtn->setEnabled(false);
    m_deleteSourceBtn->setEnabled(false);
    m_sockAddrField->setEnabled(false);
    m_sockPortField->setEnabled(false);
    m_serverPassField->setEnabled(false);
    m_showAuthInfoChkbx->setEnabled(false);
    m_useNgrt4ndChkbx->setEnabled(false);
    m_verifySslPeerChkBx->setEnabled(false);
  }
}



QString GuiPreferences::style()
{
  QString styleSheet =
      "QMenuBar, QMenu {"
      " background: #2d2d2d; "
      " color : white; "
      "}"
      "QMenuBar {"
      " font-weight : bold;"
      "}"
      "QToolBar {"
      " background: #9dc6dd;"
      " padding: 0px;"
      " height: 16px;"
      " "
      "}"
      "QHeaderView::section {"
      " background: #2d2d2d;"
      " color : white; "
      " font-weight : bold;"
      "}"
      "QTabWidget::pane { /* The tab widget frame */"
      " border-top: 2px solid #C2C7CB;"
      " background-color: #9dc6dd;"
      "}"
      "QTabWidget::tab-bar {"
      " background-color: #9dc6dd;"
      "}"
      "QTabBar::tab {"
      " background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
      " stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
      " stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
      " border: 2px solid #C4C4C3;"
      " border-bottom-color: #C2C7CB; /* same as the pane color */"
      " border-top-left-radius: 4px;"
      " border-top-right-radius: 4px;"
      " min-width: 8ex;"
      " padding: 2px;"
      "}"
      "QTabBar::tab:selected {"
      " background: #9dc6dd;"
      "}"
      "QTabBar::tab:hover {"
      " background: #3589b9;"
      "}"
      "QTabBar::tab:selected {"
      " border-color: #9B9B9B;"
      " border-bottom-color: #C2C7CB; /* same as pane color */"
      "}"
      "QTabBar::tab:!selected {"
      " margin-top: 2px; /* make non-selected tabs look smaller */"
      "}"
      " QTreeView {"
      " background: #F8F8FF;"
      "   alternate-background-color: yellow;"
      "}"
      "QTableView {"
      " background: #F8F8FF;" // #f1f1f1
      " alternate-background-color: #F8F8FF;"
      "}"
      "QToolTip {"
      " border: 2px solid darkkhaki;"
      " padding: 2px;"
      " border-radius: 3px;"
      " opacity: 500;"
      " color : #000000;"
      " font: italic large \"Times New Roman\";"
      "}"
      "QDialog {"
      " background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
      "         stop: 0 #9DC6DD, stop: 0.25 #F1F1F1,"
      "         stop: 0.4 #FFBB69, stop: 0.55 #F1F1F1, stop: 1.0 #9DC6DD);"
      "}"
      "QGraphicsView {"
      " background: #F8F8FF;"
      "}"
      ;
  return styleSheet;
}


void GuiPreferences::loadBasicLoginForm(void)
{
  m_dialog->setWindowTitle(tr("Browser requires realm authentication | %1").arg(APP_NAME));
  int line;
  line = 0,
      m_mainLayout->addWidget(new QLabel(tr("Login")), line, 0),
      m_realmLoginField = new QLineEdit(),
      m_mainLayout->addWidget(m_realmLoginField, line, 1);
  ++line,
      m_mainLayout->addWidget(new QLabel(tr("Password")), line, 0),
      m_realmPasswdField = new QLineEdit(),
      m_realmPasswdField->setEchoMode(QLineEdit::Password),
      m_mainLayout->addWidget(m_realmPasswdField, line, 1);
  ++line,
      m_applySettingBtn->setText(tr("Submit")),
      m_mainLayout->addWidget(m_applySettingBtn, line, 0, Qt::AlignRight),
      m_mainLayout->addWidget(m_cancelBtn, line, 1, Qt::AlignRight);
}


void GuiPreferences::handleSourceSelected()
{
  QVector<QRadioButton*>::iterator cur = m_sourceBtns.begin();
  QVector<QRadioButton*>::iterator end = m_sourceBtns.end();
  int idx = 0;
  while (cur != end && !(*cur)->isChecked()) { ++cur; ++idx;}

  if(cur != end) {
    fillFromSource(idx);
  }
}



void GuiPreferences::updateFields(void)
{
  m_currentSourceIndex = firstSourceSet();
  if (m_currentSourceIndex >= 0) {
    m_sourceBtns.at(m_currentSourceIndex)->click();
  } else {
    // Set default value
    m_monitorUrlField->setText("http://localhost/monitor/");
    m_sockAddrField->setText("localhost");
    m_sockPortField->setText("1983");
    m_serverPassField->setText("secret");
    m_monitorTypeField->setCurrentIndex(0);
    m_useNgrt4ndChkbx->setCheckState(Qt::Unchecked);
    m_verifySslPeerChkBx->setCheckState(Qt::Unchecked);
    m_updateIntervalField->setValue(m_settings->updateInterval());
  }
}



void GuiPreferences::setAuthChainVisibility(const int& state) {
  if(state == Qt::Checked) {
    m_serverPassField->setEchoMode(QLineEdit::Normal);
  } else {
    m_serverPassField->setEchoMode(QLineEdit::Password);
  }
}


void GuiPreferences::changePasswd(void)
{
  QString userPasswd;
  QString passwd;
  QString newPasswd;
  QString renewPasswd;
  QString key;

  if (m_userRole == ngrt4n::AdmUserRole) {
    key = Settings::ADM_PASSWD_KEY;
    userPasswd = m_settings->value(key,
                                   QString::fromStdString(ngrt4n::AdmUser)).toString();
  } else {
    key = Settings::OP_PASSWD_KEY;
    userPasswd = m_settings->value(key,
                                   QString::fromStdString(ngrt4n::OpUser)).toString();
  }
  passwd = QCryptographicHash::hash(m_oldPwdField->text().toAscii(), QCryptographicHash::Md5);
  newPasswd = QCryptographicHash::hash(m_pwdField->text().toAscii(), QCryptographicHash::Md5);
  renewPasswd = QCryptographicHash::hash(m_rePwdField->text().toAscii(), QCryptographicHash::Md5);

  if (userPasswd == passwd) {
    if(newPasswd == renewPasswd) {
      m_settings->setKeyValue(key, newPasswd);
      QMessageBox::information(m_dialog,
                               APP_NAME,
                               tr("Password updated"),
                               QMessageBox::Ok);
      m_dialog->done(0);
    } else {
      Q_EMIT errorOccurred(tr("Sorry the passwords do not match"));
    }
  } else {
    Q_EMIT errorOccurred(tr("Authentication failed"));
  }
}


void GuiPreferences::handleDonate(void)
{
  QDesktopServices appLauncher;
  appLauncher.openUrl(QUrl("http://realopinsight.com/en/index.php/page/contribute"));
}

void GuiPreferences::fillFromSource(int _sidx)
{
  SourceT src;
  m_settings->loadSource(_sidx, src);
  m_monitorUrlField->setText(src.mon_url);
  m_sockAddrField->setText(src.ls_addr);
  m_sockPortField->setText(QString::number(src.ls_port));
  m_serverPassField->setText(src.auth);
  m_monitorTypeField->setCurrentIndex(src.mon_type+1);
  m_useNgrt4ndChkbx->setCheckState(static_cast<Qt::CheckState>(src.use_ngrt4nd));
  m_verifySslPeerChkBx->setCheckState(src.verify_ssl_peer? Qt::Unchecked : Qt::Checked);
  m_updateIntervalField->setValue(m_settings->updateInterval());

  m_currentSourceIndex = _sidx;
}


QGroupBox* GuiPreferences::createUpdateBtnsGrp(void)
{
  QGroupBox* bx(new QGroupBox(tr("Update Actions")));
  QHBoxLayout* lyt(new QHBoxLayout());
  lyt->addWidget(m_applySettingBtn);
  lyt->addWidget(m_addAsSourceBtn);
  lyt->addWidget(m_deleteSourceBtn);
  bx->setLayout(lyt);
  return bx;
}



void GuiPreferences::saveAsSource(const qint32& index, const QString& type)
{
  SourceT src;
  src.id = utils::sourceId(index);
  src.mon_type = utils::convert2ApiType(type);
  src.mon_url = m_monitorUrlField->text();
  src.ls_addr = m_sockAddrField->text();
  src.ls_port = m_sockPortField->text().toInt();
  src.auth = m_serverPassField->text();
  src.use_ngrt4nd = m_useNgrt4ndChkbx->checkState();
  src.verify_ssl_peer = (m_verifySslPeerChkBx->checkState() == Qt::Unchecked);
  m_settings->setEntry(utils::sourceKey(index), utils::source2Str(src));
  m_settings->setEntry(Settings::UPDATE_INTERVAL_KEY, m_updateIntervalField->text());
  m_sourceStates->setBit(index, true);
  m_settings->setEntry(Settings::SRC_BUCKET_KEY, getSourceStatesSerialized());
  m_settings->sync();
  m_settings->emitTimerIntervalChanged(1000 * m_updateIntervalField->text().toInt());

  if (! m_updatedSources.contains(index)) {
    //FIXME: consider only if source is used in the loaded service view?
    m_updatedSources.push_back(index);
  }

  m_currentSourceIndex = index;
  updateSourceBtnState();
  m_sourceBtns.at(m_currentSourceIndex)->click();
}


QGroupBox* GuiPreferences::createScktGrp(void)
{
  QGroupBox* bx(new QGroupBox(tr("Livestatus/Ngrt4nd Endpoint")));
  QHBoxLayout* lyt(new QHBoxLayout());
  lyt->addWidget(new QLabel(tr("Server Address")));
  lyt->addWidget(m_sockAddrField);
  lyt->addWidget(new QLabel(tr("Port")), Qt::AlignRight);
  lyt->addWidget(m_sockPortField);
  lyt->addWidget(m_useNgrt4ndChkbx);
  lyt->setStretch(0, 0);
  lyt->setStretch(1, 1);
  lyt->setStretch(2, 0);
  lyt->setStretch(3, 0);
  lyt->setStretch(4, 0);
  bx->setFlat(false);
  lyt->setMargin(0);
  bx->setLayout(lyt);
  bx->setAlignment(Qt::AlignLeft);
  return bx;
}

QGroupBox* GuiPreferences::createCommonGrp(void)
{
  QHBoxLayout* sourceBtnsLyt(new QHBoxLayout());
  sourceBtnsLyt = new QHBoxLayout();
  sourceBtnsLyt->setContentsMargins(0,0,0,0);
  sourceBtnsLyt->setMargin(0);
  for (int i=0; i<MAX_SRCS; ++i)
  {
    QRadioButton* btn(new QRadioButton(QString::number(i)));
    btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_sourceBtns.push_back(btn);
    connect(btn, SIGNAL(clicked()), this, SLOT(handleSourceSelected()));
    sourceBtnsLyt->addWidget(btn);
  }
  int line =0;
  QGridLayout* lyt(new QGridLayout());

  lyt->addWidget(new QLabel(tr("Sources")), line, 0),
      lyt->addLayout(sourceBtnsLyt, line, 1, 1, 1, Qt::AlignLeft);

  lyt->addWidget(new QLabel(tr("Monitor Web URL*")), ++line, 0),
      lyt->addWidget(m_monitorUrlField, line, 1),
      m_monitorTypeField->addItem(tr("Select a monitor type")),
      m_monitorTypeField->addItems(utils::sourceTypes()),
      lyt->addWidget(m_monitorTypeField, line, 2);

  lyt->addWidget(m_verifySslPeerChkBx, ++line, 0, 1, 3, Qt::AlignCenter);

  lyt->addWidget(new QLabel(tr("Auth String")), ++line, 0),
      lyt->addWidget(m_serverPassField, line, 1),
      lyt->addWidget(m_showAuthInfoChkbx, line, 2);

  lyt->addWidget(new QLabel(tr("Update Interval")), ++line, 0),
      m_updateIntervalField->setMinimum(5),
      m_updateIntervalField->setMaximum(1200),
      lyt->addWidget(m_updateIntervalField, line, 1),
      lyt->addWidget(new QLabel(tr("seconds")), line, 2);

  lyt->setColumnStretch(0, 0);
  lyt->setColumnStretch(1, 1);

  QGroupBox* bx(new QGroupBox(tr("Common Settings")));
  bx->setFlat(false);
  bx->setLayout(lyt);
  bx->setAlignment(Qt::AlignLeft);
  return bx;
}

void GuiPreferences::applyChanges(void)
{
  switch(m_formType) {
  case ChangeMonitoringSettings:
    saveAsSource(m_currentSourceIndex, letUserSelectType());
    break;
  case BasicLoginForm:
    m_dialog->done(0);
    break;
  default:
    break;
  }
}


void GuiPreferences::addAsSource(void)
{
  int bucket = -1;
  bool ok = false;
  QString srcId = QInputDialog::getItem(m_dialog,
                                        tr("Select source index | %1").arg(APP_NAME),
                                        tr("Please select the index of the source"),
                                        utils::sourceIndexes(),
                                        0,
                                        false,
                                        &ok);
  if (ok && ! srcId.isEmpty()) {
    bucket =  srcId.toInt();
  }

  if (bucket >= 0) {
    QString srcType = letUserSelectType();
    saveAsSource(bucket, srcType);
  }
}


void GuiPreferences::deleteSource(void)
{
  if (m_currentSourceIndex>=0 && m_currentSourceIndex < MAX_SRCS) {
    m_sourceBtns.at(m_currentSourceIndex)->setEnabled(false);
    m_sourceStates->setBit(m_currentSourceIndex, false);
    m_settings->setEntry(Settings::SRC_BUCKET_KEY, getSourceStatesSerialized());
    m_settings->sync();
    updateFields();
  }
}


void GuiPreferences::addEvents(void)
{
  connect(m_cancelBtn, SIGNAL(clicked()), this, SLOT(handleCancel()));

  switch(m_formType) {
  case ChangeMonitoringSettings:
    QObject::connect(m_applySettingBtn, SIGNAL(clicked()),  this, SLOT(applyChanges()));
    connect(m_addAsSourceBtn, SIGNAL(clicked()),  this, SLOT(addAsSource()));
    connect(m_deleteSourceBtn, SIGNAL(clicked()),  this, SLOT(deleteSource()));
    connect(m_showAuthInfoChkbx, SIGNAL(stateChanged(int)), this, SLOT(setAuthChainVisibility(int)));
    break;
  case ChangePassword:
    connect(m_changePwdBtn, SIGNAL(clicked()),  this, SLOT(changePasswd()));
    break;
  case ShowAbout:
    connect(m_donateBtn, SIGNAL(clicked()),  this, SLOT(handleDonate()));
    break;
  case BasicLoginForm:
    break;
  default:
    break;
  }
}