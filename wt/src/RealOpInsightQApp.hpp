/*
 * RealOpInsightQApp.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 06-10-2014                                                 #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */

#ifndef REALOPINSIGHTQAPP_HPP
#define REALOPINSIGHTQAPP_HPP
#include <QApplication>
#include "WebUtils.hpp"



class RealOpInsightQApp : public QCoreApplication
{
public:
  RealOpInsightQApp(int& argc, char ** argv) : QCoreApplication(argc, argv) { }
  virtual ~RealOpInsightQApp() { }

  virtual bool notify(QObject * receiver, QEvent * event) {
    try {
      return QCoreApplication::notify(receiver, event);
    } catch(std::exception& ex) {
      LOG("fatal", ex.what());
    }
    return false;
  }
};


#endif // REALOPINSIGHTQAPP_HPP
