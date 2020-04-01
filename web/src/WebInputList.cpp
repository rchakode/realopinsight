#include "WebInputList.hpp"
#include <any>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>


WebInputList::WebInputList(void)
{
  m_model = std::make_shared<Wt::WStandardItemModel>();
  setModel(m_model);
  setMargin(10, Wt::Side::Right);
  Wt::WComboBox::changed().connect(this, &WebInputList::handleSelectionChanged);
}


void WebInputList::updateContentWithViewList(const DbViewsT& vlist)
{
  m_selectedItemData.clear();
  m_model->clear();
  m_model->appendRow(std::make_unique<Wt::WStandardItem>(Q_TR("-- Please select an item --")));
  for(const auto& view: vlist) {
    auto item = std::make_unique<Wt::WStandardItem>(view.name);
    item->setData(view.path, Wt::ItemDataRole::User);
    m_model->appendRow(std::move(item));
  }
  setCurrentIndex(0);
}

void WebInputList::updateContentWithSourceList(const QList<QString>& sids)
{
  m_selectedItemData.clear();
  m_model->clear();
  m_model->appendRow(std::make_unique<Wt::WStandardItem>(Q_TR("-- Please select an item --")));
  for(const auto& id: sids) {
    auto item = std::make_unique<Wt::WStandardItem>(id.toStdString());
    item->setData(id.toStdString(), Wt::ItemDataRole::User);
    m_model->appendRow(std::move(item));
  }
  setCurrentIndex(0);
}


void WebInputList::handleSelectionChanged(void)
{
  int index = currentIndex();
  if (index > 0) {
    m_selectedItem = currentText().toUTF8();
    m_selectedItemData = Wt::cpp17::any_cast<std::string>(m_model->item(index, 0)->data());
  }
}
