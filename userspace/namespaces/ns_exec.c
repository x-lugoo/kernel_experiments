#define _GNU_SOURCE

#include <getopt.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <helper.h>

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

static int child_func(void *arg)
{
	int flags = *(int *)arg;

	printf("Child PID: %d\n", getpid());
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

	while ((opt = getopt(argc, argv, "inmpuU")) != -1) {
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
		}
	}

	/* stack grows downward */
	pid = clone(child_func, child_stack + STACK_SIZE, flags
			, (void *)&flags);
	if (pid == -1)
		fatalErr("clone");

	if (waitpid(pid, NULL, 0) == -1)
		fatalErr("waitpid");

	return 0;
}
