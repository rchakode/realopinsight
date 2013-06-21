/*
 * Ngrt4nDashboard.hpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */

#ifndef NGRT4NDASHBOARD_HPP_
#define NGRT4NDASHBOARD_HPP_

#include <Wt/WApplication>
#include <Wt/WGridLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WMenu>
#include <Wt/WMenuItem>
#include <Wt/WStackedWidget>
#include "include/Ngrt4nServiceTree.hpp"
#include "include/Ngrt4nMsgConsole.hpp"
#include "include/Ngrt4nServiceMap.hpp"
#include <Wt/WLocalizedStrings>

using namespace Wt ;

class Ngrt4nDashboard : public WApplication
{
public:
  Ngrt4nDashboard(const WEnvironment& env);
  virtual ~Ngrt4nDashboard();
  void loadConfig(std::string _cfg) ;

private:
  Ngrt4nServiceTree *navTree ;
  Ngrt4nServiceMap *map ;
  Ngrt4nMsgConsole *msgConsole ;
  Ngrt4nCoreDataStructureT data ;
  WStandardItem* navTreeRoot ;
  double mapWidth ;
  double mapHeight ;

  WLayout* createMenuBar(void) ;
  WContainerWidget* createMenuBarWidget(void) ;
  bool buildNavTreeModel() ;
  bool computeMapCoordinates(Ngrt4nCoreDataStructureT & _cdata) ;
  void updateViews() ;
  void updateServicesStatuses() ;
  void updateParentStatus(const Ngrt4nServiceT& _service) ;
  void updateServiceTree(void) ;
};

#endif /* NGRT4NDASHBOARD_HPP_ */
