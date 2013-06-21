/*
 * Ngrt4nTypes.hpp
 *
 *  Created on: 25 mars 2012
 *      Author: chakode
 */

#ifndef NGRT4NTYPES_HPP_
#define NGRT4NTYPES_HPP_

#include <Wt/WString>
#include <Wt/WPaintedWidget>
#include <Wt/WStackedWidget>
#include <Wt/WPainter>
#include <Wt/WPaintDevice>
#include <Wt/WPointF>
#include <map>
#include <boost/unordered_map.hpp>

using namespace Wt ;
using namespace std ;

enum NagiosStatus
{
  NAGIOS_OK = 0,
  NAGIOS_WARNING = 1,
  NAGIOS_CRITICAL = 2,
  NAGIOS_UNKNOWN = 3,
  UNSET_STATUS = 4
} ;

enum ScreenScalingFactors
{
  Ngrt4nXScreenScaling = 72,
  Ngrt4nYScreenScaling = 100
};

enum Ngrt4nMapPadding
{
  Ngrt4nXPadding = 50,
  Ngrt4nYpadding = 50
};

enum NodeType
{
  Ngrt4nNormalService =  0 ,
  Ngrt4nAlarmService =  1,
  Ngrt4nRootService = -1
};

typedef struct Ngrt4nMonitoringInfo{
  string id;
  string host ;
  string check_command ;
  string last_state_change ;
  string alarm_msg ;
  int status ;
}Ngrt4nMonitoringInfoT;

typedef bitset<4> Ngrt4nStatusInfoT ;
typedef struct Ngrt4nService {
  int type ;
  string id;
  string name ;
  string icon ;
  string description ;
  string parent ;
  string propagation_rule ;
  string msg ;
  string notification_msg ;
  Ngrt4nStatusInfoT status_info ; /* for describing the the statuses of its child nodes*/
  int status_calc_rule ;
  double map_x ;
  double map_y ;
  double map_width ;
  double map_height ;
  bool map_enable_nav_icon ;
  bool map_expanded ;
  Ngrt4nMonitoringInfoT probe ;
  WStandardItem* navt_item ;
} Ngrt4nServiceT;

typedef struct Ngrt4nViewModels{
  WStandardItemModel* navt_mdl ;
  WStandardItemModel* mconsole_mdl ;
}Ngrt4nViewModelsT;

typedef boost::unordered_map<string, Ngrt4nServiceT> Ngrt4nListServicesT ;
typedef boost::unordered_map<string, Ngrt4nMonitoringInfoT> Ngrt4nListMonitoringInfoT ;

typedef struct Ngrt4nCoreDataStructure {
  Ngrt4nListServicesT bservices ;  /* List of business services */
  Ngrt4nListServicesT aservices ;  /*  List of alarm services */
  string graph_descr ;
  Ngrt4nListMonitoringInfoT probes ;
}Ngrt4nCoreDataStructureT ;

typedef Ngrt4nListServicesT::iterator ServiceIteratorT ;
typedef Ngrt4nListServicesT::const_iterator ServiceConstIteratorT ;
typedef boost::unordered_map<string,  WStandardItem*> Ngrt4nViewItemListT;
typedef boost::unordered_map<string, string> MapIconListT;
#endif /* NGRT4NTYPES_HPP_ */
