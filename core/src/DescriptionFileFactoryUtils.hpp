/*
* DescriptionFileFactoryUtils.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Creation : 26-06-2015                                                    #
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

#ifndef DESCRIPTIONFILEFACTORYUTILS_H
#define DESCRIPTIONFILEFACTORYUTILS_H

#include "Base.hpp"

namespace ngrt4n {
  int importHostGroupAsMap(const SourceT& srcInfo, const QString& filter, CoreDataT& cdata, QString& errorMsg);
  int saveDataAsDescriptionFile(const QString& path, const CoreDataT& cdata, QString& errorMsg);
  QString generateNodeXml(const NodeT & node);
}

#endif // DESCRIPTIONFILEFACTORYUTILS_H
