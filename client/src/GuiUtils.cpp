/*
#  GuiUtils.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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
#include "GuiUtils.hpp"
#include "utilsClient.hpp"
#include "StatsLegend.hpp"
#include <QMessageBox>

QSplashScreen* ngrt4n::infoScreen(const QString & msg) {
  QSplashScreen* screen = new QSplashScreen(QPixmap(":/images/built-in/loading-screen.png"));
  screen->showMessage(msg, Qt::AlignJustify|Qt::AlignVCenter);
  screen->show();
  return screen;
}

QColor ngrt4n::severityQColor(const int& _criticity)
{
  QColor color(ngrt4n::COLOR_UNKNOWN);
  switch (static_cast<ngrt4n::SeverityT>(_criticity)) {
    case ngrt4n::Normal:
      color = ngrt4n::COLOR_NORMAL;
      break;
    case ngrt4n::Minor:
      color = ngrt4n::COLOR_MINOR;
      break;
    case ngrt4n::Major:
      color = ngrt4n::COLOR_MAJOR;
      break;
    case ngrt4n::Critical:
      color = ngrt4n::COLOR_CRITICAL;
      break;
    default:
      break;
  }
  return color;
}

void ngrt4n::alert(const QString& msg)
{
  QMessageBox::warning(0, QObject::tr("%1 - Warning").arg(APP_NAME), msg, QMessageBox::Yes);
}

QIcon ngrt4n::severityIcon(int _severity)
{
  return QIcon(":/"+ngrt4n::getIconPath(_severity));
}

QString ngrt4n::getWelcomeMsg(const QString& utility)
{
  return QObject::tr("       > %1 %2 %3 (codename: %4)"
                     "\n        >> Realease ID: %5"
                     "\n        >> Copyright (C) 2010 - %6 RealOpInsight Labs. All rights reserved"
                     "\n        >> For bug reporting instructions, see: <%7>").arg(APP_NAME,
                                                                                   utility,
                                                                                   PKG_VERSION,
                                                                                   REL_NAME,
                                                                                   REL_INFO,
                                                                                   REL_YEAR,
                                                                                   PKG_URL);
}


QString ngrt4n::basename(const char* path)
{
  char* name = strrchr(path, '/');
  name = name ? name + 1 : const_cast<char*>(path);
  return QString(name);
}
