#! /usr/bin/env bash

if [ $# -gt 0 ]; then
    if [ "$1" == "install" ]; then
        # Prometheus monitors pod & system usage
        # Grafana provides a dashboard for it
        helm repo add prometheus-community https://prometheus-community.github.io/helm-charts
        helm install promstack prometheus-community/kube-prometheus-stack
    elif [ "$1" == "remove" ] || [ "$1" == "uninstall" ]; then
        helm uninstall promstack
    elif [ "$1" == "pods" ]; then
        kubectl --namespace default get pods -l "release=promstack"
    elif [ "$1" == "clear" ] || [ "$1" == "free-ports" ]; then
        pkill -f "port-forward" 
    fi
else
    POD_NAME=$(kubectl get pods -l "app.kubernetes.io/instance=promstack-kube-prometheus-prometheus" -o jsonpath="{.items[0].metadata.name}")
    kubectl port-forward $POD_NAME 9090 &
    echo "You can access Prometheus in your browser at localhost:9090"
    command -v xdg-open && xdg-open http://localhost:9090 &> /dev/null

    POD_NAME=$(kubectl get pods -l "app.kubernetes.io/name=grafana" -o jsonpath="{.items[0].metadata.name}")
    kubectl port-forward $POD_NAME 3000 &
    echo "You can access Grafana in your browser at localhost:3000"
    GRAFANA_PASSWORD=$(kubectl get secret promstack-grafana -o jsonpath="{.data.admin-password}" | base64 --decode ; echo)
    echo "You can login with admin & $GRAFANA_PASSWORD"
    command -v xdg-open && xdg-open http://localhost:3000 &> /dev/null
fi