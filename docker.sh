#! /usr/bin/env bash
org=${ORG:-syncronet}
name=ovenmediaengine
tag=${OME_TAG:-origin}
container=$name-$tag

if [ $# -gt 0 ]; then
    if [ "$1" == "attach" ]; then
        shift 1
        docker attach $container $@
    
    elif [ "$1" == "push" ]; then
        shift 1
        docker push $org/$name $@
    
    elif [ "$1" == "build" ]; then
        shift 1
        docker build . -t $org/$name $@

    elif [ "$1" == "dev" ]; then
        shift 1
        docker build . --build-arg BUILD_ENV=dev -t $org/$name:$tag $@
    
    elif [ "$1" == "config" ]; then
        shift 1
        docker build . --target config -t $org/$name $@

    elif [ "$1" == "clean" ]; then
        shift 1
        docker kill $container $@
        docker rm $container $@

    elif [ "$1" == "run" ]; then
        shift 1
        docker run \
            -p 1935:1935 \
            -p 3333:3333 \
            -p 3478:3478 \
            -p 8080:8080 \
            -p 9000:9000 \
            -p 9999:9999/udp \
            -p 4000-4005:4000-4005/udp \
            -p 10006-10010:10006-10010/udp \
            --name $container \
            $org/$name:$tag \
            $@
    else
        export EXTERNAL_IP=$(hostname -I | awk '{print $1}')

        docker-compose $@
    fi
else
    docker-compose ps
fi
