#!/bin/bash
# This script was written for use of ubuntu...anything else 
# untested!!!


if [ "$EUID" -ne 0 ]
    then echo "Please run as root"
    exit
fi


umount /mount

rm disk.dd
rm -r mount
