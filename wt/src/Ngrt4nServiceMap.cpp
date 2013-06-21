/*
 * Ngrt4nMap.cpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */
#include <Wt/WPointF>
#include <Wt/WRectArea>
#include "include/Ngrt4nConfigParser.hpp"
#include "include/Ngrt4nServiceMap.hpp"

const string Ngrt4nServiceMap::PLUS ;
const string Ngrt4nServiceMap::MINUS = "minus" ;
const string Ngrt4nServiceMap::DEFAULT_ICON = "Business Process" ;
const string Ngrt4nServiceMap::NETWORK_ICON = "Network" ;
const string Ngrt4nServiceMap::ROUTER_ICON = "--> Router" ;
const string Ngrt4nServiceMap::SWITCH_ICON = "--> Switch" ;
const string Ngrt4nServiceMap::FIREWALL_ICON = "--> Firewall" ;
const string Ngrt4nServiceMap::STORAGE_ICON = "Storage" ;
const string Ngrt4nServiceMap::FILER_ICON = "--> Storage Area" ;
const string Ngrt4nServiceMap::HARDDISK_ICON = "--> Hard disk" ;
const string Ngrt4nServiceMap::SERVER_ICON = "Server" ;
const string Ngrt4nServiceMap::LINUX_ICON = "--> Linux OS" ;
const string Ngrt4nServiceMap::WINDOWS_ICON = "--> Windows OS" ;
const string Ngrt4nServiceMap::SOLARIS_ICON = "--> Solaris OS" ;
const string Ngrt4nServiceMap::WEBSERVER_ICON = "--> Web Server" ;
const string Ngrt4nServiceMap::DBSERVER_ICON = "--> Database Server" ;
const string Ngrt4nServiceMap::APP_ICON = "Application" ;
const string Ngrt4nServiceMap::WEB_ICON = "--> Web Accessibility" ;
const string Ngrt4nServiceMap::DB_ICON = "--> Database Engine" ;
const string Ngrt4nServiceMap::PROCESS_ICON = "--> Process" ;
const string Ngrt4nServiceMap::LOG_ICON = "--> Logfile" ;
const string Ngrt4nServiceMap::CLOUD_ICON = "Cloud" ;
const string Ngrt4nServiceMap::HYPERVISOR_ICON = "--> Hypervisor" ;
const string Ngrt4nServiceMap::OTH_CHECK_ICON = "Other Check" ;


Ngrt4nServiceMap::Ngrt4nServiceMap()
: WPaintedWidget(0),
  scaleX (1),
  scaleY(1),
  layoutWidth(0),
  layoutHeight(0)
{
  setLayoutSizeAware(true) ;
  setPreferredMethod(InlineSvgVml) ; //TODO do that according to the user agent
  setInline(false);

  iconPaths[DEFAULT_ICON] = "icons/business-process.png";
  iconPaths[OTH_CHECK_ICON] = "icons/check.png";
  iconPaths[SERVER_ICON] = "icons/server.png";
  iconPaths[FIREWALL_ICON] = "icons/firewall.png";
  iconPaths[ROUTER_ICON] = "icons/router.png";
  iconPaths[NETWORK_ICON] = "icons/network.png";
  iconPaths[SWITCH_ICON] = "icons/switch.png";
  iconPaths[FILER_ICON] = "icons/filer.png";
  iconPaths[HARDDISK_ICON] = "icons/harddisk.png";
  iconPaths[STORAGE_ICON] = "icons/storage.png";
  iconPaths[LINUX_ICON] = "icons/linux.png" ;
  iconPaths[WINDOWS_ICON] = "icons/windows.png" ;
  iconPaths[SOLARIS_ICON] = "icons/solaris.png" ;
  iconPaths[CLOUD_ICON] = "icons/cloud.png" ;
  iconPaths[HYPERVISOR_ICON] = "icons/hypervisor.png" ;
  iconPaths[APP_ICON] = "icons/application.png" ;
  iconPaths[WEB_ICON] = "icons/web.png";
  iconPaths[WEBSERVER_ICON] = "icons/web-server.png";
  iconPaths[DB_ICON] = "icons/db.png";
  iconPaths[DBSERVER_ICON] = "icons/db-server.png" ;
  iconPaths[PROCESS_ICON] = "icons/process.png";
  iconPaths[LOG_ICON] = "icons/log.png";
}


