#include "WebInputSelector.hpp"
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>


ListSelector::ListSelector(void)
{
  setMargin(10, Wt::Right);
  setModel(new Wt::WStandardItemModel());
  Wt::WComboBox::changed().connect(this, &ListSelector::handleSelectionChanged);
}



void ListSelector::updateContentWithViewList(const DbViewsT& vlist)
{
  m_selectedItemData.clear();
  Wt::WStandardItemModel* dataModel = static_cast<Wt::WStandardItemModel*>(Wt::WComboBox::model());
  dataModel->clear();

  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText(Q_TR("-- Please select an item --"));
  dataModel->appendRow(item);

  for(const auto& view: vlist) {
    item = new Wt::WStandardItem();
    item->setText(view.name);
    item->setData(view.path, Wt::UserRole);
    dataModel->appendRow(item);
  }

  setCurrentIndex(0);
}

void ListSelector::updateContentWithSourceList(const QList<QString>& sids)
{
  m_selectedItemData.clear();
  Wt::WStandardItemModel* dataModel = static_cast<Wt::WStandardItemModel*>(Wt::WComboBox::model());
  dataModel->clear();

  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText(Q_TR("-- Please select an item --"));
  dataModel->appendRow(item);

  for(const auto& id: sids) {
    item = new Wt::WStandardItem();
    item->setText(id.toStdString());
    item->setData(id.toStdString(), Wt::UserRole);
    dataModel->appendRow(item);
  }

  setCurrentIndex(0);
}


void ListSelector::handleSelectionChanged(void)
{
  int index = currentIndex();
  Wt::WStandardItemModel* dataModel = static_cast<Wt::WStandardItemModel*>(this->model());
  if (index > 0) {
    m_selectedItem = currentText().toUTF8();
    m_selectedItemData = boost::any_cast<std::string>(dataModel->item(index, 0)->data());
  }
}


InputSelector::InputSelector():
  m_selectorType(SourceOnly),
  m_mainLayout(new Wt::WVBoxLayout())
{
  setWindowTitle(Q_TR("Input Selector"));

  m_okBtn.setText(Q_TR("Apply"));
  m_okBtn.setStyleClass("btn btn-info");
  m_okBtn.clicked().connect(this, &InputSelector::handleApply);

  m_textFilterSelector.setPlaceholderText(Q_TR("Set a group filter (optional)"));

  m_fileFilterSelector.setFileTextSize(4 * 1024); // 4MB => see also max-request-size wt config file);
  m_fileFilterSelector.changed().connect(&m_fileFilterSelector, &Wt::WFileUpload::upload);
  m_fileFilterSelector.uploaded().connect(this, &InputSelector::handleFileUploaded);
  //m_fileFilterSelector.fileTooLarge().connect(this, [=](){});

  Wt::WDialog::contents()->addWidget(&m_container);

  m_container.setMargin(10, Wt::All);
  m_container.setLayout(m_mainLayout);

  m_mainLayout->addWidget(&m_itemListSelector);
  m_mainLayout->addWidget(&m_textFilterSelector);
  m_mainLayout->addWidget(&m_fileFilterSelector);
  m_mainLayout->addWidget(&m_okBtn);
}



InputSelector::~InputSelector()
{
  m_mainLayout->removeWidget(&m_okBtn);
  m_mainLayout->removeWidget(&m_textFilterSelector);
  m_mainLayout->removeWidget(&m_itemListSelector);
  m_mainLayout->removeWidget(&m_fileFilterSelector);
  m_container.clear();
  Wt::WDialog::contents()->removeWidget(&m_container);
}

void InputSelector::updateContentWithViewList(const DbViewsT& vlist)
{
  m_itemListSelector.updateContentWithViewList(vlist);
  m_textFilterSelector.setHidden(true);
  m_fileFilterSelector.setHidden(true);
}


void InputSelector::updateContentWithSourceList(const QList<QString>& sids, int filterType)
{
  m_itemListSelector.updateContentWithSourceList(sids);

  m_selectorType = filterType;

  switch (m_selectorType) {
    case SourceOnly:
      m_textFilterSelector.setHidden(true);
      m_fileFilterSelector.setHidden(true);
      break;
    case SourceWithTextFilter:
      m_fileFilterSelector.setHidden(true);
      m_textFilterSelector.setHidden(false);
      break;
    case SourceWithFileFilter:
      m_textFilterSelector.setHidden(true);
      m_fileFilterSelector.setHidden(false);
      break;
    default:
      break;
  }
}


void InputSelector::handleFileUploaded(void)
{
 // m_fileFilterSelector.setDisabled(true);
}


void InputSelector::handleApply(void)
{
  Wt::WDialog::accept();

  m_itemTriggered.emit(m_itemListSelector.selectedItem());

  switch (m_selectorType) {
    case SourceOnly:
      m_itemTriggered.emit(m_itemListSelector.selectedItem());
      break;
    case SourceWithTextFilter:
      m_dataTriggered.emit(m_itemListSelector.selectedItemData(), m_textFilterSelector.text().toUTF8());
      break;
    case SourceWithFileFilter:
      m_dataTriggered.emit(m_itemListSelector.selectedItemData(), m_fileFilterSelector.spoolFileName());
      break;
    default:
      break;
  }
}

