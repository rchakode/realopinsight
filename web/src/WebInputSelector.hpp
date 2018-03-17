#ifndef WEBVIEWSELECTOR_H
#define WEBVIEWSELECTOR_H
#include "dbo/DbObjects.hpp"
#include "Base.hpp"
#include <Wt/WComboBox>
#include <Wt/WDialog>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WVBoxLayout>

class ListSelector : public Wt::WComboBox
{
public:
  ListSelector(void);
  std::string selectedItemData (void) const {return m_selectedItemData;}
  void updateContentWithViewList(const DbViewsT& vlist);
  void updateContentWithSourceList(const SourceListT& slist);

private:
  std::string m_selectedItemData;
  void handleSelectionChanged(void);
};



class InputSelector: public Wt::WDialog
{
public:
  InputSelector();
  ~InputSelector();
  Wt::Signal<std::string, std::string>& dataSelectionTriggered(void) {return m_dataSelectionTriggered;}
  void updateContentWithViewList(const DbViewsT& vlist);
  void updateContentWithSourceList(const SourceListT& slist);

private:
  Wt::Signal<std::string, std::string> m_dataSelectionTriggered;

  Wt::WContainerWidget m_container;
  Wt::WVBoxLayout* m_mainLayout;
  ListSelector m_listSelector;
  Wt::WLineEdit m_optionField;
  Wt::WPushButton m_okBtn;

  void handleAccept(void);
};

#endif // WEBVIEWSELECTOR_H
