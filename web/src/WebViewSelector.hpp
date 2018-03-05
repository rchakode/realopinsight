#ifndef WEBVIEWSELECTOR_H
#define WEBVIEWSELECTOR_H
#include "dbo/DbObjects.hpp"
#include <Wt/WComboBox>
#include <Wt/WDialog>
#include <Wt/WPushButton>

class ViewListSelector : public Wt::WComboBox
{
public:
  ViewListSelector(void);
  std::string selectedViewPath (void) const {return m_selectedView;}
  void updateContent(const DbViewsT& viewList);

private:
  std::string m_selectedView;
  void handleSelectionChanged(void);
};

class ViewSelectorDialog: public Wt::WDialog
{
public:
  ViewSelectorDialog();
  ~ViewSelectorDialog();
  Wt::Signal<std::string>& viewSelected(void) {return m_viewSelected;}
  void updateContent(const DbViewsT& viewList) { m_viewListSelector.updateContent(viewList); }

private:
  Wt::Signal<std::string> m_viewSelected;
  Wt::WContainerWidget m_container;
  ViewListSelector m_viewListSelector;
  Wt::WPushButton m_okBtn;

  void handleAccept(void);
};

#endif // WEBVIEWSELECTOR_H
