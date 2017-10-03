#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s path_to_binary\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (unshare(CLONE_NEWUTS) == -1) {
		perror("unshare");
		exit(EXIT_FAILURE);
	}

	if (execvp(argv[1], &argv[1]) == -1) {
		perror("execvp");
		exit(EXIT_FAILURE);
	}

	return 0;
}
