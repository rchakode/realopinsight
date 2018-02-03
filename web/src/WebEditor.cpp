/*
 * WebEditor.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)        #
# Last Update : 03-01-2018                                                 #
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

#include "WebUtils.hpp"
#include "WebEditor.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <QDebug>
#include <Wt/WApplication>
#include <Wt/WPanel>
#include <Wt/WPointF>
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WImage>
#include <Wt/WTemplate>



namespace {
  const IconMapT ICONS = ngrt4n::nodeIcons();
}

WebEditor::WebEditor(void) {

  bindFormWidgets();
}

WebEditor::~WebEditor()
{
  unbindWidgets();
}


void  WebEditor::openDescriptionFile(const QString& path)
{
//TODO: m_tree.setCoreData(&m_cdata);
}

void WebEditor::bindFormWidgets(void)
{
  setLayout(m_mainLayout = new Wt::WHBoxLayout());

  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  m_leftSubMainLayout.setContentsMargins(0, 0, 0, 0);
  m_rightSubMainLayout.setContentsMargins(0, 0, 0, 0);


  m_leftSubMainLayout.addWidget(&m_tree);
   //FIXME: bind the editor widget instead of the map one => m_rightSubMainLayout.addWidget(m_map.getWidget());
  m_mainLayout->addLayout(&m_leftSubMainLayout);
  m_mainLayout->addLayout(&m_rightSubMainLayout);

  m_mainLayout->setSpacing(2);
  m_mainLayout->setResizable(0);
  m_mainLayout->setResizable(1);
}


void WebEditor::unbindWidgets(void)
{
  m_leftSubMainLayout.removeWidget(&m_tree);
   //FIXME: unbing editor widget of the map one => m_rightSubMainLayout.removeWidget(m_map.getWidget());
  m_mainLayout->removeItem(&m_leftSubMainLayout);
  m_mainLayout->removeItem(&m_rightSubMainLayout);
  clear();
}


