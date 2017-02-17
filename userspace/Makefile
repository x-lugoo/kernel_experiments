CC=gcc
CFLAGS=-Wall -Werror

all:
	$(CC) scull_user.c $(CFLAGS) -o scull_user
	$(CC) uinput.c $(CFLAGS) -o uinput
	$(CC) evdev.c $(CFLAGS) -o evdev -I/usr/include/libevdev-1.0/ -levdev
	$(CC) evdev_libudev.c $(CFLAGS) -o evdev_libudev -I/usr/include/libevdev-1.0/ -levdev -ludev
	$(CC) input_dev_info.c $(CFLAGS) -o input_dev_info
clean:
	rm -rf scull_user uinput evdev evdev_libudev input_dev_info
