=======================================================
Tutorial: using RealOpInsight with Nagios
=======================================================

This tutorial shows how to use RealOpIinsight with Nagios and its devived systems.

`
`_Prerequisites
---------------
Here we assumes that:

+ You have read and understood the `concepts and the architecture`_ of
  RealOpInsight.
+ The `Daemon Service for Nagios`_ and RealOpInsight are installed and
  ready to use. To simplify the tutorial, all these components can be
  installed on the same machine.



Use Case
--------

The tutorial is based on the example files provided with
RealOpInsight. Available in the *examples* directory from the source
tree, this example illustrates the monitoring of a web hosting
platform through a RealOpInsight-based dashboard..
We need the following files from this folder :

+ *{literal}small_hosting_platform.nag.ngrt4n.xml *{/literal}** : The
  NGRT4N configuration file describing the monitored platform along with
  its business processes.
+ *status.dat* : The Nagios status file providing the related
  monitoring data.

In the rest of the tutorial, we'll refer to the source directory with
the variable *SRCDIR*.


Set up the Daemon Service For Nagios
------------------------------------

Log on the machine where you installed the daemon service and launch a
terminal.

Ensure that the Daemon is stopped.

::

    $ killall ngrt4nd  # You may need root privileges


Then set the authentication token by performing the following command.
You'll be prompted to enter a passphrase necessary to generate the
token.

::

    $ ngrt4nd -P 


The generated token will be printed on the console as follows:

::

    $1$ngrt4n$2x2PKY8WWu/fxMk7T4DX4/ 


Later, you can print the token using the option * -T*.

Now start the daemon.

::

    {literal}$ ngrt4nd -D -c SRCDIR/examples/status.dat{/literal}


This will start the daemon in foreground mode while loading the status
file *SRCDIR/examples/status.dat*. In contrary to the daemon mode, the
foreground mode is useful for viewing debugging messages in case of
errors.

Now you have the daemon started and listening on the port 1983
(`default port`_).


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

    {literal}$ ngrt4n-manager -d $SRCDIR/example/small_hosting_platform.nag.ngrt4n.xml{/literal}


* On Windows systems:

::

    Start->NGRT4N Monitoring Suite->RealOpInsight Operations Console


You will be invited to select a configuration file. Browse through the
file selection window that will appear to pick the suitable
configuration file in *C:\Program Files\NGRT4N/examples *(default
installation directory).

After `login`_, the interface will be loaded. See `here`_ to learn
about the Operations Console's Features. See the `FAQ`_ for the
default user credentails.

That's all for this tutorial. Now `create your own configurations`_
and enjoy monitoring with RealOpInsight!!!

