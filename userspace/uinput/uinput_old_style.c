#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/uinput.h>

int fd;

void send_event(int type, int code, int val)
{
	struct input_event ie;
	memset(&ie, 0, sizeof(ie));
	ie.type = EV_REL;
	ie.code = code;
	ie.value = val;

	if (write(fd, &ie, sizeof(ie)) < 0) {
		perror("write2");
		exit(1);
	}
}

void emit_rel(int code, int val, int syn)
{
	send_event(EV_REL, code, val);
	if (syn)
		send_event(EV_SYN, SYN_REPORT, 0);
}

int main()
{
	int i = 0;
	struct uinput_user_dev uud;

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

	memset(&uud, 0, sizeof(uud));
	snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "uinput_old_style");
	uud.id.bustype = BUS_USB;
	uud.id.vendor  = 0x1;
	uud.id.product = 0x1;
	uud.id.version = 1;

	if (write(fd, &uud, sizeof(uud)) < 0) {
		perror("write");
		exit(1);
	}

	if (ioctl(fd, UI_DEV_CREATE) == -1) {
		perror("ioctl4");
		exit(1);
	}

	sleep(2);

	while (i < 100) {
		emit_rel(REL_X, 5, 0);
		emit_rel(REL_Y, 5, 1);
		usleep(15000);
		i++;
	}

	if (ioctl(fd, UI_DEV_DESTROY) == -1) {
		perror("ioctl5");
		exit(1);
	}

	close(fd);
	return 0;
}
