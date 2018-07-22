/*
 * TestK8sHelper.cpp
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
#include "TestK8sHelper.hpp"
#include "K8sHelper.hpp"
#include "utilsCore.hpp"
#include <QtTest/QtTest>
#include <QFile>
#include <QProcessEnvironment>

TestK8sHelper::TestK8sHelper()
{

}


void TestK8sHelper::initTestCase(void)
{
  m_TEST_DATA_DIR = QProcessEnvironment::systemEnvironment().value("TEST_DATA_DIR");
}

void TestK8sHelper::test_parseNamespaces(void)
{
  QFile nsFile(m_TEST_DATA_DIR + "./list-namespaces.json");

  QVERIFY(nsFile.open(QIODevice::ReadOnly));

  K8sHelper k8s;
  auto&& out = k8s.parseNamespaces(nsFile.readAll());
  QCOMPARE(out.second, true);
  QCOMPARE(out.first.size(), 8);
}

void TestK8sHelper::test_parseNamespacedServices(void)
{
  QFile nsFile(m_TEST_DATA_DIR + "./list-services.json");

  QVERIFY(nsFile.open(QIODevice::ReadOnly));

  K8sHelper k8s;
  NodeListT bpnodes;
  QMap<QString, QMap<QString, QString>> selectorMaps;
  auto&& out = k8s.parseNamespacedServices(nsFile.readAll(), "project1", selectorMaps, bpnodes);

  for(auto&& sm: selectorMaps.toStdMap()) {
    if (sm.first == "application1") {
      QCOMPARE(sm.second.size(), 2);
      QCOMPARE(sm.second.keys().size(), 2);
      QCOMPARE(QSet<QString>::fromList(sm.second.keys()).contains(QSet<QString>::fromList(QStringList(std::initializer_list<QString>{"stage", "app"}))), true);
    } else {
      QCOMPARE(sm.second.size(), 1);
      QCOMPARE(sm.second.keys().size(), 1);
      QCOMPARE(QSet<QString>::fromList(sm.second.keys()).contains(QSet<QString>::fromList(QStringList(std::initializer_list<QString>{"app"}))), true);
    }
    QCOMPARE(sm.second["app"], sm.first);
  }

  QCOMPARE(out.second, true);
  QCOMPARE(selectorMaps.size(), 7);
}



void TestK8sHelper::test_parseNamespacedPods(void)
{
  QFile nsFile(m_TEST_DATA_DIR + "./list-pods.json");

  QVERIFY(nsFile.open(QIODevice::ReadOnly));

  K8sHelper k8s;
  CoreDataT cdata;
  auto&& out = k8s.parseNamespacedPods(nsFile.readAll(), "project1", cdata);

  ngrt4n::saveDataAsDescriptionFile("/tmp/real.xml", cdata);

  QCOMPARE(out.second, true);
}



QTEST_MAIN(TestK8sHelper)
