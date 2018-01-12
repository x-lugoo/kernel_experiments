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
	echo "Usage: handle_netns.sh <if>"
	exit 1
fi

HOST_IF=$1

NS="guest"
VH_ADDR="10.10.1.1"
VP_ADDR="10.10.1.2"

ip link delete vHOST &>/dev/null
ip netns delete $NS &>/dev/null

ip netns add $NS
ip link add vHOST type veth peer name vPEER
ip link set vPEER netns $NS

ip addr add $VH_ADDR/24 dev vHOST
ip link set vHOST up

ip netns exec $NS ip link set lo up
ip netns exec $NS ip link set vPEER name eth0
ip netns exec $NS ip link set eth0 up
ip netns exec $NS ip addr add $VP_ADDR/24 dev eth0
ip netns exec $NS ip route add default via $VH_ADDR

# enable ip forward
echo 1 > /proc/sys/net/ipv4/ip_forward

# Add masquerading to host veth
iptables -t nat -A POSTROUTING -j MASQUERADE
iptables -A FORWARD -i $HOST_IF -o vHOST -j ACCEPT
iptables -A FORWARD -i vHOST -o $HOST_IF -j ACCEPT

# execute bash inside the "guest" network namespace
ip netns exec $NS bash
