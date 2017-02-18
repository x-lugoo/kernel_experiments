#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
	int fd;
	char name[255] = "unknown";
	char phys[255];
	char uniq[255];
	char prop[255];

	if (argc != 2) {
		fprintf(stderr, "Usage: input_dev_info /dev/input/eventX\n");
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), &name) == -1) {
		perror("ioctl1");
		exit(1);
	}
	printf("Device name: %s\n", name);

	if (ioctl(fd, EVIOCGPHYS(sizeof(phys)), &phys) == -1) {
		perror("ioctl2");
		exit(1);
	}
	printf("Physical location: %s\n", phys);

	if (ioctl(fd, EVIOCGUNIQ(sizeof(uniq)), &uniq) != -1)
		printf("Unique name: %s\n", uniq);

	if (ioctl(fd, EVIOCGPROP(sizeof(prop)), &prop) != -1)
		printf("Properties: %s\n", prop);

	close(fd);

	return 0;
}
