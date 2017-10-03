#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <helper.h>

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

	if (seteuid(99))
		fatalErr("seteuid 99");

	show_info();

	return 0;
}
