#include "GuiDialogForms.hpp"
#include "Base.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>

CheckImportationSettingsForm::CheckImportationSettingsForm(const QList<QString>& sourceList, bool importFile)
  : m_sourceSelectionBox(NULL),
    m_filter(NULL),
    m_selectedFileTextField(NULL)
{
  // build generic widgets
  m_sourceSelectionBox = new QComboBox(this);
  m_sourceSelectionBox->addItems(sourceList);
  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);

  // build form
  QGridLayout* layout = new QGridLayout(this);
  layout->addWidget(new QLabel(tr("Select source*"), this), 0, 0);
  layout->addWidget(m_sourceSelectionBox, 0, 1);
  if (! importFile) {
    layout->addWidget(new QLabel(tr("Host or group filter (optional)"), this), 1, 0);
    m_filter = new QLineEdit(this);
    layout->addWidget(m_filter, 1, 1);
  } else {
    QPushButton* fileBrowser = new QPushButton(tr("browse..."), this);
    m_selectedFileTextField = new QLineEdit(this);
    m_selectedFileTextField->setReadOnly(true);
    layout->addWidget(new QLabel(tr("Select a file"), this), 1, 0);
    layout->addWidget(m_selectedFileTextField, 1, 1);
    layout->addWidget(fileBrowser, 1, 2);
    connect(fileBrowser, SIGNAL(clicked()), this, SLOT(handleSelectStatusFile()));
  }
  layout->addWidget(buttons, 2, 1);

  //events
  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

  // Disable OK button if no source
  if (sourceList.isEmpty()) {
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
}


void CheckImportationSettingsForm::handleSelectStatusFile(void)
{
  m_selectedFile = QFileDialog::getOpenFileName(this,
                                              tr("Select a status file | %1").arg(APP_NAME),
                                              ".",
                                              tr("Data files (*.dat);;All files (*)"));
  if (m_selectedFile.isNull()) {
    m_selectedFile = QString();
  } else {
    m_selectedFileTextField->setText(m_selectedFile);
  }
}
