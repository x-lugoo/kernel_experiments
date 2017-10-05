#define _GNU_SOURCE

#include <limits.h> /* PATH_MAX */
#include <sched.h> /* clone */

#include <signal.h> /* SIGCHLD */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <helper.h>

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

static int child_func(void *arg)
{
	long level = (long)arg;
	char mount_point[PATH_MAX];

	if (getppid() == 0 && level) {
		snprintf(mount_point, sizeof(mount_point), "proc%c"
				, (char)('0' + level));

		if (mkdir(mount_point, 0555) == -1)
			fatalErr("mkdir");

		if (mount("proc", mount_point, "proc", 0, NULL) == -1)
			fatalErr("mount");

		printf("Mounting procfs at %s\n", mount_point);
	}

	if (level) {
		pid_t pid;

		level--;

		pid = clone(child_func, child_stack + STACK_SIZE
				, CLONE_NEWPID | SIGCHLD, (void *)level);

		if (pid == -1)
			fatalErr("clone");

		printf("PID: %d, PPID %d, waiting for process: %d\n", getpid()
				, getppid(), pid);
		if (waitpid(pid, NULL, 0) == -1)
			fatalErr("waitpid");

		level++;
		
		/* ppid is zero when we are inside a pid namespace */
		if (getppid() == 0) {
			printf("process level %ld, umounting/deleting %s\n"
				, level, mount_point);

			if (umount(mount_point) == -1)
				fatalErr("umount");
			if (rmdir(mount_point) == -1)
				fatalErr("rmdir");
		}

		printf("process level %ld, exiting\n", level);
	} else {
		printf("Last pid, in the nested pid namespaces, sleeping\n");
		if (execlp("sleep", "sleep", "10", NULL) == -1)
			fatalErr("execlp");
	}

	return 0;
}

int main(int argc, char **argv)
{
	long level;

	level = (argc > 1) ? atoi(argv[1]) : 5;
	child_func((void *)level);

	return 0;
}
