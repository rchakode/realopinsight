/*
 * WebLicenseActivation.cpp
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

#include "WebLicenseActivation.hpp"
#include "WebUtils.hpp"
#include <QHostInfo>

WebLicenseActivation::WebLicenseActivation()
  : WebPreferencesBase(),
    Wt::WTemplate(Wt::WString::tr("license-activation-form.tpl"))
{
  bindWidget("activation-key-field", m_activationKeyField = new Wt::WLineEdit());
  bindWidget("activate-button", m_activeBtn = new Wt::WPushButton(Q_TR("Activate")));
  m_activeBtn->clicked().connect(this, &WebLicenseActivation::saveActivationKey);
}

void WebLicenseActivation::saveActivationKey(void)
{
  setKeyValue(Settings::ACTIVATION_LICENSE_KEY, m_activationKeyField->text().toUTF8().c_str());
  sync();
}

QString WebLicenseActivation::genKey(const QString& hostid, const QString& hostname, const QString& version)
{
  QCryptographicHash hasher(QCryptographicHash::Md5);
  hasher.addData(hostid.toLatin1());
  hasher.addData(hostname.toLatin1());
  hasher.addData(version.toLatin1());
  QString key = "";
  QByteArray md5 = hasher.result().toHex().toUpper();
  for (int i = 0; i < md5.length(); ++i) {
    if ((i != 0) && (i % 4 == 0))
      key.append("-");
    key += md5[i];
  }
  return key;
}

bool WebLicenseActivation::isActivated(const QString& version)
{
  return checkLicenseKey(value(Settings::ACTIVATION_LICENSE_KEY, ""), version);
}


bool WebLicenseActivation::checkLicenseKey(const QString& key, const QString& version)
{
  return isValidKey(key, getHostId(), QHostInfo::localHostName(), version);
}


bool WebLicenseActivation::isValidKey(const QString& key, const QString& hostid, const QString& hostname, const QString& version)
{
  return key == genKey(hostid, hostname, version);
}


QString WebLicenseActivation::getHostId(void)
{
  return QString::number(static_cast<unsigned int>(gethostid()), 16);
}
