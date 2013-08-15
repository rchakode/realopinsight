/*
 * Preferences.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)        #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */


#include "core/MonitorBroker.hpp"
#include "Preferences.hpp"
#include "Auth.hpp"
#include "Base.hpp"
#include "utilsClient.hpp"
#include "JsHelper.hpp"
#include <sstream>
#include <QWebView>
#include <QIntValidator>
#include <QRegExpValidator>

const qint32 Preferences::ChangePassword = 0;
const qint32 Preferences::ForceChangePassword = 1;
const qint32 Preferences::ChangeMonitoringSettings = 2;
const qint32 Preferences::ShowHelp = 3;
const qint32 Preferences::ShowAbout = 4;
const QString Preferences::UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";
const QString Preferences::ADM_UNSERNAME_KEY = "/Auth/admUser";
const QString Preferences::OP_UNSERNAME_KEY = "/Auth/opUsername";
const QString Preferences::ADM_PASSWD_KEY = "/Auth/admPasswd";
const QString Preferences::OP_PASSWD_KEY = "/Auth/opPasswd";
const QString Preferences::SRC_BUCKET_KEY = "/Sources/buckets";
const qint32 Preferences::MAX_SRCS = 10;
const QStringList Preferences::SRC_TYPES = QStringList() << "Livestatus/ngrt4nd"
                                                         << "Zabbix"
                                                         << "Zenoss";

Preferences::Preferences(const qint32& _userRole, const qint32& _action)
  : QDialog(),
    m_mainLayout (new QGridLayout(this)),
    m_userRole(_userRole),
    m_settings(new Settings()),
    m_onitorUrlField(new QLineEdit()),
    m_updateIntervalField(new QSpinBox()),
    m_brwBtn(new QPushButton(tr("&Browse..."))),
    m_oldPwdField(new QLineEdit()),
    m_pwdField(new QLineEdit()),
    m_rePwdField(new QLineEdit()),
    m_sockAddrField (new QLineEdit()),
    m_sockPortField(new QLineEdit()),
    m_serverPassField(new QLineEdit()),
    m_cancelBtn(new QPushButton(tr("&Close"))),
    m_applySettingBtn(new QPushButton(tr("&Apply settings"))),
    m_addAsSourceBtn(new QPushButton(tr("Add as &Source"))),
    m_changePwdBtn(new QPushButton(tr("C&hange password"))),
    m_donateBtn(new ImageButton(":images/built-in/donate.png")),
    m_showAuthInfoChkbx(new QCheckBox(tr("&Show in clear"))),
    m_useMklsChkbx(new QCheckBox(tr("Use&Livestatus"))),
    m_sourceBuckets(new QBitArray())
{
  qint32 line = -1;
  m_oldPwdField->setEchoMode(QLineEdit::Password);
  m_pwdField->setEchoMode(QLineEdit::Password);
  m_rePwdField->setEchoMode(QLineEdit::Password);
  m_serverPassField->setEchoMode(QLineEdit::Password);
  m_sockPortField->setValidator(new QIntValidator(1, 65535, m_sockPortField));
  switch (_action)
    {
    case Preferences::ChangeMonitoringSettings:
      setWindowTitle(tr("Monitoring Settings | %1").arg(APP_NAME));
      line++,
          m_mainLayout->addWidget(createCommonGrp(), line, 0, 1, 3);
      line++,
          m_mainLayout->addWidget(createScktGrp(), line, 0, 1, 3);
      line++,
          m_mainLayout->addWidget(m_cancelBtn, line, 0, Qt::AlignLeft),
          m_mainLayout->addWidget(m_applySettingBtn, line, 1, Qt::AlignRight),
          m_mainLayout->addWidget(m_addAsSourceBtn, line, 2, Qt::AlignRight);
      line++,
          m_mainLayout->addWidget(new QLabel(tr("(*) Required for Zabbix and Zenoss.")), line, 0, 1, 3);
      m_mainLayout->setColumnStretch(0, 0);
      m_mainLayout->setColumnStretch(1, 6);
      m_mainLayout->setColumnStretch(2, 0);
      if(_userRole == Auth::OpUserRole) {
          m_onitorUrlField->setEnabled(false);
          m_brwBtn->setEnabled(false);
          m_updateIntervalField->setEnabled(false);
          m_applySettingBtn->setEnabled(false);
          m_addAsSourceBtn->setEnabled(false);
          m_sockAddrField->setEnabled(false);
          m_sockPortField->setEnabled(false);
          m_serverPassField->setEnabled(false);
          m_showAuthInfoChkbx->setEnabled(false);
          m_useMklsChkbx->setEnabled(false);
        }
      break;
    case Preferences::ChangePassword:
    case Preferences::ForceChangePassword:
      setWindowTitle(tr("Change Password | %1").arg(APP_NAME));
      line++,
          m_mainLayout->addWidget(new QLabel(tr("Current Password")), line, 0),
          m_mainLayout->addWidget(m_oldPwdField, line, 1, 1, 2);
      line++,
          m_mainLayout->addWidget(new QLabel(tr("New password")), line, 0),
          m_mainLayout->addWidget(m_pwdField, line, 1, 1, 2);
      line++,
          m_mainLayout->addWidget(new QLabel(tr("Retype new password")), line, 0),
          m_mainLayout->addWidget(m_rePwdField, line, 1, 1, 2);
      line++,
          m_mainLayout->addWidget(m_cancelBtn, line, 1),
          m_mainLayout->addWidget(m_changePwdBtn, line, 2);

      if(_action == Preferences::ForceChangePassword) {
          m_cancelBtn->setEnabled(false);
        }
      break;
    case Preferences::ShowAbout:
      setWindowTitle(tr("About %1").arg(APP_NAME));
      QString about = QObject::tr("\n%1 %2 (codename: %3)\n"
                                  "\nRelease ID: %4\n"
                                  "\nCopyright (c) 2010-%5 NGRT4N Project. All rights reserved"
                                  "\nVisit %6 for more information\n"
                                  "\nReport Bugs: bugs@ngrt4n.com\n").arg(APP_NAME, PKG_VERSION, RELEASE_NAME, REL_INFO, REL_YEAR, PKG_URL);

      line++,
          m_mainLayout->addWidget(new QLabel(about), line, 0, 1, 2);
      line++,
          m_mainLayout->addWidget(m_donateBtn, line, 0, 1, 1, Qt::AlignLeft),
          m_mainLayout->addWidget(m_cancelBtn, line, 1, 1, 1, Qt::AlignRight);
      break;
    }
  loadProperties();
  addEvents();
}

