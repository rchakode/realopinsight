#ifndef WEBINPUTLIST_HPP
#define WEBINPUTLIST_HPP
#include "dbo/src/DbObjects.hpp"
#include "Base.hpp"
#include <Wt/WComboBox.h>
#include <Wt/WDialog.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WFileUpload.h>


class WebInputList : public Wt::WComboBox
{
public:
  WebInputList(void);
  std::string selectedItem (void) const {return m_selectedItem;}
  std::string selectedItemData (void) const {return m_selectedItemData;}
  void updateContentWithViewList(const DbViewsT& vlist);
  void updateContentWithSourceList(const QList<QString>& sids);

private:
  std::string m_selectedItem;
  std::string m_selectedItemData;
  std::shared_ptr<Wt::WStandardItemModel> m_model;
  void handleSelectionChanged(void);
};

#endif // WEBINPUTLIST_HPP
