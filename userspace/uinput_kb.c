#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <linux/input.h>
#include <linux/uinput.h>

static int fd;
static struct uinput_setup usetup;

void emit(int type, int code, int val)
{
	struct input_event ev = {
		.type = type,
		.code = code,
		.value = val
	};

	if (write(fd, &ev, sizeof(ev)) != sizeof(ev)) {
		perror("write");
		exit(1);
	}
}

void send_event(int type, int code, int val)
{
	emit(type, code, val);
	emit(EV_SYN, SYN_REPORT, 0);
}

int main()
{
	struct input_id uid;

	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_KEY) == -1) {
		perror("iotctl1");
		exit(1);
	}

	if (ioctl(fd, UI_SET_KEYBIT, KEY_SPACE) == -1) {
		perror("iotctl2");
		exit(1);
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_REL) == -1) {
		perror("iotctl1");
		exit(1);
	}

	if (ioctl(fd, UI_SET_RELBIT, REL_X) == -1) {
		perror("iotctl2");
		exit(1);
	}

	memset(&uid, 0, sizeof(uid));
	memset(&usetup, 0, sizeof(usetup));
	usetup.id = uid;
	strcpy(usetup.name, "my_device");

	if (ioctl(fd, UI_DEV_SETUP, &usetup) == -1) {
		perror("dev setup");
		exit(1);
	}

	if (ioctl(fd, UI_DEV_CREATE) == -1) {
		perror("ioctl dev create");
		exit(1);
	}

	// necessary, waits to entire system to discover the new input device and handle events
	sleep(1);

	//send_event(EV_KEY, KEY_SPACE, 1);
	//send_event(EV_KEY, KEY_SPACE, 0);

	int i;
	for (i = 0; i < 20; i++) {
		send_event(EV_REL, REL_X, 10);
		send_event(EV_REL, REL_Y, 10);
		usleep(1500);
	}


	if (ioctl(fd, UI_DEV_DESTROY) == -1) {
		perror("ioctl dev destroy");
		exit(1);
	}

	close(fd);
	exit(0);
}
