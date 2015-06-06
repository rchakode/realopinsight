/*
 * WebLicenseActivation.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 12-04-2015                                                     #
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
# ------------------------------------------------------------------------ #
*/

#ifndef WEBLICENSEACTIVATION_HPP
#define WEBLICENSEACTIVATION_HPP
#include "WebPreferencesBase.hpp"
#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <climits>



class LicenseActivationBase : public WebPreferencesBase
{
public:
  enum PackageT {
    UltimateStarter = 8,
    UltimateSmallBusiness = 20,
    UltimateMiddleBusiness = 50,
    UltimateEnterprise = 100,
    UltimateCorportate = 1000000
  };

  LicenseActivationBase(const QString& version);
  std::string lastError() const {return m_lastError.toStdString(); }
  void checkInstanceActivationLevel(void);
  bool isActivatedInstance(void) const {return m_licenseLevel > UltimateStarter;}
  static QString genKey(const QString& hostid, const QString& hostname, const QString& version, int licenseOffer);
  bool canHandleNewView(int currentViewCount, int servicesCount);
  int maxAllowedSources(void) const;
  std::string licenseOfferName(void) const;


private:
  int m_licenseLevel;
  QString m_version;
  QString m_lastError;

  bool checkKey(const QString& key, const QString& version, int package);
  bool isValidKey(const QString& key,
                  const QString& hostid, const QString& hostname,
                  const QString& version, int package);
  static QString getHostId(void);
};



class WebLicenseManager : public LicenseActivationBase, public Wt::WTemplate
{
public:
  WebLicenseManager(const QString& version);
  void saveActivationKey(void);
  void updateContent(void) {m_activationKeyField->setText(getLicenseKey().toStdString());}
  Wt::Signal<int, std::string>& licenseKeyChanged(void) {return m_licenseSetStatus;}

private:
  Wt::Signal<int, std::string> m_licenseSetStatus;
  Wt::WLineEdit* m_activationKeyField;
  Wt::WPushButton* m_activeBtn;
};

#endif // WEBLICENSEACTIVATION_HPP
