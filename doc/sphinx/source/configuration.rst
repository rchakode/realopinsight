
This document describes how to configure RealOpInsight. It's
recommended to read carefully the prerequisites section before moving
forward.


Contents
--------



Prerequisites
-------------

As introduced in the `Concepts Guide`_, RealOpInsight uses different
approaches to retrieve data from the underlying monitoring servers
(a.k.a monitoring sources):


+ For Zabbix and Zenoss, it relies on their native RPC APIs: `JSON-
  RPC`_ for Zabbix and `JSON API`_ for Zenoss.
+ To retrieve data from Nagios, Shinken, Icinga, Groundwork, op5,
  Centreon and other Nagios-derived systems that do not enable native
  APIs, it relies either on a `specific daemon`_ (ngrt4nd) on the
  monitoring server, or on Livestatus (MKLivestatus or Shinken
  Livestatus). The Livestatus APIs must be enabled over the network. See
  `here`_ or `here`_, respectively, to learn how to do that.


Since the version 2.4.0, RealOpInsight can handle monitoring views
relying on many monitoring sources.

The support of multi-source views brings few changes concerning how
data points are defined and handled. However, a special effort has
been made to keep backward-compatibility with monitoring views edited
from a former version. This is important to note. Since you don't need
to care about multi-source handling when your monitoring view rely on
only a single monitoring source.

Important: If you're using a version earlier than the version 2.4.0,
we recommand to follow this `configuration guide`_.

To upgrade to the version 2.4.0, you need to reset your configuration
settings. Otherwise, the Operations Console would be able to connect
to the remote API endpoint.


Launch the Configuration Manager
--------------------------------

On GNU/Linux and OS X systems, launch a terminal and run the following
command:

::

    $ ngrt4n-manager -c


On Windows, you can use the command from a terminal or simply use the
Start menu:

::

    Start->NGRT4N Monitoring Suite->RealOpInsight Configuration Manager


Then sign as `administrator`_ (login: *ngrt4n_adm*, default password:
*ngrt4n_adm*).



Once signed in, the configuration window will be loaded; there is a
screenshot.




Description of Configuration Parameters
---------------------------------------

As described in the following table, some of configuration parameters
may be required, optional, or not applicable(NA) at all according to
the underlying monitoring system.
Parameter Nagios/derived Zabbix/Zenoss Description Monitor Web URL
Optional Required

Sets the URL to the default web interface of the monitoring server
(E.g. http://nagios-server/nagios/, http://zabbix-server/zabbix/, http
://zenoss-server:8080/)
Auth String
- Required with ngrt4nd

- NA with Livestatus

Required

Sets the token for authenticating against the remote API endpoint:

* With Zabbix and Zenoss, this should be set with a couple
*login:password* (note the colon *':'* ) representing a valid user
account. Prior to Zabbix 2.0, this user MUST belong to the group *API
Access*;

* With ngrt4nd this corresponds to the `authentication token`_.
Server Address Required NA
Sets the IP address or the hostname of the monitoring server (e.g.
nagios-server.lan, shinken-server.example.com, 192.168.1.168)
Port Required NA
Sets the port on which ngrt4nd or the Livestatus API is listening on
on the monitoring server (e.g. 1983 for ngrt4nd)
Update Interval Required Required Set the interval after which the
Operations Console will be refreshed with new status information
retrieved from the monitoring servers


Basic Configuration for Single Monitoring Source
------------------------------------------------

Here are basic steps:


#. `Launch the Configuration Manager`_ and set parameters according to
   the target monitoring system.
#. Make sure to select the appropriate source type via the related
   combox on the form.
#. Click on *Apply Settings* to save changes.
#. And finally, click on *Close *to quit the configuration window.

At any time you can rerun the configuration manager to update
settings; a quick access is enabled via the menu *Console->Monitoring
Settings* from the Operations Console.


Avanced Configuration for Multiple Monitoring Sources
-----------------------------------------------------

Here we assume that you are familar with a basic configuration as
described in the last section. There are also some important notices
about sources management:


+ RealOpInsight can manage up to ten sources per service view
  simultaneously.
+ Each source has an identifier in the form of *"Source"+Index*, where
  Index comprised between 0 and 9 corresponds to the index of the source
  (E.g. Source0, Source2). See on the screenshot of the Configuration
  Manager for more details.
+ To associate a data point to a given source, you need to prefix the
  data point with the source identifier as follow:
  *SourceIndex:device/probe* (note the colon *':' *separator) *.* For
  example, {literal}'Source0:mysql-server01/Root Partition'{/literal}
  indicates the probe allowing to monitor the root partition on the
  server named mysql-server from the Nagios server refered with Source0.
+ It's not required to set all the sources, but if a source is not
  set, every data point related to that source will be automatically
  marked as UNKNOWN.
+ For backward-compatibily, Source0 is by convention considered as
  default source. This means that when no source is set for a data point
  the data point is considered as associated to Source0. For instance
  'Source0:localhost/ping' is equivalent to 'localhost/ping'


Let's see how to set the sources. Here are basic steps:


#. `Launch the Configuration Manager`_ and set the parameters
   according to the target monitoring system; see the description of `
   configuration parameters`_.
#. Then click on *Add as Source* (instead of on *Apply Settings* as
   for single source);
#. Select the source index from the selection list that will appear.
   Note that if you set the index of a source that has been already set,
   that source will be overwritten without requiring confirmation.
#. If you haven't yet set the source type, you will be invited to set
   it.
#. Restart at the first step to set another source. When all the
   sources you would want to define would have been completed, click on
   *Close* to quit the configuration window. You can now use the sources.




Related Links : `Tutorial for Nagios`_ | `Tutorial for Zabbix`_ |
`Tutorial for Zenoss`_

