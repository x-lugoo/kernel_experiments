#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/seccomp.h>
#include <sys/prctl.h>

int main(void)
{
	int ret;

	printf("Setting seccomp...\n");

	ret = prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
	if (ret) {
		perror("seccomp");
		exit(1);
	}

	printf("seccomp set, try redirecting stderr to stdout (should be killed)\n");

	dup2(1, 2);

	printf("Should not be printed\n");

	return 0;
}
