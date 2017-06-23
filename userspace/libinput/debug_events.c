#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <libinput.h>
#include <libudev.h>

static int stop = 0;

static void sighandler(int signal, siginfo_t *si, void *userdata)
{
	(void)signal;
	(void)si;
	(void)userdata;
	stop = 1;
}

static void print_events(struct libinput *li)
{
	struct libinput_event *ev;

	libinput_dispatch(li);
	while ((ev = libinput_get_event(li))) {
		switch (libinput_event_get_type(ev)) {
		case LIBINPUT_EVENT_KEYBOARD_KEY:
			fprintf(stderr, "Keyboard event\n");
			break;
		case LIBINPUT_EVENT_POINTER_MOTION:
			fprintf(stderr, "Pointer motion event\n");
			break;
		default:
			fprintf(stderr, "Need to handle\n");
		}
		libinput_event_destroy(ev);
		libinput_dispatch(li);
	}
}

static void mainloop(struct libinput *li)
{
	struct pollfd fds;
	struct sigaction act;

	fds.fd = libinput_get_fd(li);
	fds.events = POLLIN;
	fds.revents = 0;

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sighandler;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGINT, &act, NULL) == -1) {
		fprintf(stderr, "Failed to set sig acts (%s)\n"
				, strerror(errno));
		return;
	}

	while (!stop && poll(&fds, 1, -1) > -1)
		print_events(li);
}

static void simple_log_handler(struct libinput *li
				, enum libinput_log_priority lp
				, const char *format
				, va_list va)
{
	(void)li;
	(void)lp;
	vfprintf(stdout, format, va);
}

static int open_restricted(const char *path, int flags, void *user_data)
{
	(void)user_data;
	int fd = open(path, flags);
	if (fd < 0)
		fprintf(stderr, "Failed to open %s (%s)\n"
				, path, strerror(errno));
	return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data)
{
	(void)user_data;
	close(fd);
}

static const struct libinput_interface interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted
};

int main()
{
	struct libinput *li;
	struct udev *udev = udev_new();
	int ret = 1;

	if (!udev) {
		fprintf(stderr, "Failed to init udev\n");
		exit(1);
	}

	li = libinput_udev_create_context(&interface, NULL, udev);
	if (!li) {
		fprintf(stderr, "Failed to init context from udev\n");
		goto udev_out;
	}

	libinput_log_set_handler(li, simple_log_handler);
	libinput_log_set_priority(li, LIBINPUT_LOG_PRIORITY_DEBUG);

	if (libinput_udev_assign_seat(li, "seat0")) {
		fprintf(stderr, "Failed to assign seat to udev\n");
		goto libinput_out;
	}

	mainloop(li);

	ret = 0;

libinput_out:
	libinput_unref(li);
udev_out:
	udev_unref(udev);

	return ret;
}
