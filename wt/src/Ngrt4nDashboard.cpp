/*
 * Ngrt4nDashboard.cpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */
#include <fstream>
#include <iostream>
#include <Wt/WPanel>
#include <Wt/WPointF>
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WImage>
#include <Wt/WScrollArea>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "Ngrt4nDashboard.hpp"
#include "Ngrt4nConfigParser.hpp"
#include "Ngrt4nCoreDataStructures.hpp"

Ngrt4nDashboard::Ngrt4nDashboard(const WEnvironment& env)
: WApplication(env),
  navTree(new Ngrt4nServiceTree()),
  map(new Ngrt4nServiceMap()),
  msgConsole(new Ngrt4nMsgConsole())
{
  WContainerWidget* mainContainer =  new WContainerWidget() ;
  WContainerWidget* leftContainer = new WContainerWidget() ;
  WContainerWidget* rightContainer = new WContainerWidget() ;
  WContainerWidget* mapContainer = new WContainerWidget() ;
  WContainerWidget* msgContainer = new WContainerWidget() ;
  WVBoxLayout* mainLayout = new WVBoxLayout() ;
  WHBoxLayout* centralLayout = new WHBoxLayout() ;
  WVBoxLayout* leftLayout = new WVBoxLayout() ;
  WVBoxLayout* rightLayout = new WVBoxLayout() ;
  WVBoxLayout* mapLayout = new WVBoxLayout() ;
  WVBoxLayout* msgLayout = new WVBoxLayout() ;
  WScrollArea* mapScArea = new WScrollArea();
  WPanel* treePanel = new WPanel() ;
  WPanel* mapPanel = new WPanel() ;
  WPanel* msgPanel = new WPanel() ;

  mainContainer->setStyleClass("container") ;
  leftContainer->setStyleClass("container") ;
  rightContainer->setStyleClass("container") ;
  mapContainer->setStyleClass("container") ;
  msgContainer->setStyleClass("container") ;

  mainLayout->setContentsMargins(0, 0, 0, 0) ;
  centralLayout->setContentsMargins(0, 0, 0, 0) ;
  leftLayout->setContentsMargins(0, 0, 0, 0) ;
  rightLayout->setContentsMargins(0, 0, 0, 0) ;
  mapLayout->setContentsMargins(0, 0, 0, 0) ;
  msgLayout->setContentsMargins(0, 0, 0, 0) ;

  centralLayout->setSpacing(1) ;
  rightLayout->setSpacing(1) ;

  treePanel->setTitle(WString::tr("service.tree.title")) ;
  treePanel->setCentralWidget(navTree) ;
  leftLayout->addWidget(treePanel) ;
  leftContainer->setLayout(leftLayout) ;
  leftContainer->setOverflow(WContainerWidget::OverflowAuto) ;
  leftContainer->resize(250, mainContainer->height()) ;

  mapPanel->setTitle(WString::tr("service.map.title")) ;
  mapScArea->setWidget(map) ;
  mapLayout->addWidget(mapScArea) ;
  mapContainer->setLayout(mapLayout) ;
  mapPanel->setCentralWidget(mapContainer) ;

  msgPanel->setTitle(WString::tr("msg.panel.title")) ;
  msgLayout->addWidget(msgConsole) ;
  msgContainer->setLayout(msgLayout) ;
  msgPanel->setCentralWidget(msgContainer) ;
  msgConsole->sizeChanged().connect(map, &Ngrt4nServiceMap::msgPanelSizedChanged) ;

  rightContainer->setLayout(rightLayout) ;
  rightLayout->addWidget(mapPanel) ;
  rightLayout->addWidget(msgPanel) ;
  rightLayout->setStretchFactor(mapPanel, 3) ;
  rightLayout->setStretchFactor(msgPanel, 2) ;

  centralLayout->addWidget(leftContainer, 1) ;
  centralLayout->addWidget(rightContainer, 3) ;

  mainLayout->addWidget(createMenuBarWidget(), 0);
  mainLayout->addLayout(centralLayout, 2) ;
  mainContainer->setLayout(mainLayout) ;
  root()->addWidget(mainContainer) ;

  centralLayout->setResizable(0) ;
  centralLayout->setResizable(1) ;
  rightLayout->setResizable(0) ;
  rightLayout->setResizable(1) ;

  loadConfig("hosting-platform") ;
}

