#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "helper.h"

#define STACK_SIZE (1024  * 1024)
static char child_stack[STACK_SIZE];

static int child_func(void *arg)
{
	struct stat st;
	(void)arg;

	if (mkdir("/tmp/foo", 0755) == -1 && errno != EEXIST)
		fatalErr("mkdir");

	if (mount("none", "/tmp/foo", "tmpfs", 0, NULL) == -1)
		fatalErr("mount");

	if (stat("/tmp/foo", &st) == -1)
		fatalErr("stat");

	printf("UID: %d, GID %d\n", st.st_uid, st.st_gid);

	return 0;
}

int main(void)
{
	pid_t pid;

	pid = clone(child_func, child_stack + STACK_SIZE
			, CLONE_NEWNS | CLONE_NEWUSER | SIGCHLD, NULL);

	if (pid == -1)
		fatalErr("clone");
	return 0;
}
