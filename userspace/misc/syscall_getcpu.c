#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(void)
{
	int cpu;
	if (syscall(309, &cpu, NULL, NULL) == -1)
		perror("syscall");

	printf("CPU: %d\n", cpu);
	return 0;
}
