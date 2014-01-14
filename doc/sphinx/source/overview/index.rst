#########################################
Overview of RealOpInsight
#########################################

Basically, the goal of RealOpinsight is to allow you to focus on business needs,
instead of wasting time on false alerts. Indeed, monitoring in
demanding operations environments such as Network Operations Centers
(NOC) and large data centers is highly challenging and requires to
quickly react when incidents arise:

+ You need to *quickly evaluate the impacts of incidents on your business
  processes*, so as to prioritize their recovery from incidents that
  have higher impacts.
+ You should be able *to report certain incidents to operators able to 
  address them quickly*.
+ More generally, you need to quickly answering questions like: 
  what is the current status of my applications/end-user services? which 
  applications/end-user services are affected by a given incident? 
  which services depend on a given hardware device? etc.

Deal with these issues from native tools provided by the monitoring
systems such as Nagios, Zabbix and Zenoss is not easy. This may be
especially challenging when the amount of monitored devices increases.

That's where RealOpInsight comes in, providing an *integrated and flexible 
business intelligence toolkit*, that, based on a *hierarchical  service 
dependency tree model*, gives you an accurate insight on the healthy of 
your business processes. This chapter introduces its features, its basic 
concepts and how it works:


.. toctree::
   :maxdepth: 2

   Features <features>
   Basic Concepts and Operation <concepts>
