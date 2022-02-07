ARG     OME_VERSION=master
ARG 	STRIP=TRUE

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
RUN ${TEMP_DIR}/misc/prerequisites.sh


## Build OvenMediaEngine
RUN \
        cd ${TEMP_DIR}/src && \
        make release -j$(nproc)

RUN \
        if [ "$STRIP" = "TRUE" ] ; then strip ${TEMP_DIR}/src/bin/RELEASE/OvenMediaEngine ; fi

## Make running environment
RUN \
        cd ${TEMP_DIR}/src && \
        mkdir -p ${PREFIX}/bin/edge_conf && \
        cp ./bin/RELEASE/OvenMediaEngine ${PREFIX}/bin/ && \
        cp ../conf/Edge.xml ${PREFIX}/bin/edge_conf/Server.xml && \
        cp ../conf/Logger.xml ${PREFIX}/bin/edge_conf/Logger.xml && \
        rm -rf ${DIR}

FROM	base AS release

WORKDIR         /opt/ovenmediaengine/bin
EXPOSE          8090/tcp 3334/tcp 3479/tcp 10000-10005/udp
COPY            --from=build /opt/ovenmediaengine /opt/ovenmediaengine

CMD             ["/opt/ovenmediaengine/bin/OvenMediaEngine", "-c", "edge_conf"]

ARG     OME_VERSION=master
ARG 	STRIP=TRUE

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
RUN ${TEMP_DIR}/misc/prerequisites.sh

## Build OvenMediaEngine
RUN \
        cd ${TEMP_DIR}/src && \
        make release -j$(nproc)

RUN \
        if [ "$STRIP" = "TRUE" ] ; then strip ${TEMP_DIR}/src/bin/RELEASE/OvenMediaEngine ; fi

## Make running environment
RUN \
        cd ${TEMP_DIR}/src && \
        mkdir -p ${PREFIX}/bin/edge_conf && \
        cp ./bin/RELEASE/OvenMediaEngine ${PREFIX}/bin/ && \
        cp ../conf/Edge.xml ${PREFIX}/bin/edge_conf/Server.xml && \
        cp ../conf/Logger.xml ${PREFIX}/bin/edge_conf/Logger.xml && \
        rm -rf ${DIR}

FROM	base AS release

WORKDIR         /opt/ovenmediaengine/bin
EXPOSE          8090/tcp 3334/tcp 3479/tcp 10000-10005/udp
COPY            --from=build /opt/ovenmediaengine /opt/ovenmediaengine

CMD             ["/opt/ovenmediaengine/bin/OvenMediaEngine", "-c", "edge_conf"]
