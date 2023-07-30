# Deploy RealOpInsight on Kubernetes
This document describes how to deploy RealOpInsight on a Kubernetes cluster.

The deployment can be achieved using `Helm` or `kubectl` + `Kustomize`. Select the appropriate section below to getting started.

- [Deploy RealOpInsight on Kubernetes](#deploy-realopinsight-on-kubernetes)
  - [Installation using kubectl and Kustomize](#installation-using-kubectl-and-kustomize)
  - [Deployment with Helm](#deployment-with-helm)
  - [In-cluster access to service](#in-cluster-access-to-service)
  - [Remote access to the service](#remote-access-to-the-service)

## Installation using kubectl and Kustomize
First review the default configuration settings in the deployment ConfigMap: `kustomize/realopinsight-config.yaml`.

The manifests are configured to achieve the deployment in the namespace `monitoring`. The namespace is assumed to exist, otherwise create it first.

Use the below command to trigger the deployment.

```
kubectl -n monitoring apply -k ./manifests/kustomize
```

## Deployment with Helm
Assuming you have a Linux terminal with Helm3 installed and able to access a KUBECONFIG file to deploy resources on Kubernetes, the following command shall install an instance in the `monitoring` namespace. **The namespace must exist**, it can be changed to another value.

```
helm upgrade --namespace monitoring --install realopinsight manifests/helm/realopinsight/
```

## In-cluster access to service
On success, the deployment does create a Kubernetes **ClusterIP** service named `realopinsight`.

This service exposes the RealOpInsight's web interface (port `80`) as well as its Prometheus exporter (port `4584`).

Their in-cluster URLs are the following:
  * Web UI: `http://realopinsight.monitoring/ui`.
  * Prometheus metrics: `http://realopinsight.monitoring:4583/metrics`.


## Remote access to the service
As a developer or for debugging purposes, you can enable port forwarding for local access as follows.

```
kubectl port-forward --namespace monitoring service/realopinsight-ui 4583:80
```

For production access, typically for operations monitoring, you must have to consider the Ingress policies defined in your organization. Contact your IT staff for additional information.

