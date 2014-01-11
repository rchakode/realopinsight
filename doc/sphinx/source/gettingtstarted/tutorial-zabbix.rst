==========================================================
Tutorial: using RealOpInsight with Zabbix
==========================================================
This tutorial shows how to use RealOpInsight with Zabbix.


Prerequisites
-------------
This tutorial assumes that :

+ You read and understood the `concepts and the architecture`_ of
  RealOpInsight.
+ RealOpInsight is installed and ready to use.
+ Your Zabbix Server as well as its `JSON-RPC API`_ service are
  running.



Use Case
--------

The tutorial is based on the example files provided with
RealOpInsight. Available in the folder *examples, *the following file:


+ *small_hosting_platform.zbx.ngrt4n.xml ***: This
  file contains the configuration describing of our dashboard. This
  dashboard relies on some triggers from the default Zabbix server's
  monitoring configuration.

In the rest of the tutorial, we'll refer to the source directory with
the variable *SRCDIR*.


Check your Zabbix Installation
------------------------------
You need to ensure that:

+ The version of your Zabbix Server is greater than 1.7, from which
  Zabbix API was introduced.
+ The Zabbix Server as well as the Zabbix API service are running:
  From a browser, type *http://<zabbix_server>/zabbix/api_jsonrpc.php*
  (replace *<zabbix_server*> by the address of the Zabbix Server). You
  should have a blank page meaning that the Zabbix API is operating.



Configure your RealOpInsight Environment
----------------------------------------

Launch a terminal and start the RealOpInsight configuration wizard.

* On GNU/Linux and OS X Systems:

::

    $ ngrt4n-oc -c


* On Windows Systems:

::

    Start->NGRT4N Monitoring Suite->RealOpInsight Operations Console


Then follow the steps described `here`_ to configure your environment
properly.


Launch the Operations Console
-----------------------------

* On GNU/Linux and OS X Systems:

::

    $ ngrt4n-manager -d $SRCDIR/example/small_hosting_platform.zbx.ngrt4n.xml


* On Windows systems:

::

    Start->NGRT4N Monitoring Suite->Operations Console


You will be invited to select a configuration file. Browse through the
file selection window that will appear to pick a suitable
configuration file in *C:\Program Files\NGRT4N/examples* (default
installation directory).

After `login`_, the interface will be loaded. See `here`_ to learn
about the Operations Console's Features. See the `FAQ`_ for the
default user credentails.

Note: The configuration file is based on Zabbix 2.0.x. Therefore, if
you're using a former version, it's possible some services appear as
unknown. This is due to the fact the names of default triggers vary
from a version to another. You can edit the configuration file through
the `Editor Utility`_ to suit your Zabbix configuration.

That's all for this tutorial. Now `create your own configurations`_
and enjoy monitoring with RealOpInsight!!!

