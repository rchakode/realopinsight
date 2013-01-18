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
const QString Preferences::ADM_UNSERNAME_KEY = "/Auth/admUser";
const QString Preferences::OP_UNSERNAME_KEY = "/Auth/opUsername";
const QString Preferences::ADM_PASSWD_KEY = "/Auth/admPasswd";
const QString Preferences::OP_PASSWD_KEY = "/Auth/opPasswd";
const QString Preferences::SERVER_PASS_KEY = "/Auth/ServerAuthChain";


const QString AboutPattern = QObject::tr("\n%1 %2 (codename: %3)\n \
                        \nRelease ID: %4\n \
                        \nCopyright (c) 2010 NGRT4N Project. All rights reserved \
                        \nVisit %5 for more information\n \
                        \nReport Bugs: bugs@ngrt4n.com\n");


Preferences::Preferences(const qint32 & _userRole, const qint32 & _action)
    : QDialog(),
    userRole(_userRole),
      settings(new Settings()),
      monitorHomeField(new QLineEdit()),
      updateIntervalField(new QSpinBox()),
      bBrowse(new QPushButton(tr("&Browse..."))),
      oldPasswdField(new QLineEdit()),
      passwdField(new QLineEdit()),
      rePasswdField(new QLineEdit()),
      serverAddrField (new QLineEdit()),
      serverPortField(new QLineEdit()),
      serverPassField(new QLineEdit()),
      cancelButton(new QPushButton(tr("&Close"))),
      applySettingButton(new QPushButton("&Apply settings")),
      changePasswdButton(new QPushButton("C&hange password")),
      donateButton(new ImageButton(":images/built-in/donate.png")),
      showAuthChain(new QCheckBox(tr("&Show in clear"))),
      layout(new QGridLayout(this))
{
    qint32 line = -1;
    oldPasswdField->setEchoMode(QLineEdit::Password);
    passwdField->setEchoMode(QLineEdit::Password);
    rePasswdField->setEchoMode(QLineEdit::Password);
    serverPassField->setEchoMode(QLineEdit::Password);

    switch (_action)
    {
    case Preferences::ChangeMonitoringSettings:
        setWindowTitle(tr("Monitoring Settings | %1").arg(APP_NAME));
        line++,
                layout->addWidget(new QLabel(tr("Web Interface*")), line, 0),
                layout->addWidget(monitorHomeField, line, 1, 1, 4);
        line++,
                layout->addWidget(new QLabel(tr("Server Address**")), line, 0),
                layout->addWidget(serverAddrField, line, 1),
                layout->addWidget(new QLabel(tr("Port")), line, 2, Qt::AlignRight),
                layout->addWidget(serverPortField, line, 3);
        line++,
                layout->addWidget(new QLabel(tr("Auth chain")), line, 0),
                layout->addWidget(serverPassField, line, 1, 1, 4);
        line++,
                layout->addWidget(new QLabel(tr("")), line, 0),
                layout->addWidget(showAuthChain, line, 1, 1, 4);
        line++,
                layout->addWidget(new QLabel(tr("Update Interval")), line, 0),
                layout->addWidget(updateIntervalField, line, 1, 1, 2),
                layout->addWidget(new QLabel("seconds"), line, 3);
        line++,
                layout->addWidget(cancelButton, line, 1, 1, 2, Qt::AlignRight),
                layout->addWidget(applySettingButton, line, 3, 1, 2);
        line++,
                layout->addWidget(new QLabel(tr("(*) Required for Zabbix and Zenoss. (**) Required for Nagios.")), line, 0, 1, 5);
        layout->setColumnStretch(0, 0);
        layout->setColumnStretch(1, 6);
        layout->setColumnStretch(2, 0);
        layout->setColumnStretch(3, 1);
        if(_userRole == Auth::OpUserRole) {
            monitorHomeField->setEnabled(false);
            bBrowse->setEnabled(false);
            updateIntervalField->setEnabled(false);
            applySettingButton->setEnabled(false);
            serverAddrField->setEnabled(false);
            serverPortField->setEnabled(false);
            serverPassField->setEnabled(false);
            showAuthChain->setEnabled(false);
        }
        break;

    case Preferences::ChangePassword:
    case Preferences::ForceChangePassword:
        setWindowTitle(tr("Change Password| %1").arg(APP_NAME));
        line++,
                layout->addWidget(new QLabel(tr("Current Password")), line, 0),
                layout->addWidget(oldPasswdField, line, 1, 1, 2);
        line++,
                layout->addWidget(new QLabel(tr("New password")), line, 0),
                layout->addWidget(passwdField, line, 1, 1, 2);
        line++,
                layout->addWidget(new QLabel(tr("Retype new password")), line, 0),
                layout->addWidget(rePasswdField, line, 1, 1, 2);
        line++,
                layout->addWidget(cancelButton, line, 1),
                layout->addWidget(changePasswdButton, line, 2);

        if(_action == Preferences::ForceChangePassword) {
            cancelButton->setEnabled(false);
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
                layout->addWidget(new QLabel(about), line, 0, 1, 2);
        line++,
                layout->addWidget(donateButton, line, 0, 1, 1, Qt::AlignLeft),
                layout->addWidget(cancelButton, line, 1, 1, 1, Qt::AlignRight);
        break;
    }

    setContent();
    addEvents();
}

Preferences::~Preferences()
{
    delete updateIntervalField;
    delete oldPasswdField;
    delete passwdField;
    delete rePasswdField;
    delete changePasswdButton;
    delete cancelButton;
    delete applySettingButton;
    delete serverAddrField;
    delete serverPortField;
    delete serverPassField;
    delete donateButton;
    delete showAuthChain;
    delete layout;
}


void Preferences::showEvent (QShowEvent *)
{
    oldPasswdField->setText("");
    passwdField->setText("");
    rePasswdField->setText("");
}

void Preferences::applySettings(void)
{
    QString nagios_home;

    nagios_home = monitorHomeField->text();

    settings->setValue(URL_KEY, nagios_home);
    settings->setValue(UPDATE_INTERVAL_KEY, updateIntervalField->text());
    settings->setValue(SERVER_ADDR_KEY, serverAddrField->text());
    if(serverPortField->text().toInt() <= 0) serverPortField->setText(QString::number(MonitorBroker::DefaultPort));
    settings->setValue(SERVER_PORT_KEY, serverPortField->text());
    settings->setValue(SERVER_PASS_KEY, serverPassField->text());
    settings->sync();

    close();

    emit urlChanged(nagios_home);
}


void Preferences::changePasswd(void)
{
    QString userPasswd, passwd, newPasswd, renewPasswd, key;

    if (userRole == Auth::AdmUserRole) {
        key = ADM_PASSWD_KEY;
        userPasswd = settings->value(key).toString();
    } else {
        key = OP_PASSWD_KEY;
        userPasswd = settings->value(key).toString();
    }
    passwd = QCryptographicHash::hash(oldPasswdField->text().toAscii(), QCryptographicHash::Md5);
    newPasswd = QCryptographicHash::hash(passwdField->text().toAscii(), QCryptographicHash::Md5);
    renewPasswd = QCryptographicHash::hash(rePasswdField->text().toAscii(), QCryptographicHash::Md5);

    if(userPasswd == passwd) {
        if(newPasswd == renewPasswd) {
            settings->setKeyValue(key, newPasswd);
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
        serverPassField->setEchoMode(QLineEdit::Normal);
    } else {
        serverPassField->setEchoMode(QLineEdit::Password);
    }
}

void Preferences::setContent(void)
{
    updateIntervalField->setMinimum(5);
    updateIntervalField->setMaximum(600);
    updateInterval = settings->value(UPDATE_INTERVAL_KEY).toInt();
    updateIntervalField->setValue(updateInterval);

    monitorUrl = settings->value(URL_KEY).toString();
    monitorHomeField->setText(monitorUrl);

    serverAddr = settings->value(SERVER_ADDR_KEY).toString();
    serverAddrField->setText(serverAddr);

    serverPort = settings->value(SERVER_PORT_KEY).toString();
    serverPortField->setText(serverPort);

    serverPass = settings->value(SERVER_PASS_KEY).toString();
    serverPassField->setText(serverPass);
}

QSplashScreen* Preferences::infoScreen(const QString & msg) {
    QSplashScreen* screen = new QSplashScreen(QPixmap(":images/built-in/loading-screen.png"));
    screen->showMessage(msg, Qt::AlignJustify|Qt::AlignCenter);
    screen->show();
    return  screen;
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
            "	padding: 1px;"
            "	border-radius: 3px;"
            "	opacity: 500;"
            "	color : #000000;"
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
    connect(applySettingButton, SIGNAL(clicked()),  this, SLOT(applySettings()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(changePasswdButton, SIGNAL(clicked()),  this, SLOT(changePasswd()));
    connect(donateButton, SIGNAL(clicked()),  this, SLOT(donate()));
    connect(showAuthChain, SIGNAL(stateChanged(int)), this, SLOT(setAuthChainVisibility(int)));
}
