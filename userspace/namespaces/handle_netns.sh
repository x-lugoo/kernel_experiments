#!/bin/bash

if [ "$#" -ne 1 ];
then
	echo "Usage: $0 <create|delete>"
	exit 1
fi

if [ "$1" = "create" ];
then
	ip netns add netns1
	ip link add veth1 type veth peer name veth2
	ip link set veth2 netns netns1

	ip link add bridge0 type bridge
	ip addr add 172.17.42.1/24 dev bridge0
	ip link set bridge0 up
	ip link set veth1 master bridge0
	ip link set veth1 up

	ip netns exec netns1 ip link set veth2 name eth0
	ip netns exec netns1 ip link set eth0 up
	ip netns exec netns1 ip addr add 172.17.42.100/24 dev eth0
	ip netns exec netns1 ip route add default via 172.17.42.1
fi

if [ "$1" = "delete" ];
then
	ip link delete veth1
	ip link delete bridge0
	ip netns delete netns1
fi

exit 0
