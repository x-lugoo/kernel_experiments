#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <helper.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s path_to_binary\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (unshare(CLONE_NEWUTS) == -1)
		fatalErr("unshare");

	if (execvp(argv[1], &argv[1]) == -1)
		fatalErr("execvp");

	return 0;
}
