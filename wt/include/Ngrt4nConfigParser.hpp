/*
 * Ngrt4nConfigParser.hpp
 *
 *  Created on: 25 mars 2012
 *      Author: chakode
 */

#ifndef NGRT4NCONFIGPARSER_HPP_
#define NGRT4NCONFIGPARSER_HPP_

#include <string>
#include <boost/algorithm/string.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include "Ngrt4nCoreDataStructures.hpp"
#include <boost/lexical_cast.hpp>


using namespace xercesc ;

class Ngrt4nConfigParser
{
public:
  static bool loadNgrt4nConfig(const string & _file_name, Ngrt4nCoreDataStructureT & _core_data_structure) ;
  static bool loadNagiosCollectedData(const string & _sfile, Ngrt4nListServicesT & _aservices) ;
  static string statusToString(const int& _status) ;
  static string statusToString(const Ngrt4nStatusInfoT&) ;

  static char* getXmlAttribute(xercesc::DOMElement *_elt, const std::string & _attr_name) {
    return XMLString::transcode(_elt->getAttribute(XMLString::transcode(_attr_name.c_str())));
  }

};

#endif /* NGRT4NCONFIGPARSER_HPP_ */
