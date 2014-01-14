.. _install-ngrt4nd

Installation of ngrt4nd
========================
Ngrt4nd is a daemon that offers the service of retrieving status data 
from a Nagios installation. It relies on the Nagios's status.dat file. And
thus, due the disc I/O that this approach involves, it's not performant and is less scalable 
than using Livestatus. Hence its use is mainly appropriate for testing purpose, notably to 
quickly have an overview on how RealOpInsight works. 

This document assumes you have read and understood the :doc:`basic concepts of 
RealOpInsight <../overview/concepts>`.


Prerequisites and Dependencies
------------------------------

To install and run the software properly, your system needs to satisfy
the following dependencies:


+ g++
+ make
+ libstdc++ >= 4.6.x
+ zeromq (libraries and development files, tested with zeromq 2.x)
+ libcrypt (if you are using GNU/Linux systems, that should be already
  installed)


These software are included in the major GNU/Linux distributions and
can be easily installed through the package manager (yum, apt-get,
zypper...). The package names vary slightly from a distribution to
another, see below the installation process for Fedora, openSUSE and
Ubuntu.


Install the dependencies on Fedora
----------------------------------

::

    $ sudo yum install gcc-c++ libstdc++ make 
    zeromq-devel



Install the dependencies on openSUSE
````````````````````````````````````

::

    $ sudo zypper install gcc-c++ 
    libstdc++ make 
    zeromq-devel



Install the dependencies on Ubuntu
``````````````````````````````````

::

    $ sudo apt-get install libstdc++6 libzmq1 
    libqt4-dev



Download
--------

Go to the `download page`_ and get a appropriate tarball or binary
package.


Installation
------------

Once the download completed, go to the suitable installation section
according to that you're using sources or binary packages.

Installation from sources
``````````````````````````````````

Once the dependencies installed as mentioned previously, perform the
following steps:

::

    $ tar zxf 
    ngrt4n-d4n-X.Y.Z.tar.gz      # replace X.Y.Z by the suitable 
    version
    $ cd ngrt4n-d4n-X.Y.Z
    $./install-sh     # requires root privileges 


This installs the program in /usr/local/ *sbin* by default. An
alternative location can be set using the -d switch, type *install-sh
-h* for more details.

.. note::

  If you're using zeromq 2.x and c++11, a compiling issue (
  *error: ‘swap’ is not a member of ‘std’*) has been reported [#F1]_. The
  solution is to patch zmq.hpp by adding #include <algorithm> in the
  header.


Installation from binary packages
``````````````````````````````````

Here we assumed that you already installed all the dependencies and
also that you already downloaded a suitable binary package of the
Daemon Service.


Installation on Fedora and on openSUSE
``````````````````````````````````````

::

    $ sudo rpm -i /path/to/ngrt4n-d4n.rpm


Replace */path/to/ *ngrt4n-d4n*.rpm* with the path to the binary
package (e.g. * *ngrt4n-d4n-1.1.0*-opensuse12_2.x86_64.rpm*).


Installation on Ubuntu
``````````````````````

::

    $ sudo dpkg -i /path/to/ngrt4n-d4n.deb


Replace */path/to/realopinsight.deb* with the path to the binary
package (e.g. *ngrt4n-d4n-1.1.0-i386.deb*).


Check the Installation
----------------------
Daemon Service is comprised of one command line program named
*ngrt4nd*. As standard UNIX commands, it has a help page (ngrt4nd -h)
as well as a man page.


Set the Authentication Token
----------------------------

Perform the following command *as root*. You'll be prompted to enter a
passphrase.

::

    $ ngrt4nd -P



Start the Service
-----------------

Once the authentication token is set, you can start the service by
performing the following command.

::

    $ ngrt4nd    # requires root privileges


This runs the program in daemon mode assuming that the path to the
Nagios Status File is */usr/local/var/lib/status. dat *and that the
listening port is *1983*. These default parameters can be overridden
using appropriate options. See the help ( *ngrt4nd -h*) or the man
page for more details.

Important Note: If you have a firewall between the monitoring server
and the operator workstation, you may need to update your filtering
rules for enabling traffic on the listing port (1983, if you haven't
changed the default port).


.. [#F1] https://zeromq.jira.com/browse/LIBZMQ-342
.. _download page: http://realopinsight.com/en/index.php?page=download 
