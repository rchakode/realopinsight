
# Deploy RealOpInsight on Docker
This document describes how to deploy RealOpInsight on a Docker machine.

## Start the container
The following command shall pull the image and start an instance in background (`-d`). You can remove this option to start the instance in foreground.

With this command the data of the instance will be stored locally on the Docker machine at the specified path (value of option `--volume`, can be changed if needed).

```shell
docker run -d \
    --name realopinsight \
    --network host \
    --volume $HOME/.realopinsight:/opt/realopinsight \
    rchakode/realopinsight
```

## Accessing the service
Once the container started, it shall enable the following endpoints:
 * Web UI: http://localhost:4583/ui (default credentials: `admin`/`password`)
 * Prometheus metrics: http://localhost:4584/metrics.