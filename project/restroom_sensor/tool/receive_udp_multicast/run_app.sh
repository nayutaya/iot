#!/bin/bash
set -eu
IMAGE_NAME=iot/restroom_sensor/receive_udp_multicast
docker run --interactive --tty --rm \
  --network host \
  ${IMAGE_NAME}
