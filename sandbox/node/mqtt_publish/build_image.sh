#!/bin/bash
cd -- `dirname -- $0`
source ./env.sh
docker build \
  --tag ${IMAGE_NAME} \
  .
