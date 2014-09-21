/*
# GuiPreferences.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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

#ifndef GUIPREFERENCES_HPP
#define GUIPREFERENCES_HPP

#include <QDialog>
#include "Preferences.hpp"
#include <QAbstractButton>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QtWidgets>
#else
#   include<QtGui>
#endif


class ImageButton : public QAbstractButton {
public:
  ImageButton(const QString& ipath) {
    this->pixmap = QPixmap(ipath);
    update();
  }
  ~ImageButton(){}
  void setPixmap(const QPixmap& pm ) {pixmap = pm; update(); }
  QSize sizeHint() const {return pixmap.size();}
protected:
  void paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap );
  }
private:
  QPixmap pixmap;
};


class GuiPreferences : public Preferences
{
  Q_OBJECT

public:
  explicit GuiPreferences(qint32 _userRole, qint32 _formType);
  ~GuiPreferences();
  void clearUpdatedSources(void) { m_updatedSources.clear(); }
  QString getRealmLogin(void) const {return m_realmLoginField->text();}
  QString getRealmPasswd(void) const {return m_realmPasswdField->text();}
  void setCancelled(bool cancelled) { m_cancelled = cancelled;}
  bool getCancelled(void) const {return m_cancelled;}
  static QString style();
  void show(void);
  int exec(void) {return m_dialog->exec();}

protected :
  virtual void applyChanges(void);
  virtual void handleCancel(void);
  virtual void fillFromSource(int _sidx);
  virtual void updateAllSourceWidgetStates(void);
  virtual void updateFields(void);
  virtual void saveAsSource(const qint32& index, const QString& type);
  virtual void addAsSource(void);
  virtual void deleteSource(void);

protected Q_SLOTS:
  void setAuthChainVisibility(const int& state);
  void handleDonate(void);
  void changePasswd(void);
  void handleSourceSelected(void);

private:
  QDialog* m_dialog;
  QGridLayout* m_mainLayout;
  qint32 m_userRole;
  int m_formType;
  QLineEdit* m_monitorUrlField;
  QComboBox* m_monitorTypeField;
  QSpinBox* m_updateIntervalField;
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
  QCheckBox* m_useNgrt4ndChkbx;
  QList<qint8> m_updatedSources;
  QCheckBox* m_verifySslPeerChkBx;
  QVector<QRadioButton*> m_sourceBtns;
  QComboBox* m_languageBoxField;
  bool m_cancelled;
  QLineEdit* m_realmLoginField;
  QLineEdit* m_realmPasswdField;

  void addEvents(void);
  QGroupBox* createScktGrp(void);
  QGroupBox* createCommonGrp(void);
  QGroupBox* createUpdateBtnsGrp(void);
  void createPreferenceWindow(void);
  void createChangePasswordForm(void);
  void organizeAbortWindow(void);
  void disableFieldIfRequired(void);
  void loadBasicLoginForm(void);
  QString selectSourceType(void);
};

#endif // GUIPREFERENCES_HPP
