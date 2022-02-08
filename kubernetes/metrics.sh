#! /usr/bin/env bash

if [ $# -gt 0 ]; then
    if [ "$1" == "install" ]; then
        helm repo add bitnami https://charts.bitnami.com/bitnami
        helm repo update
        helm install -f metrics.yaml metrics bitnami/metrics-server
    elif [ "$1" == "remove" ] || [ "$1" == "uninstall" ]; then
        helm uninstall metrics
    elif [ "$1" == "pods" ]; then
        kubectl get --raw "/apis/metrics.k8s.io/v1beta1/nodes"
    fi
else
    kubectl get --raw "/apis/metrics.k8s.io/v1beta1/nodes"
fi