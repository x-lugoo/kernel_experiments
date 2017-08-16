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

#include <linux/netlink.h>

 /*netlink is used to transfer information between kernel and userspace processes*/


int main(void)
{
	int fd, len;
	struct sockaddr_nl sa;
	char buf[4096];

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_groups = -1;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
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
		len = recv(fd, buf, sizeof(buf), 0);
		if (len == -1) {
			perror("recvmsg");
			close(fd);
			exit(EXIT_FAILURE);
		}

		printf("Kernel message: %s\n", buf);
	}
	return 0;
}

