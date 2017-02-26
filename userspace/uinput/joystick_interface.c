#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/joystick.h>

int main(int argc, char **argv)
{
	int fd;
	char *dev_name = "/dev/input/js0";
	char name[255], buttons, axes;
	unsigned int version;

	if (argc > 1)
		dev_name = argv[1];

	fd = open(dev_name, O_WRONLY);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	if (ioctl(fd, JSIOCGVERSION, &version) == -1) {
		perror("jsver");
		exit(1);
	}
	printf("Joystick version: 0x%x\n", version);

	if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) == -1) {
		perror("jsname");
		exit(1);
	}
	printf("Name: %s\n", name);

	if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1) {
		perror("jsbtns");
		exit(1);
	}
	printf("Buttons: %d\n", buttons);

	if (ioctl(fd, JSIOCGAXES, &axes) == -1) {
		perror("jsaxes");
		exit(1);
	}
	printf("Axes: %d\n", axes);

	close(fd);

	return 0;
}
