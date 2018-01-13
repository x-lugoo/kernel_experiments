#!/bin/bash

# References of iptables:
#	https://gist.github.com/dpino/6c0dca1742093346461e11aa8f608a99
#	https://blog.yadutaf.fr/2014/01/19/introduction-to-linux-namespaces-part-5-net/
#	https://bbs.archlinux.org/viewtopic.php?id=114020

if [[ $EUID -ne 0 ]]; then
	echo "You need to be root! Aborting"
	exit 1
fi

if [ $# -ne 1 ]; then
	echo "Usage: handle_netns.sh <ifbr>"
	exit 1
fi

IF_BR=$1

NS="guest"
ip link delete vHOST &>/dev/null
ip netns delete $NS &>/dev/null

ip netns add $NS
ip link add vHOST type veth peer name vPEER
ip link set vPEER netns $NS
ip link set vHOST up
ip link set vHOST master "$IF_BR"

# IP's based in virbr0
ip netns exec $NS ip link set lo up
ip netns exec $NS ip link set vPEER name eth0
ip netns exec $NS ip link set eth0 up
ip netns exec $NS ip addr add 192.168.122.100/24 dev eth0
ip netns exec $NS ip route add default via 192.168.122.1

# execute bash inside the "guest" network namespace
ip netns exec $NS bash
