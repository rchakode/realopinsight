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
  void updateData(const std::string& viewName, const ListofPlatformStatusT& qosData);
  void setExportFileName(void);

  virtual void handleRequest(const Wt::Http::Request&, Wt::Http::Response& response);


private:
  ListofPlatformStatusT m_platformStatusData;
  std::string m_vname;
};



class WebCsvExportIcon : public Wt::WAnchor
{
public:
  WebCsvExportIcon(void);
  void updateData(const std::string& vname, const ListofPlatformStatusT& platformStatusData);

private:
  std::shared_ptr<WebCsvExportResource> m_csvResource;
};


#endif // WEBCSVREPORTRESOURCE_HPP
