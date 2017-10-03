#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <helper.h>

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

static void show_caps()
{
	cap_t cap;

	cap = cap_get_proc();
	if (!cap)
		fatalErr("cap_get_proc");

	printf("PID: %d, UID: %d, GID: %d\n", getpid(), getuid(), getgid());
	printf("\tcaps: %s\n", cap_to_text(cap, NULL));
}

static int childFunc(void *arg)
{
	(void)arg;
	printf("Child inside namespace\n");
	show_caps();

	return 0;
}

int main(int argc, char **argv)
{
	pid_t pid;
	int clone_user = 0;
	int clone_pid = 0;
	int clone_flags = SIGCHLD;
	int c;

	while ((c = getopt(argc, argv, "up")) != -1) {
		switch (c) {
		case 'u':
			clone_user = 1;
			break;
		case 'p':
			clone_pid = 1;
			break;
		default:
			fprintf(stderr, "Usage: clone <-p> <-u>\n");
			exit(EXIT_FAILURE);
		}
	}

	if (clone_user)
		clone_flags |= CLONE_NEWUSER;
	if (clone_pid)
		clone_flags |= CLONE_NEWPID;

	printf("Parent\n");
	show_caps();

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) <0)
		fatalErr("prctl");

	pid = clone(childFunc, child_stack + STACK_SIZE /* stack growsdownward */
			, clone_flags, NULL);

	if (pid == -1)
		fatalErr("clone");

	if (waitpid(pid, NULL, 0) == -1)
		fatalErr("waitpid");

	return 0;
}
