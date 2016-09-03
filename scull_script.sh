#!/bin/bash
module="scull"
device="scull"
mode="664"

# remove module if already loaded
if lsmod | grep $module >/dev/null; then
	/sbin/rmmod $module
fi

# call insmod with all parameters 
/sbin/insmod ./$module.ko $* || exit 1

# remove stale nodes
rm -f /dev/${device}[0-3]

major=$(grep $module /proc/devices | awk '{print $1}')
echo $major

mknod /dev/${device}0 c $major 0
chmod 777 /dev/${device}[0-3]

dmesg -c >/dev/null
