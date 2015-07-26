#ifndef WEBCSVREPORTRESOURCE_HPP
#define WEBCSVREPORTRESOURCE_HPP

#include "dbo/DbSession.hpp"
#include <Wt/WResource>
#include <Wt/WAnchor>

class WebCsvExportResource : public Wt::WResource
{
public:
  WebCsvExportResource(void);
  ~WebCsvExportResource(){ beingDeleted(); }
  void updateData(QosDataList* qosData, const std::string& viewName);

  virtual void handleRequest(const Wt::Http::Request&, Wt::Http::Response& response);


private:
  QosDataList* m_qosData;
  std::string m_viewName;
};



class WebCsvExportIcon : public Wt::WAnchor
{
public:
  WebCsvExportIcon(void);
  void updateData(QosDataList* qosData, const std::string& viewName);

private:
  WebCsvExportResource m_csvResource;
};


#endif // WEBCSVREPORTRESOURCE_HPP
