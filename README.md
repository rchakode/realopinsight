
![](./images/banners/realopinsight-overview-thumbnail.png)


![GPL v3 License](https://img.shields.io/github/license/rchakode/realopinsight.svg?label=License&style=for-the-badge)
[![Calendar Versioning](https://img.shields.io/badge/calver-YY.MM.MICRO-bb8fce.svg?style=for-the-badge)](http://calver.org)
![Docker pulls](https://img.shields.io/docker/pulls/rchakode/realopinsight.svg?label=Docker%20Pulls&style=for-the-badge)

---

- [Overview](#overview)
- [Getting Started](#getting-started)
- [License](#license)
- [Contributions](#contributions)

# Overview
RealOpInsight gathers pods and checks status from Kubernetes®, Zabbix® and Nagios® to enable upon them application-specific high-level visualization that helps track the availability of applications over time. 

Key features:

  * **Monitor business values, Define & track SLA/SLO targets:** Focus on the applications that underlie your business, set up application-aware notifications, measure, observe and analyze applications availability over time. Be able to easily evaluate the real impact of each incident.
  * **Federated & unified applications monitoring:** Break silos by setting up unified applications monitoring operations views that hide the underlying monitoring systems while helping you solving incidents with relevant business priorities.
  * **Relationship mapping & event correlation:** Map relationships among application components, set up specific business-oriented policies to handle and propagate each incident with appropriate business impact.
  * **Simple and secured integration through API:** Don't waste time on complex configuration tasks, just a couple of minute is required to deploy and get started with RealOpInsight.
  * **Tactical Operations Dashboards:** For operations staff or executive, build specific tactical dashboards to get quick insight on how your applications are operating, be able to get details in one click.
  * Open source licensed under the terms of [GPL v3 Licence](LICENSE).

![](./images/banners/screenshots.png)


# Quick start on Kubernetes
The following sequence installs RealOpInsight in the namespace `monitoring`. The namespace is created if not yet the case. 


```bash
git clone --depth 1 https://github.com/rchakode/realopinsight.git && \
  kubectl -n monitoring apply -k ./realopinsight/manifests/kustomize
```

Check the web interface

```shell
kubectl port-forward --namespace monitoring service/realopinsight-ui 4583:80

# Then open http://localhost:4583 in your browser.
```

Available _in-cluster_ service endpoints (can be exposed to the external world via an Ingress Controller):

* Web UI: http://realopinsight.monitoring/ui.
* Prometheus metrics: http://realopinsight.monitoring:4583/metrics.


# Documentation
  * [Deployment on Kubernetes](./docs/deployment-on-kubernetes.md)
  * [Deployment on Docker](./docs/deployment-on-docker.md)
  * [Integration with Kubernetes](https://realopinsight.com/docs/quickstart-kubernetes-dashboard/)
  * [Integration with Zabbix](https://realopinsight.com/docs/quickstart-zabbix-dashboard/)
  * [Integration with Nagios and alike](https://realopinsight.com/docs/quickstart-nagios-icinga-centreon-dashboard/)
  * [Design fundamentals](https://realopinsight.com/docs/monitoring-data-sources/)

> Default credentials: username => `admin`, password => `password` (should be changed).

# License
RealOpInsight is licensed under the terms of [GPLv3 License](LICENSE), mainly due to copyleft contamination by third-party software to which it's bound ([Qt](https://www.qt.io/), [Wt](https://www.webtoolkit.eu/wt)).


# Contributions
Contributions in any forms are welcomed (feedback, code, documentation, etc).

The core of RealOpInsight is based on C++14, it leverages features from [Qt](https://www.qt.io/) and [Wt](https://www.webtoolkit.eu/wt) frameworks. 
