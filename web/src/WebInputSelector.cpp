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

void ListSelector::updateContentWithSourceList(const SourceListT& vlist)
{
  m_selectedItemData.clear();
  Wt::WStandardItemModel* dataModel = static_cast<Wt::WStandardItemModel*>(Wt::WComboBox::model());
  dataModel->clear();

  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText(Q_TR("-- Please select an item --"));
  dataModel->appendRow(item);

  for(const auto& source: vlist) {
    item = new Wt::WStandardItem();
    item->setText(source.id.toStdString());
    item->setData(source.id.toStdString(), Wt::UserRole);
    dataModel->appendRow(item);
  }

  setCurrentIndex(0);
}


void ListSelector::handleSelectionChanged(void)
{
  int index = currentIndex();
  Wt::WStandardItemModel* dataModel = static_cast<Wt::WStandardItemModel*>(this->model());
  if (index>0) {
    m_selectedItemData = boost::any_cast<std::string>(dataModel->item(index, 0)->data());
  }
}



InputSelector::InputSelector():
  m_mainLayout(new Wt::WVBoxLayout())
{
  setWindowTitle(Q_TR("Input Selector"));

  m_okBtn.setText(Q_TR("Apply"));
  m_okBtn.setStyleClass("btn btn-info");
  m_okBtn.clicked().connect(this, &InputSelector::handleAccept);

  Wt::WDialog::contents()->addWidget(&m_container);

  m_container.setMargin(10, Wt::All);
  m_container.setLayout(m_mainLayout);

  m_mainLayout->addWidget(&m_listSelector);
  m_mainLayout->addWidget(&m_filterField);
  m_mainLayout->addWidget(&m_okBtn);
}



InputSelector::~InputSelector()
{
  m_mainLayout->removeWidget(&m_okBtn);
  m_mainLayout->removeWidget(&m_filterField);
  m_mainLayout->removeWidget(&m_listSelector);
  m_container.clear();
  Wt::WDialog::contents()->removeWidget(&m_container);
}

void InputSelector::updateContentWithViewList(const DbViewsT& vlist)
{
  m_listSelector.updateContentWithViewList(vlist);
  m_filterField.setHidden(true);
}


void InputSelector::updateContentWithSourceList(const SourceListT& slist)
{
  m_listSelector.updateContentWithSourceList(slist);
  m_filterField.setHidden(false);
  m_filterField.setPlaceholderText(Q_TR("Set hostgroup to filter on (optional)"));
}


void InputSelector::handleAccept(void)
{
  Wt::WDialog::accept();
  m_itemSelected.emit(m_listSelector.selectedItemData());
}
