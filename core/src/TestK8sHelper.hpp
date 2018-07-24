/*
 * TestK8sHelper.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation Date: July 2018                                                 #
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

#ifndef TESTK8SHELPER_H
#define TESTK8SHELPER_H

#include <QObject>

class TestK8sHelper : public QObject
{
  Q_OBJECT

public:
  TestK8sHelper();

private Q_SLOTS:
  void initTestCase(void);
  void test_parseNamespaces(void);
  void test_parseNamespacedServices(void);
  void test_parseNamespacedPods(void);
  void test_httpDataRetrieving(void);

private:
  QString m_TEST_DATA_DIR;
  QString m_PROXY_URL;
};

#endif // TESTK8SHELPER_H
