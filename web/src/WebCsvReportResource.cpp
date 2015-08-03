#include "WebCsvReportResource.hpp"
#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <Wt/WImage>



WebCsvExportResource::WebCsvExportResource(void)
  : Wt::WResource()
{
  std::string pathbasename = QString(m_viewName.c_str()).replace(" ", "_").toStdString();
  suggestFileName(Wt::WString("RealOpInsight_{1}_bireport.csv")
                  .arg(pathbasename));
}



void WebCsvExportResource::updateData(QosDataList* qosData, const std::string& viewName)
{
  m_qosData = qosData;
  m_viewName = viewName;
}


void WebCsvExportResource::handleRequest(const Wt::Http::Request&, Wt::Http::Response& response)
{
  response.setMimeType("text/csv");
  response.out() << "Timestamp,View,Status,Normal (%),Minor (%),Major (%),Critical (%),Unknown (%)\n";
  if (m_qosData) {
    for(const auto& entry: *m_qosData)
      response.out() << entry.toString() << std::endl;
  }
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


void WebCsvExportIcon::updateData(QosDataList* qosData, const std::string& viewName)
{
  m_csvResource.updateData(qosData, viewName);
}
