#!/bin/bash
set -eu
source ./env.sh
docker stop ${CONTAINER_NAME} || true
docker rm   ${CONTAINER_NAME} || true
docker run --detach \
  --env MQTT_SERVER_URL \
  --publish 1010:8080 \
  --restart unless-stopped \
  --name ${CONTAINER_NAME} \
  ${IMAGE_NAME}
