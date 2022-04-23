FROM rabits/qt:5.15-desktop as builder-deps

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
WORKDIR /tmp
RUN WT_VERSION=4.7.1 && \
    WT_TARBALL=${WT_VERSION}.tar.gz && \
    wget https://github.com/emweb/wt/archive/${WT_TARBALL} && \
    tar zxf ${WT_TARBALL} && \
    cd wt-${WT_VERSION}/ && \
    mkdir build && \
    cd build && \
    cmake .. -DWEBUSER=www-data -DWEBGROUP=www-data -DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick && \
    make install && \
    rm -rf /tmt/wt && \
    cd /tmp && \
    PROM_CPP_VERSION=0.10.0 && \
    PROM_CPP_DEB=prometheus-cpp_${PROM_CPP_VERSION}_amd64.deb \
    PROM_CPP_DEB_URL=https://github.com/rchakode/prometheus-cpp/releases/download/v${PROM_CPP_VERSION}/${PROM_CPP_DEB} && \
    wget $PROM_CPP_DEB_URL && \
    apt-get install ./$PROM_CPP_DEB
