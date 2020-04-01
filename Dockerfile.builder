FROM rabits/qt:5.13-desktop as builder-deps
MAINTAINER Rodrigue Chakode <rodrigue.chakode @ gmail dot com>

ENV QT_ROOT /usr
ENV WT_ROOT /usr/local
USER root

RUN apt update && \
    apt install -y \
    wget \
    cmake \
    build-essential \
    libboost-program-options1.65-dev \
    libboost-system1.65-dev \
    libboost-thread1.65-dev \
    libboost-regex1.65-dev \
    libboost-signals1.65-dev \
    libboost-filesystem1.65-dev \
    libboost-date-time1.65-dev \
    libboost-random1.65-dev \
    libgraphicsmagick++1-dev \
    libssl-dev \
    libsqlite3-dev \
    libpq-dev \
    libldap2-dev \
    libpango1.0-dev \
    libglu1-mesa-dev \
    graphviz \ 
    vim \
    bc \
    && \ 
    rm -rf /var/lib/apt/lists/*

FROM builder-deps as builder
WORKDIR /tmt/wt
RUN WT_VERSION=4.3.0 && \
    WT_TARBALL=${WT_VERSION}.tar.gz && \
    wget https://github.com/emweb/wt/archive/${WT_TARBALL} && \
    tar zxf ${WT_TARBALL} && \
    cd wt-${WT_VERSION}/ && \
    mkdir build && \
    cd build && \
    cmake .. -DWEBUSER=www-data -DWEBGROUP=www-data -DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick && \
    make install && \
    rm -rf /tmt/wt