Ngrt4nDashboard::~Ngrt4nDashboard()
{
  delete navTree ;
  delete map ;
  delete msgConsole ;
}


void Ngrt4nDashboard::loadConfig(string _cfg)
{
  bool result = Ngrt4nConfigParser::loadNgrt4nConfig(WApplication::appRoot()+"config/" + _cfg + ".xml", data) ;

  if( !result ) return ;  // Invalid config file

  if( !buildNavTreeModel() ){
      cerr << WString::tr("unable.to.build.view").arg("view") << endl;
      return ;
  }

  if(! computeMapCoordinates(data) ){
      cerr << WString::tr("unable.to.build.view").arg("map") << endl;
      return ;
  }

  updateViews() ;
}


bool Ngrt4nDashboard::buildNavTreeModel()
{
  Ngrt4nViewItemListT nav_tree_items ; /* Store the set of items */

  /* Create a item for each individual service */
  for(ServiceIteratorT service  = data.bservices.begin(); service != data.bservices.end(); ++service) {
      service->second.navt_item = Ngrt4nServiceTree::createItem(service->second) ;
      nav_tree_items.insert(pair<string, WStandardItem*>(service->second.id, service->second.navt_item)) ;
  }

  for(ServiceIteratorT service  = data.aservices.begin(); service != data.aservices.end(); ++service) {
      service->second.navt_item = Ngrt4nServiceTree::createItem(service->second) ;
      nav_tree_items.insert(pair<string, WStandardItem*>(service->second.id, service->second.navt_item)) ;
  }

  /* Organize an hierarchy according to the mapping of services */
  for(ServiceIteratorT service  = data.bservices.begin(); service != data.bservices.end(); ++service) {
      if(service->second.id == "root") {
          navTreeRoot = service->second.navt_item ;
      } else {
          Ngrt4nViewItemListT::iterator p_it = nav_tree_items.find(service->second.parent) ;
          p_it->second->appendRow(service->second.navt_item); /* Normally the search would have succeeded, according to the parsing step */
      }
  }

  for(ServiceIteratorT service  = data.aservices.begin(); service != data.aservices.end(); ++service) {
      Ngrt4nViewItemListT::iterator p_it = nav_tree_items.find(service->second.parent) ;
      p_it->second->appendRow(service->second.navt_item); /* Normally the search would have succeeded, according to the parsing step */
  }

  return true ;
}


