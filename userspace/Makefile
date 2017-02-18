CC=gcc
CFLAGS=-Wall -Werror

all:
	$(CC) scull_user.c $(CFLAGS) -o scull_user
	$(CC) uinput_kb.c $(CFLAGS) -o uinput_kb
	$(CC) evdev.c $(CFLAGS) -o evdev -I/usr/include/libevdev-1.0/ -levdev
	$(CC) evdev_libudev.c $(CFLAGS) -o evdev_libudev -I/usr/include/libevdev-1.0/ -levdev -ludev
	$(CC) event_dev_info.c $(CFLAGS) -o event_dev_info
clean:
	rm -rf scull_user uinput evdev evdev_libudev event_dev_info
