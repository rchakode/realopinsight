#include "GuiDialogForms.hpp"
#include "Base.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>

CheckImportationSettingsForm::CheckImportationSettingsForm(const QList<QString>& sourceList, bool statusFile)
  : m_sourceSelectionBox(NULL),
    m_hostFilter(NULL),
    m_statusFileArea(NULL)
{
  setWindowTitle(tr("Import data | %1").arg(APP_NAME));
  // build generic widgets
  m_sourceSelectionBox = new QComboBox(this);
  m_sourceSelectionBox->addItems(sourceList);
  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);

  // build form
  QGridLayout* layout = new QGridLayout(this);
  layout->addWidget(new QLabel(tr("Select source*"), this), 0, 0);
  layout->addWidget(m_sourceSelectionBox, 0, 1);
  if (! statusFile) {
    layout->addWidget(new QLabel(tr("Host filter (optional)"), this), 1, 0);
    m_hostFilter = new QLineEdit(this);
    layout->addWidget(m_hostFilter, 1, 1);
  } else {
    QPushButton* fileBrowser = new QPushButton(tr("browse..."), this);
    m_statusFileArea = new QLineEdit(this);
    m_statusFileArea->setReadOnly(true);
    layout->addWidget(new QLabel(tr("Status file"), this), 1, 0);
    layout->addWidget(m_statusFileArea, 1, 1);
    layout->addWidget(fileBrowser, 1, 2);
    connect(fileBrowser, SIGNAL(clicked()), this, SLOT(handleSelectStatusFile()));
  }
  layout->addWidget(buttons, 2, 1);

  //events
  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

CheckImportationSettingsForm::~CheckImportationSettingsForm()
{
}

void CheckImportationSettingsForm::handleSelectStatusFile(void)
{
  m_statusFile = QFileDialog::getOpenFileName(this,
                                              tr("Select a status file | %1").arg(APP_NAME),
                                              ".",
                                              tr("Data files (*.dat);;All files (*)"));
  if (m_statusFile.isNull()) {
    m_statusFile = QString();
  } else {
    m_statusFileArea->setText(m_statusFile);
  }
}
