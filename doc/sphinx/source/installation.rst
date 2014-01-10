
This document covers the installation of RealOpInsight. For the
configuration, see `here`_.


Prerequisites & Dependencies
----------------------------

Herein we assume that you have read and understood the `RealOpInsight
Concepts`_, or at least the section describing its `architecture`_.

To install and run the software properly, your system needs to satisfy
the following dependencies:


+ libstdc++ >= 4.6.x
+ g++ (build requirements)
+ make (build requirements)
+ Qt >= 4.6.3 (Required modules: QtCore, QtGui, QtNetwork, QtWebkit,
  QtXml, QtScript); Headers and libraries are required.
+ ZeroMQ 2.x or 3.x. Note that ZeroMQ 3.x is only supported since
  RealOpInsight 2.2.0; Header files are requied for source compilation.
+ Graphviz >= 2.20.2


For Windows users, the setup program comprises all the dependencies.

On Linux, these dependencies are included in the major distributions
and can be easily installed through the package manager (yum, apt-get,
zypper, etc). The names of packages may vary slightly from a
distribution to another, see below for the installation process on
Fedora, openSUSE, Debian and Ubuntu.

On OS X, you may need to install `MacPorts`_ to compile RealOpInsight.
This `article`_, written by `Richard Gate`_, would help you throughout
the installation. Note however that all hacks presented in the article
are now fixed in RealOpInsight, since the version 2.4.0. So you don't
normally need to hack.


Installing the dependencies on Fedora
`````````````````````````````````````

::

    $ sudo yum install gcc-c++ glibc libgcc libstdc++ make qt4-devel \
           qt-webkit-devel zeromq-devel graphviz



Installing the dependencies on openSUSE
```````````````````````````````````````

::

    $ sudo zypper install gcc-c++ libstdc++ make libqt4-devel \
           libQtWebKit libQtWebKit-devel zeromq-devel graphviz



Installing the dependencies on Debian and Ubuntu
````````````````````````````````````````````````

::

    $ sudo apt-get install g++ libzmq1 libzmq-dev libqt4-dev \
           libqt4-webkit graphviz



Download
--------
Go to the `download page`_ and get an appropriate tarball or binary
package.


Installation
------------

Once the download completed, go to the suitable installation section:


+ `Installation from source`_
+ `Installation from binaries`_



Installation from sources
~~~~~~~~~~~~~~~~~~~~~~~~~
Perform the following commands:

::

    $ tar zxf RealOpInsight-X.Y.Z.tar.gz   # replace X.Y.Z by your version
    $ cd RealOpInsight-X.Y.Z
    $ ./install-sh                         # requires root privileges 


This will install the programs in */usr/local*. This default location
can be altered through the *-d* switch, type *install-sh -h* for more
details.

Note: If you're using zeromq 2.x and c++11, a compiling issue (
*error: ‘swap’ is not a member of ‘std’*) has been reported. The
solution is to patch zmq.hpp by adding #include <algorithm> in the
header. See `here`_ for more details.


Installation from binary packages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here we assumed that you already installed all the dependencies and
also that you already downloaded a suitable binary package for
RealOpInsight.


Installation on Windows
```````````````````````

The setup program comprises the binaries and their dependencies, just
launch the setup.

Once installed, the programs are accessible from *Start -> NGRT4N
Monitoring Suite*.


Installation on Fedora and on openSUSE
``````````````````````````````````````

::

    $ sudo rpm -i /path/to/realopinsight.rpm


Replace */path/to/realopinsight.rpm* with the path of the binary
package (e.g. *realopinsight-2.1.0-opensuse12_2.x86_64.rpm*).


Installation on Ubuntu
``````````````````````

::

    $ sudo dpkg -i /path/to/realopinsight.deb


Replace */path/to/realopinsight.deb* with the path of the binary
package (e.g. *realopinsight-2.1.0-i386.deb*).


Check the Installation
----------------------
RealOpInsight is comprised of three programs:

+ `ngrt4n-manager`_: The main program enabling full control over
  RealOpInsight.
+ `ngrt4n-oc`_: The Operations Consoles Utility
+ `ngrt4n-editor`_: The Editor Utility


Each of the programs has a help (option -h), as well as a man page.
Use any of them requires user credentials.


User Credentials
----------------

RealOpInsight relies on two default user profiles:
Operator User (Limited Privileges)

+ Login: *ngrt4n_op*
+ Default password: *ngrt4n_op*


Administrator User (Full Privileges)

+ Login: *ngrt4n_adm*
+ Default password: *ngrt4n_adm*




Next Steps: `Configuring RealOpInsight`_ | `Operations Console User's
Manual`_` `_| `Editor User's Manual`_

