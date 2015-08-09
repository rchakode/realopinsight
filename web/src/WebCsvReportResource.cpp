#include "WebCsvReportResource.hpp"
#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <Wt/WImage>



WebCsvExportResource::WebCsvExportResource(void)
  : Wt::WResource()
{
  m_qosData.clear();
}

void WebCsvExportResource::setExportFileName(void)
{
  if (! m_viewName.empty()) {
    std::string escapedBaseName = QString(m_viewName.c_str()).replace(" ", "_").toStdString();
    suggestFileName(Wt::WString("RealOpInsight_REPORT_{1}.csv")
                    .arg(escapedBaseName));
  } else {
    suggestFileName("RealOpInsight_REPORT_UNSET_VIEW.csv");
  }
}


void WebCsvExportResource::updateData(const std::string& viewName, const QosDataList& qosData)
{
  m_qosData.clear();
  std::copy(qosData.begin(), qosData.end(), std::back_inserter(m_qosData));
  m_viewName = viewName;
}


void WebCsvExportResource::handleRequest(const Wt::Http::Request&, Wt::Http::Response& response)
{
  setExportFileName();
  response.setMimeType("text/csv");
  response.out() << "Timestamp,View Name,Status,Normal (%),Minor (%),Major (%),Critical (%),Unknown (%)\n";
  for (const auto& entry: m_qosData) response.out() << entry.toString() << std::endl;
}


WebCsvExportIcon::WebCsvExportIcon(void)
  : Wt::WAnchor()
{
  setToolTip(Q_TR("Export data in a CSV file"));
  setTarget(Wt::TargetNewWindow);
  // note that the ownership of the resource is not translated to the link
  setLink( Wt::WLink(&m_csvResource) );
  // the ownership of the image is transfered to the Anchor
  setImage( new Wt::WImage("images/built-in/csv-file.png") );
}


void WebCsvExportIcon::updateData(const std::string& viewName, const QosDataList& qosData)
{
  m_csvResource.updateData(viewName, qosData);
}
