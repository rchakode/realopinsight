/*
 * Logger.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 22-12-2012                                                 #
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

#ifndef LOGGER_H
#define LOGGER_H

#include <syslog.h>
#include <string>
#include <streambuf>

enum LogPriority {
    kLogEmerg   = LOG_EMERG,   // system is unusable
    kLogAlert   = LOG_ALERT,   // action must be taken immediately
    kLogCrit    = LOG_CRIT,    // critical conditions
    kLogErr     = LOG_ERR,     // error conditions
    kLogWarning = LOG_WARNING, // warning conditions
    kLogNotice  = LOG_NOTICE,  // normal, but significant, condition
    kLogInfo    = LOG_INFO,    // informational message
    kLogDebug   = LOG_DEBUG    // debug-level message
};

std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);

class Logger : public std::basic_streambuf<char, std::char_traits<char> > {
public:
    explicit Logger(std::string ident, int facility);

protected:
    int sync();
    int overflow(int c);

private:
    friend std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);
    std::string buffer_;
    int facility_;
    int priority_;
    char ident_[50];
};

#endif // LOGGER_H
