

RealOpInsight Editor User's Manual
==================================



Prerequisites
-------------
Here we assume that:

+ You have read and understood the `RealOpInsight Concepts`_, and
  especially the section related to `message contextualization`_.
+ Your RealOpInsight installation is ready to use.

Overview of the Editor
----------------------

The goal of Editor is to ease the definition of service views by
enabling to:


+ *Manage the hierarchy in a flexible way*: it provides a friendly
  user interface comprising a tree view component to ease the edition
  and the manipulation of service views (drag-and-drop to reoganize the
  hierarchy on-the-fly, context menus to add, delete, copy/paste, and
  other relevant features to ease the edition of services).
+ *Edit the services easily*: the `User Interface`_ enables to edit
  the properties of services in a simple way.

Run the Editor
--------------
To run the the program, you have a Command Line Interface (CLI), and
on Windows, there is also a quick access from Start Menu.


The Command Line Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~

The CLI command is named *`ngrt4n-editor`_. *As shown below, its usage
is quite simple.

::

    ngrt4n-editor [file]


This launches the utility and, optionally, loads an existing
configuration file passed in parameter. If no file is provided, a new
configuration will be initialized.

Type *ngrt4n-editor -h* to learn more options.


Graphical Interface (Windows only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

From Start Menu :

::

    Start -> NGRT4N Monitoring Suite -> RealOpInsight Editor

User Interface
--------------
The user interface shown on the Figure below consists of:

+ *Menus*: Accessible from the `menu bar`_ or the toolbar, they enable
  to access the Editor's features.
+ *Explorer Pane*: Represents the hierarchy in a tree view that eases
  the manipulation (select service for modification, add/delete service,
  reorganization of the tree). See `here`_ for more details.
+ *Edition Pane*: Consists of a form allowing to set or update the
  `properties`_ of services.











Menus
-----
Accessible from the menu bar with shortcuts on the toolbar, they
enable functionalities such as :

+ Open an exiting file: File -> Open
+ Save changes: File -> Save, File -> Save As
+ Import a Nagios Status File: File -> Import

Edition
-------
The edition mainly relies on the Explorer Pane that significantly
eases the manipulation of the hierarchy. Among its different
functionalities, you can note that :

+ It supports drag-and-drop that allows to quickly organize your
  hierarchy thanks to your mouse.
+ Every node enables context menus that allow to add a child node on-
  the-fly, or even to delete the related node.
+ When a node is selected, its properties (see below) are
  automatically filled into the Edition Pane from which they can be
  modified.


Important Note: According to your monitoring system (Nagios-based or
Zabbix-based), you need to save the configuration in a suitable
format. By default the format is Nagios-compatible, it can be changed
through the file selection window (File -> Save As).





Properties of Services
----------------------
Every service in a hierarchy has the following properties:

+ *Name*: Serves as a label for the service.
+ *Type*: Possible values are *Native Check* or *Business Process*
  (default) according to the role of the service in the hierarchy.
+ *Status Calculation Rule*: Defines how the status of the service is
  calculated and from the statuses of its child services.
+ *Status Propagation Rule*: This property allows to defines how the
  status of the service is propagated to its parent service.
+ *Icon*: Sets the icon to associate to the service on the Operations
  Console.
+ *Description*: Optional, this field allows to set more information
  about the service.
+ *Alarm Message*: Optional and specific to native checks, this
  property holds the custom message to show on Operations Console when
  an incident arises (when the state changes from NORMAL to another
  state).

    + The message can include `contextualization tags`_.
    + The default value is the raw output returned by the monitoring
      plugin (same as  *{plugin_output}*).

+ *Notification Message*: Optional and specific to native checks, this
  holds the custom message to show on Operations Console when the
  service is recovered (when the status changes to another state to
  NORMAL.

    + The message can include one or more `contextualization tags`_.
    + The default value is the raw output returned by the monitoring
      plugin (same as  *{plugin_output}*).

+ *Data Point*: Sets the probe associated to an IT service node. It's
  defined in form of device/probe:

    + For a Nagios-like monitoring system, this follows the pattern
      *host_name/service_description* according to the service description
      in Nagios configuration. For example, 'localhost/Current Load'
      identifies the check allowing to monitor the load of Nagios server.
    + For Zabbix, the pattern is *host_name/trigger_name.* host_name and
      trigger_name as well as the relationship between them should be valid
      entries in Zabbix. For example, 'Zabbix server/Lack of free
      swap space on{HOST.NAME}' identifies the trigger allowing to
      monitor the swap space on Zabbix server.
    + For Zenoss data points are identified through the following the
      pattern *device_name/component_name* according to the description of
      hosts and components in Zenoss. For instance, 'locahost/httpd'
      identifies the component responsible for monitoring the Apache server
      process (httpd) on Zenoss Server.

