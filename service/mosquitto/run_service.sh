#!/bin/bash
set -eu
source ./env.sh
docker stop ${CONTAINER_NAME} || true
docker rm   ${CONTAINER_NAME} || true
docker run --detach \
  --restart unless-stopped \
  --publish 1883:1883 \
  --publish 9001:9001 \
  --name ${CONTAINER_NAME} \
  ${IMAGE_NAME}
