#ifndef WEBVIEWSELECTOR_H
#define WEBVIEWSELECTOR_H
#include "dbo/src/DbObjects.hpp"
#include "WebInputList.hpp"
#include "Base.hpp"
#include <Wt/WComboBox.h>
#include <Wt/WDialog.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WFileUpload.h>

class WebInputField: public Wt::WDialog
{
public:
  enum SelectorType {
    SourceOnly = 0, // default
    SourceWithTextFilter = 1,
    SourceWithFileFilter = 2
  };

  WebInputField();
  ~WebInputField(){}
  Wt::Signal<std::string, std::string>& dataTriggered(void) {return m_dataTriggered;}
  Wt::Signal<std::string>& itemTriggered(void) {return m_itemTriggered;}
  Wt::Signal<std::string, std::string>& fileUploaded(void) {return m_fileUploaded;}

  void updateContentWithViewList(const DbViewsT& vlist);
  void updateContentWithSourceList(const QList<QString>& slist, int filterType);

private:
  Wt::Signal<std::string> m_itemTriggered;
  Wt::Signal<std::string, std::string> m_dataTriggered;
  Wt::Signal<std::string, std::string> m_fileUploaded;
  int m_selectorType;
  WebInputList* m_listItemSelectorRef;
  Wt::WLineEdit* m_textFilterSelectorRef;
  Wt::WContainerWidget* m_fileFilterContainerRef;
  Wt::WFileUpload* m_fileFilterSelectorRef;
  Wt::WContainerWidget* m_contentRef;

  void handleApply(void);
};


#endif // WEBVIEWSELECTOR_H