Preferences::~Preferences()
{
  delete m_updateIntervalField;
  delete m_oldPwdField;
  delete m_pwdField;
  delete m_rePwdField;
  delete m_changePwdBtn;
  delete m_cancelBtn;
  delete m_applySettingBtn;
  delete m_addAsSourceBtn;
  delete m_sockAddrField;
  delete m_sockPortField;
  delete m_serverPassField;
  delete m_donateBtn;
  delete m_showAuthInfoChkbx;
  delete m_useMklsChkbx;
  delete m_mainLayout;
  delete m_sourceBuckets;
}


void Preferences::showEvent (QShowEvent *)
{
  m_oldPwdField->setText("");
  m_pwdField->setText("");
  m_rePwdField->setText("");
}

void Preferences::applySettings(void)
{
  saveAsSource(0);
  m_sourceBuckets->setBit(0, true);
}

void Preferences::addAsSource(void)
{
  int bucket = 1;
  while (bucket < MAX_SRCS && m_sourceBuckets->at(bucket)) { bucket++; }
  if (bucket < MAX_SRCS) {
      bool ok;
      QString selected = QInputDialog::getItem(this,
                                               tr("Select the type of the Remote API | %1").arg(APP_NAME),
                                               tr("Please select the type of the remote API"),
                                               SRC_TYPES, 0,
                                               false,
                                               &ok);
      if (ok && !selected.isEmpty()) {
          saveAsSource(bucket, selected);
        } else {
          utils::alert(tr("Failed. No source selected."));
        }
    } else {
      utils::alert("The maximum number of sources is reached");
    }
}

