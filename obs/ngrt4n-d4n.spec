# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 02-12-2012                                                 #
#                                                                          #
# This Software is part of NGRT4N Project (http://ngrt4n.com).             #
#                                                                          #
# This is a free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#	                                                                   #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty or           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with the Software.  If not, see <http://www.gnu.org/licenses/>.	   #
#--------------------------------------------------------------------------#


Name:           ngrt4n-d4n
Summary:    	This package provides the service for retrieving data from Nagios to RealOpInsight
Version:        1.1.1
Release:        build<CI_CNT>.<B_CNT>
Group:          System/Monitoring
License:        GPLv3
URL:            http://ReolOpInsight.com
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %_tmppath/%name-%version-build
  
BuildRequires:  gcc-c++ autoconf m4 automake zeromq-devel

%description
This package provides the service for retrieving data from Nagios to RealOpInsight.

RealOpInsight (http://RealOpInsight.com/) is an advanced dashboard management
engine for common open source monitoring systems including Nagios, Zabbix, Icinga,
GroundWork, Centreon, Shinken® and op5 Monitor.

%prep
%setup -q

%build
#TODO

%install 
./install-sh -d %{buildroot}%{_prefix}

%clean
make clean
rm -rf %{buildroot}

%post
#TODO

%postun
#TODO

%files
%defattr(-,root,root)
%{_prefix}/sbin/ngrt4nd
%dir %{_prefix}/share/man/mang/
%{_prefix}/share/man/mang/ngrt4nd.1.gz
%{_prefix}/share/man/man1/ngrt4nd.1.1.gz
%doc README COPYING ChangeLog
%doc examples

%changelog
* Tue Nov 27 2012 bugs@ngrt4n.com
- See the web site (http://RealOpInsight.com) to learn what changed in the different versions of the software
