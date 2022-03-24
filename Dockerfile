ARG     OME_VERSION=master
ARG     BUILD_ENV=production
ARG 	STRIP=TRUE
ARG     GPU=TRUE

FROM    ubuntu:20.04 AS base
COPY ./ /tmp/ome
WORKDIR /tmp/ome

ENV     PREFIX=/opt/ovenmediaengine
ENV     TEMP_DIR=/tmp/ome

## Install libraries by package
ENV     DEBIAN_FRONTEND=noninteractive
#RUN     sed -i -e 's/http:\/\/archive\.ubuntu\.com\/ubuntu\//mirror:\/\/mirrors\.ubuntu\.com\/mirrors\.txt/' /etc/apt/sources.list
RUN     apt-get update && apt-get install -y tzdata sudo curl git
## Install dependencies
RUN ${TEMP_DIR}/misc/install_nvidia_docker_image.sh
RUN ${TEMP_DIR}/misc/prerequisites.sh  --enable-nvc

FROM base as build

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
        cp ../misc/entrypoint.sh ${PREFIX}/bin && \
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

# default to origin
CMD             ["/opt/ovenmediaengine/bin/entrypoint.sh", "-c", "origin_conf"]
