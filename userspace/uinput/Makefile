CC=gcc
CFLAGS=-Wall -Werror

all:
	$(CC) uinput_kb.c $(CFLAGS) -o uinput_kb
	$(CC) evdev.c $(CFLAGS) -o evdev -I/usr/include/libevdev-1.0/ -levdev
	$(CC) evdev_libudev_joystick.c $(CFLAGS) -o evdev_libudev_joystick -I/usr/include/libevdev-1.0/ -levdev -ludev
	$(CC) event_dev_info.c $(CFLAGS) -o event_dev_info
	$(CC) hid_dev_info.c $(CFLAGS) -o hid_dev_info
clean:
	rm -rf uinput_kb evdev evdev_libudev_joystick event_dev_info hid_dev_info