bool Ngrt4nDashboard::computeMapCoordinates(Ngrt4nCoreDataStructureT & _cdata)
{
  char* dotFileName = strdup("/tmp/ngrt4n-XXXXXX") ;
  int fd = mkstemp(dotFileName) ;
  if( fd == -1){
      cerr << "Unable to temporary file generate the graph file " << endl ;
      return false ;
  }
  close(fd) ;

  ofstream ofile ;
  ofile.open(dotFileName, ios_base::out) ;
  if (! ofile.good() ) {
      cerr << "Unable to generate some necessary temporary files. "
          << "Check that the directory tmp/ has the rights 777" << endl ;
      return false ;
  }
  ofile<<"strict graph  ngrt4n {\n"
      <<"node[shape=plaintext]\n"
      << _cdata.graph_descr
      << "}";
  ofile.close() ;

  ostringstream dotCoordinates ;
  dotCoordinates << dotFileName << "_1" ;

  ostringstream cmd ;
  cmd << "dot -Tplain -o "
      << dotCoordinates.str()
      << " " << dotFileName ;

  if ( system(cmd.str().c_str()) != 0 ) {
      cerr << "Unable to run 'dot'" << endl;
      return false ;
  }

  ifstream ifile ;
  ifile.open(dotCoordinates.str().c_str(), ios_base::in) ;
  if (! ifile.good() ) {
      cerr << "Unable to use the generated graph file " << dotCoordinates.str() << endl ;
      return false ;
  }

  string line ;
  vector<string> fields ;

  if(getline(ifile, line), ! ifile.eof()) { //Parse the header of the generated dot file
      boost::split(fields, line, boost::is_any_of(" "), boost::token_compress_on) ;
      if (fields[0] != "graph") {
          cerr << "The syntax of the generated graph file is wrong " << dotCoordinates.str() << endl ;
          return false ;
      }
      mapWidth = atof(fields[2].c_str()) * Ngrt4nXScreenScaling + Ngrt4nXPadding;
      mapHeight = atof(fields[3].c_str()) * Ngrt4nYScreenScaling + Ngrt4nYpadding;
  }

  while(getline(ifile, line), ! ifile.eof()){ //Parse the rest of the generated dot file
      vector<string> fields ;
      boost::split(fields, line, boost::is_any_of(" "), boost::token_compress_on) ;

      if( ! fields.size() ) continue ;
      if (fields[0] == "node") {
          ServiceIteratorT service ;
          if( service = _cdata.bservices.find(fields[1]),
              service == _cdata.bservices.end()) {

              if( service = _cdata.aservices.find(fields[1]),
                  service == _cdata.aservices.end() )  continue ;
          }
          service->second.map_x = atof(fields[2].c_str()) * Ngrt4nXScreenScaling ;
          service->second.map_y = mapHeight
              - atof(fields[3].c_str()) * Ngrt4nYScreenScaling - (Ngrt4nYpadding / 2)  //Coordinate transformation
              - 40 ;  //Icon size
          service->second.map_enable_nav_icon = (service->second.type == Ngrt4nAlarmService)? false: true ;

      } else if(fields[0] == "stop") {
          break;
      }
      else { //edge
          continue ;
      }
      fields.clear() ;
  }
  ifile.close() ;
  free(dotFileName) ;

  return true ;
}

void Ngrt4nDashboard::updateViews(void)
{
  updateServicesStatuses() ;
  msgConsole->update(data.aservices) ;
  map->update(data.bservices, data.aservices, mapWidth, mapHeight) ;
  navTree->update(navTreeRoot) ; updateServiceTree() ;
}

void Ngrt4nDashboard::updateServicesStatuses()
{
  if (! Ngrt4nConfigParser::loadNagiosCollectedData("examples/status.dat", data.aservices) ) return ;

  for(ServiceIteratorT sIt  = data.bservices.begin(); sIt != data.bservices.end(); ++sIt) {
      sIt->second.status_info.reset() ;
  }

  for(ServiceIteratorT sIt  = data.aservices.begin(); sIt != data.aservices.end(); ++sIt) {
      if(sIt->second.id != "root") {
          ServiceIteratorT pIt = data.bservices.find(sIt->second.parent) ;
          pIt->second.status_info |= sIt->second.status_info ;
          updateParentStatus(pIt->second) ;
      }
  }
}

void Ngrt4nDashboard::updateParentStatus(const Ngrt4nServiceT & _service)
{
  ServiceIteratorT pIt = data.bservices.find(_service.parent) ;
  pIt->second.status_info |= _service.status_info ;
  if(pIt->second.id != "root") updateParentStatus(pIt->second) ;
}