Ngrt4nServiceMap::~Ngrt4nServiceMap() {} ;

void Ngrt4nServiceMap::update(const Ngrt4nListServicesT & _bservices,
    const Ngrt4nListServicesT & _aservices,
    const double& _width,
    const double & _height)
{
  bservices = const_cast<Ngrt4nListServicesT*>(&_bservices) ;
  aservices = const_cast<Ngrt4nListServicesT*>(&_aservices);
  width = _width ;
  height = _height ;
  update() ; //TODO Make it dynamic
}

void Ngrt4nServiceMap::update(const bool & _init)
{
  if( _init ) {
      scaleX = layoutWidth/width ;
      scaleY = static_cast<double>(Ngrt4nYScreenScaling)/Ngrt4nXScreenScaling * scaleX ;
  }

  WPaintedWidget::update();
  WPaintedWidget::resize(width * scaleX, height * scaleY ) ;
}

void Ngrt4nServiceMap::paintEvent(Wt::WPaintDevice* _pdevice)
{
  painter = new WPainter(_pdevice) ;
  painter->scale(scaleX, scaleY) ;  //TODO Make it dynamic
  painter->setRenderHint(WPainter::Antialiasing) ;

  /* Draw node related to business services */
  for(Ngrt4nListServicesT::const_iterator it = bservices->begin() ; it != bservices->end() ; it++){
      drawNode(it->second) ;
  }

  /* Draw node related to alarm services */
  for(Ngrt4nListServicesT::const_iterator it = aservices->begin() ; it != aservices->end() ; it++){
      drawNode(it->second) ;
  }
}

/**
 * Draw edge before node to hide some details of drawing
 */
void Ngrt4nServiceMap::drawNode(const Ngrt4nServiceT & _service,
    const bool & _isBusinessService)
{

  WPointF posIcon(_service.map_x,  _service.map_y) ;
  WPointF posLabel(_service.map_x + 20, _service.map_y + 40) ; //20 = 40/2
  WPointF posNavIcon(_service.map_x + 9, _service.map_y + 46) ; //10 + 40

  WPen pen(Wt::gray) ;
  if( _service.status_info[NAGIOS_CRITICAL] ){
      pen.setColor(Wt::red) ;
  } else if( _service.status_info[NAGIOS_WARNING] ){
      pen.setColor(Wt::darkYellow) ;
  } else if( _service.status_info[NAGIOS_OK]){
      pen.setColor(Wt::darkGreen) ;
  }
  painter->setPen(pen) ;

  /* Draw the edge from parent.
   * Need to be drawn before the icon for hiding some technical details
   */
  if(_service.id != "root") {
      ServiceConstIteratorT pService = bservices->find(_service.parent) ; //should find it
      WPointF edgeP1(_service.map_x + 20, _service.map_y + 5) ;
      WPointF edgeP2(pService->second.map_x + 20, pService->second.map_y + 64) ; // 46 + 18
      painter->drawLine(edgeP1, edgeP2) ;
  }

  /* Draw icons and text
   */
  painter->drawImage(posIcon, WPainter::Image(iconPaths[_service.icon], 40, 40)) ;
  painter->drawText(posLabel.x(), posLabel.y(), WLength::Auto.toPixels(), WLength::Auto.toPixels(), AlignCenter, _service.name) ;
  if( _service.map_enable_nav_icon ) {
      string navIcon = "icons/built-in/map-nav-minus.png" ;
      painter->drawImage(posNavIcon, WPainter::Image(navIcon, 19, 18)) ;
  }
  createLink(_service) ;
}
void Ngrt4nServiceMap::createLink(const Ngrt4nServiceT & _service)
{
  ostringstream tip ;
  double x = _service.map_x * scaleX ;
  double y = _service.map_y * scaleY ;
  double width = 40.0 * scaleX ;
  double height = 60.0 * scaleY ;
  WRectArea *area = new WRectArea(x, y, width, height);

  tip << "Service : " << _service.name << " "
      << "\nStatus : " << Ngrt4nConfigParser::statusToString(_service.status_info) << " "
      << "\nDetails : " << _service.msg ;

  area->setToolTip(tip.str()) ;
  area->setLink("http://ngrt4n.com") ;
  addArea(area);
}
