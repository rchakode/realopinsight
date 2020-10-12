
![build](https://github.com/rchakode/realopinsight/workflows/CI/badge.svg)

# Overview
RealOpInsight allows to monitor, measure and observe end-user applications availability atop of Kubernetes®, Zabbix®, Nagios®. It features application-specific knowledges to aggregate and handle the status of low-level probes in a way that is meaningful for any targeted end-user application.

- [Overview](#overview)
- [Key Features](#key-features)
- [Quick Start](#quick-start)
    - [Deployment on kubernetes](#deployment-on-kubernetes)
    - [Deployment on Docker](#deployment-on-docker)
    - [Getting Started](#getting-started)
- [Contributions](#contributions)
- [Copyrights & License](#copyrights--license)

# Key Features
Fully open source ([GPL v3 Licence](LICENSE)) RealOpinsight features include:
* **Federated & Unified Monitoring:** RealOpInsight allows you to collect and consolidate data from a wide range of distributed and heterogeneous monitoring systems and provide to your operations staffs unified application operations dashboards.
* **Monitoring of Business Values:** by allowing you Focus on applications that underlie your business services. You can fire notifications at application-level, and easily generate availability reports over time.
* **Relationship Mapping & Event Correlation:** Map relationships among your applications' components along with custom severity calculation and propagation policies to meet your specific management needs.
* **Simple and Secured Integration through API:** Don't waste time on complex or boring configuration tasks, RealOpInsight only requires a read-only access to the API of the monitoring data collection backends (i.e. Kubernetes API, Zabbix API...). It's released as Docker images along with Kubernetes Helm3 manifests to ease its deployment.
* **Tactical Dashboards:** Whether you're an operations staff or executive, RealOpInsight provides tactical dashboards that allow you to have a quick insight on how your applications are operating while be able to have more details in one click.

![](./images/banners/screenshots.png)

# Quick Start
RealOpInsight is released as Docker images along with Kubernetes Helm3 manifests to ease its deployment.

The following two subsections show how to set an instance of RealOpInsight in a couple of seconds. The first subsection described describe installation on Kubernetes and the second one the deployment on Docker (or any equivalent container engine).

## Deployment on kubernetes
Assuming you have a Linux terminal with Helm3 installed and able to access a KUBECONFIG file to deploy resources on Kubernetes, the following command shall install an instance in the `monitoring` namespace. **The specified namespace must exist**, it can be changed to another value.

```bash
helm upgrade \
  --namespace monitoring \
  --install realopinsight \
  helm/realopinsight/
```

The Helm manifests deploy a ClusterIP service named `realopinsight` on port `80` to get access to the RealOpInsight's web interface.

To get a remote access to the web interface you can either enable an Ingress access or through port-forward as follows (change the namespace if different):


```bash
kubectl port-forward \
  --namespace monitoring \
  service/realopinsight 4583:80
```

Then point your browser at the following address: http://localhost:4583/realopinsight/.

The default username and password are `admin` and `qDmin123` (`password` prior to RealOpInsight `v20.10.1`).

## Deployment on Docker
The following command shall pull the image and start an instance of RealOpInsight in background. The option `-d` can be removed to start the instance in foreground.

With this command the data of the instance will be stored locally on the Docker machine at the specified path (value of option `--volume`, can be changed if needed).

```bash
$ docker run -d \
  --name realopinsight \
  --network host \
  --publish 4583:4583 \
  --volume $HOME/.realopinsight:/opt/realopinsight \
  rchakode/realopinsight
```

Then point your browser at the following address: http://localhost:4583/realopinsight/.

The default username and password are `admin` and `qDmin123` (`password` prior to RealOpInsight `v20.10.1`).


## Getting Started
Consider one of the following resources to start integrating RealOpInsight with your target monitoring environment.
* [Integration with Kubernetes](https://realopinsight.com/docs/quickstart-kubernetes-dashboard/)
* [Integration with Zabbix](https://realopinsight.com/docs/quickstart-zabbix-dashboard/)
* [Integration with Nagios and related systems](https://realopinsight.com/docs/quickstart-nagios-icinga-centreon-dashboard/)


# Contributions
Contributions in any form (feedback, code, documentation...) are welcome.

For code contributions it's required to have skills in writing code in C+11 or later.

The software also includes [Qt](https://www.qt.io/) and [Wt](https://www.webtoolkit.eu/wt) at it core. Having fundamentals to develop software using this frameworks may be helpful, but not required.

# Copyrights & License
This project has been initiated by Rodrigue Chakode and open to contributions.

Contributions in any form (code, documentation...) must be aligned and compliant with the following terms.

The software is licensed under the terms of [GPLv3 License](LICENSE), mainly due to contamination by third-party systems that the software is bound to. All those third-party systems may offer dual licenses (e.g. for proprietary uses), subject to pay some license fees. You can contact us you're interested in such a use.
