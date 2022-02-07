ARG     OME_VERSION=master
ARG     BUILD_ENV=production
ARG 	STRIP=TRUE
ARG     GPU=FALSE
ARG     MODE=origin

FROM    ubuntu:20.04 AS base

ENV     PREFIX=/opt/ovenmediaengine
ENV     TEMP_DIR=/tmp/ome

## Install libraries by package
ENV     DEBIAN_FRONTEND=noninteractive
RUN     sed -i -e 's/http:\/\/archive\.ubuntu\.com\/ubuntu\//mirror:\/\/mirrors\.ubuntu\.com\/mirrors\.txt/' /etc/apt/sources.list
RUN     apt-get update && apt-get install -y tzdata sudo curl git

FROM    base AS sources

COPY ./ /tmp/ome
WORKDIR /tmp/ome

FROM sources as build

## Install dependencies
RUN \
        if [ "$GPU" = "TRUE" ] ; then \
                ${TEMP_DIR}/misc/install_nvidia_docker_image.sh ; \
                ${TEMP_DIR}/misc/prerequisites.sh  --enable-nvc ; \
        else \
                ${TEMP_DIR}/misc/prerequisites.sh ; \
        fi

## Build OvenMediaEngine
RUN \
        cd ${TEMP_DIR}/src && \
        make release -j$(nproc)

RUN \
        if [ "$STRIP" = "TRUE" ] ; then strip ${TEMP_DIR}/src/bin/RELEASE/OvenMediaEngine ; fi

## Make running environment
RUN \
        cd ${TEMP_DIR}/src && \
        mkdir -p ${PREFIX}/bin/origin_conf && \
        mkdir -p ${PREFIX}/bin/edge_conf && \
        cp ./bin/RELEASE/OvenMediaEngine ${PREFIX}/bin/ && \
        cp ../conf/Origin.xml ${PREFIX}/bin/origin_conf/Server.xml && \
        cp ../conf/Logger.xml ${PREFIX}/bin/origin_conf/Logger.xml && \
        cp ../conf/Edge.xml ${PREFIX}/bin/edge_conf/Server.xml && \
        cp ../conf/Logger.xml ${PREFIX}/bin/edge_conf/Logger.xml && \
        rm -rf ${DIR}

FROM	base AS release

WORKDIR         /opt/ovenmediaengine/bin
EXPOSE          80/tcp 8080/tcp 8090/tcp 1935/tcp 3333/tcp 3334/tcp 4000-4005/udp 10000-10010/udp 9000/tcp
COPY            --from=build /opt/ovenmediaengine /opt/ovenmediaengine

CMD             ["/opt/ovenmediaengine/bin/OvenMediaEngine", "-c", "$MODE_config"]