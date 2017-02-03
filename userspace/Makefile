CC=gcc
CFLAGS=-Wall -Werror

all:
	$(CC) scull_user.c $(CFLAGS) -o scull_user
	$(CC) uinput.c $(CFLAGS) -o uinput
	$(CC) evdev.c $(CFLAGS) -o evdev -I/usr/include/libevdev-1.0/ -levdev
clean:
	rm -rf scull_user
