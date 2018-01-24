#!/bin/bash

# Create cgroups
cgcreate -g devices:/sandbox

# Deny access to devices
cgset -r devices.deny=a sandbox
cgset -r devices.allow="c *:* m" sandbox
cgset -r devices.allow="b *:* m" sandbox
# Allow access to console, null, zero, random, unrandom
for d in "c 5:0" "c 5:1" "c 5:2" "c 1:3" "c 1:5" "c 1:7" "c 1:8" "c 1:9" "c 136:*" ; do
	cgset -r devices.allow="$d rw" sandbox
done

# Join cgroup, netns and activate resources limit
cgexec -g devices:/sandbox	\
	prlimit --nofile=256 --nproc=512 --locks=32	\
	unshare --mount --uts --ipc --pid --mount-proc=/proc	\
--fork sh -c "
mount -t tmpfs none /home
mount -t tmpfs none /tmp
mount -t tmpfs none /sys
mount -t tmpfs none /var/log
exec su -l ${SUDO_USER}
"
