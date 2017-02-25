#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>

#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	struct libevdev *dev = NULL;
	struct libevdev_uinput *uidev = NULL;

	int fd = open("/dev/uinput", O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	dev = libevdev_new();
	libevdev_set_name(dev, "my_evdev_device");
	libevdev_enable_event_code(dev, EV_KEY, KEY_SPACE, NULL);

	if (libevdev_uinput_create_from_device(dev, fd, &uidev) < 0) {
		printf("error creating evdev from uinput\n");
		libevdev_free(dev);
	}

	/* not necessary anymore */
	libevdev_free(dev);

	/* necesary, waits for the Screen Server to handle the incoming events */
	sleep(1);

	libevdev_uinput_write_event(uidev, EV_KEY, KEY_SPACE, 1);
	libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
	libevdev_uinput_write_event(uidev, EV_KEY, KEY_SPACE, 0);
	libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);

	libevdev_uinput_destroy(uidev);
	close(fd);

	return 0;
}
