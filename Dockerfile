FROM rchakode/realopinsight-builder:v22.05.00-qt515-wt471 as builder
ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH
WORKDIR /app
COPY . .
RUN chown -R user:user /app && \
    qmake realopinsight.pro -spec linux-g++ CONFIG+=release CONFIG+=server && /usr/bin/make  && \
    qmake realopinsight.pro -spec linux-g++ CONFIG+=release CONFIG+=reportd && /usr/bin/make && \
    qmake realopinsight.pro -spec linux-g++ CONFIG+=release CONFIG+=setupdb && /usr/bin/make && \
    chmod +x ./container-extract-dist.sh && ./container-extract-dist.sh
USER user
ENTRYPOINT ["/bin/bash"]


FROM ubuntu:18.04
ENV APP_USER_UID=54583
ENV APP_USER realopinsight
ENV LD_LIBRARY_PATH /app/lib:$LD_LIBRARY_PATH
ENV REALOPINSIGHT_ROOT_DIR /app
ENV REALOPINSIGHT_DATA_DIR /data
WORKDIR /app
COPY --from=builder /app/dist .
RUN apt update && \
    apt install -y libsqlite3-0 graphviz sudo && \
    (id ${APP_USER} || useradd ${APP_USER} -u $APP_USER_UID) && \
    echo "${APP_USER} ALL=NOPASSWD: ALL" > /etc/sudoers.d/user && \
    mkdir -p /app/www/run /data && \
    chown -R ${APP_USER}:${APP_USER} /app /data
VOLUME ["/data"]
USER ${APP_USER}
ENTRYPOINT ["/app/container-entrypoint.sh"]
