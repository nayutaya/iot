#!/bin/bash
set -eu
source ./env.sh
docker run --interactive --tty --rm \
  --env MQTT_SERVER_URL \
  --volume $(pwd):/workspace \
  ${IMAGE_NAME} \
  /bin/bash
