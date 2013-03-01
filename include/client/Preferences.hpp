/*
 * Preferences.hpp
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


#ifndef SNAVPREFERENCES_HPP_
#define SNAVPREFERENCES_HPP_

#include "core/ns.hpp"
#include "Auth.hpp"
#include "Base.hpp"
#include <QSplashScreen>


class ImageButton : public QAbstractButton {
  Q_OBJECT
private:
  QPixmap pixmap;
public:
  ImageButton(const QString & ipath) {this->pixmap = QPixmap(ipath); update();}
  ~ImageButton(){}
  void setPixmap(const QPixmap& pm ) {pixmap = pm; update(); }
  QSize sizeHint() const {return pixmap.size();}
protected:
  void paintEvent( QPaintEvent*) {
    QPainter p( this );
    p.drawPixmap(0, 0, pixmap );
  }
};

class Preferences : public QDialog
{
  Q_OBJECT

public:
  Preferences(const qint32 & _userRole = Auth::OpUserRole, const qint32 & _action = Preferences::ChangePassword);
  virtual ~Preferences();
  static QString style();
  static QSplashScreen* infoScreen(const QString & msg="");

  static const qint32 ChangePassword;
  static const qint32 ForceChangePassword;
  static const qint32 ChangeMonitoringSettings;
  static const qint32 ShowHelp;
  static const qint32 ShowAbout;
  static const QString URL_KEY;
  static const QString UPDATE_INTERVAL_KEY;
  static const QString SERVER_ADDR_KEY;
  static const QString SERVER_PORT_KEY;
  static const QString ADM_UNSERNAME_KEY;
  static const QString OP_UNSERNAME_KEY;
  static const QString ADM_PASSWD_KEY;
  static const QString OP_PASSWD_KEY;
  static const QString SERVER_PASS_KEY;

public slots:
  void applySettings(void);
  void changePasswd(void);
  void donate(void);
  void setAuthChainVisibility(const int & state);

signals:
  void urlChanged(QString);

protected :
  void showEvent (QShowEvent *);

private:
  qint32 userRole;
  Settings* settings;
  QLineEdit* monitorHomeField;
  QSpinBox* updateIntervalField;
  QPushButton *bBrowse;
  qint32 updateInterval;
  QString monitorUrl;
  QString serverAddr;
  QString serverPort;
  QString serverPass;
  QLineEdit* oldPasswdField;
  QLineEdit* passwdField;
  QLineEdit* rePasswdField;
  QLineEdit* msockAddrField;
  QLineEdit* msockPortField;
  QLineEdit* serverPassField;
  QPushButton* cancelButton;
  QPushButton* applySettingButton;
  QPushButton* changePasswdButton;
  ImageButton* donateButton;
  QCheckBox* mshowAuthInfo;
  QCheckBox* museMkLs;
  QGridLayout* mmainLayout;

  QGroupBox* createScktCfgBox(void);
  void setContent(void);
  void addEvents(void);
};


#endif /* SNAVPREFERENCES_HPP_ */
