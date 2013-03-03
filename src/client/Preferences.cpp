/*
 * Preferences.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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
#include <sstream>
#include "Base.hpp"
#include "utilsClient.hpp"
#include <QWebView>

const qint32 Preferences::ChangePassword = 0;
const qint32 Preferences::ForceChangePassword = 1;
const qint32 Preferences::ChangeMonitoringSettings = 2;
const qint32 Preferences::ShowHelp = 3;
const qint32 Preferences::ShowAbout = 4;
const QString Preferences::URL_KEY = "/Monitor/nagiosHome";
const QString Preferences::UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";
const QString Preferences::SERVER_ADDR_KEY = "/Monitor/serverAddr";
const QString Preferences::SERVER_PORT_KEY = "/Monitor/serverPort";
const QString Preferences::USE_MKLS_KEY = "/Monitor/UseMkLs";
const QString Preferences::ADM_UNSERNAME_KEY = "/Auth/admUser";
const QString Preferences::OP_UNSERNAME_KEY = "/Auth/opUsername";
const QString Preferences::ADM_PASSWD_KEY = "/Auth/admPasswd";
const QString Preferences::OP_PASSWD_KEY = "/Auth/opPasswd";
const QString Preferences::SERVER_PASS_KEY = "/Auth/ServerAuthChain";


const QString AboutPattern = QObject::tr("\n%1 %2 (codename: %3)\n"
                                         "\nRelease ID: %4\n"
                                         "\nCopyright (c) 2010 NGRT4N Project. All rights reserved"
                                         "\nVisit %5 for more information\n"
                                         "\nReport Bugs: bugs@ngrt4n.com\n");

Preferences::Preferences(const qint32 & _userRole, const qint32 & _action)
  : QDialog(),
    muserRole(_userRole),
    msettings(new Settings()),
    monitorUrlField(new QLineEdit()),
    mupdateIntervalField(new QSpinBox()),
    mbrwBtn(new QPushButton(tr("&Browse..."))),
    moldPwdField(new QLineEdit()),
    mpwdField(new QLineEdit()),
    mrePwdField(new QLineEdit()),
    msockAddrField (new QLineEdit()),
    msockPortField(new QLineEdit()),
    mserverPassField(new QLineEdit()),
    mcancelBtn(new QPushButton(tr("&Close"))),
    mapplySettingBtn(new QPushButton("&Apply settings")),
    mchangePwdBtn(new QPushButton("C&hange password")),
    mdonateBtn(new ImageButton(":images/built-in/donate.png")),
    mshowAuthInfoChkbx(new QCheckBox(tr("&Show in clear"))),
    museMklsChkbx(new QCheckBox(tr("Use &Livestatus"))),
    mmainLayout (new QGridLayout(this))
{
  qint32 line = -1;
  moldPwdField->setEchoMode(QLineEdit::Password);
  mpwdField->setEchoMode(QLineEdit::Password);
  mrePwdField->setEchoMode(QLineEdit::Password);
  mserverPassField->setEchoMode(QLineEdit::Password);
  switch (_action)
    {
    case Preferences::ChangeMonitoringSettings:
      setWindowTitle(tr("Monitoring Settings | %1").arg(APP_NAME));
      line++,
          mmainLayout->addWidget(createBaseGrp(), line, 0, 1, 3);
      line++,
          mmainLayout->addWidget(createScktGrp(), line, 0, 1, 3);
      line++,
          mmainLayout->addWidget(createCommonGrp(), line, 0, 1, 3);
      line++,
          mmainLayout->addWidget(mcancelBtn, line, 1, Qt::AlignRight),
          mmainLayout->addWidget(mapplySettingBtn, line, 2);
      line++,
          mmainLayout->addWidget(new QLabel(tr("(*) Required for Zabbix and Zenoss.")), line, 0, 1, 3);
      mmainLayout->setColumnStretch(0, 0);
      mmainLayout->setColumnStretch(1, 6);
      mmainLayout->setColumnStretch(2, 0);
      if(_userRole == Auth::OpUserRole) {
          monitorUrlField->setEnabled(false);
          mbrwBtn->setEnabled(false);
          mupdateIntervalField->setEnabled(false);
          mapplySettingBtn->setEnabled(false);
          msockAddrField->setEnabled(false);
          msockPortField->setEnabled(false);
          mserverPassField->setEnabled(false);
          mshowAuthInfoChkbx->setEnabled(false);
          museMklsChkbx->setEnabled(false);
        }
      break;

    case Preferences::ChangePassword:
    case Preferences::ForceChangePassword:
      setWindowTitle(tr("Change Password| %1").arg(APP_NAME));
      line++,
          mmainLayout->addWidget(new QLabel(tr("Current Password")), line, 0),
          mmainLayout->addWidget(moldPwdField, line, 1, 1, 2);
      line++,
          mmainLayout->addWidget(new QLabel(tr("New password")), line, 0),
          mmainLayout->addWidget(mpwdField, line, 1, 1, 2);
      line++,
          mmainLayout->addWidget(new QLabel(tr("Retype new password")), line, 0),
          mmainLayout->addWidget(mrePwdField, line, 1, 1, 2);
      line++,
          mmainLayout->addWidget(mcancelBtn, line, 1),
          mmainLayout->addWidget(mchangePwdBtn, line, 2);

      if(_action == Preferences::ForceChangePassword) {
          mcancelBtn->setEnabled(false);
        }
      break;

    case Preferences::ShowAbout:
      setWindowTitle(tr("About %1").arg(APP_NAME));
      QString about = AboutPattern.arg(APP_NAME)
          .arg(PKG_VERSION)
          .arg(RELEASE_NAME)
          .arg(REL_INFO)
          .arg(PKG_URL);

      line++,
          mmainLayout->addWidget(new QLabel(about), line, 0, 1, 2);
      line++,
          mmainLayout->addWidget(mdonateBtn, line, 0, 1, 1, Qt::AlignLeft),
          mmainLayout->addWidget(mcancelBtn, line, 1, 1, 1, Qt::AlignRight);
      break;
    }

  setContent();
  addEvents();
}

Preferences::~Preferences()
{
  delete mupdateIntervalField;
  delete moldPwdField;
  delete mpwdField;
  delete mrePwdField;
  delete mchangePwdBtn;
  delete mcancelBtn;
  delete mapplySettingBtn;
  delete msockAddrField;
  delete msockPortField;
  delete mserverPassField;
  delete mdonateBtn;
  delete mshowAuthInfoChkbx;
  delete museMklsChkbx;
  delete mmainLayout;
}


void Preferences::showEvent (QShowEvent *)
{
  moldPwdField->setText("");
  mpwdField->setText("");
  mrePwdField->setText("");
}

void Preferences::applySettings(void)
{
  QString homeUrl = monitorUrlField->text();
  msettings->setValue(URL_KEY, homeUrl);
  msettings->setValue(UPDATE_INTERVAL_KEY, mupdateIntervalField->text());
  msettings->setValue(SERVER_ADDR_KEY, msockAddrField->text());
  if(msockPortField->text().toInt() <= 0) msockPortField->setText(QString::number(MonitorBroker::DefaultPort));
  msettings->setValue(SERVER_PORT_KEY, msockPortField->text());
  msettings->setValue(SERVER_PASS_KEY, mserverPassField->text());
  museMkls = static_cast<Qt::CheckState>(museMklsChkbx->checkState());
  msettings->setValue(USE_MKLS_KEY, museMkls);
  msettings->sync();
  close();
  emit urlChanged(homeUrl);
}


void Preferences::changePasswd(void)
{
  QString userPasswd;
  QString passwd;
  QString newPasswd;
  QString renewPasswd;
  QString key;

  if (muserRole == Auth::AdmUserRole) {
      key = ADM_PASSWD_KEY;
      userPasswd = msettings->value(key).toString();
    } else {
      key = OP_PASSWD_KEY;
      userPasswd = msettings->value(key).toString();
    }
  passwd = QCryptographicHash::hash(moldPwdField->text().toAscii(), QCryptographicHash::Md5);
  newPasswd = QCryptographicHash::hash(mpwdField->text().toAscii(), QCryptographicHash::Md5);
  renewPasswd = QCryptographicHash::hash(mrePwdField->text().toAscii(), QCryptographicHash::Md5);

  if(userPasswd == passwd) {
      if(newPasswd == renewPasswd) {
          msettings->setKeyValue(key, newPasswd);
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


void Preferences::setAuthChainVisibility(const int & state) {
  if(state == Qt::Checked) {
      mserverPassField->setEchoMode(QLineEdit::Normal);
    } else {
      mserverPassField->setEchoMode(QLineEdit::Password);
    }
}

QGroupBox* Preferences::createBaseGrp(void)
{
  QGroupBox* bx(new QGroupBox());
  QHBoxLayout* lyt(new QHBoxLayout());
  lyt->addWidget(new QLabel(tr("Web Interface*")));
  lyt->addWidget(monitorUrlField);
  lyt->setStretch(0, 0);
  lyt->setStretch(1, 1);
  bx->setLayout(lyt);
  bx->setAlignment(Qt::AlignLeft);
  return bx;
}

QGroupBox* Preferences::createScktGrp(void)
{
  QGroupBox* bx(new QGroupBox(tr("Livestatus/ngrt4nd Settings")));
  QHBoxLayout* lyt(new QHBoxLayout());
  lyt->addWidget(new QLabel(tr("Server Address")));
  lyt->addWidget(msockAddrField);
  lyt->addWidget(new QLabel(tr("Port")), Qt::AlignRight);
  lyt->addWidget(msockPortField);
  lyt->addWidget(museMklsChkbx);
  lyt->setStretch(0, 0);
  lyt->setStretch(1, 1);
  lyt->setStretch(2, 0);
  lyt->setStretch(3, 0);
  lyt->setStretch(4, 0);
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
      lyt->addWidget(new QLabel(tr("Auth chain")), line, 0),
      lyt->addWidget(mserverPassField, line, 1, 1, 2);
  line++,
      lyt->addWidget(new QLabel(tr("")), line, 0),
      lyt->addWidget(mshowAuthInfoChkbx, line, 1, 1, 2);
  line++,
      lyt->addWidget(new QLabel(tr("Update Interval")), line, 0),
      lyt->addWidget(mupdateIntervalField, line, 1),
      lyt->addWidget(new QLabel("seconds"), line, 2);
  lyt->setColumnStretch(0, 0);
  lyt->setColumnStretch(1, 1);
  bx->setLayout(lyt);
  bx->setAlignment(Qt::AlignLeft);
  return bx;
}

void Preferences::setContent(void)
{
  mupdateIntervalField->setMinimum(5);
  mupdateIntervalField->setMaximum(600);
  mupdateIntervalField->setValue(msettings->value(UPDATE_INTERVAL_KEY).toInt());
  monitorUrlField->setText(msettings->value(URL_KEY).toString());
  msockAddrField->setText(msettings->value(SERVER_ADDR_KEY).toString());
  msockPortField->setText(msettings->value(SERVER_PORT_KEY).toString());
  mserverPassField->setText(msettings->value(SERVER_PASS_KEY).toString());
  museMkls = static_cast<Qt::CheckState>(msettings->value(USE_MKLS_KEY).toInt());
  museMklsChkbx->setCheckState(museMkls);
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


void Preferences::addEvents(void)
{
  connect(mapplySettingBtn, SIGNAL(clicked()),  this, SLOT(applySettings()));
  connect(mcancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
  connect(mchangePwdBtn, SIGNAL(clicked()),  this, SLOT(changePasswd()));
  connect(mdonateBtn, SIGNAL(clicked()),  this, SLOT(donate()));
  connect(mshowAuthInfoChkbx, SIGNAL(stateChanged(int)), this, SLOT(setAuthChainVisibility(int)));
}
