#include "WebCsvReportResource.hpp"
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WImage.h>



WebCsvExportResource::WebCsvExportResource(void)
  : Wt::WResource()
{
  m_platformStatusData.clear();
}

void WebCsvExportResource::setExportFileName(void)
{
  if (! m_vname.empty()) {
    std::string escapedBaseName = QString(m_vname.c_str()).replace(" ", "_").toStdString();
    suggestFileName(Wt::WString("RealOpInsight_REPORT_{1}.csv").arg(escapedBaseName));
  } else {
    suggestFileName("RealOpInsight_REPORT_UNSET_VIEW.csv");
  }
}


void WebCsvExportResource::updateData(const std::string& vname, const ListofPlatformStatusT& qosData)
{
  m_platformStatusData.clear();
  std::copy(qosData.begin(), qosData.end(), std::back_inserter(m_platformStatusData));
  m_vname = vname;
}


void WebCsvExportResource::handleRequest(const Wt::Http::Request&, Wt::Http::Response& response)
{
  setExportFileName();
  response.setMimeType("text/csv");
  response.out() << "Timestamp,Platform Name,Status,Normal (%),Minor (%),Major (%),Critical (%),Unknown (%)\n";
  for (const auto& entry: m_platformStatusData) {
    response.out() << entry.toString() << std::endl;
  }
}


WebCsvExportIcon::WebCsvExportIcon(void)
  : Wt::WAnchor()
{
  m_csvResource = std::make_shared<WebCsvExportResource>();
  Wt::WLink link(m_csvResource);
  link.setTarget(Wt::LinkTarget::NewWindow);
  setLink(link);
  setToolTip(Q_TR("Export data in a CSV file"));
  setImage(std::make_unique<Wt::WImage>("images/built-in/csv-file.png"));
}


void WebCsvExportIcon::updateData(const std::string& vname, const ListofPlatformStatusT& statusData)
{
  m_csvResource->updateData(vname, statusData);
}
