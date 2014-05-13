# ------------------------------------------------------------------------------- #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)           #
# Last Update : 02-04-2014                                                        #
#                                                                                 #
# This Software is part of RealOpInsight (http://realopinsight.com).              #
#                                                                                 #
# This is a free software: you can redistribute it and/or modify                  #
# it under the terms of the GNU General Public License as published by            #
# the Free Software Foundation, either version 3 of the License, or               #
# (at your option) any later version.                                             #
#	                                                                          #
# The Software is distributed in the hope that it will be useful,                 #
# but WITHOUT ANY WARRANTY; without even the implied warranty or                  #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	                  #
# GNU General Public License for more details.                                    #
#                                                                                 #
# You should have received a copy of the GNU General Public License               #
# along with the Software.  If not, see <http://www.gnu.org/licenses/>.	          #
#---------------------------------------------------------------------------------#
    
%define binprefix     realopinsight
    
Name:           realopinsight-workstation
Summary:    	Workstation Edition of RealOpInsight Software
Version:        X.Y.Z
Release:        build<CI_CNT>.<B_CNT>
Group:          System/Monitoring
License:        GPLv3
URL:            http://ReolOpInsight.com
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %_tmppath/%name-%version-build


BuildRequires:  gcc-c++  

%if 0%{?centos_version}
BuildRequires:  libzmq3
%else
BuildRequires:  zeromq-devel
%endif

%if 0%{?suse_version} >= 1140
BuildRequires:  libqt4 >= 4.6.3 libqt4-devel libQtWebKit4 libQtWebKit-devel
%endif

%if 0%{?fedora}
BuildRequires:  libgcc glibc gcc-c++ libstdc++ qt >= 4.6.3 qt4-devel
BuildRequires:  qtwebkit qtwebkit qt-webkit-devel
%endif


BuildRequires: graphviz
Requires:  graphviz

%description
RealOpinsight is an open source business service monitoring 
dashboard software that enables IT operations staff to deal 
with monitoring with focus on business. Not a monitor, 
RealOpInsight provides specialized capabilities for 
monitoring the real healthy of business services and cloud 
applications. 

It supports Nagios, Zabbix, Zenoss, Icinga, op5, Centreon, 
Shinken, GroundWork, and many other common monitoring 
systems. 

%prep
%setup -q

%build
#TODO

%install
bash ./install-sh.obs -d %{buildroot}%{_prefix}

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
%doc README LICENSE ChangeLog
%doc examples

%changelog
* Tue May 13 2014 bugs@realopinsight.com
- See the web site (http://RealOpInsight.com) to learn what changed in the different versions of the software
