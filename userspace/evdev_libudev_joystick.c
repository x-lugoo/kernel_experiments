#include <libudev.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* udev docs: https://www.freedesktop.org/software/systemd/man/#U */

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
	udev_enumerate_add_match_property(enumerate, "ID_INPUT_JOYSTICK", "1");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		/* when dealing with a joystick device, the final device will be
		 * an event and/or a joystick, so node will be empty
		 */
		node = udev_device_get_devnode(dev);
		if (!node)
			continue;

		/* our filter is set to find a joystick who is controlled
		 * by USB bus only */
		dev = udev_device_get_parent_with_subsystem_devtype(dev
						, "usb", "usb_device");
		if (!dev)
			continue;

		/* one joystick gets the following scheme on udev:
		 * inputX	(doesn't have an entry on /dev)
		 * 	eventX	(dev/input/eventX)
		 * 	jsX	(/dev/input/jsX)
		 * For more info, see udev-browse application
		 *
		 * As we are using evdev here, the code above will fail
		 * with inputX, and jsX, letting just eventX to control the
		 * device.
		 */
		fd = open(node, O_RDONLY | O_NONBLOCK);
		if (libevdev_new_from_fd(fd, &ev) == 0) {
			joyfound = 1;
			break;
		}
	}

	if (!joyfound) {
		fprintf(stderr, "Joystick not found, aborting.\n");
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
