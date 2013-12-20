/*
 * Preferences.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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


#ifndef SNAVPREFERENCES_HPP_
#define SNAVPREFERENCES_HPP_

#include "ns.hpp"
#include "Settings.hpp"
#include "Base.hpp"
#include <QSplashScreen>

class ImageButton : public QAbstractButton {
  Q_OBJECT
private:
  QPixmap pixmap;
public:
  ImageButton(const QString& ipath) {this->pixmap = QPixmap(ipath); update();}
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
  enum FormTypeT {
    ChangePassword=0,
    ForceChangePassword,
    ChangeMonitoringSettings,
    ShowHelp,
    ShowAbout,
    BasicLoginForm
  };

  Preferences(qint32 _userRole, qint32 _formType);
  virtual ~Preferences();
  static QString style();
  QBitArray* getSourceStates() const { return m_sourceStates; }
  bool isSetSource(int idx) {return (idx < MAX_SRCS && m_sourceStates)? m_sourceStates->at(idx) : false; }
  void clearUpdatedSources(void) { m_updatedSources.clear(); }
  QString getRealmLogin(void) const {return m_realmLoginField->text();}
  QString getRealmPasswd(void) const {return m_realmPasswdField->text();}
  void setCancelled(bool cancelled) { m_cancelled = cancelled;}
  bool getCancelled(void) const {return m_cancelled;}


public Q_SLOTS:
  void handleCancel(void);
  void applySettings(void);
  void addAsSource(void);
  void deleteSource(void);
  void changePasswd(void);
  void donate(void);
  void setAuthChainVisibility(const int& state);
  void handleSourceSelected();

Q_SIGNALS:
  void urlChanged(QString);
  void sourcesChanged (QList<qint8>);

protected :
  void showEvent (QShowEvent *);

private:
  QGridLayout* m_mainLayout;
  qint32 m_userRole;
  int m_formType;

  Settings* m_settings;
  QBitArray* m_sourceStates;

  QLineEdit* m_monitorUrlField;
  QComboBox* m_monitorTypeField;
  QSpinBox* m_updateIntervalField;
  QPushButton *m_brwBtn;
  QLineEdit* m_oldPwdField;
  QLineEdit* m_pwdField;
  QLineEdit* m_rePwdField;
  QLineEdit* m_sockAddrField;
  QLineEdit* m_sockPortField;
  QLineEdit* m_serverPassField;
  QPushButton* m_cancelBtn;
  QPushButton* m_applySettingBtn;
  QPushButton* m_addAsSourceBtn;
  QPushButton* m_deleteSourceBtn;
  QPushButton* m_changePwdBtn;
  ImageButton* m_donateBtn;
  QCheckBox* m_showAuthInfoChkbx;
  QCheckBox* m_useMklsChkbx;
  QList<qint8> m_updatedSources;
  QCheckBox* m_verifySslPeerChkBx;
  QVector<QRadioButton*> m_sourceBtns;
  int m_selectedSource;
  bool m_cancelled;

  QLineEdit* m_realmLoginField;
  QLineEdit* m_realmPasswdField;

  void addEvents(void);
  QGroupBox* createScktGrp(void);
  QGroupBox* createCommonGrp(void);
  void loadProperties(void);
  void updateFields(void);
  void fillFromSource(int _sidx);
  void saveAsSource(const qint32& idx, const QString& _stype);
  QString getSourceStatesSerialized(void);
  void initSourceStates();
  void initSourceStates(const QString& str);
  QString selectSourceType(void);
  int firstSourceSet(void);
  void updateSourceBtnState(void);
  QGroupBox* createUpdateBtnsGrp(void);
  void organizePrefWindow(void);
  void organizeChangePasswdWindow(void);
  void organizeAbortWindow(void);
  void disableInputField(void);
  void loadBasicLoginForm(void);
};


#endif /* SNAVPREFERENCES_HPP_ */
