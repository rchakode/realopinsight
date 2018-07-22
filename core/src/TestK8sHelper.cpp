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
  QFile nsesDataFile(m_TEST_DATA_DIR + "/list-namespaces.json");

  QVERIFY(nsesDataFile.open(QIODevice::ReadOnly));

  K8sHelper k8s;
  auto&& out = k8s.parseNamespaces(nsesDataFile.readAll());
  QCOMPARE(out.second, true);
  QCOMPARE(out.first.size(), 8);
}

void TestK8sHelper::test_parseNamespacedServices(void)
{
  QFile servicesDataFile(m_TEST_DATA_DIR + "list-services.json");

  QVERIFY(servicesDataFile.open(QIODevice::ReadOnly));

  K8sHelper k8s;
  NodeListT bpnodes;
  QMap<QString, QMap<QString, QString>> serviceSelectorInfos;
  auto&& out = k8s.parseNamespacedServices(servicesDataFile.readAll(), "project1", serviceSelectorInfos, bpnodes);

  QCOMPARE(out.second, true);
  QCOMPARE(serviceSelectorInfos.size(), 7);
  QCOMPARE(bpnodes.size(), 7);

  for(auto&& sm: serviceSelectorInfos.toStdMap()) {
    if (sm.first == "application1") {
      QCOMPARE(sm.second.size(), 2);
      QCOMPARE(sm.second.keys().size(), 2);
      QCOMPARE(QSet<QString>::fromList(sm.second.keys()).contains(QSet<QString>::fromList({"stage", "app"})), true);
    } else {
      QCOMPARE(sm.second.size(), 1);
      QCOMPARE(sm.second.keys().size(), 1);
      QCOMPARE(QSet<QString>::fromList(sm.second.keys()).contains(QSet<QString>::fromList({"app"})), true);
    }
    QCOMPARE(sm.second["app"], sm.first);
  }

}


void TestK8sHelper::test_parseNamespacedPods(void)
{
  QFile servicesDataFile(m_TEST_DATA_DIR + "list-services.json");

  QVERIFY(servicesDataFile.open(QIODevice::ReadOnly));

  K8sHelper k8s;
  NodeListT serviceBpnodes;
  QMap<QString, QMap<QString, QString>> serviceSelectorInfos;
  auto&& outServices = k8s.parseNamespacedServices(servicesDataFile.readAll(), "project1", serviceSelectorInfos, serviceBpnodes);

  QCOMPARE(outServices.second, true);

  QFile podsDataFile(m_TEST_DATA_DIR + "/list-pods.json");

  QVERIFY(podsDataFile.open(QIODevice::ReadOnly));

  NodeListT podBpnodes;
  NodeListT podCnodes;
  auto&& outPods = k8s.parseNamespacedPods(podsDataFile.readAll(), "project1", serviceSelectorInfos, podBpnodes, podCnodes);

  //    NodeT rnode;
  //    rnode.id =ngrt4n::ROOT_ID;
  //    rnode.parent = "";
  //    rnode.name = *k8sNamespaces.begin();
  //    rnode.type = NodeType::BusinessService;
  //    rnode.sev_prule = PropRules::Unchanged;
  //    rnode.sev_crule = CalcRules::Worst;
  //    rnode.weight = ngrt4n::WEIGHT_UNIT;
  //    rnode.icon = ngrt4n::DEFAULT_ICON;
  //    rnode.description = "";
  //ngrt4n::saveDataAsDescriptionFile("/tmp/real.xml", cdata);

  QCOMPARE(outPods.second, true);
}



QTEST_MAIN(TestK8sHelper)
