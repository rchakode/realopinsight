/*
 * WebLicenseActivation.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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

#ifndef WEBLICENSEACTIVATION_HPP
#define WEBLICENSEACTIVATION_HPP
#include "WebPreferencesBase.hpp"
#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>

class WebLicenseActivation : public WebPreferencesBase, public Wt::WTemplate
{
public:
  WebLicenseActivation();
  void saveActivationKey(void);
  bool isActivated(const QString& version);

private:
  Wt::WLineEdit* m_activationKeyField;
  Wt::WPushButton* m_activeBtn;

  QString genLicenseKey(const QString& hostid, const QString& hostname, const QString& version);
  bool checkLicenseKey(const QString& key, const QString& version);
  bool isValidKey(const QString& key, const QString& hostid, const QString& hostname, const QString& version);
  QString getHostId(void);

};

#endif // WEBLICENSEACTIVATION_HPP
