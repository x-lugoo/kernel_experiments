/* inspired by: https://gist.github.com/sbz/1090868/0b190b8c222689f142242fdf92e56051d4afc6da */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>

cap_value_t cap_list[CAP_LAST_CAP+1];

void set_cap(cap_t cap, cap_flag_t flag, cap_flag_value_t flag_val, int num)
{
	if (cap_set_flag(cap, flag, num, cap_list, flag_val)) {
		perror("cap_set_flag");
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	cap_t cap;

	if (getuid() != 0) {
		fprintf(stderr, "Calling from a user different from root. Aborting\n");
		exit(EXIT_FAILURE);
	}

	cap = cap_get_pid(getpid());
	if (!cap) {
		perror("cap_get_pid");
		exit(EXIT_FAILURE);
	}

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0) {
		perror("prctl");
		exit(EXIT_FAILURE);
	}

	printf("Show cap from privilegied user\n");
	/* show cap as privilegied user */
	printf("%s\n", cap_to_text(cap, NULL));

	cap_list[0] = CAP_SETUID;
	cap_list[1] = CAP_SYS_ADMIN;

	set_cap(cap, CAP_EFFECTIVE, CAP_CLEAR, 2);
	set_cap(cap, CAP_PERMITTED, CAP_CLEAR, 2);

	printf("\nRemoving CAP_SETUID and CAP_SYS_ADMIN from root\n");
	cap_set_proc(cap);

	printf("\nShow caps again\n");
	printf("%s\n", cap_to_text(cap, NULL));

	if (setuid(99) == 0) {
		printf("user: %d, %d\n", getuid(), geteuid());
		fprintf(stderr, "ERR: SETUID DIDN'T FAILED\n");
		exit(EXIT_FAILURE);
	}

	printf("setuid failed as expected\n");

	return 0;
}