void Preferences::saveAsSource(const qint32& _idx, const QString& _stype)
{
  SourceT src;
  src.id = utils::sourceId(_idx);
  src.mon_type = utils::convert2ApiType(_stype);
  src.mon_url = m_onitorUrlField->text();
  src.ls_addr = m_sockAddrField->text();
  src.ls_port = m_sockPortField->text().toInt();
  src.auth = m_serverPassField->text();
  src.use_ls = m_useMklsChkbx->checkState();
  m_settings->setValue(utils::sourceKey(_idx), utils::source2Str(src));
  m_settings->setValue(UPDATE_INTERVAL_KEY, m_updateIntervalField->text());
  m_sourceBuckets->setBit(_idx, true);
  m_settings->setValue(SRC_BUCKET_KEY, getSourceBucketsserialized());
  m_settings->sync();
  if (_idx == 0) {
      close();
      emit urlChanged(src.mon_url);
    }
}

void Preferences::changePasswd(void)
{
  QString userPasswd;
  QString passwd;
  QString newPasswd;
  QString renewPasswd;
  QString key;

  if (m_userRole == Auth::AdmUserRole) {
      key = ADM_PASSWD_KEY;
      userPasswd = m_settings->value(key, Auth::AdmUser).toString();
    } else {
      key = OP_PASSWD_KEY;
      userPasswd = m_settings->value(key, Auth::OpUser).toString();
    }
  passwd = QCryptographicHash::hash(m_oldPwdField->text().toAscii(), QCryptographicHash::Md5);
  newPasswd = QCryptographicHash::hash(m_pwdField->text().toAscii(), QCryptographicHash::Md5);
  renewPasswd = QCryptographicHash::hash(m_rePwdField->text().toAscii(), QCryptographicHash::Md5);

  if(userPasswd == passwd) {
      if(newPasswd == renewPasswd) {
          m_settings->setKeyValue(key, newPasswd);
          QMessageBox::information(this,
                                   APP_NAME,
                                   tr("Password updated"),
                                   QMessageBox::Ok);

          done(0);
        } else {
          utils::alert(tr("Sorry the passwords do not match"));
        }
    } else {
      utils::alert(tr("Authentication failed"));
    }
}

void Preferences::donate(void)
{
  QDesktopServices appLauncher;
  appLauncher.openUrl(QUrl("http://realopinsight.com/en/index.php?page=contribute"));
}


void Preferences::setAuthChainVisibility(const int& state) {
  if(state == Qt::Checked) {
      m_serverPassField->setEchoMode(QLineEdit::Normal);
    } else {
      m_serverPassField->setEchoMode(QLineEdit::Password);
    }
}

