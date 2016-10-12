#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include "../drivers/scull.h"

int main()
{
	int quantum, qset;
	int fd = open("/dev/scull0", O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	if (ioctl(fd, SCULL_IOCGQUANTUM, &quantum) == -1) {
		perror("ioctl quantum");
		exit(1);
	}
	printf("quantum: %d\n", quantum);

	if (ioctl(fd, SCULL_IOCGQSET, &qset) == -1) {
		perror("ioctl qset");
		exit(1);
	}
	printf("qset: %d\n", qset);

	close(fd);
	exit(0);
}
