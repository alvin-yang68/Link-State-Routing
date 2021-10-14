#!/bin/bash

docker build -t cs435_mp2 .
docker run --name cs435_mp2_compiled cs435_mp2
docker cp cs435_mp2_compiled:/usr/src/cs435_mp2/ls_router .
docker rm cs435_mp2_compiled
docker rmi cs435_mp2