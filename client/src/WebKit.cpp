/*
 * Webkit.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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


#include "WebKit.hpp"
#include "Preferences.hpp"
#include "GuiDashboard.hpp"
#include <QAuthenticator>

WebKit::WebKit(const QString& _url, QWidget* _parent)
  :QWebView(_parent),
    m_urlpath(_url)
{
  settings()->setAttribute(QWebSettings::PluginsEnabled, true);
  settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
  settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
  load(QUrl(m_urlpath));
  addEvents();
}

WebKit::~WebKit() {}

void WebKit::setUrl(const QString& url )
{
  load(QUrl(url));
  m_urlpath = url;
}
void WebKit::handleLoadFinished(bool ok)
{
  if (!ok) {
    setHtml(tr("Unable to load the url (<a href='%1'>%2</a>)").arg(m_urlpath, m_urlpath));
  }
}

void WebKit::handleAuthenticationRequired(QNetworkReply*, QAuthenticator* authenticator)
{
  std::unique_ptr<GuiPreferences> form(new GuiPreferences(ngrt4n::OpUserRole, Preferences::BasicLoginForm));
  form->exec();
  if (! form->getCancelled()) {
    authenticator->setUser(form->getRealmLogin());
    authenticator->setPassword(form->getRealmPasswd());
  }
  form->setCancelled(false);
}


void WebKit::addEvents(void)
{
  connect(this, SIGNAL(loadFinished(bool)), this, SLOT(handleLoadFinished(bool)));
  connect(page()->networkAccessManager(), SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
          this, SLOT(handleAuthenticationRequired(QNetworkReply*, QAuthenticator*)));
}
