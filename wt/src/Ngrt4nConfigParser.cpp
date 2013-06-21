/*
 * Ngrt4nConfigParser.cpp
 *
 *  Created on: 25 mars 2012
 *      Author: chakode
 */

#include "Ngrt4nConfigParser.hpp"
#include <fstream>


/* Services with empty id are ignored */
bool Ngrt4nConfigParser::loadNgrt4nConfig(const string & _file_name,
    Ngrt4nCoreDataStructureT & _core_data_struct)
{
  string graph_content ;
  try
  {
      XMLPlatformUtils::Initialize() ;
      XercesDOMParser* xml_parser = new XercesDOMParser() ;
      xml_parser->setValidationScheme(XercesDOMParser::Val_Always) ;
      xml_parser->setDoNamespaces(true) ;

      ErrorHandler* error_handler = (ErrorHandler*) new HandlerBase() ;
      xml_parser->setErrorHandler(error_handler) ;

      ifstream file(_file_name.c_str(), ifstream::in) ;
      if (! file.good()) {
          cerr << WString::tr("config.file.doesnot.exist") << endl ;
          return false ;
      }

      xml_parser->parse(_file_name.c_str()) ;

      DOMDocument *config_xml_doc = xml_parser->getDocument() ;
      DOMNodeList* xml_service_list = config_xml_doc->getChildNodes() ;

      XMLSize_t count = xml_service_list->getLength() ;
      if(count != 1) {
          cerr << WString::tr("invalid.config.file") << endl ;
          return false ;
      }

      DOMNode* xml_root_node = xml_service_list->item(0) ;
      xml_service_list = xml_root_node->getChildNodes() ;
      count = xml_service_list->getLength() ;
      for (XMLSize_t i = 0; i < count; i++)
        {
          DOMNode* service = xml_service_list->item(i); if(!service->getNodeType()) continue ;
          string node_name = XMLString::transcode(service->getNodeName() ) ;

          if(node_name == "Service"){
              Ngrt4nServiceT node ;

              DOMElement* elt = dynamic_cast<DOMElement*>(service) ;
              node.id = Ngrt4nConfigParser::getXmlAttribute(elt, "id") ;
              node.status_calc_rule = atoi(Ngrt4nConfigParser::getXmlAttribute(elt, "statusCalcRule")) ;
              node.type = atoi(Ngrt4nConfigParser::getXmlAttribute(elt, "type")) ;

              if(node.id == "") continue ;

              DOMNodeList*  property_list = service->getChildNodes() ;
              XMLSize_t count_p = property_list->getLength();
              for (XMLSize_t j = 0; j < count_p; j++)
                {
                  DOMNode* property = property_list->item(j) ; if(! property->getNodeType() ) continue ;
                  string property_name = XMLString::transcode(property->getNodeName() );

                  if ( property_name == "Name" ){
                      node.name =XMLString::transcode(property->getTextContent()) ;
                  }
                  else if( property_name == "Parent" ){
                      node.parent =XMLString::transcode(property->getTextContent()) ;
                  }
                  else if( property_name == "Icon" ){
                      node.icon =XMLString::transcode(property->getTextContent()) ;
                  }
                  else if ( property_name == "Description" ){
                      node.description =XMLString::transcode(property->getTextContent()) ;
                  }
                  else if ( property_name == "PropagationRule" ){
                      node.propagation_rule =XMLString::transcode(property->getTextContent()) ;
                  }
                  else if ( property_name == "AlarmMsg" ){
                      node.msg =XMLString::transcode(property->getTextContent()) ;
                  }
                  else if ( property_name == "NotificationMsg" ){
                      node.notification_msg =XMLString::transcode(property->getTextContent()) ;
                  }
                  else if (property_name == "SubServices" ){
                      if( node.type != Ngrt4nAlarmService ) {
                          continue ;
                          /*
                           * TODO
                         cerr << WString::tr("bad.definition.non.probe.service")
                          << "ID : " << node.id << "Name : " << node.name << endl ;
                          return false ;
                           */
                      }
                      node.probe.id = XMLString::transcode(property->getTextContent()) ;
                  }
                } //end inner loop (traverse service properties)

              if(node.parent == "") {
                  if( node.id != "root") {
                      std::cerr << WString::tr("bad.definition.non.root.service")
                      << "ID : " << node.id << "Name : " << node.name << endl ;
                      return false ;
                  }
              }

              if (node.type == Ngrt4nAlarmService ){
                  _core_data_struct.aservices.insert(std::pair<std::string, Ngrt4nServiceT>(node.id, node)) ;
              } else {
                  _core_data_struct.bservices.insert(std::pair<std::string, Ngrt4nServiceT>(node.id, node)) ;
              }

              if( node.id != "root") {
                  _core_data_struct.graph_descr =  node.id + "[label=\""+ node.name + "\"]\n"
                      + _core_data_struct.graph_descr + node.parent + "--" + node.id + "\n" ;
              }else {
                  _core_data_struct.graph_descr =  node.id + "[label=\""+ node.name + "\"]\n" ;
              }


          } // end if service
        } // end outer loop (traverse service)

      delete xml_parser ;
      delete error_handler ;
      XMLPlatformUtils::Terminate();
  }
  catch (const XMLException& ex)
  {
      char* message = XMLString::transcode(ex.getMessage());
      cerr << "Exception : \n" << message << "\n";
      XMLString::release(&message);
      return false ;
  }
  catch (DOMException& ex)
  {
      char* message = XMLString::transcode(ex.getMessage());
      cerr << "Exception : \n" << message << "\n";
      XMLString::release(&message);

      return false ;
  }
  catch (...)
  {
      cerr << "Unknown fatal error ! "<< endl;
      return false ;
  }

  return true;
}


