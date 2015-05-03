/*
 * WebLicenseActivationBase.cpp
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
#--------------------------------------------------------------------------#
 */

#include "WebLicenseManager.hpp"
#include "WebUtils.hpp"
#include <QHostInfo>


namespace {
  int MAX_IT_SERVICES_PER_VIEW_STARTER = 50;
}

LicenseActivationBase::LicenseActivationBase(const QString& version)
  : WebPreferencesBase(),
    m_licenseLevel(UltimateStarter),
    m_version(version),
    m_lastError("")
{
  checkInstanceActivationLevel();
}

QString LicenseActivationBase::genKey(const QString& hostid, const QString& hostname, const QString& version, int licenseOffer)
{
  QCryptographicHash hasher(QCryptographicHash::Md5);
  hasher.addData(hostid.toLatin1());
  hasher.addData(hostname.toLatin1());
  hasher.addData(QString::number(licenseOffer).toLatin1());
  if (version.length() > 4) {
    hasher.addData(version.mid(0, 4).toLatin1());
  } else {
    hasher.addData(version.toLatin1());
  }
  QString serialKey = "";
  QByteArray md5 = hasher.result().toHex().toUpper();
  for (int i = 0; i < md5.length(); ++i) {
    if ((i != 0) && (i % 4 == 0))
      serialKey.append("-");
    serialKey += md5[i];
  }
  return serialKey;
}


bool LicenseActivationBase::checkKey(const QString& key, const QString& version, int package)
{
  return isValidKey(key, getHostId(), QHostInfo::localHostName(), version, package);
}


void LicenseActivationBase::checkInstanceActivationLevel(void)
{
  QString licenseKey = getLicenseKey();
  if (checkKey(licenseKey, m_version, UltimateSmallBusiness))
    m_licenseLevel = UltimateSmallBusiness;
  else if (checkKey(licenseKey, m_version, UltimateMiddleBusiness))
    m_licenseLevel = UltimateMiddleBusiness;
  else if (checkKey(licenseKey, m_version, UltimateEnterprise))
    m_licenseLevel = UltimateEnterprise;
  else if (checkKey(licenseKey, m_version, UltimateCorportate))
    m_licenseLevel = UltimateCorportate;
  else
    m_licenseLevel = UltimateStarter;
}

bool LicenseActivationBase::isValidKey(const QString& key,
                                       const QString& hostid, const QString& hostname,
                                       const QString& version, int package)
{
  return key == genKey(hostid, hostname, version, package);
}


QString LicenseActivationBase::getHostId(void)
{
  return QString::number(static_cast<unsigned int>(gethostid()), 16);
}



bool LicenseActivationBase::canHandleNewView(int currentViewCount, int newItServicesCount)
{
  bool success = false;
  if (m_licenseLevel == UltimateStarter  && newItServicesCount > MAX_IT_SERVICES_PER_VIEW_STARTER) {
    m_lastError = QObject::tr("The Starter license offer can handle up to %1 IT services. "
                              " Purchase a license key online: http://realopinsight.com/"
                              ).arg(QString::number(MAX_IT_SERVICES_PER_VIEW_STARTER));
  } else {
    success = (currentViewCount < m_licenseLevel);
    if (! success) {
      m_lastError = QObject::tr("Your license offer is limited to %1 views."
                                " Purchase a license key online: http://realopinsight.com/"
                                ).arg(QString::number(m_licenseLevel));
    }
  }

  return success;
}


int LicenseActivationBase::maxAllowedSources(void) const
{
  int result = 1;
  switch(m_licenseLevel) {
    case UltimateCorportate:
      result = MAX_SRCS;
      break;
    case UltimateEnterprise:
      result = 5;
      break;
    case UltimateMiddleBusiness:
      result = 2;
      break;
    case UltimateSmallBusiness:
      result = 1;
      break;
    case UltimateStarter:
    default:
      break;
  }
  return result;
}



WebLicenseManager::WebLicenseManager(const QString& version)
  : LicenseActivationBase(version),
    Wt::WTemplate(Wt::WString::tr("license-activation-form.tpl"))
{
  bindWidget("activation-key-field", m_activationKeyField = new Wt::WLineEdit());
  bindWidget("activate-button", m_activeBtn = new Wt::WPushButton(Q_TR("Activate")));
  m_activeBtn->clicked().connect(this, &WebLicenseManager::saveActivationKey);
}


void WebLicenseManager::saveActivationKey(void)
{
  setKeyValue(Settings::ACTIVATION_LICENSE_KEY, m_activationKeyField->text().toUTF8().c_str());
  sync();
}
