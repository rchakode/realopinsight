#ifndef GUIPREFERENCES_HPP
#define GUIPREFERENCES_HPP

#include <QDialog>
#include "Preferences.hpp"


class LIBREALOPINSIGHTSHARED_EXPORT ImageButton : public QAbstractButton {
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
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap );
  }
};


class GuiPreferences : public Preferences, public QDialog
{
  Q_OBJECT
public:
  explicit GuiPreferences(qint32 _userRole, qint32 _formType, QWidget *parent = 0);
  ~GuiPreferences();
  void clearUpdatedSources(void) { m_updatedSources.clear(); }
  QString getRealmLogin(void) const {return m_realmLoginField->text();}
  QString getRealmPasswd(void) const {return m_realmPasswdField->text();}
  void setCancelled(bool cancelled) { m_cancelled = cancelled;}
  bool getCancelled(void) const {return m_cancelled;}
  static QString style();

protected :
  virtual void showEvent (QShowEvent *);
  virtual void updateSourceBtnState(void);
  virtual void applyChanges(void);
  virtual void handleCancel(void);
  virtual void fillFromSource(int _sidx);
  virtual void updateSourceBtnState(void);
  virtual void updateFields(void);
  virtual void saveAsSource(const qint32& index, const QString& type);
  virtual void addAsSource(void);
  virtual void deleteSource(void);

protected Q_SLOTS:
  void setAuthChainVisibility(const int& state);
  void handleDonate(void);
  void changePasswd(void);

private:
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
  void handleSourceSelected();
  QString letUserSelectType(void);
};

#endif // GUIPREFERENCES_HPP