QGroupBox* Preferences::createScktGrp(void)
{
  QGroupBox* bx(new QGroupBox(tr("Livestatus/Ngrt4nd Endpoint")));
  QHBoxLayout* lyt(new QHBoxLayout());
  lyt->addWidget(new QLabel(tr("Server Address")));
  lyt->addWidget(m_sockAddrField);
  lyt->addWidget(new QLabel(tr("Port")), Qt::AlignRight);
  lyt->addWidget(m_sockPortField);
  lyt->addWidget(m_useMklsChkbx);
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

QGroupBox* Preferences::createCommonGrp(void)
{
  QGroupBox* bx(new QGroupBox(tr("Common Settings")));
  QGridLayout* lyt(new QGridLayout());
  int line;
  line = 0,
      lyt->addWidget(new QLabel(tr("Monitor Web URL*")), line, 0),
      lyt->addWidget(m_onitorUrlField, line, 1, 1, 2);
  line++,
      lyt->addWidget(new QLabel(tr("Auth String")), line, 0),
      lyt->addWidget(m_serverPassField, line, 1),
      lyt->addWidget(m_showAuthInfoChkbx, line, 2);
  line++,
      lyt->addWidget(new QLabel(tr("Update Interval")), line, 0),
      lyt->addWidget(m_updateIntervalField, line, 1),
      lyt->addWidget(new QLabel(tr("seconds")), line, 2);
  lyt->setColumnStretch(0, 0);
  lyt->setColumnStretch(1, 1);
  bx->setFlat(false);
  bx->setLayout(lyt);
  bx->setAlignment(Qt::AlignLeft);
  return bx;
}

void Preferences::loadProperties(void)
{
  m_updateIntervalField->setMinimum(5);
  m_updateIntervalField->setMaximum(600);
  m_updateIntervalField->setValue(m_settings->value(UPDATE_INTERVAL_KEY, MonitorBroker::DefaultUpdateInterval).toInt());

  loadSourceBuckets();
  SourceT src;
  m_settings->loadSource(0, src);
  m_onitorUrlField->setText(src.mon_url);
  m_sockAddrField->setText(src.ls_addr);
  m_sockPortField->setText(QString::number(src.ls_port));
  m_serverPassField->setText(src.auth);
  m_useMkls = static_cast<Qt::CheckState>(src.use_ls);
  m_useMklsChkbx->setCheckState(m_useMkls);
}

QString Preferences::style() {
  QString styleSheet =
      "QMenuBar, QMenu {"
      "	background: #2d2d2d; "
      "	color : white; "
      "}"
      "QMenuBar {"
      "	font-weight : bold;"
      "}"
      "QToolBar {"
      "	background: #9dc6dd;"
      "	padding: 0px;"
      "	height: 16px;"
      "	"
      "}"
      "QHeaderView::section {"
      "	background: #2d2d2d;"
      "	color : white; "
      "	font-weight : bold;"
      "}"
      "QTabWidget::pane { /* The tab widget frame */"
      "	border-top: 2px solid #C2C7CB;"
      "	background-color: #9dc6dd;"
      "}"
      "QTabWidget::tab-bar {"
      "	background-color: #9dc6dd;"
      "}"
      "QTabBar::tab {"
      "	background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
      "	stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
      "	stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
      "	border: 2px solid #C4C4C3;"
      "	border-bottom-color: #C2C7CB; /* same as the pane color */"
      "	border-top-left-radius: 4px;"
      "	border-top-right-radius: 4px;"
      "	min-width: 8ex;"
      "	padding: 2px;"
      "}"
      "QTabBar::tab:selected {"
      "	background: #9dc6dd;"
      "}"
      "QTabBar::tab:hover {"
      "	background: #3589b9;"
      "}"
      "QTabBar::tab:selected {"
      "	border-color: #9B9B9B;"
      "	border-bottom-color: #C2C7CB; /* same as pane color */"
      "}"
      "QTabBar::tab:!selected {"
      "	margin-top: 2px; /* make non-selected tabs look smaller */"
      "}"
      " QTreeView {"
      "	background: #F8F8FF;"
      "   alternate-background-color: yellow;"
      "}"
      "QTableView {"
      "	background: #F8F8FF;" // #f1f1f1
      "	alternate-background-color: #F8F8FF;"
      "}"
      "QToolTip {"
      "	border: 2px solid darkkhaki;"
      "	padding: 2px;"
      "	border-radius: 3px;"
      "	opacity: 500;"
      "	color : #000000;"
      " font: italic large \"Times New Roman\";"
      "}"
      "QDialog {"
      "	background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
      "		stop: 0 #9DC6DD, stop: 0.25 #F1F1F1,"
      "		stop: 0.4 #FFBB69, stop: 0.55 #F1F1F1, stop: 1.0 #9DC6DD);"
      "}"
      "QGraphicsView {"
      "	background: #F8F8FF;"
      "}"
      ;
  return styleSheet;
}

QString Preferences::getSourceBucketsserialized(void)
{
  QString str = "";
  for (int i = 0; i < MAX_SRCS; i++) str += m_sourceBuckets->at(i)? "1" : "0";
  return str;
}

void Preferences::loadSourceBuckets()
{
  QString str = m_settings->value(SRC_BUCKET_KEY).toString();
  setSourceBuckets(str);
}

void Preferences::setSourceBuckets(const QString& str)
{
  if (!str.isEmpty()) {
      for (int i = 0; i < MAX_SRCS; i++) m_sourceBuckets->setBit(i, str.at(i).digitValue());
    } else {
      m_sourceBuckets->setBit(0, true);
      for (int i=1; i < MAX_SRCS; i++) m_sourceBuckets->setBit(i, false);
    }
}

void Preferences::addEvents(void)
{
  connect(m_applySettingBtn, SIGNAL(clicked()),  this, SLOT(applySettings()));
  connect(m_addAsSourceBtn, SIGNAL(clicked()),  this, SLOT(addAsSource()));
  connect(m_cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_changePwdBtn, SIGNAL(clicked()),  this, SLOT(changePasswd()));
  connect(m_donateBtn, SIGNAL(clicked()),  this, SLOT(donate()));
  connect(m_showAuthInfoChkbx, SIGNAL(stateChanged(int)), this, SLOT(setAuthChainVisibility(int)));
}
