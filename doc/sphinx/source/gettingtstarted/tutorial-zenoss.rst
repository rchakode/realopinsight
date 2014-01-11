=========================================================
Tutorial: using RealOpInsight with Zenoss
=========================================================

This chapter describes how to use RealOpInsight with Zenoss.

Prerequisites
-------------
This tutorial assumes that :

+ You read and understood the Concepts and the Architecture of
  RealOpInsight.
+ RealOpInsight is installed and ready to use.
+ Your Zenoss installation is running.


Use Case
--------

The tutorial is based on the example file
*small_hosting_platform.zns.ngrt4n.xml* provided with RealOpInsight. This file is located in the directory:


+ *SRC_DIR/examples* from source tree;
+ */usr/share/doc/packages/realopinsight/examples* if you installed
  the software from a Linux binary package
+ *C:\Program Files\NGRT4N\examples* (by default) in Windows systems.


In the rest of the tutorial, we'll refer to this folder as
EXAMPLE_DIR.


Check your Zenoss Installation
------------------------------
You need to ensure that:

+ The version of your Zenoss Server is greater than 3.2.
+ The Zenoss is running.



Configure your RealOpInsight Environment
----------------------------------------

Launch a terminal and start the RealOpInsight configuration wizard.

* On GNU/Linux and OS X Systems:

::

    $ ngrt4n-oc -c


* On Windows Systems:

::

    Start->NGRT4N Monitoring Suite->RealOpInsight Operations Console


Then follow the steps described in the configuration guide to configure your environment
properly.


Launch the Operations Console
-----------------------------

* On GNU/Linux and OS X Systems:

::

    $ ngrt4n-manager -d $EXAMPLE_DIR/small_hosting_platform.zns.ngrt4n.xml


* On Windows systems:

::

    Start->NGRT4N Monitoring Suite->Operations Console


You will be invited to select a configuration file. Browse through the
file selection window that shall appear to pick the configuration
file.

Use a valid user account to sign in the system. See the FAQ for the default user credentials.

According to the components assigned to the Zenoss server you will
have less or more alerts in the Operations Console. This example
simulates the monitoring of a hosting platform and relies on the
following components:nts:


+ Procesesses

    + httpd
    + zenping
    + mysqld

+ IP Services

    + http
    + https
    + ssh

+ And the Ping status of the Zenoss server.

If some of them are not set to the Zenoss server you can assign them
to see the effect in the Operations Console.

