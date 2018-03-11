#include "WebViewSelector.hpp"
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>

ViewListSelector::ViewListSelector(void)
{
  setMargin(10, Wt::Right);
  setModel(new Wt::WStandardItemModel());
  Wt::WComboBox::changed().connect(this, &ViewListSelector::handleSelectionChanged);
}



void ViewListSelector::updateContent(const DbViewsT& viewList)
{
  m_selectedView.clear();
  Wt::WStandardItemModel* dataModel = static_cast<Wt::WStandardItemModel*>(Wt::WComboBox::model());
  dataModel->clear();

  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText(Q_TR("-- Please select an item --"));
  dataModel->appendRow(item);

  Q_FOREACH(const DboView& view, viewList) {
    item = new Wt::WStandardItem();
    item->setText(view.name);
    item->setData(view.path, Wt::UserRole);
    dataModel->appendRow(item);
  }

  setCurrentIndex(0);
}


void ViewListSelector::handleSelectionChanged(void)
{
  int index = currentIndex();
  Wt::WStandardItemModel* dataModel = static_cast<Wt::WStandardItemModel*>(this->model());
  if (index>0) {
    m_selectedView = boost::any_cast<std::string>(dataModel->item(index, 0)->data());
  }
}



ViewSelectorDialog::ViewSelectorDialog()
{
  m_okBtn.clicked().connect(this, &ViewSelectorDialog::handleAccept);

  Wt::WDialog::contents()->addWidget(&m_container);

  m_container.clear();
  m_container.setMargin(10, Wt::All);

  m_container.addWidget(&m_viewListSelector);
  m_container.addWidget(&m_okBtn);

  m_okBtn.setText(Q_TR("Apply"));
  setWindowTitle(Q_TR("View Selector"));
}



ViewSelectorDialog::~ViewSelectorDialog()
{
  m_container.removeWidget(&m_viewListSelector);
  m_container.removeWidget(&m_okBtn);
  Wt::WDialog::contents()->removeWidget(&m_container);
}



void ViewSelectorDialog::handleAccept(void)
{
  Wt::WDialog::accept();
  m_viewSelected.emit(m_viewListSelector.selectedViewPath());
}
