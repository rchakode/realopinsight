/*
 * Auth.cpp
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


#include "Auth.hpp"
#include "Preferences.hpp"
#include "utilsClient.hpp"
#include "GuiUtils.hpp"
#include <QLabel>
#include <QPushButton>

Auth::Auth()
  : QDialog(),
    settings (new Settings())
{
  setWindowTitle(tr("%1 - Login").arg(APP_NAME));
  m_layout = new QGridLayout(this);
  qint32 line = 0;
  QPixmap logo(":images/built-in/logo.png");
  QLabel* llogo =  new QLabel(); llogo->setPixmap(logo);
  m_layout->addWidget(llogo, line, 0, 1, 3, Qt::AlignLeft);
  line++;
  m_layout->addWidget(new QLabel(tr("Version %1 (%2)").arg(PKG_VERSION, REL_NAME)),
                    line, 0, 2, 1, Qt::AlignLeft);
  line++;
  m_layout->addWidget(new QLabel(tr("Login")), line, 1, Qt::AlignRight);
  m_layout->addWidget(m_loginField = new QLineEdit(ngrt4n::OpUser.c_str()), line, 2, Qt::AlignJustify);
  line++;
  m_layout->addWidget(new QLabel(tr("Password")), line, 1, Qt::AlignRight);
  m_layout->addWidget(m_passwordField = new QLineEdit(), line, 2, Qt::AlignJustify);
  m_passwordField->setEchoMode(QLineEdit::Password);
  line++;
  m_layout->addWidget(m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok), line, 1, 1, 3, Qt::AlignRight);
  m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Sign in"));
  m_buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
  line++; QString copying = tr("\nCopyright (c) 2010-%1 NGRT4N Project. All rights reserved.").arg(REL_YEAR);
  m_layout->addWidget(new QLabel(copying), line, 0, 1, 3, Qt::AlignLeft);
  addEvents();
}

Auth::~Auth()
{
  delete m_loginField;
  delete m_passwordField;
  delete m_layout;
}


void Auth::cancel(void)
{
  exit(1);
}


void Auth::authentificate(void)
{
  QString userName = m_loginField->text();
  QString userPasswd = QCryptographicHash::hash(ngrt4n::toByteArray(m_passwordField->text()), QCryptographicHash::Md5);
  QString rootPasswd =  settings->setEntry(Settings::ADM_PASSWD_KEY);
  QString opPasswd =  settings->setEntry(Settings::OP_PASSWD_KEY);
  if(	! rootPasswd.isEmpty()
      && userName == QString::fromStdString(ngrt4n::AdmUser)
      && userPasswd == rootPasswd ) {
    done(AdmUserRole);
  } else if( !opPasswd.isEmpty()
             && userName == QString::fromStdString(ngrt4n::OpUser)
             && userPasswd == opPasswd ) {
    done(OpUserRole);
  } else {
    ngrt4n::alert(tr("Authentication failed: wrong username or password"));
  }
}


void Auth::addEvents(void)
{
  connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(authentificate()));
}
