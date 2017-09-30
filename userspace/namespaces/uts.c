#define _GNU_SOURCE /* enables clone() */

#include <fcntl.h> /* O_RDONLY */
#include <sched.h> /* clone(), setns() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strlen() */
#include <sys/types.h>
#include <sys/utsname.h> /* uname() */
#include <sys/wait.h> /* SIGCHLD, wait() */
#include <unistd.h> /* sethostname, setdomainname() */

#define STACK_SIZE (1024 * 1024)
static char child_stack1[STACK_SIZE];
static char child_stack2[STACK_SIZE];

static int new_utsns(void *arg)
{
	struct utsname uts;
	int pid = getpid();

	if (uname(&uts) == -1) {
		perror("uname");
		exit(EXIT_FAILURE);
	}
	printf("(%d) uts.nodename (before sethosnamet): %s\n", pid
			, uts.nodename);

	if (sethostname(arg, strlen(arg)) == -1) {
		perror("sethostname");
		exit(EXIT_FAILURE);
	}

	if (uname(&uts) == -1) {
		perror("uname");
		exit(EXIT_FAILURE);
	}
	printf("(%d) uts.nodename (after sethostname): %s\n", pid
			, uts.nodename);

	printf("(%d) Wait for one more process to enter in the same"
			" namespace...\n", pid);
	sleep(3);

	return 0;
}

static int new_proc_same_ns(void *arg)
{
	struct utsname uts;
	char nspath[255];
	int fd, lpid = getpid();
	int *pid = (int *)arg;

	if (snprintf(nspath, sizeof(nspath), "/proc/%d/ns/uts", *pid) < 0) {
		perror("snprintf");
		exit(EXIT_FAILURE);
	}

	fd = open(nspath, O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if (setns(fd, CLONE_NEWUTS) == -1) {
		perror("setns");
		exit(EXIT_FAILURE);
	}

	if (uname(&uts) == -1) {
		perror("uname");
		exit(EXIT_FAILURE);
	}

	printf("(%d) uts.nodename (inside ns): %s\n", lpid, uts.nodename);

	if (close(fd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int main (int argc, char **argv)
{
	pid_t pid[2];
	struct utsname uts;
	int lpid = getpid();

	if (argc != 2) {
		fprintf(stderr, "Usage: utc new_namespace_hostname\n");
		exit(EXIT_FAILURE);
	}

	if (getuid() != 0) {
		fprintf(stderr, "This program uses CLONE_NEWUTS, so only root "
				"can do that. Please retry as root "
				"(or any user with CAP_SYS_ADMIN)\n");
		exit(EXIT_FAILURE);
	}

	if (uname(&uts) == -1) {
		perror("uname");
		exit(EXIT_FAILURE);
	}
	printf("(%d) uts.nodename in parent: %s\n", lpid, uts.nodename);

	pid[0] = clone(new_utsns, child_stack1 + STACK_SIZE
			, CLONE_NEWUTS | SIGCHLD, argv[1]);
	if (pid[0] == -1) {
		perror("clone1");
		exit(EXIT_FAILURE);
	}

	pid[1] = clone(new_proc_same_ns, child_stack2 +STACK_SIZE, SIGCHLD
			, &(pid[0]));
	if (pid[0] == -1) {
		perror("clone2");
		exit(EXIT_FAILURE);
	}

	/* first wait for the second process, which terminates earlier */
	if (waitpid(pid[1], NULL, 0) == -1) {
		perror("waitipid1");
		exit(EXIT_FAILURE);
	}

	if (waitpid(pid[0], NULL, 0) == -1) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	printf("(%d) All things set, exiting from parent\n", lpid);

	return 0;
}
