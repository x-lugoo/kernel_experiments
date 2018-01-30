#!/bin/bash

TARGETDIR="/mnt/chroot"

mkdir -p $TARGETDIR/{bin,lib64,proc,sys,dev,dev/shm,dev/pts}

cp /bin/bash $TARGETDIR/bin
cp /lib64/{libtinfo.so.*,libdl.so.*,libc.so.*,ld-linux-x86-64.so.*} $TARGETDIR/lib64

mount -t proc proc $TARGETDIR/proc
mount -t sysfs sysfs $TARGETDIR/sys
mount -t devtmpfs devtmpfs $TARGETDIR/dev
mount -t tmpfs tmpfs $TARGETDIR/dev/shm
mount -t devpts devpts $TARGETDIR/dev/pts

# Copy /etc/hosts
/bin/cp -f /etc/hosts $TARGETDIR/etc/

# Copy /etc/resolv.conf
/bin/cp -f /etc/resolv.conf $TARGETDIR/etc/resolv.conf

chroot $TARGETDIR
