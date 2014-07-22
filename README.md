RealOpInsight is an open source business service monitoring dashboard
toolkit that enables IT operations staff to deal with monitoring with
focus on business. It supports Nagios, Zenoss, Zabbix, Shinken, Centreon,
Naemon, Icinga, op5, and more.


RealOpInsight Workstation is released under the terms of GPLv3 License.
See the LICENSE file.Prior to the installation, you may need to read
the license offer.
 

INSTALLATION
=
This component needs to be installed on the user workstation.

Dependencies
==

- gcc-c++ >= 4.6.3
- Qt >= 4.6.3 (required modules : QtCore, QtGui, QtSvg, QtWebKit, QtXml, QtScript)
- libzmq (librairies and development files)
- graphviz
 
Installation
==

```
  $ tar zxf RealOpInsightX.Y.Y.tar.gz  # X.Y.Z refers to the version
  $ cd RealOpInsight-X.Y.Z
  $ sudo ./install-sh
```
