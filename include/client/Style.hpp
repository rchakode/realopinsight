/*
 * Style.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 7 juin 2012                                                #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#ifndef STYLE_HPP_
#define STYLE_HPP_

#include <QtCore>

namespace client {

QString styleSheet =
		"QMenuBar, QMenu {"
		"	background: #2d2d2d ; "
		"	color : white; "
		"}"
		"QMenuBar {"
		"	font-weight : bold ;"
		"}"
		"QToolBar {"
		"	background: #9dc6dd;"
		"	padding: 0px;"
		"	height: 16px;"
		"	"
		"}"
		"QHeaderView::section {"
		"	background: #2d2d2d;"
		"	color : white; "
		"	font-weight : bold ;"
		"}"
		"QTableView {"
		"	background:  lightgray;"
		"}"
		"QTabWidget::pane { /* The tab widget frame */"
		"	border-top: 2px solid #C2C7CB;"
		"	background-color: #9dc6dd;"
		"}"
		"QTabWidget::tab-bar {"
		"	background-color: #9dc6dd;"
		"}"
		"QTabBar::tab {"
		"	background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
		"	stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
		"	stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
		"	border: 2px solid #C4C4C3;"
		"	border-bottom-color: #C2C7CB; /* same as the pane color */"
		"	border-top-left-radius: 4px;"
		"	border-top-right-radius: 4px;"
		"	min-width: 8ex;"
		"	padding: 2px;"
		"}"
		"QTabBar::tab:selected {"
		"	background: #9dc6dd;"
		"}"
		"QTabBar::tab:hover {"
		"	background: #3589b9;"
		"}"
		"QTabBar::tab:selected {"
		"	border-color: #9B9B9B;"
		"	border-bottom-color: #C2C7CB; /* same as pane color */"
		"}"
		"QTabBar::tab:!selected {"
		"	margin-top: 2px; /* make non-selected tabs look smaller */"
		"}"
		;

}
#endif /* STYLE_HPP_ */
