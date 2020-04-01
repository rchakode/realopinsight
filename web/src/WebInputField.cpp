#include "WebInputField.hpp"
#include <any>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>

WebInputField::WebInputField()
  : Wt::WDialog(),
    m_selectorType(SourceOnly)
{
  setWindowTitle(Q_TR("Input Selector"));

  auto layout = std::make_unique<Wt::WVBoxLayout>();

  auto listItemSelector = std::make_unique<WebInputList>();
  m_listItemSelectorRef = listItemSelector.get();
  layout->addWidget(std::move(listItemSelector));

  auto textFilterSelector = std::make_unique<Wt::WLineEdit>();
  m_textFilterSelectorRef = textFilterSelector.get();
  textFilterSelector->setPlaceholderText(Q_TR("Set a group filter (optional)"));
  layout->addWidget(std::move(textFilterSelector));

  auto fileFilter = std::make_unique<Wt::WFileUpload>();
  m_fileFilterSelectorRef = fileFilter.get();
  fileFilter->setFileTextSize(4 * 1024);
  fileFilter->changed().connect(fileFilter.get(), &Wt::WFileUpload::upload);
  auto fileFilterContainer = std::make_unique<Wt::WContainerWidget>();
  m_fileFilterContainerRef = fileFilterContainer.get();
  fileFilterContainer->addWidget(std::move(fileFilter));
  layout->addWidget(std::move(fileFilterContainer));

  auto applyBtn = std::make_unique<Wt::WPushButton>();
  applyBtn->setText(Q_TR("Apply"));
  applyBtn->setStyleClass("btn btn-info");
  applyBtn->clicked().connect(this, &WebInputField::handleApply);
  layout->addWidget(std::move(applyBtn));

  auto content = std::make_unique<Wt::WContainerWidget>();
  m_contentRef = content.get();
  content->setMargin(10, Wt::AllSides);
  content->setLayout(std::move(layout));
  Wt::WDialog::contents()->addWidget(std::move(content));
}

void WebInputField::updateContentWithViewList(const DbViewsT& vlist)
{
  m_listItemSelectorRef->updateContentWithViewList(vlist);
  m_textFilterSelectorRef->setHidden(true);
  m_fileFilterSelectorRef->setHidden(true);
}


void WebInputField::updateContentWithSourceList(const QList<QString>& sids, int filterType)
{
  m_listItemSelectorRef->updateContentWithSourceList(sids);
  m_selectorType = filterType;
  switch (m_selectorType) {
  case SourceOnly:
    m_textFilterSelectorRef->setHidden(true);
    m_fileFilterSelectorRef->setHidden(true);
    break;
  case SourceWithTextFilter:
    m_fileFilterSelectorRef->setHidden(true);
    m_textFilterSelectorRef->setHidden(false);
    break;
  case SourceWithFileFilter:
    m_textFilterSelectorRef->setHidden(true);
    m_fileFilterSelectorRef->setHidden(false);
    break;
  default:
    break;
  }
}


void WebInputField::handleApply(void)
{
  Wt::WDialog::accept();
  m_itemTriggered.emit(m_listItemSelectorRef->selectedItem());
  m_dataTriggered.emit(m_listItemSelectorRef->selectedItemData(), m_textFilterSelectorRef->text().toUTF8());
  m_fileUploaded.emit(m_listItemSelectorRef->selectedItem(), m_fileFilterSelectorRef->spoolFileName());

  auto fileFilter = std::make_unique<Wt::WFileUpload>();
  m_fileFilterSelectorRef = fileFilter.get();
  fileFilter->setFileTextSize(4 * 1024);
  fileFilter->changed().connect(fileFilter.get(), &Wt::WFileUpload::upload);

  m_fileFilterContainerRef->removeWidget(m_fileFilterSelectorRef);
  m_fileFilterContainerRef->addWidget(std::move(fileFilter));
}

