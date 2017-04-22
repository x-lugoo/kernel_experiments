#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/uinput.h>

int fd;

void emit(int type, int code, int val)
{
	struct input_event ie;

	ie.type = type;
	ie.code = code;
	ie.value = val;
	ie.time.tv_sec = 0;
	ie.time.tv_usec = 0;

	if (write(fd, &ie, sizeof(ie)) < 0) {
		perror("write2");
		exit(1);
	}
}

int main()
{
	int i = 50;
	struct uinput_setup usetup;

	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_KEY) == -1) {
		perror("ioctl0");
		exit(1);
	}

	if (ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) == -1) {
		perror("ioctl0.1");
		exit(1);
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_REL) == -1) {
		perror("ioctl1");
		exit(1);
	}

	if (ioctl(fd, UI_SET_RELBIT, REL_X) == -1) {
		perror("ioctl2");
		exit(1);
	}

	if (ioctl(fd, UI_SET_RELBIT, REL_Y) == -1) {
		perror("ioctl3");
		exit(1);
	}

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x1234;
	snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "Example of uinput mouse");

	if (ioctl(fd, UI_DEV_SETUP, &usetup) == -1) {
		perror("dev setup");
		exit(1);
	}

	if (ioctl(fd, UI_DEV_CREATE) == -1) {
		perror("ioctl4");
		exit(1);
	}

	/* wait some time to let the Window Manager to get the new virtual device */
	sleep(1);

	/* move the mouse cursor to 5 units per axis */
	while (i--) {
		emit(EV_REL, REL_X, 5);
		emit(EV_REL, REL_Y, 5);
		emit(EV_SYN, SYN_REPORT, 0);
		usleep(15000);
	}

	if (ioctl(fd, UI_DEV_DESTROY) == -1) {
		perror("ioctl5");
		exit(1);
	}

	close(fd);
	return 0;
}
