#ifndef WEBVIEWSELECTOR_H
#define WEBVIEWSELECTOR_H
#include "dbo/DbObjects.hpp"
#include "Base.hpp"
#include <Wt/WComboBox>
#include <Wt/WDialog>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WVBoxLayout>
#include <Wt/WFileUpload>


class ListSelector : public Wt::WComboBox
{
public:
  ListSelector(void);
  std::string selectedItem (void) const {return m_selectedItem;}
  std::string selectedItemData (void) const {return m_selectedItemData;}
  void updateContentWithViewList(const DbViewsT& vlist);
  void updateContentWithSourceList(const QList<QString>& sids);

private:
  std::string m_selectedItem;
  std::string m_selectedItemData;
  void handleSelectionChanged(void);
};

class InputSelector: public Wt::WDialog
{
public:
  enum SelectorType {
    SourceOnly = 0, // default
    SourceWithTextFilter = 1,
    SourceWithFileFilter = 2
  };

  InputSelector();
  ~InputSelector();
  Wt::Signal<std::string, std::string>& dataTriggered(void) {return m_dataTriggered;}
  Wt::Signal<std::string>& itemTriggered(void) {return m_itemTriggered;}
  void updateContentWithViewList(const DbViewsT& vlist);
  void updateContentWithSourceList(const QList<QString>& slist, int filterType);

private:
  Wt::Signal<std::string, std::string> m_dataTriggered;
  Wt::Signal<std::string> m_itemTriggered;

  int m_selectorType;
  Wt::WContainerWidget m_container;
  Wt::WVBoxLayout* m_mainLayout;
  ListSelector m_itemListSelector;
  Wt::WLineEdit m_textFilterSelector;
  Wt::WFileUpload m_fileFilterSelector;
  Wt::WPushButton m_okBtn;

  void handleApply(void);
  void handleFileUploaded(void);
};




#endif // WEBVIEWSELECTOR_H
