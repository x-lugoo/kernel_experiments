#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>

void show_info()
{
	uid_t uid, euid;

	uid = getuid();
	euid = geteuid();

	printf("UID: %d, EUID: %d\n", uid, euid);
}

int main(void)
{
	show_info();

	if (seteuid(99)) {
		perror("seteuid 99");
		exit(EXIT_FAILURE);
	}

	show_info();

	return 0;
}
