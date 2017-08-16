/*Thanks to https://gist.github.com/tiebingzhang/aafc2953b430d5586bd1135cad85100f*/
/*
 * More info: 	man 7 rtnetlnk
 * man 3 rtnetlnk
 * man 7 netlnk
 * man 3 netlnk
 * https://en.wikipedia.org/wiki/Netlink
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <net/if.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

/*
 * netlink is used to transfer information between kernel and userspace processes
 * RTnetlink allows the kernel's routing tables to be read and altered. It can be used
 * by userpace programs when combined with netlink messages
 * netlink messages
 **/

int main(void)
{
	int fd, len;
	struct sockaddr_nl sa;
	struct nlmsghdr *nh;
	struct ifinfomsg *rtif;
	char ifname[IF_NAMESIZE], buf[4096];

	struct iovec iov = { buf, sizeof(buf) };

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	/*RTMGRP: RTnetlink Multicast Group*/
	sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

        if (bind(fd, (struct sockaddr *) &sa, sizeof(sa))) {
		perror("bind");
		close(fd);
		exit(EXIT_FAILURE);
	}

	while(1) {
		struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
		len = recvmsg(fd, &msg, 0);
		if (len == -1) {
			perror("recvmsg");
			close(fd);
			exit(EXIT_FAILURE);
		}

		for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
			/* The end of multipart message. */
			if (nh->nlmsg_type == NLMSG_DONE){
				printf("got msg done\n");
				break;
			}

			if (nh->nlmsg_type == NLMSG_ERROR){
				printf("got msg error\n");
				continue;
			}

			/*Filtering to RTM_NEWLINK, RTM_DELLINK, RTM_GETLINK and RTM_SETLINK*/
			if (nh->nlmsg_type < RTM_NEWADDR){
				rtif = (struct ifinfomsg *)NLMSG_DATA(nh);
				printf("Interface %s is %s\n"
						, if_indextoname(rtif->ifi_index,ifname)
						, (rtif->ifi_flags&IFF_RUNNING)
							? "Connected"
							: "Disconnected");
			}
		}
	}
	return 0;
}

