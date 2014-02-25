/*
 * Auth.hpp
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

#ifndef SNAVAUTH_HPP_
#define SNAVAUTH_HPP_
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QSettings>
#include "Base.hpp"
#include "Settings.hpp"

class Auth : public QDialog
{
  Q_OBJECT
public:
  Auth();
  virtual ~Auth();

  enum RoleT {
    AdmUserRole = 100,
    OpUserRole = 101
  };

public Q_SLOTS:
  void cancel(void) ;
  void authentificate(void) ;


private:
  QDialogButtonBox* m_buttonBox;
  QLineEdit* m_loginField;
  QLineEdit* m_passwordField;
  QGridLayout* m_layout;
  Settings* settings;

  void addEvents(void);
};

#endif /* SNAVAUTH_HPP_ */
