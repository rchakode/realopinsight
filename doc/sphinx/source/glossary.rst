========================
Glossary
========================
This chapter describes the terminologies used in this documentation.

.. glossary::

  Device
    Any physical component composing an IT infrastructure. E.g. servers, 
    routers, switches, etc.

  Incident
    Any disruption to the normal operation of a device or of a service 
    that the device enables. E.g. failure of a hard-drive, a crash of a process, 
    a network failure.

  Monitoring item
    A probe allowing to get the status of a device regarding a given type of
    incidents. E.g. a monitoring item can allow to check the availability of the 
    http service on a web server.                       
    For Nagios-based system, a monitoring item corresponds to a 'check'. E.g. 
    check_mysql allows to monitor the status of MySQL daemon.
    For Zabbix, it corresponds to a 'trigger'. E.g. the ping trigger allows 
    to monitor the availability of a device. For Zenoss a monitoring item 
    corresponds to a 'component'. E.g. the httpd component allows to check 
    the status of httpd daemon.

  Data point
    Defines the relationship between a monitoring item and a device. 
    In RealOpInsight context, each data point is uniquely characterized with 
    an identifier with the pattern device_id/probe:
    For Nagios-based systems, a data point is identified with the pattern 
    host_name/service_description. 
    For Zabbix, they're identitied with the pattern host/trigger_name.
    For Zenoss, they're identified with the pattern device_name/component_name.

  Severity of incident
    Refers to the level of impact an incident has on the healthy of a device or 
    a service  enabled by the device. 
    For example: Nagios and the derived systems use four levels of severities
   also known as *Service States*: OK, WARNING, CRITICAL and UNKNOWN.
    Zabbix defines six levels of severities: NOT CLASSIFIED, INFORMATION, 
    WARNING, AVERAGE, HIGH and DISASTER.
    Zenoss also defines six levels severities: CLEAR, DEBUG, INFO, WARNING,
    ERROR and CRITICAL.See their respective documentation for more details.

  Service
    Refers to an IT functionality or a high level service (business service) 
    as decribed next.

  IT service
    Also called native check, an IT service corresponds to a basic IT 
    functionality (e.g. mysqld service).

  Business service
    Also called business process, a business service defines a high level 
    service not directly related to a basic IT functionality. It may depend
    on *one or more* basic IT services, and/or other business services. 
    E.g. a backup service and a hosting service.

  Monitored platform
    Represents a set of monitored services. In RealOpInsight context the services 
    are related among them within an hierarchy of services as described in the next 
    section.

  Severity propagation rule
    Defines how the severity of a service is propagated to its parent within 
    an hierarchy.

  Severity calculation rule
    Defines how the severity of a service is calculated according to the 
    severities of its sub services.

  Dashboard
    A visualization interface that offers a simple and summarizing way to get 
    an insight on the healthy of a monitored platform.

  Nagios-based system
  Nagios-derived system
    Refers to Nagios itself, Shinken, Centreon, Icinga, GroundWork, or any other 
    monitoring system that relies on the same concepts than Nagios.

  Monitoring source
    Or simply source, refers to a monitoring server enabling API (native or not)
    to retrieve data related to probes.
