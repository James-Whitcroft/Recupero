#!/bin/bash
# This script was written for use of ubuntu...anything else 
# untested!!!

if [ "$EUID" -ne 0 ]
    then echo "Please run as root"
    exit
fi

dd if=/dev/zero of=./disk.dd count=512 bs=1MB

mkfs.ext2 disk.dd

mkdir mount

cd mount

echo "test test test test test test \n" >> testfile 
