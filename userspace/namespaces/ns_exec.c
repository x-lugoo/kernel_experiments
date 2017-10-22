#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/eventfd.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <helper.h>

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

static int wait_fd = -1;
static char val = 1;

__attribute__((unused))
static int ret;

/* map user 1000 to user 0 (root) inside namespace */
static void set_maps(pid_t pid, const char *map) {
	int fd, data_len;
	char path[PATH_MAX];
	char data[] = "0 1000 1";

	if (!strncmp(map, "gid_map", 7)) {
		if (snprintf(path, PATH_MAX, "/proc/%d/setgroups", pid) < 0)
			fatalErr("snprintf");

		/* check if setgroups exists, in order to set the group map */
		fd = open(path, O_RDWR);
		if (fd == -1 && errno != ENOENT)
			fatalErr("setgroups");

		if (write(fd, "deny", 5) == -1)
			fatalErr("write setgroups");

		if (close(fd) == -1)
			fatalErr("close setgroups");
	}

	if (snprintf(path, PATH_MAX, "/proc/%d/%s", pid, map) < 0)
		fatalErr("snprintf");

	fd = open(path, O_RDWR);
	if (fd == -1)
		fatalErr("open");

	data_len = strlen(data);

	if (write(fd, data, data_len) != data_len)
		fatalErr("write");
}

static int child_func(void *arg)
{
	int flags = *(int *)arg;
	cap_t cap = cap_get_proc();

	if (flags & CLONE_NEWUSER)
		/* blocked by parent process */
		ret = read(wait_fd, &val, sizeof(char));

	printf("PID: %d, PPID: %d\n", getpid(), getppid());
	printf("eUID: %d, eGID: %d\n", geteuid(), getegid());
	printf("capabilities: %s\n", cap_to_text(cap, NULL));

	if (flags & CLONE_NEWNS) {
		/* necessary on Fedora, as it mount with propagation enabled
		 * by default:
		 * https://lwn.net/Articles/635563/
		 */
		if (mount(NULL, "/proc", NULL, MS_SLAVE, NULL) < 0)
			fatalErr("mount slave");
		/* Now the process only lists the PID's inside the namespace */
		if (mount("proc", "/proc", "proc", 0, NULL) < 0)
			fatalErr("mount proc");
		printf("/proc was made slave and remounted\n");
	}

	if (execlp("/bin/bash", "bash", NULL) == -1)
		fatalErr("execlp");

	return 0;
}

int main(int argc, char **argv)
{
	pid_t pid;
	int flags = SIGCHLD;
	int opt;

	static struct option long_opt[] = {
		{"unshare-ipc", no_argument, 0, 'i'},
		{"unshare-net", no_argument, 0, 'n'},
		{"unshare-mount", no_argument, 0, 'm'},
		{"unshare-pid", no_argument, 0, 'p'},
		{"unshare-uts", no_argument, 0, 'u'},
		{"unshare-user", no_argument, 0, 'U'},
		{0, 0, 0, 0},
	};

	while (1) {
		opt = getopt_long(argc, argv, "inmpuU", long_opt, NULL);
		if (opt == -1)
			break;

		switch (opt) {
		case 'i':
			flags |= CLONE_NEWIPC;
			break;
		case 'n':
			flags |= CLONE_NEWNET;
			break;
		case 'm':
			flags |= CLONE_NEWNS;
			break;
		case 'p':
			flags |= CLONE_NEWPID;
			break;
		case 'u':
			flags |= CLONE_NEWUTS;
			break;
		case 'U':
			flags |= CLONE_NEWUSER;
			break;
		default:
			fprintf(stderr, "Usage: %s <-i for ipcns> <-n for"
					"netns> <-m for newns> <-p for pidns>"
					" <-u for newuts> ,-U for userns\n"
					, argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/* avoid acquiring capabilities form the executable file on execlp */
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0, 0) == -1)
		fatalErr("PR_SET_NO_NEW_PRIVS");

	if (flags & CLONE_NEWUSER) {
		wait_fd = eventfd(0, EFD_CLOEXEC);
		if (wait_fd == -1)
			fatalErr("eventfd");
	}

	/* stack grows downward */
	pid = clone(child_func, child_stack + STACK_SIZE, flags
			, (void *)&flags);
	if (pid == -1)
		fatalErr("clone");

	if (flags & CLONE_NEWUSER) {
		set_maps(pid, "uid_map");
		set_maps(pid, "gid_map");
		ret = write(wait_fd, &val, 8);
	}

	if (waitpid(pid, NULL, 0) == -1)
		fatalErr("waitpid");

	return 0;
}
