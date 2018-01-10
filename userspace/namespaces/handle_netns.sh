#!/bin/bash

# References of iptables:
#	https://gist.github.com/dpino/6c0dca1742093346461e11aa8f608a99

set -x

if [[ $EUID -ne 0 ]]; then
	echo "You need to be root! Aborting"
	exit 1
fi

NS="guest"
VH_ADDR="10.200.1.1"
VP_ADDR="10.200.1.2"

ip link delete vHOST &>/dev/null
ip netns delete $NS &>/dev/null

ip netns add $NS
ip link add vHOST type veth peer name vPEER
ip link set vPEER netns $NS

ip addr add $VH_ADDR/24 dev vHOST
ip link set vHOST up

ip netns exec $NS ip link set vPEER name eth0
ip netns exec $NS ip link set eth0 up
ip netns exec $NS ip addr add $VP_ADDR/24 dev eth0
ip netns exec $NS ip route add default via $VH_ADDR

# enable ip forward
echo 1 > /proc/sys/net/ipv4/ip_forward

# Flush forward rules
iptables -P FORWARD DROP
iptables -F FORWARD

# Flush nat rules
iptables -t nat -F

# Add masquerading to host veth
iptables -t nat -A POSTROUTING -s $VH_ADDR/24 -o enp0s8 -j MASQUERADE
iptables -A FORWARD -i enp0s8 -o $VH_ADDR -j ACCEPT
iptables -A FORWARD -o enp0s8 -i $VH_ADDR -j ACCEPT

# execute bash inside the "guest" network namespace
ip netns exec $NS bash