string Ngrt4nConfigParser::statusToString(const int& _status)
{
  switch(_status)
  {
  case NAGIOS_OK:
    return "Normal";
    break;

  case NAGIOS_WARNING:
    return  "Warning";
    break;

  case NAGIOS_CRITICAL:
    return  "Critical";
    break;

  default:
    return "Unknown";
    break;
  }

  return "Unknown";
}

string Ngrt4nConfigParser::statusToString(const Ngrt4nStatusInfoT& _status)
{

  if( _status == NAGIOS_OK){
      return "Normal";
  } else  if( _status ==  NAGIOS_WARNING){
      return  "Warning";
  }  if( _status ==  NAGIOS_CRITICAL){
      return  "Critical";
  }

  return "Unknown";
}


bool Ngrt4nConfigParser::loadNagiosCollectedData(const string & _sfile, Ngrt4nListServicesT & _aservices)
{
  string line, check_type;
  Ngrt4nMonitoringInfoT info;

  /* First make a snapshot of the status file before treat it ; */
  string snapshot = "/tmp/status.dat.snap" ;
  FILE* stFile = fopen(_sfile.c_str(), "rt") ;
  FILE* fSnapshot =  fopen(snapshot.c_str(), "wt") ;

  if( stFile == NULL || fSnapshot == NULL ){
      cerr << "Unable to access to check the status file : " << _sfile << endl;
      return false ;
  }

  fseek(stFile, 0, SEEK_END) ; size_t size = ftell(stFile) ; rewind(stFile) ;

  char* buffer = (char*)malloc(size * sizeof(char)) ;

  size_t nbRead = fread(buffer, 1, size, stFile);
  if(nbRead != size){
      cerr << "Error while reading the status file : " << _sfile << endl;
      return false ;
  }

  size_t nbWrite = fwrite(buffer, 1, size, fSnapshot);

  if(nbWrite != size){
      cerr << "Error while creating a snapshot of the status file : " << endl;
      return false ;
  }

  fcloseall() ; // End of the copy

  /* Now start parsing */
  ifstream stFileStream ;
  stFileStream.open(snapshot.c_str(), std::ios_base::in) ;
  if (! stFileStream.good() ) {
      cerr << "Unable to access the snapshot of the the status file " << endl ;
      return false ;
  }

  while (getline(stFileStream, line) , ! stFileStream.eof()) {

      if(line.find("#") != string::npos ) continue ;

      if( line.find("hoststatus") == string::npos &&
          line.find("servicestatus") == string::npos ) continue ;

      info.status = UNSET_STATUS ;
      while (getline(stFileStream, line), ! stFileStream.eof()) {

          size_t pos = line.find("}") ; if( pos != string::npos ) break ;
          pos = line.find("=") ; if(pos == string::npos) continue ;

          string param = boost::trim_copy(line.substr(0, pos));
          string value = boost::trim_copy(line.substr(pos+1, string::npos)) ;

          if(param == "host_name") {
              info.host =
                  info.id =
                      boost::trim_copy(line.substr(pos+1)) ;
          }
          else if(param == "service_description") {
              info.id += "/" + value ;
          }
          else if(param == "check_command") {
              info.check_command = value ;
          }
          else if(param == "current_state") {
              info.status = atoi(value.c_str()) ;
          }
          else if(param == "last_state_change") {
              info.last_state_change = value ;
          }
          else if(param == "plugin_output")
            {
              info.alarm_msg = value;
            }
      }
      for(ServiceIteratorT it = _aservices.begin(); it != _aservices.end() ; it ++){
          if( info.id == it->second.probe.id) {
              it->second.status_info.reset() ; it->second.status_info[info.status] = true ;

              if( it->second.msg == "")it->second.msg =info.alarm_msg ;
              it->second.probe = info ;
              break ;
          }
      }

  }
  stFileStream.close() ;

  return true;
}
