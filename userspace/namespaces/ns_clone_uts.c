#define _GNU_SOURCE /* enables clone() */

#include <fcntl.h> /* O_RDONLY */
#include <sched.h> /* clone(), setns() */
#include <signal.h> /* signal() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strlen() */
#include <sys/types.h>
#include <sys/utsname.h> /* uname() */
#include <sys/wait.h> /* SIGCHLD, wait() */
#include <unistd.h> /* sethostname, setdomainname() */

#include <helper.h>

#define STACK_SIZE (1024 * 1024)
static char child_stack1[STACK_SIZE];
static char child_stack2[STACK_SIZE];

static void set_signals(struct sigaction *sigact, sigset_t *waitset)
{
	sigemptyset(&(sigact->sa_mask));
	sigemptyset(waitset);

	sigaction(SIGUSR1, sigact, NULL);
	sigaddset(waitset, SIGUSR1);

	sigprocmask(SIG_BLOCK, waitset, NULL);
}

static int new_utsns(void *arg)
{
	struct utsname uts;
	struct sigaction sigact;
	sigset_t waitset;
	int sig, pid = getpid();

	set_signals(&sigact, &waitset);

	if (uname(&uts) == -1)
		fatalErr("uname");

	printf("(%d) uts.nodename (before sethostname): %s\n", pid
			, uts.nodename);

	if (sethostname(arg, strlen(arg)) == -1)
		fatalErr("sethostname");

	if (uname(&uts) == -1)
		fatalErr("uname");

	printf("(%d) uts.nodename (after sethostname): %s\n", pid
			, uts.nodename);

	/* send signal to parent pid to create the second process */
	kill(getppid(), SIGUSR1);

	printf("(%d) Wait for one more process to enter in the same"
			" namespace...\n", pid);

	(void)sigwait(&waitset, &sig);

	printf("(%d) Signal received, exiting...\n", pid);

	return 0;
}

static int new_proc_same_ns(void *arg)
{
	struct utsname uts;
	char nspath[255];
	int fd, lpid = getpid();
	int pid = *(int *)arg;

	if (snprintf(nspath, sizeof(nspath), "/proc/%d/ns/uts", pid) < 0)
		fatalErr("snprintf");

	fd = open(nspath, O_RDONLY);
	if (fd == -1)
		fatalErr("open");

	if (setns(fd, CLONE_NEWUTS) == -1)
		fatalErr("setns");

	if (uname(&uts) == -1)
		fatalErr("uname");

	printf("(%d) uts.nodename (inside ns): %s\n", lpid, uts.nodename);

	if (close(fd) == -1)
		fatalErr("close");

	/* send signal to child1 to finish execution */
	kill(pid, SIGUSR1);

	return 0;
}

int main (int argc, char **argv)
{
	pid_t pid[2];
	struct sigaction sigact;
	sigset_t waitset;
	struct utsname uts;
	int sig, lpid = getpid();

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

	if (uname(&uts) == -1)
		fatalErr("uname");
	printf("(%d) uts.nodename in parent: %s\n", lpid, uts.nodename);

	set_signals(&sigact, &waitset);

	pid[0] = clone(new_utsns, child_stack1 + STACK_SIZE
			, CLONE_NEWUTS | SIGCHLD, argv[1]);
	if (pid[0] == -1)
		fatalErr("clone1");

	/* wait for pid[0] to set the new hostname before calling pid[1] */
	(void)sigwait(&waitset, &sig);

	pid[1] = clone(new_proc_same_ns, child_stack2 +STACK_SIZE, SIGCHLD
			, &(pid[0]));
	if (pid[0] == -1)
		fatalErr("clone2");

	/* first wait for the second process, which terminates earlier */
	if (waitpid(pid[1], NULL, 0) == -1)
		fatalErr("waitpid1");

	if (waitpid(pid[0], NULL, 0) == -1)
		fatalErr("waitpid0");

	printf("(%d) All things set, exiting from parent\n", lpid);

	return 0;
}
