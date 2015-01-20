#!/bin/sh
set -x
module="lab1"
device="lab1"
insmod ./$module.ko $* || exit 1
rm -f /dev/${device}[0]
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)
mknod /dev/${device}0 c $major 0
chmod a+rw /dev/${device}0
