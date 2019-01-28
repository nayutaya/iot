#!/bin/bash
set -eu
source ./env.sh
docker run --interactive --tty --rm \
  --network host \
  ${IMAGE_NAME}
