#!/bin/bash

rsync -avz --exclude-from=${HOME}/research/etc/rsync-excludes \
    --exclude="checkpoint*" --exclude="dist_transfer.tar.gz" \
    -e ssh hjg@hpc.msu.edu:himalaya/expr/$1 var/
