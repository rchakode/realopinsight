# Dockerfile for building the Docker distribution of RealOpInsight Ultimate

FROM ubuntu:14.04
MAINTAINER RealOpInsight Labs <support@realopinsight.com>
ENV REALOPINSIGHT_DISTRIB_BASENAME realopinsight-ultimate-2015r1.ubuntu1404.x86_64
ENV REALOPINSIGHT_DISTRIB_FILE ${REALOPINSIGHT_DISTRIB_BASENAME}.tar.gz

COPY ./container-entrypoint.sh /
ENTRYPOINT ["/bin/bash", "/container-entrypoint.sh"]

RUN apt-get update && apt-get install -y apache2 \
    graphviz \
    libapache2-mod-fcgid \
    openssh-server \
    openssh-client


COPY $REALOPINSIGHT_DISTRIB_FILE /tmp/

WORKDIR /tmp
RUN tar zxf $REALOPINSIGHT_DISTRIB_FILE

WORKDIR $REALOPINSIGHT_DISTRIB_BASENAME
RUN ["/bin/bash", "-c", "echo -e 'y\nn' | $PWD/install-ultimate-distrib.sh"]

WORKDIR /tmp


RUN rm -rf $REALOPINSIGHT_DISTRIB_BASENAME*

WORKDIR $HOME