void Ngrt4nDashboard::updateServiceTree(void)
{
  string icon ;

  for(ServiceIteratorT sIt  = data.bservices.begin(); sIt != data.bservices.end(); ++sIt) {
      icon = "icons/built-in/unknown.png" ;
      if( sIt->second.status_info[NAGIOS_CRITICAL] ){
          icon = "icons/built-in/critical.png" ;
      } else if(  sIt->second.status_info[NAGIOS_WARNING] ){
          icon = "icons/built-in/warning.png" ;
      } else if(  sIt->second.status_info[NAGIOS_OK]){
          icon = "icons/built-in/normal.png" ;
      }
      sIt->second.navt_item->setIcon(icon) ;
  }

  for(ServiceIteratorT sIt  = data.aservices.begin(); sIt != data.aservices.end(); ++sIt) {
      icon = "icons/built-in/unknown.png" ;
      if(  sIt->second.status_info[NAGIOS_CRITICAL] ){
          icon = "icons/built-in/critical.png" ;
      } else if(  sIt->second.status_info[NAGIOS_WARNING] ){
          icon = "icons/built-in/warning.png" ;
      } else if(  sIt->second.status_info[NAGIOS_OK]){
          icon = "icons/built-in/normal.png" ;
      }
      sIt->second.navt_item->setIcon(icon) ;
  }
}


WLayout* Ngrt4nDashboard::createMenuBar(void)
{
  WHBoxLayout *menu_box = new WHBoxLayout() ;

  menu_box->setSpacing(0) ; menu_box->setContentsMargins(0, 0, 0, 0) ;
  WPushButton *b ;

  b =  new WPushButton(WString::tr("refresh.menu.text")) ;
  b->setIcon("icons/built-in/refresh.png") ;
  menu_box->addWidget(b, 0);

  b =  new WPushButton(WString::tr("zoom.in")) ;
  b->setIcon("icons/built-in/zoomin.png") ;
  menu_box->addWidget(b, 0);

  b =  new WPushButton(WString::tr("zoom.out")) ;
  b->setIcon("icons/built-in/zoomout.png") ;
  menu_box->addWidget(b, 0);

  b =  new WPushButton(WString::tr("save.map.menu.text")) ;
  b->setIcon("icons/built-in/disket.png") ;
  menu_box->addWidget(b, 0);

  b =  new WPushButton(WString::tr("help.text")) ;
  b->setIcon("icons/built-in/help.png") ;
  menu_box->addWidget(b, 0);

  b =  new WPushButton(WString::tr("disconnect.text")) ;
  b->setIcon("icons/built-in/logout.png") ;
  menu_box->addWidget(b, 0);

  return menu_box ;
}

WContainerWidget* Ngrt4nDashboard::createMenuBarWidget(void)
{
  WContainerWidget* menu_bar = new WContainerWidget() ;
  WHBoxLayout *layout = new WHBoxLayout() ; layout->setSpacing(0) ;
  menu_bar->setStyleClass("menubar") ;
  layout->setSpacing(0) ; menu_bar->setPadding(0, All) ; layout->setContentsMargins(0, 0, 0, 0) ;

  WPushButton *b ;
  b =  new WPushButton(WString::tr("refresh.menu.text")) ;
  b->setIcon("icons/built-in/refresh.png") ;
  b->setStyleClass("button") ;
  layout->addWidget(b, 0);

  b =  new WPushButton(WString::tr("zoom.in")) ;
  b->setIcon("icons/built-in/zoomin.png") ;
  b->setStyleClass("button") ;
  layout->addWidget(b, 0);

  b =  new WPushButton(WString::tr("zoom.out")) ;
  b->setIcon("icons/built-in/zoomout.png") ;
  b->setStyleClass("button") ;
  layout->addWidget(b, 0);

  b =  new WPushButton(WString::tr("save.map.menu.text")) ;
  b->setIcon("icons/built-in/disket.png") ;
  b->setStyleClass("button") ;
  layout->addWidget(b, 0);

  b =  new WPushButton(WString::tr("help.text")) ;
  b->setIcon("icons/built-in/help.png") ;
  b->setStyleClass("button") ;
  layout->addWidget(b, 0);

  b =  new WPushButton(WString::tr("disconnect.text")) ;
  b->setIcon("icons/built-in/logout.png") ;
  b->setStyleClass("button") ;
  layout->addWidget(b, 0);

  menu_bar->setLayout(layout) ;
  return menu_bar ;
}


