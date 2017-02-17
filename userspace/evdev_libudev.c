#include <libudev.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main()
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;
	const char *node;

	struct libevdev *ev= NULL, *uev = NULL;
	struct libevdev_uinput *uinput_ev = NULL;
	int fd, rc = 1, ufd, joyfound = 0;

	udev = udev_new();
	if (!udev) {
		perror("udev_new");
		return 1;
	}

	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;

		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		node = udev_device_get_devnode(dev);
		if (!node)
			continue;

		// TODO: find a way to find a joypad device and it's event
		// handler
		dev = udev_device_get_parent_with_subsystem_devtype(dev
						, "usb", "usb_device");
		if (!dev)
			continue;

		const char *m = udev_device_get_sysattr_value(dev, "manufacturer");
		if (!m)
			continue;

		if (strncmp(m, "Dragon", 6))
			continue;

		// found a dragonrise controller, stop
		joyfound = 1;
		break;
	}

	if (!joyfound) {
		fprintf(stderr, "Dragonrise not found\n");
		return 1;
	}

	fd = open(node, O_RDONLY | O_NONBLOCK);
	rc = libevdev_new_from_fd(fd, &ev);
	if (rc < 0) {
		fprintf(stderr, "failed to init evdev: %s\n", strerror(-rc));
		return 1;
	}

	printf("Dev node path: %s\n", node);
	printf("Vendor ID/Product ID: %x/%x\n", libevdev_get_id_vendor(ev)
						, libevdev_get_id_product(ev));
	printf("Manufacturer/Product: %s\n", libevdev_get_name(ev));

	ufd = open("/dev/uinput", O_RDWR);
	if (fd == -1) {
		perror("open uinput");
		return 1;
	}

	uev = libevdev_new();
	libevdev_set_name(uev, "input_mapper");
	libevdev_enable_event_code(uev, EV_KEY, KEY_LEFT, NULL);
	libevdev_enable_event_code(uev, EV_KEY, KEY_RIGHT, NULL);

	if (libevdev_uinput_create_from_device(uev, ufd, &uinput_ev)) {
		fprintf(stderr, "uinput could not be created\n");
		udev_enumerate_unref(enumerate);
		udev_unref(udev);
		return 1;
	}

	libevdev_free(uev);

	do {
		struct input_event iev;
		rc = libevdev_next_event(ev, LIBEVDEV_READ_FLAG_NORMAL, &iev);
		// TODO: check ABS_Z
		if ((iev.type == EV_ABS && iev.code == ABS_Z) || iev.type == EV_SYN)
			continue;
		if (rc == 0)
			printf("Event: %s %s %d\n"
				, libevdev_event_type_get_name(iev.type)
				, libevdev_event_code_get_name(iev.type, iev.code)
				, iev.value);

		// redirect event to uinput, and convert to arrow keys
		if (iev.type == EV_ABS && iev.code == ABS_X && (iev.value == 0 || iev.value == 255)) {
			int key = iev.value == 0 ? KEY_LEFT : KEY_RIGHT;

			libevdev_uinput_write_event(uinput_ev, EV_KEY, key, 1);
			libevdev_uinput_write_event(uinput_ev, EV_SYN, SYN_REPORT, 0);
			libevdev_uinput_write_event(uinput_ev, EV_KEY, key, 0);
			libevdev_uinput_write_event(uinput_ev, EV_SYN, SYN_REPORT, 0);
		}
	} while (rc == 0 || rc == 1 || rc == -EAGAIN);

	libevdev_uinput_destroy(uinput_ev);
	close(fd);
	close(ufd);

	udev_enumerate_unref(enumerate);
	udev_unref(udev);
	
	return 0;
}
