# Overview
RealOpInsight is an open-source system for federation and application operations monitoring dashboard for Kubernetes®, Nagios®, Zabbix®, Centreon®, Icinga®, ManageEngine OpManager®, Zenoss®, Pandora FMS®, OP5 Monitor®, and various other systems.

Among others, RealOpinsight features include:

* **Tactical Dashboards:** Whether you're an operations staff or executive, RealOpInsight provides tactical dashboards that allow you to have a quick insight on how your applications are operating while be able to have more details in one click.
* **Federated & Unified Monitoring:** RealOpInsight allows you to collect and consolidate data from a wide range of distributed and heterogeneous monitoring systems and provide to your operations staffs unified application operations dashboards.
* **Monitoring of Business Values:** by allowing you Focus on applications that underlie your business services. You can fire notifications at application-level, and easily generate availability reports over time.
* **Relationship Mapping & Event Correlation:** Map relationships among your applications' components along with custom severity calculation and propagation policies to meet your specific management needs.
* **Quick API-based Integration:** Don't waste time on complex or boring configuration tasks, RealOpInsight only requires a read-only access to the API of the monitoring data collection backends (i.e. Kubernetes API, Zabbix API...)
* **Open Source:** RealOpInsight is full open source, built with interoperability as basic concept. It's released under GPLv3 license. Commercial use with no copy-left is subject to specific agreements.

![](./images/banners/screenshots.png)

# Installation

RealOpInsight can be easily installed using one of the following methods:

* Binaries for Ubuntu Server 16.04 LTS.
* Docker image based on Ubuntu Server 16.04 LTS.
* Virtual machine images (Open Virtualization Format, OVF 2.0), a.k.a virtual machine image, based on Ubuntu Server 16.04 LTS. The appliance can be installed on VirtualBox, VMware, or any other OVA-compatible hypervisors.

## Installation via Docker
The following command will pull and start a Docker instance of RealOpInsight in background.

```
$ docker run -d \
  --name realopinsight \
  --network host \
  --publish 4583:4583 \
  rchakode/realopinsight
```

Once the container started, the RealOpinsight web interface can be accessed on on your local machine at http://127.0.0.1:4583/realopinsight/.

You can then move to the Getting Started section for more instructions.

## Installation via a Virtual Machine Image (OVF 2.0)
The following steps describe how to install RealOpInsight using an OVF image on [Oracle VirtulaBox](https://www.virtualbox.org/). You can easily adapt the steps if you use a VMware virtualization tool instead.

* Go to the [release page](https://github.com/RealOpInsightLabs/realopinsight/releases) and get the latest OVF distribution archive.
* Unzip the distribution archive, you will obtain an OVA (Open Virtualization Appliance) file.
* If not yet the case [donwload and install VirtualBox](https://www.virtualbox.org/wiki/VBoxInstallAndRun).
* Launch VirtualBox and select the menu `File -> Import Virtual Appliance`.
* Browse your file system and select the OVA file of RealOpInsight.
* Review the default virtual machine settings. You would likely consider to change the hardware requirements according to your needs (i.e. memory, CPU, network, etc).
* Validate the importation and wait that the process completes. That may take a few minutes.
* Once the virtual machine started, the RealOpinsight web interface can be accessed on on your local machine at [http://<VM_ADDR>:4583/realopinsight](http://VM_ADDR:4583/realopinsight/) (replace `<VM_ADDR>` by the IP address or the hostname of the virtual machine).

You can then move to the Getting Started section for more instructions.

## Installation/Upgrade via a Binary Distribution

* Log on the machine that will host RealOpInsight.
* Install Graphviz dependency

  ```
  $ sudo apt-get install graphviz
  ```      

* Go to the [release page](https://github.com/RealOpInsightLabs/realopinsight/releases) and get the latest binary distribution tarball.
* Uncompress the archive, move to the distribution directory, and start the installation process:

  ```
    $ tar zxf realopinsight-ultimate-<VERSION>.ubuntu1404.x86_64.tar.gz       
    $ cd tar zxf realopinsight-ultimate-<VERSION>.ubuntu1404.x86_64
    $ sudo ./install-ultimate-distrib.sh
  ```

    Replace `<VERSION>` with the downloaded version

* When prompted, you must accept the license terms in order to move forward.
* When prompted to select the type of installation you want to perform.
    * Type `n` to proceed with a new installation.
    * Type `u` to proceed with an installation update.
    This process is interactive, you may be prompted to provide additional information regarding your existing installation.
* Once the virtual machine started, the RealOpinsight web interface can be accessed on on your local machine at [http://<HOST_ADDR>:4583/realopinsight](http://VM_ADDR:4583/realopinsight/) (replace `<HOST_ADDR>` by the IP address or the hostname of your the installation server).

You can then move to the Getting Started section for more instructions.


# Getting Started using RealOpInsight

According to the installation option you selected, open the RealOpInsight web interface and login as administrator (username: `admin`, password: `password`).

For production use, the default password must be changed password as soon as possible for security reasons.
