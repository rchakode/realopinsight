#ifndef DIALOGFORMS_HPP
#define DIALOGFORMS_HPP
#include <QDialog>
#include <QList>
#include <QString>
#include <QComboBox>
#include <QLineEdit>


class CheckImportationSettingsForm: public QDialog
{
  Q_OBJECT
public:
  CheckImportationSettingsForm(const QList<QString>& sourceList, bool statusFile);

  QString selectedSource(void) const {return m_sourceSelectionBox->currentText();}
  QString filter(void) const {return m_filter->text();}
  QString selectedStatusFile(void) const {return m_statusFile;}

public Q_SLOTS:
  void handleSelectStatusFile(void);

private:
  QComboBox* m_sourceSelectionBox;
  QLineEdit* m_filter;
  QLineEdit* m_statusFileArea;
  QString m_statusFile;
};

#endif // DIALOGFORMS_HPP
