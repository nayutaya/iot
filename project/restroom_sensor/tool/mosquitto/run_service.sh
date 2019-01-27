#!/bin/bash
set -eu
source ./env.sh
docker run --interactive --tty --rm \
  --network host \
  --publish 1883:1883 \
  --publish 9001:9001 \
  ${IMAGE_NAME}
