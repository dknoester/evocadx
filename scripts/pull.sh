#!/bin/bash

rsync -avz --exclude-from=${HOME}/research/etc/rsync-excludes \
    --exclude="checkpoint*" --exclude="dist_transfer.tar.gz" \
    -e ssh dk@hpc.msu.edu:/mnt/home/dolsonem/evocadx/expr/ var/
