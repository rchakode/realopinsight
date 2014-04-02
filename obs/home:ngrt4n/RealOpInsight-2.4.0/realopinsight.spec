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
    
%define binprefix     ngrt4n
    
Name:           RealOpInsight
Summary:    	Dashboard Software for Nagios, Zabbix, Zenoss, Icinga, Shinken, Centreon, etc
Version:        2.4.0
Release:        build<CI_CNT>.<B_CNT>
Group:          System/Monitoring
License:        GPLv3
URL:            http://ReolOpInsight.com
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %_tmppath/%name-%version-build

%if 0%{?centos_version}
BuildRequires:  libzmq3
%else
BuildRequires:  zeromq-devel
%endif

%if 0%{?suse_version} >= 1140
BuildRequires:  gcc-c++ libqt4-devel >= 4.6.3 libQtWebKit4 libQtWebKit-devel
%endif

%if 0%{?fedora}
BuildRequires:  gcc-c++ qt >= 4.6.3 qt4-devel qt-webkit-devel
%endif

%if 0%{?centos_version}
BuildRequires:  gcc-c++ qt-devel qtwebkit
%endif

%if 0%{?suse_version} >= 1140
Requires:  graphviz
%endif
%if 0%{?fedora}
Requires: graphviz qt >= 4.6.3 qtwebkit glibc libgcc >= 4.3 libstdc++
%endif

%description
RealOpInsight (http://RealOpInsight.com/) is an advanced dashboard management
engine for common open source monitoring systems including Nagios, Zabbix, Icinga,
GroundWork, Centreon, Shinken® and op5 Monitor.

The Solution brings Novel Concepts along with a Powerful Technology that allow
operators to be effective in challenging operating environments such as

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
%{_prefix}/bin/%{binprefix}-manager
%{_prefix}/bin/%{binprefix}-oc
%{_prefix}/bin/%{binprefix}-editor
%doc %{_prefix}/share/man/man1/%{binprefix}-manager.1.gz
%doc %{_prefix}/share/man/man1/%{binprefix}-oc.1.gz
%doc %{_prefix}/share/man/man1/%{binprefix}-editor.1.gz
%doc README COPYING ChangeLog
%doc examples

%changelog
* Tue Nov 27 2012 bugs@ngrt4n.com
- See the web site (http://RealOpInsight.com) to learn what changed in the different versions of the software
