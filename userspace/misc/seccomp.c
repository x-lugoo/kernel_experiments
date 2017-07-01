#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/filter.h>
#include <linux/audit.h>
#include <linux/seccomp.h>
#include <sys/ptrace.h>
#include <sys/prctl.h>

#define FTEST "/tmp/test"

void check_strict()
{
	int ret;

	printf("Setting seccomp...\n");

	// TODO: check why this doesn't work
	//ret = seccomp(SECCOMP_MODE_STRICT, 0, NULL);
	ret = prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
	if (ret) {
		perror("seccomp");
		exit(1);
	}

	printf("seccomp set, try redirecting stderr to stdout (should be killed)\n");

	dup2(1, 2);

	printf("Should not be printed\n");
}

int main(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "sf")) != -1) {
		switch (opt) {
		case 's':
			check_strict();
			break;
		case 'f':
			//check_filter();
			break;
		default:
			fprintf(stderr, "Usage: -s (for strict)\n");
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
