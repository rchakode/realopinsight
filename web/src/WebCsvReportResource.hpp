#ifndef WEBCSVREPORTRESOURCE_HPP
#define WEBCSVREPORTRESOURCE_HPP

#include "dbo/src/DbSession.hpp"
#include <Wt/WResource.h>
#include <Wt/WAnchor.h>

class WebCsvExportResource : public Wt::WResource
{
public:
  WebCsvExportResource(void);
  ~WebCsvExportResource(){ beingDeleted(); }
  void updateData(const std::string& viewName, const PlatformStatusList& qosData);
  void setExportFileName(void);

  virtual void handleRequest(const Wt::Http::Request&, Wt::Http::Response& response);


private:
  PlatformStatusList m_qosData;
  std::string m_viewName;
};



class WebCsvExportIcon : public Wt::WAnchor
{
public:
  WebCsvExportIcon(void);
  void updateData(const std::string& viewName, const PlatformStatusList& qosData);

private:
  std::shared_ptr<WebCsvExportResource> m_csvResource;
};


#endif // WEBCSVREPORTRESOURCE_HPP
