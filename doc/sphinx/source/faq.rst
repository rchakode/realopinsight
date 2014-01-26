.. _faq:

Frequently Asked Questions
==============================

.. contents::

What does RealOpInsight mean?
-----------------------------

RealOpInsight means Real Operations Insight from the fact that it is
intended to enable administrators/operators to have a real insight on
the healthy of their business processes.


Does RealOpInsight is a standalone monitoring system?
-----------------------------------------------------

No, we do not intend to reinvent the wheel. RealOpInsight is a
complementary tool working on top of existing systems like Nagios and
Zabbix. The aim being to provide those proven tools with advanced and
powerful dashboard management capabilities.


Do I need a separate installation of RealOpInsight for the different kinds of monitoring systems?
--------------------------------------------------------------------------------------------------

No, independently to the monitoring system you're using, Nagios,
Zabbix, Zenoss, Icinga, Centreon or whatever, only one installation is
necessary. The target monitoring system is detected automatically from
the format of the configuration file that you do set when editing
describing the platform to monitor with RealOpInsight.


What are the default user credentials?
--------------------------------------
Here is the list of the default user profiles:
Operator User (Limited Privileges)

+ Login: *ngrt4n_op*
+ Password: *ngrt4n_op*


Administrator User (Full Privileges)

+ Login: *ngrt4n_adm*
+ Password: *ngrt4n_adm*



How to change my password?
--------------------------
There are two ways to change your password:

+ The first one is to run the Operations Console, and go to the menu
  *Monitoring Settings -> Change Password*.
+ Alternatively, you can use the Configuration Manager.



How to enable MKLivestatus over a Xinetd TCP socket?
----------------------------------------------------

Create the appropriate xinetd configuration in /etc/xinet.d/livestatus
and restart xinetd. This configuration should look as follows:

::

    service livestatus
    {
    	type = UNLISTED
    	port = 6557
    	socket_type = stream
    	protocol = tcp
    	wait = no
    	cps = 100 3
            instances = 500
            per_source = 250
    	flags = NODELAY
    	user = nagios
    	server = /usr/bin/unixcat
    	server_args = /var/lib/nagios/rw/live
    	disable = no
    }


Basically, this means that the service shall listen on the port 6557
and shall forward requests to the UNIX socket located in
/var/lib/nagios/live (server_args), which is assumed to be the path of
the native socket managed by the Livestatus NEB. For more details, see
the official documentation of MKLivestatus.


How to enable Shinken Livestatus API over a TCP socket?
-------------------------------------------------------

Add the following lines in your Shinken configuration file (shinken-
specific.cfg), and restart Shinken.

::

    define module {
      module_name Livestatus 
      module_type livestatus
      host *
      port 50000
      socket /usr/local/shinken/var/rw/live
    }


In short, this means that the service shall listen on any network
interfaces (host \*) on port the 50000 on the Shinken server. For more
details, see the official documentation of Shinken Livestatus.

