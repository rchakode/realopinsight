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
#	                                                                              #
# The Software is distributed in the hope that it will be useful,                 #
# but WITHOUT ANY WARRANTY; without even the implied warranty or                  #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	                  #
# GNU General Public License for more details.                                    #
#                                                                                 #
# You should have received a copy of the GNU General Public License               #
# along with the Software.  If not, see <http://www.gnu.org/licenses/>.	          #
#---------------------------------------------------------------------------------#
     
%define binprefix          realopinsight
%define package_basename   realopinsight-workstation
%define distrib_info       generic
%define target_version     X.Y.Z
%define tarball_basename   %{package_basename}-%{target_version}
%define tarball            %{tarball_basename}.tar.gz

%if 0%{?fedora_version}
  %define distrib_info fedora%{?fedora_version}
%endif

%if 0%{?suse_version}
  %define distrib_info    suse%{?suse_version}
%endif
 
%if 0%{?rhel_version}
  %define distrib_info    rhel%{?rhel_version}
%endif

%if 0%{?centos_version}
  %define distrib_info    centos%{?centos_version}
%endif

 
Name:           %{package_basename}
Summary:    	Workstation Edition of RealOpInsight Software
Version:        %{target_version}
Release:        %{distrib_info}.built<CI_CNT>.<B_CNT>
Group:          System/Monitoring
License:        GPLv3
URL:            http://ReolOpInsight.com
Source0:        %{tarball}
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
%setup -q -n %{tarball_basename}

%build
# NA

%install
%if 0%{?rhel_version} || 0%{?centos_version}
  bash ./install-sh -d %{buildroot}%{_prefix} -z
%else
  bash ./install-sh -d %{buildroot}%{_prefix}
%endif

%clean
make clean
rm -rf %{buildroot}

%post
# NA

%postun
# NA

%files
%defattr(-,root,root)
%{_prefix}/bin/%{binprefix}-manager
%{_prefix}/bin/%{binprefix}-oc
%{_prefix}/bin/%{binprefix}-editor
%doc %{_prefix}/share/man/man1/%{binprefix}-manager.1.gz
%doc %{_prefix}/share/man/man1/%{binprefix}-oc.1.gz
%doc %{_prefix}/share/man/man1/%{binprefix}-editor.1.gz
%doc README.md LICENSE ChangeLog
%doc examples

%changelog
* Sun Sep 21 2014 bugs@realopinsight.com
- See http://RealOpInsight.com to learn what's news in your version of the software
