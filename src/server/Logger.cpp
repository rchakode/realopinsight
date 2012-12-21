/*
 * Logger.cpp
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

#include "Logger.hpp"
#include <string.h>
#include <ostream>

Logger::Logger(std::string ident, int facility) {
    facility_ = facility;
    priority_ = LOG_DEBUG;
    strncpy(ident_, ident.c_str(), sizeof(ident_));
    ident_[sizeof(ident_)-1] = '\0';

    openlog(ident_, LOG_PID, facility_);
}

int Logger::sync() {
    if (buffer_.length()) {
        syslog(priority_, buffer_.c_str());
        buffer_.erase();
        priority_ = LOG_DEBUG; // default to debug for each message
    }
    return 0;
}

int Logger::overflow(int c) {
    if (c != EOF) {
        buffer_ += static_cast<char>(c);
    } else {
        sync();
    }
    return c;
}

std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority) {
    static_cast<Logger *>(os.rdbuf())->priority_ = (int)log_priority;
    return os;
}
