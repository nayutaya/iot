#!/bin/bash
cd -- `dirname -- $0`
source ./env.sh
docker build \
  --build-arg REACT_APP_LOCATION_NAME=${REACT_APP_LOCATION_NAME} \
  --tag ${IMAGE_NAME} \
  .
