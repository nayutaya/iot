#!/bin/bash
IMAGE_NAME=iot/restroom_sensor/receive_udp_multicast
cd -- `dirname -- $0`
docker build \
  --tag ${IMAGE_NAME} \
  .
