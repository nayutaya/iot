#!/bin/bash
set -eu
source ./env.sh
docker run --interactive --tty --rm \
  --network host \
  --volume $(pwd):/workspace \
  ${IMAGE_NAME} \
  /bin/bash
