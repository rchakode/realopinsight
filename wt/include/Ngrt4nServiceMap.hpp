/*
 * Ngrt4nMap.hpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */

#ifndef NGRT4NSERVICEMAP_HPP_
#define NGRT4NSERVICEMAP_HPP_

#include <Wt/WPaintedWidget>
#include <Wt/WContainerWidget>
#include <Wt/WPainter>
#include <Wt/WObject>
#include <Wt/WLength>
#include <Wt/WSignal>
#include "include/Ngrt4nCoreDataStructures.hpp"

using namespace Wt ;

class Ngrt4nServiceMap : public WPaintedWidget
{
public:
  Ngrt4nServiceMap();
  virtual
  ~Ngrt4nServiceMap();
  void setWidth(const double & _width) {width =  _width ;}
  void setHeight(const double & _height) {height =  _height ;}
  void update(const bool & _init = false);
  void update(const Ngrt4nListServicesT & _bservices, const Ngrt4nListServicesT & _aservices,  const double& _width, const double & _height) ;
  void  msgPanelSizedChanged(int width, int height){ layoutWidth = width ; update(true) ;}

protected:
  void paintEvent(Wt::WPaintDevice *paintDevice) ;

private:
  double width ;
  double height ;
  double scaleX ;
  double scaleY ;
  double layoutWidth ;
  double layoutHeight ;
  MapIconListT iconPaths ;
  WPainter* painter ;
  Ngrt4nListServicesT* aservices ;
  Ngrt4nListServicesT* bservices ;

  void drawNode(const Ngrt4nServiceT & _service, const bool & _isBusinessService = true) ;
  void createLink(const Ngrt4nServiceT & _service) ;

 static const string PLUS ;
 static const string MINUS ;
 static const string DEFAULT_ICON ;
 static const string NETWORK_ICON ;
 static const string ROUTER_ICON ;
 static const string SWITCH_ICON ;
 static const string FIREWALL_ICON ;
 static const string STORAGE_ICON ;
 static const string FILER_ICON ;
 static const string HARDDISK_ICON ;
 static const string SERVER_ICON ;
 static const string LINUX_ICON ;
 static const string WINDOWS_ICON  ;
 static const string SOLARIS_ICON ;
 static const string WEBSERVER_ICON ;
 static const string DBSERVER_ICON ;
 static const string APP_ICON ;
 static const string WEB_ICON ;
 static const string DB_ICON ;
 static const string PROCESS_ICON ;
 static const string LOG_ICON ;
 static const string CLOUD_ICON ;
 static const string HYPERVISOR_ICON ;
 static const string OTH_CHECK_ICON ;

};

#endif /* NGRT4NSERVICEMAP_HPP_ */
