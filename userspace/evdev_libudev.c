#include <libudev.h>
#include <libevdev/libevdev.h>

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

	struct libevdev *ev= NULL;
	int fd, rc = 1;

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

		// TODO: filter by event device
		dev = udev_device_get_parent_with_subsystem_devtype(dev
						, "usb", "usb_device");
		if (!dev)
			continue;

		const char *m = udev_device_get_sysattr_value(dev, "manufacturer");
		if (!m)
			continue;

		printf("Dev node path: %s\n", node);
		printf("Vendor ID/Product ID: %s/%s\n"
			, udev_device_get_sysattr_value(dev, "idVendor")
			, udev_device_get_sysattr_value(dev, "idProduct"));
		printf("Manufacturer/Product: %s/%s\n"
			, m
			, udev_device_get_sysattr_value(dev, "product"));
		// found a dragonrise controller, stop
		if (!strncmp(m, "Dragon", 6))
			break;
	}

	fd = open(node, O_RDONLY | O_NONBLOCK);
	rc = libevdev_new_from_fd(fd, &ev);
	if (rc < 0) {
		fprintf(stderr, "failed to init evdev: %s\n", strerror(-rc));
		return 1;
	}

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
	} while (rc == 0 || rc == 1 || rc == -EAGAIN);

	udev_enumerate_unref(enumerate);
	udev_unref(udev);
	
	return 0;
}
