#!/bin/sh

# Halium 9
mkdir -p /dev/__properties__
mkdir -p /dev/socket

# Mount a tmpfs on /apex if we should
if [ -e "/apex" ]; then
    mount -t tmpfs tmpfs /apex
fi

# mount binderfs if needed
if [ ! -e /dev/binder ]; then
    mkdir -p /dev/binderfs
    mount -t binder binder /dev/binderfs -o stats=global
    ln -s /dev/binderfs/*binder /dev
fi
