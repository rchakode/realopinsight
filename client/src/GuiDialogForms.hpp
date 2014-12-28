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
  CheckImportationSettingsForm(const QList<QString>& sourceList, bool importFile);

  QString selectedSource(void) const {return m_sourceSelectionBox->currentText().trimmed();}
  QString filter(void) const {return m_filter->text();}
  QString selectedFile(void) const {return m_selectedFile;}

public Q_SLOTS:
  void handleSelectStatusFile(void);

private:
  QComboBox* m_sourceSelectionBox;
  QLineEdit* m_filter;
  QLineEdit* m_selectedFileTextField;
  QString m_selectedFile;
};

#endif // DIALOGFORMS_HPP
